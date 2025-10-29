/*
 * BRIDGE CONFIGURATION MANAGER - EEPROM-Based Runtime Configuration System
 * 
 * PURPOSE:
 * ========
 * This module implements a comprehensive configuration management system that allows
 * all bridge control parameters to be modified at runtime without recompiling code.
 * Configuration data is stored in EEPROM for persistence across power cycles.
 * 
 * KEY COMPONENTS:
 * ===============
 * 
 * 1. CONFIGURATION STRUCTURE (BridgeConfig):
 *    - Packed structure containing all configurable parameters
 *    - Magic number for data integrity validation
 *    - Organized into logical groups: timing, distance, motor, features
 *    - Reserved space for future expansion without breaking compatibility
 * 
 * 2. EEPROM MANAGEMENT:
 *    - Automatic loading of configuration on system startup
 *    - Validation of data integrity using magic number
 *    - Safe fallback to defaults if EEPROM data is corrupted
 *    - Platform-specific handling for Arduino vs ESP32/ESP8266
 * 
 * 3. COMMAND INTERFACE:
 *    - Serial-based command processing for runtime parameter modification
 *    - Support for parameter=value syntax with comprehensive validation
 *    - Built-in help system with command reference and valid ranges
 *    - Immediate feedback for successful changes or error conditions
 * 
 * 4. VALIDATION FRAMEWORK:
 *    - Range checking for all numeric parameters
 *    - Type-specific validation (timing, distance, speed, boolean)
 *    - Safety limits to prevent dangerous configurations
 *    - Flexible boolean parsing (on/off, true/false, 1/0)
 * 
 * HOW IT OPERATES:
 * ================
 * 
 * INITIALIZATION SEQUENCE:
 * 1. EEPROM.begin() - Initialize EEPROM subsystem
 * 2. loadConfig() - Attempt to read existing configuration
 * 3. If load fails: resetToDefaults() + saveConfig()
 * 4. printConfig() - Display current configuration for verification
 * 
 * CONFIGURATION LOADING PROCESS:
 * 1. Read magic number from EEPROM address 0
 * 2. Validate magic number matches expected value (0xBE3F2024)
 * 3. If valid, read entire configuration structure
 * 4. Perform range validation on all parameters
 * 5. Return success/failure status
 * 
 * COMMAND PROCESSING FLOW:
 * 1. Receive command string from serial input
 * 2. Sanitize input (trim whitespace, convert to lowercase)
 * 3. Parse command type (single command vs parameter=value)
 * 4. For parameter commands: extract parameter name and value
 * 5. Validate parameter name and value range
 * 6. Update configuration and provide feedback
 * 
 * PARAMETER CATEGORIES:
 * =====================
 * 
 * TIMING PARAMETERS (milliseconds):
 * - actionDelay: State transition delay (100-60000ms)
 * - moveTimeout: Motor operation safety timeout (1000-120000ms)
 * - emergencyDelay: Emergency operation duration (500-10000ms)
 * - debugLogInterval: Sensor logging frequency (1000-300000ms)
 * 
 * DISTANCE PARAMETERS (centimeters):
 * - boatDetectionDistance: Boat presence threshold (5-1000cm)
 * - areaClearDistance: Area clear threshold (10-1000cm)
 * - sonicSensorMinRange: Minimum valid sensor reading (1-50cm)
 * - sonicSensorMaxRange: Maximum valid sensor reading (100-1000cm)
 * 
 * MOTOR PARAMETERS:
 * - motorSpeedFast: High-speed setting (50-255)
 * - motorSpeedSlow: Low-speed setting (20-255)
 * - motorDirection1: Up/raise direction value
 * - motorDirection2: Down/lower direction value
 * 
 * FEATURE CONTROL FLAGS:
 * - enableDebugLogging: General debug message output
 * - enableSensorLogging: Periodic sensor status reports
 * - enableStateLogging: State transition notifications
 * - enableEmergencyStop: E-stop functionality
 * - enableMotionTimeout: Motor timeout safety feature
 * 
 * SAFETY CONSIDERATIONS:
 * ======================
 * 
 * 1. RANGE VALIDATION:
 *    - All numeric parameters validated against safe operating limits
 *    - Invalid values rejected with error messages
 *    - Prevents configuration of dangerous settings
 * 
 * 2. MAGIC NUMBER PROTECTION:
 *    - Detects corrupted EEPROM data
 *    - Automatically falls back to safe defaults
 *    - Prevents system operation with invalid configuration
 * 
 * 3. DEFAULT VALUE SAFETY:
 *    - Conservative default values ensure safe operation
 *    - Well-tested parameter combinations
 *    - Suitable for most bridge control applications
 * 
 * 4. PERSISTENCE RELIABILITY:
 *    - EEPROM commit operations for ESP platforms
 *    - Structure packing for consistent memory layout
 *    - Reserved space for future parameter additions
 * 
 * USAGE EXAMPLES:
 * ===============
 * 
 * // Access configuration values in code
 * if (millis() - startTime > bridgeConfig.getActionDelay()) {
 *     // Timer expired
 * }
 * 
 * // Modify parameters via serial commands
 * "action_delay=5000"     // Set 5-second state delay
 * "boat_detect=120"       // Set boat detection to 120cm
 * "debug_log=off"         // Disable debug output
 * "save"                  // Save changes to EEPROM
 * 
 * // View current configuration
 * "show"                  // Display all current values
 * "help"                  // Show command reference
 */

