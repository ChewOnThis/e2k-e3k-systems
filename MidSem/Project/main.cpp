#include "esp_attr.h"
#include <Arduino.h>
#include <esp32-hal-gpio.h>
#include "main.h"


bridgeState currentState;
TrafficModule traffic (Pin_TrafficRed, Pin_TrafficYellow, Pin_TrafficGreen);
Sonic sonic1 (Pin_SonicTrigger_1, Pin_SonicEcho_1);
Sonic sonic2 (Pin_SonicTrigger_2, Pin_SonicEcho_2);
Motor motor (Pin_DIR1, Pin_DIR2);
GlobalStates states;
bool EStop = false;

void setup(){
    Serial.begin(115200);

    attachInterrupt(digitalPinToInterrupt(Pin_LS_Top), disableMotor, FALLING);

    traffic.init();
    webPage_init();
    sonic1.init();
    sonic2.init();
    motor.init();

    initStates();

    currentState = lowered;
}

void IRAM_ATTR disableMotor() {
    motor.disable();
}

void initStates() {
    states.stateSwitch = 0;
    states.activateBridgeBtn = 0;
    states.photoCellState = 0;
    states.sonicState = 0;
    states.trafficState = 0;
    states.debugBtn = 0;
    states.BridgeState = bridgeState::lowered;

    pinMode(Pin_LS_Bottom, INPUT_PULLUP);
    pinMode(Pin_LS_Top, INPUT_PULLUP);
}

void loop() {
    server.handleClient();
    stateMachine(currentState);
    // Serial.print("LS1: " + (String)!digitalRead(Pin_LS_Bottom) + " , ");
    // Serial.println("LS2: " + (String)!digitalRead(Pin_LS_Top));

}