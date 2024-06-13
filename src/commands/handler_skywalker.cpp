#include "roaster.h"
#include "handler_skywalker.h"

#define CMD_COOL    "COOL"
#define CMD_DRUM    "DRUM"


cmndCool::cmndCool(State *state):
    ControlCommand(CMD_COOL, state) {
}


cmndCool::_handleValue(int32_t value) {
    if ( value ) {
        state->commanded.cool.on();
    } else {;
        state->commanded.cool.off();
    }
}


cmndDrum::cmndDrum(State *state):
    ControlCommand(CMD_DRUM, state) {
}


cmndDrum::_handleValue(int32_t value) {
    state->commanded.drum.set(value);
}