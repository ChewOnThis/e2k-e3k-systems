/*
 * MAIN BRIDGE CONTROL SYSTEM - Entry Point and Core Integration
 * 
 * PURPOSE:
 * ========
 * This is the primary entry point for the configurable bridge control system.
 * It initializes all subsystems, manages the main operation loop, and provides
 * integration between the configuration system and the state machine.
 * 
 * SYSTEM ARCHITECTURE:
 * ====================
 * 
 * HARDWARE COMPONENTS:
 * - Traffic Light Module: Controls boat/vehicle traffic signals
 * - Sonic Sensors (2): Distance measurement for boat detection and clearance
 * - DC Motor: Bridge raising/lowering mechanism
 * - Serial Interface: Configuration and debugging communication
 * 
 * SOFTWARE MODULES:
 * - BridgeConfig: EEPROM-based configuration management
 * - StateMachine: Bridge operation state control
 * - TrafficModule: Traffic light control logic
 * - SonicSensor: Distance measurement and validation
 * - DCMotor: Motor control with direction and speed
 * - WebPage: Optional web interface (if enabled)
 * 
 * INITIALIZATION SEQUENCE:
 * ========================
 * 
 * 1. SERIAL COMMUNICATION SETUP:
 *    - Initialize Serial at 115200 baud for debugging and configuration
 *    - Provides real-time system status and command interface
 * 
 * 2. CONFIGURATION SYSTEM STARTUP:
 *    - bridgeConfig.begin() - Load parameters from EEPROM
 *    - Validate configuration integrity with magic number
 *    - Fall back to safe defaults if EEPROM is corrupted
 *    - Display current configuration for verification
 * 
 * 3. HARDWARE INITIALIZATION:
 *    - Traffic light module setup with pin configuration
 *    - Sonic sensor initialization for distance measurement
 *    - Motor controller setup with direction pins
 *    - Web interface initialization (optional feature)
 * 
 * 4. SYSTEM STATE INITIALIZATION:
 *    - Set initial bridge state to LOWERED (safe default)
 *    - Perform initial sensor reading and logging
 *    - System ready for operation
 * 
 * MAIN OPERATION LOOP:
 * ====================
 * 
 * The loop() function performs continuous monitoring and control:
 * 
 * 1. STATE MACHINE EXECUTION:
 *    - Call runStateMachine() to process current bridge state
 *    - Handle boat detection, bridge movement, and safety checks
 *    - All timing and thresholds controlled by configuration parameters
 * 
 * 2. CONFIGURATION COMMAND PROCESSING:
 *    - Check for incoming serial commands
 *    - Process parameter modification requests
 *    - Provide immediate feedback for configuration changes
 *    - Enable real-time system tuning without code recompilation
 * 
 * 3. CONTINUOUS MONITORING:
 *    - Periodic sensor status logging (if enabled)
 *    - Emergency stop monitoring
 *    - System health checks
 * 
 * INTEGRATION WITH CONFIGURATION SYSTEM:
 * =======================================
 * 
 * The main loop integrates seamlessly with the configuration system:
 * 
 * - All operational parameters sourced from bridgeConfig
 * - Real-time parameter changes take effect immediately
 * - No system restart required for configuration updates
 * - Persistent storage ensures settings survive power cycles
 * 
 * COMMAND INTERFACE INTEGRATION:
 * ==============================
 * 
 * Serial commands processed in main loop:
 * - "show" - Display current configuration
 * - "help" - Show command reference
 * - "parameter=value" - Modify specific settings
 * - "save" - Store changes to EEPROM
 * - "reset" - Restore factory defaults
 * 
 * DEBUG AND MONITORING:
 * =====================
 * 
 * Configurable debug output levels:
 * - General debug messages (system status, state changes)
 * - Sensor logging (periodic distance readings)
 * - State logging (detailed state machine transitions)
 * 
 * All debug output can be enabled/disabled via configuration:
 * - enableDebugLogging: General system messages
 * - enableSensorLogging: Periodic sensor status
 * - enableStateLogging: State transition details
 * 
 * SAFETY FEATURES:
 * ================
 * 
 * 1. SAFE STARTUP STATE:
 *    - Bridge always starts in LOWERED position
 *    - Initial sensor validation before operation
 *    - Configuration validation before accepting commands
 * 
 * 2. EMERGENCY HANDLING:
 *    - Emergency stop functionality (if enabled)
 *    - Motor timeout protection
 *    - Sensor validation and fallback behavior
 * 
 * 3. PARAMETER VALIDATION:
 *    - All configuration changes validated before acceptance
 *    - Safe operating ranges enforced
 *    - Invalid configurations rejected with error messages
 * 
 * FUTURE EXPANSION:
 * =================
 * 
 * The modular architecture supports easy extension:
 * - Additional sensors can be integrated via configuration
 * - New safety features can be added as configurable options
 * - Web interface can be enhanced with full parameter control
 * - Remote monitoring and control capabilities
 */

#include "main.h"

TrafficModule trafficLight (Pin_TrafficRed, Pin_TrafficYellow, Pin_TrafficGreen);
Sonic sonic1 (Pin_SonicTrigger_1, Pin_SonicEcho_1);
Sonic sonic2 (Pin_SonicTrigger_2, Pin_SonicEcho_2);
Sonic sonicSensor (Pin_SonicTrigger_1, Pin_SonicEcho_1); // Primary sensor for boat detection
Motor motor (Pin_DIR1, Pin_DIR2);


//bridgeState currentState;

void setup(){
    Serial.begin(115200);
    
    // Initialize configuration system
    bridgeConfig.begin();
    
    // Debug initialization message
    debugLog("System starting - initializing bridge control");

    trafficLight.init();
    webPage_init();
    sonic1.init();
    sonic2.init();
    sonicSensor.init();
    motor.init();

    currentState = lowered;
    debugLog("System initialized - bridge state set to LOWERED");
    debugLogSensors();
    
    server.handleClient();
}

void loop() {
    server.handleClient();
    
    // Process configuration commands from Serial
    if (Serial.available()) {
        String command = Serial.readStringUntil('\n');
        bridgeConfig.processConfigCommand(command);
    }
    
    // Run the state machine with current state
    stateMachine(currentState);
    
    // Periodic debug sensor logging using configurable interval
    static unsigned long lastDebugLog = 0;
    unsigned long debugInterval = bridgeConfig.getDebugLogInterval();
    
    if (millis() - lastDebugLog >= debugInterval) {
        debugLogSensors();
        lastDebugLog = millis();
    }
    
    // Small delay to prevent overwhelming the system
    delay(50);
}