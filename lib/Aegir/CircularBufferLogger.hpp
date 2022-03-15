#ifndef __CIRCULAR_BUFFER_HPP__
#define __CIRCULAR_BUFFER_HPP__

#include "AppendableLogger.hpp"

template <typename ValueType>
class CircularBufferLogger: public AppendableLogger<ValueType>
{
    size_t _capacity;
    size_t _size;
    size_t _write_pos;
    ValueType *_buf;

public:
    CircularBufferLogger(size_t capacity) : _capacity(capacity), _size(0), _write_pos(0)
    {
        _buf = new ValueType[capacity];
    }
    ~CircularBufferLogger()
    {
        delete[] _buf;
    }

    void append(ValueType v) override
    {
        _buf[_write_pos++] = v;
        if (_write_pos == _capacity)
            _write_pos = 0;
        if (++_size > _capacity)
            _size = _capacity;
    }

    size_t size() override
    {
        return _size;
    }

    ValueType at(size_t idx) override
    {
        if (idx >= _size)
            return 0;

        size_t pos = _capacity + _write_pos - _size + idx;
        if (pos >= _capacity)
            pos -= _capacity;
        return _buf[pos];
    }
};

#endif // __CIRCULAR_BUFFER_HPP__
