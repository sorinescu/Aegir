#ifndef __WEIGHT_MEASURE_HPP__
#define __WEIGHT_MEASURE_HPP__

#include <HX711.h>
#include "MovingAverage.hpp"
#include "Measurer.hpp"

class WeightMeasure : public Measurer<float>
{
    LongMovingAverage<5> _value;
    HX711 _load_cell;
    long _prev_raw_weight;
    long _curr_raw_weight;
    float _offset;
    float _scale;

public:
    WeightMeasure() : _prev_raw_weight(0), _curr_raw_weight(0), _offset(0), _scale(1) {}

    void setup(uint8_t sck_pin, uint8_t data_pin);
    void loop();

    bool has_new_value();

    float scale() { return _scale; }
    void set_scale(float scale);

    void set_weight(float weight);
    void set_weight_offset(float offset);

    virtual float measure();
    virtual float toFloat(float raw);
};

#endif
