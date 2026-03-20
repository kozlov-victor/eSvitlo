#ifndef V_BASE64_H
#define V_BASE64_H
#include <Arduino.h>
#include <base64.h>

static constexpr int8_t _B64_TABLE[256] = {
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,

    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,62,-1,-1,-1,63,
    52,53,54,55,56,57,58,59,60,61,-1,-1,-1,-2,-1,-1,

    -1,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,
    15,16,17,18,19,20,21,22,23,24,25,-1,-1,-1,-1,-1,

    -1,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,
    41,42,43,44,45,46,47,48,49,50,51,-1,-1,-1,-1,-1
};

class VBase64 {

public:

    static String encode(const String& input) {
        return base64::encode(input);
    }

    static String decode(const String& input) {

        const size_t len = input.length();
        if (len == 0) return "";

        const size_t outMax = (len * 3) / 4 + 3;

        uint8_t out[outMax];
        size_t outPos = 0;

        const char* data = input.c_str();

        for (size_t i = 0; i < len; i += 4) {
            if (i+3>input.length()-1) break;

            int8_t a = _B64_TABLE[(uint8_t)data[i]];
            int8_t b = _B64_TABLE[(uint8_t)data[i+1]];
            int8_t c = _B64_TABLE[(uint8_t)data[i+2]];
            int8_t d = _B64_TABLE[(uint8_t)data[i+3]];

            out[outPos++] = (a << 2) | (b >> 4);

            if (data[i+2] != '=') {
                out[outPos++] = (b << 4) | (c >> 2);
            }

            if (data[i+3] != '=') {
                out[outPos++] = (c << 6) | d;
            }
        }

        return String((char*)out, outPos);
    }
};

#endif //V_BASE64_H