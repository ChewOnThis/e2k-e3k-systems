#ifndef WEBPAGE_H
#define WEBPAGE_H
#include <Arduino.h>
#include "main.h"

#define ssid "ESP32WA8"
#define password "12345678"

void setup_routes();
void handle_root();
void handle_state();
void handle_switch1_on();
void handle_switch2_on();
void handle_switch1_off();
void handle_switch2_off();
String createHTML();

#endif

