#include "DCMotor.h"

Motor::Motor(uint8_t drive, uint8_t dir) {
    DrivePin = drive;
    DirectionPin = dir;
}

void Motor::init() {
    pinMode(DrivePin, OUTPUT);
    pinMode(DirectionPin, OUTPUT);
}

void Motor::disable()
{
    digitalWrite(DrivePin, LOW);
    digitalWrite(DirectionPin, LOW);
}

void Motor::run (int pwm, int direction) {
    digitalWrite( DirectionPin , ((direction==1)?HIGH:LOW) );
    analogWrite( DrivePin, pwm );
}