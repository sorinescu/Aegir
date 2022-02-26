#ifndef __MOVING_AVERAGE_HPP__
#define __MOVING_AVERAGE_HPP__

template <typename ValueType, typename AccValueType, size_t SampleCount>
class MovingAverage
{
    ValueType _samples[SampleCount];
    size_t _size;
    size_t _pos;
    ValueType _curr_value;
    bool _dirty;

public:
    MovingAverage()
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

        if (_dirty)
        {
            AccValueType acc = 0;
            for (size_t i = 0; i < _size; i++)
                acc += _samples[i];

            // Reject outliers (more than 5% deviation from average)
            ValueType avg_sample = ValueType(acc / _size);
            ValueType max_diff = avg_sample / 20;
            
            if (max_diff < 0)
                max_diff = -max_diff;

            acc = 0;            
            for (size_t i = 0; i < _size; i++)
            {
                ValueType diff = _samples[i] - avg_sample;
                if (avg_sample > _samples[i])
                    diff = avg_sample - _samples[i];

                if (diff <= max_diff)
                    acc += _samples[i];
            }

            _curr_value = ValueType(acc / _size);

            _dirty = false;
        }

        return _curr_value;
    }
};

template <int SampleCount>
using Uint16MovingAverage = MovingAverage<uint16_t, uint32_t, SampleCount>;
template <int SampleCount>
using LongMovingAverage = MovingAverage<long, long, SampleCount>;

#endif // __MOVING_AVERAGE_HPP__
