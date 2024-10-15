#include <logging.h>

#include "state_pid.h"


/**
 * @brief Initializes the PID instance. Set up the timers etc
 * @returns true if initialization is a success
 */
bool PID_Control::begin() {
    if ( this->isInitialized ) return true;

    // Configure the timer and attach interrupt
    TIM_TypeDef  *instance = TIM6;
    this->_timer = new HardwareTimer(instance);
    this->_timer->setOverflow(PID_CYCLE_TIME_MS*1000, MICROSEC_FORMAT);

    // Update PID settings
    this->turnOff();
    this->loadProfile( this->_nvm->settings.pidCurrentProfile );
    this->_pid.SetOutputLimits(0, 100);

    this->_timer->attachInterrupt(
        std::bind(&PID_Control::_compute, this)
    );
    this->_timer->pause();

    this->isInitialized = true;
    return true;
}


/**
 * @brief load a profile from NVM settings
 * @param profileNum -- index of the loaded profile
 */
void PID_Control::loadProfile(uint8_t profileNum) {
    if ( profileNum >= PID_NUM_PROFILES ) {
        WARN(F("Profile ")); WARN(profileNum); WARNLN(F(" is not valid"));
        return;
    }
    const t_NvmPIDSettings *profile =
        &(this->_nvm->settings.pidProfiles[profileNum]);
    this->_pid.SetTunings(
        profile->kP,
        profile->kI,
        profile->kD,
        profile->pmode,
        profile->dmode,
        profile->iAwMode
    );
    this->updateCycleTimeMs( profile->cycleTimeMS );
}


/**
 * @brief query current PID controller mode
 * @return true if the PID controller is actively controlling the output
 */
bool PID_Control::isOn() {
    return
        this->_pid.GetMode() == (uint8_t) QuickPID::Control::automatic
        || this->_pid.GetMode() == (uint8_t) QuickPID::Control::timer;
}


/**
 * @brief Turn off the PID controller
 */
void PID_Control::turnOff() {
    this->_pid.SetMode(QuickPID::Control::manual);
    this->_timer->pause();
}


/**
 * @brief Turn on the PID controller
 */
void PID_Control::turnOn() {
    this->_pid.Initialize();
    this->_pid.SetMode(QuickPID::Control::timer);
    this->_timer->resume();
}


/**
 * @brief Update loop cycle time: update PID & Timer
 */
void PID_Control::updateCycleTimeMs(uint32_t ctMS) {
    uint32_t ctus = ctMS * 1000;
    this->_pid.SetSampleTimeUs(ctus);
    if ( this->_timer ) this->_timer->setOverflow(ctus, MICROSEC_FORMAT);
}


/**
 * @brief Update the PID setpoint. The new Setpoint is always in the current
 *        units of measrement, but internally PID uses the Setpoint in C
 * @paragraph setPoint -- new setpoint in the current units of measurement
 */
void PID_Control::updateSetPointC(float setPointC) {
    this->setp = setPointC;
}


/**
 * @brief update PID tuning parameters
 */
void PID_Control::updateTuning(float kP, float kI, float kD) {
    this->_pid.SetTunings(kP, kI, kD);
}


/**
 * @brief Do the PID calculation here
 */
void PID_Control::_compute() {
    if ( NULL == getLogicalChanTempC ) {
        ERRORLN(F("No callback for getLogicalChanTempC"));
        return;
    }
    uint8_t profileNum = this->_nvm->settings.pidCurrentProfile;
    uint8_t chan = this->_nvm->settings.pidProfiles[ profileNum ].chan;
    float tempC = this->getLogicalChanTempC( chan );

    DEBUG(millis()); DEBUG(F(" PID Profile chan: ")); DEBUG(chan);
    DEBUG(F(" tempC: ")); DEBUGLN(tempC);

    if ( !isnan( tempC )) {
        this->input = tempC;
        if ( this->_pid.Compute() ) {
            DEBUG(millis()); DEBUG(F(" PID compute settings output to: "));
            DEBUGLN(this->output);
            this->_heat->set((uint8_t) this->output);
        }
    }
}