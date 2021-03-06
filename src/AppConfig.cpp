#include <Arduino.h>
#include <EEPROM.h>
#include "AppConfig.hpp"

#define APP_CONFIG_SIZE 512 // must be multiple of 4
#define APP_CONFIG_CURR_VER 1

// Global instance
AppConfig app_config;

AppConfig::AppConfig()
{
    setDefaults();
}

void AppConfig::begin()
{
    EEPROM.begin(APP_CONFIG_SIZE);

    uint8_t ver = EEPROM.read(0);
    bool read_ok = false;
    switch (ver)
    {
    case 1:
        read_ok = readV1();
        break;
    default:
        Serial.printf("Invalid stored app config version %d\n", ver);
    }

    if (!read_ok)
    {
        Serial.println("Flash config seems corrupted - resetting to defaults");
        reset();
    }
}

#define __read_with_chsum(v)                   \
    do                                         \
    {                                          \
        uint8_t *p = (uint8_t *)&v;            \
        for (size_t i = 0; i < sizeof(v); ++i) \
        {                                      \
            p[i] = EEPROM.read(addr++);        \
            cksum += p[i];                     \
        }                                      \
    } while (0);

bool AppConfig::readV1()
{
    uint8_t cksum = 0;
    int addr = 1; // version is at 0, already read in constructor

    __read_with_chsum(_weight_scale);

    uint8_t actual_cksum = EEPROM.read(addr);

    Serial.printf("App config: read %d bytes, cksum=%#x, expected=%#x\n", addr, actual_cksum, cksum);
    Serial.printf("App config: weight_scale=%f\n", _weight_scale);
    
    return cksum == actual_cksum;
}

void AppConfig::setDefaults()
{
    _weight_scale = 1;
}

void AppConfig::reset()
{
    setDefaults();
    commit();
}

void AppConfig::commit()
{
    uint8_t cksum = 0;

    int addr = 0;
    EEPROM.write(addr++, APP_CONFIG_CURR_VER);

#define __write_with_chsum(v)                  \
    do                                         \
    {                                          \
        uint8_t *p = (uint8_t *)&v;            \
        for (size_t i = 0; i < sizeof(v); ++i) \
        {                                      \
            EEPROM.write(addr++, p[i]);        \
            cksum += p[i];                     \
        }                                      \
    } while (0);

    __write_with_chsum(_weight_scale);

    EEPROM.write(addr, cksum);
    EEPROM.commit();

    Serial.printf("App config: wrote %d bytes, cksum=%#x\n", addr, cksum);
    Serial.printf("App config: weight_scale=%f\n", _weight_scale);
}
