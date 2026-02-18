#pragma once

#include <Arduino.h>
#include "../libs/server/v_base_controller/v_base_controller.h"
#include "../libs/server/v_server/v_server.h"
#include "../libs/server/v_request/v_request.h"
#include "../libs/server/v_response/v_response.h"
#include "../libs/static/static.h"
#include "../firmware_version.h"

class StaticController: public VBaseController {
private:
    static StaticController* self;
    String eTag;
public:
    explicit StaticController(VServer *server):VBaseController(server) {
        self = this;
        eTag = FirmwareVersion::getFirmwareVersion() + "_" + E_TAG;
    }
    void initRoutes() override {
        server->getRegistry()->registerRoute("/","GET",this,[](VRequest* req, VResponse* resp){
            resp->writeBuffer(assets_index_html, req, self->eTag);
        });
        server->getRegistry()->registerRoute("/out/index.js","GET",this,[](VRequest* req, VResponse* resp){
            resp->writeBuffer(assets_index_js, req, self->eTag);
        });
        server->getRegistry()->registerRoute("/out/all.css","GET",this,[](VRequest* req, VResponse* resp){
            resp->writeBuffer(assets_all_css, req, self->eTag);
        });
        server->getRegistry()->registerRoute("/assets/icon.png","GET",this,[](VRequest* req, VResponse* resp){
            resp->writeBuffer(assets_icon_png, req, self->eTag);
        });
    }
};


StaticController* StaticController::self = nullptr;