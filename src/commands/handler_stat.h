#ifndef __CMD_STAT_H
#define __CMD_STAT_H

#include "base.h"

class cmndStat : public Command {
    public:
        cmndStat(State *state);
    protected:
        void _doCommand( CmndParser *pars);
};

#endif // __CMD_STAT_H