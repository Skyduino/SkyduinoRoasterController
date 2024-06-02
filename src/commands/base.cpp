#include "base.h"

Command::Command(const char* cmdName): CmndBase(cmdName) {
}

bool Command::begin(void) {
    return true;
}

bool Command::doCommand(CmndParser *pars) {
    if( 0 != strcmp( keyword, pars->cmndName() ) ) {
        // no comand name match, bail out
        return false;
    }

    // handle the matched command
    this->_doCommand(pars);
    return true;
}
