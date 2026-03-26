#ifndef V_REQUEST_H
#define V_REQUEST_H

#include <Arduino.h>
#include "../v_hash_table/v_hash_table.h"
#include "../v_json_lite/v_json_lite.h"

class VRequest {
public:
    String method;
    VHashTable<String> *headers;
    VHashTable<String> *params;
    JsonValue *body = nullptr;

    VRequest(const String &method, VHashTable<String> *headers, VHashTable<String> *params) {
        this->method = method;
        this->headers = headers;
        this->params = params;
    }

};

#endif