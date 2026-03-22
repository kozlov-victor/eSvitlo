#ifndef VHTTP_H
#define VHTTP_H
#include <Arduino.h>

#include "../server/v_hash_table/v_hash_table.h"
#include "../../_stubs/stubs.h"

struct VHttpResponse {
    int code;
    String body;
};

class VHttp {
private:
    struct ParsedUrl {
        String host;
        uint16_t port;
        String path;
    };
    ParsedUrl parsedUrl;

    static ParsedUrl parseUrl(const String& url) {
        ParsedUrl result;
        result.port = 80;      // default HTTP
        result.path = "/";

        String working = url;

        // Прибрати http:// якщо є
        if (working.startsWith("http://")) {
            working.remove(0, 7);
        }

        // Відділити path
        int slashIndex = working.indexOf('/');
        if (slashIndex != -1) {
            result.path = working.substring(slashIndex);
            working = working.substring(0, slashIndex);
        }

        // Перевірити порт
        const int colonIndex = working.indexOf(':');
        if (colonIndex != -1) {
            result.host = working.substring(0, colonIndex);
            result.port = working.substring(colonIndex + 1).toInt();
        } else {
            result.host = working;
        }

        return result;
    }

    VHttpResponse call(const String &method, VHashTable<String> *headers = nullptr, String requestBody = "") const {

        if (WiFi.status() != WL_CONNECTED) {
            return {-1, "no wifi"};
        }

        constexpr int TIMEOUT = 10000;
        VHttpResponse resp{-1,"UNKNOWN"};

        WiFiClient client;
        client.setTimeout(TIMEOUT);

        Serial.print("Connecting to ");
        Serial.println(parsedUrl.host);
        Serial.println(parsedUrl.port);

        if (!client.connect(parsedUrl.host.c_str(), parsedUrl.port)) {
            Serial.println("TCP connect failed");
            resp.body = "TCP failed";
            client.stop();
            return resp;
        }

        client.print( method + " " + parsedUrl.path + " HTTP/1.1\r\n");
        client.print("Host: " + parsedUrl.host + "\r\n");
        if (headers) {
            headers->forEach([&client](const String &key, const String &val) {
                client.print(key + ": " + val + "\r\n");
            });
        }
        if (!headers || !headers->has("User-Agent")) client.print("User-Agent: ESP32\r\n");
        if (!headers || !headers->has("Accept")) client.print("Accept: */*\r\n");
        if (!headers || !headers->has("Content-Length")) client.print("Content-Length: " + String(requestBody.length()) + "\r\n");
        client.print("Connection: close\r\n");
        client.print("\r\n");
        if (requestBody.length() > 0) {
            client.print(requestBody);
        }

        // ===== WAIT FOR RESPONSE =====
        long start = millis();
        while (!client.available()) {
            if (millis() - start > TIMEOUT) {
                Serial.println("Timeout waiting response");
                client.stop();
                resp.body = "timeout";
                return resp;
            }
            delay(1);
            yield();
        }

        // ===== READ STATUS LINE =====
        String statusLine;
        start = millis();

        while (true) {
            if (client.available()) {
                char c = client.read();
                statusLine += c;

                if (statusLine.endsWith("\r\n")) {
                    break;
                }
            }

            if (millis() - start > TIMEOUT) {
                Serial.println("Status line timeout");
                client.stop();
                resp.body = "Status timeout";
                return resp;
            }

            delay(1);
            yield();
        }

        statusLine.trim();

        int statusCode = -1;
        if (statusLine.startsWith("HTTP/1.1")) {
            statusCode = statusLine.substring(9, 12).toInt();
        }
        resp.code = statusCode;

        // ===== HEADERS =====
        String headerBuffer;
        start = millis();

        while (true) {
            if (client.available()) {
                const char c = client.read();
                headerBuffer += c;

                if (headerBuffer.endsWith("\r\n\r\n")) {
                    break;
                }
            }

            if (millis() - start > TIMEOUT) {
                Serial.println("Header timeout");
                client.stop();
                resp.body = "Header timeout";
                return resp;
            }

            delay(1);
            yield();
        }

        // ===== READ BODY =====
        String body;
        start = millis();

        while (client.connected() || client.available()) {

            while (client.available()) {
                body += (char)client.read();
                start = millis(); // reset timeout якщо щось прийшло
            }

            if (millis() - start > TIMEOUT) {
                Serial.println("Body read timeout");
                resp.body = "Body read timeout";
                return resp;
            }
            delay(1);
            yield();
        }
        Serial.println(body);
        resp.body = body;
        client.stop();
        return resp;
    }

public:
    explicit VHttp(const String& url) {
        this->parsedUrl = parseUrl(url);
    }

    VHttpResponse get(VHashTable<String> *headers = nullptr, const String &requestBody = "") const {
        return call("GET", headers, requestBody);
    }

    VHttpResponse post(VHashTable<String> *headers = nullptr, const String &requestBody = "") const {
        return call("POST", headers, requestBody);
    }

};

#endif //VHTTP_H