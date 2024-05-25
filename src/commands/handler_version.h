#ifndef __CMD_VERSION_H
#define __CMD_VERSION_H

#include <cmndproc.h>

class cmndVersion : public CmndBase {
    public:
        cmndVersion();
        virtual bool doCommand( CmndParser* pars);
};

#endif // __CMD_VERSION_H