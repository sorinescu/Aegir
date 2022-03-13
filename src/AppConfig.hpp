#ifndef __APP_CONFIG_HPP__
#define __APP_CONFIG_HPP__

#define MAX_TEMP_CONTROL_PROFILES 4
#define TEMP_CONTROL_PROFILE_NAME_SIZE 48

struct TempRelayConfig
{
    float kp, kd, ki;
    bool in_use;          // True if this relay is used and its config is valid
    bool ssr;             // If true, the relay is a SRR; otherwise, it's a mechanical relay
    bool normally_closed; // If true, the relay is closed when the control pin is OFF
    bool cooling;         // If true, the relay controls a cooling device; otherwise, it controls a heating device
};

struct TempControlProfile
{
    char name[TEMP_CONTROL_PROFILE_NAME_SIZE];
    TempRelayConfig relays[2]; // There are only two physical relays on the board; the one(s) in use by the profile have the "in_use" flag set
    bool valid;                // True if this profile was configured by the user
};

class AppConfig
{
public:
    AppConfig();

    void begin();

    float weight_scale() { return _config._weight_scale; }
    void set_weight_scale(float value) { _config._weight_scale = value; }

    TempControlProfile const *temp_control_profile(size_t idx)
    {
        if (idx >= 2)
            return nullptr;

        return &_config._temp_control_profiles[idx];
    }
    void set_temp_control_profile(size_t idx, TempControlProfile const &config)
    {
        if (idx >= MAX_TEMP_CONTROL_PROFILES)
            return;
        _config._temp_control_profiles[idx] = config;
    }

    void commit();
    void reset();

private:
    struct
    {
        float _weight_scale;
        TempControlProfile _temp_control_profiles[MAX_TEMP_CONTROL_PROFILES];
    } _config;

    void setDefaults();
    bool readV1();
    bool readV2();
};

extern AppConfig app_config;

#endif // __APP_CONFIG_HPP__
