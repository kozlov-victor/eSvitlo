#pragma once

#include <Arduino.h>
#include "../libs/server/v_base_controller/v_base_controller.h"
#include "../libs/server/v_server/v_server.h"
#include "../libs/server/v_request/v_request.h"
#include "../libs/server/v_response/v_response.h"
#include "../libs/server/v_table_multi_type/v_table_multi_type.h"
#include "../libs/server/v_auth/v_auth.h"


class PingInfoController : public VBaseController {
private:
    static PingInfoController* self;
    VTimer* timer = nullptr;
public:

    explicit PingInfoController(VServer *server): VBaseController(server) {
        self = this;
    }

    String lastPingResponse = "";
    boolean isAccessPoint = false;
    long tickCnt = 0;
    void initRoutes() override {
        server->getRegistry()->registerRoute("/ping/getTickInfo","GET",this,[](VRequest* req, VResponse* resp){
            VTableMultitype result;
            result.putString("lastPingResponse",self->lastPingResponse);
            result.putInt("tick",self->tickCnt);
            result.putInt("time",millis());
            result.putBoolean("isAccessPoint",self->isAccessPoint);
            result.putInt("signal",WiFi.RSSI());
            resp->writeJson(result);
        });
        server->getRegistry()->registerRoute("/ping/restart","POST",this,[](VRequest* req, VResponse* resp){
            VTableMultitype result;
            result.putBoolean("success",true);
            resp->writeJson(result);
            self->timer = new VTimer();
            self->timer->once = true;
            self->timer->callback = []() {
                delete self->timer;
                self->timer = nullptr;
                ESP.restart();
            };
            self->timer->start(500);
        });
        server->getRegistry()->registerRoute("/ping/health","POST",this,[](VRequest* req, VResponse* resp){
            VTableMultitype result;
            result.putBoolean("alive",true);
            resp->writeJson(result);
        });
    }

    boolean authorise(VRequest *request) override {
        return VAuth::checkToken(request);
    }


    void tick() override {
        if (timer!=nullptr) timer->tick();
    }
};

PingInfoController* PingInfoController::self = nullptr;