#ifndef V_REQUEST_H
#define V_REQUEST_H

#include <Arduino.h>
#include "../v_hash_table/v_hash_table.h"
#include "../v_table_multi_type/v_table_multi_type.h"

class VRequest {
public:
    String method;
    VHashTable<String> *headers;
    VTableMultitype *params;

    VRequest(const String &method, VHashTable<String> *headers, VTableMultitype *params) {
        this->method = method;
        this->headers = headers;
        this->params = params;
    }

};

#endif