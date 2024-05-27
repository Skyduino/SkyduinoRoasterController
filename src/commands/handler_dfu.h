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


class cmndDFU : public Command {
    public:
        cmndDFU();
        virtual bool doCommand( CmndParser* pars);

    private:
        TimerMS timer = TimerMS(CMD_DFU_TIMEOUT_MS);
        int challenge = 0;

        void enterDFU();
        void processChallenge(int response);
};

#endif // __CMD_DFU_H