#include "roaster.h"
#include "handler_ot.h"

#define CMD_OT1 "OT1"
#define CMD_OT2 "OT2"
#define CMD_LED "LED"


cmndOT1::cmndOT1(uint8_t* dutyCycle):
    cmndPWM(CMD_OT1, dutyCycle, PIN_HEAT, PWM_FREQ_HEAT) {
}


cmndOT2::cmndOT2(uint8_t* dutyCycle):
    cmndPWM(CMD_OT2, dutyCycle, PIN_EXHAUST, PWM_FREQ_EXHAUST) {
}


cmndLED::cmndLED():
    cmndPWM(CMD_LED, &pwmValue, PIN_LED, PWM_FREQ_LED) {
}