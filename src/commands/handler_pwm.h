#ifndef __CMD_PWM_H
#define __CMD_PWM_H

#include "base.h"

class cmndPWM : public Command {
    public:
        cmndPWM(const char* cmdName, uint8_t* otValue, uint8_t pin, uint32_t freq);
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

#endif // __CMD_PWM_H