#include <Arduino.h>

#include "roaster.h"
#include "logging.h"
#include "tick-timer.h"
#include "commands.h"


/*
 * Until this is replaced by an Class, this structure
 * contains the entire state, status, config, reported
 * and commanded roaster status
 */
t_State state = {
  // t_StateCommanded
  {0, 0, 0, 0, 0},
  // t_StateReported
  {0, 0, 0, 0, 0},
  // t_Config
  {
    // chanMapping
    {TEMPERATURE_CHANNEL_ROASTER+1, TEMPERATURE_CHANNEL_THERMOCOUPLE+1, 0, 0},
    false,  // isMetric
  },
  // t_Status
  {
    TimerMS(TC4_COMM_TIMEOUT_MS),      // tc4ComTimeOut
    1                                  // tcStatus
  }
};


void setup() {
  Serial.begin(115200);
  Serial.setTimeout(100);

  setupCommandHandlers();
  Serial.println(F(VERSION));
}

void loop() {
  // Check Serial Communication
  commandsLoopTick();
}