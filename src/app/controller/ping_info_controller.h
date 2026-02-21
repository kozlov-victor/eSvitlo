#pragma once

#include <Arduino.h>
#include "../libs/server/v_base_controller/v_base_controller.h"
#include "../libs/server/v_server/v_server.h"
#include "../libs/server/v_request/v_request.h"
#include "../libs/server/v_response/v_response.h"
#include "../libs/server/v_table_multi_type/v_table_multi_type.h"
#include "../libs/server/v_auth/v_auth.h"
#include "../libs/v_timer/v_timer.h"


class PingInfoController : public VBaseController {
private:
    VTimer* timer = nullptr;
public:

    explicit PingInfoController(VServer *server): VBaseController(server) {

    }

    String lastPingResponse = "";
    boolean isAccessPoint = false;
    long tickCnt = 0;

    void getTickInfo(VRequest* req, VResponse* resp) {
        VTableMultitype result;
        result.putString("lastPingResponse",lastPingResponse);
        result.putInt("tick",tickCnt);
        result.putInt("time",millis());
        result.putBoolean("isAccessPoint", isAccessPoint);
        result.putInt("signal",WiFi.RSSI());
        resp->writeJson(result);
    }

    void restart(VRequest* req, VResponse* resp) {
        VTableMultitype result;
        result.putBoolean("success",true);
        timer = new VTimer();
        timer->once = true;
        timer->onDone(this,[](void* ctx) {
            auto* self = static_cast<PingInfoController*>(ctx);
            delete self->timer;
            self->timer = nullptr;
            ESP.restart();
        });
        timer->start(500);
        resp->writeJson(result);
    }

    void health(VRequest* req, VResponse* resp) {
        VTableMultitype result;
        result.putBoolean("alive",true);
        resp->writeJson(result);
    }

    void initRoutes() override {
        server->getRegistry()->registerRoute<PingInfoController,&PingInfoController::getTickInfo>(
            "/ping/getTickInfo","GET",this
        );
        server->getRegistry()->registerRoute<PingInfoController,&PingInfoController::restart>(
            "/ping/restart","POST",this
        );
        server->getRegistry()->registerRoute<PingInfoController,&PingInfoController::health>(
            "/ping/health","POST",this
        );
    }

    boolean authorise(VRequest *request) override {
        return VAuth::checkToken(request);
    }


    void tick() override {
        if (timer!=nullptr) timer->tick();
    }
};