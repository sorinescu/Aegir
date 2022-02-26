#ifndef __MOVING_AVERAGE_HPP__
#define __MOVING_AVERAGE_HPP__

template <typename ValueType, typename AccValueType, size_t SampleCount>
class MovingAverage
{
    ValueType _outlier_threshold_div;
    ValueType _samples[SampleCount];
    size_t _size;
    size_t _pos;
    ValueType _curr_value;
    bool _dirty;

public:
    MovingAverage(ValueType outlier_threshold_div = 0) : _outlier_threshold_div(outlier_threshold_div)
    {
        reset();
    }

    void reset()
    {
        _size = 0;
        _pos = 0;
        _curr_value = 0;
        _dirty = false;
    }

    void add(ValueType sample)
    {
        _samples[_pos] = sample;
        if (_size < SampleCount)
            ++_size;
        _pos = (_pos + 1) % SampleCount;
        _dirty = true;
    }

    ValueType avg()
    {
        if (_size == 0)
            return 0;

        if (!_dirty)
            return _curr_value;

        // No outlier detection
        if (_outlier_threshold_div == 0)
        {
            AccValueType acc = 0;
            for (size_t i = 0; i < _size; i++)
                acc += _samples[i];

            _curr_value = ValueType(acc / _size);
            _dirty = false;

            return _curr_value;
        }


        // Reject outliers which have more than a (100/_outlier_threshold_div)% deviation from median
        ValueType med = median();
        ValueType max_diff = med / _outlier_threshold_div;

        if (max_diff < 0)
            max_diff = -max_diff;

        AccValueType acc = 0;
        size_t size = 0;
        for (size_t i = 0; i < _size; i++)
        {
            ValueType diff;
            if (med > _samples[i])
                diff = med - _samples[i];
            else
                diff = _samples[i] - med;

            if (diff <= max_diff)
            {
                acc += _samples[i];
                size++;
            }
        }

        if (size != 0)
            _curr_value = ValueType(acc / size);
        else
            _curr_value = 0;

        _dirty = false;

        return _curr_value;
    }

    ValueType median() const
    {
        if (_size == 0)
            return 0;
            
        ValueType sorted_samples[_size];
        for (size_t i = 0; i < _size; i++)
            sorted_samples[i] = _samples[i];

        // Sort values to obtain median.
        // We only need the smallest _size/2 elements.
        size_t i;
        for (i = 0; i + i <= _size; i++)
            for (size_t j = i + 1; j < _size; j++)
                if (sorted_samples[i] > sorted_samples[j])
                {
                    ValueType tmp = sorted_samples[i];
                    sorted_samples[i] = sorted_samples[j];
                    sorted_samples[j] = tmp;
                }

        return sorted_samples[i-1];
    }
};

template <int SampleCount>
using Uint16MovingAverage = MovingAverage<uint16_t, uint32_t, SampleCount>;
template <int SampleCount>
using LongMovingAverage = MovingAverage<long, long, SampleCount>;

#endif // __MOVING_AVERAGE_HPP__
