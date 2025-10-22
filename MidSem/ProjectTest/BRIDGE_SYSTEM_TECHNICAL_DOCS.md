# Bridge Control System - Technical Documentation

## Table of Contents
1. [System Overview](#system-overview)
2. [Configuration Management System](#configuration-management-system)
3. [Debug Logging Architecture](#debug-logging-architecture)
4. [State Machine Operation](#state-machine-operation)
5. [EEPROM Storage Implementation](#eeprom-storage-implementation)
6. [Serial Command Interface](#serial-command-interface)
7. [Code Architecture and Flow](#code-architecture-and-flow)
8. [Testing and Calibration Guide](#testing-and-calibration-guide)
9. [Troubleshooting Reference](#troubleshooting-reference)

---

## System Overview

The Bridge Control System has been enhanced with a comprehensive configuration management and debugging framework that transforms it from a static, compile-time configured system into a dynamic, runtime-configurable platform. This enhancement enables rapid testing, field calibration, and behavior modification without requiring code recompilation.

### Key Architectural Changes

1. **Configuration Abstraction Layer**: All hardcoded constants have been replaced with configurable parameters accessed through a centralized configuration manager
2. **EEPROM Persistence**: Configuration data persists across power cycles using Arduino's EEPROM library
3. **Runtime Command Interface**: Serial-based command system allows real-time parameter modification
4. **Conditional Logging System**: Debug output can be selectively enabled/disabled based on configuration flags
5. **Validation Framework**: Input validation ensures system safety by preventing invalid parameter values

---

## Configuration Management System

### BridgeConfig Class Architecture

The `BridgeConfigManager` class serves as the central hub for all system configuration. It implements a singleton pattern through a global instance `bridgeConfig` that provides system-wide access to configuration parameters.

#### Configuration Structure (`BridgeConfig`)

The configuration data is stored in a packed structure that maps directly to EEPROM memory:

```cpp
struct BridgeConfig {
    uint32_t magic;                    // Validation marker (0xBE3F2024)
    
    // Timing parameters (milliseconds)
    uint32_t actionDelay;              // State transition delay
    uint32_t moveTimeout;              // Motor operation safety timeout
    uint32_t emergencyDelay;           // Emergency operation duration
    uint32_t debugLogInterval;         // Sensor logging frequency
    
    // Distance thresholds (centimeters)
    uint16_t boatDetectionDistance;    // Boat presence threshold
    uint16_t areaClearDistance;        // Area clear threshold
    uint16_t sonicSensorMinRange;      // Minimum valid sensor reading
    uint16_t sonicSensorMaxRange;      // Maximum valid sensor reading
    
    // Motor control parameters
    uint8_t motorSpeedFast;            // High-speed setting (0-255)
    uint8_t motorSpeedSlow;            // Low-speed setting (0-255)
    uint8_t motorDirection1;           // Up/raise direction
    uint8_t motorDirection2;           // Down/lower direction
    
    // Feature control flags
    bool enableDebugLogging;           // General debug output control
    bool enableSensorLogging;          // Periodic sensor status logs
    bool enableStateLogging;           // State transition notifications
    bool enableEmergencyStop;          // E-stop functionality
    bool enableMotionTimeout;          // Motor timeout safety
    
    uint8_t reserved[16];              // Future expansion space
};
```

#### Magic Number Validation

The configuration structure includes a magic number (`0xBE3F2024`) that serves multiple purposes:
- **Integrity Check**: Validates that EEPROM contains valid configuration data
- **Version Control**: Can be modified to force configuration reset during firmware updates
- **Corruption Detection**: Detects if EEPROM data has been corrupted

### Configuration Lifecycle

#### 1. Initialization (`begin()`)
```cpp
void BridgeConfigManager::begin() {
    EEPROM.begin(sizeof(BridgeConfig) + 16);  // Initialize EEPROM with buffer
    
    if (!loadConfig()) {                      // Attempt to load existing config
        resetToDefaults();                    // Fall back to defaults if invalid
        saveConfig();                         // Save defaults to EEPROM
    }
    
    printConfig();                            // Display current configuration
}
```

The initialization process follows a fail-safe approach:
1. Initialize EEPROM with sufficient space for the configuration structure plus buffer
2. Attempt to load existing configuration from EEPROM
3. If loading fails (invalid magic number or corrupted data), reset to factory defaults
4. Save the current configuration to ensure EEPROM is in a known good state
5. Display the active configuration for verification

#### 2. Configuration Loading (`loadConfig()`)
```cpp
bool BridgeConfigManager::loadConfig() {
    uint32_t magic;
    EEPROM.get(EEPROM_START_ADDR, magic);     // Read magic number first
    
    if (magic != CONFIG_MAGIC) {
        return false;                         // Invalid configuration marker
    }
    
    EEPROM.get(EEPROM_START_ADDR, config);    // Load entire structure
    
    // Validate parameter ranges
    if (config.actionDelay > 60000 || config.actionDelay < 100) return false;
    if (config.moveTimeout > 120000 || config.moveTimeout < 1000) return false;
    // ... additional validation checks
    
    return true;
}
```

The loading process implements multiple validation layers:
1. **Magic Number Check**: Ensures EEPROM contains valid configuration data
2. **Structure Loading**: Reads the entire configuration structure in one operation
3. **Range Validation**: Verifies each parameter falls within safe operating limits
4. **Sanity Checks**: Ensures logical consistency between related parameters

#### 3. Configuration Saving (`saveConfig()`)
```cpp
bool BridgeConfigManager::saveConfig() {
    config.magic = CONFIG_MAGIC;              // Set validation marker
    EEPROM.put(EEPROM_START_ADDR, config);    // Write entire structure
    
    #if defined(ESP32) || defined(ESP8266)
        return EEPROM.commit();               // Commit changes on ESP platforms
    #else
        return true;                          // Arduino automatically commits
    #endif
}
```

The saving process ensures data integrity:
1. **Magic Number Assignment**: Marks the configuration as valid
2. **Atomic Write**: Writes the entire structure in one operation
3. **Platform-Specific Commit**: Handles differences between Arduino and ESP platforms

---

## Debug Logging Architecture

### Conditional Logging System

The debug logging system has been redesigned from a simple output mechanism to a sophisticated, configurable framework that respects system configuration and provides multiple granularity levels.

#### Logging Categories

The system implements three distinct logging categories, each controlled by its own configuration flag:

1. **Debug Logging** (`enableDebugLogging`): General operational messages and state information
2. **Sensor Logging** (`enableSensorLogging`): Periodic sensor status reports and hardware state
3. **State Logging** (`enableStateLogging`): State machine transitions and control flow

#### Debug Function Implementation

```cpp
void debugLog(const char* message) {
    if (bridgeConfig.isDebugLoggingEnabled()) {        // Check configuration flag
        Serial.print(F("[DEBUG] "));                   // Category prefix
        Serial.print(millis());                        // Timestamp
        Serial.print(F("ms: "));                       // Separator
        Serial.println(message);                       // Message content
    }
}
```

**Operation Flow:**
1. **Configuration Check**: Verifies if debug logging is enabled before any output
2. **Memory Optimization**: Uses `F()` macro to store strings in flash memory instead of RAM
3. **Timestamp Generation**: Provides millisecond-precision timing for event correlation
4. **Formatted Output**: Consistent format across all debug messages

#### Sensor Logging Implementation

```cpp
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
```

**Sensor Status Format:**
- **Timestamp**: Millisecond precision for timing analysis
- **Digital Inputs**: Binary state representation (PRESSED/CLEAR, HIT/CLEAR)
- **Analog Inputs**: Numeric values with units (cm for distance)
- **Compact Format**: All sensor states on one line for easy parsing

#### State Transition Logging

```cpp
void debugLogStateChange(bridgeState newState) {
    if (bridgeConfig.isStateLoggingEnabled()) {
        Serial.print(F("[STATE] "));
        Serial.print(millis());
        Serial.print(F("ms: Changing to "));
        Serial.println(getStateName(newState));
    }
}
```

**State Change Tracking:**
- **Previous State Management**: Tracks state changes using `previousState` variable
- **String Conversion**: Uses `getStateName()` helper for human-readable state names
- **Change Detection**: Only logs when actual state transitions occur

---

## State Machine Operation

### Enhanced State Machine Logic

The state machine has been transformed from a simple switch-based system to a sophisticated control system that integrates configuration management, conditional logging, and robust state tracking.

#### State Change Detection

```cpp
void stateMachine(bridgeState state) {
    // Log state change if different from previous state
    if (state != previousState && bridgeConfig.isStateLoggingEnabled()) {
        debugLogStateChange(state);
        previousState = state;
    }
    
    switch (state) {
        // ... state handling
    }
}
```

**State Tracking Mechanism:**
1. **Change Detection**: Compares current state with `previousState` to detect transitions
2. **Conditional Logging**: Only logs if state logging is enabled in configuration
3. **State Updates**: Updates `previousState` after logging to prevent duplicate messages
4. **Automatic Tracking**: No manual state change calls required throughout the code

#### Configurable State Timing

The state machine now uses configurable timing parameters instead of hardcoded constants:

```cpp
// Old Implementation
bool timerFinished() { return millis() - startTime > 3000; }  // Hardcoded 3 seconds

// New Implementation
bool timerFinished() { 
    return millis() - startTime > bridgeConfig.getActionDelay(); 
}
```

**Benefits of Configurable Timing:**
- **Field Calibration**: Adjust timing without recompilation
- **Testing Flexibility**: Use shorter delays for rapid testing
- **Environmental Adaptation**: Tune timing for different operational conditions
- **Safety Margins**: Easily adjust timeouts for various safety requirements

#### One-Time State Initialization

Critical states now implement one-time initialization to prevent repeated execution of setup operations:

```cpp
case prepareRaise:
    trafficLight.cycle(0);
    
    static bool prepRaiseInitialized = false;        // One-time flag
    if (!prepRaiseInitialized) {
        startTime = millis();                        // Initialize timer
        if (bridgeConfig.isSensorLoggingEnabled()) {
            debugLogSensors();                       // Log initial state
        }
        prepRaiseInitialized = true;                 // Mark as initialized
    }
    
    if (eStopPressed()) {
        prepRaiseInitialized = false;                // Reset on state exit
        currentState = emergencyLower;
    }
    // ...
```

**Initialization Pattern Benefits:**
1. **Efficiency**: Prevents repeated timer resets and sensor logging
2. **Accuracy**: Ensures timing starts exactly when state is entered
3. **Resource Conservation**: Reduces unnecessary function calls and serial output
4. **State Cleanup**: Resets initialization flags when exiting states

### Enhanced Helper Functions

Helper functions now integrate configuration parameters and conditional behavior:

#### Boat Detection with Configurable Distance

```cpp
bool boatDetected() { 
    int distance = sonicSensor.poll_cm();
    return (distance > 0 && distance < bridgeConfig.getBoatDetectionDistance()); 
}
```

**Enhanced Logic:**
- **Zero Check**: Validates sensor reading before comparison
- **Configurable Threshold**: Uses runtime-configurable detection distance
- **Range Validation**: Ensures positive distance reading

#### Emergency Stop with Configuration Control

```cpp
bool eStopPressed() { 
    return bridgeConfig.isEmergencyStopEnabled() ? 
           digitalRead(Pin_EStop) == HIGH : false; 
}
```

**Configuration Integration:**
- **Feature Toggle**: Can disable E-stop functionality for testing
- **Safety Override**: Maintains hardware connection while providing software control
- **Test Safety**: Allows testing without triggering emergency conditions

---

## EEPROM Storage Implementation

### Memory Layout and Management

The EEPROM storage system implements a structured approach to non-volatile memory management with integrity checking and safe defaults.

#### Memory Organization

```
EEPROM Memory Layout:
┌─────────────────┬────────────────┬───────────────┬──────────────┐
│ Magic Number    │ Timing Config  │ Sensor Config │ Feature Flags│
│ (4 bytes)       │ (16 bytes)     │ (8 bytes)     │ (8 bytes)    │
├─────────────────┼────────────────┼───────────────┼──────────────┤
│ Motor Config    │ Reserved Space │                │              │
│ (4 bytes)       │ (16 bytes)     │                │              │
└─────────────────┴────────────────┴───────────────┴──────────────┘
Total: ~56 bytes + safety buffer
```

#### Data Integrity Mechanisms

1. **Magic Number Validation**: 32-bit marker at the beginning of the configuration
2. **Range Checking**: Every parameter validated against safe operating limits
3. **Structure Versioning**: Magic number can be changed to force configuration updates
4. **Safe Defaults**: Fallback to known-good values if EEPROM data is invalid

#### EEPROM Operations

**Write Operation:**
```cpp
bool BridgeConfigManager::saveConfig() {
    config.magic = CONFIG_MAGIC;                    // Mark as valid
    EEPROM.put(EEPROM_START_ADDR, config);          // Atomic write
    
    #if defined(ESP32) || defined(ESP8266)
        return EEPROM.commit();                     // Platform-specific commit
    #endif
    return true;
}
```

**Read Operation:**
```cpp
bool BridgeConfigManager::loadConfig() {
    uint32_t magic;
    EEPROM.get(EEPROM_START_ADDR, magic);           // Verify integrity first
    
    if (magic != CONFIG_MAGIC) return false;       // Invalid data
    
    EEPROM.get(EEPROM_START_ADDR, config);          // Load full structure
    
    // Validate each parameter...
    return true;
}
```

---

## Serial Command Interface

### Command Processing Architecture

The serial command interface provides a comprehensive method for runtime configuration modification through a text-based command system.

#### Command Parser Implementation

```cpp
bool BridgeConfigManager::processConfigCommand(String command) {
    command.trim();                                 // Remove whitespace
    command.toLowerCase();                          // Normalize case
    
    if (command == "help") {
        printConfigMenu();
        return true;
    }
    
    // Parse parameter=value format
    int equalPos = command.indexOf('=');
    if (equalPos > 0) {
        String param = command.substring(0, equalPos);
        String value = command.substring(equalPos + 1);
        
        param.trim();
        value.trim();
        
        // Process parameter based on type...
    }
    
    return false;  // Unknown command
}
```

**Command Processing Flow:**
1. **Input Sanitization**: Removes whitespace and normalizes case
2. **Command Classification**: Identifies single commands vs. parameter assignments
3. **Parameter Extraction**: Separates parameter names from values
4. **Type-Specific Processing**: Handles different data types appropriately
5. **Validation**: Ensures parameter values fall within safe ranges
6. **Feedback**: Provides confirmation or error messages

#### Command Categories

**1. Information Commands:**
- `show`: Displays current configuration in formatted table
- `help`: Shows complete command reference with examples

**2. Configuration Management:**
- `save`: Commits current configuration to EEPROM
- `reset`: Restores factory default values

**3. Parameter Modification:**
- Timing: `action_delay=3000`, `move_timeout=8000`
- Distance: `boat_detect=100`, `area_clear=150`
- Motor: `speed_fast=255`, `speed_slow=128`
- Features: `debug_log=on`, `sensor_log=off`

#### Input Validation Framework

Each parameter type implements specific validation rules:

```cpp
// Timing Parameter Validation
if (param == "action_delay") {
    uint32_t val = value.toInt();
    if (val >= 100 && val <= 60000) {              // 100ms to 60s range
        setActionDelay(val);
        Serial.print(F("Action delay set to: ")); 
        Serial.println(val);
        return true;
    }
}

// Boolean Parameter Validation
else if (param == "debug_log") {
    bool enable = (value == "on" || value == "true" || value == "1");
    setDebugLogging(enable);
    Serial.print(F("Debug logging: ")); 
    Serial.println(enable ? F("ON") : F("OFF"));
    return true;
}
```

**Validation Principles:**
- **Range Checking**: Numeric parameters must fall within safe operational limits
- **Type Conversion**: Automatic conversion from string to appropriate data type
- **Flexible Boolean**: Accepts multiple formats for boolean values (on/off, true/false, 1/0)
- **Immediate Feedback**: Confirms successful changes or reports errors
- **Safety First**: Rejects any parameter that could compromise system operation

---

## Code Architecture and Flow

### Initialization Sequence

The system follows a specific initialization order to ensure all components are properly configured:

```
1. Serial Communication Setup (115200 baud)
2. Configuration System Initialization
   ├── EEPROM.begin()
   ├── Load existing configuration or reset to defaults
   ├── Validate all parameters
   └── Display current configuration
3. Hardware Module Initialization
   ├── Traffic light system
   ├── Web server components
   ├── Sonic sensors (primary and secondary)
   └── Motor controller
4. State Machine Initialization
   ├── Set initial state (lowered)
   ├── Log system startup
   └── Perform initial sensor reading
5. Start main operation loop
```

### Main Operation Loop

The main loop integrates multiple concurrent operations:

```cpp
void loop() {
    server.handleClient();                          // Web server processing
    
    if (Serial.available()) {                       // Command processing
        String command = Serial.readStringUntil('\n');
        bridgeConfig.processConfigCommand(command);
    }
    
    stateMachine(currentState);                     // State machine execution
    
    static unsigned long lastDebugLog = 0;          // Periodic logging
    unsigned long debugInterval = bridgeConfig.getDebugLogInterval();
    
    if (millis() - lastDebugLog >= debugInterval) {
        debugLogSensors();
        lastDebugLog = millis();
    }
    
    delay(50);                                      // System pacing
}
```

**Loop Components:**
1. **Web Server**: Handles HTTP requests for web interface
2. **Serial Commands**: Processes configuration commands from serial port
3. **State Machine**: Executes bridge control logic based on current state
4. **Periodic Logging**: Outputs sensor status at configurable intervals
5. **System Pacing**: Prevents overwhelming the system with excessive loop speed

### Memory Management

The system implements several memory optimization techniques:

#### Flash Memory Usage (`F()` Macro)
```cpp
Serial.print(F("[DEBUG] "));  // String stored in flash, not RAM
```

#### Static Variable Optimization
```cpp
static bool prepRaiseInitialized = false;  // Persistent between function calls
```

#### Structure Packing
```cpp
struct BridgeConfig {
    uint32_t magic;          // 4 bytes
    uint32_t actionDelay;    // 4 bytes
    // ... efficiently packed
};
```

---

## Testing and Calibration Guide

### Quick Test Configuration

For rapid development and testing, use these optimized settings:

```
action_delay=1000       # Fast state transitions
move_timeout=3000       # Short safety timeouts
emergency_delay=500     # Quick emergency operations
debug_interval=2000     # Frequent status updates
debug_log=on            # Enable all debugging
sensor_log=on
state_log=on
save
```

### Sensor Calibration Process

#### Boat Detection Tuning
1. **Baseline Setup**: Place system in normal operation position
2. **Distance Measurement**: Use actual boat or obstacle at desired detection point
3. **Parameter Adjustment**: 
   ```
   boat_detect=85          # Start with conservative value
   area_clear=200          # Ensure significant gap
   ```
4. **Testing**: Verify detection triggers reliably
5. **Fine-tuning**: Adjust values based on environmental conditions

#### Sonic Sensor Range Validation
```
sonic_min=5             # Account for sensor dead zone
sonic_max=350           # Practical maximum for environment
show                    # Verify settings
```

### Performance Testing

#### Motor Speed Optimization
```
speed_fast=200          # Start with reduced speed for testing
speed_slow=100          # Ensure controllable movement
```

#### Timing Optimization
```
action_delay=2000       # Balance safety and responsiveness
move_timeout=10000      # Allow for slower mechanical systems
emergency_delay=1500    # Quick but complete emergency actions
```

### Safety Testing Protocol

1. **Disable Safety Features Temporarily**:
   ```
   emergency_stop=off
   motion_timeout=off
   ```

2. **Test Individual Components**:
   - Verify motor directions and speeds
   - Test limit switch responses
   - Validate sonic sensor readings

3. **Re-enable Safety Features**:
   ```
   emergency_stop=on
   motion_timeout=on
   save
   ```

4. **Verify Emergency Procedures**:
   - Test E-stop functionality
   - Verify timeout behavior
   - Confirm safe state recovery

---

## Troubleshooting Reference

### Common Configuration Issues

#### Configuration Not Persisting
**Symptoms**: Settings reset after power cycle
**Diagnosis**: Check for successful save confirmation
**Solution**:
```
show                    # Verify current settings
save                    # Explicit save command
# Look for "Configuration saved to EEPROM" message
```

#### Invalid Parameter Ranges
**Symptoms**: Parameter changes rejected
**Diagnosis**: Values outside safe operating ranges
**Solution**: Check command reference for valid ranges
```
help                    # Display valid ranges
```

#### Corrupted EEPROM Data
**Symptoms**: System starts with unexpected values
**Diagnosis**: EEPROM data corruption
**Solution**:
```
reset                   # Reset to factory defaults
save                    # Save clean configuration
```

### Debug Output Analysis

#### State Machine Issues
Look for patterns in state transition logs:
```
[STATE] 12345ms: Changing to PREPARE_RAISE
[STATE] 15345ms: Changing to RAISING
[STATE] 18234ms: Changing to RAISED
```

**Normal Pattern**: Sequential state progression with appropriate timing
**Problem Indicators**: Stuck states, rapid cycling, unexpected transitions

#### Sensor Reading Issues
Monitor sensor logs for anomalies:
```
[SENSORS] 12345ms - EStop:CLEAR | TopLimit:CLEAR | BottomLimit:HIT | Sonic:245cm
```

**Problem Indicators**:
- Conflicting limit switches (both HIT simultaneously)
- Erratic sonic readings (rapid fluctuations)
- Sensor readings outside expected ranges

#### Timing Problems
Check debug intervals and operation timing:
```
[DEBUG] 12345ms: Boat detected, preparing to raise bridge
[DEBUG] 15345ms: Preparation timer completed, starting to raise bridge
```

**Problem Indicators**:
- Delayed responses to sensor inputs
- Premature timeouts
- Irregular timing intervals

### Recovery Procedures

#### Emergency Configuration Reset
If system becomes unresponsive or exhibits dangerous behavior:
1. Power cycle the system
2. Immediately send: `reset`
3. Verify with: `show`
4. Save with: `save`

#### Selective Feature Disable
For troubleshooting specific issues:
```
emergency_stop=off      # Disable if E-stop interfering with testing
motion_timeout=off      # Disable if timeouts too aggressive
debug_log=off           # Reduce output if overwhelming
```

#### Communication Recovery
If serial communication becomes garbled:
1. Reset Arduino/ESP32
2. Verify baud rate (115200)
3. Send `help` to test communication
4. Use `show` to verify system state

---

This comprehensive documentation provides complete understanding of the enhanced bridge control system, covering every aspect from high-level architecture to specific troubleshooting procedures. The system now operates as a professional-grade control platform with extensive configurability and diagnostic capabilities.