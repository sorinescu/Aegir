#ifndef __APP_CONFIG_HPP__
#define __APP_CONFIG_HPP__

class AppConfig
{
    float _weight_scale;

    void setDefaults();
    bool readV1();

public:
    AppConfig();

    void begin();

    float weight_scale() { return _weight_scale; }
    void set_weigth_scale(float value) { _weight_scale = value; }

    void commit();
    void reset();
};

extern AppConfig app_config;

#endif // __APP_CONFIG_HPP__
