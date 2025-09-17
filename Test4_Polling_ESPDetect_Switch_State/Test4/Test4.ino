#include <WiFi.h>
#include <WebServer.h>

/*Put your SSID & Password*/
const char* ssid = "ESP32WA8";   // Enter SSID here
const char* password = "12345678";  //Enter Password here
IPAddress local_ip(192,168,1,1);
IPAddress gateway(192,168,1,1);
IPAddress subnet(255,255,255,0);
WebServer server(80);

int pin = 18;
bool switch1 = LOW;
bool switch2 = LOW;
bool state1 = 0;
bool state2 = 0;
int photostate = 0;
int photopin = 36;

int buttonpin1 = 23;
int buttonpin2 = 4;
int buttonpin3 = 15;

int trafficredpin = 25;
int trafficyellowpin = 26;
int trafficgreenpin = 27;
int state = 0;

int statuspin = 19;

int trigPin1 = 33;
int echoPin1 = 35;
float duration1, distance1;  

int trigPin2 = 32;
int echoPin2 = 34;
float duration2, distance2; 

int buttonState1 = 0;
int buttonState2 = 0;
int buttonState3 = 0;

int motorpin1 = 12;
int motorpin2 = 13;


void setup() {
    Serial.begin(115200);
   // delay(100);
    pinMode(25, OUTPUT);
    pinMode(26, OUTPUT);
    pinMode(27, OUTPUT);
    pinMode(statuspin, OUTPUT);
    pinMode(pin, OUTPUT);
    pinMode(photopin, INPUT);
    pinMode(trigPin1, OUTPUT);  
	pinMode(echoPin1, INPUT);
    pinMode(buttonpin1, INPUT_PULLUP);  
    pinMode(trigPin2, OUTPUT);  
	pinMode(echoPin2, INPUT);
    pinMode(buttonpin2, INPUT_PULLUP);
    pinMode(buttonpin3, INPUT_PULLUP); 
    pinMode(motorpin1, OUTPUT);
    pinMode(motorpin2, OUTPUT);
    analogWrite(motorpin1, 128);



//     WiFi.softAP(ssid, password);
//     WiFi.softAPConfig(local_ip, gateway, subnet);
//     delay(100);

//     setup_routes();
//     server.begin();
//     Serial.println("HTTP server started");

    

}

void loop() {
    if (!digitalRead(buttonpin2)) {
        digitalWrite(motorpin2, HIGH);
        Serial.println("HIGH");
    } else if (!digitalRead(buttonpin3)) {
        digitalWrite(motorpin2, LOW);
        Serial.println("LOW");
    }
            
    photostate = analogRead(photopin);
    //Serial.println(photostate);
    //delay(100);
    if (photostate < 650) {
        digitalWrite(pin, HIGH);
    } else {
        digitalWrite(pin, LOW);
    }


    // delay(100);
    int temp = digitalRead(buttonpin1);
    if (temp != 1) {
        buttonState1 = !buttonState1;
    }
    digitalWrite(statuspin, buttonState1);

    digitalWrite(trigPin1, LOW);  
	delayMicroseconds(2);  
	digitalWrite(trigPin1, HIGH);  
	delayMicroseconds(10);  
	digitalWrite(trigPin1, LOW);
    duration1 = pulseIn(echoPin1, HIGH);
    distance1 = (duration1*.0343)/2;
    Serial.print("Distance1:");  
	Serial.println(distance1);  
	delay(100);

    digitalWrite(trigPin2, LOW);  
	delayMicroseconds(2);  
	digitalWrite(trigPin2, HIGH);  
	delayMicroseconds(10);  
	digitalWrite(trigPin2, LOW);
    duration2 = pulseIn(echoPin2, HIGH);
    distance2 = (duration2*.0343)/2;
    Serial.print("Distance2:");  
	Serial.println(distance2);  
	delay(100);

    switch(state) {
        case 0:
            digitalWrite(trafficgreenpin, HIGH);
            digitalWrite(trafficyellowpin, LOW);
            digitalWrite(trafficredpin, LOW);
            if (distance1 < 30 | distance2 < 30) {
                state ++;
            }

            break;
        case 1:
            digitalWrite(trafficgreenpin, LOW);
            digitalWrite(trafficyellowpin, HIGH);
            digitalWrite(trafficredpin, LOW);
            state++;
            break;
        case 2:
            digitalWrite(trafficgreenpin, LOW);
            digitalWrite(trafficyellowpin, LOW);
            digitalWrite(trafficredpin, HIGH);
            if (distance1 > 30 & distance2 > 30) {
                state = 0;}
            break;
    }
    
    //server.handleClient();
}

void setup_routes() {
    server.on("/", handle_root);
    server.on("/state", handle_state);
    server.on("/switch1on", handle_switch1_on);
    server.on("/switch1off", handle_switch1_off);
    server.on("/switch2on", handle_switch2_on);
    server.on("/switch2off", handle_switch2_off);
}

void handle_root() {
    server.send(200, "text/html", createHTML());
}

