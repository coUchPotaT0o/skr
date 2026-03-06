// Pin configuration - update to match your SKR Mini E3 V3 board wiring

#pragma once

// Step pins (placeholders - change to your board's pins)
#define X_STEP_PIN 2
#define X_DIR_PIN 3
#define Y_STEP_PIN 4
#define Y_DIR_PIN 5
#define Z_STEP_PIN 6
#define Z_DIR_PIN 7
#define E_STEP_PIN 8
#define E_DIR_PIN 9

// Rotary encoder pins (A, B, push)
#define ENC_A_PIN 10
#define ENC_B_PIN 11
#define ENC_BTN_PIN 12

// LCD pins for ST7920 128x64 (software SPI) - adjust as needed
#define LCD_CLK_PIN 13
#define LCD_DAT_PIN 14
#define LCD_CS_PIN 15

// Hardware timer tick frequency (Hz) used to schedule step toggles from ISR.
// Higher values yield finer step timing resolution but increase ISR load.
// Start around 20000 (20 kHz) and adjust if needed.
#define TICK_HZ 20000
