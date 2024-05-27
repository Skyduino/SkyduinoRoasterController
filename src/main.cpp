#include <Arduino.h>
#include <filterRC.h>

#include "roaster.h"
#include "logging.h"
#include "commands.h"
#include "tc-handler.h"


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
    // Filters
    {filterRC(), filterRC(), filterRC(), filterRC()},
    false,  // isMetric
  },
  // t_Status
  {
    TimerMS(TC4_COMM_TIMEOUT_MS),      // tc4ComTimeOut
    1                                  // tcStatus
  }
};

static TCHandler tcHandler(&state);

void setup() {
  Serial.begin(115200);
  Serial.setTimeout(100);

  setupCommandHandlers();
  tcHandler.begin();

  Serial.println(F(VERSION));
}

void loop() {
  // Check Serial Communication
  commandsLoopTick();

  // update thermocouple & ambient temp
  tcHandler.loopTick();
}