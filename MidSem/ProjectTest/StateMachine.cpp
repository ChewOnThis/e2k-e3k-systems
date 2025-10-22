/*
 * BRIDGE CONTROL STATE MACHINE - Enhanced Configurable Version
 * 
 * MAJOR CHANGES FROM ORIGINAL:
 * ============================
 * 
 * 1. CONFIGURATION INTEGRATION:
 *    - All hardcoded constants replaced with configurable parameters
 *    - Runtime parameter access through bridgeConfig global instance
 *    - EEPROM persistence for all settings
 * 
 * 2. CONDITIONAL LOGGING SYSTEM:
 *    - Three logging categories: Debug, Sensor, State
 *    - Each category can be enabled/disabled independently
 *    - Memory-optimized using F() macro for flash storage
 *    - Automatic timestamp generation for all log entries
 * 
 * 3. ENHANCED STATE MANAGEMENT:
 *    - Previous state tracking with previousState variable
 *    - One-time initialization pattern for state entry operations
 *    - Automatic state transition logging when enabled
 *    - Helper functions integrated with configuration system
 * 
 * 4. SAFETY ENHANCEMENTS:
 *    - Configurable safety features (E-stop, timeouts)
 *    - Range validation for all sensor inputs
 *    - Graceful degradation when features disabled
 * 
 * HOW THE CODE WORKS:
 * ===================
 * 
 * CONFIGURATION ACCESS PATTERN:
 * - Instead of: if (millis() - startTime > 3000)
 * - Now use:    if (millis() - startTime > bridgeConfig.getActionDelay())
 * 
 * CONDITIONAL LOGGING PATTERN:
 * - Check configuration flag before any output operation
 * - Example: if (bridgeConfig.isDebugLoggingEnabled()) debugLog("message");
 * - Logging functions automatically check their respective flags
 * 
 * STATE INITIALIZATION PATTERN:
 * - Static boolean flags prevent repeated initialization in states
 * - Pattern: static bool initialized = false; if (!initialized) { setup; initialized = true; }
 * - Flags reset when exiting states to allow re-initialization
 * 
 * HELPER FUNCTION ENHANCEMENTS:
 * - All sensor thresholds now use configurable values
 * - Safety features can be disabled for testing via configuration
 * - Range validation integrated into sensor reading functions
 * 
 * MEMORY OPTIMIZATION:
 * - F() macro stores constant strings in flash memory instead of RAM
 * - Reduced memory footprint for embedded systems with limited RAM
 * - Efficient EEPROM usage with packed configuration structure
 * 
 * RUNTIME BEHAVIOR:
 * - System loads configuration from EEPROM on startup
 * - Parameters can be modified via serial commands without recompilation
 * - Changes persist across power cycles when saved to EEPROM
 * - Invalid configurations automatically reset to safe defaults
 */

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


// Helper functions

bool eStopPressed()   { 
  return bridgeConfig.isEmergencyStopEnabled() ? digitalRead(Pin_EStop) == HIGH : false; 
}
bool topLimitHit()    { return digitalRead(Pin_LS_1) == HIGH; }
bool bottomLimitHit() { return digitalRead(Pin_LS_2) == HIGH; }
bool boatDetected()   { 
  int distance = sonicSensor.poll_cm();
  return (distance > 0 && distance < bridgeConfig.getBoatDetectionDistance()); 
}
bool areaClear()      { 
  int distance = sonicSensor.poll_cm();
  return (distance > bridgeConfig.getAreaClearDistance()); 
}
bool timerFinished()  { 
  return millis() - startTime > bridgeConfig.getActionDelay(); 
}
bool motionTimeout()  { 
  return bridgeConfig.isMotionTimeoutEnabled() ? 
    (millis() - startTime > bridgeConfig.getMoveTimeout()) : false; 
}

void startMotorUp()   { 
  if (bridgeConfig.isDebugLoggingEnabled()) {
    debugLog("Motor UP started"); 
  }
  motor.run(bridgeConfig.getMotorSpeedFast(), bridgeConfig.getMotorDirection1()); 
}

void startMotorDown() { 
  if (bridgeConfig.isDebugLoggingEnabled()) {
    debugLog("Motor DOWN started"); 
  }
  motor.run(bridgeConfig.getMotorSpeedFast(), bridgeConfig.getMotorDirection2()); 
}

void stopMotor()      { 
  if (bridgeConfig.isDebugLoggingEnabled()) {
    debugLog("Motor stopped"); 
  }
  motor.disable(); 
}


// Main state machine logic

