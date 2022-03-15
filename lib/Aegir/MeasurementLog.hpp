#ifndef __MEASUREMENT_LOG_HPP__
#define __MEASUREMENT_LOG_HPP__

#include "Measurer.hpp"
#include "AppendableLogger.hpp"

class MeasurementLogOps
{
public:
    virtual size_t size() = 0;
    // virtual unsigned long startTimeMillis() const = 0;
    virtual unsigned long timeMillisAt(size_t idx) = 0;
    virtual float at(size_t idx) = 0;
};

template <typename ValueType>
class MeasurementLog : public MeasurementLogOps
{
    Measurer<ValueType> *_measurer;
    AppendableLogger<ValueType> *_logger;
    unsigned long _start_millis;
    unsigned long _last_sample_millis;
    unsigned long _sampling_interval_millis;

public:
    MeasurementLog(Measurer<ValueType> *measurer, AppendableLogger<ValueType> *logger, unsigned long sampling_interval_millis) : _measurer(measurer), _logger(logger), _start_millis(0), _last_sample_millis(0), _sampling_interval_millis(sampling_interval_millis)
    {
    }

    bool collect()
    {
        unsigned long now = millis();

        if (size() == 0)
        {
            _start_millis = now;
            _last_sample_millis = now;
            _logger->append(_measurer->measure());
            return true;
        }

        uint16_t sample_count = (now - _last_sample_millis) / _sampling_interval_millis;
        if (sample_count == 0)
            return false; // no new value recorded

        // Serial.printf("now=%ld _start_millis=%ld sample_count=%d logger->size=%d\n", now, _start_millis, (int)sample_count, _logger->size());

        ValueType val = _measurer->measure();
        for (uint16_t i = 0; i < sample_count; i++)
        {
            size_t old_size = _logger->size();
            _logger->append(val); // assume the missing values were the same as now

            // The logger has a fixed capacity and it's full, therefore the oldest value (collected at _start_millis) was overwritten.
            // Advance _start_millis to the new first entry.
            if (old_size == _logger->size())
                _start_millis += _sampling_interval_millis;
        }

        _last_sample_millis += _sampling_interval_millis * sample_count;
        
        return true; // recorded new value(s)
    }

    size_t size()
    {
        return _logger->size();
    }

    // unsigned long startTimeMillis() const
    // {
    //     return _start_millis;
    // }

    unsigned long timeMillisAt(size_t idx)
    {
        if (idx >= size())
            return 0;
        return _start_millis + idx * _sampling_interval_millis;
    }

    float at(size_t idx)
    {
        return _measurer->toFloat(_logger->at(idx));
    }
};

#endif // __MEASUREMENT_LOG_HPP__
