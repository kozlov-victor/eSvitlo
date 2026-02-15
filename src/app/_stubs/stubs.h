#ifndef ARDUINO_STUBS_H
#define ARDUINO_STUBS_H

#ifdef __JETBRAINS_IDE__

// --- ESP32-specific attribute stubs ---
#ifndef IRAM_ATTR
#undef IRAM_ATTR
#define IRAM_ATTR
#endif

#ifndef DRAM_ATTR
#define DRAM_ATTR
#endif

#ifndef RTC_IRAM_ATTR
#define RTC_IRAM_ATTR
#endif

#ifndef RTC_DATA_ATTR
#define RTC_DATA_ATTR
#endif

#ifndef PROGMEM
#define PROGMEM
#endif

#ifndef SOC_GPIO_PIN_COUNT
#define SOC_GPIO_PIN_COUNT 1
#endif


#include <cstring>  // для memset, memcpy, memmove, memcmp
#include <cstdint>
#include <cstddef>

// Arduino типи
typedef std::uint8_t  byte;
typedef std::uint8_t  uint8_t;
typedef std::uint16_t uint16_t;
typedef std::uint32_t uint32_t;
typedef std::uint32_t uint;
typedef std::uint64_t uint64_t;

typedef std::int8_t   int8_t;
typedef std::int16_t  int16_t;
typedef std::int32_t  int32_t;
typedef std::int64_t  int64_t;

// ======================= SPI =======================
class SPI_ {
public:
    void begin() {}
    void end() {}
    uint8_t transfer(uint8_t val) { return 0; }
    void beginTransaction(...) {}
    void endTransaction() {}
};
extern SPI_ SPI;

// ======================= EEPROM =======================
class EEPROM_ {
public:
    uint8_t read(int) { return 0; }
    void write(int, uint8_t) {}
    void commit() {}
};
extern EEPROM_ EEPROM;

// ======================= Serial =======================
class Serial {
public:
    void begin(int) {}
    void end() {}
    void println(const char*) {}
    void println(int) {}
    void println(uint32_t) {}
    void println(float) {}
    void print(const char*) {}
    void print(int) {}
    void print(uint32_t) {}
    void print(float) {}
    int available() { return 0; }
    int read() { return 0; }
};

// ======================= Pin functions =======================
inline void pinMode(uint8_t, int) {}
inline void digitalWrite(uint8_t, uint8_t) {}
inline void analogWrite(uint8_t, uint8_t) {}
inline void delay(int) {}
inline unsigned long millis() { return 0; }


// ======================= WiFi =======================
class WiFi_ {
public:
    void begin(const char*, const char*) {}
    void disconnect() {}
    int status() { return 0; }
};
extern WiFi_ WiFi;

class WiFiClient {
public:
    bool connect(const char*, uint16_t) { return true; }
    bool connect(const char*, uint16_t, int timeout) { return true; }
    size_t write(const uint8_t*, size_t) { return 1; }
    int read() { return 1; }
    int available() { return 1; }
    void stop() {}
    bool connected() { return true; }
};

// ======================= ESP32 specific placeholders =======================
inline void delayMicroseconds(int) {}

#endif // __JETBRAINS_IDE__
#endif // ARDUINO_STUBS_H