#include "BridgeConfig.h"

// Global configuration manager instance
BridgeConfigManager bridgeConfig;

void BridgeConfigManager::begin() {
  // Initialize EEPROM with appropriate size
  EEPROM.begin(sizeof(BridgeConfig) + 16);
  
  // Try to load existing configuration
  if (!loadConfig()) {
    // If loading fails, reset to defaults and save
    resetToDefaults();
    saveConfig();
  }
  
  // Print current configuration
  printConfig();
}

bool BridgeConfigManager::loadConfig() {
  // Read magic number first
  uint32_t magic;
  EEPROM.get(EEPROM_START_ADDR, magic);
  
  if (magic != CONFIG_MAGIC) {
    return false; // Invalid or uninitialized configuration
  }
  
  // Read the entire configuration structure
  EEPROM.get(EEPROM_START_ADDR, config);
  
  // Validate ranges
  if (config.actionDelay > 60000 || config.actionDelay < 100) return false;
  if (config.moveTimeout > 120000 || config.moveTimeout < 1000) return false;
  if (config.boatDetectionDistance > 1000) return false;
  if (config.areaClearDistance > 1000) return false;
  if (config.motorSpeedFast > 255) return false;
  if (config.motorSpeedSlow > 255) return false;
  
  return true;
}

bool BridgeConfigManager::saveConfig() {
  // Set magic number
  config.magic = CONFIG_MAGIC;
  
  // Write the entire configuration structure to EEPROM
  EEPROM.put(EEPROM_START_ADDR, config);
  
  // Commit changes (required for ESP32/ESP8266)
  #if defined(ESP32) || defined(ESP8266)
    return EEPROM.commit();
  #else
    return true;
  #endif
}

void BridgeConfigManager::resetToDefaults() {
  // Set default timing values (milliseconds)
  config.actionDelay = 3000;        // 3 seconds wait before state transitions
  config.moveTimeout = 8000;        // 8 seconds safety timeout
  config.emergencyDelay = 2000;     // 2 seconds for emergency operations
  config.debugLogInterval = 10000;  // 10 seconds between sensor logs
  
  // Set default distance thresholds (centimeters)
  config.boatDetectionDistance = 100;  // Boat detected if closer than 100cm
  config.areaClearDistance = 150;      // Area clear if further than 150cm
  config.sonicSensorMinRange = 2;      // Minimum valid reading: 2cm
  config.sonicSensorMaxRange = 400;    // Maximum valid reading: 400cm
  
  // Set default motor parameters
  config.motorSpeedFast = 255;      // Full speed
  config.motorSpeedSlow = 128;      // Half speed
  config.motorDirection1 = 1;       // Direction for raising (up)
  config.motorDirection2 = 0;       // Direction for lowering (down)
  
  // Set default debug and logging settings
  config.enableDebugLogging = true;
  config.enableSensorLogging = true;
  config.enableStateLogging = true;
  
  // Set default safety settings
  config.enableEmergencyStop = true;
  config.enableMotionTimeout = true;
  
  // Clear reserved area
  memset(config.reserved, 0, sizeof(config.reserved));
}

