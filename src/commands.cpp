#include <cmndproc.h>

#include "logging.h"
#include "state.h"
#include "commands.h"
#include "commands/handler_chan.h"
#include "commands/handler_dfu.h"
#include "commands/handler_filt.h"
#include "commands/handler_ot.h"
#include "commands/handler_read.h"
#include "commands/handler_stat.h"
#include "commands/handler_skywalker.h"
#ifdef USE_STEPPER_DRUM
#include "commands/handler_stepper_drum.h"
#endif // USE_STEPPER_DRUM
#include "commands/handler_unit.h"
#include "commands/handler_version.h"

// command line parameter delimiters
#define DELIM "; ,="

/*
 * Global state
 */
extern State state;

cmndAbort   cmnd_handler_abrt = cmndAbort( &state );
cmndChan    cmnd_handler_chan = cmndChan( &state );
cmndCool    cmnd_handler_cool = cmndCool( &state );
cmndDrum    cmnd_handler_drum = cmndDrum( &state );
cmndDFU     cmnd_handler_dfu;
cmndFilt    cmnd_handler_filt = cmndFilt( &state );
cmndOff     cmnd_handler_off  = cmndOff( &state );
cmndOT1     cmnd_handler_ot1  = cmndOT1( &state );
cmndOT2     cmnd_handler_ot2  = cmndOT2( &state );
cmndRead    cmnd_handler_read = cmndRead( &state );
cmndStat    cmnd_handler_stat = cmndStat( &state );
#ifdef USE_STEPPER_DRUM
cmndSteps   cmnd_handler_steps= cmndSteps( &state );
cmndMaxRPM  cmnd_handler_rpm  = cmndMaxRPM( &state ); 
#endif // USE_STEPPER_DRUM
cmndUnit    cmnd_handler_unit = cmndUnit( &state );
cmndVersion cmnd_handler_version;

static CmndInterp ci( DELIM ); // command interpreter object

void setupCommandHandlers(void) {
    Command* commands[] = {
#ifdef USE_STEPPER_DRUM
        &cmnd_handler_steps,
        &cmnd_handler_rpm,
#endif // USE_STEPPER_DRUM
        &cmnd_handler_abrt,
        &cmnd_handler_version,
        &cmnd_handler_dfu,
        &cmnd_handler_stat,
        &cmnd_handler_unit,
        &cmnd_handler_filt,
        &cmnd_handler_chan,
        &cmnd_handler_off,
        &cmnd_handler_cool,
        &cmnd_handler_drum,
        &cmnd_handler_ot1,
        &cmnd_handler_ot2,
        &cmnd_handler_read
    };

    uint8_t count = sizeof(commands) / sizeof(commands[0]);
    for (uint8_t i=0; i < count; i++) {
        Command* handler = commands[i];
        bool success = handler->begin();
        if (success) {
            DEBUG(F("Successfuly initialized: "));
            DEBUGLN(handler->getName());
            ci.addCommand(handler);
        }
    }
}

void commandsLoopTick(void) {
    ci.checkSerial();
}