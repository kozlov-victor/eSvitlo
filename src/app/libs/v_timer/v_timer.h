#ifndef V_TIMER_H
#define V_TIMER_H

#include <Arduino.h>

class VTimer {
private:
    unsigned long lastTime = 0;
    unsigned long interval = 1000;
    bool started = false;
    bool firstTick = true;
    void* context = nullptr;
    void (*callback)(void*) = nullptr;

public:
    boolean once = false;

    void onDone(void* context, void (*callback)(void*)) {
        this->context = context;
        this->callback = callback;
    }

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
            if (!once) {
                if (callback) {
                    callback(context);
                }
            }
            return;
        }

        if (now - lastTime >= interval) {
            lastTime = now;
            if (callback) {
                callback(context);
            }
            if (once) started = false;
        }
    }
};

#endif