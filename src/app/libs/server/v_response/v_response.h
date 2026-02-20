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
const VResponseCode V_RESPONSE_NOT_FOUND {404, "Not Found"};

class VResponse {
private:
    WiFiClient *client;
    VHashTable<String> *headers;

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

    void writeResponse(const int code, const String &status, const uint8_t* buffer = nullptr, const int len = 0) {
        writeHeader(code, status);
        if (buffer) {
            this->client->write(buffer, len);
        }
    }

    void writeResponse(const int code, const String &status, const String &body) {
        writeHeader(code, status);
        this->client->print(body);
    }

    void setContentLength(const int val) {
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
    void writeJson(const VTableMultitype &resp) {
        const String body = resp.stringify();
        writeText("application/json", body);
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
        if (req->headers->has("if-none-match")) {
            const String eTag = req->headers->get("if-none-match");
            isTheSame = eTag==etagExpected;
        }
        if (isTheSame) {
            writeStatus(V_RESPONSE_NOT_MODIFIED);
        }
        else {
            this->headers->put("ETag",etagExpected);
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