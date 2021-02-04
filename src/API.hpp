#ifndef __AEGIR_API_HPP__
#define __AEGIR_API_HPP__

class PositiveTempMeasure;

class API
{
    PositiveTempMeasure *_temp;
    bool _isStarted;

public:
    API(PositiveTempMeasure *temp) : _temp(temp), _isStarted(false) {}
    void start();
    bool isStarted() { return _isStarted; }
};

#endif
