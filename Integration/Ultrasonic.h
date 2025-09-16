#pragma once
#include <Arduino.h>

class Ultrasonic {
public:
  Ultrasonic(int trig, int echo) : _trig(trig), _echo(echo) {}
  void begin() { pinMode(_trig, OUTPUT); pinMode(_echo, INPUT); digitalWrite(_trig, LOW); }
  long distanceMM() {
    // very light pseudo: you will replace with a robust median filter
    digitalWrite(_trig, LOW); delayMicroseconds(3);
    digitalWrite(_trig, HIGH); delayMicroseconds(10);
    digitalWrite(_trig, LOW);
    unsigned long us = pulseIn(_echo, HIGH, 30000); // 30ms timeout
    return (long)(us * 0.343 / 2.0); // mm at ~20°C
  }
private:
  int _trig,_echo;
};
