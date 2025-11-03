#include "esp32-hal-gpio.h"
#include "StateMachine.h"
#include "main.h"
#include "TrafficLight.h"
#include "SonicSensor.h"
#include "DCMotor.h"


// Timers
unsigned long startTime = 0;
const unsigned long lowerDelay = 10000; // wait before next action
const unsigned long yellowDelay = 5000;
const int boatcount_required = 4;
const int detection_distance = 30;
const int seabed_distance = 20;
int boatDetection_count = 0;
extern bool EStop;
bool status_on = false;

unsigned long slowthefuckdown = 0;
const unsigned long delaytimebitch = 550;







// Helper functions
void flash(int pin)    { status_on = !status_on; digitalWrite(pin, status_on); }
bool blink()           { if (millis() - slowthefuckdown < delaytimebitch) {return false;} else {slowthefuckdown = millis(); return true;} }
void bottomInterrupt() { detachInterrupt(digitalPinToInterrupt(Pin_LS_Top));     attachInterrupt(digitalPinToInterrupt(Pin_LS_Bottom), disableMotor, FALLING);}
void topInterrupt()    { detachInterrupt(digitalPinToInterrupt(Pin_LS_Bottom));  attachInterrupt(digitalPinToInterrupt(Pin_LS_Top), disableMotor, FALLING);}
bool eStopPressed()    { bool temp = !digitalRead(Pin_EStop); return temp;}
bool topLimitHit()     { bool temp = !digitalRead(Pin_LS_Top); return temp;}
bool bottomLimitHit()  { bool temp = !digitalRead(Pin_LS_Bottom);  return temp;}
bool boatDetected()    { 
  double dist1 = sonic1.poll_cm(); 
  double dist2 = sonic2.poll_cm(); 
  Serial.print("Dist: " + (String)dist1 + " - " + (String)dist2); 
  return dist1 < detection_distance || dist2 < detection_distance; }     // boat nearby
bool timerFinished(long delay)   { /*Serial.println(millis());*/ /*Serial.println(startTime);*/ return (millis() - startTime) > delay; }
bool timerUp()         { return millis() - startTime > 8000;}
void startMotorUp()    { motor.run(64, 1); Serial.println("Motor UP started."); }
void startMotorDown()  { motor.run(64, 0); Serial.println("Motor DOWN started."); }
void stopMotor()       { motor.disable(); Serial.println("Motor stopped."); }


// Main state machine logic

void stateMachine(bridgeState state) {
  // Serial.println(state);
  // EStop = eStopPressed();
  // if (EStop) {
  //   switch (state) {
  //     case lowered:
  //     case prepareRaise:
  //     case raising:
  //       state = emergencyLower;
          //  bottomInterrupt();
  //       break;
  //     case raised:
  //     case prepareLower:
  //     case lowering:
  //       state = emergencyRaise;
          //  topInterrupt();
  //       break;
  //     default:
  //       break;
  //   }
  //   EStop = false;
  // }
  
  switch (state) {
    //  DOWN (lowered) 
    case lowered:
      digitalWrite(Pin_Status, LOW);
      digitalWrite(Pin_Buzzer, LOW);
      traffic.cycle(2); //2 = Green
      // Serial.println("STATE: DOWN - Road Traffic Accepted");
      if (boatDetected()) {
        boatDetection_count++;
      }
      if (boatDetection_count >= boatcount_required) {
        // Serial.println("Boat detected → PREPARE TO RAISE");
        boatDetection_count = 0;
        currentState = prepareRaise;
        Serial.println("STATE: PREP TO RAISE — waiting before lifting");
        startTime = millis();
      }
      break;

    // === PREP TO RAISE ===
    case prepareRaise:
      digitalWrite(Pin_Status, HIGH);
      if (blink())flash(Pin_Buzzer);
      traffic.cycle(1);
      // Serial.println("STATE: PREP TO RAISE — waiting before lifting");
      
      Serial.println(timerFinished(yellowDelay));
      if (timerFinished(yellowDelay)) {
        topInterrupt();
        // Serial.println("Prep timer done → RAISING");
        Serial.println("Bridge fully raised → UP");
        currentState = raising;
        startMotorUp();
      }
      break;

    // === RAISING ===
    case raising:
      digitalWrite(Pin_Buzzer, LOW);
      if (blink())flash(Pin_Status);
      traffic.cycle(0);
      // Serial.println("STATE: RAISING");
      
      startTime = millis();
      if (topLimitHit()) {
        stopMotor();
        Serial.println("STATE: UP (bridge up for boats)");

        // Serial.println("Bridge fully raised → UP");
        currentState = raised;
        startTime = millis();
      }
      break;

    // === UP (raised) ===
    case raised:
      digitalWrite(Pin_Status, LOW);
      digitalWrite(Pin_Buzzer, LOW);
      // Serial.println("STATE: UP (bridge up for boats)");
      traffic.cycle(0);
      if (timerUp()) {
        // Serial.println("Boat clear → PREPARE TO LOWER");
        currentState = prepareLower;
      Serial.println("STATE: PREP TO LOWER");

        startTime = millis();
      }
      break;

    // === PREP TO LOWER ===
    case prepareLower:
      digitalWrite(Pin_Status, HIGH);
      if (blink())flash(Pin_Buzzer);
      // Serial.println("STATE: PREP TO LOWER");
      traffic.cycle(0);
      if (timerFinished(lowerDelay)) {
        bottomInterrupt();
        // Serial.println("Prep timer done → LOWERING");
        currentState = lowering;
        startMotorDown();
        startTime = millis();
      Serial.println("STATE: LOWERING");

      }
      break;

    // === LOWERING ===
    case lowering:
      // Serial.println("STATE: LOWERING");
      digitalWrite(Pin_Buzzer, LOW);
      if (blink())flash(Pin_Status);
      
      if (bottomLimitHit()) {
        stopMotor();
        // Serial.println("Bridge fully lowered → DOWN");
        traffic.cycle(2);
        currentState = lowered;
        startTime = millis();
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
