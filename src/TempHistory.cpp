#include "TempHistory.hpp"
#include "TempMeasure.hpp"

#define HIST_OFFSET(offset) ((uint16_t(offset) + _curr_idx) % _history.capacity())

bool TempHistory::collect()
{
    unsigned long t = millis();

    if (_size == 0)
    {
        _curr_idx = 0;
        _curr_millis = t;
        _size = 1;
        return false;
    }

    uint16_t offset = (t - _curr_millis) / _sampling_interval_millis;
    // Serial.printf("before t=%ld curr_millis=%ld curr_idx=%hu offset=%hu curr_size=%hu\n", t, _curr_millis, _curr_idx, offset, _size);

    // Zero out missing intermediate values
    for (uint16_t i = 0; i + 1 < offset; i++)
        _history.set(HIST_OFFSET(i), 0);

    _curr_idx = HIST_OFFSET(offset);
    _curr_millis += _sampling_interval_millis * offset;

    _history.set(_curr_idx, _temp->currentRawTemp(_precision));

    _size += offset;
    if (_size > _history.capacity())
        _size = _history.capacity();

    // Serial.printf("after curr_idx=%hu curr_size=%hu capacity=%hu\n", _curr_idx, _size, _history.capacity());
    return offset != 0;     // a new sample was collected
}

float TempHistory::at(uint16_t idx)
{
    if (idx >= _size)
        return 0;

    uint16_t offset = (idx + _curr_idx + 1) % _size;
    // Serial.printf("idx=%hu offset=%hu\n", idx, offset);
    return _temp->convertRawTemp(_history.get(offset), _precision);
}

unsigned long TempHistory::timeMillisAt(uint16_t idx)
{
    if (idx >= _size)
        return 0;

    return _curr_millis - (_size - 1 - idx) * _sampling_interval_millis;
}