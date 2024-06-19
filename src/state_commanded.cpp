#include "state.h"
#include "state_commanded.h"


ControlBasic::ControlBasic() {
    value = 0;
}


bool StateCommanded::begin() {
    bool isSuccess = true;

    isSuccess &= this->heat.begin();
    isSuccess &= this->vent.begin();
    isSuccess &= this->drum.begin();
    isSuccess &= this->cool.begin();
    isSuccess &= this->filter.begin();
    return isSuccess;
}


bool StateCommanded::loopTick() {
    return true;
}


void ControlBasic::on() {
    this->set(100);
}


void ControlBasic::off() {
    this->set(0);
}


uint8_t ControlBasic::get() {
    return this->value;
}


void ControlBasic::set(uint8_t value) {
    this->value = value;
    this->_setAction(value);
}


bool ControlOnOff::begin() {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);

    return true;
}


void ControlOnOff::_setAction(uint8_t value) {
    if ( value ) {
        digitalWrite(pin, HIGH);
    } else {
        digitalWrite(pin, LOW);
    }
}


ControlPWM::ControlPWM(uint8_t pin, uint32_t freq): ControlOnOff(pin) {
    this->freq = freq;
    this->timer = NULL;
}


bool ControlPWM::begin() {
    ControlOnOff::begin();
    
    if ( NULL == this->timer ) {
        TIM_TypeDef *Instance = (TIM_TypeDef *)pinmap_peripheral(
            digitalPinToPinName(pin),
            PinMap_PWM
        );
        channel = STM_PIN_CHANNEL(
            pinmap_function(digitalPinToPinName(pin),
            PinMap_PWM)
        );

        timer = new HardwareTimer(Instance);
        this->set(0);
    }

    return true;
}


void ControlPWM::_setAction(uint8_t value) {
    if (NULL == timer) {
        // was not initialized yet
        this->begin();
    } else {
        // set pwm to 0
        timer->setPWM(this->channel, this->pin, this->freq, value);
    }
}
