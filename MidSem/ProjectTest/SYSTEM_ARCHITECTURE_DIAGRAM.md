# Bridge Control System - Code Architecture Diagram

## System Architecture Overview

```
┌─────────────────────────────────────────────────────────────────┐
│                        BRIDGE CONTROL SYSTEM                   │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  ┌─────────────────┐    ┌──────────────────┐                   │
│  │   main.cpp      │    │   StateMachine   │                   │
│  │                 │    │                  │                   │
│  │ • System Init   │◄──►│ • State Logic    │                   │
│  │ • Main Loop     │    │ • Transitions    │                   │
│  │ • Command Proc  │    │ • Helper Funcs   │                   │
│  └─────────────────┘    └──────────────────┘                   │
│           │                       │                             │
│           ▼                       ▼                             │
│  ┌─────────────────────────────────────────────────────────────┐ │
│  │              BRIDGE CONFIGURATION MANAGER                  │ │
│  │                                                             │ │
│  │  ┌─────────────┐  ┌──────────────┐  ┌─────────────────┐   │ │
│  │  │   Config    │  │    EEPROM    │  │    Command      │   │ │
│  │  │  Structure  │  │   Storage    │  │   Processing    │   │ │
│  │  │             │  │              │  │                 │   │ │
│  │  │ • Timing    │◄─┤ • Load/Save  │◄─┤ • Serial Parse │   │ │
│  │  │ • Distance  │  │ • Validation │  │ • Parameter Set │   │ │
│  │  │ • Motor     │  │ • Magic #    │  │ • Range Check  │   │ │
│  │  │ • Features  │  │ • Integrity  │  │ • Feedback     │   │ │
│  │  └─────────────┘  └──────────────┘  └─────────────────┘   │ │
│  └─────────────────────────────────────────────────────────────┘ │
│           │                                                     │
│           ▼                                                     │
│  ┌─────────────────────────────────────────────────────────────┐ │
│  │                    DEBUG LOGGING SYSTEM                    │ │
│  │                                                             │ │
│  │  ┌─────────────┐  ┌──────────────┐  ┌─────────────────┐   │ │
│  │  │   Debug     │  │   Sensor     │  │     State       │   │ │
│  │  │   Logger    │  │   Logger     │  │    Logger       │   │ │
│  │  │             │  │              │  │                 │   │ │
│  │  │ • Messages  │  │ • Hardware   │  │ • Transitions   │   │ │
│  │  │ • Timestamp │  │ • Status     │  │ • Flow Track    │   │ │
│  │  │ • Conditnl  │  │ • Periodic   │  │ • State Names   │   │ │
│  │  └─────────────┘  └──────────────┘  └─────────────────┘   │ │
│  └─────────────────────────────────────────────────────────────┘ │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
                                │
                                ▼
┌─────────────────────────────────────────────────────────────────┐
│                        HARDWARE LAYER                          │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  ┌─────────────┐  ┌──────────────┐  ┌─────────────────────────┐ │
│  │   Sensors   │  │    Motor     │  │    Traffic Lights       │ │
│  │             │  │              │  │                         │ │
│  │ • Sonic     │  │ • DC Motor   │  │ • Red/Yellow/Green      │ │
│  │ • Limit SW  │  │ • Direction  │  │ • State Control         │ │
│  │ • E-Stop    │  │ • Speed      │  │ • Safety Indication     │ │
│  └─────────────┘  └──────────────┘  └─────────────────────────┘ │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

## Data Flow Diagram

```
┌──────────────┐    ┌─────────────────┐    ┌──────────────────┐
│ Serial Input │───►│ Command Parser  │───►│ Config Manager   │
└──────────────┘    └─────────────────┘    └──────────────────┘
                                                      │
                                                      ▼
┌──────────────┐    ┌─────────────────┐    ┌──────────────────┐
│    EEPROM    │◄───│ Configuration   │◄───│ Parameter Update │
│   Storage    │    │  Validation     │    │ & Range Check    │
└──────────────┘    └─────────────────┘    └──────────────────┘
       │                     │                        │
       ▼                     ▼                        ▼
┌──────────────┐    ┌─────────────────┐    ┌──────────────────┐
│ System Boot  │───►│ Config Load     │───►│ State Machine    │
│ Initialize   │    │ or Reset        │    │ Configuration    │
└──────────────┘    └─────────────────┘    └──────────────────┘
                                                      │
                                                      ▼
