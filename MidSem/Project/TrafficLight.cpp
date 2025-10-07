#include "TrafficLight.h"

TrafficModule::TrafficModule(uint8_t red, uint8_t yellow, uint8_t green) {
    redPin = red;
    yellowPin = yellow;
    greenPin = green;
    currentState = 0;
}

void TrafficModule::init() {
    pinMode(redPin, OUTPUT);
    pinMode(yellowPin, OUTPUT);
    pinMode(greenPin, OUTPUT);
}

void TrafficModule::cycle() {
    currentState = (currentState==2) ? 0 : currentState++ ;
}