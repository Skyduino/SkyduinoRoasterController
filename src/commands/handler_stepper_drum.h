#ifndef __CMD_STEPPER_H
#define __CMD_STEPPER_H

#include "base.h"

class cmndSteps : public ControlCommand {
    public:
        cmndSteps(State *state);

    protected:
        void _handleValue(int32_t value);
};

class cmndMaxRPM : public ControlCommand {
    public:
        cmndMaxRPM(State *state);

    protected:
        void _handleValue(int32_t value);
};

#endif // __CMD_STEPPER_H