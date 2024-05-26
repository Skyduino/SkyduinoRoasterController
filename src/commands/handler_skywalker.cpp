#include "roaster.h"
#include "handler_skywalker.h"

#define CMD_COOL    "COOL"
#define CMD_DRUM    "DRUM"


cmndCool::cmndCool(uint8_t* dutyCycle):
    cmndPWM(CMD_COOL, dutyCycle, PIN_COOL, PWM_FREQ_COOL) {
}


cmndDrum::cmndDrum(uint8_t* dutyCycle):
    cmndPWM(CMD_DRUM, dutyCycle, PIN_DRUM, PWM_FREQ_DRUM) {
}