#pragma once
#include <Arduino.h>

class LimitSwitch {
public:
  LimitSwitch(int pinTop, int pinBottom, int pinEstop) :
    _top(pinTop), _bot(pinBottom), _est(pinEstop) {}

  void begin() {
    pinMode(_top, INPUT_PULLUP);
    pinMode(_bot, INPUT_PULLUP);
    pinMode(_est, INPUT_PULLUP);
  }
  bool topPressed()    const { return digitalRead(_top)==LOW; }
  bool bottomPressed() const { return digitalRead(_bot)==LOW; }
  bool eStop()         const { return digitalRead(_est)==LOW; }

private:
  int _top,_bot,_est;
};
