#pragma once

#include <Arduino.h>
#include "../libs/server/v_base_controller/v_base_controller.h"
#include "../libs/server/v_request/v_request.h"
#include "../libs/server/v_response/v_response.h"
#include "../libs/server/v_server/v_server.h"
#include "../libs/server/v_table_multi_type/v_table_multi_type.h"
#include "../libs/ota/ota_agent.h"
#include "../firmware_version.h"

class OtaController  : public VBaseController {

private:
    OtaAgent* otaAgent;

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

    }

    ~OtaController() override {
        if (otaAgent) delete otaAgent;
    }

    void otaUpgrade(VRequest* req, VResponse* resp) {
        const String url = getEp() + "/upgrade";
        resp->startSSE();
        otaAgent->loadUpgrade(url,resp);
    }

    void otaVersion(VRequest* req, VResponse* resp) {
        VTableMultitype result;
        result.putString("version",FirmwareVersion::getFirmwareVersion());
        resp->writeJson(result);
    }

    void otaUpdate(VRequest* req, VResponse* resp) {
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
        server->getRegistry()->registerRoute<OtaController,&OtaController::otaUpgrade>(
            "/ota/upgrade","POST",this
        );
        server->getRegistry()->registerRoute<OtaController,&OtaController::otaVersion>(
            "/ota/version","POST",this
        );
        server->getRegistry()->registerRoute<OtaController,&OtaController::otaUpdate>(
            "/ota/update","POST",this
        );
    }

    boolean authorise(VRequest *request) override {
        return VAuth::checkToken(request);
    }
};