#ifndef __AEGIR_API_HPP__
#define __AEGIR_API_HPP__

class PositiveTempMeasure;
class WeightMeasure;
template <typename V>
class MeasurementLog;
class MeasurementLogOps;
class AsyncWebServerRequest;

class API
{
    PositiveTempMeasure *_temp;
    WeightMeasure *_weight;
    MeasurementLog<uint16_t> *_temp_history;
    MeasurementLog<float> *_weight_history;
    size_t _temp_stream_idx;
    size_t _weight_stream_idx;
    bool _is_started;

    void getHistoryMinutes(AsyncWebServerRequest *request, MeasurementLogOps *log, size_t *curr_idx);

public:
    API(PositiveTempMeasure *temp, WeightMeasure *weight, MeasurementLog<uint16_t> *temp_history, MeasurementLog<float> *weight_history) : _temp(temp), _weight(weight), _temp_history(temp_history), _weight_history(weight_history), _is_started(false) {}

    void start();
    bool isStarted() { return _is_started; }

    void sendCurrentTemp();
    void sendCurrentWeight();
};

#endif
