#ifndef __MOVING_AVERAGE_HPP__
#define __MOVING_AVERAGE_HPP__

template <typename ValueType, typename AccValueType, int SampleCount>
class MovingAverage
{
    ValueType _samples[SampleCount];
    size_t _size;
    size_t _pos;

public:
    MovingAverage()
    {
        reset();
    }

    void reset()
    {
        _size = 0;
        _pos = 0;
    }

    void add(ValueType sample)
    {
        _samples[_pos] = sample;
        if (_size < SampleCount)
            ++_size;
        _pos = (_pos + 1) % SampleCount;
    }

    ValueType avg() const
    {
        if (_size == 0)
            return 0;

        AccValueType acc = 0;
        for (size_t i = 0; i < _size; i++)
            acc += _samples[i];
        return ValueType(acc / _size);
    }
};

template <int SampleCount>
using Uint16MovingAverage = MovingAverage<uint16_t, uint32_t, SampleCount>;
template <int SampleCount>
using LongMovingAverage = MovingAverage<long, long, SampleCount>;

#endif // __MOVING_AVERAGE_HPP__
