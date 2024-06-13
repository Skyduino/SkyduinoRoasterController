#ifndef __CMD_SKYWALKER_H
#define __CMD_SKYWALKER_H

#include "base.h"

class cmndCool : public ControlCommand {
    public:
        cmndCool(State *state);

    protected:
        void _handleValue(int32_t value);
};

class cmndDrum : public ControlCommand {
    public:
        cmndDrum(State *state);

    protected:
        void _handleValue(int32_t value);
};

#endif // __CMD_SKYWALKER_H