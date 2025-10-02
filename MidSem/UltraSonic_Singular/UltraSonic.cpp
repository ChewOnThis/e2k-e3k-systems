#include "UltraSonic.h"

UltraSonic::UltraSonic(int trig, int echo) {
    trigPin = trig;
    echoPin = echo;
}

void init() {
    pinMode(trigPin, OUTPUT);
    pinMode(echoPin, INPUT);
    attachInterrupt(digitalPinToInterrupt(echoPinA))
}
double pollRaw() {
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
    return pulseIn(echoPin, HIGH);
}
double poll_cm() {
    return (pollRaw()*0.0343)/2.0;
}