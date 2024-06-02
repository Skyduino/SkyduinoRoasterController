#ifndef __CMD_READ_H
#define __CMD_READ_H

#include "base.h"

class cmndRead : public Command {
    public:
        cmndRead();
    protected:
        void _doCommand( CmndParser *pars);
};

#endif // __CMD_READ_H