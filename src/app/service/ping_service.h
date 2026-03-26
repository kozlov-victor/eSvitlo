#include <Arduino.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include "../libs/server/v_service/v_service.h"
#include "../libs/v_http/v_http.h"
#include "../libs/server/v_json_lite/v_json_lite.h"

struct PingResponse {
    int code;
    String message;
};

class PingService {
private:
    VHttp* vHttp;

Service(PingService)

public:

    explicit PingService() = default;

    void setUrl(const String &url) {
        vHttp = new VHttp(url);
    }

    PingResponse call() const {

        WiFi.setSleep(false);
        esp_wifi_set_ps(WIFI_PS_NONE);

        VHttpResponse response {-1,"UNKNOWN"};

        for (int i=1;i<=3;i++) {
            if (i>1) Serial.printf("attempt %i\n",i);
            response = vHttp->get();
            if (response.code==-1) {
                delay(500*i);
            }
            else break;
        }

        PingResponse pingResponse;
        pingResponse.code = response.code;
        pingResponse.message = response.body;
        if (response.code!=200) {
            return pingResponse;
        }

        JsonValue* pingCallResponse = JsonParser::parse(response.body);
        const String status = pingCallResponse->get("status")->asString();
        if (status=="OK") {
            pingResponse.message = pingCallResponse->get("time")->asString();
        }
        else {
            pingResponse.message = pingCallResponse->get("error")->asString();
            if (pingResponse.message.isEmpty()) {
                pingResponse.message = response.body;
            }
        }
        return pingResponse;

    }


};
