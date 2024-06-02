#include <stdlib.h>

#include "roaster.h"
#include "logging.h"
#include "handler_ot.h"
#include "state.h"


// ----------------------------
cmndPWM::cmndPWM(const char* cmdName, uint8_t* otValue, uint8_t pin, uint32_t freq):
    Command ( cmdName ), pin( pin )
{
    this->otValue   = otValue;
    this->freq      = freq;
    this->timer     = NULL;
}

void cmndPWM::_doCommand(CmndParser *pars)
{
    int32_t newValue = strtol(pars->paramStr(1), NULL, 10);
    if (newValue < 0 || newValue > 100) {
        WARN(F("Value '"));
        WARN(newValue);
        WARNLN(F("' is out of uint8_t range, ignoring it"));
        return;
    }
    
    *this->otValue = (uint8_t) newValue & 0xff;
    this->setPWM(newValue);
}

void cmndPWM::setupPin() {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
}


void cmndPWM::off() {
    this->setPWM(0);
}


void cmndPWM::setPWM(uint32_t duty) {
    if (NULL == timer) {
        // was not initialized yet
        this->setupPin();
    } else {
        // set pwm to 0
        timer->setPWM(this->channel, this->pin, this->freq, duty);
    }
}


bool cmndPWM::begin() {
    this->setupPin();

    TIM_TypeDef *Instance = (TIM_TypeDef *)pinmap_peripheral(
        digitalPinToPinName(pin),
        PinMap_PWM
    );
    channel = STM_PIN_CHANNEL(
        pinmap_function(digitalPinToPinName(pin),
        PinMap_PWM)
    );

    timer = new HardwareTimer(Instance);
    this->setPWM(0);

    return true;
}