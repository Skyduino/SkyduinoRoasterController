#ifndef __CMD_DFU_H
#define __CMD_DFU_H

#include <tick-timer.h>

#include "base.h"

#ifndef BOOTLOADER_ADDR
#define BOOTLOADER_ADDR 0x1FFF0000UL
#endif  //BOOTLOADER_ADDR
#ifndef CMD_DFU_TIMEOUT_MS
#define CMD_DFU_TIMEOUT_MS 5000UL
#endif // CMD_DFU_TIMEOUT_MS


class cmndChallenge : public Command {
    public:
        cmndChallenge(const char *cmdName): Command(cmdName) {};
        cmndChallenge(const char *cmdName, State *state):
            Command(cmdName, state) {};
    protected:
        void _doCommand( CmndParser* pars);

    private:
        TimerMS timer = TimerMS(CMD_DFU_TIMEOUT_MS);
        int challenge = 0;

        void processChallenge(int response);

    protected:
        virtual void executeCommand() =0;
};


class cmndDFU : public cmndChallenge {
    public:
        cmndDFU();
    
    protected:
        void executeCommand();
};


class cmndReset : public cmndChallenge {
    public:
        cmndReset();
    
    protected:
        void executeCommand();
};


class cmndDflts: public cmndChallenge {
    public:
        cmndDflts(State *state);
    
    protected:
        void executeCommand();
};


#endif // __CMD_DFU_H