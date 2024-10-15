#include <logging.h>

#include "state_pid.h"

#define _NVM_GETPIDPROF(x) (this->_nvm->settings.pidProfiles[ x ])
#define _NVM_PIDPROFCURRENT _NVM_GETPIDPROF( this->_nvm->settings.pidCurrentProfile )

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
    this->_pid.SetOutputLimits(0, 100);
    this->_syncPidSettings();

    this->_timer->attachInterrupt(
        std::bind(&PID_Control::_compute, this)
    );
    this->_timer->pause();

    this->isInitialized = true;
    return true;
}


/**
 * @brief change and make current a new PID profile
 * @param profileNum -- index of the PID profile to activate
 */
void PID_Control::activateProfile(uint8_t profileNum) {
    if ( profileNum >= PID_NUM_PROFILES ) {
        WARN(F("Profile ")); WARN(profileNum); WARNLN(F(" is not valid"));
        return;
    }
    this->_nvm->settings.pidCurrentProfile = profileNum;
    this->_nvm->markDirty();
    this->_syncPidSettings();
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
 * @brief use a different channel for PID's input
 */
void PID_Control::updateChan(uint8_t chan) {
    _NVM_PIDPROFCURRENT.chan = chan;
    this->_nvm->markDirty();
}


/**
 * @brief Update loop cycle time: update PID & Timer
 */
void PID_Control::updateCycleTimeMs(uint32_t ctMS) {
    _NVM_PIDPROFCURRENT.cycleTimeMS = ctMS;
    this->_nvm->markDirty();
    this->_syncPidSettings();
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
    _NVM_PIDPROFCURRENT.kP = kP;
    _NVM_PIDPROFCURRENT.kI = kI;
    _NVM_PIDPROFCURRENT.kD = kD;
    this->_nvm->markDirty();
    this->_syncPidSettings();
}


/**
 * @brief Do the PID calculation here
 */
void PID_Control::_compute() {
    if ( NULL == getLogicalChanTempC ) {
        ERRORLN(F("No callback for getLogicalChanTempC"));
        return;
    }
    float tempC = this->getLogicalChanTempC( _NVM_PIDPROFCURRENT.chan );

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


/**
 * @brief Set PID settings to match the current NVM PID profile
 */
void PID_Control::_syncPidSettings() {
    const t_NvmPIDSettings *profile = &_NVM_PIDPROFCURRENT;
    this->_pid.SetTunings(
        profile->kP,
        profile->kI,
        profile->kD,
        profile->pmode,
        profile->dmode,
        profile->iAwMode
    );

    uint32_t ctus = 1000 * profile->cycleTimeMS;
    this->_pid.SetSampleTimeUs(ctus);
    if ( this->_timer ) this->_timer->setOverflow(ctus, MICROSEC_FORMAT);

}
