#ifndef __CMD_STUNE_H
#define __CMD_STUNE_H

#include "base.h"

class cmndSTune : public Command {
    public:
        cmndSTune(State *state);

    protected:
        void _doCommand( CmndParser *pars );
        void _handleStart( CmndParser *pars );
        void _handleStop( CmndParser *pars );
};

#endif // __CMD_STUNE_H