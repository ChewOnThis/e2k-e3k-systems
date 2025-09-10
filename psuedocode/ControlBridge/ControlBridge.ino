 
#include "config.h"
#include "Raising.h"
#include "Lowering.h"
#include "PrepRaise.h"
#include "PrepLow.h"
#include "EmergencyLower.h"
#include "EmergencyRaise.h"
#include <WebServer.h>
#include <WiFi.h>

/*    Simple FSM from our diagram (only essentials)     */
enum State { DOWN, PREP_RAISE, RAISING, UP, PREP_LOWER, LOWERING,
             EMERGENCY_LOWER, EMERGENCY_RAISE };

static State    state = DOWN;
static uint32_t t0    = 0;
static bool     first = true;

/* one shot requests (set in readInputs) */
static bool reqRaise=false, reqLower=false, reqCancel=false;

/*    forward    */
void gotoState(State s);
void readInputs();

/*    Arduino std lifecycle    */
void setup() {
  Serial.begin(115200);
  ioBegin();                // pinModes etc.
  gotoState(DOWN);
}
void loop() {
  readInputs();

  switch (state) {
    case DOWN: {
      if (first) { first=false; motorStop(); roadGreen(); marineRed(); gatesUp(); }
      if (reqRaise || boatWaiting()) { reqRaise=false; gotoState(PREP_RAISE); }
    } break;

    case PREP_RAISE: {
      if (first) { first=false; PrepRaise::enter(); }
      using PrepRaise::Result;
      Result r = PrepRaise::tick();
      if      (r == Result::TO_RAISING)         gotoState(RAISING);
      else if (r == Result::TO_EMERGENCY_LOWER) gotoState(EMERGENCY_LOWER);
    } break;

    case RAISING: {
      if (first) { first=false; Raising::enter(); }
      using Raising::Result;
      Result r = Raising::tick();
      if      (r == Result::TO_UP)               gotoState(UP);
      else if (r == Result::TO_EMERGENCY_LOWER)  gotoState(EMERGENCY_LOWER);
    } break;

    case UP: {
      if (first) { first=false; motorStop(); roadRed(); marineGreen(); gatesDown(); }
      if (reqLower) { reqLower=false; gotoState(PREP_LOWER); }
    } break;

    case PREP_LOWER: {
      if (first) { first=false; PrepLow::enter(); }
      using PrepLow::Result;
      Result r = PrepLow::tick();
      if      (r == Result::TO_UP)       gotoState(UP);
      else if (r == Result::TO_LOWERING) gotoState(LOWERING);
    } break;

    case LOWERING: {
      if (first) { first=false; Lowering::enter(); }
      using Lowering::Result;
      Result r = Lowering::tick();
      if      (r == Result::TO_DOWN)             gotoState(DOWN);
      else if (r == Result::TO_EMERGENCY_RAISE)  gotoState(EMERGENCY_RAISE);
    } break;

    case EMERGENCY_LOWER: {
      if (first) { first=false; EmergencyLower::enter(); }
      using EmergencyLower::Result;
      Result r = EmergencyLower::tick();
      if (r == EmergencyLower::Result::TO_DOWN) gotoState(DOWN);
    } break;

    case EMERGENCY_RAISE: {
      if (first) { first=false; EmergencyRaise::enter(); }
      using EmergencyRaise::Result;
      Result r = EmergencyRaise::tick();
      if (r == EmergencyRaise::Result::TO_UP) gotoState(UP);
    } break;
  }
}

/*    FSM helper    */
void gotoState(State s) { state = s; first = true; t0 = millis(); }

/* ============================================================================
   HARDWARE HELPERS — implement to wiring.
   NOTE: these satisfy the `extern` declarations inside Raising.h/Lowering.h.
   For bring up, the Serial commands below let you test without hardware:
     'o' = open (raise), 'c' = close (lower), 'a' = abort, 'e' = estop toggle
   ========================================================================== */

//  requests / inputs   
static bool g_estop=false;

void readInputs() {
  while (Serial.available()) {
    char c = Serial.read();
    if (c=='o') reqRaise  = true;
    if (c=='c') reqLower  = true;
    if (c=='a') reqCancel = true;
    if (c=='e') g_estop   = !g_estop;  // toggle for testing
  }
  // TODO: map the actual buttons/sensors here
}

bool eStop()             { return g_estop; }             // wire to E STOP
bool cancelRequested()   { return reqCancel; }           // keep latched until handled
bool carOnBridge()       { /* plates/mats */ return false; }
bool boatWaiting()       { /* waiting sensor */ return false; }
bool boatUnderSpan()     { /* under span sensor */ return false; }
bool topLimitPressed()   { /* top limit switch */ return false; }
bool bottomLimitPressed(){ /* bottom limit switch */ return false; }

//  lights & gates  
void roadGreen()  { /* LEDs */ }
void roadYellow() { /* LEDs */ }
void roadRed()    { /* LEDs */ }
void marineGreen(){ /* LEDs */ }
void marineRed()  { /* LEDs */ }
void gatesUp()    { /* servo up */ }
void gatesDown()  { /* servo down/lock */ }

//  motor primitives 
void motorEnable()    { /* H bridge EN */ }
void motorDirUp()     { /* DIR=UP */ }
void motorDirDown()   { /* DIR=DOWN */ }
void motorCruiseUp()  { /* PWM fast up */ }
void motorSlowUp()    { /* PWM slow up */ }
void motorCruiseDown(){ /* PWM fast down */ }
void motorSlowDown()  { /* PWM slow down */ }
void motorStop()      { /* PWM=0 */ }
