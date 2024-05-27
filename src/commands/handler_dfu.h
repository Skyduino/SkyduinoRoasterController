#ifndef __CMD_DFU_H
#define __CMD_DFU_H

#include "base.h"

#ifndef BOOTLOADER_ADDR
#define BOOTLOADER_ADDR 0x1FFF0000UL
#endif  //BOOTLOADER_ADDR


class cmndDFU : public Command {
    public:
        cmndDFU();
        virtual bool doCommand( CmndParser* pars);

    private:
        void enterDFU();
};

#endif // __CMD_DFU_H