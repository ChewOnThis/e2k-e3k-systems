#include "main.h"

TrafficModule traffic (Pin_TrafficRed, Pin_TrafficYellow, Pin_TrafficGreen);
Sonic sonic1 (Pin_SonicTrigger_1, Pin_SonicEcho_1);
Sonic sonic2 (Pin_SonicTrigger_2, Pin_SonicEcho_2);
Motor motor (Pin_DIR1, Pin_DIR2);


bridgeState currentState;

void setup(){
    Serial.begin(115200);

    traffic.init();
    webPage_init();
    sonic1.init();
    sonic2.init();
    motor.init();

    currentState = lowered;
    server.handleClient();
}

void loop() {
    client.handle()
}