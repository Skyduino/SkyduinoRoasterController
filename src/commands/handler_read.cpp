#include "handler_read.h"
#include "state.h"

#define CMD_READ "READ"

cmndRead::cmndRead(State *state) :
    Command( CMD_READ, state ) {
}


void cmndRead::_doCommand(CmndParser *pars) {
  Serial.print(state->reported.getAmbient());
  uint8_t mapping = 0;
  for (uint8_t i = 0; i < TEMPERATURE_CHANNELS_MAX; i++) {
    mapping = state->cfg.getChanMapping(i);
    if ((mapping >= 1)
        && (mapping <= TEMPERATURE_CHANNELS_MAX)) {
        Serial.print(F(","));
        Serial.print(state->reported.getChanTemp(mapping - 1));
    }
  }
  Serial.print(F(","));
  Serial.print(state->commanded.heat.get());
  Serial.print(',');
  Serial.print(state->commanded.vent.get());
  Serial.print(',');
  Serial.println(F("0"));
}