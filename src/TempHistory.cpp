#include "TempHistory.hpp"
#include "TempMeasure.hpp"

bool TempHistory::collect()
{
    unsigned long t = millis();

    if (size() == 0)
    {
        _logger.append(_temp->currentRawTemp());
        return true;
    }

    uint16_t sample_count = (t - _curr_millis) / _sampling_interval_millis;
    if (sample_count == 0)
        return false; // no new value recorded

    uint16_t temp = _temp->currentRawTemp();
    for (uint16_t i = 0; i < sample_count; i++)
        _logger.append(temp); // assume the missing values were the same as now

    _curr_millis += sample_count * _sampling_interval_millis;

    return true; // recorded new value(s)
}

float TempHistory::at(size_t idx)
{
    return _temp->convertRawTemp(_logger.at(idx));
}

unsigned long TempHistory::timeMillisAt(size_t idx)
{
    if (idx >= size())
        return 0;
    return idx * _sampling_interval_millis;
}
