#pragma once

#include <Arduino.h>
#include <Preferences.h>
#include "../libs/server/v_base_controller/v_base_controller.h"
#include "../libs/server/v_server/v_server.h"
#include "../libs/server/v_request/v_request.h"
#include "../libs/server/v_response/v_response.h"
#include "../libs/server/v_json_lite/v_json_lite.h"
#include "../libs/server/v_auth/v_auth.h"

class SsidController: public VBaseController {
public:
    explicit SsidController(VServer *server) : VBaseController(server) {}


    void get(VRequest* req, VResponse* resp) {
        JsonObjectBuilder result;
        Preferences preferences;
        preferences.begin("app", false);

        result.setString("ssid",preferences.getString("ssid",""));
        result.setString("password",preferences.getString("password",""));
        result.setString("id",preferences.getString("id",""));
        result.setString("spot",preferences.getString("spot",""));
        result.setString("ep",preferences.getString("ep","http://35.192.99.228:5000"));
        result.setInt("time",preferences.getLong("time",60L));
        result.setInt("controlPin",preferences.getInt("controlPin",1));
        preferences.end();
        resp->writeJson(result.getRoot());
    }

    void save(VRequest* req, VResponse* resp) {
        Preferences preferences;
        preferences.begin("app", false);
        preferences.putString("ssid",req->body->get("ssid")->asString().c_str());
        preferences.putString("password",req->body->get("password")->asString().c_str());
        preferences.putString("id",req->body->get("id")->asString().c_str());
        preferences.putString("ep",req->body->get("ep")->asString().c_str());
        preferences.putString("spot",req->body->get("spot")->asString().c_str());
        preferences.putLong("time",req->body->get("time")->asInt());
        preferences.putInt("controlPin",req->body->get("controlPin")->asInt());
        preferences.end();

        JsonObjectBuilder result;
        result.setBool("success",true);
        resp->writeJson(result.getRoot());
    }

    void initRoutes() override {
        Route("/ssid/get","GET",SsidController,get);
        Route("/ssid/save","POST",SsidController,save);
    }

    boolean authorise(VRequest *request) override {
        return VAuth::checkToken(request);
    }

};