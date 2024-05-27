#ifndef __CMD_UNIT_H
#define __CMD_UNIT_H

#include "state.h"
#include "base.h"

class cmndUnit : public Command {
    public:
        cmndUnit(t_State *state);
        virtual bool doCommand( CmndParser *pars);

    private:
        t_State *state;
};

#endif // __CMD_UNIT_H