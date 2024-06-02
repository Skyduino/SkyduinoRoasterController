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

void cmndVersion::_doCommand(CmndParser *pars) {
    Serial.println(F(VERSION));
}