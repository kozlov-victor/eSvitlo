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
    WiFiServer *server;
    VRouteRegistry* registry;

    void listenToNextClient() {
        WiFiClient client = server->available();
        if (!client) return;
        Serial.println("Accepted new connection");

        String method = "";
        String url = "";
        VHashTable<String> headers;
        VTableMultitype params;

        readRequest(client, &method, &url, &headers, &params);

        VRequest req(method, &headers, &params);
        VResponse resp(&client);
        if (!registry->handleRequest(url, method, &req, &resp)) {
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

    static void readRequest(WiFiClient &client, String *method, String *url, VHashTable<String> *headers,
                            const VTableMultitype *params) {
        String currentLine = "";
        boolean isFirstLine = true;
        boolean isBody = false;
        String bodyRaw;
        int contentLengthCnt = 0;
        int contentLength = 0;
        unsigned long lastTime = millis();
        unsigned long currTime = lastTime;

        while (client.connected()) {
            const unsigned long TIMEOUT = 10000;
            currTime = millis();
            const unsigned long delta = currTime - lastTime;
            if (delta > TIMEOUT) {
                Serial.println("Connection timeout");
                break;
            }
            if (client.available()) {
                const char c = client.read();
                lastTime = millis();
                //Serial.println(String(c) + " " + int(c));
                if (isBody) {
                    // is body, accumulate raw body until contentLengthCnt is lt contentLength
                    bodyRaw += c;
                    contentLengthCnt++;
                    if (contentLength > 0) {
                        if (contentLengthCnt == contentLength) {
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
                        if (headers->has("content-length")) {
                            contentLength = headers->get("content-length").toInt();
                        }
                        if (contentLength==0) {
                            // if body is empty
                            break;
                        }
                        bodyRaw.reserve(contentLength);
                        isBody = true;
                    }
                    if (isFirstLine) {
                        // first line (ie GET /index HTTP 1.1)
                        isFirstLine = false;
                        parseFirstLine(currentLine, method, url, params);
                    } else if (!isBody) {
                        // headers
                        parseHeaderLine(currentLine, headers);
                    }
                    currentLine = "";
                } else {
                    // accumulate line
                    if (!isBody) currentLine += c;
                }
            }
            yield();
        }
        //Serial.println("bodyRaw = "  + bodyRaw);
        parseBody(bodyRaw, params);
    }

    static void parseFirstLine(const String &firstLine, String *method, String *url, const VTableMultitype *params) {
        const auto parts = VStrings::splitBy(firstLine, ' ');
        if (parts->size() >= 2) {
            *method = parts->getAt(0).toString();
            *url = parts->getAt(1).toString();
            if (url->indexOf("?") > -1) {
                const auto urlParts = VStrings::splitBy(parts->getAt(1), '?');
                *url = urlParts->getAt(0).toString();
                const StringSegment queryString = urlParts->getAt(1);
                const auto queryParts = VStrings::splitBy(queryString, '&');
                for (size_t i = 0; i < queryParts->size(); i++) {
                    const auto pair = VStrings::splitBy(queryParts->getAt(i), '=');
                    if (pair->size()>=2) {
                        params->putString(pair->getAt(0).toString(), pair->getAt(1).toString());
                    }
                    else {
                        params->putString(pair->getAt(0).toString(), "");
                    }
                    delete pair;
                }
                delete queryParts;
                delete urlParts;
            }
        }
        delete parts;
    }

    static void parseHeaderLine(const String &line, VHashTable<String> *headers) {
        const auto pair = VStrings::splitBy(line, ':');
        if (pair->size() >= 2) {
            String key = pair->getAt(0).toString();
            key.trim();
            key.toLowerCase();
            String val = pair->getAt(1).toString();
            val.trim();
            headers->put(key, val);
        }
        delete pair;
    }

    static void parseBody(const String &bodyRaw, const VTableMultitype *params) {
        const VTableMultitype body = VTableMultitype::parseJson(bodyRaw);
        params->putAll(body);
    }

    String login;
    String password;
    boolean wifiClient = false;
    unsigned long lastCheck = 0;

public:
    explicit VServer(int port):registry(new VRouteRegistry()) {
        server = new WiFiServer(port);
    }

    ~VServer() {
        delete registry;
        delete server;
    }

    IPAddress setupAsAccessPoint() {
        WiFi.mode(WIFI_AP);
        WiFi.softAP("esp32", "12345678");
        Serial.println("Access point activated");
        Serial.print("IP address: ");
        IPAddress addr = WiFi.softAPIP();
        Serial.println(addr);
        server->begin();
        return addr;
    }

    IPAddress setupAsWifiClient(String ssid, String pass) {
        login = ssid;
        password = pass;
        wifiClient = true;
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
        server->begin();
        return addr;
    }


    VRouteRegistry* getRegistry() const {
        return registry;
    }

    void tick() {
        if (wifiClient) {
            if (millis() - lastCheck > 5000) {
                Serial.println("tick");
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
        listenToNextClient();
        registry->tick();
    }
};

#endif
