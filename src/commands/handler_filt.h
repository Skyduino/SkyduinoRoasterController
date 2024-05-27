#ifndef __CMD_FILT_H
#define __CMD_FILT_H

#include "handler_filt.h"
#include "base.h"

class cmndFilt : public Command {
    public:
        cmndFilt(t_State *state);
        virtual bool doCommand( CmndParser* pars);

    private:
        t_State *state;
};

#endif // __CMD_FILT_H