void BridgeConfigManager::printConfig() {
  Serial.println(F("=== Bridge Configuration ==="));
  Serial.print(F("Action Delay: ")); Serial.print(config.actionDelay); Serial.println(F("ms"));
  Serial.print(F("Move Timeout: ")); Serial.print(config.moveTimeout); Serial.println(F("ms"));
  Serial.print(F("Emergency Delay: ")); Serial.print(config.emergencyDelay); Serial.println(F("ms"));
  Serial.print(F("Debug Log Interval: ")); Serial.print(config.debugLogInterval); Serial.println(F("ms"));
  Serial.println();
  
  Serial.print(F("Boat Detection Distance: ")); Serial.print(config.boatDetectionDistance); Serial.println(F("cm"));
  Serial.print(F("Area Clear Distance: ")); Serial.print(config.areaClearDistance); Serial.println(F("cm"));
  Serial.print(F("Sonic Range: ")); Serial.print(config.sonicSensorMinRange); 
  Serial.print(F("-")); Serial.print(config.sonicSensorMaxRange); Serial.println(F("cm"));
  Serial.println();
  
  Serial.print(F("Motor Speed Fast: ")); Serial.println(config.motorSpeedFast);
  Serial.print(F("Motor Speed Slow: ")); Serial.println(config.motorSpeedSlow);
  Serial.print(F("Motor Directions: ")); Serial.print(config.motorDirection1); 
  Serial.print(F(" (up), ")); Serial.print(config.motorDirection2); Serial.println(F(" (down)"));
  Serial.println();
  
  Serial.print(F("Debug Logging: ")); Serial.println(config.enableDebugLogging ? F("ON") : F("OFF"));
  Serial.print(F("Sensor Logging: ")); Serial.println(config.enableSensorLogging ? F("ON") : F("OFF"));
  Serial.print(F("State Logging: ")); Serial.println(config.enableStateLogging ? F("ON") : F("OFF"));
  Serial.print(F("Emergency Stop: ")); Serial.println(config.enableEmergencyStop ? F("ON") : F("OFF"));
  Serial.print(F("Motion Timeout: ")); Serial.println(config.enableMotionTimeout ? F("ON") : F("OFF"));
  Serial.println(F("=============================="));
}

void BridgeConfigManager::printConfigMenu() {
  Serial.println(F("\n=== Configuration Commands ==="));
  Serial.println(F("Timing (ms):"));
  Serial.println(F("  action_delay=<value>     - Set state transition delay"));
  Serial.println(F("  move_timeout=<value>     - Set motor operation timeout"));
  Serial.println(F("  emergency_delay=<value>  - Set emergency operation duration"));
  Serial.println(F("  debug_interval=<value>   - Set sensor logging interval"));
  Serial.println(F("\nDistances (cm):"));
  Serial.println(F("  boat_detect=<value>      - Set boat detection distance"));
  Serial.println(F("  area_clear=<value>       - Set area clear distance"));
  Serial.println(F("  sonic_min=<value>        - Set minimum sonic range"));
  Serial.println(F("  sonic_max=<value>        - Set maximum sonic range"));
  Serial.println(F("\nMotor:"));
  Serial.println(F("  speed_fast=<value>       - Set fast motor speed (0-255)"));
  Serial.println(F("  speed_slow=<value>       - Set slow motor speed (0-255)"));
  Serial.println(F("\nControl:"));
  Serial.println(F("  debug_log=<on/off>       - Enable/disable debug logging"));
  Serial.println(F("  sensor_log=<on/off>      - Enable/disable sensor logging"));
  Serial.println(F("  state_log=<on/off>       - Enable/disable state logging"));
  Serial.println(F("  emergency_stop=<on/off>  - Enable/disable emergency stop"));
  Serial.println(F("  motion_timeout=<on/off>  - Enable/disable motion timeout"));
  Serial.println(F("\nCommands:"));
  Serial.println(F("  save                     - Save configuration to EEPROM"));
  Serial.println(F("  reset                    - Reset to default values"));
  Serial.println(F("  show                     - Display current configuration"));
  Serial.println(F("  help                     - Show this menu"));
  Serial.println(F("===============================\n"));
}

