#include "esp32-hal-gpio.h"
#include "DCMotor.h"

Motor::Motor(uint8_t drive, uint8_t dir, uint8_t en) {
    DrivePin = drive;
    DirectionPin = dir;
    EnablePin = en;
}

void Motor::init() {
    pinMode(DrivePin, OUTPUT);
    pinMode(DirectionPin, OUTPUT);
    pinMode(EnablePin, OUTPUT);
    digitalWrite(EnablePin, LOW);
    analogWrite(DrivePin, 64);
    
}

void Motor::disable()
{
    // Serial.println("Start disable");
    digitalWrite(EnablePin, LOW);
    // Serial.println("Done Disable");
}

void Motor::run (int pwm, int direction) {
    digitalWrite( DirectionPin , ((direction==1)?HIGH:LOW) );
    digitalWrite( EnablePin, HIGH );
}