void handle_state() {
    String json = "{";
    json += "\"switch1\":"; json += switch1 ? "1":"0"; json += ",";
    json += "\"switch2\":"; json += switch2 ? "1":"0"; json += ",";
    json += "\"state1\":";  json += state1  ? "1":"0"; json += ",";
    json += "\"state2\":";  json += state2  ? "1":"0";
    json += "}";
    server.send(200, "application/json", json);
}

void handle_switch1_on(){ 
    switch1 = 1; 
    digitalWrite(pin, HIGH);
    Serial.println("LED1 Status : ON");
    server.send(200, "text/html", createHTML()); 
}

void handle_switch1_off(){ 
    switch1 = 0; 
    digitalWrite(pin, LOW); 
    Serial.println("LED1 Status : OFF");
    server.send(200, "text/html", createHTML()); 
}

void handle_switch2_on(){ 
    switch2 = 1; 
    digitalWrite(33, HIGH); 
    Serial.println("LED2 Status : ON");
    server.send(200, "text/html", createHTML()); 
}

void handle_switch2_off(){ 
    switch2 = 0; 
    digitalWrite(33, LOW); 
    Serial.println("LED2 Status : OFF");
    server.send(200, "text/html", createHTML()); 
}


// Auto-generated by Convert on 2025-09-17 09:52:13

