#ifndef WEBPAGE_H
#define WEBPAGE_H
#include <Arduino.h>
#include "main.h"
#include <WebServer.h>
#include <WiFi.h>

#define ssid "ESP32WA8"
#define password "12345678"

extern WebServer server;
extern int switch1, switch2, state1, state2;

void setup_routes();
void handle_root();
void handle_state();
void handle_switch1_on();
void handle_switch2_on();
void handle_switch1_off();
void handle_switch2_off();
String createHTML();

// Added web integration API
void Webpage_init();
void Webpage_poll();
bool Web_reqRaise();
bool Web_reqLower();
bool Web_reqAbort();
void Web_setState(int s1,int s2);

#endif

