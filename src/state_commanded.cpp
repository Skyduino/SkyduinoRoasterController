#include <logging.h>

#include "state.h"
#include "state_commanded.h"


ControlBasic::ControlBasic() {
    value = 0;
}


bool ControlBasic::loopTick() {
    return true;
}


/**
 * @brief Emergency shutdown of all controls
 */
void StateCommanded::abort() {
    ControlBasic* controls[] = {
        &cool,
        &drum,
        &filter,
        &heat,
        &vent
    };

    uint8_t count = sizeof(controls) / sizeof(controls[0]);
    for (uint8_t i=0; i < count; i++) {
        controls[i]->abort();
    }
}


bool StateCommanded::begin()
{
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


/**
 * @brief Turn everything off gracefully
 */
void StateCommanded::off() {
    ControlBasic* controls[] = {
        &cool,
        &drum,
        &filter,
        &heat,
        &vent
    };

    uint8_t count = sizeof(controls) / sizeof(controls[0]);
    for (uint8_t i=0; i < count; i++) {
        controls[i]->off();
    }
}


/**
 * @brief check whether the roaster is controlled by the TC4 protocol or the
 *        remote. By default, the roaster is controlled by the remote, however,
 *        if any OT1, OT2, commands are recieved, then the control is
 *        relinguished to TC4 protocol. Off commands resets to the deault
 *        state.
 * 
 * @return true -- if the roaster is conrolled by the TC4 protocol
 */
bool StateCommanded::isArtisanIncontrol() {
    return this->_isArtisanInControl;
}


/**
 * @brief set control to the TC4 protocol
 * 
 * @param value -- true, if the roaster is to be controlled by the TC4 protocol
 */
void StateCommanded::setControlToArtisan(bool value) {
    this->_isArtisanInControl = value;
}


/**
 * @brief Abort -- Emergency shutdown
 *
 * Turn everything off and lock out the controls
 */
void ControlBasic::abort() {
    _abortAction();
    off();
    this->_isAborted = true;
}


/**
 * @brief action to perform prior the public abort() method
 * 
 * This is a handler for the "pre-abort" actions, called from the public
 * abort method, giving a chance to do any extra changes, before the conrol
 * switches off and is locked out from other changes. 
 */
void ControlBasic::_abortAction() {
}

void ControlBasic::on() {
    this->set(100);
}

bool ControlBasic::isOn() {
    return (this->value > 0);
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
    if ( _isAborted ) return;

    if ( value ) {
        digitalWrite(pin, HIGH);
    } else {
        digitalWrite(pin, LOW);
    }
}


ControlPWM::ControlPWM(uint32_t pin, uint32_t freq): ControlOnOff(pin) {
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
    if ( _isAborted ) return;

    if (NULL == timer) {
        // was not initialized yet
        this->begin();
    } else {
        // set pwm to 0
        this->_setPWM( value );
        DEBUG(micros()); DEBUG(F(" PWM::_setAction value: ")); DEBUGLN(value);
    }
}


void ControlPWM::_setPWM(uint8_t value) {
    this->timer->setPWM(this->channel, this->pin, this->freq, value);
    timer->refresh();
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
        if ( isOn() ) {
            // transitioning to ON, set the PWM
            DEBUG(micros()); DEBUG(F(" loopTick: Setting SSR to ")); DEBUGLN(this->oldValue);
            ControlPWM::_setAction(this->oldValue);
        } else {
            // transitioning to OFF, turn off the relay
            DEBUG(micros()); DEBUGLN(F(" loopTick: Turning off Heat Relay"));
            this->heatRelay.off();
        }
        isTransitioning = false;
    }

    return this->heatRelay.loopTick();
}


/**
 * @brief Emergency heater shutdown
 *
 * Emergency heater turn off. If the heater is On, turn it off, wait 11ms
 * for zero crossing and then turn off the heating relay.
 */
void ControlHeat::_abortAction() {
    bool wasItOn = isOn();
    this->off();

    if ( wasItOn ) {
        // if it was on, then wait for a zero crossing in worst case scenario
        // 50Hz + 1MS
        delay(11);
    }
    DEBUG(micros()); DEBUGLN(F(" _abort: Turning off Heat relay"));
    this->heatRelay.off();

    // double tap: explicitly turn relay off via GPIO
    pinMode(PIN_HEAT_RELAY, OUTPUT);
    digitalWrite(PIN_HEAT_RELAY, LOW);
}


void ControlHeat::_setAction(uint8_t newValue) {
    if ( _isAborted ) return;

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
        DEBUG(micros()); DEBUGLN(F(" _setAction: Turning off SSR"));
        ControlPWM::_setAction(0);
        if ( newIsOn ) {
            // The state is transitioning to ON
            // turn on the Relay and loopTick will set the new PWM value
            DEBUG(micros()); DEBUGLN(F(" _setAction: Turning on Heat Relay"));
            this->heatRelay.on();
        }
    } else {
        DEBUG(micros()); DEBUG(F(" _setAction: Setting SSR to ")); DEBUGLN(newValue);
        ControlPWM::_setAction(newValue);
    }
}


#ifdef USE_STEPPER_DRUM
bool ControlDrum::begin() {
    bool isSuccess = true;
    
    isSuccess &= this->_enable.begin();
    isSuccess &= this->_drum.begin();
    isSuccess &= ControlPWM::begin();

    return isSuccess;
}


/**
 * @brief convert value (%) into timer overflow duration, based on the steps
 *        per revolution and Max supported rpm. 100% value is Max rpm
 * @param value Value % of max RPM
 * @return timer overflow duration in us
*/ 
uint32_t ControlDrum::durationFromValue(uint8_t value) {
    uint16_t rpm = ( value * this->_max_rpm ) / 100;
    return 60 * 1000 * 1000 / ( rpm * this->_steps_per_rev );
}


/**
 * @brief convert value (%) into PWM frequence, based on the steps
 *        per revolution and Max supported rpm. 100% value is Max rpm
 * @param value Value % of max RPM
 * @return timer PWM frequency in HZ
*/ 
uint32_t ControlDrum::frequencyFromValue(uint8_t value) {
    uint32_t freq = ( value * this->_max_rpm * this->_steps_per_rev ) \
                    / 6000;
    return freq;
}


/**
 * @brief sets steps per revolution for the stepper drum driver
 * @param steps number of steps for a complete revolution
 */
void ControlDrum::setStepsPerRevolution(uint16_t steps) {
    this->_steps_per_rev = steps;
    this->set( this->get() );
}


/**
 * @brief sets max RPM for the stepper driver
 * @param rpm -- max rpm
 */
void ControlDrum::setMaxRPM(uint8_t rpm) {
    this->_max_rpm = rpm;
    this->set( this->get() );
}


void ControlDrum::_setAction(uint8_t value) {
    if ( _isAborted ) return;
    this->_drum.set(value);

    // Turn the stepper enable pin on or off. DRV8825 is nEN
    if ( 0 == value ) {
        this->_enable.on();
    } else {
        this->_enable.off();
    }
    ControlPWM::_setAction( value );
}


void ControlDrum::_setPWM(uint8_t value) {
    DEBUG(micros()); DEBUG(F(" Stepper Drum value: ")); DEBUGLN(value);

    uint32_t freq = frequencyFromValue(value);
    if ( 0 == freq ) {
        this->timer->pause();
    } else {
        this->timer->setPWM(channel, pin, freq, 50);
    }
}


void ControlDrum::_abortAction()
{
    this->_drum.abort();
    this->_enable.abort();
}
#endif // USE_STEPPER_DRUM