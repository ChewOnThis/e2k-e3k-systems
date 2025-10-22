# Bridge Control System - Quick Reference

## Overview
Enhanced bridge control system with runtime configuration, EEPROM storage, and comprehensive debug logging. All parameters can be modified without recompiling code.

## Quick Start Commands

### View Current Configuration
```
show                    # Display all current settings
help                    # Show command reference
```

### Common Configuration Changes
```
# Fast testing setup
action_delay=1000
move_timeout=3000
debug_interval=2000
save

# Boat detection calibration
boat_detect=100         # Adjust based on actual distance
area_clear=150          # Must be > boat_detect
save

# Debug control
debug_log=on            # Enable/disable debug messages
sensor_log=on           # Enable/disable sensor status
state_log=on            # Enable/disable state transitions
save
```

### Emergency Recovery
```
reset                   # Reset all settings to defaults
save                    # Save the reset configuration
```

## Key Features

### Configuration Storage
- **EEPROM Persistence**: Settings survive power cycles
- **Magic Number Validation**: Detects corrupted configuration
- **Safe Defaults**: Automatic fallback to known-good values
- **Range Validation**: Prevents invalid parameter values

### Debug Logging Categories
- **Debug Log**: General operational messages (`debugLog()`)
- **Sensor Log**: Hardware status reports (`debugLogSensors()`)
- **State Log**: State machine transitions (`debugLogStateChange()`)

### Runtime Parameters
- **Timing**: Action delays, timeouts, emergency durations
- **Distances**: Boat detection, area clear thresholds
- **Motor**: Speed settings and direction control
- **Safety**: Emergency stop, timeout enables/disables

## Code Integration

### Configuration Access
```cpp
// Use configuration values instead of constants
if (millis() - startTime > bridgeConfig.getActionDelay()) {
    // Timer finished
}

if (distance < bridgeConfig.getBoatDetectionDistance()) {
    // Boat detected
}

motor.run(bridgeConfig.getMotorSpeedFast(), bridgeConfig.getMotorDirection1());
```

### Conditional Logging
```cpp
// Logging respects configuration flags
if (bridgeConfig.isDebugLoggingEnabled()) {
    debugLog("Operation completed");
}

// Automatic configuration checking in logging functions
debugLogSensors();  // Only outputs if sensor logging enabled
```

### State Machine Integration
```cpp
// State changes automatically logged if enabled
stateMachine(currentState);

// Helper functions use configurable parameters
bool boatDetected() { 
    int distance = sonicSensor.poll_cm();
    return (distance > 0 && distance < bridgeConfig.getBoatDetectionDistance()); 
}
```

## Files Structure

### Core Files
- **`BridgeConfig.h`**: Configuration structure and class definitions
- **`BridgeConfig.cpp`**: EEPROM management and command processing
- **`StateMachine.h/cpp`**: Enhanced state machine with configurable parameters
- **`main.h/cpp`**: System initialization and command interface

### Documentation
- **`BRIDGE_SYSTEM_TECHNICAL_DOCS.md`**: Comprehensive technical documentation
- **`README_QUICK_REFERENCE.md`**: This quick reference guide

## Default Configuration Values

```cpp
// Timing (milliseconds)
actionDelay = 3000          // State transition delay
moveTimeout = 8000          // Motor operation timeout
emergencyDelay = 2000       // Emergency operation duration
debugLogInterval = 10000    // Sensor logging frequency

// Distances (centimeters)
boatDetectionDistance = 100 // Boat presence threshold
areaClearDistance = 150     // Area clear threshold
sonicSensorMinRange = 2     // Minimum valid sensor reading
sonicSensorMaxRange = 400   // Maximum valid sensor reading

// Motor settings
motorSpeedFast = 255        // High speed (0-255)
motorSpeedSlow = 128        // Low speed (0-255)
motorDirection1 = 1         // Up/raise direction
motorDirection2 = 0         // Down/lower direction

// Feature flags (all enabled by default)
enableDebugLogging = true
enableSensorLogging = true
enableStateLogging = true
enableEmergencyStop = true
enableMotionTimeout = true
```

## Troubleshooting Quick Fixes

### System Not Responding
```
reset
save
```

### Too Much Debug Output
```
debug_log=off
sensor_log=off
save
```

### Timing Issues
```
action_delay=5000       # Slower state changes
move_timeout=15000      # Longer motor timeouts
save
```

### Sensor Calibration
```
boat_detect=80          # Adjust for environment
area_clear=200          # Ensure good separation
sonic_min=5             # Account for sensor limits
sonic_max=300           # Practical maximum range
save
```

For detailed information, see `BRIDGE_SYSTEM_TECHNICAL_DOCS.md`.