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
Display display;
VServer vServer(80);

StaticController staticController;
SsidController ssidController;
PingInfoController pingInfoController;
boolean isAccessPoint = false;
VTimer vTimer;
Ping ping;
unsigned long tick = 0;

void log(String str) {
    display.clear();
    display.drawText(0,0,str);
    display.update();
}

void printIp(String note, IPAddress ip) {
    log(
        note + ":\n" +
        ip[0] + "." + ip[1] + ".\n" +
        ip[2] + "." + ip[3]
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

    display.begin(); // 72X40
    display.setOffset(36,25);
    log("Svitlo\nLoading...");
    delay(1000);

    display.clear();
    display.drawBitmap(0,0,63,32,logo);
    display.update();
    delay(3000);

    isAccessPoint = digitalRead(CONTROL_PIN)==LOW;
    PingInfoController::isAccessPoint = isAccessPoint;
    if (isAccessPoint) {
        IPAddress addr = vServer.setupAsAccessPoint();
        printIp("hot spot", addr);
    }
    else {
        Preferences preferences;
        preferences.begin("app", false);
        String ssid = preferences.getString("ssid","");
        String password = preferences.getString("password","");
        String ep   = preferences.getString("ep", "");
        String id   = preferences.getString("id", "");
        String spot = preferences.getString("spot", "");
        long time = preferences.getLong("time",60L);
        preferences.end();

        if (time<=0) {
            log("bad time!");
            while (true) {}
        }
        if (ssid=="") {
            log("no WIFI\nconfig!");
        }
        else {
            log("connecting to:\n " + ssid);
        }
        IPAddress addr = vServer.setupAsWifiClient(ssid,password);
        printIp("WIFI", addr);
        delay(4500);
        log("ping...");

        String url = ep + "/send?chat_id=" + VStrings::uriEncode(id);
        if (!spot.isEmpty()) {
            url += "&spot_id=" + VStrings::uriEncode(spot);
        }
        ping.setUrl(url);

        vTimer.callback = [](){
            const PingResponse result = ping.call();
            tick++;
            PingInfoController::tick = tick;

            display.clear();
            if (result.code==200) {
                display.drawText(
                    0,0,
                    String(tick) + "\n" + VStrings::replaceAll(result.message,' ',"\n")
                );
                PingInfoController::lastPingResponse = result.message;
            }
            else {
                display.drawText(0,0,"tick: " + String(tick) + "\n" + result.code + "\n" + result.message);
            }
            display.update();
        };
        delay(4500);
        vTimer.start(time*1000);

    }
    VRouteRegistry::registerController(staticController);
    VRouteRegistry::registerController(ssidController);
    VRouteRegistry::registerController(pingInfoController);
}


void loop() {
    vServer.loop();
    vServer.listenToNextClient();
    vTimer.tick();
    delay(1);
}
