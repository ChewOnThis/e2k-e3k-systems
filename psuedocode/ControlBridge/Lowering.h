#pragma once
 
#include "config.h"

namespace Lowering {
  enum class Result { STAY, TO_DOWN, TO_EMERGENCY_RAISE };

  // extern helpers implemented in ControlBridge.ino
  extern void roadRed();
  extern void marineRed();
  extern void gatesDown();
  extern void motorEnable();
  extern void motorDirDown();
  extern void motorCruiseDown();
  extern void motorSlowDown();
  extern void motorStop();
  extern bool eStop();
  extern bool cancelRequested();
  extern bool boatUnderSpan();
  extern bool bottomLimitPressed();

  // private state (header only)
  static bool     s_inited  = false;
  static bool     s_decel   = false;
  static uint32_t s_tStart  = 0;

  inline void enter() {
    s_inited = true; s_decel = false; s_tStart = millis();
    roadRed();  marineRed(); gatesDown();
    motorEnable(); motorDirDown(); motorCruiseDown();
  }

  inline Result tick() {
    if (!s_inited) enter();

    // Abort or boat detected while closing -> Emergency Raise
    if (eStop() || cancelRequested() || boatUnderSpan()) {
      motorStop(); return Result::TO_EMERGENCY_RAISE;
    }

    const uint32_t el = millis() - s_tStart;

    // Decelerate near the bottom
    if (!s_decel && el >= T_CLOSE_EXPECTED) { s_decel = true; motorSlowDown(); }
    else { s_decel ? motorSlowDown() : motorCruiseDown(); }

    if (bottomLimitPressed()) { motorStop(); return Result::TO_DOWN; }

    if (el >= T_CLOSE_MAX) { motorStop(); return Result::TO_EMERGENCY_RAISE; }

    return Result::STAY;
  }
} // namespace Lowering
