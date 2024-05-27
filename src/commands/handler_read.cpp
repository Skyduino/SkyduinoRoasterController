#include "handler_read.h"
#include "state.h"

#define CMD_READ "READ"


/*
 * Global state
 */
extern t_State state;

// ----------------------------
cmndRead::cmndRead() :
    Command( CMD_READ ) {
}

bool cmndRead::doCommand(CmndParser *pars)
{
  if( strcmp( keyword, pars->cmndName() ) == 0 ) {
    handleRead();
    return true;
  }

  return false;
}

void cmndRead::handleRead(void) {
  Serial.print(state.reported.ambient);
  uint8_t mapping = 0;
  for (uint8_t i = 0; i < TEMPERATURE_CHANNELS_MAX; i++) {
    mapping = state.cfg.chanMapping[i];
    if ((mapping >= 1)
        && (mapping <= TEMPERATURE_CHANNELS_MAX)) {
        Serial.print(F(","));
        Serial.print(state.reported.chanTemp[mapping - 1]);
    }
  }
  Serial.print(F(","));
  Serial.print(state.commanded.heat);
  Serial.print(',');
  Serial.print(state.commanded.vent);
  Serial.print(',');
  Serial.println(F("0"));

  state.status.tc4ComTimeOut.reset();
}