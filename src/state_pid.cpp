#include <logging.h>

#include "state_pid.h"

#define _NVM_GETPIDPROF(x) (this->_nvm->settings.pidProfiles[ x ])
#define _NVM_PIDPROFCURRENT _NVM_GETPIDPROF( this->_nvm->settings.pidCurrentProfile )
#define _NVM_PIDPROFCONSERV _NVM_GETPIDPROF( this->_nvm->settings.pidConservProfile )
#define _NVM_PIDPROFFAN     _NVM_GETPIDPROF( this->_nvm->settings.pidFanProfile )


PID_Control::PID_Control(EepromSettings *nvm, ControlHeat *heat, ControlPWM *vent):
    _nvm(nvm), _heat(heat), _vent(vent)
{
    this->_pidFan.SetControllerDirection( QuickPID::Action::reverse );
};

/**
 * @brief Emergency PID shutdown. Turn off and clean up
 */
void PID_Control::abort() {
    this->turnOff();
    this->setp = 20;
    this->output = 0;
    this->_timer->detachInterrupt();
    this->_state = this->State::aborted;
}


/**
 * @brief Initializes the PID instance. Set up the timers etc
 * @returns true if initialization is a success
 */
bool PID_Control::begin() {
    if ( this->getState() != this->State::needsInit ) return true;

    // Configure the timer and attach interrupt
    TIM_TypeDef  *instance = TIM6;
    this->_timer = new HardwareTimer(instance);
    this->_timer->setOverflow(PID_CYCLE_TIME_MS*1000, MICROSEC_FORMAT);

    // Update PID settings
    this->turnOff();
    this->_pid.SetOutputLimits(1, 100);
    this->_pidFan.SetOutputLimits(0, 100);
    this->_syncPidSettings();

    this->_timer->attachInterrupt(
        std::bind(&PID_Control::_compute, this)
    );
    this->_timer->pause();

    this->_state = this->State::off;
    return true;
}


/**
 * @brief change and make current a new PID profile
 * @param profileNum -- index of the PID profile to activate
 * @param isConservative -- false (default) if this is a regular tuning
 *        profile. true -- if this is a conservative tunine profile
 * @return true -- if a correct profile was selected
 */
bool PID_Control::activateProfile(uint8_t profileNum, bool isConservative) {
    if ( profileNum >= PID_NUM_PROFILES ) {
        WARN(F("Profile ")); WARN(profileNum); WARNLN(F(" is not valid"));
        return false;
    }
    if ( isConservative ) {
        this->_nvm->settings.pidConservProfile = profileNum;
    } else {
        // check if we're using conservative profiles, if not, update both
        if ( _nvm->settings.pidConservProfile
             == _nvm->settings.pidCurrentProfile )
                this->_nvm->settings.pidConservProfile = profileNum;
        this->_nvm->settings.pidCurrentProfile = profileNum;
    }
    this->_nvm->markDirty();
    this->_syncPidSettings();

    return true;
}


/**
 * @brief Get current logical channel temperature
 * @return temperature C
 */
float PID_Control::getTempReadingC() {
    if ( NULL == getLogicalChanTempC ) {
        ERRORLN(F("No callback for getLogicalChanTempC"));
        return NAN;
    }
    return this->getLogicalChanTempC( _NVM_PIDPROFCURRENT.chan );
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
    if ( getState() == this->State::needsInit
         || getState() == this->State::autotune ) return;
    this->_pid.SetMode(QuickPID::Control::manual);
    this->_pidFan.SetMode(QuickPID::Control::manual);
    this->_timer->pause();
    this->_state = this->State::off;
}


/**
 * @brief Turn on the PID controller
 */
void PID_Control::turnOn() {
    if ( this->getState() == this->State::needsInit
         || this->getState() == this->State::autotune
         || this->getState() == this->State::aborted ) return;

    this->input = this->getTempReadingC();

    this->_pid.Initialize();
    this->_pid.SetMode(QuickPID::Control::timer);
    if ( FanMode::automatic == this->getFanMode() ) {
        this->_pidFan.Initialize();
        this->_fanMin = this->_vent->get();
        this->_pidFan.SetOutputLimits( this->_fanMin, 100 );
        this->_pidFan.SetMode( QuickPID::Control::timer );
    }
    this->_timer->resume();
    this->_state = this->State::on;
    this->_plotStart = millis();
}


/**
 * @brief print PIDs state
 */
void PID_Control::print() {
    Serial.print(F( "[PID] State: " ));
    switch ( getState() ) {
        case this->State::needsInit:
            Serial.print(F( "'Needs Initialization" ));
            break;

        case this->State::off:
            Serial.print(F( "'Off' " ));
            break;

        case this->State::on:
            Serial.print(F( "'On' " ));
            break;

        case this->State::autotune:
            Serial.print(F( "'Auto Tuning' " ));
            break;

        case this->State::aborted:
            Serial.print(F( "'Aborted' " ));
            break;
    };
    const char tmplt[] PROGMEM = "Setpoint=%f, P-term=%f, I-term=%f, D-term=%f, Error=%f";
    char buf[sizeof(tmplt) * 2];
    buf[sizeof(buf)-1] = 0;
    snprintf_P(buf, sizeof(buf)-1, tmplt,
        setp,
        _pid.GetPterm(),
        _pid.GetIterm(),
        _pid.GetDterm(),
        setp - getTempReadingC()
    );
    Serial.println( buf );

    Serial.print(F( "[FAN PID] State: " ));
    Serial.print( getFanMode() == FanMode::manual ? F("'Manual' ") : F("'Automatic' "));
    snprintf_P(buf, sizeof(buf)-1, tmplt,
        setp,
        _pidFan.GetPterm(),
        _pidFan.GetIterm(),
        _pidFan.GetDterm(),
        setp - getTempReadingC()
    );
    Serial.println( buf );
}


