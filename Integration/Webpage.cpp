#include <Arduino.h>
#include "Webpage.h"
#include <WebServer.h>
#include <WiFi.h>
#include "Config.h"

WebServer server(80);
int switch1=0, switch2=0, state1=0, state2=0;
static bool reqRaise=false, reqLower=false, reqAbort=false; // internal flags

void setup_routes(){
  server.on("/", handle_root);
  server.on("/state", handle_state);
  server.on("/switch1on", handle_switch1_on);
  server.on("/switch1off", handle_switch1_off);
  server.on("/switch2on", handle_switch2_on);
  server.on("/switch2off", handle_switch2_off);
}

void handle_root(){ server.send(200,"text/html",createHTML()); }
void handle_state(){
  String json="{";
  json += "\"switch1\":"; json += switch1?"1":"0"; json += ",";
  json += "\"switch2\":"; json += switch2?"1":"0"; json += ",";
  json += "\"state1\":";  json += state1 ?"1":"0"; json += ",";
  json += "\"state2\":";  json += state2 ?"1":"0"; json += "}";
  server.send(200,"application/json",json);
}
void handle_switch1_on(){  // E-STOP asserted
  switch1=1; reqAbort=true; reqRaise=false; reqLower=false; server.send(200,"text/html",createHTML()); }
void handle_switch1_off(){ // E-STOP cleared
  switch1=0; reqAbort=false; server.send(200,"text/html",createHTML()); }
void handle_switch2_on(){  // Request raise
  switch2=1; reqRaise=true; reqLower=false; server.send(200,"text/html",createHTML()); }
void handle_switch2_off(){ // Request lower
  switch2=0; reqRaise=false; reqLower=true; server.send(200,"text/html",createHTML()); }

String createHTML(){
  String str="";
  str += "<!DOCTYPE html><html lang='en'><head><meta charset='utf-8'/>";
  str += "<meta name='viewport' content='width=device-width,initial-scale=1'/>";
  str += "<title>WA8 Control Panel</title><style>body{font-family:Arial;margin:0;padding:32px;text-align:center;color:#333}";
  str += ".row{display:flex;justify-content:center;gap:28px;margin:24px 0}";
  str += ".btn{padding:14px 32px;border:0;border-radius:14px;font-size:20px;color:#fff;cursor:pointer;box-shadow:0 2px 6px rgba(0,0,0,.2)}";
  str += ".on{background:#2e7d32}.off{background:#c62828}.box{width:140px;height:26px;border-radius:14px;background:#c62828}";
  str += ".box.on{background:#2e7d32}.status{margin-top:30px;font-family:monospace;font-size:14px}";
  str += "</style></head><body><h1>WA8 Single Leaf Bridge</h1><h2>Wireless Control Panel</h2>";
  str += "<div class='row'>";
  str += "<div><h3>E-Stop</h3><button id='switch1' class='btn off'>OFF</button></div>";
  str += "<div><h3>Bridge Command</h3><button id='switch2' class='btn off'>LOWER</button></div>";
  str += "</div><div class='row'>";
  str += "<div><h3>Abort</h3><div id='state1' class='box'></div></div>";
  str += "<div><h3>Raising</h3><div id='state2' class='box'></div></div>";
  str += "</div><div class='status' id='status'></div><script>";
  str += "function btn(el,on,labelOn,labelOff){el.classList.toggle('on',on);el.classList.toggle('off',!on);el.textContent=on?labelOn:labelOff;}";
  str += "function box(el,on){el.classList.toggle('on',on);}";
  str += "function apply(s){btn(b1,s.switch1,'ON','OFF');btn(b2,s.switch2,'RAISE','LOWER');box(st1,s.state1);box(st2,s.state2);st.textContent='State:'+JSON.stringify(s);}";
  str += "function poll(){fetch('/state').then(r=>r.json()).then(apply).catch(()=>{});}setInterval(poll,1000);window.addEventListener('load',poll);";
  str += "const b1=document.getElementById('switch1');const b2=document.getElementById('switch2');const st1=document.getElementById('state1');const st2=document.getElementById('state2');const st=document.getElementById('status');";
  str += "b1.onclick=()=>{fetch('/'+(b1.classList.contains('on')?'switch1off':'switch1on')).then(()=>poll());};";
  str += "b2.onclick=()=>{fetch('/'+(b2.classList.contains('on')?'switch2off':'switch2on')).then(()=>poll());};";
  str += "</script></body></html>";
  return str;
}

// Web integration API
void Webpage_init(){ WiFi.softAP(ssid,password); setup_routes(); server.begin(); }
void Webpage_poll(){ server.handleClient(); }
bool Web_reqRaise(){ return reqRaise; }
bool Web_reqLower(){ return reqLower; }
bool Web_reqAbort(){ return reqAbort; }
void Web_setState(int s1,int s2){ state1=s1; state2=s2; }