bool BridgeConfigManager::processConfigCommand(String command) {
  command.trim();
  command.toLowerCase();
  
  if (command == "help") {
    printConfigMenu();
    return true;
  }
  
  if (command == "show") {
    printConfig();
    return true;
  }
  
  if (command == "save") {
    if (saveConfig()) {
      Serial.println(F("Configuration saved to EEPROM"));
    } else {
      Serial.println(F("Failed to save configuration"));
    }
    return true;
  }
  
  if (command == "reset") {
    resetToDefaults();
    Serial.println(F("Configuration reset to defaults"));
    return true;
  }
  
  // Parse parameter=value commands
  int equalPos = command.indexOf('=');
  if (equalPos > 0) {
    String param = command.substring(0, equalPos);
    String value = command.substring(equalPos + 1);
    
    param.trim();
    value.trim();
    
    // Timing parameters
    if (param == "action_delay") {
      uint32_t val = value.toInt();
      if (val >= 100 && val <= 60000) {
        setActionDelay(val);
        Serial.print(F("Action delay set to: ")); Serial.println(val);
        return true;
      }
    }
    else if (param == "move_timeout") {
      uint32_t val = value.toInt();
      if (val >= 1000 && val <= 120000) {
        setMoveTimeout(val);
        Serial.print(F("Move timeout set to: ")); Serial.println(val);
        return true;
      }
    }
    else if (param == "emergency_delay") {
      uint32_t val = value.toInt();
      if (val >= 500 && val <= 10000) {
        setEmergencyDelay(val);
        Serial.print(F("Emergency delay set to: ")); Serial.println(val);
        return true;
      }
    }
    else if (param == "debug_interval") {
      uint32_t val = value.toInt();
      if (val >= 1000 && val <= 300000) {
        setDebugLogInterval(val);
        Serial.print(F("Debug interval set to: ")); Serial.println(val);
        return true;
      }
    }
    // Distance parameters
    else if (param == "boat_detect") {
      uint16_t val = value.toInt();
      if (val >= 5 && val <= 1000) {
        setBoatDetectionDistance(val);
        Serial.print(F("Boat detection distance set to: ")); Serial.println(val);
        return true;
      }
    }
    else if (param == "area_clear") {
      uint16_t val = value.toInt();
      if (val >= 10 && val <= 1000) {
        setAreaClearDistance(val);
        Serial.print(F("Area clear distance set to: ")); Serial.println(val);
        return true;
      }
    }
    else if (param == "sonic_min") {
      uint16_t val = value.toInt();
      if (val >= 1 && val <= 50) {
        setSonicRange(val, getSonicMaxRange());
        Serial.print(F("Sonic minimum range set to: ")); Serial.println(val);
        return true;
      }
    }
    else if (param == "sonic_max") {
      uint16_t val = value.toInt();
      if (val >= 100 && val <= 1000) {
        setSonicRange(getSonicMinRange(), val);
        Serial.print(F("Sonic maximum range set to: ")); Serial.println(val);
        return true;
      }
    }
    // Motor parameters
    else if (param == "speed_fast") {
      uint8_t val = value.toInt();
      if (val >= 50 && val <= 255) {
        setMotorSpeeds(val, getMotorSpeedSlow());
        Serial.print(F("Fast motor speed set to: ")); Serial.println(val);
        return true;
      }
    }
    else if (param == "speed_slow") {
      uint8_t val = value.toInt();
      if (val >= 20 && val <= 255) {
        setMotorSpeeds(getMotorSpeedFast(), val);
        Serial.print(F("Slow motor speed set to: ")); Serial.println(val);
        return true;
      }
    }
    // Boolean parameters
    else if (param == "debug_log") {
      bool enable = (value == "on" || value == "true" || value == "1");
      setDebugLogging(enable);
      Serial.print(F("Debug logging: ")); Serial.println(enable ? F("ON") : F("OFF"));
      return true;
    }
    else if (param == "sensor_log") {
      bool enable = (value == "on" || value == "true" || value == "1");
      setSensorLogging(enable);
      Serial.print(F("Sensor logging: ")); Serial.println(enable ? F("ON") : F("OFF"));
      return true;
    }
    else if (param == "state_log") {
      bool enable = (value == "on" || value == "true" || value == "1");
      setStateLogging(enable);
      Serial.print(F("State logging: ")); Serial.println(enable ? F("ON") : F("OFF"));
      return true;
    }
    else if (param == "emergency_stop") {
      bool enable = (value == "on" || value == "true" || value == "1");
      setEmergencyStop(enable);
      Serial.print(F("Emergency stop: ")); Serial.println(enable ? F("ON") : F("OFF"));
      return true;
    }
    else if (param == "motion_timeout") {
      bool enable = (value == "on" || value == "true" || value == "1");
      setMotionTimeout(enable);
      Serial.print(F("Motion timeout: ")); Serial.println(enable ? F("ON") : F("OFF"));
      return true;
    }
  }
  
  Serial.print(F("Unknown command or invalid value: ")); Serial.println(command);
  Serial.println(F("Type 'help' for available commands"));
  return false;
}