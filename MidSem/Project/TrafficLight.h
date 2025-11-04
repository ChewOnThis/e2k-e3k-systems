#ifndef TRAFFICLIGHT_H
#define TRAFFICLIGHT_H

#include <Arduino.h>

class TrafficModule{
    private:
        uint8_t redPin;
        uint8_t yellowPin;
        uint8_t greenPin;
        uint8_t currentState;
    public:
        TrafficModule(uint8_t red, uint8_t yellow, uint8_t green);
        void init();
        void updateLights();
        void cycle(int check);
        void setRed(bool);
        void setGreen(bool);
        void setBlue(bool);
        uint8_t getCurrent();
};

#endif