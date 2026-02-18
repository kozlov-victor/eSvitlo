#ifndef V_TIMER_H
#define V_TIMER_H

#include <Arduino.h>

class VTimer {
private:
    unsigned long lastTime = 0;
    unsigned long interval = 1000;
    bool started = false;
    bool firstTick = true;

public:
    void (*callback)(void) = nullptr;
    boolean once = false;

    void start(unsigned long ms) {
        if (ms<=0) return;
        interval = ms;
        started = true;
        firstTick = true;
    }

    void stop() {
        started = false;
    }

    void tick() {
        if (!started || !callback) return;

        unsigned long now = millis();

        if (firstTick) {
            firstTick = false;
            lastTime = now;
            if (!once) callback();
            return;
        }

        if (now - lastTime >= interval) {
            lastTime = now;
            callback();
            if (once) started = false;
        }
    }
};

#endif