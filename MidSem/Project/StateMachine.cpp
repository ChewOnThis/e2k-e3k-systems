#include "StateMachine.h"
#include "main.h"
#include "TrafficLight.h"
#include "SonicSensor.h"
#include "DCMotor.h"

// External modules (from other files)
extern TrafficModule trafficLight;
extern Sonic sonicSensor;
extern Motor motor;

//  starting position
bridgeState currentState = lowered;

// Timers
unsigned long startTime = 0;
const unsigned long actionDelay = 3000; // wait before next action
const unsigned long moveTimeout = 8000; // safety limit for motion


// Helper functions

bool eStopPressed()   { return digitalRead(Pin_EStop) == HIGH; }
bool topLimitHit()    { return digitalRead(Pin_LS_1) == HIGH; }
bool bottomLimitHit() { return digitalRead(Pin_LS_2) == HIGH; }
bool boatDetected()   { return sonicSensor.poll_cm() < 100; }     // boat nearby
bool areaClear()      { return sonicSensor.poll_cm() > 150; }     // no boat
bool timerFinished()  { return millis() - startTime > actionDelay; }
bool motionTimeout()  { return millis() - startTime > moveTimeout; }

void startMotorUp()   { Serial.println("Motor UP started (simulated)."); motor.run(255, 1); }
void startMotorDown() { Serial.println("Motor DOWN started (simulated)."); motor.run(255, 0); }
void stopMotor()      { Serial.println("Motor stopped."); motor.disable(); }


// Main state machine logic

void stateMachine(bridgeState state) {
  switch (state) {

    //  DOWN (lowered) 
    case lowered:
      trafficLight.cycle(2);
      Serial.println("STATE: DOWN (bridge open for cars)");

      if (boatDetected()) {
        Serial.println("Boat detected → PREPARE TO RAISE");
        currentState = prepareRaise;
      }
      break;

    // === PREP TO RAISE ===
    case prepareRaise:
      TrafficLight.cycle(0);

      Serial.println("STATE: PREP TO RAISE — waiting before lifting");
      startTime = millis();

      if (eStopPressed()) {
        Serial.println("E-Stop! → EMERGENCY LOWER");
        currentState = emergencyLower;
      } 
      else if (timerFinished()) {
        Serial.println("Prep timer done → RAISING");
        currentState = raising;
      }
      break;

    // === RAISING ===
    case raising:
      Serial.println("STATE: RAISING");
      startMotorUp();
      startTime = millis();

      if (eStopPressed()) {
        Serial.println("⚠️ E-STOP → EMERGENCY LOWER");
        currentState = emergencyLower;
      } 
      else if (topLimitHit() || motionTimeout()) {
        stopMotor();
        Serial.println("Bridge fully raised → UP");
        currentState = raised;
      }
      break;

    // === UP (raised) ===
    case raised:
      Serial.println("STATE: UP (bridge up for boats)");
      TrafficLight.cycle(0);


      if (areaClear()) {
        Serial.println("Boat clear → PREPARE TO LOWER");
        currentState = prepareLower;
      }
      break;

    // === PREP TO LOWER ===
    case prepareLower:
      Serial.println("STATE: PREP TO LOWER");
      TrafficLight.cycle(0);
      
      startTime = millis();

      if (eStopPressed()) {
        Serial.println("E-Stop! → EMERGENCY RAISE");
        currentState = emergencyRaise;
      } 
      else if (timerFinished()) {
        Serial.println("Prep timer done → LOWERING");
        currentState = lowering;
      }
      break;

    // === LOWERING ===
    case lowering:
      Serial.println("STATE: LOWERING");
      startMotorDown();
      startTime = millis();

      if (eStopPressed()) {
        Serial.println("⚠️ E-STOP → EMERGENCY RAISE");
        currentState = emergencyRaise;
      } 
      else if (bottomLimitHit() || motionTimeout()) {
        stopMotor();
        Serial.println("Bridge fully lowered → DOWN");
        TrafficLight.cycle(2);
        
        currentState = lowered;
      }
      break;

    // === EMERGENCY LOWER ===
    case emergencyLower:
      Serial.println("⚠️ EMERGENCY LOWER TRIGGERED!");
      startMotorDown();
      delay(2000);
      stopMotor();
      currentState = lowered;
      break;

    // === EMERGENCY RAISE ===
    case emergencyRaise:
      Serial.println("⚠️ EMERGENCY RAISE TRIGGERED!");
      startMotorUp();
      delay(2000);
      stopMotor();
      currentState = raised;
      break;

    default:
      break;
  }
}
