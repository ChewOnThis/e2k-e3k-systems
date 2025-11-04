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

void TrafficModule::updateLights() {
    switch (currentState) {
        case 0:
            digitalWrite(redPin, HIGH);
            digitalWrite(yellowPin, LOW);
            digitalWrite(greenPin, LOW);
            break;
        case 1:
            digitalWrite(redPin, LOW);
            digitalWrite(yellowPin, HIGH);
            digitalWrite(greenPin, LOW);
            break;
        case 2:
            digitalWrite(redPin, LOW);
            digitalWrite(yellowPin, LOW);
            digitalWrite(greenPin, HIGH);
            break;
    }
}
uint8_t TrafficModule::getCurrent() {
    return currentState;
}



void TrafficModule::cycle(int check) {
    currentState = (currentState++)%2 ;
    if (currentState != check){
        currentState = check;
    }
    updateLights();
}