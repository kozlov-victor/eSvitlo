#ifndef V_RESPONSE_H
#define V_RESPONSE_H

#include <stdint.h>  // Додаємо цей заголовок для типів uint8_t, uint16_t тощо
#include <Arduino.h>
#include <WiFi.h>
#include "../v_static/v_static.h"
#include "../v_response/v_response.h"

struct VResponseCode {
    int code;
    String hint;
};

const VResponseCode V_RESPONSE_OK {200, "Ok"};
const VResponseCode V_RESPONSE_NOT_MODIFIED {304, "Not Modified"};
const VResponseCode V_RESPONSE_FORBIDDEN {403, "Forbidden"};
const VResponseCode V_RESPONSE_BAD_REQUEST {400, "Bad Request"};
const VResponseCode V_RESPONSE_UNAUTHORIZED {401, "Unauthorized"};

class VResponse {
private:
    WiFiClient *client;

public:
    explicit VResponse(WiFiClient *c) {
        this->client = c;
    }

    void writeStatus(const VResponseCode &code) {
        this->client->println("HTTP/1.1 " + String(code.code) + " " + code.hint);
        this->client->println("Connection: close");
        this->client->println();
    }

    void writeText(const String &mimetype, String &resp) {
        const int len = resp.length();
        this->client->println("HTTP/1.1 200 Ok");
        this->client->println("Content-type: " + mimetype);
        this->client->println("Content-Length: " + String(len));
        this->client->println(); // The HTTP response starts with blank line
        this->client->println(resp);// The HTTP response ends with another blank line
    }
    void writeJson(const VTableMultitype &resp) {
        const String body = resp.stringify();
        const int len = body.length();
        this->client->println("HTTP/1.1 200 Ok");
        this->client->println("Content-type: application/json");
        this->client->println("Content-Length: " + String(len));
        this->client->println("Connection: close");
        this->client->println(); // The HTTP response starts with blank line
        this->client->println(body); // The HTTP response ends with another blank line
    }
    void writeBuffer(const V_FILE &file) {
        this->client->println("HTTP/1.1 200 Ok");
        this->client->println("Content-type: " + file.mime);
        this->client->println("Content-Length: " + String(file.size));
        this->client->println("Connection: close");
        this->client->println(); // The HTTP response starts with blank line
        this->client->write(file.buff, file.size);
        this->client->println(); // The HTTP response ends with another blank line
    }
    void writeBuffer(const V_FILE &file, const VRequest* req, const String &etagExpected) {
        boolean isTheSame = false;
        if (req->headers->has("if-none-match")) {
            const String eTag = req->headers->get("if-none-match");
            isTheSame = eTag==etagExpected;
        }
        if (isTheSame) {
            writeStatus(V_RESPONSE_NOT_MODIFIED);
        }
        else {
            this->client->println("HTTP/1.1 200 Ok");
            this->client->println("Content-type: " + file.mime);
            this->client->println("Content-Length: " + String(file.size));
            this->client->println("ETag: " + etagExpected);
            this->client->println("Connection: close");
            this->client->println(); // The HTTP response starts with blank line
            this->client->write(file.buff, file.size);
            this->client->println(); // The HTTP response ends with another blank line
        }
    }
};

#endif