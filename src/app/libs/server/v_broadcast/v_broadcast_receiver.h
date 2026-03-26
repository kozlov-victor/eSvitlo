
#include <Arduino.h>
#include <WiFiUdp.h>
#include <WiFi.h>

#ifndef V_BROADCAST_RECEIVER_H
#define V_BROADCAST_RECEIVER_H



class VBroadcastReceiver {

private:
    int port;
    WiFiUDP udp;
public:

    explicit VBroadcastReceiver(const int port):port(port) {
        udp.begin(port);
    }

    void update() {
        int packetSize = udp.parsePacket();
        if (packetSize) {
            char buf[256]; // достатньо великий
            const int len = udp.read(buf, sizeof(buf) - 1); // читаємо і залишаємо місце для '\0'
            if (len > 0) buf[len] = '\0'; // термінуємо рядок правильно
            Serial.println(buf);
            if (String(buf) == "DISCOVER_SENSOR") {
                Serial.println("Will respond!");
                IPAddress ip = WiFi.localIP();
                udp.beginPacket(udp.remoteIP(), udp.remotePort());
                udp.print("SENSOR:" + String(ip[0]) + "." + String(ip[1]) + "."+String(ip[2]) + "." + String(ip[3]));
                udp.endPacket();
            }
        }
    }

};

#endif