#include <cmndproc.h>

#include "logging.h"
#include "state.h"
#include "commands.h"
#include "commands/handler_read.h"
#include "commands/handler_chan.h"
#include "commands/handler_version.h"
#include "commands/handler_ot.h"

// command line parameter delimiters
#define DELIM "; ,="

/*
 * Global state
 */
extern t_State state;

cmndRead    cmnd_handler_read;
cmndChan    cmnd_handler_chan;
cmndVersion cmnd_handler_version;
cmndOT1     cmnd_handler_ot1 = cmndOT1(&(state.commanded.heat));

static CmndInterp ci( DELIM ); // command interpreter object

void setupCommandHandlers(void) {
    Command* commands[] = {
        &cmnd_handler_chan,
        &cmnd_handler_read,
        &cmnd_handler_version,
        &cmnd_handler_ot1
    };

    uint8_t count = sizeof(commands) / sizeof(commands[0]);
    for (uint8_t i=0; i < count; i++) {
        Command* handler = commands[i];
        bool success = handler->begin();
        if (success) {
            DEBUG(F("Successfuly initialized: "));
            DEBUGLN(handler->getName());
            Serial.print(F("Successfuly initialized: "));
            Serial.println(handler->getName());
            ci.addCommand(handler);
        }
    }
}

void commandsLoopTick(void) {
    ci.checkSerial();
}