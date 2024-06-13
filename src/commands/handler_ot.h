#ifndef __CMD_OT_H
#define __CMD_OT_H

#include "base.h"

class cmndOT1 : public ControlCommand {
    public:
        cmndOT1(State *state);

    protected:
        void handleValue(int32_t value);
};

class cmndOT2 : public ControlCommand {
    public:
        cmndOT2(State *state);

    protected:
        void _handleValue(int32_t value);
};

#endif // __CMD_OT_H