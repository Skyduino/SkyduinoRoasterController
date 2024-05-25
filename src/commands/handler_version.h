#ifndef __CMD_VERSION_H
#define __CMD_VERSION_H

#include "base.h"

class cmndVersion : public Command {
    public:
        cmndVersion();
        virtual bool doCommand( CmndParser* pars);
};

#endif // __CMD_VERSION_H