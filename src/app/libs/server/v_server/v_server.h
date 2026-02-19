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
    VRouteRegistry *registry;

    void listenToNextClient() {
        WiFiClient client = server->available();
        if (!client) return;
        Serial.println("Accepted new connection");

        String method = "";
        String url = "";
        VHashTable<String> requestHeaders;
        VTableMultitype params;

        readRequest(client, &method, &url, &requestHeaders, &params);

        VRequest req(method, &requestHeaders, &params);
        VHashTable<String> responseHeaders;
        responseHeaders.put("Connection","close");
        VResponse resp(&client, &responseHeaders);

        if (!registry->handleRequest(url, method, &req, &resp)) {
            Serial.println("Not handled");
            resp.writeStatus(V_RESPONSE_NOT_FOUND, "<h1>404</h1>");
        }
        client.flush(); // Ensure all data is sent
        client.stop();
        Serial.println("Client connection closed");
    }

    static void readRequest(WiFiClient &client, String *method, String *url, VHashTable<String> *headers,
                            VTableMultitype *params) {
        String currentLine = "";
        boolean isFirstLine = true;
        boolean isBody = false;
        String bodyRaw;
        int contentLengthCnt = 0;
        int contentLength = 0;
        const int MAX_BODY = 2048;
        const unsigned long TIMEOUT = 10000;
        unsigned long lastTime = millis();
        unsigned long currTime = lastTime;

        while (client.connected()) {
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
                    if (contentLengthCnt > MAX_BODY) {
                        Serial.println("too large body");
                        break;
                    }
                }
                if (c == '\r') continue;
                if (c == '\n') {
                    // is new line
                    if (currentLine.isEmpty()) {
                        if (*method == "GET") {
                            break;
                        }
                        if (headers->has("content-length")) {
                            contentLength = headers->get("content-length").toInt();
                            if (contentLength == 0) {
                                break;
                            }
                            bodyRaw.reserve(contentLength);
                        }
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


    static void parseHeaderLine(const String &line, VHashTable<String> *headers) {
        const int colon = line.indexOf(':');
        if (colon < 0) return;

        String key = line.substring(0, colon);
        String val = line.substring(colon + 1);

        key.trim();
        key.toLowerCase();
        val.trim();

        headers->put(key, val);
    }

    static void parseBody(const String &bodyRaw, VTableMultitype *params) {
        const VTableMultitype body = VTableMultitype::parseJson(bodyRaw);
        params->putAll(body);
    }

    static void parseQueryString(const String &qs, VTableMultitype *params) {
        int i = 0;
        while (i < (int) qs.length()) {
            int amp = qs.indexOf('&', i);
            if (amp < 0) amp = qs.length();
            String pair = qs.substring(i, amp);

            int eq = pair.indexOf('=');
            if (eq >= 0) {
                String k = pair.substring(0, eq);
                String v = pair.substring(eq + 1);
                params->putString(k, v);
            } else if (pair.length() > 0) {
                params->putString(pair, "");
            }
            i = amp + 1;
        }
    }

    static void parseFirstLine(const String &line, String *method, String *url, VTableMultitype *params) {
        // line: "GET /path?x=1 HTTP/1.1"
        const int sp1 = line.indexOf(' ');
        if (sp1 < 0) return;
        const int sp2 = line.indexOf(' ', sp1 + 1);

        *method = line.substring(0, sp1);
        if (sp2 < 0) *url = line.substring(sp1 + 1);
        else *url = line.substring(sp1 + 1, sp2);

        int q = url->indexOf('?');
        if (q >= 0) {
            const String qs = url->substring(q + 1);
            *url = url->substring(0, q);
            parseQueryString(qs, params);
        }
    }

    String login;
    String password;
    boolean wifiClient = false;
    unsigned long lastCheck = 0;
    boolean started = false;

public:
    explicit VServer(int port) : registry(new VRouteRegistry()) {
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
        started = true;
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
        WiFi.begin(ssid, pass);
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
        started = true;
        return addr;
    }


    VRouteRegistry *getRegistry() const {
        return registry;
    }

    void tick() {
        if (!started) return;
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
