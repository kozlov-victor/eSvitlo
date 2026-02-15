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
        String url;
        String method;
        void (*handler)(VRequest*,VResponse*);
    };
    static VArrayList<VRouteInfo>& routes() {  // header-only, старий трюк для ініціалізації статіс-поля в header
        static VArrayList<VRouteInfo> instance;
        return instance;
    }
public:
    static void registerRoute(const String url, String method, void (*handler)(VRequest*,VResponse*)) {
        routes().add({url, method, handler});
    }
    static void registerController(VBaseController &ctrl) {
        ctrl.initRoutes();
    }
    static boolean handleRequest(String url, String method, VRequest* req, VResponse* resp) {
        Serial.println("handling request: " + url + " (" + method + ")");
        for (size_t i=0;i<routes().size();i++) {
            VRouteInfo route = routes().getAt(i);
            if (url==route.url && method==route.method) {
                route.handler(req,resp);
                return true;
            }
        }
        return false;
    }
};

#endif