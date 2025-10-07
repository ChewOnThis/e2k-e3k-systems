#include "WebPage.h"

IPAddress local_ip  (192, 168, 1, 1);
IPAddress gateway   (192, 168, 1, 1);
IPAddress subnet    (255, 255, 255, 0);
WebServer server(80);

void webPage_init() {
    WiFi.softAPConfig(local_ip, gateway, subnet);
    delay(100);
    webPage_setupRoutes();
    server.begin();
    Serial.println("HTTP Server Started");
}

void webPage_setupRoutes(){
    server.on("/", handle_root);
    server.on("/state", handle_stateUpdate);
    server.on("/eStop/on", handle_eStop_on);
    server.on("/debug/on", handle_debug_on);
    server.on("/debug/off", handle_debug_off);
    server.on("/switchState", handle_switchState);
    server.on("/activateBridge/raise", handle_activateBridge_raise);
    server.on("/activateBridge/lower", handle_activateBridge_lower);
}

//Routes
void handle_root(){ server.send(200, "text/html", createHTML()); }
void handle_stateUpdate(){
    String json = "{";

    json += "}";
    server.send(200, "application/json", json);
}
void handle_eStop_on(){
    //state variable change
    //other gpio stuff
    Serial.println("ESTOP Status : ON");
    server.send(200, "text/html" ,createHTML());
}
void handle_eStop_off(){
    //state variable change
    //other gpio stuff
    Serial.println("ESTOP Status : OFF");
    server.send(200, "text/html" ,createHTML());
}
void handle_debug_on(){
    //state variable change
    //other gpio stuff
    Serial.println("DEBUG Status : ON");
    server.send(200, "text/html" ,createHTML());
}
void handle_debug_off(){
    //state variable change
    //other gpio stuff
    Serial.println("DEBUG Status : OFF");
    server.send(200, "text/html" ,createHTML());
}
void handle_switchState(){
    //state variable change
    //other gpio stuff
    Serial.print("SWITCHED TO NEXT STATE : ");
    // Serial.print(bridgeState++);
    // Serial.print(stateMachine.getKey(bridgeState));
    server.send(200, "text/html" ,createHTML());
}
void handle_activateBridge_raise(){
    //state variable change
    //other gpio stuff
    Serial.println("ACTIVATE BRIDGE Status : RAISE");
    server.send(200, "text/html" ,createHTML());
}
void handle_activateBridge_lower(){
    //state variable change
    //other gpio stuff
    Serial.println("ACTIVATE BRIDGE Status : LOWER");
    server.send(200, "text/html" ,createHTML());
}

//TODO Convert HTML
String createHTML(){
    String str = "";

    return str;
}