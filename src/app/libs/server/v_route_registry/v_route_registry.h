#ifndef V_ROUTE_REGISTRY_H
#define V_ROUTE_REGISTRY_H

#include <Arduino.h>
#include "../v_array_list/v_array_list.h"
#include "../v_request/v_request.h"
#include "../v_response/v_response.h"
#include "../v_base_controller/v_base_controller.h"


class VRouteRegistry {
private:
    struct VRouteInfo {
        const char* url;
        const char* method;
        VBaseController* controller;
        void(*handler)(void*,VRequest*, VResponse*);
    };

    VArrayList<VRouteInfo*> *routes;
    VArrayList<VBaseController*> *controllers;

    static void handleUnauthorised(VResponse* resp) {
        resp->writeStatus(V_RESPONSE_FORBIDDEN);
    }
public:
    VRouteRegistry()
        : routes(new VArrayList<VRouteInfo*>()),
          controllers(new VArrayList<VBaseController*>()) {
    }

    void registerRoute(
        const char* url,
        const char* method,
        VBaseController* controller,
        void(*handler)(void*,VRequest*, VResponse*)
    ) {
        routes->add(new VRouteInfo{
            url,
            method,
            controller,
            handler
        });
    }

    void registerController(VBaseController *ctrl) {
        controllers->add(ctrl);
        ctrl->initRoutes();
    }
    boolean handleRequest(const String& url, const String &method, VRequest* req, VResponse* resp) const {
        Serial.println("handling request: " + url + " (" + method + ")");
        for (size_t i=0;i<routes->size();i++) {
            const VRouteInfo* route = routes->getAt(i);
            if (url==route->url && method==route->method) {
                if (!route->controller->authorise(req)) {
                    handleUnauthorised(resp);
                }
                else {
                    route->handler(
                        route->controller,
                        req,
                        resp
                    );
                }
                return true;
            }
        }
        return false;
    }
    void tick() {
        for (int i=0;i<controllers->size();i++) {
            controllers->getAt(i)->tick();
        }
    }
};

#endif