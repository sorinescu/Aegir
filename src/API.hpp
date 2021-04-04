#ifndef __AEGIR_API_HPP__
#define __AEGIR_API_HPP__

class PositiveTempMeasure;
class TempHistory;
class HX711_ADC;

class API
{
    PositiveTempMeasure *_temp;
    TempHistory *_temp_history;
    HX711_ADC *_load_cell;
    bool _is_started;
    uint16_t _temp_stream_idx;

public:
    API(PositiveTempMeasure *temp, TempHistory *temp_history, HX711_ADC *load_cell) : _temp(temp), _temp_history(temp_history), _load_cell(load_cell), _is_started(false) {}

    void start();
    bool isStarted() { return _is_started; }

    void sendCurrentTemp();
    void sendCurrentWeight();
};

#endif
