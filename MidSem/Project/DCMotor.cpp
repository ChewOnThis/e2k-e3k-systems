#include "DCMotor.h"

Motor::Motor(uint8_t drive, uint8_t dir) {
    DrivePin = drive;
    DirectionPin = dir;
}

void Motor::init() {
    pinMode(DrivePin, OUTPUT);
    pinMode(DirectionPin, OUTPUT);
}

void Motor::run (int pwm, int direction) {
    digitalWrite( DirectionPin , ((direction==1)?HIGH:LOW) );
    analogWrite( DrivePin, pwm );
}