//-
//LOWERING state handler (called every loop while state == LOWERING)
//Responsibilities (mirrors your flow):
//1) Init + enable motor to lower
//2) Continuously check E-Stop
//3) After EXPECTED time elapses -> reduce motor speed (soft landing)
//4) If bottom limit pressed -> stop + transition to RESUME_ROAD
//5) Any E-Stop at any point -> transition to EMERGENCY_RAISE
//6) If abort pressed OR boat detected under span -> EMERGENCY_RAISE
//Interlocks (defensive):
//- Gates must be DOWN and road must be RED/CLEAR while lowering
//- Respect CLOSE_MAX timeout -> fault/emergency path
//-

struct LoweringCtx {
  bool initialised = false;
  bool inDecel     = false;
  uint32_t tStart  = 0;      //millis when LOWERING entered
  uint32_t tDecel  = 0;      //when we switched to slow approach
};

LoweringCtx lowering;

void enterLowering() {
  lowering = {};
  lowering.initialised = true;
  lowering.tStart = millis();

  //Outputs on entry
  Sig.road(RED);                 //keep road locked
  Sig.marine(RED);               //marine red while closing
  Sig.gates(DOWN_LOCK);          //hold gates down firmly

  //Motor init: drive deck DOWN
  Act.setDirection(DOWN);
  Act.enable();                  //H-bridge enable if available
  Act.rampTo(PWM_CLOSE_CRUISE);  //start ramp toward cruise speed
}

void tickLowering() {

  // Global safety dominator 
  if (Buttons.estop() || Link.commandIs(ESTOP)) {
    Act.stop();
    transitionTo(EMERGENCY_RAISE);
    return;
  }

  // Abort/boat re-appears under span -> Emergency Raise 
  if (Link.cancelRequested() || In.boatUnderSpan(UNDER_DIST_MM)) {
    Act.stop();
    transitionTo(EMERGENCY_RAISE);
    return;
  }

  // Runtime interlocks (defensive duplication) 
  //Gates must remain down during motion; if not, abort upward
  if (!Sig.gatesAreDown()) {
    Act.stop();
    transitionTo(EMERGENCY_RAISE);
    return;
  }

  // Expected-time logic -> decelerate near the bottom 
  uint32_t tNow = millis();
  uint32_t elapsed = tNow - lowering.tStart;

  if (!lowering.inDecel && elapsed >= T_CLOSE_EXPECTED) {
    lowering.inDecel = true;
    lowering.tDecel  = tNow;
    Act.rampTo(PWM_CLOSE_SLOW);        //creep into bottom limit
  } else if (!lowering.inDecel) {
    Act.rampTo(PWM_CLOSE_CRUISE);      //accelerate/hold cruise
  } else {
    Act.rampTo(PWM_CLOSE_SLOW);        //remain slow in final approach
  }

  // Check E-Stop again (second guard like your diagram) 
  if (Buttons.estop() || Link.commandIs(ESTOP)) {
    Act.stop();
    transitionTo(EMERGENCY_RAISE);
    return;
  }

  // Terminal condition: Bottom (closed) limit pressed 
  if (Act.atClosedLimit()) {
    Act.stop();
    transitionTo(RESUME_ROAD);         //raise gates, then road green
    return;
  }

  // Timeout/fault guard 
  if (elapsed >= T_CLOSE_MAX) {
    Act.stop();
    setFault(CLOSE_TIMEOUT);
    transitionTo(EMERGENCY_RAISE);     //or FAULT, per your policy
    return;
  }
}

//In FSM switch:
case LOWERING:
  if (!lowering.initialised) enterLowering();
  tickLowering();
  break;
