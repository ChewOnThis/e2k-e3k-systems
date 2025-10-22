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
bridgeState previousState = lowered;

// Timers
unsigned long startTime = 0;
const unsigned long actionDelay = 3000; // wait before next action
const unsigned long moveTimeout = 8000; // safety limit for motion

// Debug configuration
const unsigned long debugLogInterval = 1000; // Log state info every second when in motion


// Helper functions

bool eStopPressed()   { return digitalRead(Pin_EStop) == HIGH; }
bool topLimitHit()    { return digitalRead(Pin_LS_1) == HIGH; }
bool bottomLimitHit() { return digitalRead(Pin_LS_2) == HIGH; }
bool boatDetected()   { return sonicSensor.poll_cm() < 100; }     // boat nearby
bool areaClear()      { return sonicSensor.poll_cm() > 150; }     // no boat
bool timerFinished()  { return millis() - startTime > actionDelay; }
bool motionTimeout()  { return millis() - startTime > moveTimeout; }

void startMotorUp()   { 
  debugLog("Motor UP started"); 
  motor.run(255, 1); 
}

void startMotorDown() { 
  debugLog("Motor DOWN started"); 
  motor.run(255, 0); 
}

void stopMotor()      { 
  debugLog("Motor stopped"); 
  motor.disable(); 
}


// Main state machine logic

void stateMachine(bridgeState state) {
  // Log state change if different from previous state
  if (state != previousState) {
    debugLogStateChange(state);
    previousState = state;
  }

  switch (state) {

    //  DOWN (lowered) 
    case lowered:
      trafficLight.cycle(2);
      
      if (boatDetected()) {
        debugLog("Boat detected, preparing to raise bridge");
        currentState = prepareRaise;
      }
      break;

    // === PREP TO RAISE ===
    case prepareRaise:
      trafficLight.cycle(0);
      
      // Initialize timer on first entry to this state
      static bool prepRaiseInitialized = false;
      if (!prepRaiseInitialized) {
        startTime = millis();
        debugLogSensors();
        prepRaiseInitialized = true;
      }

      if (eStopPressed()) {
        debugLog("Emergency stop pressed during prep to raise");
        prepRaiseInitialized = false;
        currentState = emergencyLower;
      } 
      else if (timerFinished()) {
        debugLog("Preparation timer completed, starting to raise bridge");
        prepRaiseInitialized = false;
        currentState = raising;
      }
      break;

    // === RAISING ===
    case raising:
      // Initialize motor on first entry to this state
      static bool raisingInitialized = false;
      if (!raisingInitialized) {
        startMotorUp();
        startTime = millis();
        raisingInitialized = true;
      }

      if (eStopPressed()) {
        debugLog("EMERGENCY: E-stop pressed while raising bridge");
        raisingInitialized = false;
        currentState = emergencyLower;
      } 
      else if (topLimitHit() || motionTimeout()) {
        stopMotor();
        debugLog(topLimitHit() ? "Top limit switch reached" : "Motion timeout reached");
        raisingInitialized = false;
        currentState = raised;
      }
      break;

    // === UP (raised) ===
    case raised:
      trafficLight.cycle(0);

      if (areaClear()) {
        debugLog("Area clear detected, preparing to lower bridge");
        currentState = prepareLower;
      }
      break;

    // === PREP TO LOWER ===
    case prepareLower:
      trafficLight.cycle(0);
      
      // Initialize timer on first entry to this state
      static bool prepLowerInitialized = false;
      if (!prepLowerInitialized) {
        startTime = millis();
        debugLogSensors();
        prepLowerInitialized = true;
      }

      if (eStopPressed()) {
        debugLog("Emergency stop pressed during prep to lower");
        prepLowerInitialized = false;
        currentState = emergencyRaise;
      } 
      else if (timerFinished()) {
        debugLog("Preparation timer completed, starting to lower bridge");
        prepLowerInitialized = false;
        currentState = lowering;
      }
      break;

    // === LOWERING ===
    case lowering:
      // Initialize motor on first entry to this state
      static bool loweringInitialized = false;
      if (!loweringInitialized) {
        startMotorDown();
        startTime = millis();
        loweringInitialized = true;
      }

      if (eStopPressed()) {
        debugLog("EMERGENCY: E-stop pressed while lowering bridge");
        loweringInitialized = false;
        currentState = emergencyRaise;
      } 
      else if (bottomLimitHit() || motionTimeout()) {
        stopMotor();
        debugLog(bottomLimitHit() ? "Bottom limit switch reached" : "Motion timeout reached");
        trafficLight.cycle(2);
        loweringInitialized = false;
        currentState = lowered;
      }
      break;

    // === EMERGENCY LOWER ===
    case emergencyLower:
      debugLog("EMERGENCY PROCEDURE: Lowering bridge immediately");
      debugLogSensors();
      startMotorDown();
      delay(2000);
      stopMotor();
      debugLog("Emergency lower completed");
      currentState = lowered;
      break;

    // === EMERGENCY RAISE ===
    case emergencyRaise:
      debugLog("EMERGENCY PROCEDURE: Raising bridge immediately");
      debugLogSensors();
      startMotorUp();
      delay(2000);
      stopMotor();
      debugLog("Emergency raise completed");
      currentState = raised;
      break;

    default:
      break;
  }
}

// Helper function to get state name as string
const char* getStateName(bridgeState state) {
  const char* stateNames[] = {
    "UNKNOWN", "LOWERED", "PREPARE_RAISE", "RAISING", 
    "RAISED", "PREPARE_LOWER", "LOWERING", "EMERGENCY_LOWER", "EMERGENCY_RAISE"
  };
  
  if (state >= 1 && state <= 8) {
    return stateNames[state];
  }
  return stateNames[0]; // UNKNOWN
}

// Debug logging functions
void debugLog(const char* message) {
  Serial.print(F("[DEBUG] "));
  Serial.print(millis());
  Serial.print(F("ms: "));
  Serial.println(message);
}

void debugLogSensors() {
  Serial.print(F("[SENSORS] "));
  Serial.print(millis());
  Serial.print(F("ms - EStop:"));
  Serial.print(eStopPressed() ? F("PRESSED") : F("CLEAR"));
  Serial.print(F(" | TopLimit:"));
  Serial.print(topLimitHit() ? F("HIT") : F("CLEAR"));
  Serial.print(F(" | BottomLimit:"));
  Serial.print(bottomLimitHit() ? F("HIT") : F("CLEAR"));
  Serial.print(F(" | Sonic:"));
  Serial.print(sonicSensor.poll_cm());
  Serial.println(F("cm"));
}

void debugLogStateChange(bridgeState newState) {
  Serial.print(F("[STATE] "));
  Serial.print(millis());
  Serial.print(F("ms: Changing to "));
  Serial.println(getStateName(newState));
}
