#ifndef __ROASTER_H
#define __ROASTER_H

#include "state.h"

#define VERSION "Skyduino version " SKYDUINO_VERSION " " SKYDUINO_COMMIT

// TC4 protocol communication timeout in milliseconds
#define TC4_COMM_TIMEOUT_MS                 10000UL
// physical channel for thermocouple temperature
#define TEMPERATURE_CHANNEL_THERMOCOUPLE    0
// physical channel for temperature reported by roaster
#define TEMPERATURE_CHANNEL_ROASTER         1

#define TEMPERATURE_ROASTER(x) x.chanTemp[TEMPERATURE_CHANNEL_ROASTER]
#define TEMPERATURE_TC(x)      x.chanTemp[TEMPERATURE_CHANNEL_THERMOCOUPLE]

#endif  // __ROASTER_H