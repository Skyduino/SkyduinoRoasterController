#include "handler_stat.h"
#include "state.h"

#define CMD_STAT "STAT"

cmndStat::cmndStat(State *state) :
    Command( CMD_STAT, state ) {
}


void cmndStat::_doCommand(CmndParser *pars) {
    state->reported.printStatistics();
}