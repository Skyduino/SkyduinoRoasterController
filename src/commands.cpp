#include <cmndproc.h>

#include "logging.h"
#include "state.h"
#include "commands.h"
#include "commands/handler_chan.h"
#include "commands/handler_dfu.h"
#include "commands/handler_filt.h"
#include "commands/handler_ot.h"
#include "commands/handler_read.h"
#include "commands/handler_skywalker.h"
#include "commands/handler_unit.h"
#include "commands/handler_version.h"

// command line parameter delimiters
#define DELIM "; ,="

/*
 * Global state
 */
extern t_State state;

cmndChan    cmnd_handler_chan;
cmndCool    cmnd_handler_cool= cmndCool(&(state.commanded.cool));
cmndDrum    cmnd_handler_drum= cmndDrum(&(state.commanded.drum));
cmndDFU     cmnd_handler_dfu;
cmndFilt    cmnd_handler_filt= cmndFilt(&state);
cmndLED     cmnd_handler_led = cmndLED();
cmndOT1     cmnd_handler_ot1 = cmndOT1(&(state.commanded.heat));
cmndOT2     cmnd_handler_ot2 = cmndOT2(&(state.commanded.vent));
cmndRead    cmnd_handler_read;
cmndUnit    cmnd_handler_unit= cmndUnit(&state);
cmndVersion cmnd_handler_version;

static CmndInterp ci( DELIM ); // command interpreter object

void setupCommandHandlers(void) {
    Command* commands[] = {
        &cmnd_handler_led,
        &cmnd_handler_version,
        &cmnd_handler_dfu,
        &cmnd_handler_unit,
        &cmnd_handler_filt,
        &cmnd_handler_chan,
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