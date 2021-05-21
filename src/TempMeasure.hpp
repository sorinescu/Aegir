#ifndef __TEMP_MEASURE_HPP__
#define __TEMP_MEASURE_HPP__

#include <stdint.h>
#include <OneWire.h>
#include "MovingAverage.hpp"

// Requires external power for the DS (i.e. not parasitic power mode)
class PositiveTempMeasure
{
    OneWire _ds;
    Uint16MovingAverage<5> _value;
    byte _addr[8];
    byte _type_s;
    bool _conversion_pending;

public:
    PositiveTempMeasure(uint8_t pin) : _ds(pin), _type_s(0xff), _conversion_pending(false) {}
    void setup();
    void loop();

    static float convertRawTemp(uint16_t raw, byte precision_bits=11);
    uint16_t currentRawTemp(byte precision_bits=11);
    float currentTemp();
};

#endif
