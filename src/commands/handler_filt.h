#ifndef __CMD_FILT_H
#define __CMD_FILT_H

#include "handler_filt.h"
#include "base.h"

class cmndFilt : public Command {
    public:
        cmndFilt(State *state);
    
    protected:
        void _doCommand( CmndParser* pars);

    private:
        State *state;
};

#endif // __CMD_FILT_H