#include <Arduino.h>
#include <filterRC.h>

#include "roaster.h"
#include "state.h"
#include "logging.h"
#include "commands.h"
#include "safemon.h"


/*
 * Until this is replaced by an Class, this structure
 * contains the entire state, status, config, reported
 * and commanded roaster status
 */
State state;
SafetyMonitor safeMon = SafetyMonitor( &state );

void setup() {
  Serial.begin(115200);
  Serial.setTimeout(100);
  Serial.println(F(VERSION));

  safeMon.begin();

  while ( !(state.begin()) ) {
    Serial.println(F("Failed to initialize"));
    delay(100);
  };

  setupCommandHandlers();

}

void loop() {
  // for loop timing statistics
  state.stats.loopStart();

  // Check Serial Communication
  commandsLoopTick();

  state.loopTick();

  safeMon.loopTick();

  state.stats.loopEnd();
}