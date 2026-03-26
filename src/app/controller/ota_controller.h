#pragma once

#include <Arduino.h>
#include <Preferences.h>

#include "../libs/server/v_base_controller/v_base_controller.h"
#include "../libs/server/v_request/v_request.h"
#include "../libs/server/v_response/v_response.h"
#include "../libs/server/v_server/v_server.h"
#include "../libs/ota/ota_agent.h"
#include "../firmware_version.h"
#include "../service/app_service.h"
#include "../libs/server/v_json_lite/v_json_lite.h"
#include "../libs/server/v_auth/v_auth.h"

class OtaController  : public VBaseController {

private:
    OtaAgent* otaAgent;
    AppService* appService;

    static String getEp() {
        Preferences preferences;
        preferences.begin("app", false);
        String ep = preferences.getString("ep", "");
        preferences.end();
        return ep;
    }

public:
    explicit OtaController(VServer *server):VBaseController(server), otaAgent(new OtaAgent()) {
        Inject(appService,AppService)
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
        JsonObjectBuilder result;
        result.setString("version",FirmwareVersion::getFirmwareVersion());
        resp->writeJson(result.getRoot());
    }

    void update(VRequest* req, VResponse* resp) {
        const String ep = getEp();
        JsonObjectBuilder result;
        if (ep.isEmpty()) {
            result.setBool("success",false);
            result.setString("error","bad endpoint");
        }
        else {
            const String url = ep + "/update";
            const OtaResult otaResult = otaAgent->getLastVersion(url);

            result.setBool("success",otaResult.success);
            if (otaResult.success) {
                result.setString("version",otaResult.body);
            }
            else {
                result.setString("error",otaResult.body);
            }
        }
        resp->writeJson(result.getRoot());
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
