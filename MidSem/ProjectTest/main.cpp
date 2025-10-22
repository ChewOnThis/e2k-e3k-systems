#include "main.h"

TrafficModule trafficLight (Pin_TrafficRed, Pin_TrafficYellow, Pin_TrafficGreen);
Sonic sonic1 (Pin_SonicTrigger_1, Pin_SonicEcho_1);
Sonic sonic2 (Pin_SonicTrigger_2, Pin_SonicEcho_2);
Sonic sonicSensor (Pin_SonicTrigger_1, Pin_SonicEcho_1); // Primary sensor for boat detection
Motor motor (Pin_DIR1, Pin_DIR2);


//bridgeState currentState;

void setup(){
    Serial.begin(115200);
    
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
    
    // Run the state machine
    stateMachine(currentState);
    
    // Periodic debug sensor logging every 10 seconds
    static unsigned long lastDebugLog = 0;
    if (millis() - lastDebugLog > 10000) {
        debugLogSensors();
        lastDebugLog = millis();
    }
    
    delay(100); // Small delay to prevent overwhelming the system
}