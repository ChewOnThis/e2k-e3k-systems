
//RAISING state handler (called every loop while state == RAISING)
//Responsibilities (from the flowchart):
// 1) Init + enable motor to raise
// 2) Continuously check E-Stop
// 3) After EXPECTED time elapses -> reduce motor speed (soft landing)
// 4) If top limit pressed -> stop + transition to UP
// 5) Any E-Stop at any point -> transition to EMERGENCY_LOWER
// 6) Optional abort/cancel or late road user -> transition to EMERGENCY_LOWER
// Interlocks:
// - Gates must be DOWN and road must be CLEAR to keep raising
// - Respect OPEN_MAX timeout -> FAULT/EMERGENCY path
// -

//context object carried across ticks (owned by the FSM)
struct RaisingCtx {
  bool initialised = false;
  bool inDecel     = false;
  uint32_t tStart  = 0;      //millis when RAISING entered
  uint32_t tDecel  = 0;      //millis when we decided to slow down
};

RaisingCtx raising;

void enterRaising() {
  raising = {};                 //reset flags
  raising.initialised = true;
  raising.tStart = millis();

  //Outputs on entry (Moore-style)
  Sig.road(RED);                //road locked
  Sig.marine(RED);              //no marine go yet
  Sig.gates(DOWN_LOCK);         //hold gates down firmly

  //Motor init (Init motor, enable motor)
  Act.setDirection(UP);
  Act.enable();                 //H-bridge enable if available
  Act.rampTo(PWM_OPEN_CRUISE);  //start ramping up to cruise
}

//Called each loop while in RAISING
void tickRaising() {

  //Global safety dominator
  if (Buttons.estop() || Link.commandIs(ESTOP)) {
    Act.stop();
    transitionTo(EMERGENCY_LOWER);   //End -> change state to Emergency Lower
    return;
  }

  //Runtime interlocks (defensive duplication) 
  //If a car appears or override/cancel demands abort, go to Emergency Lower
  if (In.roadOccupied() || Link.cancelRequested()) {
    Act.stop();
    transitionTo(EMERGENCY_LOWER);
    return;
  }

  //Gates must be proved DOWN while raising; otherwise abort
  if (!Sig.gatesAreDown()) {
    Act.stop();
    transitionTo(EMERGENCY_LOWER);
    return;
  }

  //Expected-time logic -> decelerate (Has expected time ended?) 
  uint32_t tNow = millis();
  uint32_t elapsed = tNow - raising.tStart;

  if (!raising.inDecel && elapsed >= T_OPEN_EXPECTED) {
    //Lower Motor Speed (soft landing region)
    raising.inDecel = true;
    raising.tDecel  = tNow;
    Act.rampTo(PWM_OPEN_SLOW);       //smaller duty to creep into top limit
  } else if (!raising.inDecel) {
    //still accelerating / cruising toward target
    Act.rampTo(PWM_OPEN_CRUISE);     //trapezoidal profile inside BridgeActuator
  } else {
    //already in slow region; keep creeping upward
    Act.rampTo(PWM_OPEN_SLOW);
  }

  //Check E-Stop again (the diagram shows another guard here) 
  if (Buttons.estop() || Link.commandIs(ESTOP)) {
    Act.stop();
    transitionTo(EMERGENCY_LOWER);
    return;
  }

  //Terminal condition: Top limit switch pressed 
  if (Act.atOpenLimit()) {
    Act.stop();
    transitionTo(UP);                //Change state to UP
    return;
  }

  //Timeout / fault guard 
  if (elapsed >= T_OPEN_MAX) {
    Act.stop();
    setFault(OPEN_TIMEOUT);
    transitionTo(EMERGENCY_LOWER);   //or FAULT, per your policy
    return;
  }
}

//optional helper for the FSM switch-case
case RAISING:
  if (!raising.initialised) enterRaising();
  tickRaising();
  break;
