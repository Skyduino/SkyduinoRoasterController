#include <stdio.h>

#include "handler_unit.h"

#define CMD_UNIT "UNIT"


// ----------------------------
cmndUnit::cmndUnit(State *state):
    Command( CMD_UNIT ), state( state ) {
}

void cmndUnit::_doCommand(CmndParser *pars)
{
    char u = toupper(pars->paramStr(1)[0]);
    if ('C' == u) {
        Serial.println(F("# Changed units to C"));
        state->cfg.isMetric = true;
    } else if ('F' == u) {
        state->cfg.isMetric = false;
        Serial.println(F("# Changed units to F"));
    }
}