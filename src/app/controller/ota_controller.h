#pragma once

#include <Arduino.h>
#include "../libs/server/v_base_controller/v_base_controller.h"
#include "../libs/server/v_request/v_request.h"
#include "../libs/server/v_response/v_response.h"
#include "../libs/server/v_server/v_server.h"
#include "../libs/server/v_table_multi_type/v_table_multi_type.h"
#include "../libs/ota/ota_agent.h"
#include "../firmware_version.h"
#include "../service/app_service.h"

class OtaController  : public VBaseController {

private:
    OtaAgent* otaAgent;
    AppService* appService;

    // http://192.168.3.135:8080
    static String getEp() {
        Preferences preferences;
        preferences.begin("app", false);
        String ep = preferences.getString("ep", "");
        preferences.end();
        return ep;
    }

public:
    explicit OtaController(VServer *server):VBaseController(server), otaAgent(new OtaAgent()) {
        appService = &AppService::instance();
    }

    ~OtaController() override {
        if (otaAgent) delete otaAgent;
    }

    void upgrade(VRequest* req, VResponse* resp) {
        const String url = getEp() + "/upgrade";
        appService->log("Upgrade...");
        resp->startSSE();
        otaAgent->loadUpgrade(url,resp);
    }

    void version(VRequest* req, VResponse* resp) {
        VTableMultitype result;
        result.putString("version",FirmwareVersion::getFirmwareVersion());
        resp->writeJson(result);
    }

    void update(VRequest* req, VResponse* resp) {
        const String ep = getEp();
        VTableMultitype result;
        if (ep.isEmpty()) {
            result.putBoolean("success",false);
            result.putString("error","bad endpoint");
        }
        else {
            const String url = ep + "/update";
            const OtaResult otaResult = otaAgent->getLastVersion(url);

            result.putBoolean("success",otaResult.success);
            if (otaResult.success) {
                result.putString("version",otaResult.body);
            }
            else {
                result.putString("error",otaResult.body);
            }
        }
        resp->writeJson(result);
    }

    void initRoutes() override {
        Route("/ota/upgrade","POST",OtaController,upgrade)
        Route("/ota/version","POST",OtaController,version)
        Route("/ota/update","POST",OtaController,update)
    }

    boolean authorise(VRequest *request) override {
        return VAuth::checkToken(request);
    }
};