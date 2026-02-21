
#include "../app/_stubs/stubs.h"
#include <Arduino.h>
#include "../app/service/root_service.h"

extern "C" void __attribute__((constructor)) preinit_marker() {
    esp_rom_printf("PREINIT\n");
}


RootService* rootService;

void setup() {
    rootService = &RootService::instance();
    rootService->setup();
}

void loop() {
    rootService->loop();
    yield();
}