void stateMachine(bridgeState state) {
  // Log state change if different from previous state
  if (state != previousState && bridgeConfig.isStateLoggingEnabled()) {
    debugLogStateChange(state);
    previousState = state;
  }

  switch (state) {

    //  DOWN (lowered) 
    case lowered:
      trafficLight.cycle(2);
      
      if (boatDetected()) {
        if (bridgeConfig.isDebugLoggingEnabled()) {
          debugLog("Boat detected, preparing to raise bridge");
        }
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
        if (bridgeConfig.isSensorLoggingEnabled()) {
          debugLogSensors();
        }
        prepRaiseInitialized = true;
      }

      if (eStopPressed()) {
        if (bridgeConfig.isDebugLoggingEnabled()) {
          debugLog("Emergency stop pressed during prep to raise");
        }
        prepRaiseInitialized = false;
        currentState = emergencyLower;
      } 
      else if (timerFinished()) {
        if (bridgeConfig.isDebugLoggingEnabled()) {
          debugLog("Preparation timer completed, starting to raise bridge");
        }
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
        if (bridgeConfig.isDebugLoggingEnabled()) {
          debugLog("EMERGENCY: E-stop pressed while raising bridge");
        }
        raisingInitialized = false;
        currentState = emergencyLower;
      } 
      else if (topLimitHit() || motionTimeout()) {
        stopMotor();
        if (bridgeConfig.isDebugLoggingEnabled()) {
          debugLog(topLimitHit() ? "Top limit switch reached" : "Motion timeout reached");
        }
        raisingInitialized = false;
        currentState = raised;
      }
      break;

    // === UP (raised) ===
    case raised:
      trafficLight.cycle(0);

      if (areaClear()) {
        if (bridgeConfig.isDebugLoggingEnabled()) {
          debugLog("Area clear detected, preparing to lower bridge");
        }
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
        if (bridgeConfig.isSensorLoggingEnabled()) {
          debugLogSensors();
        }
        prepLowerInitialized = true;
      }

      if (eStopPressed()) {
        if (bridgeConfig.isDebugLoggingEnabled()) {
          debugLog("Emergency stop pressed during prep to lower");
        }
        prepLowerInitialized = false;
        currentState = emergencyRaise;
      } 
      else if (timerFinished()) {
        if (bridgeConfig.isDebugLoggingEnabled()) {
          debugLog("Preparation timer completed, starting to lower bridge");
        }
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
        if (bridgeConfig.isDebugLoggingEnabled()) {
          debugLog("EMERGENCY: E-stop pressed while lowering bridge");
        }
        loweringInitialized = false;
        currentState = emergencyRaise;
      } 
      else if (bottomLimitHit() || motionTimeout()) {
        stopMotor();
        if (bridgeConfig.isDebugLoggingEnabled()) {
          debugLog(bottomLimitHit() ? "Bottom limit switch reached" : "Motion timeout reached");
        }
        trafficLight.cycle(2);
        loweringInitialized = false;
        currentState = lowered;
      }
      break;

    // === EMERGENCY LOWER ===
    case emergencyLower:
      if (bridgeConfig.isDebugLoggingEnabled()) {
        debugLog("EMERGENCY PROCEDURE: Lowering bridge immediately");
        debugLogSensors();
      }
      startMotorDown();
      delay(bridgeConfig.getEmergencyDelay());
      stopMotor();
      if (bridgeConfig.isDebugLoggingEnabled()) {
        debugLog("Emergency lower completed");
      }
      currentState = lowered;
      break;

    // === EMERGENCY RAISE ===
    case emergencyRaise:
      if (bridgeConfig.isDebugLoggingEnabled()) {
        debugLog("EMERGENCY PROCEDURE: Raising bridge immediately");
        debugLogSensors();
      }
      startMotorUp();
      delay(bridgeConfig.getEmergencyDelay());
      stopMotor();
      if (bridgeConfig.isDebugLoggingEnabled()) {
        debugLog("Emergency raise completed");
      }
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
  if (bridgeConfig.isDebugLoggingEnabled()) {
    Serial.print(F("[DEBUG] "));
    Serial.print(millis());
    Serial.print(F("ms: "));
    Serial.println(message);
  }
}

void debugLogSensors() {
  if (bridgeConfig.isSensorLoggingEnabled()) {
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
}

void debugLogStateChange(bridgeState newState) {
  if (bridgeConfig.isStateLoggingEnabled()) {
    Serial.print(F("[STATE] "));
    Serial.print(millis());
    Serial.print(F("ms: Changing to "));
    Serial.println(getStateName(newState));
  }
}
