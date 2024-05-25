#include "handler_version.h"
#include "state.h"
#include "roaster.h"

#define CMD_VERSION "VERSION"


/*
 * Global state
 */
extern t_State state;

// ----------------------------
cmndVersion::cmndVersion() :
    Command ( CMD_VERSION ) {
}

bool cmndVersion::doCommand(CmndParser *pars)
{
    if( strcmp( keyword, pars->cmndName() ) == 0 ) {
        Serial.println(F(VERSION));
        return true;
    }

   return false;
}