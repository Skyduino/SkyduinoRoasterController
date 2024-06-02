#ifndef __CMD_CHAN_H
#define __CMD_CHAN_H

#include "base.h"

class cmndChan : public Command {
    public:
        cmndChan();
    protected:
        void _doCommand( CmndParser* pars);
};

#endif // __CMD_CHAN_H