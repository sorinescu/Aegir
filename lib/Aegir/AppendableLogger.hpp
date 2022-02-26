#ifndef __APPENDABLE_LOGGER_HPP__
#define __APPENDABLE_LOGGER_HPP__

template <typename ValueType>
class AppendableLogger {
public:
    virtual ~AppendableLogger() {}
    virtual void append(ValueType v) = 0;
    virtual size_t size() = 0;
    virtual ValueType at(size_t idx) = 0;
};

#endif // __APPENDABLE_LOGGER_HPP__
