#include <WiFi.h>
#include <WebServer.h>

/*Put your SSID & Password*/
const char* ssid = "ESP32WA8";   // Enter SSID here
const char* password = "12345678";  //Enter Password here
IPAddress local_ip(192,168,1,1);
IPAddress gateway(192,168,1,1);
IPAddress subnet(255,255,255,0);
WebServer server(80);

void setup() {
    Serial.begin(115200);
    delay(100);

    WiFi.softAP(ssid, password);
    WiFi.softAPConfig(local_ip, gateway, subnet);
    delay(100);

    setup_routes();

}

void setup_routes() {
    server.on("/", handle_root);          // serve your HTML
    server.on("/state", handle_state);
    server.on("/switch1on", handle_switch1_on);
    server.on("/switch1off", handle_switch1_off);
    server.on("/switch2on", handle_switch2_on);
    server.on("/switch2off", handle_switch2_off);
}

void handle_state() {
    String json = "{";
    json += "\"switch1\":"; json += switch1 ? "1":"0"; json += ",";
    json += "\"switch2\":"; json += switch2 ? "1":"0"; json += ",";
    json += "\"state1\":";  json += state1  ? "1":"0"; json += ",";
    json += "\"state2\":";  json += state2  ? "1":"0";
    json += "}";
}

void handle_switch1_on(){ 
    switch1 = 1; 
    /* do GPIO etc */ 
    server.send(200, "text/plain", "ok"); 
}

void handle_switch1_off(){ 
    switch1 = 0; 
    /* do GPIO etc */ 
    server.send(200, "text/plain", "ok"); 
}

void handle_switch2_on(){ 
    switch2 = 1; 
    /* do GPIO etc */ 
    server.send(200, "text/plain", "ok"); 
}

void handle_switch2_off(){ 
    switch2 = 0; 
    /* do GPIO etc */ 
    server.send(200, "text/plain", "ok"); 
}

