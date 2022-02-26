#ifndef __MEASURER_HPP__
#define __MEASURER_HPP__

template <typename ValueType>
class Measurer
{
public:
    virtual ValueType measure() = 0;
    virtual float toFloat(ValueType raw) = 0;

    float measureFloat() { return toFloat(measure()); }
};

#endif
