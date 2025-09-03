#pragma once
#include <Arduino.h>
#include "config.h"

namespace EmergencyRaise {
  enum class Result { STAY, TO_UP };

  // extern helpers
  extern void roadRed();
  extern void marineRed();
  extern void gatesDown();
  extern void motorEnable();
  extern void motorDirUp();
  extern void motorCruiseUp();
  extern void motorStop();
  extern bool topLimitPressed();

  static bool s_inited = false;

  inline void enter() {
    s_inited = true;
    roadRed(); marineRed(); gatesDown();
    motorEnable(); motorDirUp(); motorCruiseUp();
  }

  inline Result tick() {
    if (!s_inited) enter();

    if (topLimitPressed()) {
      motorStop();
      return Result::TO_UP;
    }
    return Result::STAY;
  }
}