#ifndef __TEMP_HISTORY_HPP__
#define __TEMP_HISTORY_HPP__

#include <BitArray.h>

class PositiveTempMeasure;

class TempHistory
{
    PositiveTempMeasure *_temp;
    BitArray _history;
    uint16_t _precision;
    uint16_t _size;
    uint16_t _curr_idx;
    unsigned long _curr_millis;
    unsigned long _sampling_interval_millis;

    uint16_t idx_for_time(unsigned long t);

public:
    TempHistory(PositiveTempMeasure *temp, uint16_t precision, uint16_t capacity, unsigned long sampling_interval_millis) : _temp(temp), _precision(precision), _size(0), _curr_millis(0), _sampling_interval_millis(sampling_interval_millis) {
        _history.begin(precision, capacity);
    }

    void collect();
    
    unsigned long startTimeMillis() const { return _curr_millis; }
    uint16_t size() const { return _size; }
    float operator[](uint16_t idx);
};

#endif
