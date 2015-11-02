#include <Arduino.h>
#include <avr/eeprom.h>

#define VBAT_ALARM_MIN  60
#define VBAT_ALARM_MAX  200
#define HYST_MAX 990
#define EEPROM_MARKER 0xDEADBEEF

enum e_switch_beep{
    BEEP_OFF,
    BEEP_QUIET,
    BEEP_LOUD,
    BEEP_LOUDER,
};

typedef struct{
    int8_t      current_channel; // 0 - 39
    uint16_t    auto_threshold;
    uint8_t     vbat_alarm;
    uint8_t     beep_volume;
    uint16_t    switch_period;
    uint32_t    marker;
}s_conf;

static s_conf config;

void writeConfig() {
    eeprom_update_block((void*)&config, (void*)0, sizeof(config));
}

void resetConfig() {
    uint8_t i;
    config.current_channel = 0;
    config.auto_threshold = 800;
    config.vbat_alarm = 109;
    config.beep_volume = BEEP_QUIET;
    config.switch_period = 0;
    config.marker = EEPROM_MARKER;
    writeConfig();
}

void readConfig() {
    eeprom_read_block((void*)&config, (void*)0, sizeof(config));
    if(config.marker != EEPROM_MARKER) {
        resetConfig();
    }
}
