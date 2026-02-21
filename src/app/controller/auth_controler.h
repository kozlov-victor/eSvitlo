#pragma once

#include <Arduino.h>
#include "../libs/server/v_base_controller/v_base_controller.h"
#include "../libs/server/v_request/v_request.h"
#include "../libs/server/v_response/v_response.h"
#include "../libs/server/v_server/v_server.h"
#include "../libs/server/v_table_multi_type/v_table_multi_type.h"
#include "../libs/server/v_auth/v_auth.h"

class AuthController  : public VBaseController {
public:
    explicit AuthController(VServer* server) : VBaseController(server) {}

    void createToken(VRequest* req, VResponse* resp) {
        const String token = VAuth::createToken(req->params->getString("login"),req->params->getString("password"),60*2);
        if (token.isEmpty()) {
            resp->writeStatus(V_RESPONSE_UNAUTHORIZED);
            return;
        }
        VTableMultitype result;
        result.putString("token",token);
        resp->writeJson(result);
    }

    void initRoutes() override {
        Route("/auth/createToken","POST",AuthController,createToken)
    }
};
