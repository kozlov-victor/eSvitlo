#include <Arduino.h>
#include "../libs/server/v_base_controller/v_base_controller.h"
#include "../libs/server/v_route_registry/v_route_registry.h"
#include "../libs/server/v_request/v_request.h"
#include "../libs/server/v_response/v_response.h"
#include "../libs/static/static.h"

class StaticController: public VBaseController {
public:
    void initRoutes() override {
        VRouteRegistry::registerRoute("/","GET",[](VRequest* req, VResponse* resp){
            resp->writeBuffer(assets_index_html, req, E_TAG);
        });
        VRouteRegistry::registerRoute("/out/index.js","GET",[](VRequest* req, VResponse* resp){
            resp->writeBuffer(assets_index_js, req, E_TAG);
        });
        VRouteRegistry::registerRoute("/out/all.css","GET",[](VRequest* req, VResponse* resp){
            resp->writeBuffer(assets_all_css, req, E_TAG);
        });
        VRouteRegistry::registerRoute("/assets/icon.png","GET",[](VRequest* req, VResponse* resp){
            resp->writeBuffer(assets_icon_png, req, E_TAG);
        });
    }
};