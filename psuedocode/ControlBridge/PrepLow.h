#pragma once
 
#include "config.h"

namespace PrepLow {
  enum class Result { STAY, TO_LOWERING, TO_UP };

  // extern helpers
  extern void marineRed();
  extern bool boatUnderSpan();
  extern bool boatWaiting();

  static bool s_inited = false;

  inline void enter() {
    s_inited = true;
    marineRed();
  }

  inline Result tick() {
    if (!s_inited) enter();

    if (boatUnderSpan() || boatWaiting()) {
      return Result::TO_UP;
    }
    return Result::TO_LOWERING;
  }
}