
#ifndef SVITLO_FIRMWARE_VERSION_H
#define SVITLO_FIRMWARE_VERSION_H
#include <Arduino.h>

#define FIRMWARE_VERSION "FW_VERSION=2026.1.27"

class FirmwareVersion {
private:

public:
    static String getFirmwareVersion() {
        const char* src = FIRMWARE_VERSION;
        const char* eq = strchr(src, '='); // шукаємо перший '='

        if (!eq || *(eq + 1) == '\0') {
            return "";
        }

        return String(eq + 1);
    }
};

#endif //SVITLO_FIRMWARE_VERSION_H