

#ifndef V_BROADCAST_SENDER_H
#define V_BROADCAST_SENDER_H
#include <Arduino.h>
#include <WiFiUdp.h>
#include <WiFi.h>


class VBroadcastSender {
private:
    int port;
    WiFiUDP udp;
public:

    explicit VBroadcastSender(const int port): port(port) {

    }

    void discover(const String &topic) {
        const IPAddress broadcast = WiFi.localIP() | ~WiFi.subnetMask();
        udp.beginPacket(broadcast, port);
        udp.print(topic.c_str());
        udp.endPacket();

        delay(500);

        const int packetSize = udp.parsePacket();
        if (packetSize) {
            char buf[256];
            const int len = udp.read(buf, sizeof(buf) - 1);
            if (len > 0) buf[len] = '\0';
            Serial.println(buf);
        }
    }

};

#endif