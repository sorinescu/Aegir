#ifndef __MEASUREMENT_LOG_HPP__
#define __MEASUREMENT_LOG_HPP__

#include "Measurer.hpp"
#include "AppendableLogger.hpp"

class MeasurementLogOps
{
public:
    virtual size_t size() = 0;
    virtual unsigned long startTimeMillis() const = 0;
    virtual unsigned long timeMillisAt(size_t idx) = 0;
    virtual float at(size_t idx) = 0;
};

template <typename ValueType>
class MeasurementLog : public MeasurementLogOps
{
    Measurer<ValueType> *_measurer;
    AppendableLogger<ValueType> *_logger;
    unsigned long _curr_millis;
    unsigned long _sampling_interval_millis;

public:
    MeasurementLog(Measurer<ValueType> *measurer, AppendableLogger<ValueType> *logger, unsigned long sampling_interval_millis) : _measurer(measurer), _logger(logger), _curr_millis(0), _sampling_interval_millis(sampling_interval_millis)
    {
    }

    bool collect()
    {
        unsigned long t = millis();

        if (size() == 0)
        {
            _logger->append(_measurer->measure());
            return true;
        }

        uint16_t sample_count = (t - _curr_millis) / _sampling_interval_millis;
        if (sample_count == 0)
            return false; // no new value recorded

        ValueType temp = _measurer->measure();
        for (uint16_t i = 0; i < sample_count; i++)
            _logger->append(temp); // assume the missing values were the same as now

        _curr_millis += sample_count * _sampling_interval_millis;

        return true; // recorded new value(s)
    }

    size_t size()
    {
        return _logger->size();
    }

    unsigned long startTimeMillis() const
    {
        return _curr_millis;
    }

    unsigned long timeMillisAt(size_t idx)
    {
        if (idx >= size())
            return 0;
        return idx * _sampling_interval_millis;
    }

    float at(size_t idx)
    {
        return _measurer->toFloat(_logger->at(idx));
    }
};

#endif // __MEASUREMENT_LOG_HPP__
