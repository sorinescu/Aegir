#ifndef __APP_CONFIG_HPP__
#define __APP_CONFIG_HPP__

#define MAX_TEMP_CONTROL_PROFILES 4
#define MAX_TEMP_CONTROL_PROFILE_RELAYS 2
#define TEMP_CONTROL_PROFILE_NAME_SIZE 48

struct TempRelayConfig
{
    float kp, kd, ki;
    bool active;          // True if this relay is used by the profile and its config is valid
    bool ssr;             // If true, the relay is a SRR; otherwise, it's a mechanical relay
    bool normally_closed; // If true, the relay is closed when the control pin is OFF
    bool cooling;         // If true, the relay controls a cooling device; otherwise, it controls a heating device
};

struct TempControlProfile
{
    char name[TEMP_CONTROL_PROFILE_NAME_SIZE];
    TempRelayConfig relay_config[MAX_TEMP_CONTROL_PROFILE_RELAYS]; // There are only two physical relays on the board; the one(s) in use by the profile have the "in_use" flag set
    bool valid;                                                    // True if this profile was configured by the user
};

struct FirebaseAuthConfig
{
    char api_key[64];
    char user_email[256];
    char user_password[64];
};

class AppConfig
{
public:
    AppConfig();

    void begin();

    float weight_scale() { return _config._weight_scale; }
    void set_weight_scale(float value) { _config._weight_scale = value; }

    size_t temp_control_profile_count() const
    {
        for (size_t i = 0; i < MAX_TEMP_CONTROL_PROFILES; ++i)
        {
            if (!_config._temp_control_profiles[i].valid)
                return i;
        }
        return MAX_TEMP_CONTROL_PROFILES;
    }

    TempControlProfile const *temp_control_profile(size_t idx) const
    {
        if (idx >= 2)
            return nullptr;
        return &_config._temp_control_profiles[idx];
    }

    void set_temp_control_profile_count(size_t count)
    {
        for (size_t i = count; i < MAX_TEMP_CONTROL_PROFILES; ++i)
        {
            memset(&_config._temp_control_profiles[i], 0, sizeof(_config._temp_control_profiles[i]));
        }
    }

    void set_temp_control_profile(size_t idx, TempControlProfile const &config)
    {
        if (idx >= MAX_TEMP_CONTROL_PROFILES)
            return;
        _config._temp_control_profiles[idx] = config;
    }

    FirebaseAuthConfig const &firebase_auth() const
    {
        return _config._firebase_auth;
    }

    void set_firebase_auth(FirebaseAuthConfig const &auth)
    {
        _config._firebase_auth = auth;
    }

    void commit();
    void reset();

private:
    struct
    {
        float _weight_scale;
        TempControlProfile _temp_control_profiles[MAX_TEMP_CONTROL_PROFILES];
        FirebaseAuthConfig _firebase_auth;
    } _config;

    void setDefaults();
    bool readV1();
    bool readV2();
};

extern AppConfig app_config;

#endif // __APP_CONFIG_HPP__
