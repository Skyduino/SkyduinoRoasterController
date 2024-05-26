#include <stdlib.h>

#include "roaster.h"
#include "logging.h"
#include "handler_ot.h"
#include "state.h"

#define CMD_OT1 "OT1"
#define CMD_OT2 "OT2"


// ----------------------------
cmndOT::cmndOT(const char* cmdName, uint8_t* otValue, uint8_t pin, uint32_t freq):
    Command ( cmdName ), pin( pin )
{
    this->otValue   = otValue;
    this->freq      = freq;
    this->timer     = NULL;
}

bool cmndOT::doCommand(CmndParser *pars)
{
    if( strcmp( keyword, pars->cmndName() ) == 0 ) {
        int32_t newValue = strtol(pars->paramStr(1), NULL, 10);
        if (newValue < 0 || newValue > 255) {
            WARN(F("Value '"));
            WARN(newValue);
            WARNLN(F("' is out of uint8_t range, ignoring it"));
            return true;
        }

        char buf[128];
        uint8_t len;

        len = sprintf(buf, "Executing %s command: old value: %u, new value: %d",
                      pars->cmndName(),
                      *this->otValue,
                      (int) newValue
                     );
        buf[len+1] = '\0';
        Serial.println(buf);

        *this->otValue = (uint8_t) newValue & 0xff;
        this->setPWM(newValue);
        return true;
    }

   return false;
}

void cmndOT::setupPin() {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
}


void cmndOT::off() {
    this->setPWM(0);
}


void cmndOT::setPWM(uint32_t duty) {
    if (NULL == timer) {
        // was not initialized yet
        this->setupPin();
    } else {
        // set pwm to 0
        timer->setPWM(this->channel, this->pin, this->freq, duty);
    }
}


bool cmndOT::begin() {
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


cmndOT1::cmndOT1(uint8_t* otValue) : cmndOT(CMD_OT1, otValue, PIN_LED, 1) {
}