/**
 * @brief update the I-Anti-Windup mode
 * @param mode -- 0 - AwCondition, 1 - iAwClamp, 2 - iAwOff 
 */
bool PID_Control::updateAWMode(uint8_t mode)
{
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
 * @brief select FAN PID profile
 */
bool PID_Control::selectFanProfile(uint8_t profileNum) {
    if ( profileNum >= PID_NUM_PROFILES ) {
        WARN(F("Profile ")); WARN(profileNum); WARNLN(F(" is not valid"));
        return false;
    }

    this->_nvm->settings.pidFanProfile = profileNum;
    this->_nvm->markDirty();
    this->_syncPidSettings();

    return true;
}


/**
 * @brief Set minimum fan duty. Regardless of what FAN PID is calling
 *        the lower duty cycle is set by the OT2 command
 */
void PID_Control::setFanMin(uint8_t value) {
    this->_fanMin = value;
    this->_pidFan.SetOutputLimits( _fanMin, 100 );
}


/**
 * @brief Do the PID calculation here
 */
void PID_Control::_compute() {
    float tempC = this->getTempReadingC();

    DEBUG(F(" tempC: ")); DEBUGLN(tempC);

    if ( !isnan( tempC )) {
        this->input = tempC;
        this->_switchProfilesIfNeeded();
        // if error is under 60C, then limit output to 80%
        if ( abs( setp - input ) < 60 ) {
            _pid.SetOutputLimits( 1, 80 );
        } else {
            _pid.SetOutputLimits( 1, 100 );
        }
        if ( this->_pid.Compute() ) {
            DEBUG(millis()); DEBUG(F(" PID compute settings output to: "));
            DEBUGLN(this->output);
            this->_heat->set((uint8_t) this->output);
        }

        // fan pid calc
        if ( FanMode::automatic == this->getFanMode() ) {
            if ( this->_pidFan.Compute() ) {
                DEBUG(millis()); DEBUG(F(" FAN Pid output: "));
                DEBUGLN(this->exhaustOutp);
                this->_vent->set( (uint8_t) this->exhaustOutp );
            }
        }
    }

    if ( this->_plotPid ) _plotPidTuner();
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
    this->_isConservTuning = false;
    this->_pidFan.SetTunings(
        _NVM_PIDPROFFAN.kP,
        _NVM_PIDPROFFAN.kI,
        _NVM_PIDPROFFAN.kD,
        _NVM_PIDPROFFAN.pMode,
        _NVM_PIDPROFFAN.dMode,
        _NVM_PIDPROFFAN.iAwMode
    );
    uint32_t ctus = 1000 * profile->cycleTimeMS;
    this->_pid.SetSampleTimeUs(ctus);
    if ( this->_timer ) this->_timer->setOverflow(ctus, MICROSEC_FORMAT);
}


/**
 * @brief Switch to conservative profile/tuning if needed
 */
void PID_Control::_switchProfilesIfNeeded() {
    float gap = abs( this->setp - this->input );
    if ( gap < PID_CONSERV_ERR ) {
        // Use Conserv tuning profile
        if ( !(this->_isConservTuning) ) {
            _pid.SetTunings(
                _NVM_PIDPROFCONSERV.kP,
                _NVM_PIDPROFCONSERV.kI,
                _NVM_PIDPROFCONSERV.kD,
                _NVM_PIDPROFCONSERV.pMode,
                _NVM_PIDPROFCONSERV.dMode,
                _NVM_PIDPROFCONSERV.iAwMode
            );
            this->_isConservTuning = true;
            DEBUG(millis()); DEBUGLN(F(" Using conservative tuning"));
        }
    } else {
        // Use regular tuning profile
        if ( this->_isConservTuning ) {
            _pid.SetTunings(
                _NVM_PIDPROFCURRENT.kP,
                _NVM_PIDPROFCURRENT.kI,
                _NVM_PIDPROFCURRENT.kD,
                _NVM_PIDPROFCURRENT.pMode,
                _NVM_PIDPROFCURRENT.dMode,
                _NVM_PIDPROFCURRENT.iAwMode
            );
            this->_isConservTuning = false;
            DEBUG(millis()); DEBUGLN(F(" Using regular tuning"));
        }
    }
}


/**
 * @brief Plot PID data for tunning
 */
void PID_Control::_plotPidTuner() {
    uint32_t now = millis() - this->_plotStart;

    const char tmplt[] PROGMEM = "# PID us: %f, setpoint: %f, input: %f, heat: %f, fan: %f";
    char buf[sizeof(tmplt) * 2];
    buf[sizeof(buf)-1] = 0;
    snprintf_P(buf, sizeof(buf)-1, tmplt,
        now / 1000.0f,
        setp,
        input,
        output,
        exhaustOutp
    );
    Serial.println( buf );
}