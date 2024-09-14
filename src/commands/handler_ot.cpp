#include "roaster.h"
#include "handler_ot.h"

#define CMD_OT1 "OT1"
#define CMD_OT2 "OT2"


cmndOT1::cmndOT1(State *state):
    ControlCommand(CMD_OT1, state) {
}


void cmndOT1::_handleValue(int32_t value) {
    state->commanded.heat.set(value);
    state->commanded.setControlToArtisan(true);
}


cmndOT2::cmndOT2(State *state):
    ControlCommand(CMD_OT2, state) {
}


void cmndOT2::_handleValue(int32_t value) {
    state->commanded.vent.set(value);
    state->commanded.setControlToArtisan(true);
}