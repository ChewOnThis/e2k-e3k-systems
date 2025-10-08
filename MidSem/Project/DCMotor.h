#ifndef DCMOTOR_H
#define DCMOTOR_H

#include <Arduino.h>

class Motor {
    private:
        uint8_t DrivePin;
        uint8_t DirectionPin;
    public:
        Motor(uint8_t drive, uint8_t dir);
        void init();
        void disable();
        void run(int pwm, int direction);
};

#endif