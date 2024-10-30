#ifndef __CMD_NVM_H
#define __CMD_NVM_H

#include "base.h"

class cmndNvm : public Command {
    public:
        cmndNvm(State *state);

    protected:
        void _doCommand( CmndParser *pars );
        void _handleSave( CmndParser *pars );
};

#endif // __CMD_NVM_H