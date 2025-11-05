#include "esp32-hal-gpio.h"
#include "StateMachine.h"
#include "main.h"
#include "TrafficLight.h"
#include "SonicSensor.h"
#include "DCMotor.h"


// Timers
unsigned long startTime = 0;
const unsigned long lowerDelay = 8000; // wait before next action
const unsigned long yellowDelay = 12000;
const unsigned long raiseDelay = 3000;
const int boatcount_required = 4;

const int seabed_distance = 20;
int boatDetection_count = 0;
extern bool EStop;
bool status_on = false;

unsigned long blinkDelayStart = 0;
const unsigned long blinkDelayDuration = 550;
bool blinkState = false;







// Helper functions
void flash(int pin)    { digitalWrite(pin, blinkState); }
void blink()           { if ((millis() - blinkDelayStart) >= blinkDelayDuration) { blinkDelayStart = millis(); blinkState=!blinkState; } }
void bottomInterrupt() { detachInterrupt(digitalPinToInterrupt(Pin_LS_Top));     attachInterrupt(digitalPinToInterrupt(Pin_LS_Bottom), disableMotor, FALLING);}
void topInterrupt()    { detachInterrupt(digitalPinToInterrupt(Pin_LS_Bottom));  attachInterrupt(digitalPinToInterrupt(Pin_LS_Top), disableMotor, FALLING);}
bool eStopPressed()    { bool temp = !digitalRead(Pin_EStop); return temp;}
bool topLimitHit()     { bool temp = !digitalRead(Pin_LS_Top); return temp;}
bool bottomLimitHit()  { bool temp = !digitalRead(Pin_LS_Bottom);  return temp;}
bool boatDetected()    { return ultrasonics; }  // boat nearby
bool timerFinished(long delay)   { /*Serial.println(millis());*/ /*Serial.println(startTime);*/ return (millis() - startTime) > delay; }
bool timerUp()         { return millis() - startTime > 8000;}
void startMotorUp()    { motor.run(64, 1); Serial.println("Motor UP started."); }
void startMotorDown()  { motor.run(64, 0); Serial.println("Motor DOWN started."); }
void stopMotor()       { motor.disable(); Serial.println("Motor stopped."); }


void stateMachine(bridgeState state) {
  // if (EStop) {
  //   switch (state) {
  //     case lowered: case prepareRaise: case raising:
  //       state = emergencyLower;
  //       bottomInterrupt();
  //       break;
  //     case raised: case prepareLower: case lowering:
  //       state = emergencyRaise;
  //       topInterrupt();
  //       break;
  //     default:
  //       break;
  //   }
  //   EStop = false;
  // }
  switch (state) {
    case lowered:
      digitalWrite(Pin_Status, LOW);
      digitalWrite(Pin_Buzzer, LOW);
      digitalWrite(Pin_BoatLight, HIGH);
      traffic.cycle(2); //2 = Green
      if (boatDetected()) { boatDetection_count++; }
      if (boatDetection_count >= boatcount_required) {
        Serial.println("Boat detected → PREPARE TO RAISE");
        boatDetection_count = 0;
        currentState = prepareRaise;
        Serial.println("STATE: PREP TO RAISE — waiting before lifting");
        startTime = millis();
      }
      break;
    case prepareRaise:
      digitalWrite(Pin_BoatLight, HIGH);
      blink();
      flash(Pin_Buzzer);
      flash(Pin_Status);
      traffic.cycle(1);
      if (timerFinished(yellowDelay)) {
        topInterrupt();
        Serial.println("Prep timer done → RAISING");
        currentState = raising;
        startMotorUp();
      }
      break;
    case raising:
      digitalWrite(Pin_BoatLight, HIGH);
      digitalWrite(Pin_Status, HIGH);
      blink();
      flash(Pin_Buzzer);
      traffic.cycle(0);
      startTime = millis();
      if (topLimitHit()) {
        stopMotor();
        Serial.println("STATE: UP (bridge up for boats)");
        currentState = raised;
        startTime = millis();
      }
      break;
    case raised:
      digitalWrite(Pin_Status, LOW);
      digitalWrite(Pin_Buzzer, LOW);
      digitalWrite(Pin_BoatLight, LOW);
      traffic.cycle(0);
      if (timerUp()) {
        currentState = prepareLower;
        Serial.println("STATE: PREP TO LOWER");
        startTime = millis();
      }
      break;
    case prepareLower:
      digitalWrite(Pin_Status, HIGH);
      blink();
      flash(Pin_Buzzer); 
      flash(Pin_BoatLight);
      traffic.cycle(0);
      if (timerFinished(lowerDelay)) {
        bottomInterrupt();
        currentState = lowering;
        startMotorDown();
        startTime = millis();
        Serial.println("STATE: LOWERING");
      }
      break;
    case lowering:
      digitalWrite(Pin_BoatLight, HIGH);
      digitalWrite(Pin_Status, HIGH);
      blink();
      flash(Pin_Buzzer); 
      if (bottomLimitHit()) {
        stopMotor();
        Serial.println("Bridge fully lowered → DOWN");
        traffic.cycle(2);
        currentState = lowered;
        startTime = millis();
      }
      break;
    case emergencyLower:
      Serial.println("EMERGENCY LOWER TRIGGERED!");
      delay(2000);
      stopMotor();
      currentState = lowered;
      break;
    case emergencyRaise:
      Serial.println("EMERGENCY RAISE TRIGGERED!");
      delay(2000);
      stopMotor();
      currentState = raised;
      break;
    default: break;
  }
}
