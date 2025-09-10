#pragma once
// Pins (adjust to your board)
static const int PIN_MOTOR_PWM = 27;
static const int PIN_MOTOR_DIR = 26;

// ESP32 PWM parameters
static const int PWM_MOTOR_CH  = 0;
static const int PWM_FREQ_HZ   = 20000;
static const int PWM_RES_BITS  = 8;

// Timers (ms)
static const unsigned long T_YELLOW         = 2000;
static const unsigned long T_OPEN_EXPECTED  = 5200;
static const unsigned long T_OPEN_MAX       = 7000;
static const unsigned long T_CLOSE_EXPECTED = 5200;
static const unsigned long T_CLOSE_MAX      = 7000;

// Speeds (abstract 0..255; .ino writes to PWM)
static const int PWM_OPEN_CRUISE  = 170;
static const int PWM_OPEN_SLOW    = 110;
static const int PWM_CLOSE_CRUISE = 160;
static const int PWM_CLOSE_SLOW   = 100;
