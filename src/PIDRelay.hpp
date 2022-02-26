#ifndef __PID_RELAY_HPP__
#define __PID_RELAY_HPP__

#include <stdint.h>
#include <QuickPID.h>
#include <sTune.h>

class PIDRelay
{
    uint8_t _pin;
    uint8_t _pin_mode;
    uint8_t _on_value;
    unsigned long _time_window_millis;
    unsigned long _time_window_start_millis;
    long _rate_multiplier;
    float _input;
    float _pwm_fill_rate;
    float _setpoint;
    bool _autotuning;
    QuickPID _pid;
    sTune* _stune;

public:
    // time_window_millis must be a multiple of 1000 !
    PIDRelay(uint8_t pin, uint8_t pin_mode, uint8_t on_value, long time_window_millis, bool output_increases_with_input = true);

    void set_pid_params(float kp, float ki, float kd);
    void get_pid_params(float *kp, float *ki, float *kd);
    
    // Enters auto (PID) mode and sets target value (e.g. desired temperature).
    void set_auto_target_value(float value);

    // Enters manual mode and sets the PWM fill rate.
    // A rate of 0 means the relay will be premanently off.
    // A rate of 1.0 means the relay will be permanently on.
    void set_manual_pwm_fill_rate(float pwm_fill_rate);

    // Enters autotune mode.
    // The `update` function will return true when the autotune is complete.
    void autotune(float target_value, float emergency_stop_input, float min_input, float max_input);

    // Returns true while there is an autotune in progress.
    bool is_autotuning() const;

    // Must be called at every loop() with the current value of the input (e.g. measured temperature).
    // Returns true if there is an autotune in progress and it has computed the new PID parameters.
    // In this case, the new values can be obtained by calling `get_pid_params`.
    bool update(float input);
};

#endif // __PID_RELAY_HPP__
