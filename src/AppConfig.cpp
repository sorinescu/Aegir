#include <Arduino.h>
#include <EEPROM.h>
#include "AppConfig.hpp"

#define APP_CONFIG_SIZE 512 // must be multiple of 4
#define APP_CONFIG_CURR_VER 1

AppConfig::AppConfig(): _autocommit(true) {
    EEPROM.begin(APP_CONFIG_SIZE);

    setDefaults();

    uint8_t ver = EEPROM.read(0);
    bool read_ok = false;
    switch (ver) {
    case 1:
        read_ok = readV1();
        break;
    default:
        Serial.printf("Invalid stored app config version %d\n", ver);
    }

    if (!read_ok) {
        Serial.println("Flash config seems corrupted - resetting to defaults");
        reset();
    }
}

#define __read_with_chsum(v) \
    v = EEPROM.read(addr++); \
    cksum += v;

bool AppConfig::readV1() {
    uint8_t cksum = 0;
    int addr = 1;   // version is at 0, already read in constructor

    // TODO read params
    // uint8_t param1;
    // __read_with_chsum(param1);

    uint8_t actual_cksum = EEPROM.read(addr);
    return cksum == actual_cksum;
}

void AppConfig::setDefaults() {
    // TODO set default param values
}

void AppConfig::reset() {
    setDefaults();
    commit();
}

void AppConfig::commit() {
    uint8_t cksum = 0;

    int addr = 0;
    EEPROM.write(addr++, APP_CONFIG_CURR_VER);

#define __write_with_chsum(v) \
    cksum += v; \
    EEPROM.write(addr++, v);

    EEPROM.write(addr, cksum);
    EEPROM.commit();
}
