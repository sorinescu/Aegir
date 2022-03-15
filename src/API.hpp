#ifndef __AEGIR_API_HPP__
#define __AEGIR_API_HPP__

class PositiveTempMeasure;
class WeightMeasure;
template <typename V>
class MeasurementLog;
class MeasurementLogOps;
class AsyncWebServerRequest;
struct JsonVariantWrapper;

class API
{
    PositiveTempMeasure *_temp;
    WeightMeasure *_weight;
    MeasurementLog<uint16_t> *_temp_history_recent;
    MeasurementLog<float> *_weight_history_recent;
    size_t _temp_stream_idx;
    size_t _weight_stream_idx;
    bool _is_started;

    void getHistoryRecent(AsyncWebServerRequest *request, MeasurementLogOps *log, size_t *curr_idx);

    void getConfig(AsyncWebServerRequest *request);
    void setConfig(AsyncWebServerRequest *request, JsonVariantWrapper const &json);

public:
    API(PositiveTempMeasure *temp, MeasurementLog<uint16_t> *temp_history_recent,
        WeightMeasure *weight, MeasurementLog<float> *weight_history_recent)
        : _temp(temp), _weight(weight),
          _temp_history_recent(temp_history_recent),
          _weight_history_recent(weight_history_recent),
          _is_started(false) {}

    void start();
    bool isStarted() { return _is_started; }

    void sendCurrentTemp();
    void sendCurrentWeight();
};

#endif
