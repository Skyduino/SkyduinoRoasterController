#include <cmndproc.h>

#include "commands.h"
#include "commands/handler_read.h"
#include "commands/handler_chan.h"

// command line parameter delimiters
#define DELIM "; ,="

extern cmndRead cmnd_handler_read;
extern cmndRead cmnd_handler_chan;

static CmndInterp ci( DELIM ); // command interpreter object

void setupCommandHandlers(void) {
    ci.addCommand(&cmnd_handler_chan);
    ci.addCommand(&cmnd_handler_read);
}