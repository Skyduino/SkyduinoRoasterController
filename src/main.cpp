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
State state;
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