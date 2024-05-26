#ifndef __CMD_OT_H
#define __CMD_OT_H

#include "handler_pwm.h"

class cmndOT1 : public cmndPWM {
    public:
        cmndOT1(uint8_t* otValue);
};

class cmndOT2 : public cmndPWM {
    public:
        cmndOT2(uint8_t* otValue);
};

class cmndLED : public cmndPWM {
    public:
        cmndLED();
    private:
        uint8_t pwmValue;
};

#endif // __CMD_OT_H