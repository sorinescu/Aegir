#include "TempHistory.hpp"
#include "TempMeasure.hpp"

#define HIST_OFFSET(offset) ((uint16_t(offset) + _curr_idx) % _history.capacity())

void TempHistory::collect()
{
    unsigned long t = millis();

    if (_size == 0)
    {
        _curr_idx = 0;
        _curr_millis = t;
        _size = 1;
        return;
    }

    uint16_t curr_offset = (t - _curr_millis) / _sampling_interval_millis;
    Serial.printf("before t=%ld curr_millis=%ld curr_idx=%hu curr_offset=%hu curr_size=%hu\n", t, _curr_millis, _curr_idx, curr_offset, _size);

    // Zero out missing intermediate values
    for (uint16_t i = 0; i + 1 < curr_offset; i++)
        _history.set(HIST_OFFSET(i), 0);

    _history.set(HIST_OFFSET(curr_offset), _temp->currentRawTemp(_precision));

    _curr_idx = HIST_OFFSET(curr_offset);
    _curr_millis += _sampling_interval_millis * curr_offset;

    _size += curr_offset;
    if (_size > _history.capacity())
        _size = _history.capacity();

    Serial.printf("after t=%ld curr_millis=%ld curr_idx=%hu curr_offset=%hu curr_size=%hu capacity=%hu\n", t, _curr_millis, _curr_idx, curr_offset, _size, _history.capacity());
}

float TempHistory::operator[](uint16_t idx)
{
    if (idx >= _size)
        return 0;

    uint16_t offset = HIST_OFFSET(idx - _size + uint16_t(1));
    Serial.printf("idx=%hu offset=%hu\n", idx, offset);
    return _temp->convertRawTemp(_history.get(offset), _precision);
}