#pragma once
#include <Arduino.h>

// Low-level hardware is supplied by Integration.ino
using MotorVoid = void (*)();
using MotorPWM  = void (*)(int duty255);

class Motor {
public:
  void bind(MotorVoid en, MotorVoid dis, MotorVoid up, MotorVoid down, MotorPWM write) {
    _en=en; _dis=dis; _up=up; _down=down; _write=write;
  }
  void enable()  { if (_en)  _en();  }
  void disable() { if (_dis) _dis(); }
  void dirUp()   { if (_up)  _up();  }
  void dirDown() { if (_down)_down();}
  void setSpeed(int duty255) { if (_write) _write(duty255); _last=duty255; }
  void stop() { setSpeed(0); }
  int  last() const { return _last; }
private:
  MotorVoid _en=nullptr,_dis=nullptr,_up=nullptr,_down=nullptr; MotorPWM _write=nullptr;
  int _last=0;
};