┌──────────────┐    ┌─────────────────┐    ┌──────────────────┐
│ Debug Output │◄───│ Conditional     │◄───│ Runtime          │
│ (Serial)     │    │ Logging         │    │ Operations       │
└──────────────┘    └─────────────────┘    └──────────────────┘
```

## State Machine Flow with Configuration

```
                    ┌─────────────────┐
                    │    LOWERED      │
                    │ (Traffic: GO)   │
                    └─────────────────┘
                            │
                            │ boatDetected()
                            │ [distance < config.boatDetectionDistance]
                            ▼
                    ┌─────────────────┐
                    │  PREPARE_RAISE  │
                    │ (Traffic: STOP) │
                    └─────────────────┘
                            │
                            │ timerFinished()
                            │ [time > config.actionDelay]
                            ▼
                    ┌─────────────────┐
                    │    RAISING      │
                    │ (Motor: UP)     │
                    └─────────────────┘
                            │
                            │ topLimitHit() || motionTimeout()
                            │ [time > config.moveTimeout]
                            ▼
                    ┌─────────────────┐
                    │     RAISED      │
                    │ (Traffic: STOP) │
                    └─────────────────┘
                            │
                            │ areaClear()
                            │ [distance > config.areaClearDistance]
                            ▼
                    ┌─────────────────┐
                    │  PREPARE_LOWER  │
                    │ (Traffic: STOP) │
                    └─────────────────┘
                            │
                            │ timerFinished()
                            │ [time > config.actionDelay]
                            ▼
                    ┌─────────────────┐
                    │    LOWERING     │
                    │ (Motor: DOWN)   │
                    └─────────────────┘
                            │
                            │ bottomLimitHit() || motionTimeout()
                            ▼
                    ┌─────────────────┐
                    │    LOWERED      │
                    │ (Traffic: GO)   │
                    └─────────────────┘

Emergency States:
    eStopPressed() ──► EMERGENCY_LOWER ──► LOWERED
                  └──► EMERGENCY_RAISE  ──► RAISED
```

## Configuration Parameter Dependencies

```
┌─────────────────────────────────────────────────────────────────┐
│                    CONFIGURATION PARAMETERS                    │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  TIMING PARAMETERS                                              │
│  ├── actionDelay ────────► State transition timing             │
│  ├── moveTimeout ────────► Motor safety timeout                │
│  ├── emergencyDelay ─────► Emergency operation duration        │
│  └── debugLogInterval ───► Sensor logging frequency            │
│                                                                 │
│  DISTANCE PARAMETERS                                            │
│  ├── boatDetectionDistance ──► Boat presence threshold         │
│  ├── areaClearDistance ──────► Area clear threshold            │
│  ├── sonicSensorMinRange ────► Minimum valid reading           │
│  └── sonicSensorMaxRange ────► Maximum valid reading           │
│                                                                 │
│  MOTOR PARAMETERS                                               │
│  ├── motorSpeedFast ─────────► High-speed operation            │
│  ├── motorSpeedSlow ─────────► Low-speed operation             │
│  ├── motorDirection1 ────────► Up/raise direction              │
│  └── motorDirection2 ────────► Down/lower direction            │
│                                                                 │
│  FEATURE TOGGLES                                                │
│  ├── enableDebugLogging ─────► General debug messages          │
│  ├── enableSensorLogging ────► Periodic sensor status          │
│  ├── enableStateLogging ─────► State transition messages       │
│  ├── enableEmergencyStop ────► E-stop functionality            │
│  └── enableMotionTimeout ────► Motor timeout safety            │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

## Memory Organization

```
EEPROM Layout (56+ bytes):
┌────────────┬──────────────┬────────────┬──────────────┬─────────────┐
│ Magic      │ Timing       │ Distance   │ Motor        │ Features    │
│ Number     │ Parameters   │ Thresholds │ Settings     │ Flags       │
│ (4 bytes)  │ (16 bytes)   │ (8 bytes)  │ (4 bytes)    │ (8 bytes)   │
├────────────┼──────────────┼────────────┼──────────────┼─────────────┤
│ 0xBE3F2024 │ actionDelay  │ boatDetect │ speedFast    │ debugLog    │
│            │ moveTimeout  │ areaClear  │ speedSlow    │ sensorLog   │
│            │ emergDelay   │ sonicMin   │ direction1   │ stateLog    │
│            │ debugInterval│ sonicMax   │ direction2   │ eStop       │
│            │              │            │              │ timeout     │
└────────────┴──────────────┴────────────┴──────────────┴─────────────┘
                                                                      │
                                                                      ▼
                                                            ┌─────────────┐
                                                            │ Reserved    │
                                                            │ Space       │
                                                            │ (16 bytes)  │
                                                            └─────────────┘
```

## Command Processing Flow

```
Serial Input
     │
     ▼
┌─────────────────┐
│ String Buffer   │
│ (readStringUntil│
│ newline)        │
└─────────────────┘
     │
     ▼
┌─────────────────┐
│ Input           │
│ Sanitization    │
│ (trim, toLower) │
└─────────────────┘
     │
     ▼
┌─────────────────┐      ┌─────────────────┐
│ Command Type    │ Yes  │ Single Command  │
│ Detection       │─────►│ Processing      │
│ (help, show,    │      │ (help, show,    │
│ save, reset)    │      │ save, reset)    │
└─────────────────┘      └─────────────────┘
     │ No
     ▼
┌─────────────────┐
│ Parameter=Value │
│ Parsing         │
│ (indexOf '=')   │
└─────────────────┘
     │
     ▼
┌─────────────────┐      ┌─────────────────┐
│ Parameter Type  │ Yes  │ Value           │
│ Validation      │─────►│ Conversion      │
│ (known param?)  │      │ & Range Check   │
└─────────────────┘      └─────────────────┘
     │ No                        │
     ▼                           ▼
┌─────────────────┐      ┌─────────────────┐
│ Error Message   │      │ Configuration   │
│ "Unknown        │      │ Update &        │
│ command"        │      │ Confirmation    │
└─────────────────┘      └─────────────────┘
```

This architecture provides a clear understanding of how the various components interact and how the configuration system enables runtime parameter modification while maintaining system safety and integrity.