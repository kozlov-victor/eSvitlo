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

        void (*invoker)(
            VBaseController*,
            VRequest*,
            VResponse*
        );
    };


    template<typename T,void (T::*Method)(VRequest*, VResponse*)>
    static void invoke(
        VBaseController* base,
        VRequest* req,
        VResponse* resp
    ) {
        T* real = static_cast<T*>(base);
        (real->*Method)(req, resp);
    }

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


    template<typename T,void (T::*Method)(VRequest*, VResponse*)>
    void registerRoute(
        const char* url,
        const char* method,
        T* controller
    ) {
        routes->add(new VRouteInfo{
            url,
            method,
            controller,
            &invoke<T, Method>
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
                    route->invoker(
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