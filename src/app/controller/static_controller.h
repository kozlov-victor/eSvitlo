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
    String eTag;
public:
    explicit StaticController(VServer *server):VBaseController(server) {
        eTag = FirmwareVersion::getFirmwareVersion() + "_" + E_TAG;
    }

    void index(VRequest* req, VResponse* resp) {
        resp->writeBuffer(assets_index_html, req, eTag);
    }

    void indexJs(VRequest* req, VResponse* resp) {
        resp->writeBuffer(assets_index_js, req, eTag);
    }

    void allCss(VRequest* req, VResponse* resp) {
        resp->writeBuffer(assets_all_css, req, eTag);
    }

    void favicon(VRequest* req, VResponse* resp) {
        resp->writeBuffer(assets_icon_png, req, eTag);
    }

    void initRoutes() override {
        server->getRegistry()->registerRoute<StaticController,&StaticController::index>(
            "/","GET", this
        );
        server->getRegistry()->registerRoute<StaticController,&StaticController::indexJs>(
            "/out/index.js","GET", this
        );
        server->getRegistry()->registerRoute<StaticController,&StaticController::allCss>(
            "/out/all.css","GET", this
        );
        server->getRegistry()->registerRoute<StaticController,&StaticController::favicon>(
            "/assets/icon.png","GET", this
        );
    }
};