#include "roaster.h"
#include "handler_skywalker.h"

#define CMD_COOL    "COOL"
#define CMD_DRUM    "DRUM"
#define CMD_OFF     "OFF"


cmndCool::cmndCool(State *state):
    ControlCommand(CMD_COOL, state) {
}


void cmndCool::_handleValue(int32_t value) {
    if ( value ) {
        state->commanded.cool.on();
    } else {;
        state->commanded.cool.off();
    }
}


cmndDrum::cmndDrum(State *state):
    ControlCommand(CMD_DRUM, state) {
}


void cmndDrum::_handleValue(int32_t value) {
    state->commanded.drum.set(value);
}


cmndOff::cmndOff(State *state):
    Command(CMD_OFF, state) {
}


void cmndOff::_doCommand(CmndParser *pars) {
    state->commanded.off();
}
