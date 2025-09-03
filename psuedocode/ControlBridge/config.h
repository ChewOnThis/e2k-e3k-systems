#pragma once
 
#include <WebServer.h>
#include <WiFi.h>

// TO TUNE
static const uint32_t T_OPEN_EXPECTED   = 5200;   // ms, when to start slowing while raising
static const uint32_t T_OPEN_MAX        = 7000;   // ms, hard ceiling to reach top limit
static const uint32_t T_CLOSE_EXPECTED  = 5200;   // ms, when to start slowing while lowering
static const uint32_t T_CLOSE_MAX       = 7000;   // ms, hard ceiling to reach bottom limit
static const uint32_t T_YELLOW          = 2000;   // ms, yellow light before raising