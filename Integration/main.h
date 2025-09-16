#pragma once
#include <Arduino.h>
#include "Config.h"

// Forward declarations
class Motor;
class LimitSwitch;
class Ultrasonic;

// Timings & speeds (tune later)
struct MainConfig {
  uint32_t T_YELLOW         = 2000;
  uint32_t T_OPEN_EXPECTED  = 5200;
  uint32_t T_OPEN_MAX       = 7000;
  uint32_t T_CLOSE_EXPECTED = 5200;
  uint32_t T_CLOSE_MAX      = 7000;
  int PWM_OPEN_CRUISE  = 170;
  int PWM_OPEN_SLOW    = 110;
  int PWM_CLOSE_CRUISE = 160;
  int PWM_CLOSE_SLOW   = 100;
};

// External effectors/sensors supplied by Integration.ino
using VoidFn       = void (*)();
using BoolFn       = bool (*)();

void Main_init(Motor* m, LimitSwitch* ls, Ultrasonic* wait, Ultrasonic* under,
               const MainConfig& cfg = MainConfig());

void Main_setEnvironment(VoidFn roadG, VoidFn roadY, VoidFn roadR,
                         VoidFn marineG, VoidFn marineR,
                         VoidFn gatesUp, VoidFn gatesDown, BoolFn gatesAreDown,
                         BoolFn carOnBridge, BoolFn boatWaiting, BoolFn boatUnder);

void Main_tick(bool reqRaise, bool reqLower, bool reqAbort, uint32_t tNow);
