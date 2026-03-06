#include <Arduino.h>
#include "config.h"
#include "motor_control.h"
#include <Encoder.h>
#include <U8g2lib.h>
#include <HardwareTimer.h>

// Simple firmware: use the printer's rotary encoder + 12864 screen
// to set a constant speed and direction for each motor.

U8G2_ST7920_128X64_F_SW_SPI u8g2(U8G2_R0, /* clock=*/ LCD_CLK_PIN, /* data=*/ LCD_DAT_PIN, /* cs=*/ LCD_CS_PIN);


Encoder encoder(ENC_A_PIN, ENC_B_PIN);
const int ENC_BUTTON_PIN = ENC_BTN_PIN;

MotorController motors[4]; // X, Y, Z, E

int selectedMotor = 0;
long lastEncPos = 0;
unsigned long lastEncMoveTime = 0;
int lastBtnState = HIGH;
unsigned long lastBtnChange = 0;
const unsigned long BTN_DEBOUNCE_MS = 40;

void drawMenu() {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x10_tr);
  for (int i = 0; i < 4; ++i) {
    if (i == selectedMotor) u8g2.drawStr(0, 10 + i * 12, ">");
    else u8g2.drawStr(0, 10 + i * 12, " ");
    char buf[64];
    snprintf(buf, sizeof(buf), "%c %s: %ld Hz %s",
             motors[i].direction > 0 ? '+' : '-',
             (i==0?"X":(i==1?"Y":(i==2?"Z":"E"))),
             (long)motors[i].frequency,
             motors[i].running?"on":"off");
    u8g2.drawStr(12, 10 + i * 12, buf);
  }
  u8g2.sendBuffer();
}

void setup() {
  pinMode(ENC_BUTTON_PIN, INPUT_PULLUP);
  u8g2.begin();

  // initialize motor controllers with pins from config
  motors[0].begin(X_STEP_PIN, X_DIR_PIN);
  motors[1].begin(Y_STEP_PIN, Y_DIR_PIN);
  motors[2].begin(Z_STEP_PIN, Z_DIR_PIN);
  motors[3].begin(E_STEP_PIN, E_DIR_PIN);

  // register instances for central tick helper
  register_motor_instance(0, &motors[0]);
  register_motor_instance(1, &motors[1]);
  register_motor_instance(2, &motors[2]);
  register_motor_instance(3, &motors[3]);

  // configure hardware timer to call motor_tick_isr at TICK_HZ
  unsigned long tick_us = 1000000UL / TICK_HZ;
  set_tick_us(tick_us);
  HardwareTimer timer(TIM2);
  timer.setOverflow(TICK_HZ, HERTZ_FORMAT);
  timer.attachInterrupt(motor_tick_isr);
  timer.resume();

  // default: stopped
  for (int i = 0; i < 4; ++i) {
    motors[i].setFrequency(0);
    motors[i].setDirection(1);
    motors[i].stop();
  }

  drawMenu();
}

void loop() {
  long pos = encoder.read();
  if (pos != lastEncPos) {
    long delta = pos - lastEncPos;
    unsigned long now = millis();
    unsigned long dt = (lastEncMoveTime==0)? 0 : (now - lastEncMoveTime);
    lastEncMoveTime = now;
    lastEncPos = pos;

    // acceleration based on rotation speed (shorter dt => faster changes)
    int speedMult = 1;
    if (dt > 0 && dt < 40) speedMult = 8;
    else if (dt < 100) speedMult = 4;
    else if (dt < 300) speedMult = 2;

    // if button held, increase multiplier for fast adjustments
    int btnHeld = (digitalRead(ENC_BUTTON_PIN) == LOW);
    if (btnHeld) speedMult *= 5;

    long step = (long)delta * speedMult;
    long newf = (long)motors[selectedMotor].frequency + step; // 1 Hz per detent base
    if (newf < 0) newf = 0;
    motors[selectedMotor].setFrequency((uint32_t)newf);
    if (newf > 0) motors[selectedMotor].start();
    else motors[selectedMotor].stop();
    drawMenu();
  }

  // button to cycle motor selection / toggle direction on long press
  static unsigned long btnDownAt = 0;
  int rawBtn = digitalRead(ENC_BUTTON_PIN);
  unsigned long now = millis();
  if (rawBtn != lastBtnState && (now - lastBtnChange) > BTN_DEBOUNCE_MS) {
    lastBtnState = rawBtn;
    lastBtnChange = now;
    if (rawBtn == LOW) {
      // pressed
      btnDownAt = now;
    } else {
      // released
      if (btnDownAt != 0) {
        unsigned long held = now - btnDownAt;
        if (held >= 800) {
          // long press -> toggle direction
          motors[selectedMotor].setDirection(-motors[selectedMotor].direction);
          drawMenu();
        } else {
          // short press -> next motor
          selectedMotor = (selectedMotor + 1) % 4;
          drawMenu();
        }
      }
      btnDownAt = 0;
    }
  }

  delay(20);

  // advance stepper toggles; keep this tight for better timing
  motor_tick_isr();
}