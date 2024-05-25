#include "handler_ot.h"
#include "state.h"

#define CMD_OT1 "OT1"
#define CMD_OT2 "OT2"


cmndOT1::cmndOT1(uint8_t* otValue) : cmndOT(CMD_OT1, otValue) {
}

// ----------------------------
cmndOT::cmndOT(const char* cmdName, uint8_t* otValue) : CmndBase ( cmdName) {
   this->otValue = otValue; 
}

bool cmndOT::doCommand(CmndParser *pars)
{
    if( strcmp( keyword, pars->cmndName() ) == 0 ) {
        return true;
    }

   return false;
}