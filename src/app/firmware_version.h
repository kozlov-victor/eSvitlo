
#ifndef SVITLO_FIRMWARE_VERSION_H
#define SVITLO_FIRMWARE_VERSION_H
#include <Arduino.h>

#include "../app/libs/server/v_strings/v_strings.h"

#define FIRMWARE_VERSION "FW_VERSION=2026.1.16"

class FirmwareVersion {
private:

public:
    static String getFirmwareVersion() {
        const char* src = FIRMWARE_VERSION;
        const char* eq = strchr(src, '='); // шукаємо перший '='

        if (!eq || *(eq + 1) == '\0') {
            return ""; // нічого після '='
        }

        return String(eq + 1); // створюємо String тільки з частини після '='
    }
};

#endif //SVITLO_FIRMWARE_VERSION_H