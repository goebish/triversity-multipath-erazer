#include <Arduino.h>
#include <avr/eeprom.h>

#define EEPROM_MARKER 0xDEADBEEF

enum{
    MODE_MANUAL,
    MODE_AUTO
};

typedef struct{
    int8_t      current_channel; // 0 - 39
    uint8_t     select_mode;
    uint16_t    auto_threshold;
    uint32_t    marker;
}s_conf;

s_conf config;

void writeConfig() {
    eeprom_update_block((void*)&config, (void*)0, sizeof(config));
}

void resetConfig() {
    uint8_t i;
    config.current_channel = 0;
    config.select_mode = MODE_MANUAL;
    config.auto_threshold = 800;
    config.marker = EEPROM_MARKER;
    writeConfig();
}

void readConfig() {
    eeprom_read_block((void*)&config, (void*)0, sizeof(config));
    if(config.marker != EEPROM_MARKER) {
        resetConfig();
    }
}
