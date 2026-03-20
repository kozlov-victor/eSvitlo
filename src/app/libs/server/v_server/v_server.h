#ifndef V_SERVER_H
#define V_SERVER_H

#include <Arduino.h>
#include <WiFi.h>
#include "esp_wifi.h"
#include "../v_hash_table/v_hash_table.h"
#include "../v_route_registry/v_route_registry.h"
#include "../v_request/v_request.h"
#include "../v_response/v_response.h"
#include "../v_table_multi_type/v_table_multi_type.h"

class VServer {
private:
    WiFiServer *server;
    VRouteRegistry *registry;
    void* context = nullptr;
    void (*onConnectionLostCb)(void*) = nullptr;
    void (*onReconnectedCb)(void*) = nullptr;
    bool reconnecting = false;
    const int MAX_REQ_PER_CONN = 8;

    void checkStatus() {

        static uint32_t hb = 0;
        if (millis() - hb > 5000) {
            hb = millis();
            Serial.printf("HB wifi=%d heap=%u min=%u\n", WiFi.status(), ESP.getFreeHeap(), ESP.getMinFreeHeap());
        }

        if (wifiClient) {
            if (millis() - lastCheck > 5000) {
                lastCheck = millis();
                if (WiFi.status() != WL_CONNECTED) {
                    Serial.println("WiFi lost, trying reconnect...");
                    if (onConnectionLostCb) onConnectionLostCb(context);
                    WiFi.reconnect();
                    reconnecting = true;
                }
            }
            // не блокуємо loop
            if (reconnecting && WiFi.status() == WL_CONNECTED) {
                reconnecting = false;
                Serial.println("WiFi reconnected");
                if (onReconnectedCb) this->onReconnectedCb(this->context);
            }
        }
    }

    void listenToNextClient() {
        WiFiClient client = server->available();
        if (!client) return;

        client.setNoDelay(true);

        Serial.println("Accepted new connection");

        const uint32_t IDLE_BETWEEN_REQUESTS_MS = 400; // скільки чекаємо наступний request на keep-alive

        for (int r = 0; r < MAX_REQ_PER_CONN && client.connected(); r++) {
            // дочекайся початку наступного request (або вихід по idle)
            uint32_t t1 = millis();
            while (client.connected() && !client.available() && (millis() - t1) < IDLE_BETWEEN_REQUESTS_MS) yield();
            if (!client.available()) break;

            String method, url;
            VHashTable<String> requestHeaders;
            VTableMultitype params;

            readRequest(client, &method, &url, &requestHeaders, &params);
            if (method.length() == 0) break;

            VRequest req(method, &requestHeaders, &params);
            VHashTable<String> responseHeaders;
            VResponse resp(&client, &responseHeaders);

            // якщо клієнт явно просить close — закриваємо
            bool wantClose = false;
            if (requestHeaders.has("connection")) {
                String c = requestHeaders.get("connection");
                c.toLowerCase();
                if (c.indexOf("close") >= 0) wantClose = true;
            }

            if (wantClose) {
                responseHeaders.put("Connection", "close");
            } else {
                responseHeaders.put("Connection", "keep-alive");
                responseHeaders.put("Keep-Alive", "timeout=3, max=8");
            }

            if (!registry->handleRequest(url, method, &req, &resp)) {
                resp.writeStatus(V_RESPONSE_NOT_FOUND, "<h1>404</h1>");
            }

            if (wantClose) break;
        }

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
        const unsigned long TIMEOUT = 1500;
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

                        // якщо є chunked — зараз не підтримуємо, але й не висимо
                        if (headers->has("transfer-encoding") &&
                            headers->get("transfer-encoding").indexOf("chunked") >= 0) {
                            Serial.println("Chunked request body not supported -> break");
                            break;
                        }

                        if (headers->has("content-length")) {
                            contentLength = headers->get("content-length").toInt();
                            if (contentLength == 0) {
                                break;
                            }
                            bodyRaw.reserve(contentLength);
                        }
                        else {
                            Serial.println("Content-length not provided -> break");
                            break;
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
            else {
                yield();
            }
        }
        //Serial.println("bodyRaw = "  + bodyRaw);
        if (headers->has("content-type") && headers->get("content-type").indexOf("/json") >= 0) {
            parseBody(bodyRaw, params);
        }
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

    void onConnectionLost(void* context, void (*callback)(void*)) {
        this->context = context;
        this->onConnectionLostCb = callback;
    }

    void onReconnected(void* context, void (*callback)(void*)) {
        this->context = context;
        this->onReconnectedCb = callback;
    }

    IPAddress setupAsAccessPoint() {
        WiFi.mode(WIFI_AP);
        WiFi.softAP("esp32", "12345678");
        Serial.println("Access point activated");
        Serial.print("IP address: ");
        IPAddress addr = WiFi.softAPIP();
        Serial.println(addr);
        server->setNoDelay(true);
        server->begin(80,MAX_REQ_PER_CONN);
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
        WiFi.mode(WIFI_STA);
        WiFi.setSleep(false);
        esp_wifi_set_ps(WIFI_PS_NONE);
        server->setNoDelay(true);

        wifi_country_t country;
        strcpy(country.cc, "UA");
        country.schan = 1;
        country.nchan = 13;
        country.max_tx_power = 20;     // обовʼязкове поле в IDF 5
        country.policy = WIFI_COUNTRY_POLICY_AUTO;

        esp_wifi_set_country(&country);

        WiFi.begin(ssid, pass);
        while (WiFi.status() != WL_CONNECTED) {
            yield();
            delay(500);
            Serial.print(".");
        }
        // Print local IP address and start web server
        Serial.println("");
        Serial.println("WiFi connected.");
        IPAddress addr = WiFi.localIP();
        Serial.println("IP address: ");
        Serial.println(addr);
        server->begin(80,MAX_REQ_PER_CONN);
        started = true;
        return addr;
    }


    VRouteRegistry *getRegistry() const {
        return registry;
    }

    void tick() {
        if (!started) return;
        checkStatus();
        listenToNextClient();
        registry->tick();
    }
};

#endif
