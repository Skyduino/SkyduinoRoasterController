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

#define TEMPERATURE_ROASTER(x) x[TEMPERATURE_CHANNEL_ROASTER]
#define TEMPERATURE_TC(x)      x[TEMPERATURE_CHANNEL_THERMOCOUPLE]

// PIN Definitions
#define PIN_LED     PA3
#define PIN_EXHAUST PA8
#define PIN_DRUM    PA15
#define PIN_HEAT    PB9
#define PIN_NTC     PC5
#define PIN_COOL    PC8

#define SPI_BTCS            PA4

// PWM frequencies Hz
#define PWM_FREQ_COOL       60
#define PWM_FREQ_DRUM       60
#define PWM_FREQ_EXHAUST    60
#define PWM_FREQ_HEAT       1
#define PWM_FREQ_LED        120

#endif  // __ROASTER_H