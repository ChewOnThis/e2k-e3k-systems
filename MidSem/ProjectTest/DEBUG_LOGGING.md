# Debug Logging System

## Overview
Enhanced debug information logging has been added to the bridge control system to help with troubleshooting and monitoring system behavior. The system now includes improved state management and memory optimization.

## Debug Functions

### `debugLog(const char* message)`
Logs a general debug message with timestamp.
- **Format**: `[DEBUG] {timestamp}ms: {message}`
- **Example**: `[DEBUG] 15234ms: Boat detected, preparing to raise bridge`
- **Memory**: Uses F() macro to store strings in flash memory

### `debugLogSensors()`
Logs the current state of all sensors.
- **Format**: `[SENSORS] {timestamp}ms - EStop:{state} | TopLimit:{state} | BottomLimit:{state} | Sonic:{distance}cm`
- **Example**: `[SENSORS] 15234ms - EStop:CLEAR | TopLimit:CLEAR | BottomLimit:HIT | Sonic:245cm`
- **Memory**: Optimized with F() macro for reduced RAM usage

### `debugLogStateChange(bridgeState newState)`
Logs when the state machine changes to a new state.
- **Format**: `[STATE] {timestamp}ms: Changing to {STATE_NAME}`
- **Example**: `[STATE] 15234ms: Changing to PREPARE_RAISE`

### `getStateName(bridgeState state)`
Helper function that returns the string name of a bridge state.
- **Returns**: Constant string representing the state name
- **Safety**: Includes bounds checking to prevent array overflow

## When Debug Information is Logged

### System Startup
- System initialization message
- Initial sensor readings
- Starting state confirmation

### State Transitions
- Automatic logging when state changes
- Sensor readings during preparation states
- Motor start/stop operations

### Critical Events
- Emergency stop activations
- Limit switch activations
- Motion timeouts
- Emergency procedures

### Periodic Monitoring
- Sensor status every 10 seconds during operation
- Helps track system health over time

## Example Debug Output

```
[DEBUG] 1234ms: System starting - initializing bridge control
[DEBUG] 1456ms: System initialized - bridge state set to LOWERED
[SENSORS] 1457ms - EStop:CLEAR | TopLimit:CLEAR | BottomLimit:HIT | Sonic:245cm
[DEBUG] 15234ms: Boat detected, preparing to raise bridge
[STATE] 15235ms: Changing to PREPARE_RAISE
[SENSORS] 15236ms - EStop:CLEAR | TopLimit:CLEAR | BottomLimit:HIT | Sonic:89cm
[DEBUG] 18234ms: Preparation timer completed, starting to raise bridge
[STATE] 18235ms: Changing to RAISING
[DEBUG] 18236ms: Bridge motor started - raising
[DEBUG] 22145ms: Top limit switch reached
[STATE] 22146ms: Changing to RAISED
[SENSORS] 32234ms - EStop:CLEAR | TopLimit:HIT | BottomLimit:CLEAR | Sonic:156cm
```

## Troubleshooting with Debug Logs

### Common Issues to Look For

1. **Sensor Conflicts**: Both limit switches active simultaneously
2. **Stuck States**: State not changing when expected
3. **Sensor Noise**: Rapidly changing sonic sensor readings
4. **Timing Issues**: Motion timeouts occurring frequently
5. **Emergency Conditions**: Unexpected E-stop activations

### Reading the Logs

- **Timestamps** help correlate events and measure timing
- **Sensor logs** show the physical state of the system
- **State changes** track the decision-making process
- **Debug messages** provide context for why actions are taken

## Code Improvements

### State Management
- **Improved State Tracking**: Uses `previousState` variable instead of static local variable for better state management
- **One-time Initialization**: States that require initialization (motor start, timer setup) now use static flags to ensure operations happen only once per state entry
- **Cleaner State Logic**: Removed redundant Serial.println statements, consolidating output through debug functions

### Memory Optimization
- **Flash Memory Usage**: Uses F() macro to store constant strings in flash memory instead of RAM
- **Reduced Duplication**: Centralized state name handling with `getStateName()` function
- **Bounds Checking**: Added safety checks to prevent array access violations

### Performance Improvements
- **Reduced Loop Delay**: Decreased main loop delay from 100ms to 50ms for more responsive operation
- **Efficient Timing**: Better use of timing constants and consistent interval checking
- **Optimized Logging**: Reduced redundant debug output while maintaining essential information

### Code Quality
- **Consistent Formatting**: Improved code structure and readability
- **Better Function Organization**: Logical grouping of related functions
- **Enhanced Comments**: More descriptive comments explaining state behavior

## Configuration

The debug logging is enabled by default. To modify behavior:

1. **Change logging frequency**: Modify the `debugInterval` constant in `main.cpp`
2. **Add custom debug messages**: Use `debugLog("your message")` in your code
3. **Disable specific logging**: Comment out `debugLogSensors()` calls as needed
4. **Memory optimization**: All debug strings use F() macro for flash storage

## Files Modified

- `StateMachine.h` - Added debug function declarations and helper function
- `StateMachine.cpp` - Implemented optimized debug functions, improved state management, and memory usage
- `main.cpp` - Enhanced main loop with better timing and state machine integration
- `DEBUG_LOGGING.md` - Updated documentation with improvements and new features