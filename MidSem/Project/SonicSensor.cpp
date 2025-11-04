#include "SonicSensor.h"

Sonic::Sonic(int trig, int echo) {
    trigPin = trig;
    echoPin = echo;
}

void Sonic::init() {
    pinMode(trigPin, OUTPUT);
    pinMode(echoPin, INPUT);
}

double Sonic::pollRaw() {
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
    return pulseIn(echoPin, HIGH);
}

double Sonic::poll_cm() {
    return pollRaw() * 0.0343 / 2.0;
}