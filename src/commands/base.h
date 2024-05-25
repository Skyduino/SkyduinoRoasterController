#ifndef __CMD_BASE_H
#define __CMD_BASE_H

#include <cmndproc.h>

class Command : public CmndBase {
    public:
        Command(const char* cmdName);
        //virtual bool doCommand( CmndParser* pars);
        virtual bool begin(void);
};

#endif // __CMD_BASE_H