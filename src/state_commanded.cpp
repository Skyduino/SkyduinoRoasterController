#include "state.h"
#include "state_commanded.h"


ControlBasic::ControlBasic() {
    value = 0;
}


bool ControlBasic::loopTick() {
    return true;
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
    bool isSuccess = true;
    isSuccess &= this->heat.loopTick();
    isSuccess &= this->vent.loopTick();
    isSuccess &= this->drum.loopTick();
    isSuccess &= this->cool.loopTick();
    isSuccess &= this->filter.loopTick();

    return isSuccess;
}


/**
 * Output to serial heat & air values.
 * part of output in response to READ command
*/
void StateCommanded::printState() {
    Serial.print(F(","));
    Serial.print(heat.get());
    Serial.print(',');
    Serial.print(vent.get());
    Serial.print(',');
    Serial.println(F("0"));
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


bool ControlHeat::begin()
{
    transitionTimer = new TimerMS(CONTROL_HEAT_SSR_RELAY_DELAY_MS);
    bool isSuccess = true;
    isSuccess &= this->heatRelay.begin();
    isSuccess &= ControlPWM::begin();

    return isSuccess;
}


bool ControlHeat::loopTick() {
    if ( isTransitioning 
         and NULL != this->transitionTimer
         and this->transitionTimer->hasTicked())
    {
        if ( get() > 0 ) {
            // transitioning to ON, set the PWM
            ControlPWM::_setAction(this->oldValue);
        } else {
            // transitioning to OFF, turn off the relay
            this->heatRelay.off();
        }
        isTransitioning = false;
    }

    return this->heatRelay.loopTick();
}


void ControlHeat::_setAction(uint8_t newValue) {
    // is the state transitioning from Off->On or On->Off
    bool oldIsOn = (this->oldValue > 0);
    bool newIsOn = (newValue > 0);

    this->isTransitioning = oldIsOn ^ newIsOn;
    this->oldValue = newValue;

    if ( isTransitioning ) {
        if ( NULL == transitionTimer ) {
            this->begin();
        }
        this->transitionTimer->reset();

        // If transitioning from On to Off, then set PWM to 0 and next loopTick
        // will turn off the Heat realy on timer expiration
        // If transitioning from Off to On, then make sure the SSR is off(pwm=0)
        // and turn on the relay now. Next loopTick will turn on the SSR (set the
        // desired PWM)
        ControlPWM::_setAction(0);
        if ( newIsOn ) {
            // The state is transitioning to ON
            // turn on the Relay and loopTick will set the new PWM value
            this->heatRelay.on();
        }
    } else {
        ControlPWM::_setAction(newValue);
    }
}
