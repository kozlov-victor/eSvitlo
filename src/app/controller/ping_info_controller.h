
#include <Arduino.h>
#include "../libs/server/v_base_controller/v_base_controller.h"
#include "../libs/server/v_route_registry/v_route_registry.h"
#include "../libs/server/v_request/v_request.h"
#include "../libs/server/v_response/v_response.h"
#include "../libs/server/v_table_multi_type/v_table_multi_type.h"


class PingInfoController : public VBaseController {
public:
    static String lastPingResponse;
    static boolean isAccessPoint;
    static long tick;
    void initRoutes() override {
        VRouteRegistry::registerRoute("/ping/getTickInfo","GET",[](VRequest* req, VResponse* resp){
            VTableMultitype result;
            result.putString("lastPingResponse",lastPingResponse);
            result.putInt("tick",tick);
            result.putInt("time",millis());
            result.putBoolean("isAccessPoint",isAccessPoint);
            result.putInt("signal",WiFi.RSSI());
            resp->writeJson(result);
        });
        VRouteRegistry::registerRoute("/ping/restart","POST",[](VRequest* req, VResponse* resp){
            VTableMultitype result;
            result.putBoolean("success",true);
            resp->writeJson(result);
            ESP.restart();
        });
    }
};

String PingInfoController::lastPingResponse = "";
long PingInfoController::tick = 0;
boolean PingInfoController::isAccessPoint = false;