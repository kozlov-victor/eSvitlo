#include <Arduino.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include "../libs/server/v_table_multi_type/v_table_multi_type.h"
#include "../libs/server/v_service/v_service.h"

struct PingResponse {
    int code;
    String message;
};


class PingService {
private:
    struct ParsedUrl {
        String host;
        uint16_t port;
        String path;
    };
    ParsedUrl parsedUrl;

    static ParsedUrl parseUrl(const String& url)
    {
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
        int colonIndex = working.indexOf(':');
        if (colonIndex != -1) {
            result.host = working.substring(0, colonIndex);
            result.port = working.substring(colonIndex + 1).toInt();
        } else {
            result.host = working;
        }

        return result;
    }

Service(PingService)

public:

    explicit PingService() = default;

    void setUrl(const String &url) {
        this->parsedUrl = parseUrl(url);
    }

    PingResponse call() const {
        if (WiFi.status() != WL_CONNECTED) {
            return {-1, "no wifi"};
        }

        WiFi.setSleep(false);
        esp_wifi_set_ps(WIFI_PS_NONE);

        constexpr int MAX_RETRIES = 3;
        constexpr int RETRY_DELAY_MS = 500;
        constexpr int TIMEOUT = 10000;
        PingResponse resp{-1,"UNKNOWN"};

        for (int attempt = 1; attempt <= MAX_RETRIES; attempt++) {
            if (attempt>1) {
                Serial.print("attempt ");
                Serial.println(attempt);
            }

            WiFiClient client;
            client.setTimeout(TIMEOUT);

            Serial.print("Connecting to ");
            Serial.println(parsedUrl.host);
            Serial.println(parsedUrl.port);

            if (!client.connect(parsedUrl.host.c_str(), parsedUrl.port)) {
                Serial.println("TCP connect failed");
                resp.message = "TCP failed";
            }
            else {
                // Формуємо HTTP GET вручну
                client.print(
                    "GET " + parsedUrl.path + " HTTP/1.1\r\n" +
                    "Host: " + parsedUrl.host + "\r\n" +
                    "Connection: close\r\n\r\n"
                );


                // ===== WAIT FOR RESPONSE =====
                long start = millis();
                while (!client.available()) {
                    if (millis() - start > TIMEOUT) {
                        Serial.println("Timeout waiting response");
                        client.stop();
                        resp.message = "timeout";
                        break;
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
                        resp.message = "Status timeout";
                        break;
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

                // ===== SKIP HEADERS =====
                String headerBuffer;
                while (client.available()) {
                    char c = client.read();
                    headerBuffer += c;
                    if (headerBuffer.endsWith("\r\n\r\n")) {
                        break;
                    }
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
                        resp.message = "Body read timeout";
                        break;
                    }

                    delay(1);
                    yield();
                }

                Serial.println(body);

                const VTableMultitype pingCallResponse = VTableMultitype::parseJson(body);
                const String status = pingCallResponse.getString("status");
                if (status=="OK") {
                    resp.message = pingCallResponse.getString("time");
                }
                else {
                    resp.message = pingCallResponse.getString("error");
                    if (resp.message.isEmpty()) {
                        resp.message = body;
                    }
                }
                client.stop();
                break;
            }
            client.stop();
            delay(RETRY_DELAY_MS * attempt);
        }
        return resp;
    }


};