String createHTML() {
  String str = "";
  str += "<!DOCTYPE html>\n";
  str += "<html lang=\"en\">\n";
  str += "<head>\n";
  str += "    <meta charset=\"utf-8\" />\n";
  str += "    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\" />\n";
  str += "    <title>WA8 Control Panel</title>\n";
  str += "    <style>\n";
  str += "        body{font-family:Arial,Helvetica,sans-serif;color:#444;text-align:center;margin:0;padding:40px 16px;}\n";
  str += "        .title{font-size:30px;font-weight:700;letter-spacing:1px;margin:20px 0 30px;}\n";
  str += "        .switch-layer{display:flex;align-items:center;justify-content:center;margin:30px 0;gap:18px;}\n";
  str += "        .switch-control{display:flex;align-items:center;justify-content:center;}\n";
  str += "        .switch-label{font-size:22px;min-width:90px;text-align:center;}\n";
  str += "        .btn-wrap{min-width:180px;text-align:center;}\n";
  str += "        .btn-toggle{font-size:20px;padding:14px 28px;border:none;border-radius:12px;cursor:pointer;box-shadow:0 2px 6px rgba(0,0,0,.15);transition:transform .05s ease,opacity .1s ease;}\n";
  str += "        .btn-toggle:active{transform:scale(0.98);}\n";
  str += "        .btn-toggle.on{background:#2e7d32;color:#fff;}   /* green */\n";
  str += "        .btn-toggle.off{background:#c62828;color:#fff;}  /* red   */\n";
  str += "        .state-layer{display:flex;align-items:center;justify-content:center;margin:30px 0;gap:18px;}\n";
  str += "        .state-control{display:flex;align-items:center;justify-content:center;}\n";
  str += "        .state-label{font-size:22px;min-width:90px;text-align:center;}\n";
  str += "        .state-wrap{min-width:180px;justify-content: center;align-items: center;}\n";
  str += "        .state-toggle{font-size:20px;padding:14px 28px;border:none;border-radius:12px;cursor:pointer;box-shadow:0 2px 6px rgba(0,0,0,.15);transition:transform .05s ease,opacity .1s ease;}\n";
  str += "        .state-toggle:active{transform:scale(0.98);}\n";
  str += "        .state-toggle.on{background:#2e7d32;color:#fff;}   /* green */\n";
  str += "        .state-toggle.off{background:#c62828;color:#fff;}  /* red   */\n";
  str += "        .hint{font-size:12px;color:#777;margin-top:4px;}\n";
  str += "        .status{margin-top:24px;color:#666;font-size:13px;}\n";
  str += "        .hr{height:1px;background:#eee;margin:24px auto;max-width:640px;}\n";
  str += "        footer{text-align:center;background-color: #abbaba;color:#777;}\n";
  str += "        .state-box{height:20px;width:100%;background-color: #c62828;border-radius: 10px;}\n";
  str += "        .state-box.on{background-color: #2e7d32;}\n";
  str += "        .state-box.off{background-color: #c62828;}\n";
  str += "    </style>\n";
    
  str += "<script>\n";
   str += "window.onload = function() {";
  str += "  const state = { switch1: 0, switch2: 0, state1: 0, state2: 0 };\n";
  str += "  const statusEl = document.getElementById('status');\n";
  str += "  const btn1 = document.getElementById('switch1');\n";
  str += "  const btn2 = document.getElementById('switch2');\n";
  str += "  const state1Box = document.getElementById(\"state1\");\n";
  str += "  const state2Box = document.getElementById(\"state2\");\n";
  str += "  function setBtn(btn, on) {\n";
  str += "    btn.classList.toggle('on', on);\n";
  str += "    btn.classList.toggle('off', !on);\n";
  str += "    btn.textContent = on ? 'ON' : 'OFF';\n";
  str += "    btn.setAttribute('aria-pressed', on ? 'true' : 'false');\n";
  str += "  }\n";
  str += "  function setStateBox(el, on) {\n";
  str += "    el.classList.toggle('on', on);\n";
  str += "    el.classList.toggle('off', !on);\n";
  str += "    el.style.backgroundColor = on ? '#2e7d32' : '#c62828'; // green/red\n";
  str += "  }\n";
  str += "  function render() {\n";
  str += "    setBtn(btn1, !!state.switch1);\n";
  str += "    setBtn(btn2, !!state.switch2);\n";
  str += "    setStateBox(state1Box, !!state.state1);\n";
  str += "    setStateBox(state2Box, !!state.state2);\n";
  str += "    statusEl.textContent =\n";
  str += "      `State: { switch1: ${state.switch1}, switch2: ${state.switch2}, state1: ${state.state1}, state2: ${state.state2} }`;\n";
  str += "  }\n";
  //Auto Refresh State every 1000ms
  str += "  function poll(){\n";
  str += "    fetch('/state').then(r=>r.json()).then(applyState).catch(()=>{}); \n";
  str += "  }\n";

  str += "  window.addEventListener('load', poll);\n";
  str += "  function applyState(s){\n";
  str += "    const on = v => v === 1 | v === \"1\" || v === true;\n";
  str += "    setBtn(document.getElementById('switch1'), on(s.switch1));\n";
  str += "    setBtn(document.getElementById('switch2'), on(s.switch2));\n";
  str += "    setStateBox(document.getElementById('state1'), on(s.state1));\n";
  str += "    setStateBox(document.getElementById('state2'), on(s.state2));\n";
  str += "  }\n";
  str += "  // Button click events – flip switch *and* state box together\n";
  str += "  btn1.addEventListener('click', () => {\n";
  str += "    state.switch1 = state.switch1 ? 0 : 1;\n";
  str += "    state.state1 = state.switch1;   // mirror state\n";
  str += "    fetch(state.switch1 ? '/switch1on' : '/switch1off');";
  str += "    render();\n";
  str += "  });\n";
  str += "  btn2.addEventListener('click', () => {\n";
  str += "    state.switch2 = state.switch2 ? 0 : 1;\n";
  str += "    state.state2 = state.switch2;   // mirror state\n";
  str += "    fetch(state.switch2 ? '/switch2on' : '/switch2off');";
  str += "    render();\n";
  str += "  });\n";
  str += "  render();\n";
  str += "  setInterval(poll, 1000); \n";
  str += "  poll();\n";
   str += "};";
  str += "</script>\n";
  str += "</head>\n";
  str += "\n";
  str += "<body>\n";
  str += "  <h1 class=\"title\">WA8 Single Leaft Bridge</h1>\n";
  str += "  <h2 class=\"subtitle\">Wireless Control Panel</h2>\n";
  str += "  <div class=\"switch-layer\">\n";
  str += "    <!-- Switch 1 - E-Stop -->\n";
  str += "    <div class=\"switch-control\">\n";
  str += "      <div class=\"switches-label\">E-Stop:\n";
  str += "        <div class=\"btn-wrap\">\n";
  str += "          <button id=\"switch1\" class=\"btn-toggle off\" aria-pressed=\"false\">OFF</button>\n";
  str += "          <div class=\"hint\"> click to toggle </div>\n";
  str += "        </div>\n";
  str += "      </div>\n";
  str += "    </div>\n";
  str += "    <!-- Switch 2 - Activate Bridge -->\n";
  str += "    <div class=\"switch-control\">\n";
  str += "      <div class=\"switches-label\">Activate Bridge:\n";
  str += "        <div class=\"btn-wrap\">\n";
  str += "          <button id=\"switch2\" class=\"btn-toggle off\" aria-pressed=\"false\">OFF</button>\n";
  str += "          <div class=\"hint\"> click to toggle </div>\n";
  str += "        </div>\n";
  str += "      </div>\n";
  str += "    </div>\n";
  str += "  </div>\n";
  str += "  <div class=\"state-layer\">\n";
  str += "    <div class=\"state-control\">\n";
  str += "      <div class=\"state-label\">State 1:\n";
  str += "        <div class=\"state-wrap\">\n";
  str += "          <div id=\"state1\"class=\"state-box\"></div>\n";
  str += "        </div>\n";
  str += "      </div>\n";
  str += "    </div>\n";
  str += "    <div class=\"state-control\">\n";
  str += "      <div class=\"state-label\">State 2:\n";
  str += "        <div class=\"state-wrap\">\n";
  str += "          <div id=\"state2\" class=\"state-box\"></div>\n";
  str += "        </div>\n";
  str += "      </div>\n";
  str += "    </div>\n";
  str += "  </div>\n";
  str += "  <div class=\"hr\"></div>\n";
  str += "  <div class=\"status\" id=\"status\">State: { switch1: 0, switch2: 0 }</div>\n";
  str += "  <div class=\"hr\"></div>\n";
  str += "  <footer>\n";
  str += "    <small>\n";
  str += "      Team WA8 Wifi Control Webpage for AP Access to ESP32 Dev Board\n";
  str += "    </small>\n";
  str += "  </footer>\n";

  str += "</body>\n";
  str += "</html>\n";
  return str;
}

