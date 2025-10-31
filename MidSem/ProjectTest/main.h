#ifndef MAIN_H
#define MAIN_H

#include <Arduino.h>
#include "WebPage.h"
#include "StateMachine.h"
#include "DCMotor.h"
#include "SonicSensor.h"
#include "TrafficLight.h"
#include "BridgeConfig.h"

#define Pin_PhotoCell        4
#define Pin_DIR2            12
#define Pin_DIR1            13 
#define Pin_BoatLight       14
#define Pin_LS_1            16
#define Pin_LS_2            17
#define Pin_Street          18
#define Pin_Status          19
#define Pin_Buzzer          21
#define Pin_BridgeMove      22
#define Pin_EStop           23
#define Pin_TrafficRed      25
#define Pin_TrafficYellow   26
#define Pin_TrafficGreen    27
#define Pin_SonicTrigger_2  32
#define Pin_SonicTrigger_1  33
#define Pin_SonicEcho_2     36
#define Pin_SonicEcho_1     39

#define MotorSpeed_Fast
#define MotorSpeed_Slow

#define WebServer_PollingRate   1000 //ms
#define WebServer_SSID          "ESP32WA8"
#define WebServer_Password      "12345678"

extern TrafficModule traffic;
extern Sonic sonic1;
extern Sonic sonic2;
extern Motor motor;

#endif