#include "main.h"
#include "Motor.h"
#include "LimitSwitch.h"
#include "Ultrasonic.h"
#include "Config.h"

// Dependencies
static Motor*       Act=nullptr;
static LimitSwitch* Limits=nullptr;
static Ultrasonic*  SonarWait=nullptr;
static Ultrasonic*  SonarUnder=nullptr;
static MainConfig   C;

// env callbacks (from .ino)
static VoidFn roadG, roadY, roadR, marineG, marineR, gatesUpFn, gatesDownFn;
static BoolFn gatesDownOk, carOn, boatWait, boatUnder;

// Finite State Machine based off Bridge State diagram! 
enum State { DOWN, PREP_RAISE, RAISING, UP, PREP_LOW, LOWERING, EMERG_RAISE, EMERG_LOWER };
static State S = DOWN;
static uint32_t tEntry = 0;

// per-motion contexts
static bool raisingDecel=false, loweringDecel=false;

void Main_init(Motor* m, LimitSwitch* ls, Ultrasonic* wait, Ultrasonic* under,
               const MainConfig& cfg) {
  Act=m; Limits=ls; SonarWait=wait; SonarUnder=under; C=cfg;
  S = DOWN; tEntry = millis();
}

void Main_setEnvironment(VoidFn _roadG, VoidFn _roadY, VoidFn _roadR,
                         VoidFn _marineG, VoidFn _marineR,
                         VoidFn _gatesUp, VoidFn _gatesDown, BoolFn _gatesAreDown,
                         BoolFn _carOn, BoolFn _boatWaiting, BoolFn _boatUnder) {
  roadG=_roadG; roadY=_roadY; roadR=_roadR;
  marineG=_marineG; marineR=_marineR;
  gatesUpFn=_gatesUp; gatesDownFn=_gatesDown; gatesDownOk=_gatesAreDown;
  carOn=_carOn; boatWait=_boatWaiting; boatUnder=_boatUnder;
}

// small helpers
static inline void gotoState(State ns, uint32_t tNow) { S=ns; tEntry=tNow; }

void Main_tick(bool reqRaise, bool reqLower, bool reqAbort, uint32_t tNow) {

  // realtime inputs
  bool estop = Limits->eStop();
  bool top   = Limits->topPressed();
  bool bot   = Limits->bottomPressed();

  switch (S) {

    case DOWN: {
      // outputs
      roadG(); marineR(); gatesUpFn(); Act->stop();
      if (reqRaise || boatWait()) gotoState(PREP_RAISE, tNow);
    } break;

    case PREP_RAISE: {
      static bool primed=false;
      if (!primed) { primed=true; tEntry=tNow; roadY(); marineR(); gatesDownFn(); }
      if (carOn()) { primed=false; gotoState(EMERG_LOWER, tNow); break; }
      if (tNow - tEntry >= C.T_YELLOW && gatesDownOk()) { primed=false; gotoState(RAISING, tNow); }
    } break;

    case RAISING: {
      // on entry
      if (tNow - tEntry == 0) {
        raisingDecel=false; roadR(); marineR(); gatesDownFn();
        Act->enable(); Act->dirUp(); Act->setSpeed(C.PWM_OPEN_CRUISE);
      }
      if (estop || reqAbort || carOn()) { Act->stop(); gotoState(EMERG_LOWER, tNow); break; }

      uint32_t el = tNow - tEntry;
      if (!raisingDecel && el >= C.T_OPEN_EXPECTED) { raisingDecel=true; Act->setSpeed(C.PWM_OPEN_SLOW); }
      else { Act->setSpeed(raisingDecel ? C.PWM_OPEN_SLOW : C.PWM_OPEN_CRUISE); }

      if (estop)                         { Act->stop(); gotoState(EMERG_LOWER, tNow); }
      else if (top)                      { Act->stop(); gotoState(UP, tNow); }
      else if (el >= C.T_OPEN_MAX)       { Act->stop(); gotoState(EMERG_LOWER, tNow); }
    } break;

    case UP: {
      roadR(); marineG(); gatesDownFn(); Act->stop();
      // If no boat is waiting or under the span, start PREP_LOW automatically
      if (reqLower || (!boatWait() && !boatUnder())) gotoState(PREP_LOW, tNow);
    } break;

    case PREP_LOW: {
      marineR(); // road remains red; gates remain down
      if (boatUnder() || boatWait()) gotoState(UP, tNow);
      else                           gotoState(LOWERING, tNow); // "All clear"
    } break;

    case LOWERING: {
      if (tNow - tEntry == 0) {
        loweringDecel=false; roadR(); marineR(); gatesDownFn();
        Act->enable(); Act->dirDown(); Act->setSpeed(C.PWM_CLOSE_CRUISE);
      }
      if (estop || reqAbort || boatUnder()) { Act->stop(); gotoState(EMERG_RAISE, tNow); break; }

      uint32_t el = tNow - tEntry;
      if (!loweringDecel && el >= C.T_CLOSE_EXPECTED) { loweringDecel=true; Act->setSpeed(C.PWM_CLOSE_SLOW); }
      else { Act->setSpeed(loweringDecel ? C.PWM_CLOSE_SLOW : C.PWM_CLOSE_CRUISE); }

      if (bot)                       { Act->stop(); gotoState(DOWN, tNow); }
      else if (el >= C.T_CLOSE_MAX)  { Act->stop(); gotoState(EMERG_RAISE, tNow); }
    } break;

    case EMERG_RAISE: {
      roadR(); marineR(); gatesDownFn(); Act->enable(); Act->dirUp(); Act->setSpeed(C.PWM_OPEN_CRUISE);
      if (top) { Act->stop(); gotoState(UP, tNow); }
    } break;

    case EMERG_LOWER: {
      roadR(); marineR(); gatesDownFn(); Act->enable(); Act->dirDown(); Act->setSpeed(C.PWM_CLOSE_CRUISE);
      if (bot) { Act->stop(); gotoState(DOWN, tNow); }
    } break;
  }
}

typedef void (*VoidFn)();
typedef bool (*BoolFn)();
