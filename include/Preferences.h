#ifndef CLION_PREFERENCES_H
#define CLION_PREFERENCES_H

#include <cstdint>
#include <cstddef>

class Preferences {
public:
    Preferences() = default;

    bool begin(const char* name, bool readOnly = false) { return true; }
    void end() {}

    bool clear() { return true; }
    bool remove(const char* key) { return true; }

    // ---- Put ----
    size_t putChar(const char* key, int8_t value) { return 1; }
    size_t putUChar(const char* key, uint8_t value) { return 1; }
    size_t putShort(const char* key, int16_t value) { return 2; }
    size_t putUShort(const char* key, uint16_t value) { return 2; }
    size_t putInt(const char* key, int32_t value) { return 4; }
    size_t putUInt(const char* key, uint32_t value) { return 4; }
    size_t putLong(const char* key, int32_t value) { return 4; }
    size_t putULong(const char* key, uint32_t value) { return 4; }
    size_t putLong64(const char* key, int64_t value) { return 8; }
    size_t putULong64(const char* key, uint64_t value) { return 8; }
    size_t putFloat(const char* key, float value) { return 4; }
    size_t putDouble(const char* key, double value) { return 8; }
    size_t putBool(const char* key, bool value) { return 1; }

    size_t putString(const char* key, const char* value) {
        return value ? strlen(value) : 0;
    }

    size_t putBytes(const char* key, const void* value, size_t len) {
        return len;
    }

    // ---- Get ----
    int8_t   getChar(const char* key, int8_t def = 0) { return def; }
    uint8_t getUChar(const char* key, uint8_t def = 0) { return def; }
    int16_t getShort(const char* key, int16_t def = 0) { return def; }
    uint16_t getUShort(const char* key, uint16_t def = 0) { return def; }
    int32_t getInt(const char* key, int32_t def = 0) { return def; }
    uint32_t getUInt(const char* key, uint32_t def = 0) { return def; }
    int32_t getLong(const char* key, int32_t def = 0) { return def; }
    uint32_t getULong(const char* key, uint32_t def = 0) { return def; }
    int64_t getLong64(const char* key, int64_t def = 0) { return def; }
    uint64_t getULong64(const char* key, uint64_t def = 0) { return def; }
    float getFloat(const char* key, float def = 0.0f) { return def; }
    double getDouble(const char* key, double def = 0.0) { return def; }
    bool getBool(const char* key, bool def = false) { return def; }

    size_t getString(const char* key, char* buffer, size_t maxLen) {
        if (buffer && maxLen) buffer[0] = '\0';
        return 0;
    }

    String getString(const char* key, const String& def = "") {
        return def;
    }

    size_t getBytes(const char* key, void* buffer, size_t maxLen) {
        return 0;
    }

    bool isKey(const char* key) { return false; }
    size_t freeEntries() { return 0; }
};

#endif // CLION_PREFERENCES_H
