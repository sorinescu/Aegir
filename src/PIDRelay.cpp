#include <Arduino.h>
#include "PIDRelay.hpp"

#define RELAY_ON() digitalWrite(_pin, _on_value)
#define RELAY_OFF() digitalWrite(_pin, HIGH ^ _on_value)
#define PWM_RANGE 1000

PIDRelay::PIDRelay(uint8_t pin, uint8_t pin_mode, uint8_t on_value, long time_window_millis, float kp, float ki, float kd)
    : _pin(pin), _pin_mode(pin_mode), _on_value(on_value), _time_window_millis(time_window_millis),
      _input(0), _pwm_fill_rate(0), _setpoint(0), _pid(&_input, &_pwm_fill_rate, &_setpoint, kp, ki, kd, DIRECT)
{
    pinMode(_pin, _pin_mode);
    RELAY_OFF();

    _time_window_start_millis = millis();
    _rate_multiplier = _time_window_millis / PWM_RANGE;
    
    _pid.SetSampleTime(_time_window_millis);
    _pid.SetOutputLimits(0, PWM_RANGE);
}

void PIDRelay::update(float input)
{
    _input = input;
    _pid.Compute();

    if (millis() - _time_window_start_millis >= _time_window_millis)
    { 
        // Time to shift the Relay Window
        _time_window_start_millis += _time_window_millis;
    }

    if (_pwm_fill_rate * _rate_multiplier > millis() - _time_window_start_millis) 
        RELAY_ON();
    else 
        RELAY_OFF();
}
