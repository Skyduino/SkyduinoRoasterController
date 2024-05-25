#include <cmndproc.h>

#include <state.h>
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

cmndRead cmnd_handler_read;
cmndChan cmnd_handler_chan;
cmndVersion cmnd_handler_version;
cmndOT1 cmndOT1(&(state.commanded.heat));

static CmndInterp ci( DELIM ); // command interpreter object

void setupCommandHandlers(void) {
    ci.addCommand(&cmnd_handler_chan);
    ci.addCommand(&cmnd_handler_read);
    ci.addCommand(&cmnd_handler_version);
    ci.addCommand(&cmndOT1);
}

void commandsLoopTick(void) {
    ci.checkSerial();
}