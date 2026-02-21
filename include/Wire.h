#ifndef CLION_WIRE_H
#define CLION_WIRE_H
#include <cstdint>

class Wire_ {
public:
    void begin();
    void begin(int,int);
    void beginTransmission(int);
    void write(...);
    uint8_t endTransmission();
    uint8_t endTransmission(bool);
    uint8_t requestFrom(int, int);
    int available();
    int read();
    void setClock(int);
};

extern Wire_ Wire;


#endif //CLION_WIRE_H