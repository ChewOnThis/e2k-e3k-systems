#include "StateMachine.h"
#include "main.h"
#include "TrafficLight.h"
#include "SonicSensor.h"
#include "DCMotor.h"


// Timers
unsigned long startTime = 0;
const unsigned long actionDelay = 3000; // wait before next action
int boatDetection_count = 0;
extern bool EStop;



// Helper functions

bool eStopPressed()   { bool temp = !digitalRead(Pin_EStop); Serial.println("EStop" + (String)temp); return temp;}
bool topLimitHit()    { bool temp = !digitalRead(Pin_LS_Top); Serial.println("Top" + (String)temp); return temp;}
bool bottomLimitHit() { bool temp = !digitalRead(Pin_LS_Top); Serial.println("Bottom" + (String)temp); return temp;}
bool boatDetected()   { return sonic1.poll_cm() < 100 && sonic2.poll_cm() < 100; }     // boat nearby
bool timerFinished()  { Serial.println(millis()); Serial.println(startTime); return (millis() - startTime) > actionDelay; }
bool timerUp()        { return millis() - startTime > 8000;}
void startMotorUp()   { motor.run(64, 1); Serial.println("Motor UP started."); }
void startMotorDown() { motor.run(64, 0); Serial.println("Motor DOWN started."); }
void stopMotor()      { motor.disable(); Serial.println("Motor stopped."); }


// Main state machine logic

void stateMachine(bridgeState state) {
  Serial.println(state);
  // EStop = eStopPressed();
  // if (EStop) {
  //   switch (state) {
  //     case lowered:
  //     case prepareRaise:
  //     case raising:
  //       state = emergencyLower;
  //       break;
  //     case raised:
  //     case prepareLower:
  //     case lowering:
  //       state = emergencyRaise;
  //       break;
  //     default:
  //       break;
  //   }
  //   EStop = false;
  // }
  
  switch (state) {
    //  DOWN (lowered) 
    case lowered:
      traffic.cycle(2); //2 = Green
      Serial.println("STATE: DOWN - Road Traffic Accepted");
      if (boatDetected()) {
        boatDetection_count++;
      }
      if (boatDetection_count >= 2) {
        Serial.println("Boat detected → PREPARE TO RAISE");
        currentState = prepareRaise;
        startTime = millis();
      }
      break;

    // === PREP TO RAISE ===
    case prepareRaise:
      traffic.cycle(1);
      Serial.println("STATE: PREP TO RAISE — waiting before lifting");
      
      Serial.println(timerFinished());
      if (timerFinished()) {
        Serial.println("Prep timer done → RAISING");
        currentState = raising;
        startMotorUp();
      }
      break;

    // === RAISING ===
    case raising:
      traffic.cycle(0);
      Serial.println("STATE: RAISING");
      
      startTime = millis();
      if (topLimitHit()) {
        stopMotor();
        Serial.println("Bridge fully raised → UP");
        currentState = raised;
        startTime = millis();
      }
      break;

    // === UP (raised) ===
    case raised:
      Serial.println("STATE: UP (bridge up for boats)");
      traffic.cycle(0);
      if (timerUp()) {
        Serial.println("Boat clear → PREPARE TO LOWER");
        currentState = prepareLower;
      }
      break;

    // === PREP TO LOWER ===
    case prepareLower:
      Serial.println("STATE: PREP TO LOWER");
      traffic.cycle(0);
      
      startTime = millis();

      if (timerFinished()) {
        Serial.println("Prep timer done → LOWERING");
        currentState = lowering;
        startMotorDown();
      }
      break;

    // === LOWERING ===
    case lowering:
      Serial.println("STATE: LOWERING");
      
      startTime = millis();
      if (bottomLimitHit() || (millis() - startTime > 1000 && topLimitHit())) {
        stopMotor();
        Serial.println("Bridge fully lowered → DOWN");
        traffic.cycle(2);
        currentState = lowered;
      }
      break;

    // === EMERGENCY LOWER ===
    case emergencyLower:
      Serial.println("EMERGENCY LOWER TRIGGERED!");
      // startMotorDown();
      delay(2000);
      stopMotor();
      currentState = lowered;
      break;

    // === EMERGENCY RAISE ===
    case emergencyRaise:
      Serial.println("EMERGENCY RAISE TRIGGERED!");
      // startMotorUp();
      delay(2000);
      stopMotor();
      currentState = raised;
      break;

    default:
      break;
  }
}
