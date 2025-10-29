#ifndef BRIDGECONFIG_H
#define BRIDGECONFIG_H

#include <Arduino.h>
#include <EEPROM.h>

// EEPROM Configuration Structure
struct BridgeConfig {
  // Magic number to verify valid configuration
  uint32_t magic;
  
  // Timing constants (milliseconds)
  uint32_t actionDelay;        // Wait time before state transitions
  uint32_t moveTimeout;        // Safety timeout for motor operations
  uint32_t emergencyDelay;     // Duration for emergency operations
  uint32_t debugLogInterval;   // Periodic sensor logging interval
  
  // Distance thresholds (centimeters)
  uint16_t boatDetectionDistance;  // Distance to detect boat presence
  uint16_t areaClearDistance;      // Distance to consider area clear
  uint16_t sonicSensorMinRange;    // Minimum valid sonic sensor reading
  uint16_t sonicSensorMaxRange;    // Maximum valid sonic sensor reading
  
  // Motor parameters
  uint8_t motorSpeedFast;      // Fast motor speed (0-255)
  uint8_t motorSpeedSlow;      // Slow motor speed (0-255)
  uint8_t motorDirection1;     // Direction 1 (up/raise)
  uint8_t motorDirection2;     // Direction 2 (down/lower)
  
  // Debug and logging settings
  bool enableDebugLogging;     // Enable/disable debug output
  bool enableSensorLogging;    // Enable/disable periodic sensor logs
  bool enableStateLogging;     // Enable/disable state transition logs
  
  // Safety settings
  bool enableEmergencyStop;    // Enable/disable E-stop functionality
  bool enableMotionTimeout;    // Enable/disable motion timeout safety
  
  // Reserved for future use
  uint8_t reserved[16];
};

// Configuration management class
class BridgeConfigManager {
private:
  static const uint32_t CONFIG_MAGIC = 0xBE3F2024;  // Magic number
  static const int EEPROM_START_ADDR = 0;
  BridgeConfig config;
  
public:
  // Initialize configuration system
  void begin();
  
  // Load configuration from EEPROM
  bool loadConfig();
  
  // Save configuration to EEPROM
  bool saveConfig();
  
  // Reset to default values
  void resetToDefaults();
  
  // Getters for configuration values
  uint32_t getActionDelay() const { return config.actionDelay; }
  uint32_t getMoveTimeout() const { return config.moveTimeout; }
  uint32_t getEmergencyDelay() const { return config.emergencyDelay; }
  uint32_t getDebugLogInterval() const { return config.debugLogInterval; }
  
  uint16_t getBoatDetectionDistance() const { return config.boatDetectionDistance; }
  uint16_t getAreaClearDistance() const { return config.areaClearDistance; }
  uint16_t getSonicMinRange() const { return config.sonicSensorMinRange; }
  uint16_t getSonicMaxRange() const { return config.sonicSensorMaxRange; }
  
  uint8_t getMotorSpeedFast() const { return config.motorSpeedFast; }
  uint8_t getMotorSpeedSlow() const { return config.motorSpeedSlow; }
  uint8_t getMotorDirection1() const { return config.motorDirection1; }
  uint8_t getMotorDirection2() const { return config.motorDirection2; }
  
  bool isDebugLoggingEnabled() const { return config.enableDebugLogging; }
  bool isSensorLoggingEnabled() const { return config.enableSensorLogging; }
  bool isStateLoggingEnabled() const { return config.enableStateLogging; }
  bool isEmergencyStopEnabled() const { return config.enableEmergencyStop; }
  bool isMotionTimeoutEnabled() const { return config.enableMotionTimeout; }
  
  // Setters for configuration values
  void setActionDelay(uint32_t value) { config.actionDelay = value; }
  void setMoveTimeout(uint32_t value) { config.moveTimeout = value; }
  void setEmergencyDelay(uint32_t value) { config.emergencyDelay = value; }
  void setDebugLogInterval(uint32_t value) { config.debugLogInterval = value; }
  
  void setBoatDetectionDistance(uint16_t value) { config.boatDetectionDistance = value; }
  void setAreaClearDistance(uint16_t value) { config.areaClearDistance = value; }
  void setSonicRange(uint16_t minRange, uint16_t maxRange) { 
    config.sonicSensorMinRange = minRange; 
    config.sonicSensorMaxRange = maxRange; 
  }
  
  void setMotorSpeeds(uint8_t fast, uint8_t slow) { 
    config.motorSpeedFast = fast; 
    config.motorSpeedSlow = slow; 
  }
  void setMotorDirections(uint8_t dir1, uint8_t dir2) { 
    config.motorDirection1 = dir1; 
    config.motorDirection2 = dir2; 
  }
  
  void setDebugLogging(bool enable) { config.enableDebugLogging = enable; }
  void setSensorLogging(bool enable) { config.enableSensorLogging = enable; }
  void setStateLogging(bool enable) { config.enableStateLogging = enable; }
  void setEmergencyStop(bool enable) { config.enableEmergencyStop = enable; }
  void setMotionTimeout(bool enable) { config.enableMotionTimeout = enable; }
  
  // Configuration display and testing
  void printConfig();
  void printConfigMenu();
  bool processConfigCommand(String command);
};

// Global configuration manager instance
extern BridgeConfigManager bridgeConfig;

#endif