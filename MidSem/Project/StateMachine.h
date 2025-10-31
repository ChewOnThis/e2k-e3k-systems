#ifndef STATEMACHINE_H
#define STATEMACHINE_H

#include <Arduino.h>

enum bridgeState : uint8_t {
    lowered = 1,
    prepareRaise = 2,
    raising = 3,
    raised = 4,
    prepareLower = 5,
    lowering = 6,
    emergencyLower = 7,
    emergencyRaise = 8
};

extern bridgeState currentState;
void stateMachine(bridgeState state);

bool eStopPressed();
bool topLimitHit();
bool bottomLimitHit();
bool boatDetected();
bool timerFinished();
bool timerUp();
void startMotorUp();
void startMotorDown();
void stopMotor();



#endif