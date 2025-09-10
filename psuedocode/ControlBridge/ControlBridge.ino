/* ControlBridge.ino  — big sketch that owns hardware + PWM and calls state modules */
#include "Config.h"
#include "Up.h"
#include "Down.h"
#include "PrepRaise.h"
#include "PrepLow.h"
#include "EmergencyRaise.h"
#include "EmergencyLower.h"

// --------- State enum ----------
enum State { DOWN, PREP_RAISE, RAISING, UP, PREP_LOW, LOWERING, EMERG_RAISE, EMERG_LOWER };
State state = DOWN;
bool  first = true;

// --------- One-shot requests set by readInputs() ----------
bool reqRaise=false, reqLower=false, reqAbort=false;

// --------- Simple helpers visible to all modules (declared extern in their headers) ----------
void roadRed();    void roadYellow();  void roadGreen();
void marineRed();  void marineGreen();
void gatesUp();    void gatesDown();   bool gatesAreDown();
bool eStop();      bool carOnBridge(); bool boatWaiting(); bool boatUnderSpan();
bool topLimit();   bool bottomLimit();

// ----- Motor control (PWM) — THIS WRITES SPEED TO A PWM PIN -----
void motorEnable();         // enable H-bridge (if you have EN pin)
void motorDisable();
void motorDirUp();          // set DIR pin to 'up'
void motorDirDown();        // set DIR pin to 'down'
void motorWriteSpeed(int duty255);  // 0..255 → PWM
void motorStop() { motorWriteSpeed(0); }

// PWM implementation (ESP32 ledc OR AVR analogWrite) — pseudocode
void motorWriteSpeed(int duty255) {
  duty255 = constrain(duty255,0,255);
#ifdef ARDUINO_ARCH_ESP32
  // ledcChannel configured in setup
  // map 8-bit 0..255 → timer resolution
  ledcWrite(PWM_MOTOR_CH, duty255);        // <-- actual write to PWM
#else
  analogWrite(PIN_MOTOR_PWM, duty255);     // AVR/Uno/Mega path
#endif
}

// --------- Setup / Loop ----------
void setup() {
  Serial.begin(115200);

  // pins
  pinMode(PIN_MOTOR_PWM, OUTPUT);
  pinMode(PIN_MOTOR_DIR, OUTPUT);
#ifdef ARDUINO_ARCH_ESP32
  ledcSetup(PWM_MOTOR_CH, PWM_FREQ_HZ, PWM_RES_BITS);
  ledcAttachPin(PIN_MOTOR_PWM, PWM_MOTOR_CH);
#endif

  // set safe posture
  roadGreen(); marineRed(); gatesUp(); motorStop();

  // enter DOWN
  Down_enter();
}

void loop() {
  // read buttons/sensors and set requests
  readInputs();

  switch (state) {

    case DOWN: {
      DownResult r = Down_tick(reqRaise, boatWaiting());
      if      (r == DownResult::TO_PREP_RAISE) { state = PREP_RAISE; PrepRaise_enter(); }
      else if (r == DownResult::STAY) { /* do nothing */ }
    } break;

    case PREP_RAISE: {
      PrepRaiseResult r = PrepRaise_tick(carOnBridge(), millis());
      if      (r == PrepRaiseResult::TO_EMERG_LOWER) { state = EMERG_LOWER; EmergencyLower_enter(); }
      else if (r == PrepRaiseResult::TO_RAISING)     { state = RAISING;     Up_enterRaising(); }
    } break;

    case RAISING: {
      RaisingResult r = Up_tickRaising(reqAbort, carOnBridge(), topLimit(), millis());
      if      (r == RaisingResult::TO_UP)              { state = UP;          Up_enter(); }
      else if (r == RaisingResult::TO_EMERGENCY_LOWER) { state = EMERG_LOWER; EmergencyLower_enter(); }
    } break;

    case UP: {
      UpResult r = Up_tick(reqLower, boatDetected());
      if      (r == UpResult::TO_PREP_LOW)  { state = PREP_LOW; PrepLow_enter(); }
    } break;

    case PREP_LOW: {
      PrepLowResult r = PrepLow_tick(boatUnderSpan());
      if      (r == PrepLowResult::TO_UP)       { state = UP;       Up_enter(); }
      else if (r == PrepLowResult::TO_LOWERING) { state = LOWERING; Down_enterLowering(); }
    } break;

    case LOWERING: {
      LoweringResult r = Down_tickLowering(reqAbort, boatUnderSpan(), bottomLimit(), millis());
      if      (r == LoweringResult::TO_DOWN)            { state = DOWN; Down_enter(); }
      else if (r == LoweringResult::TO_EMERGENCY_RAISE) { state = EMERG_RAISE; EmergencyRaise_enter(); }
    } break;

    case EMERG_RAISE: {
      EmergRaiseResult r = EmergencyRaise_tick(topLimit());
      if (r == EmergRaiseResult::TO_UP) { state = UP; Up_enter(); }
    } break;

    case EMERG_LOWER: {
      EmergLowerResult r = EmergencyLower_tick(bottomLimit());
      if (r == EmergLowerResult::TO_DOWN) { state = DOWN; Down_enter(); }
    } break;
  }
}

/* ----------------- INPUTS + LIGHTS + GATES (STUBS) ----------------- */
// These are placeholders; wire to your pins.
void readInputs() {
  // Example Serial UI: 'o' open, 'c' close, 'a' abort, 'e' estop toggle
  while (Serial.available()) {
    char c = Serial.read();
    if (c=='o') reqRaise=true;
    if (c=='c') reqLower=true;
    if (c=='a') reqAbort=true;
    if (c=='e') toggleEStop();
  }
}
bool boatDetected()   { return boatUnderSpan() || boatWaiting(); }
bool eStop()          { /* read E-STOP pin (LOW=pressed) */ return false; }
bool carOnBridge()    { /* read approach plates */ return false; }
bool boatWaiting()    { /* waiting sensor */ return false; }
bool boatUnderSpan()  { /* under-span sensor */ return false; }
bool topLimit()       { /* top limit switch */ return false; }
bool bottomLimit()    { /* bottom limit switch */ return false; }

void roadGreen()  { /* set LEDs exclusive */ }
void roadYellow() { /* set LEDs exclusive */ }
void roadRed()    { /* set LEDs exclusive */ }
void marineGreen(){ /* set LEDs */ }
void marineRed()  { /* set LEDs */ }
void gatesUp()    { /* servo to up */ }
void gatesDown()  { /* servo to down/lock */ }
bool gatesAreDown(){ return true; }

void motorEnable()  { /* EN pin high if used */ }
void motorDisable() { /* EN pin low */ }
void motorDirUp()   { digitalWrite(PIN_MOTOR_DIR, HIGH); }
void motorDirDown() { digitalWrite(PIN_MOTOR_DIR, LOW);  }
