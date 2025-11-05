#ifndef SONICSENSOR_H
#define SONICSENSOR_H

#include <Arduino.h>

class Sonic {
    public:
        int trigPin;
        int echoPin;
        Sonic(int trig, int echo);
        void init();
        double pollRaw();
        double poll_cm();
};

#endif