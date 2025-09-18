#include <Arduino.h>
#include "Config.h"   
#include "main.h"
#include "Motor.h"
#include "LimitSwitch.h"
#include "Ultrasonic.h"
#include "Webpage.h"

// Singletons
Motor       Act;
LimitSwitch Limits(PIN_LIM_TOP, PIN_LIM_BOT, PIN_ESTOP);
Ultrasonic  SonarWait(PIN_TRIG_WAIT,  PIN_ECHO_WAIT);
Ultrasonic  SonarUnder(PIN_TRIG_UNDER, PIN_ECHO_UNDER);


// Polarity-reversed duty cycle helper
// Maps speed (0 = max speed, MAX_SPEED = stopped) to duty (0 = off, 255 = full power)
int invertedDutyFromSpeed(int speed, int maxSpeed = 255) {
    speed = constrain(speed, 0, maxSpeed);
    // Inverse relationship: higher speed -> lower duty
    return map(speed, 0, maxSpeed, 255, 0);
}
// Usage example:
// int duty = invertedDutyFromSpeed(currentSpeed, 255);


// ---------- Low-level hooks used by Motor (requested: PWM write in .ino) ----------
void motorHwEnable()   {/* EN pin high if you have one */}
void motorHwDisable()  {/* EN pin low  */}
void motorHwDirUp()    { digitalWrite(PIN_MOTOR_DIR, HIGH); }
void motorHwDirDown()  { digitalWrite(PIN_MOTOR_DIR, LOW);  }
void motorHwWritePWM(int duty255) {
  duty255 = constrain(duty255, 0, 255);
  analogWrite(PIN_MOTOR_PWM, duty255);
}

// ---------- Lights / gates (stubs) ----------
void roadGreen()  {}
void roadYellow() {}
void roadRed()    {}
void marineGreen(){}
void marineRed()  {}
void gatesUp()    {}
void gatesDown()  {}
bool gatesAreDown(){ return true; }

// ---------- Other inputs ----------
bool carOnBridge()   { /* pressure plates */ return false; }
bool boatWaiting()   { return SonarWait.distanceMM()  < BOAT_WAITING_MM; }
bool boatUnderSpan() { return SonarUnder.distanceMM() < BOAT_UNDER_MM;   }

// ---------- Wiring the world ----------
void setup() {
  Serial.begin(115200);
  pinMode(PIN_MOTOR_DIR, OUTPUT);
  pinMode(PIN_MOTOR_PWM, OUTPUT);
  pinMode(PIN_BTN_RAISE, INPUT_PULLUP);
  pinMode(PIN_BTN_LOWER, INPUT_PULLUP);
  pinMode(PIN_BTN_ABORT, INPUT_PULLUP);



  // Initialise sensors
  Limits.begin();
  SonarWait.begin();
  SonarUnder.begin();

  // Provide the Motor wrapper with our low-level hooks
  Act.bind(motorHwEnable, motorHwDisable, motorHwDirUp, motorHwDirDown, motorHwWritePWM);

  // Pass config object (optional, uses default if omitted)
  Main_init(&Act, &Limits, &SonarWait, &SonarUnder);
  Webpage_init();
}

void loop() {
  Webpage_poll();
  // Read buttons (active-low)
  bool reqRaiseBtn = digitalRead(PIN_BTN_RAISE) == LOW;
  bool reqLowerBtn = digitalRead(PIN_BTN_LOWER) == LOW;
  bool reqAbortBtn = digitalRead(PIN_BTN_ABORT) == LOW;
  bool reqRaise = reqRaiseBtn || Web_reqRaise();
  bool reqLower = reqLowerBtn || Web_reqLower();
  bool reqAbort = reqAbortBtn || Web_reqAbort();

  // Provide environment (lights/gates + sensors) via callbacks
  Main_setEnvironment(roadGreen, roadYellow, roadRed,
                      marineGreen, marineRed,
                      gatesUp, gatesDown, gatesAreDown,
                      carOnBridge, boatWaiting, boatUnderSpan);

  // Run FSM
  Main_tick(reqRaise, reqLower, reqAbort, millis());
  Web_setState(reqAbort?1:0, reqRaise?1:0); // simple mapping
}
