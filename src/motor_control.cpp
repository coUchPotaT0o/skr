#include "motor_control.h"
#include <Arduino.h>

static MotorController* g_motors[4] = {nullptr, nullptr, nullptr, nullptr};
// tick period in microseconds (set by main based on TICK_HZ)
static volatile unsigned long g_tick_us = 50; // default 20 kHz

MotorController::MotorController() {}

void MotorController::begin(uint8_t stepPin, uint8_t dirPin) {
  _stepPin = stepPin;
  _dirPin = dirPin;
  pinMode(_stepPin, OUTPUT);
  pinMode(_dirPin, OUTPUT);
  digitalWrite(_stepPin, LOW);
  digitalWrite(_dirPin, LOW);
}

void MotorController::setFrequency(uint32_t hz) {
  frequency = hz;
  if (hz == 0) {
    _halfPeriodMicros = 0;
    stop();
  } else {
    _halfPeriodMicros = 1000000UL / (hz * 2UL);
    // compute initial ticks until first toggle based on current tick period
    if (g_tick_us > 0) {
      uint32_t ticks = (_halfPeriodMicros + g_tick_us/2) / g_tick_us;
      if (ticks == 0) ticks = 1;
      _ticksUntilToggle = ticks;
    } else {
      _ticksUntilToggle = 1;
    }
  }
}

void MotorController::setDirection(int dir) {
  direction = (dir >= 0) ? 1 : -1;
  digitalWrite(_dirPin, direction > 0 ? HIGH : LOW);
}

void MotorController::start() {
  if (frequency == 0) return;
  running = true;
  if (g_tick_us > 0) {
    uint32_t ticks = (_halfPeriodMicros + g_tick_us/2) / g_tick_us;
    if (ticks == 0) ticks = 1;
    _ticksUntilToggle = ticks;
  } else {
    _ticksUntilToggle = 1;
  }
}

void MotorController::stop() {
  running = false;
  digitalWrite(_stepPin, LOW);
}

// Called from main loop to perform step toggles; keeps callers simple and safe
void MotorController::toggleStep() {
  // This is called from ISR context on each tick; use _ticksUntilToggle
  if (!running || _halfPeriodMicros == 0) return;
  if (_ticksUntilToggle == 0) {
    _state = !_state;
    digitalWrite(_stepPin, _state ? HIGH : LOW);
    // reload ticks for next half-period
    if (g_tick_us > 0) {
      uint32_t ticks = (_halfPeriodMicros + g_tick_us/2) / g_tick_us;
      if (ticks == 0) ticks = 1;
      _ticksUntilToggle = ticks;
    } else {
      _ticksUntilToggle = 1;
    }
  } else {
    _ticksUntilToggle--;
  }
}

// A small helper to call all motors toggle from single place
void motor_tick_isr() {
  for (int i = 0; i < 4; ++i) {
    if (g_motors[i]) g_motors[i]->toggleStep();
  }
}

// Optional: register motor pointers so external tick can call toggleStep
void register_motor_instance(int idx, MotorController* m) {
  if (idx >= 0 && idx < 4) g_motors[idx] = m;
}

// called from main to set tick period in microseconds
void set_tick_us(unsigned long us) {
  g_tick_us = us;
}
