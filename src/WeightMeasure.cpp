#include "WeightMeasure.hpp"

void WeightMeasure::setup(uint8_t sck_pin, uint8_t data_pin)
{
    _load_cell.begin(sck_pin, data_pin);
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
    if (abs(raw_weight - _prev_raw_weight) > 100)
    {
        _prev_raw_weight = raw_weight;
        return true;
    }
    return false;
}

float WeightMeasure::measure()
{
    return _value.avg() / _scale - _offset;
}

void WeightMeasure::set_scale(float scale)
{
    if (scale == 0)
        return;

    _scale = scale;
}

void WeightMeasure::set_weight(float weight)
{
    if (weight == 0)
        return;

    _scale = _curr_raw_weight / (weight + _offset);
}

void WeightMeasure::set_weight_offset(float weight_offset)
{
    _offset = weight_offset;
}

float WeightMeasure::toFloat(float raw)
{
    return raw;
}
