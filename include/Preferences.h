#ifndef CLION_PREFERENCES_H
#define CLION_PREFERENCES_H

#include <cstdint>
#include <cstddef>

class Preferences {
public:
    Preferences() = default;

    bool begin(const char* name, bool readOnly = false);
    void end();

    bool clear();
    bool remove(const char* key);

    // ---- Put ----
    size_t putChar(const char* key, int8_t value);
    size_t putUChar(const char* key, uint8_t value);
    size_t putShort(const char* key, int16_t value);
    size_t putUShort(const char* key, uint16_t value);
    size_t putInt(const char* key, int32_t value);
    size_t putUInt(const char* key, uint32_t value);
    size_t putLong(const char* key, int32_t value);
    size_t putULong(const char* key, uint32_t value);
    size_t putLong64(const char* key, int64_t value);
    size_t putULong64(const char* key, uint64_t value);
    size_t putFloat(const char* key, float value);
    size_t putDouble(const char* key, double value);
    size_t putBool(const char* key, bool value);
    size_t putString(const char* key, const char* value);

    size_t putBytes(const char* key, const void* value, size_t len);

    // ---- Get ----
    int8_t   getChar(const char* key, int8_t def = 0);
    uint8_t getUChar(const char* key, uint8_t def = 0);
    int16_t getShort(const char* key, int16_t def = 0);
    uint16_t getUShort(const char* key, uint16_t def = 0);
    int32_t getInt(const char* key, int32_t def = 0);
    uint32_t getUInt(const char* key, uint32_t def = 0);
    int32_t getLong(const char* key, int32_t def = 0);
    uint32_t getULong(const char* key, uint32_t def = 0);
    int64_t getLong64(const char* key, int64_t def = 0);
    uint64_t getULong64(const char* key, uint64_t def = 0);
    float getFloat(const char* key, float def = 0.0f);
    double getDouble(const char* key, double def = 0.0);
    bool getBool(const char* key, bool def = false);

    size_t getString(const char* key, char* buffer, size_t maxLen);

    String getString(const char* key, const String& def = "");

    size_t getBytes(const char* key, void* buffer, size_t maxLen);

    bool isKey(const char* key);
    size_t freeEntries();
};

#endif // CLION_PREFERENCES_H
