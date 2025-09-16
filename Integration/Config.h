// ALL TO BE CHANGED/TUNED

// Pins 
#define PIN_MOTOR_PWM      27
#define PIN_MOTOR_DIR      26
#define PIN_BTN_RAISE      14
#define PIN_BTN_LOWER      12
#define PIN_BTN_ABORT      13
#define PIN_ESTOP          33
#define PIN_LIM_TOP        32
#define PIN_LIM_BOT        25

// pins for ULTRASONIC
#define PIN_TRIG_WAIT       4
#define PIN_ECHO_WAIT       5
#define PIN_TRIG_UNDER     18
#define PIN_ECHO_UNDER     19

// ESP32 PWM 
#define PWM_CH              0
#define PWM_FREQ            20000
#define PWM_RES             8      // bits, -> duty 0..255

// Timing (ms) 
#define T_YELLOW_MS         2000UL
#define T_OPEN_EXPECTED_MS  5200UL
#define T_OPEN_MAX_MS       7000UL
#define T_CLOSE_EXPECTED_MS 5200UL
#define T_CLOSE_MAX_MS      7000UL

// Duty “speeds” (0..255) 
#define DUTY_OPEN_CRUISE    170
#define DUTY_OPEN_SLOW      110
#define DUTY_CLOSE_CRUISE   160
#define DUTY_CLOSE_SLOW     100

// Sensor thresholds 
#define BOAT_WAITING_MM     400
#define BOAT_UNDER_MM       300
