#ifndef __CMD_SKYWALKER_H
#define __CMD_SKYWALKER_H

#include "handler_pwm.h"

class cmndCool : public cmndPWM {
    public:
        cmndCool(uint8_t* dutyCycle);
};

class cmndDrum : public cmndPWM {
    public:
        cmndDrum(uint8_t* dutyCycle);
};

#endif // __CMD_SKYWALKER_H