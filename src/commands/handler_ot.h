#ifndef __CMD_OT_H
#define __CMD_OT_H

#include "base.h"

class cmndOT : public Command {
    public:
        cmndOT(const char* cmdName, uint8_t* otValue, uint8_t pin, uint32_t freq);
        virtual bool doCommand( CmndParser* pars);
        bool begin();
        void virtual off();
    private:
        uint8_t*        otValue;
        uint8_t         pin;
        HardwareTimer*  timer;
        uint32_t        channel;
        uint32_t        freq;
        void virtual setupPin();
        void virtual setPWM(uint32_t duty);
};

class cmndOT1 : public cmndOT {
    public:
        cmndOT1(uint8_t* otValue);
};

#endif // __CMD_OT_H