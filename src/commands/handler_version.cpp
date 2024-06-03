#include "handler_version.h"
#include "state.h"
#include "roaster.h"

#define CMD_VERSION "VERSION"

// ----------------------------
cmndVersion::cmndVersion() :
    Command ( CMD_VERSION ) {
}

void cmndVersion::_doCommand(CmndParser *pars) {
    Serial.println(F(VERSION));
}