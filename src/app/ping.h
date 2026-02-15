#include <Arduino.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include "../app/libs/server/v_table_multi_type/v_table_multi_type.h"

struct PingResponse {
    int code;
    String message;
};

class Ping {
private:
    String url;

public:

    void setUrl(const String &url) {
        this->url = url;
    }

    PingResponse call() {
        if (WiFi.status() != WL_CONNECTED) {
            return {-1, "wifi disconnected"};
        }

        WiFi.setSleep(false);

        constexpr int MAX_RETRIES = 3;
        constexpr int RETRY_DELAY_MS = 500;
        PingResponse resp{-1,"UNKNOWN"};

        for (int attempt = 1; attempt <= MAX_RETRIES; attempt++) {
            WiFiClient client;
            HTTPClient http;

            if (attempt>1) {
                Serial.print("attempt ");
                Serial.println(attempt);
            }
            Serial.println("url: " + url);

            http.setTimeout(8000);
            http.begin(client, url);

            int code = http.GET();
            Serial.print("HTTP Response code: "); Serial.println(code);
            resp.code = code;

            if (code > 0) {
                String respText = http.getString();
                Serial.println(respText);
                const VTableMultitype pingCallResponse = VTableMultitype::parseJson(respText);
                String status = pingCallResponse.getString("status");
                if (status=="OK") {
                    resp.message = pingCallResponse.getString("time");
                }
                else {
                    resp.message = pingCallResponse.getString("error");
                    if (resp.message.isEmpty()) {
                        resp.message = respText;
                    }
                }
                http.end();
                client.stop();
                break;
            }

            http.end();
            client.stop();
            delay(RETRY_DELAY_MS);
        }

        return resp;
    }


};