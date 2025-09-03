#include <WiFi.h>
#include <WebServer.h>

/*Put your SSID & Password*/
const char* ssid = "ESP32WA8";   // Enter SSID here
const char* password = "12345678";  //Enter Password here

WebServer server(80);

uint8_t LED1pin = 23;
bool LED1status = LOW;



void setup() {
  Serial.begin(115200);
  delay(100);
  pinMode(LED1pin, OUTPUT);
  //pinMode(LED2pin, OUTPUT);

  Serial.println("Connecting to ");
  Serial.println(ssid);

  //connect to your local wi-fi network
  WiFi.begin(ssid, password);

  //check wi-fi is connected to wi-fi network
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected..!");
  Serial.print("Got IP: ");
  Serial.println(WiFi.localIP());

  server.on("/", handle_OnConnect);
  server.on("/led1on", handle_led1on);
  server.on("/led1off", handle_led1off);
  //server.on("/led2on", handle_led2on);
  //server.on("/led2off", handle_led2off);
  server.onNotFound(handle_NotFound);

  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
}

void handle_OnConnect() {
  LED1status = LOW;
  //LED2status = LOW;
  digitalWrite(LED1pin, LOW);
  //digitalWrite(LED2pin, LOW);
  Serial.println("LED1 Status: OFF | LED2 Status: OFF");
  server.send(200, "text/html", createHTML());
}

void handle_led1on() {
  LED1status = HIGH;
  digitalWrite(LED1pin, HIGH);
  Serial.println("LED1 Status: ON");
  server.send(200, "text/html", createHTML());
}

void handle_led1off() {
  LED1status = LOW;
  digitalWrite(LED1pin, LOW);
  Serial.println("LED1 Status: OFF");
  server.send(200, "text/html", createHTML());
}

void handle_led2on() {
  LED2status = HIGH;
  digitalWrite(LED2pin, HIGH);
  Serial.println("LED2 Status: ON");
  server.send(200, "text/html", createHTML());
}

void handle_led2off() {
  LED2status = LOW;
  digitalWrite(LED2pin, LOW);
  Serial.println("LED2 Status: OFF");
  server.send(200, "text/html", createHTML());
}

void handle_NotFound() {
  server.send(404, "text/plain", "Not found");
}

String createHTML() {
  String str = "<!DOCTYPE html> <html>";
  str += "<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">";
  str += "<style>";
  str += "body {font-family: Arial, sans-serif; color: #444; text-align: center;}";
  str += ".title {font-size: 30px; font-weight: bold; letter-spacing: 2px; margin: 80px 0 55px;}";
  str += ".led-control {display: flex; align-items: center; justify-content: center; margin: 40px 0; gap: 30px;}";
  str += ".led-label {font-size: 26px;}";
  str += ".toggle-switch {width: 120px; height: 60px;}";
  str += ".slider {position: absolute; width: 120px; height: 60px; background-color: #f1f1f1; transition: .4s; border-radius: 60px; border: 1px solid #ddd;}";
  str += ".slider:before {content: ''; position: absolute; height: 52px; width: 52px; left: 4px; top: 4px; background-color: white; transition: .4s; border-radius: 50%; box-shadow: 0 2px 5px rgba(0, 0, 0, .3);}";
  str += ".slider.on {background-color: #4285f4; border: none;}";
  str += ".slider.on:before {transform: translateX(60px);}";
  str += "a {display: block; height: 100%; width: 100%; text-decoration: none;}";
  str += "</style>";
  str += "</head>";
  str += "<body>";
  str += "<h1 class=\"title\">LED CONTROLLER</h1>";

  // LED 1 Control
  str += "<div class=\"led-control\">";
  str += "<span class=\"led-label\">LED 1</span>";
  str += "<div class=\"toggle-switch\">";
  if (LED1status) {
    str += "<a href=\"/led1off\">";
    str += "<div class=\"slider on\"></div>";
    str += "</a>";
  } else {
    str += "<a href=\"/led1on\">";
    str += "<div class=\"slider\"></div>";
    str += "</a>";
  }
  str += "</div>";
  str += "</div>";

  // LED 2 Control
//   str += "<div class=\"led-control\">";
//   str += "<span class=\"led-label\">LED 2</span>";
//   str += "<div class=\"toggle-switch\">";
//   if (LED2status) {
//     str += "<a href=\"/led2off\">";
//     str += "<div class=\"slider on\"></div>";
//     str += "</a>";
//   } else {
//     str += "<a href=\"/led2on\">";
//     str += "<div class=\"slider\"></div>";
//     str += "</a>";
//   }
//   str += "</div>";
//   str += "</div>";

//   str += "</body>";
//   str += "</html>";
  return str;
}