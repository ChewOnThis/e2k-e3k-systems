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

void stateMachine(bridgeState state);
void stateCycle();

#endif