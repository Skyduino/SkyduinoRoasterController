#ifndef __CMD_UNIT_H
#define __CMD_UNIT_H

#include "state.h"
#include "base.h"

class cmndUnit : public Command {
    public:
        cmndUnit(State *state);
    
    protected:
        void _doCommand( CmndParser *pars);

    private:
        State *state;
};

#endif // __CMD_UNIT_H