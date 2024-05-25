#ifndef __CMD_OT_H
#define __CMD_OT_H

#include "base.h"

class cmndOT : public Command {
    public:
        cmndOT(const char* cmdName, uint8_t* otValue);
        virtual bool doCommand( CmndParser* pars);
    private:
        uint8_t* otValue;
};

class cmndOT1 : public cmndOT {
    public:
        cmndOT1(uint8_t* otValue);
};

#endif // __CMD_OT_H