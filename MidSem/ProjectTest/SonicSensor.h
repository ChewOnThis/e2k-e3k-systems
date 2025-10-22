#ifndef SONICSENSOR_H
#define SONICSENSOR_H

#include <Arduino.h>

class Sonic {
    private:
        int trigPin;
        int echoPin;
    public:
        Sonic(int trig, int echo);
        void init();
        double pollRaw();
        double poll_cm();
};

#endif