//Traffic Lights handler
//Responsibilities-
//1.control traffic light for all( road+ pedesterian +marine)
//2.Fail safe: default to RED
// 3.Green only when bridge is fully closed and gates are open 
//4. RED in all other situation 

struct TrafficCtx{
bool initialised =false;
uni32_t tStart =0;//millis when entered 
};

TrafficCtx traffic;

void enterTrafficLghts(){
  traffic ={};
traffic.initialised = true;
traffic.tStart=millis();

//Default safe output
Sig.traffic(RED);
}

void tickTrafficLights() {
  // Global safety dominator
  if (Buttons.estop() || Link.commandIs(ESTOP)) {
    Sig.traffic(RED);
    transitionTo(EMERGENCY_RAISE);
    return;
  }

  // Logic: traffic only allowed when bridge is safe
  if (Act.atClosedLimit() && Sig.gatesAreUp()) {
    // Bridge closed & gates open → everyone may go
    Sig.traffic(GREEN);
  } else {
    // Otherwise → stop everyone
    Sig.traffic(RED);
  }

  // Timeout/fault guard
  uint32_t elapsed = millis() - traffic.tStart;
  if (elapsed > T_LIGHTS_MAX) {
    Sig.traffic(RED);
    setFault(LIGHTS_TIMEOUT);
    transitionTo(EMERGENCY_RAISE);
    return;
  }
}

// In FSM switch:
case TRAFFIC_LIGHTS:
  if (!traffic.initialised) enterTrafficLights();
  tickTrafficLights();
  break;
