#ifndef V_RESPONSE_H
#define V_RESPONSE_H

#include <Arduino.h>
#include <WiFi.h>
#include "../v_static/v_static.h"
#include "../v_hash_table/v_hash_table.h"
#include "../v_request/v_request.h"
#include "../v_json_lite/v_json_lite.h"

struct VResponseCode {
    int code;
    String hint;
};

const VResponseCode V_RESPONSE_OK {200, "Ok"};
const VResponseCode V_RESPONSE_NOT_MODIFIED {304, "Not Modified"};
const VResponseCode V_RESPONSE_FORBIDDEN {403, "Forbidden"};
const VResponseCode V_RESPONSE_BAD_REQUEST {400, "Bad Request"};
const VResponseCode V_RESPONSE_UNAUTHORIZED {401, "Unauthorized"};
const VResponseCode V_RESPONSE_NOT_FOUND {404, "Not Found"};

class VResponse {
private:
    WiFiClient *client;
    VHashTable<String> *headers;

    void _writeBuffer(const uint8_t* mem, size_t len) {
        const size_t CHUNK = 1024;          // 512..1460 теж ок
        uint8_t tmp[CHUNK];

        size_t sent = 0;
        while (sent < len && this->client->connected()) {
            size_t n = len - sent;
            if (n > CHUNK) n = CHUNK;

            memcpy_P(tmp, mem + sent, n); // читаємо з PROGMEM у RAM

            size_t w = this->client->write(tmp, n);       // скільки реально записалось
            if (w == 0) {                     // TCP буфер може бути зайнятий
                delay(1);
                yield();
                continue;
            }
            sent += w;
            yield();
        }

        if (sent != len) {
            Serial.printf("WARN: sent %u/%u bytes\n", (unsigned)sent, (unsigned)len);
        }
    }

    void writeHeader(int code, const String &status) {
        client->print("HTTP/1.1 ");
        client->print(code);
        client->print(" ");
        client->print(status);
        client->print("\r\n");

        headers->forEach([this](const String &key, const String &val) {
            client->print(key);
            client->print(": ");
            client->print(val);
            client->print("\r\n");
        });

        client->print("\r\n");
    }

    void writeResponse(const int code, const String &status, const uint8_t* buffer = nullptr, const unsigned int len = 0) {
        writeHeader(code, status);
        if (buffer && len) {
            _writeBuffer(buffer, len);
        }
    }

    void writeResponse(const int code, const String &status, const String &body) {
        writeHeader(code, status);
        this->client->print(body);
    }

    void setContentLength(const unsigned int val) {
        this->headers->put("Content-Length",String(val));
    }

public:
    explicit VResponse(WiFiClient *c, VHashTable<String> *headers) {
        this->client = c;
        this->headers = headers;
    }

    void setContentType(const String &value) {
        headers->put("Content-type",value);
    }

    void writeStatus(const VResponseCode &code) {
        writeResponse(code.code, code.hint);
    }

    void writeStatus(const VResponseCode &code, const String &body) {
        writeResponse(code.code, code.hint, body);
    }

    void writeText(const String &mimetype, const String &body) {
        setContentType(mimetype);
        setContentLength(body.length());
        writeResponse(V_RESPONSE_OK.code,V_RESPONSE_OK.hint,body);
    }
    void writeJson(JsonValue &resp) {
        const String body = resp.toString();
        writeText("application/json", body);
    }
    void writeBuffer(const uint8_t* buff, size_t size) {
        setContentType("application/octet-stream");
        setContentLength(size);
        writeResponse(V_RESPONSE_OK.code,V_RESPONSE_OK.hint,buff,size);
    }
    void writeBuffer(const V_FILE &file) {
        setContentType(file.mime);
        setContentLength(file.size);
        if (file.gzip) {
            this->headers->put("content-encoding","gzip");
        }
        writeResponse(V_RESPONSE_OK.code,V_RESPONSE_OK.hint,file.buff,file.size);
    }
    void writeBuffer(const V_FILE &file, const VRequest* req, const String &etagExpected) {
        boolean isTheSame = false;
        this->headers->put("ETag",etagExpected);
        if (req->headers->has("if-none-match")) {
            const String eTag = req->headers->get("if-none-match");
            isTheSame = eTag==etagExpected;
        }
        if (isTheSame) {
            setContentLength(0);
            writeStatus(V_RESPONSE_NOT_MODIFIED);
        }
        else {
            setContentType(file.mime);
            writeBuffer(file);
        }
    }
    void startSSE() {
        setContentType("text/event-stream");
        headers->put("Connection","keep-alive");
        headers->put("Cache-Control","no-cache");
        writeHeader(V_RESPONSE_OK.code,V_RESPONSE_OK.hint);
    }

    void sendSSE(const String &data) {
        client->print("data: ");
        client->print(data);
        client->print("\n\n");
    }

};

#endif