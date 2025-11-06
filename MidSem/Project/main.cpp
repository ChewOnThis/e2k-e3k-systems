#include <sys/errno.h>
#include "esp_attr.h"
#include <Arduino.h>
#include <esp32-hal-gpio.h>
#include "main.h"


bridgeState currentState;
TrafficModule traffic (Pin_TrafficRed, Pin_TrafficYellow, Pin_TrafficGreen);
Sonic sonic1 (Pin_SonicTrigger_1, Pin_SonicEcho_1);
Sonic sonic2 (Pin_SonicTrigger_2, Pin_SonicEcho_2);
Motor motor (Pin_DIR1, Pin_DIR2, Pin_Enable);
bool EStop = false;
bool Manual = false;
bool streetLightOn = false;
bool ultrasonics = false;
double sonic1Dist_cm = 0;
double sonic2Dist_cm = 0;

void setup(){
    Serial.begin(115200);
    traffic.init();
    webPage_init();
    sonic1.init();
    sonic2.init();
    motor.init();
    initPins();
    initInterrupts();

    currentState = lowered;
}

void IRAM_ATTR disableMotor() {
    motor.disable();
}

void IRAM_ATTR detectEstop() {
    EStop = true;
}

void initInterrupts() {
    // attachInterrupt(digitalPinToInterrupt(Pin_EStop), detectEstop, FALLING);
}



void initPins() {
    pinMode(Pin_EStop, INPUT_PULLUP);
    pinMode(Pin_LS_Bottom, INPUT_PULLUP);
    pinMode(Pin_LS_Top, INPUT_PULLUP);
    pinMode(Pin_PhotoCell, INPUT);
    pinMode(Pin_Street, OUTPUT);
    pinMode(Pin_Status, OUTPUT);
    pinMode(Pin_Buzzer, OUTPUT);
    pinMode(Pin_BoatLight, OUTPUT);
}

void loop() {


    server.handleClient();
    sonics();
    stateMachine(currentState);
    streetLights();
    // Serial.print("LS1: " + (String)!digitalRead(Pin_LS_Bottom) + " , ");
    // Serial.println("LS2: " + (String)!digitalRead(Pin_LS_Top));

}

int lightLevelSamples = 20;

void sonics() {
  sonic1Dist_cm = sonic1.poll_cm(); 
  sonic2Dist_cm = sonic2.poll_cm(); 
  Serial.println("Dist: " + (String)sonic1Dist_cm + " - " + (String)sonic2Dist_cm); 
  ultrasonics = sonic1Dist_cm < detection_distance || sonic2Dist_cm < detection_distance;
}

void streetLights() {
    int lightLevel = 0;
    for (int i=0; i < lightLevelSamples; i++)
    {
        lightLevel += analogRead(Pin_PhotoCell);
    }
    lightLevel /= lightLevelSamples;

    bool temp = (lightLevel >= 1300) ? LOW : HIGH;
    if (temp != streetLightOn) {
        streetLightOn = temp;
        digitalWrite(Pin_Street, temp);
        // Serial.println("StreetLight change: " + (String)temp + " , " + (String) lightLevel);
    }
    // Serial.println(lightLevel);
}