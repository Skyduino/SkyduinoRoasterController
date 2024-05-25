#ifndef __CMD_READ_H
#define __CMD_READ_H

#include <cmndproc.h>

class cmndRead : public CmndBase {
    public:
        cmndRead();
        virtual bool doCommand( CmndParser* pars);
    private:
        void handleRead(void);
};

#endif // __CMD_READ_H