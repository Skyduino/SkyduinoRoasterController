#ifndef _SW_STATE_H
#define _SW_STATE_H

#include <filterRC.h>

#include "tick-timer.h"

typedef struct {
  uint8_t heat;
  uint8_t vent;
  uint8_t cool;
  uint8_t filter;
  uint8_t drum;
} t_StateCommanded;

typedef struct {
    double chanTemp[TEMPERATURE_CHANNELS_MAX]; // Physical temp channels
    double ambient;  // Ambient temperature
} t_StateReported;

typedef struct {
    uint8_t chanMapping[TEMPERATURE_CHANNELS_MAX];
    filterRC<double> filter[TEMPERATURE_CHANNELS_MAX];
    bool isMetric;
} t_Config;

typedef struct {
    TimerMS tc4ComTimeOut;
    uint8_t tcStatus;
} t_Status;

class State {
    Public:
        t_StateCommanded    commanded;
        t_StateReported     reported;
        t_Config            cfg;
        t_Status            status;
};

#endif  // _SW_STATE_H