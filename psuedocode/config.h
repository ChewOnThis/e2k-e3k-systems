//times
const uint32_t T_OPEN_EXPECTED = 5200;   //when to start slowing (tune in lab)
const uint32_t T_OPEN_MAX      = 7000;   //hard ceiling for open action

//speeds (abstract 0..255 will be mapped to LEDC duty inside BridgeActuator)
//ESP32’s PWM hardware (LEDC) actually wants a duty value whose range depends on the resolution configured
const int PWM_OPEN_CRUISE = 170;         //brisk but safe
const int PWM_OPEN_SLOW   = 110;         //gentle approach to top limit
