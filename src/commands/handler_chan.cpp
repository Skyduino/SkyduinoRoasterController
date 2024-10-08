#include "handler_chan.h"
#include "state.h"

#define CMD_CHAN "CHAN"

// ----------------------------
cmndChan::cmndChan(State *state) :
    Command( CMD_CHAN, state ) {
}

void cmndChan::_doCommand(CmndParser *pars)
{
    char str[2];
    uint8_t n;
    uint8_t len = strlen( pars->paramStr(1) );
    if( len == TEMPERATURE_CHANNELS_MAX ) { // must match number of channels or take no action
        for( int i = 0; i < len; i++ ) {
            str[0] = pars->paramStr(1)[i]; // next character
            str[1] = '\0'; // force it to be char[2]
            n = atoi( str );
            if( n <= TEMPERATURE_CHANNELS_MAX ) {
                state->reported.setChanMapping(i, n);
            } else {
                state->reported.setChanMapping(i, 0);
            }
      }
      Serial.print(F("# Active channels set to "));
      Serial.println( pars->paramStr(1) );
    }
}