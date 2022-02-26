#include <Arduino.h>
#include "PIDRelay.hpp"

#define RELAY_ON() digitalWrite(_pin, _on_value)
#define RELAY_OFF() digitalWrite(_pin, HIGH ^ _on_value)
#define PWM_RANGE 1000
#define AUTOTUNE_OUTPUT_STEP (PWM_RANGE / 20)
#define AUTOTUNE_TEST_TIME_SECONDS 5 * 60
#define AUTOTUNE_TEST_SAMPLES 200

PIDRelay::PIDRelay(uint8_t pin, uint8_t pin_mode, uint8_t on_value, long time_window_millis, bool output_increases_with_input)
    : _pin(pin), _pin_mode(pin_mode), _on_value(on_value), _time_window_millis(time_window_millis),
      _input(0), _pwm_fill_rate(0), _setpoint(0), _autotuning(false),
      _pid(&_input, &_pwm_fill_rate, &_setpoint, 0, 0, 0, output_increases_with_input ? QuickPID::Action::direct : QuickPID::Action::reverse),
      _stune(nullptr)
{
    pinMode(_pin, _pin_mode);
    RELAY_OFF();

    _time_window_start_millis = millis();
    _rate_multiplier = _time_window_millis / PWM_RANGE;

    _pid.SetSampleTimeUs(_time_window_millis * 1000);
    _pid.SetOutputLimits(0, PWM_RANGE);
    _pid.SetProportionalMode(QuickPID::pMode::pOnMeas);
    _pid.SetAntiWindupMode(QuickPID::iAwMode::iAwClamp);
    _pid.SetMode(QuickPID::Control::manual); // just to be safe
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

void PIDRelay::autotune(float target_value, float emergency_stop_input, float min_input, float max_input)
{
    if (is_autotuning())
        return;

    _stune = new sTune();
    _stune->Configure(max_input - min_input, PWM_RANGE, 0, AUTOTUNE_OUTPUT_STEP, AUTOTUNE_TEST_TIME_SECONDS,
                      0, AUTOTUNE_TEST_SAMPLES);
    _stune->SetControllerAction(_pid.GetDirection() == uint8_t(QuickPID::Action::direct) ? sTune::Action::directIP : sTune::Action::reverseIP);
    _stune->SetTuningMethod(sTune::TuningMethod::NoOvershoot_PID);
    _stune->SetEmergencyStop(emergency_stop_input);

    _stune->SetSerialMode(sTune::SerialMode::printOFF);
}

bool PIDRelay::is_autotuning() const
{
    return _stune != nullptr;
}

void PIDRelay::set_auto_target_value(float value)
{
    _setpoint = value;
    _pid.SetMode(QuickPID::Control::automatic);
}

void PIDRelay::set_manual_pwm_fill_rate(float pwm_fill_rate)
{
    if (pwm_fill_rate < 0 || pwm_fill_rate > 1.0)
        return;

    _pid.SetMode(QuickPID::Control::manual);
    _pwm_fill_rate = pwm_fill_rate * PWM_RANGE;
}

bool PIDRelay::update(float input)
{
    _input = input;

    if (is_autotuning() && _stune->Run() == sTune::TunerStatus::tunings) // active just once when sTune is done
    {
        float kp, ki, kd;

        _stune->GetAutoTunings(&kp, &ki, &kd); // sketch variables updated by sTune
        delete _stune;
        _stune = nullptr;

        _pid.SetTunings(kp, ki, kd); // update PID with the new tunings
        _pid.Compute();

        // Safety first - turn off output
        set_manual_pwm_fill_rate(0);
        RELAY_OFF();

        return true;
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
