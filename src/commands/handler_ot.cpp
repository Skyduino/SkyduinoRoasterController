#include "roaster.h"
#include "handler_ot.h"

#define CMD_OT1 "OT1"
#define CMD_OT2 "OT2"


cmndOT1::cmndOT1(State *state):
    Command(CMD_OT1, state)
}


cmndOT1::_handleValue(int32_t value) {
    state->commanded.heat.set(vlaue);
}


cmndOT2::cmndOT2(State *state):
    Command(CMD_OT2, state)
}


cmndOT2::_handleValue(int32_t value) {
    state->commanded.vent.set(vlaue);
}