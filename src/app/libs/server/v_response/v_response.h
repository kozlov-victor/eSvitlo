#ifndef V_RESPONSE_H
#define V_RESPONSE_H

#include <stdint.h>  // Додаємо цей заголовок для типів uint8_t, uint16_t тощо
#include <Arduino.h>
#include <WiFi.h>
#include "../static/static.h"
#include "../v_response/v_response.h"

class VResponse {
private:
    WiFiClient *client;

public:
    VResponse(WiFiClient *c) {
        this->client = c;
    }

    void writeText(String mimetype, String resp) {
        this->client->println("HTTP/1.1 200 Ok");
        this->client->println("Content-type: " + mimetype);
        this->client->println("Connection: close");
        this->client->println(); // The HTTP response starts with blank line
        this->client->println(resp);// The HTTP response ends with another blank line
    }
    void writeJson(VTableMultitype &resp) {
        this->client->println("HTTP/1.1 200 Ok");
        this->client->println("Content-type: application/json");
        this->client->println("Connection: close");
        this->client->println(); // The HTTP response starts with blank line
        this->client->println(resp.stringify()); // The HTTP response ends with another blank line
    }
    void writeBuffer(const V_FILE &file) {
        this->client->println("HTTP/1.1 200 Ok");
        this->client->println("Content-type: " + file.mime);
        this->client->println("Connection: close");
        this->client->println(); // The HTTP response starts with blank line
        this->client->write(file.buff, file.size);
        this->client->println(); // The HTTP response ends with another blank line
    }
    void writeBuffer(const V_FILE &file, VRequest* req, const String &etagExpected) {
        String eTag = req->headers->get("if-none-match");
        boolean isTheSame = eTag==etagExpected;
        if (isTheSame) {
            this->client->println("HTTP/1.1 304 Not Modified");
            this->client->println("Connection: close");
            this->client->println();
        }
        else {
            this->client->println("HTTP/1.1 200 Ok");
            this->client->println("Content-type: " + file.mime);
            this->client->println("ETag: " + etagExpected);
            this->client->println("Connection: close");
            this->client->println(); // The HTTP response starts with blank line
            this->client->write(file.buff, file.size);
            this->client->println(); // The HTTP response ends with another blank line
        }
    }
};

#endif