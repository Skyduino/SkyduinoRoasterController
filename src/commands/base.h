#ifndef __CMD_BASE_H
#define __CMD_BASE_H

#include <cmndproc.h>

#include "state.h"

class Command : public CmndBase {
    public:
        Command(const char *cmdName): CmndBase(cmdName) {};
        Command(const char *cmdName, State *state);
        bool doCommand( CmndParser* pars);
        virtual bool begin(void);

    protected:
        State *state;
        virtual void _doCommand( CmndParser *pars) =0;
};


class ControlCommand: public Command {
    public:
        ControlCommand(const char *cmdName, State *state);
        ControlCommand(const char *cmdName, State *state, uint32_t clamp_min, int32_t clamp_max);
    protected:
        void _doCommand( CmndParser* pars);
        void virtual _handleValue(int32_t value) =0;

    private:
        int32_t min;
        int32_t max;
};

#endif // __CMD_BASE_H