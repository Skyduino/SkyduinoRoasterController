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

void setup() {
  Serial.begin(115200);
  Serial.setTimeout(100);
  Serial.println(F(VERSION));

  while ( !(state.begin()) ) {
    Serial.println(F("Failed to initialize"));
    delay(100);
  };

  setupCommandHandlers();

}

void loop() {
  // Check Serial Communication
  commandsLoopTick();

  state.loopTick();
}