#ifndef OTAAGENT_H
#define OTAAGENT_H

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <Update.h>

struct OtaResult {
    bool success;
    String body;
};

class OtaAgent {
private:
    OtaResult get(String url) {
        HTTPClient http;

        http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);

        if (url.startsWith("https://")) {
            WiFiClientSecure client;
            http.begin(client, url);
        }
        else {
            WiFiClient client;
            http.begin(client, url);
        }

        int httpCode = http.GET();
        Serial.println(httpCode);

        if (httpCode != HTTP_CODE_OK) {
            http.end();
            return {false, "Api error: " + String(httpCode)};
        }

        String payload = http.getString();
        http.end();
        Serial.println(payload);
        return {true, payload};

    }
public:

    OtaResult getLastVersion(String firmwareUrl) {
        OtaResult result = get(firmwareUrl);
        if (!result.success) return result;
        String version = VTableMultitype::parseJson(result.body).getString("version");
        return {true, version};
    }

    OtaResult loadUpdate(String firmwareUrl) {
        HTTPClient http;

        http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);

        if (firmwareUrl.startsWith("https://")) {
            WiFiClientSecure client;
            http.begin(client, firmwareUrl);
        }
        else {
            WiFiClient client;
            http.begin(client, firmwareUrl);
        }

        int httpCode = http.GET();

        if (httpCode == HTTP_CODE_OK) {
            int contentLength = http.getSize();
            Serial.println("Content-Length: " + String(contentLength));
            if (contentLength<=0) {
                return {false, "bad content length " + String(contentLength)};
            }

            if (!Update.begin(contentLength)) {
                Serial.println("Not enough space");
                http.end();
                return {false, "No space"};
            }

            WiFiClient* stream = http.getStreamPtr();
            uint8_t buf[512];
            size_t written = 0;

            while (written < contentLength) {

                int len = stream->readBytes(buf, sizeof(buf));

                if (len > 0) {

                    size_t w = Update.write(buf, len);
                    if (w != len) {
                        Serial.println("Write failed");
                        http.end();
                        return {false, "Write failed"};
                    }

                    written += w;
                    Serial.println("written: " + String(written));

                } else {
                    delay(1);
                }
            }

            if (!Update.isFinished()) {
                Serial.println("Update not finished");
                http.end();
                Serial.println("Error: " + String(Update.getError()));
                return {false, "Incomplete"};
            }

            if (Update.end()) {
                Serial.println("Complete");
                http.end();
                return {true, "OK"};
            } else {
                http.end();
                return {false, "Update failed"};
            }

        } else {
            http.end();
            return {false, "HTTP error " + String(httpCode)};
        }
    }

};

#endif //OTAAGENT_H