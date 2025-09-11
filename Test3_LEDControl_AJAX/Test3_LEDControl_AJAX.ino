#include <WiFi.h>
#include <WebServer.h>

/*Put your SSID & Password*/
const char* ssid = "ESP32WA8";   // Enter SSID here
const char* password = "12345678";  //Enter Password here

IPAddress local_ip(192,168,1,1);
IPAddress gateway(192,168,1,1);
IPAddress subnet(255,255,255,0);

WebServer server(80);

uint8_t LED1pin = 2;
bool LED1status = LOW;

uint8_t LED2pin = 23;
bool LED2status = LOW;




void setup() {
  Serial.begin(115200);
  delay(100);
  pinMode(LED1pin, OUTPUT);
  pinMode(LED2pin, OUTPUT);

  WiFi.softAP(ssid, password);
  WiFi.softAPConfig(local_ip, gateway, subnet);
  delay(100);

  server.on("/", handle_OnConnect);
  server.on("/state", handle_state);
  server.on("/led1on", handle_led1on);
  server.on("/led1off", handle_led1off);
  server.on("/led2on", handle_led2on);
  server.on("/led2off", handle_led2off);
  server.onNotFound(handle_NotFound);

  server.begin();
  Serial.println("HTTP server started");
  digitalWrite(2, HIGH);
}

void loop() {
  unsigned long before = micros();
  server.handleClient();
  Serial.println(micros() - before);
  delay(100);
}

void handle_OnConnect() {
  LED1status = LOW;
  LED2status = LOW;
  digitalWrite(LED1pin, LOW);
  digitalWrite(LED2pin, LOW);
  Serial.println("LED1 Status: OFF | LED2 Status: OFF");
  server.send(200, "text/html", createHTML());
}

void handle_state() {
  String json = "{";
  json += "\"led1\":"; json += LED1status ? "1" : "0"; json += ",";
  json += "\"led2\":"; json += LED2status ? "1" : "0";
  json += "}";
  server.send(200, "application/json", json);
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
  String str = "<!DOCTYPE html><html>";
  str += "<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">";
  str += "<style>";
  str += "body {font-family: Arial, sans-serif; color: #444; text-align: center;}";
  str += ".title {font-size: 30px; font-weight: bold; letter-spacing: 2px; margin: 80px 0 55px;}";
  str += ".led-control {display: flex; align-items: center; justify-content: center; margin: 40px 0; gap: 30px;}";
  str += ".led-label {font-size: 26px;}";
  str += ".toggle-switch {position: relative; width: 120px; height: 60px;}";
  str += ".slider {position: absolute; width: 120px; height: 60px; background-color: #f1f1f1; transition: .2s; border-radius: 60px; border: 1px solid #ddd;}";
  str += ".slider:before {content: ''; position: absolute; height: 52px; width: 52px; left: 4px; top: 4px; background-color: white; transition: .2s; border-radius: 50%; box-shadow: 0 2px 5px rgba(0,0,0,.3);}";
  str += ".slider.on {background-color: #4285f4; border: none;}";
  str += ".slider.on:before {transform: translateX(60px);}";
  str += "a {display: block; height: 100%; width: 100%; text-decoration: none;}";
  str += "</style>";
  // --- auto-refresh state every 1000 ms
  str += "<script>";
  str += "function applyState(s){";
  str += "  var l1=document.getElementById('led1'); var l2=document.getElementById('led2');";
  str += "  if(l1){ l1.classList.toggle('on', !!s.led1); }";
  str += "  if(l2){ l2.classList.toggle('on', !!s.led2); }";
  str += "}";
  str += "function poll(){ fetch('/state').then(r=>r.json()).then(applyState).catch(()=>{}); }";
  str += "setInterval(poll, 1000); window.addEventListener('load', poll);";
  // optional: prevent full page loads, use fetch() for faster toggles
  str += "function toggleLed(n){";
  str += "  const l1 = document.getElementById('led1'); const l2 = document.getElementById('led2');";
  str += "  let url = '/';";
  str += "  if(n===1){ url = l1.classList.contains('on')?'/led1off':'/led1on'; }";
  str += "  else if(n===2){ url = l2.classList.contains('on')?'/led2off':'/led2on'; }";
  str += "  fetch(url).then(()=>poll());";
  str += "  return false;";
  str += "}";
  str += "</script>";
  str += "</head><body>";
  str += "<h1 class=\"title\">LED CONTROLLER</h1>";

  // LED 1
  str += "<div class=\"led-control\">";
  str += "<span class=\"led-label\">LED 1</span>";
  str += "<div class=\"toggle-switch\">";
  // Keep links for compatibility, but handle clicks via JS to avoid full reload
  if (LED1status) {
    str += "<a href=\"/led1off\" onclick=\"return toggleLed(1)\">";
    str += "<div id=\"led1\" class=\"slider on\"></div>";
    str += "</a>";
  } else {
    str += "<a href=\"/led1on\" onclick=\"return toggleLed(1)\">";
    str += "<div id=\"led1\" class=\"slider\"></div>";
    str += "</a>";
  }
  str += "</div></div>";

  // LED 2
  str += "<div class=\"led-control\">";
  str += "<span class=\"led-label\">LED 2</span>";
  str += "<div class=\"toggle-switch\">";
  if (LED2status) {
    str += "<a href=\"/led2off\" onclick=\"return toggleLed(2)\">";
    str += "<div id=\"led2\" class=\"slider on\"></div>";
    str += "</a>";
  } else {
    str += "<a href=\"/led2on\" onclick=\"return toggleLed(2)\">";
    str += "<div id=\"led2\" class=\"slider\"></div>";
    str += "</a>";
  }
  str += "</div></div>";

  str += "</body></html>";
  return str;
}
