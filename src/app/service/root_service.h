#ifndef SVITLO_ROOT_SERVICE_H
#define SVITLO_ROOT_SERVICE_H
#include <Arduino.h>
#include <Preferences.h>
#include <Wire.h>
#include "../libs/server/v_service/v_service.h"
#include "../libs/server/v_server/v_server.h"
#include "../libs/v_timer/v_timer.h"
#include "../controller/ssid_controller.h"
#include "../controller/static_controller.h"
#include "../controller/ping_info_controller.h"
#include "../controller/ota_controller.h"
#include "../controller/auth_controler.h"
#include "../controller/personal_account_controller.h"
#include "./app_service.h"
#include "./ping_service.h"

class RootService {
Service(RootService)
private:
    AppService* appService = nullptr;
    PingService* pingService = nullptr;
    VTimer *vTimer;
    VServer *vServer;
    const int CONTROL_PIN = 1;
    boolean isAccessPoint = false;

    void printIp(const String &note, const IPAddress &ip) const {
        appService->log(
            note + ":\n" +
            String(ip[0]) + "." + String(ip[1]) + ".\n" +
            String(ip[2]) + "." + String(ip[3])
        );
    }

    void initControllers() {
        vServer->getRegistry()->registerController(new StaticController(vServer));
        vServer->getRegistry()->registerController(new SsidController(vServer));
        vServer->getRegistry()->registerController(new PingInfoController(vServer));
        vServer->getRegistry()->registerController(new OtaController(vServer));
        vServer->getRegistry()->registerController(new AuthController(vServer));
        vServer->getRegistry()->registerController(new PersonalAccountController(vServer));
    }

    void initAuth() {
        Preferences preferences;
        preferences.begin("app", false);
        const String adminLogin     = preferences.getString("admin-login", "");
        const String adminPassword  = preferences.getString("admin-password", "");
        preferences.end();
        VAuth::setDefaultCreds("eSvitlo","pass123");
        if (!adminLogin.isEmpty() && !adminPassword.isEmpty()) {
            VAuth::setCreds(adminLogin, adminPassword);
        }
        else {
            VAuth::reset();
        }
    }

    void initPing(const String &ep, const String &id, const String &spot, const long &time) {
        appService->log("ping...");

        String url = ep + "/send?chat_id=" + VStrings::uriEncode(id);
        if (!spot.isEmpty()) {
            url += "&spot_id=" + VStrings::uriEncode(spot);
        }
        pingService->setUrl(url);
        vTimer->onDone(this,[](void* ctx) {
            auto* self = static_cast<RootService*>(ctx);
            const PingResponse result = self->pingService->call();
            AppService::instance().tickCnt++;
            const auto tick = AppService::instance().tickCnt;
            if (result.code==200) {
                self->appService->log(
                    VStrings::padCenter(String(tick),10) +
                    "\n" +
                    VStrings::replaceAll(result.message,' ',"\n")
                );
                AppService::instance().lastPingResponse = result.message;
            }
            else {
                const String err = String(result.code) + "\n" + result.message;
                self->appService->log(VStrings::padCenter(String(tick),10) + "\n" + err);
                self->appService->lastPingResponse = err;
            }
        });
        delay(4500);
        vTimer->start(time*1000);
    }

    void initWiFiMode() {
        Preferences preferences;
        preferences.begin("app", false);
        const String ssid           = preferences.getString("ssid","");
        const String password       = preferences.getString("password","");
        const String ep             = preferences.getString("ep", "");
        const String id             = preferences.getString("id", "");
        const String spot           = preferences.getString("spot", "");
        const long   time           = preferences.getLong("time",60L);
        preferences.end();

        if (time<=0) {
            appService->log("bad\nping time!");
            while (true) {
                yield();
            }
        }
        if (ssid.isEmpty()) {
            appService->log("no WIFI\nconfig!");
            while (true) {
                yield();
            }
        }
        appService->log(String("connecting to:\n ") + ssid);

        const IPAddress addr = vServer->setupAsWifiClient(ssid,password);
        printIp("WIFI", addr);
        delay(4500);
        initPing(ep,id,spot,time);
    }

public:
    explicit RootService() {
        vTimer = new VTimer();
        vServer = new VServer(80);
    }

    void setup() {
        Serial.begin(115200);
        delay(300);
        Serial.println("BOOT");
        Wire.begin(5, 6);
        Wire.setClock(100000);
        delay(200);
        pingService = &PingService::instance();
        appService = &AppService::instance();

        pinMode(CONTROL_PIN,INPUT_PULLUP);
        appService->setup();
        delay(200);

        appService->log("Svitlo\nLoading...");
        delay(400);

        appService->drawLogo();
        delay(3000);

        appService->log(FirmwareVersion::getFirmwareVersion());
        delay(1000);

        initAuth();

        isAccessPoint = digitalRead(CONTROL_PIN)==LOW;
        appService->isAccessPoint = isAccessPoint;
        if (isAccessPoint) {
            const IPAddress addr = vServer->setupAsAccessPoint();
            printIp("hot spot", addr);
        }
        else {
            initWiFiMode();
        }
        initControllers();
    }

    void loop() {
        vServer->tick();
        vTimer->tick();
        yield();
    }

};

#endif //SVITLO_ROOT_SERVICE_H