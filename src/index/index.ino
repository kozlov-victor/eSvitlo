#include <Wire.h>
#include "../app/_stubs/stubs.h"
#include <Arduino.h>
#include <Preferences.h>
#include "../app/libs/display/display.h"
#include "../app/ping.h"
#include "../app/controller/ssid_controller.h"
#include "../app/controller/static_controller.h"
#include "../app/libs/server/v_server/v_server.h"
#include "../app/libs/v_timer/v_timer.h"
#include "../app/controller/ping_info_controller.h"
#include "../app/controller/ota_controller.h"
#include "../app/controller/auth_controler.h"
#include "../app/controller/personal_account_controller.h"

StaticController *staticController;
SsidController *ssidController;
PingInfoController *pingInfoController;
OtaController *otaController;
AuthController *authController;
PersonalAccountController *personalAccountController;

extern "C" void __attribute__((constructor)) preinit_marker() {
    esp_rom_printf("PREINIT\n");
}

static const uint8_t logo[] PROGMEM = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00,
    0x00, 0x3E, 0x00, 0x00, 0x10, 0x10, 0x03, 0xF0, 0x00, 0x22, 0x00, 0x04, 0x10, 0x18, 0x06, 0x10, 0x00, 0x20, 0x00, 0x00,
    0x08, 0x08, 0x0C, 0x18, 0x00, 0x20, 0x40, 0x00, 0x08, 0x08, 0x08, 0x08, 0x00, 0x20, 0x40, 0x64, 0x08, 0x08, 0x08, 0x08,
    0x00, 0x30, 0x20, 0x44, 0x7F, 0x08, 0x08, 0x08, 0x1F, 0x18, 0x20, 0x44, 0x88, 0x08, 0x08, 0x08, 0x31, 0x0E, 0x10, 0x84,
    0x08, 0x08, 0x08, 0x08, 0x21, 0x02, 0x10, 0x84, 0x08, 0x08, 0x08, 0x18, 0x3F, 0x03, 0x08, 0x84, 0x08, 0x08, 0x0C, 0x10,
    0x20, 0x01, 0x0C, 0x84, 0x08, 0x08, 0x04, 0x10, 0x20, 0x01, 0x05, 0x04, 0x08, 0x08, 0x04, 0x10, 0x21, 0x03, 0x07, 0x04,
    0x08, 0x08, 0x06, 0x30, 0x37, 0x66, 0x03, 0x00, 0x0F, 0xEF, 0xF3, 0xE0, 0x1C, 0x3C, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x19, 0xF8, 0x01, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x39,
    0x84, 0x01, 0x7F, 0x80, 0x00, 0x7E, 0xF0, 0x27, 0x04, 0x01, 0x18, 0x80, 0x00, 0x43, 0x88, 0x30, 0x0C, 0x01, 0x80, 0x80,
    0x00, 0x43, 0x18, 0x10, 0x18, 0x00, 0xC0, 0x80, 0x00, 0x40, 0x10, 0x18, 0x10, 0x00, 0x61, 0x80, 0x00, 0x60, 0x30, 0x06,
    0x30, 0x00, 0x33, 0x00, 0x00, 0x30, 0x60, 0x03, 0xA0, 0x00, 0x1E, 0x00, 0x00, 0x18, 0x40, 0x00, 0xE0, 0x00, 0x04, 0x00,
    0x00, 0x0E, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00
};


const int CONTROL_PIN = 1;
VServer *vServer;

boolean isAccessPoint = false;
VTimer *vTimer;
Ping *ping;
Display *display;
unsigned long tick = 0;

void log(const String &str) {
    display->clear();
    display->drawText(0,0,str);
    display->update();
}

void printIp(const String &note, const IPAddress &ip) {
    log(
        note + ":\n" +
        String(ip[0]) + "." + String(ip[1]) + ".\n" +
        String(ip[2]) + "." + String(ip[3])
    );
}

void setup() {
    Serial.begin(115200);
    delay(300);
    Serial.println("BOOT");
    Wire.begin(5, 6);
    Wire.setClock(100000);
    delay(200);
    pinMode(CONTROL_PIN,INPUT_PULLUP);
    delay(200);
    display = new Display();
    display->begin(); // 72X40
    display->setOffset(36,25);

    log("Svitlo\nLoading...");


    vTimer = new VTimer();
    ping = new Ping();
    vServer = new VServer(80);
    staticController = new StaticController(vServer);
    ssidController = new SsidController(vServer);
    pingInfoController = new PingInfoController(vServer);
    otaController = new OtaController(vServer);
    authController = new AuthController(vServer);
    personalAccountController = new PersonalAccountController(vServer,pingInfoController);
    delay(400);

    display->clear();
    display->drawBitmap(0,0,63,32,logo);
    display->update();
    delay(3000);

    log(FirmwareVersion::getFirmwareVersion());
    delay(1000);

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

    isAccessPoint = digitalRead(CONTROL_PIN)==LOW;
    pingInfoController->isAccessPoint = isAccessPoint;
    if (isAccessPoint) {
        const IPAddress addr = vServer->setupAsAccessPoint();
        printIp("hot spot", addr);
    }
    else {
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
            log("bad\nping time!");
            while (true) {
                yield();
            }
        }
        if (ssid.isEmpty()) {
            log("no WIFI\nconfig!");
            while (true) {
                yield();
            }
        }
        log(String("connecting to:\n ") + ssid);

        const IPAddress addr = vServer->setupAsWifiClient(ssid,password);
         printIp("WIFI", addr);
         delay(4500);
         log("ping...");

         String url = ep + "/send?chat_id=" + VStrings::uriEncode(id);
         if (!spot.isEmpty()) {
              url += "&spot_id=" + VStrings::uriEncode(spot);
         }
        ping->setUrl(url);

        vTimer->onDone(nullptr,[](void*) {
            const PingResponse result = ping->call();
            tick++;
            pingInfoController->tickCnt = tick;

            display->clear();
            if (result.code==200) {
                display->drawText(
                    0,0,
                    VStrings::padCenter(String(tick),11) +
                    "\n" +
                    VStrings::replaceAll(result.message,' ',"\n")
                );
                pingInfoController->lastPingResponse = result.message;
            }
            else {
                const String err = String(result.code) + "\n" + result.message;
                display->drawText(0,0,
                    VStrings::padCenter(String(tick),11) + "\n" + err);
                pingInfoController->lastPingResponse = err;
            }
            display->update();
        });
        delay(4500);
        vTimer->start(time*1000);
    }
    vServer->getRegistry()->registerController(staticController);
    vServer->getRegistry()->registerController(ssidController);
    vServer->getRegistry()->registerController(pingInfoController);
    vServer->getRegistry()->registerController(otaController);
    vServer->getRegistry()->registerController(authController);
    vServer->getRegistry()->registerController(personalAccountController);
}

void loop() {
    vServer->tick();
    vTimer->tick();
    yield();
}
