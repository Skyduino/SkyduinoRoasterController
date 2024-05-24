#ifndef _SW_STATE_H
#define _SW_STATE_H

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
} t_StateReported;

typedef struct {
    uint8_t chanMapping[TEMPERATURE_CHANNELS_MAX];
    char CorF;
} t_Config;

typedef struct {
    TimerMS tc4ComTimeOut;
    uint8_t tcStatus;
} t_Status;

typedef struct {
    t_StateCommanded    commanded;
    t_StateReported     reported;
    t_Config            cfg;
    t_Status            status;
} t_State;

#endif  // _SW_STATE_H