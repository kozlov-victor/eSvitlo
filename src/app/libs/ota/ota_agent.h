#ifndef OTAAGENT_H
#define OTAAGENT_H

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <Update.h>
#include "../server/v_response/v_response.h"
#include "../v_http/v_http.h"

struct OtaResult {
    bool success;
    String body;
};

struct OtaProgressResult {
    bool completed = false;
    boolean progress = false;
    boolean success = false;
    String body = "";
};

class OtaAgent {
private:
    void addHeader(HTTPClient &http) {
        http.addHeader("X-eSvitlo-app", "eSvitlo-ESP32-device");
    }


    static void writeProgress(const OtaProgressResult &result, VTableMultitype &body, VResponse *resp) {
        body.putBoolean("success", result.success);
        body.putBoolean("progress", result.progress);
        body.putBoolean("completed", result.completed);
        body.putString("body", result.body);
        resp->sendSSE(body.stringify());
    }

public:

    OtaResult getLastVersion(const String &firmwareUrl) {
        const VHttp vHttp = VHttp(firmwareUrl);
        VHashTable<String> headers;
        headers.put("X-eSvitlo-app", "eSvitlo-ESP32-device");
        const VHttpResponse response = vHttp.get(&headers);

        OtaResult result = {false, response.body};
        if (response.code!=200) {
            return result;
        }
        const String version = VTableMultitype::parseJson(response.body).getString("version");
        return {true, version};
    }

    void loadUpgrade(const String &firmwareUrl, VResponse *resp) {
        HTTPClient http;
        http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);

        WiFiClient *clientPtr = nullptr;
        WiFiClientSecure secureClient;
        WiFiClient plainClient;

        if (firmwareUrl.startsWith("https://")) {
            secureClient.setInsecure();
            clientPtr = &secureClient;
        } else {
            clientPtr = &plainClient;
        }

        http.begin(*clientPtr, firmwareUrl);
        addHeader(http);
        int httpCode = http.GET();

        VTableMultitype respMessage;
        OtaProgressResult progressResult;

        if (httpCode == HTTP_CODE_OK) {
            int contentLength = http.getSize();
            Serial.println("Content-Length: " + String(contentLength));
            if (contentLength<=0) {
                progressResult.completed = true;
                progressResult.success = false;
                progressResult.progress = false;
                progressResult.body = "bad content length " + String(contentLength);
                writeProgress(progressResult, respMessage, resp);
                return;
            }

            if (!Update.begin(contentLength)) {
                Serial.println("Not enough space");
                http.end();
                progressResult.completed = true;
                progressResult.success = false;
                progressResult.progress = false;
                progressResult.body = "No space";
                writeProgress(progressResult, respMessage, resp);
                return;
            }

            WiFiClient* stream = http.getStreamPtr();
            uint8_t buf[512];
            size_t written = 0;
            const unsigned long TIMEOUT = 10000;
            unsigned long lastTime = millis();
            unsigned long currTime = lastTime;
            int loop = 0;

            while (written < contentLength) {

                currTime = millis();
                const unsigned long delta = currTime - lastTime;
                if (delta > TIMEOUT) {
                    Serial.println("Timeout");
                    http.end();
                    progressResult.completed = true;
                    progressResult.success = false;
                    progressResult.progress = false;
                    progressResult.body = "Read timeout";
                    writeProgress(progressResult, respMessage, resp);
                    return;
                }

                const int len = stream->readBytes(buf, sizeof(buf));

                if (len > 0) {
                    lastTime = millis();
                    size_t w = Update.write(buf, len);
                    if (w != len) {
                        Serial.println("Write failed");
                        http.end();
                        progressResult.completed = true;
                        progressResult.success = false;
                        progressResult.progress = false;
                        progressResult.body = "Write failed";
                        writeProgress(progressResult, respMessage, resp);
                        return;
                    }

                    written += w;
                    int progress = written * 100 / contentLength;
                    Serial.println("written: " + String(written));
                    if ((loop % 100)==0) {
                        progressResult.completed = false;
                        progressResult.success = true;
                        progressResult.progress = true;
                        progressResult.body = String(progress);
                        writeProgress(progressResult, respMessage, resp);
                    }
                    loop++;
                } else {
                    yield();
                }
            }

            progressResult.completed = false;
            progressResult.success = true;
            progressResult.progress = true;
            progressResult.body = String(100);
            writeProgress(progressResult, respMessage, resp);
            delay(200);

            if (!Update.isFinished()) {
                Serial.println("Update not finished");
                http.end();
                Serial.println("Error: " + String(Update.getError()));
                progressResult.completed = true;
                progressResult.success = false;
                progressResult.progress = false;
                progressResult.body = "Incomplete";
                writeProgress(progressResult, respMessage, resp);
                return;
            }

            if (Update.end()) {
                Serial.println("Complete");
                http.end();
                progressResult.completed = true;
                progressResult.success = true;
                progressResult.progress = false;
                progressResult.body = "OK";
                writeProgress(progressResult, respMessage, resp);
            } else {
                http.end();
                progressResult.completed = true;
                progressResult.success = false;
                progressResult.progress = false;
                progressResult.body = "Update failed";
                writeProgress(progressResult, respMessage, resp);
            }

        } else {
            http.end();
            progressResult.completed = true;
            progressResult.success = false;
            progressResult.progress = false;
            progressResult.body = "HTTP error " + String(httpCode);
            writeProgress(progressResult, respMessage, resp);
        }
    }

};

#endif //OTAAGENT_H