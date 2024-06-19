#include "logging.h"
#include "base.h"


Command::Command(const char* cmdName, State *state):
    CmndBase(cmdName), state(state) {
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


ControlCommand::ControlCommand(const char *cmdName, State *state):
    Command(cmdName, state) {
}


void ControlCommand::_doCommand(CmndParser *pars)
{
    int32_t newValue = strtol(pars->paramStr(1), NULL, 10);
    if (newValue < this->min || newValue > this->max) {
        WARN(F("Value '"));
        WARN(newValue);
        WARNLN(F("' is out of uint8_t range, ignoring it"));
        return;
    }
    
    this->_handleValue(newValue);
}