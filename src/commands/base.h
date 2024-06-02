#ifndef __CMD_BASE_H
#define __CMD_BASE_H

#include <cmndproc.h>

class Command : public CmndBase {
    public:
        Command(const char* cmdName);
        bool doCommand( CmndParser* pars);
        virtual bool begin(void);
    protected:
        virtual void _doCommand( CmndParser *pars) =0;
};

#endif // __CMD_BASE_H