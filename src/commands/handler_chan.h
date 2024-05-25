#ifndef __CMD_CHAN_H
#define __CMD_CHAN_H

#include "base.h"

class cmndChan : public Command {
    public:
        cmndChan();
        virtual bool doCommand( CmndParser* pars);
};

#endif // __CMD_CHAN_H