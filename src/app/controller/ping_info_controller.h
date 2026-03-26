#pragma once

#include <Arduino.h>
#include "../libs/server/v_base_controller/v_base_controller.h"
#include "../libs/server/v_server/v_server.h"
#include "../libs/server/v_request/v_request.h"
#include "../libs/server/v_response/v_response.h"
#include "../libs/server/v_json_lite/v_json_lite.h"
#include "../libs/server/v_auth/v_auth.h"
#include "../libs/v_timer/v_timer.h"
#include "../service/app_service.h"

class PingInfoController : public VBaseController {
private:
    VTimer* timer = nullptr;
    AppService* appService = nullptr;
public:

    explicit PingInfoController(VServer *server): VBaseController(server) {
        Inject(appService,AppService)
    }

    void getTickInfo(VRequest* req, VResponse* resp) {
        JsonObjectBuilder result;
        result.setString("lastPingResponse",appService->lastPingResponse);
        result.setInt("tick",appService->tickCnt);
        result.setInt("errorCnt",appService->errorCnt);
        result.setInt("time",millis());
        result.setBool("isAccessPoint", appService->isAccessPoint);
        result.setInt("signal",WiFi.RSSI());
        resp->writeJson(result.getRoot());
    }

    void restart(VRequest* req, VResponse* resp) {
        appService->log("restart...");
        JsonObjectBuilder result;
        result.setBool("success",true);
        timer = new VTimer();
        timer->once = true;
        timer->onDone(this,[](void* ctx) {
            auto* self = static_cast<PingInfoController*>(ctx);
            delete self->timer;
            self->timer = nullptr;
            ESP.restart();
        });
        timer->start(500);
        resp->writeJson(result.getRoot());
    }

    void health(VRequest* req, VResponse* resp) {
        JsonObjectBuilder result;
        result.setBool("alive",true);
        resp->writeJson(result.getRoot());
    }

    void getScreen(VRequest* req, VResponse* resp) {
        resp->writeBuffer(appService->getBuffer(),appService->getBufferSize());
    }

    void initRoutes() override {
        Route("/ping/getTickInfo","GET",PingInfoController,getTickInfo)
        Route("/ping/restart","POST",PingInfoController,restart)
        Route("/ping/health","POST",PingInfoController,health)
        Route("/ping/getScreen","POST",PingInfoController,getScreen)
    }

    boolean authorise(VRequest *request) override {
        return VAuth::checkToken(request);
    }


    void tick() override {
        if (timer!=nullptr) timer->tick();
    }
};