#pragma once

#include <Arduino.h>
#include "Preferences.h"
#include "../libs/server/v_base_controller/v_base_controller.h"
#include "../libs/server/v_server/v_server.h"
#include "../libs/server/v_request/v_request.h"
#include "../libs/server/v_response/v_response.h"
#include "../libs/server/v_auth/v_auth.h"

class PersonalAccountController  : public VBaseController {
private:

public:
    explicit PersonalAccountController(VServer *server) : VBaseController(server) {
    }

    void creds(VRequest* req, VResponse* resp) {
        Preferences preferences;
        preferences.begin("app", false);
        String login = req->params->getString("login");
        String password = req->params->getString("password");
        if (login.isEmpty() || password.isEmpty()) {
            resp->writeStatus(V_RESPONSE_BAD_REQUEST);
            return;
        }
        preferences.putString("admin-login",login.c_str());
        preferences.putString("admin-password",password.c_str());
        preferences.end();
        VAuth::setCreds(login, password);
        resp->writeStatus(V_RESPONSE_OK);
    }

    void reset(VRequest* req, VResponse* resp) {
        if (req->params->getString("password") != "_^tavyzaibalyzabuvatyparoli$!") {
            resp->writeStatus(V_RESPONSE_UNAUTHORIZED);
            return;
        }
        VAuth::reset();
        resp->writeStatus(V_RESPONSE_OK);
    }

    void initRoutes() override {
        server->getRegistry()->registerRoute<PersonalAccountController,&PersonalAccountController::creds>(
            "/personal-account/creds","POST",this
        );
        server->getRegistry()->registerRoute<PersonalAccountController,&PersonalAccountController::reset>(
            "/personal-account/reset","POST",this
        );
    }

    boolean authorise(VRequest *) override {
        return AppService::instance().isAccessPoint;
    }
};
