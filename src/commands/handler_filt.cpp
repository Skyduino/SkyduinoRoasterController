#include "state.h"
#include "handler_filt.h"

#define CMD_FILT "FILT"

// ----------------------------
cmndFilt::cmndFilt(t_State *state):
    Command( CMD_FILT ), state( state ) {
}

// FILT,ppp,ppp,ppp,ppp where ppp = percent filtering on logical channels 1 to 4
bool cmndFilt::doCommand(CmndParser *pars) {
    if( 0 != strcmp( keyword, pars->cmndName() ) ) {
        return false;
    }

    for (uint8_t idx=0; idx < TEMPERATURE_CHANNELS_MAX; ++idx) {
        uint8_t len = strlen(pars->paramStr(idx));
        if( len > 0 ) {  // is there a parameter?
            int filter = atoi( pars->paramStr(idx+1) );
            // convert from logical to physical channel
            uint8_t chan = state->cfg.chanMapping[idx];
            if( chan > 0 ) { // is the physical channel active?
                --chan;
                state->cfg.filter[chan].init( filter );
                Serial.print(F("# Physical channel ")); Serial.print( chan );
                Serial.print(F(" filter set to ")); Serial.println( filter );
            }
        }
    }

    return true;
}