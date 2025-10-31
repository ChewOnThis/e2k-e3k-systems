#ifndef WEBPAGE_H
#define WEBPAGE_H

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>

#define SSID "ESP32WA8"
#define PASSWORD "12345678"

extern IPAddress local_ip;
extern IPAddress gateway;
extern IPAddress subnet;
extern WebServer server;

String stateName();

void webPage_init();
void webPage_setupRoutes();

//Routes
void handle_root();
void handle_stateUpdate();
void handle_eStop_on();
void handle_eStop_off();
void handle_debug_on();
void handle_debug_off();
void handle_switchState();
void handle_activateBridge_raise();
void handle_activateBridge_lower();

String createHTML();

#endif