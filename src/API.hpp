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
    MeasurementLog<uint16_t> *_temp_history_recent;
    MeasurementLog<uint16_t> *_temp_history_full;
    MeasurementLog<float> *_weight_history_recent;
    MeasurementLog<float> *_weight_history_full;
    size_t _temp_stream_idx;
    size_t _weight_stream_idx;
    bool _is_started;

    void getHistoryMinutes(AsyncWebServerRequest *request, MeasurementLogOps *log, size_t *curr_idx);

public:
    API(PositiveTempMeasure *temp, MeasurementLog<uint16_t> *temp_history_recent, MeasurementLog<uint16_t> *temp_history_full,
        WeightMeasure *weight, MeasurementLog<float> *weight_history_recent, MeasurementLog<float> *weight_history_full)
        : _temp(temp), _weight(weight),
          _temp_history_recent(temp_history_recent), _temp_history_full(temp_history_full),
          _weight_history_recent(weight_history_recent), _weight_history_full(weight_history_full),
          _is_started(false) {}

    void start();
    bool isStarted() { return _is_started; }

    void sendCurrentTemp();
    void sendCurrentWeight();
};

#endif
