#include "handler_read.h"
#include "state.h"

#define CMD_READ "READ"

cmndRead::cmndRead(State *state) :
    Command( CMD_READ, state ) {
}


void cmndRead::_doCommand(CmndParser *pars) {
    state->printState();
}