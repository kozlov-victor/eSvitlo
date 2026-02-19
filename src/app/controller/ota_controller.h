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
    static OtaController* self;

    // http://192.168.3.135:8080
    String getEp() {
        Preferences preferences;
        preferences.begin("app", false);
        String ep = preferences.getString("ep", "");
        preferences.end();
        return ep;
    }

public:
    explicit OtaController(VServer *server):VBaseController(server), otaAgent(new OtaAgent()) {
        self = this;
    }

    ~OtaController() override {
        if (otaAgent) delete otaAgent;
    }

    void initRoutes() override {
        server->getRegistry()->registerRoute("/ota/upgrade","POST",this,[](VRequest* req, VResponse* resp){
            const String url = self->getEp() + "/upgrade";
            const OtaResult otaResult = self->otaAgent->loadUpdate(url);
            Serial.println(otaResult.body);
            VTableMultitype result;
            result.putString("status",otaResult.body);
            result.putBoolean("success",otaResult.success);
            resp->writeJson(result);
        });
        server->getRegistry()->registerRoute("/ota/version","POST",this,[](VRequest* req, VResponse* resp){
            VTableMultitype result;
            result.putString("version",FirmwareVersion::getFirmwareVersion());
            resp->writeJson(result);
        });
        server->getRegistry()->registerRoute("/ota/update","POST",this,[](VRequest* req, VResponse* resp){
            const String ep = self->getEp();
            VTableMultitype result;
            if (ep.isEmpty()) {
                result.putBoolean("success",false);
                result.putString("error","bad endpoint");
            }
            else {
                const String url = ep + "/update";
                const OtaResult otaResult = self->otaAgent->getLastVersion(url);

                result.putBoolean("success",otaResult.success);
                if (otaResult.success) {
                    result.putString("version",otaResult.body);
                }
                else {
                    result.putString("error",otaResult.body);
                }
            }
            resp->writeJson(result);
        });
    }

    boolean authorise(VRequest *request) override {
        return VAuth::checkToken(request);
    }
};

OtaController* OtaController::self = nullptr;