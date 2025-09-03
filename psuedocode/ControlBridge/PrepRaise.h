#pragma once
 
#include "config.h"

namespace PrepRaise {
  enum class Result { STAY, TO_RAISING, TO_EMERGENCY_LOWER };

  // extern helpers
  extern void roadYellow();
  extern void marineRed();
  extern void gatesDown();
  extern void roadRed();
  extern bool carOnBridge();
  extern bool eStop();

  // private state
  static bool     s_inited  = false;
  static uint32_t s_tStart  = 0;

  inline void enter() {
    s_inited = true; s_tStart = millis();
    roadYellow(); marineRed(); gatesDown();
  }

  inline Result tick() {
    if (!s_inited) enter();

    if (carOnBridge() || eStop()) {
      return Result::TO_EMERGENCY_LOWER;
    }
    if (millis() - s_tStart >= T_YELLOW) {
      roadRed();
      return Result::TO_RAISING;
    }
    return Result::STAY;
  }
}