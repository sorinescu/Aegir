#ifndef __TEMP_HISTORY_HPP__
#define __TEMP_HISTORY_HPP__

#include "FileValueLogger.hpp"

class PositiveTempMeasure;

class TempHistory
{
    PositiveTempMeasure *_temp;
    FileValueLogger<uint16_t> _logger;
    unsigned long _curr_millis;
    unsigned long _sampling_interval_millis;

public:
    TempHistory(PositiveTempMeasure *temp, unsigned long sampling_interval_millis) : _temp(temp), _curr_millis(0), _sampling_interval_millis(sampling_interval_millis)
    {
    }

    void begin(const char *log_path) {
        _logger.open(log_path);
    }

    bool collect();

    unsigned long startTimeMillis() const { return _curr_millis; }
    size_t size() const { return _logger.size(); }

    unsigned long timeMillisAt(size_t idx);
    float at(size_t idx);
};

#endif
