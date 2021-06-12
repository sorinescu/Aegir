#ifndef __PID_RELAY_HPP__
#define __PID_RELAY_HPP__

#include <stdint.h>
#include <PID_v1.h>

class PIDRelay
{
    uint8_t _pin;
    uint8_t _pin_mode;
    uint8_t _on_value;
    long _time_window_millis;
    long _time_window_start_millis;
    long _rate_multiplier;
    float _input;
    float _pwm_fill_rate;
    float _setpoint;
    PID _pid;
public:
    PIDRelay(uint8_t pin, uint8_t pin_mode, uint8_t on_value, long time_window_millis, float kp, float ki, float kd);

    void update(float input);
};

#endif // __PID_RELAY_HPP__
