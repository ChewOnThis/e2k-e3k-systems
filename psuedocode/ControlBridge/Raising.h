#pragma once
 
#include "config.h"

namespace Raising {
  enum class Result { STAY, TO_UP, TO_EMERGENCY_LOWER };

  // extern helpers implemented in ControlBridge.ino
  extern void roadRed();
  extern void marineRed();
  extern void gatesDown();
  extern void motorEnable();
  extern void motorDirUp();
  extern void motorCruiseUp();
  extern void motorSlowUp();
  extern void motorStop();
  extern bool eStop();
  extern bool carOnBridge();
  extern bool cancelRequested();
  extern bool topLimitPressed();

  // private state (header-only)
  static bool     s_inited  = false;
  static bool     s_decel   = false;
  static uint32_t s_tStart  = 0;

  inline void enter() {
    s_inited = true; s_decel = false; s_tStart = millis();
    roadRed();  marineRed(); gatesDown();
    motorEnable(); motorDirUp(); motorCruiseUp();
  }

  inline Result tick() {
    if (!s_inited) enter();

    if (eStop() || cancelRequested() || carOnBridge()) {
      motorStop(); return Result::TO_EMERGENCY_LOWER;
    }

    const uint32_t el = millis() - s_tStart;

    if (!s_decel && el >= T_OPEN_EXPECTED) { s_decel = true; motorSlowUp(); }
    else { s_decel ? motorSlowUp() : motorCruiseUp(); }

    if (eStop()) { motorStop(); return Result::TO_EMERGENCY_LOWER; }

    if (topLimitPressed()) { motorStop(); return Result::TO_UP; }

    if (el >= T_OPEN_MAX) { motorStop(); return Result::TO_EMERGENCY_LOWER; }

    return Result::STAY;
  }
} // namespace Raising
