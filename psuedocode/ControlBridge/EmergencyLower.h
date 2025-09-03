#pragma once
#include <Arduino.h>
#include "config.h"

namespace EmergencyLower {
  enum class Result { STAY, TO_DOWN };

  // extern helpers
  extern void roadRed();
  extern void marineRed();
  extern void gatesDown();
  extern void motorEnable();
  extern void motorDirDown();
  extern void motorCruiseDown();
  extern void motorStop();
  extern bool bottomLimitPressed();

  static bool s_inited = false;

  inline void enter() {
    s_inited = true;
    roadRed(); marineRed(); gatesDown();
    motorEnable(); motorDirDown(); motorCruiseDown();
  }

  inline Result tick() {
    if (!s_inited) enter();

    if (bottomLimitPressed()) {
      motorStop();
      return Result::TO_DOWN;
    }
    return Result::STAY;
  }
}