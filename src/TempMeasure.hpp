#ifndef __TEMP_MEASURE_HPP__
#define __TEMP_MEASURE_HPP__

#include <stdint.h>
#include <OneWire.h>
#include "MovingAverage.hpp"
#include "Measurer.hpp"

// Requires external power for the DS (i.e. not parasitic power mode)
class PositiveTempMeasure: public Measurer<uint16_t>
{
    OneWire _ds;
    Uint16MovingAverage<5> _value;
    byte _addr[8];
    byte _type_s;
    bool _conversion_pending;
    uint16_t _prev_value;

public:
    PositiveTempMeasure(uint8_t pin) : _ds(pin), _type_s(0xff), _conversion_pending(false), _prev_value(0) {}
    void setup();
    void loop();

    bool has_new_value();

    virtual uint16_t measure();
    virtual float toFloat(uint16_t raw);
};

#endif
