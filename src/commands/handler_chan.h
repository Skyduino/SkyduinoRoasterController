#ifndef __CMD_CHAN_H
#define __CMD_CHAN_H

#include <cmndproc.h>

class cmndChan : public CmndBase {
    public:
        cmndChan();
        virtual bool doCommand( CmndParser* pars);
};

#endif // __CMD_CHAN_H