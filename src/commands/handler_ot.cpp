#include <stdlib.h>

#include "logging.h"
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
        int32_t newValue = strtol(pars->paramStr(1), NULL, 10);
        if (newValue < 0 || newValue > 255) {
            WARN(F("Value '"));
            WARN(newValue);
            WARNLN(F("' is out of uint8_t range, ignoring it"));
            return true;
        }

        char buf[128];
        uint8_t len;

        len = sprintf(buf, "Executing %s command: old value: %u, new value: %d",
                      pars->cmndName(),
                      *this->otValue,
                      (int) newValue
                     );
        buf[len+1] = '\0';
        Serial.println(buf);

        *this->otValue = (uint8_t) newValue & 0xff;
        return true;
    }

   return false;
}