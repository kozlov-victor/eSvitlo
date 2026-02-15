#ifndef V_SERVER_H
#define V_SERVER_H

#include <Arduino.h>
#include <WiFi.h>
#include "../v_array_list/v_array_list.h"
#include "../v_hash_table/v_hash_table.h"
#include "../v_strings/v_strings.h"
#include "../v_route_registry/v_route_registry.h"
#include "../v_request/v_request.h"
#include "../v_response/v_response.h"
#include "../v_table_multi_type/v_table_multi_type.h"

class VServer {
private:
    WiFiServer server;

    void readRequest(WiFiClient &client, String *method, String *url, VHashTable<String> *headers,
                     VTableMultitype *params) {
        String currentLine = "";
        boolean isFirstLine = true;
        boolean isBody = false;
        String bodyRaw;
        int contentLengthCnt = 0;
        unsigned long lastTime = millis();
        unsigned long currTime = lastTime;

        while (client.connected()) {
            const unsigned long TIMEOUT = 10000;
            currTime = millis();
            unsigned long delta = currTime - lastTime;
            if (delta > TIMEOUT) {
                Serial.println("Connection timeout");
                break;
            }
            if (client.available()) {
                char c = client.read();
                lastTime = millis();
                //Serial.println(String(c) + " " + int(c));
                if (isBody) {
                    // is body, accumulate raw body untill contentLengthCnt is lt contentLength
                    bodyRaw += c;
                    contentLengthCnt++;
                    String contentLength = headers->get("content-length");
                    if (contentLength.length() > 0) {
                        if (contentLengthCnt == contentLength.toInt()) {
                            break;
                        }
                    }
                }
                if (c == '\r') continue;
                if (c == '\n') {
                    // is new line
                    if (currentLine.length() == 0) {
                        if (*method == "GET") {
                            break;
                        }
                        if (headers->get("content-length") == "0") {
                            // if body is empty
                            break;
                        }
                        isBody = true;
                    }
                    if (isFirstLine) {
                        // first line (ie GET /index HTTP 1.1)
                        isFirstLine = false;
                        this->parseFirstLine(currentLine, method, url, params);
                    } else if (!isBody) {
                        // headers
                        this->parseHeaderLine(currentLine, headers);
                    }
                    currentLine = "";
                } else {
                    // accumulate line
                    if (!isBody) currentLine += c;
                }
            }
        }
        //Serial.println("bodyRaw = "  + bodyRaw);
        this->parseBody(bodyRaw, params);
    }

    void parseFirstLine(String firstLine, String *method, String *url, VTableMultitype *params) {
        VArrayList<String> parts = VStrings::splitBy(firstLine, ' ');
        if (parts.size() >= 2) {
            *method = parts.getAt(0);
            *url = parts.getAt(1);
            if (url->indexOf("?") > -1) {
                VArrayList<String> urlParts = VStrings::splitBy(*url, '?');
                *url = urlParts.getAt(0);
                String queryString = urlParts.getAt(1);
                VArrayList<String> queryParts = VStrings::splitBy(queryString, '&');
                for (size_t i = 0; i < queryParts.size(); i++) {
                    VArrayList<String> pair = VStrings::splitBy(queryParts.getAt(i), '=');
                    params->putString(pair.getAt(0), pair.getAt(1));
                }
            }
        }
    }

    void parseHeaderLine(String line, VHashTable<String> *headers) {
        VArrayList<String> pair = VStrings::splitBy(line, ':');
        if (pair.size() >= 2) {
            String key = pair.getAt(0);
            key.trim();
            key.toLowerCase();
            String val = pair.getAt(1);
            val.trim();
            headers->put(key, val);
        }
    }

    void parseBody(String bodyRaw, VTableMultitype *params) {
        VTableMultitype body = VTableMultitype::parseJson(bodyRaw);
        params->putAll(body);
    }

    String login;
    String password;
    boolean wifiClient = false;
    unsigned long lastCheck = 0;

public:
    VServer(int port) {
        this->server = WiFiServer(port);
    }

    IPAddress setupAsAccessPoint() {
        WiFi.mode(WIFI_AP);
        WiFi.softAP("esp32", "12345678");
        Serial.println("Access point activated");
        Serial.print("IP address: ");
        IPAddress addr = WiFi.softAPIP();
        Serial.println(addr);
        this->server.begin();
        return addr;
    }

    IPAddress setupAsWifiClient(String ssid, String password) {
        this->login = ssid;
        this->password = password;
        this->wifiClient = true;
        Serial.print("Connecting to ");
        Serial.println(ssid);
        WiFi.setAutoReconnect(true);
        WiFi.persistent(false);
        WiFi.begin(ssid, password);
        while (WiFi.status() != WL_CONNECTED) {
            delay(500);
            Serial.print(".");
        }
        // Print local IP address and start web server
        Serial.println("");
        Serial.println("WiFi connected.");
        IPAddress addr = WiFi.localIP();
        Serial.println("IP address: ");
        Serial.println(addr);
        this->server.begin();
        return addr;
    }

    void listenToNextClient() {
        WiFiClient client = server.available();
        if (!client) return;
        Serial.println("Accepted new connection");

        String method = "";
        String url = "";
        VHashTable<String> headers;
        VTableMultitype params;

        this->readRequest(client, &method, &url, &headers, &params);

        VRequest req(method, &headers, &params);
        VResponse resp(&client);
        if (!VRouteRegistry::handleRequest(url, method, &req, &resp)) {
            Serial.println("Not handled");
            client.println("HTTP/1.1 404 Not Found");
            client.println("Connection: close");
            client.println("Content-type: text/html");
            client.println();
            client.println("<h1>404</h1>");
        }
        client.flush(); // Ensure all data is sent
        client.stop();
        Serial.println("Client connection closed");
    }

    void loop() {
        if (!wifiClient) return;

        if (millis() - lastCheck > 5000) {
            lastCheck = millis();

            if (WiFi.status() != WL_CONNECTED) {
                Serial.println("WiFi lost, reconnecting...");
                WiFi.disconnect();
                WiFi.begin(login, password);
                while (WiFi.status() != WL_CONNECTED) {
                    delay(1000);
                    Serial.print("-----reconnecting-----");
                    Serial.println(WiFi.status());
                }
            }
        }
    }
};

#endif
