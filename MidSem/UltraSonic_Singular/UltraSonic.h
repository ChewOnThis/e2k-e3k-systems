#ifndef ULTRASONIC_H
#define ULTRASONCIC_H
#include <Arduino.h>

class UltraSonic {
    private:
        int trigPin;
        int echoPin;

    public:
        UltraSonic(int trig, int echo);
        void init();
        double pollRaw();
        double poll_cm();
};

#endif