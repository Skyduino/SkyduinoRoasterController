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
 * @return true -- if a correct profile was selected
 */
 bool PID_Control::activateProfile(uint8_t profileNum) {
    if ( profileNum >= PID_NUM_PROFILES ) {
        WARN(F("Profile ")); WARN(profileNum); WARNLN(F(" is not valid"));
        return false;
    }
    this->_nvm->settings.pidCurrentProfile = profileNum;
    this->_nvm->markDirty();
    this->_syncPidSettings();

    return true;
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
 * @brief update the I-Anti-Windup mode
 * @param mode -- 0 - AwCondition, 1 - iAwClamp, 2 - iAwOff 
 */
bool PID_Control::updateAWMode(uint8_t mode) {
    if ( mode > (uint8_t) QuickPID::iAwMode::iAwOff ) {
        DEBUGLN(F("Wrong I anti-windup mode"));
        return false;
    }
    DEBUG(micros()); DEBUG(F(" Setting I anti-windup: ")); DEBUGLN( mode );
    _NVM_PIDPROFCURRENT.iAwMode = (QuickPID::iAwMode) mode;
    this->_nvm->markDirty();
    this->_syncPidSettings();

    return true;
}

/**
 * @brief use a different channel for PID's input
 */
bool PID_Control::updateChan(uint8_t chan) {
    if ( chan > TEMPERATURE_CHANNELS_MAX || 0 == chan) {
        WARNLN(F("Wrong channel"));
        return false;
    }
    _NVM_PIDPROFCURRENT.chan = chan;
    this->_nvm->markDirty();

    return true;
}


/**
 * @brief Update loop cycle time: update PID & Timer
 */
bool PID_Control::updateCycleTimeMs(uint32_t ctMS) {
    if ( ctMS < 100 ) return false;

    _NVM_PIDPROFCURRENT.cycleTimeMS = ctMS;
    this->_nvm->markDirty();
    this->_syncPidSettings();

    return true;
}


/**
 * @brief update the D-Mode
 * @param mode -- 0 - D on error, 1 - D on measurement
 */
bool PID_Control::updateDMode(uint8_t mode) {
    if ( mode > (uint8_t) QuickPID::dMode::dOnMeas ) {
        DEBUGLN(F("Wrong D-mode"));
        return false;
    }
    _NVM_PIDPROFCURRENT.dMode = (QuickPID::dMode) mode;
    this->_nvm->markDirty();
    this->_syncPidSettings();

    return true;
}


/**
 * @brief update the P-Mode
 * @param mode -- 0 - D on error, 1 - P on measurement, 2 - P on both error
 *             and measurement
 */
bool PID_Control::updatePMode(uint8_t mode) {
    if ( mode > (uint8_t) QuickPID::pMode::pOnErrorMeas ) {
        DEBUGLN(F("Wrong P-mode"));
        return false;
    }
    _NVM_PIDPROFCURRENT.pMode = (QuickPID::pMode) mode;
    this->_nvm->markDirty();
    this->_syncPidSettings();

    return true;
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
        profile->pMode,
        profile->dMode,
        profile->iAwMode
    );

    uint32_t ctus = 1000 * profile->cycleTimeMS;
    this->_pid.SetSampleTimeUs(ctus);
    if ( this->_timer ) this->_timer->setOverflow(ctus, MICROSEC_FORMAT);

}
