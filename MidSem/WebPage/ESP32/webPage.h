#ifndef WEBPAGE_H
#define WEBPAGE_H

#include <Wifi.h>
#include <Webserver.h>

#define SSID "ESP32WA8"
#define PASSWORD "12345678"

IPAddress local_ip  (192, 168, 1, 1);
IPAddress gateway   (192, 168, 1, 1);
IPAddress subnet    (255, 255, 255, 0);
Webserver server(80);

void init();
void setup_routes();
String createHTML();

//Routes
void handle_root();
void handle_stateUpdate();
void handle_switch1_on();

#endif