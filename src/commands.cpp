#include <cmndproc.h>

#include "commands.h"
#include "commands/handler_read.h"
#include "commands/handler_chan.h"
#include "commands/handler_version.h"

// command line parameter delimiters
#define DELIM "; ,="

extern cmndRead cmnd_handler_read;
extern cmndChan cmnd_handler_chan;
extern cmndVersion cmnd_handler_version;

static CmndInterp ci( DELIM ); // command interpreter object

void setupCommandHandlers(void) {
    ci.addCommand(&cmnd_handler_chan);
    ci.addCommand(&cmnd_handler_read);
    ci.addCommand(&cmnd_handler_version);
}

void commandsLoopTick(void) {
    ci.checkSerial();
}