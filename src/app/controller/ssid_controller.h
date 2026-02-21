#pragma once

#include <Arduino.h>
#include <Preferences.h>
#include "../libs/server/v_base_controller/v_base_controller.h"
#include "../libs/server/v_server/v_server.h"
#include "../libs/server/v_request/v_request.h"
#include "../libs/server/v_response/v_response.h"
#include "../libs/server/v_table_multi_type/v_table_multi_type.h"
#include "../libs/server/v_auth/v_auth.h"

class SsidController: public VBaseController {
public:
    explicit SsidController(VServer *server) : VBaseController(server) {}


    void get(VRequest* req, VResponse* resp) {
        VTableMultitype result;
        Preferences preferences;
        preferences.begin("app", false);

        result.putString("ssid",preferences.getString("ssid",""));
        result.putString("password",preferences.getString("password",""));
        result.putString("id",preferences.getString("id",""));
        result.putString("spot",preferences.getString("spot",""));
        result.putString("ep",preferences.getString("ep","http://35.192.99.228:5000"));
        result.putInt("time",preferences.getLong("time",60L));
        preferences.end();
        resp->writeJson(result);
    }

    void save(VRequest* req, VResponse* resp) {
        Preferences preferences;
        preferences.begin("app", false);
        preferences.putString("ssid",req->params->getString("ssid").c_str());
        preferences.putString("password",req->params->getString("password").c_str());
        preferences.putString("id",req->params->getString("id").c_str());
        preferences.putString("ep",req->params->getString("ep").c_str());
        preferences.putString("spot",req->params->getString("spot").c_str());
        preferences.putLong("time",req->params->getInt("time"));
        preferences.end();

        VTableMultitype result;
        result.putBoolean("success",true);
        resp->writeJson(result);
    }

    void initRoutes() override {
        Route("/ssid/get","GET",SsidController,get);
        Route("/ssid/save","GET",SsidController,save);
    }

    boolean authorise(VRequest *request) override {
        return VAuth::checkToken(request);
    }

};