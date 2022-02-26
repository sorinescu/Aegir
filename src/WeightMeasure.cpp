#include "WeightMeasure.hpp"

void WeightMeasure::setup(uint8_t sck_pin, uint8_t data_pin)
{
    _load_cell.begin(data_pin, sck_pin);
    
    // Because the circuit haz a 15k/10k voltage divider on `data_pin`,
    // we can't use INPUT_PULLUP mode as it is set by `_load_cell.begin()`
    pinMode(data_pin, INPUT);

    _load_cell.power_down();
    delay(50);
    _load_cell.power_up();
}

void WeightMeasure::loop()
{
    if (!_load_cell.is_ready())
        return;

    _curr_raw_weight = _load_cell.read();
    _value.add(_curr_raw_weight);

    // Tare on init
    if (_offset == 0)
        _offset = _curr_raw_weight / _scale;
}

bool WeightMeasure::has_new_value()
{
    long raw_weight = _value.avg();
    if (_force_new_value || abs(raw_weight - _prev_raw_weight) > 100)
    {
        _prev_raw_weight = raw_weight;
        _force_new_value = false;
        return true;
    }
    return false;
}

float WeightMeasure::measure()
{
    float value = _value.avg() / _scale - _offset;
    if (value < 0)
        return 0;
    return value;
}

void WeightMeasure::set_scale(float scale)
{
    if (scale == 0)
        return;

    _scale = scale;
}

void WeightMeasure::set_weight(float weight)
{
    if (weight + _offset == 0)
        return;

    _scale = _curr_raw_weight / (weight + _offset);
    _force_new_value = true;
}

void WeightMeasure::set_weight_offset(float weight_offset)
{
    _offset = weight_offset;
    _force_new_value = true;
}

float WeightMeasure::toFloat(float raw)
{
    return raw;
}
