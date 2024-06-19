#include "state.h"
#include "handler_filt.h"

#define CMD_FILT "FILT"

// ----------------------------
cmndFilt::cmndFilt(State *state):
    Command( CMD_FILT ), state( state ) {
}

// FILT,ppp,ppp,ppp,ppp where ppp = percent filtering on logical channels 1 to 4
void cmndFilt::_doCommand(CmndParser *pars) {
    for (uint8_t idx=0; idx < TEMPERATURE_CHANNELS_MAX; ++idx) {
        uint8_t len = strlen(pars->paramStr(idx));
        if( len > 0 ) {  // is there a parameter?
            int filter = atoi( pars->paramStr(idx+1) );
            state->reported.setChanFilter(idx, filter);
        }
    }
}