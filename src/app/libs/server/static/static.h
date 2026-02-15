#ifndef V_STATIC_H
#define V_STATIC_H

#include <Arduino.h>
#include <stdint.h>  // Додаємо цей заголовок для типів uint8_t, uint16_t тощо

struct V_FILE
{
    String mime;
    const uint8_t* buff;
    size_t size;
};

#endif
V_STATIC_H
    
