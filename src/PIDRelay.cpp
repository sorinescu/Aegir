#include <Arduino.h>
#include "PIDRelay.hpp"

#define RELAY_ON() digitalWrite(_pin, _on_value)
#define RELAY_OFF() digitalWrite(_pin, HIGH ^ _on_value)
#define PWM_RANGE 1000

PIDRelay::PIDRelay(uint8_t pin, uint8_t pin_mode, uint8_t on_value, long time_window_millis, bool output_increases_with_input)
    : _pin(pin), _pin_mode(pin_mode), _on_value(on_value), _time_window_millis(time_window_millis),
      _input(0), _pwm_fill_rate(0), _setpoint(0), _autotuning(false),
      _pid(&_input, &_pwm_fill_rate, &_setpoint, 0, 0, 0, output_increases_with_input ? QuickPID::DIRECT : QuickPID::REVERSE)
{
    pinMode(_pin, _pin_mode);
    RELAY_OFF();

    _time_window_start_millis = millis();
    _rate_multiplier = _time_window_millis / PWM_RANGE;

    _pid.SetSampleTimeUs(_time_window_millis * 1000);
    _pid.SetOutputLimits(0, PWM_RANGE);
}

void PIDRelay::set_pid_params(float kp, float ki, float kd)
{
    _pid.SetTunings(kp, ki, kd);
}

void PIDRelay::get_pid_params(float *kp, float *ki, float *kd)
{
    *kp = _pid.GetKp();
    *ki = _pid.GetKi();
    *kd = _pid.GetKd();
}

void PIDRelay::autotune(float target_value)
{
    if (is_autotuning())
        return;

    // Setpoint is the desired value of the measured input (e.g. the target temperature)
    // Output is the current controller output (e.g. the PWM fill rate or the analog voltage at the output)
    // We never want to overshoot the setpoint
    _pid.AutoTune(tuningMethod::NO_OVERSHOOT_PID);

    byte outputStep = PWM_RANGE / 100;
    byte hysteresis = PWM_RANGE / 500;
    float output = PWM_RANGE / 3; // set a conservative initial output value

    _pid.autoTune->autoTuneConfig(outputStep, hysteresis, target_value, output, QuickPID::DIRECT, false, _time_window_millis * 1000);
}

bool PIDRelay::is_autotuning() const
{
    return _pid.autoTune != nullptr;
}

void PIDRelay::set_auto_target_value(float value)
{
    _setpoint = value;
    _pid.SetMode(QuickPID::AUTOMATIC);
}

void PIDRelay::set_manual_pwm_fill_rate(float pwm_fill_rate)
{
    if (pwm_fill_rate < 0 || pwm_fill_rate > 1.0)
        return;

    _pid.SetMode(QuickPID::MANUAL);
    _pwm_fill_rate = pwm_fill_rate * PWM_RANGE;
}

bool PIDRelay::update(float input)
{
    _input = input;

    if (is_autotuning())
    {
        if (_pid.autoTune->autoTuneLoop() == AutoTunePID::TUNINGS)
        {
            float kp, ki, kd;
            _pid.autoTune->setAutoTuneConstants(&kp, &ki, &kd);
            _pid.SetTunings(kp, ki, kd);

            _pid.clearAutoTune();
            _pid.autoTune = nullptr;

            // Safety first - turn off output
            set_manual_pwm_fill_rate(0);
            RELAY_OFF();
            return true;
        }
    }
    else
    {
        _pid.Compute();
    }

    if (millis() - _time_window_start_millis >= _time_window_millis)
    {
        // Time to shift the Relay Window
        _time_window_start_millis += _time_window_millis;
    }

    if (_pwm_fill_rate * _rate_multiplier > millis() - _time_window_start_millis)
        RELAY_ON();
    else
        RELAY_OFF();
    
    return false;
}
