#pragma once
#include <Arduino.h>

class MotorController {
public:
  MotorController();
  void begin(uint8_t stepPin, uint8_t dirPin);
  void setFrequency(uint32_t hz);
  void setDirection(int dir);
  void start();
  void stop();
  volatile uint32_t frequency = 0;
  volatile int direction = 1; // +1 or -1
  volatile bool running = false;

  // internal tick countdown until next half-step toggle (in ISR ticks)
  volatile uint32_t _ticksUntilToggle = 0;

private:
  uint8_t _stepPin;
  uint8_t _dirPin;
  volatile bool _state = false;
  unsigned long _halfPeriodMicros = 0;
  unsigned long _lastToggle = 0;
  void toggleStep();
  friend void motor_tick_isr();
};

// helper to register instances so a central tick function may call toggleStep()
void register_motor_instance(int idx, MotorController* m);

// central tick function to be called frequently (e.g., from loop or a timer ISR)
void motor_tick_isr();

// set tick period in microseconds (call from main during setup)
void set_tick_us(unsigned long us);
