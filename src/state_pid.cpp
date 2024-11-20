#include <logging.h>

#include "state_pid.h"

#define _NVM_PID            this->_nvm->settings.pid
#define _NVM_PIDPROFNORMAL  this->_nvm->settings.pid.tuneNormal
#define _NVM_PIDPROFCONSERV this->_nvm->settings.pid.tuneConserv
#define _NVM_PIDPROFFAN     this->_nvm->settings.pid.tuneFan


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
    this->_isFanPidActive = false;
    this->_syncPidSettings();

    this->_timer->attachInterrupt(
        std::bind(&PID_Control::_compute, this)
    );
    this->_timer->pause();

    this->_state = this->State::off;
    return true;
}


/**
 * @brief set the setpoint error threshold gap for switching to the
 *        conservative profile
 * @param setpointGapC (float) -- threshold in C for the setpoint gap
 */
bool PID_Control::setConservProfileGapC(float setpointGapC) {
    _NVM_PID.cnsPrfErrorC = setpointGapC;
    return true;
}


/**
 * @brief Get current logical channel temperature
 * @return temperature C
 */
float PID_Control::getTempReadingC() {
    if ( NULL == getChanTempC ) {
        ERRORLN(F("No callback for getChanTempC"));
        return NAN;
    }
    return this->getChanTempC( _NVM_PID.chan - 1 );
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
    this->_heat->set( this->_origHeat );
    this->_isFanPidActive = false;
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
    this->_origHeat = this->output = this->_heat->get();

    this->_pid.Initialize();
    this->_pid.SetMode(QuickPID::Control::timer);
    if ( FanMode::automatic == this->getFanMode() ) {
        this->exhaustOutp = this->_fanMin = this->_vent->get();
        this->_pidFan.SetOutputLimits( this->_fanMin, 100 );
        if ( setp <= input ) {
            // Temp overshoot, turn on the fan
            this->_pidFan.SetMode( QuickPID::Control::timer );
            this->_isFanPidActive = true;
        } else {
            this->_pidFan.SetMode( QuickPID::Control::manual );
            this->_isFanPidActive = false;
        }
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
            Serial.print(F( "'Needs Initialization' " ));
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
    const char tmplt[] PROGMEM = "Mode=%d, Setpoint=%f, P-term=%f, I-term=%f, D-term=%f, sum=%f, Error=%f";
    char buf[sizeof(tmplt) * 2];
    buf[sizeof(buf)-1] = 0;
    snprintf_P(buf, sizeof(buf)-1, tmplt,
        _pid.GetMode(),
        setp,
        _pid.GetPterm(),
        _pid.GetIterm(),
        _pid.GetDterm(),
        _pid.GetOutputSum(),
        setp - getTempReadingC()
    );
    Serial.println( buf );

    Serial.print(F( "[FAN PID] State: " ));
    Serial.print( getFanMode() == FanMode::manual ? F("'Manual' ") : F("'Automatic' "));
    snprintf_P(buf, sizeof(buf)-1, tmplt,
        _pidFan.GetMode(),
        setp,
        _pidFan.GetPterm(),
        _pidFan.GetIterm(),
        _pidFan.GetDterm(),
        _pidFan.GetOutputSum(),
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
    _NVM_PIDPROFNORMAL.iAwMode = (QuickPID::iAwMode) mode;
    _NVM_PIDPROFCONSERV.iAwMode = (QuickPID::iAwMode) mode;
    _NVM_PIDPROFFAN.iAwMode = (QuickPID::iAwMode) mode;
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
    _NVM_PID.chan = chan;

    return true;
}


/**
 * @brief Update loop cycle time: update PID & Timer
 */
bool PID_Control::updateCycleTimeMs(uint32_t ctMS) {
    if ( ctMS < 100 ) return false;

    _NVM_PID.cycleTimeMS = ctMS;
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
    _NVM_PIDPROFNORMAL.dMode = (QuickPID::dMode) mode;
    _NVM_PIDPROFCONSERV.dMode = (QuickPID::dMode) mode;
    _NVM_PIDPROFFAN.dMode = (QuickPID::dMode) mode;
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
    _NVM_PIDPROFNORMAL.pMode = (QuickPID::pMode) mode;
    _NVM_PIDPROFCONSERV.pMode = (QuickPID::pMode) mode;
    _NVM_PIDPROFFAN.pMode = (QuickPID::pMode) mode;
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
 * @brief update PID tuning parameters for the Nth profile
 * @param profile -- profile type to update
 * @param kP
 * @param kI
 * @param kD
 * @param pmode
 */
bool PID_Control::updateProfileTuning(PID_Control::Profile profile, float kP, float kI, float kD, QuickPID::pMode pmode) {
    t_PidTune *tune;

    if ( PID_Control::Profile::normal == profile ) {
        tune = &(_NVM_PIDPROFNORMAL);
    } else if ( PID_Control::Profile::conservative == profile ) {
        tune = &(_NVM_PIDPROFCONSERV);
    } else if ( PID_Control::Profile::fan == profile ) {
        tune = &(_NVM_PIDPROFFAN);
    } else {
        return false;
    }

    tune->kP = kP;
    tune->kI = kI;
    tune->kD = kD;
    tune->pMode = pmode;
    this->_syncPidSettings();

    return true;
}


/**
 * @brief Set Fan Temperature gap -- the error between the temperature and
 *        PID setpoint, when to turn on the FAN PID, if in auto fan mode
 * @param gap -- temperature offset = setpoint + gap
 */
bool PID_Control::setFanTempGapC(float gap) {
    if ( abs( gap ) > PID_FAN_ERR_C_MAX ) return false;

    _NVM_PID.fanSPErrorC = gap;
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
        if ( input < setp ) {
            if ( abs( setp - input ) > 50 ) {
                _pid.SetOutputLimits( 15, 100 );
            } else {
                _pid.SetOutputLimits( 15, 80 );
            }
        } else {
            // overshot
            _pid.SetOutputLimits( 1, 60 );
        }
        if ( this->_pid.Compute() ) {
            DEBUG(millis()); DEBUG(F(" PID compute settings output to: "));
            DEBUGLN(this->output);
            this->_heat->set((uint8_t) this->output);
        }

        // fan pid calc
        if ( FanMode::automatic == this->getFanMode() ) {
            bool threshold = ( input >= setp + _NVM_PID.fanSPErrorC );
            if ( threshold ^ (this->_isFanPidActive) ) {
                // Transitioning from active -> idle or vice versa
                if ( threshold ) {
                    this->exhaustOutp = this->_fanMin = this->_vent->get();
                    this->_pidFan.SetOutputLimits( this->_fanMin, 100 );
                    this->_pidFan.SetMode( QuickPID::Control::timer );
                    this->_isFanPidActive = true;
                } else {
                    this->_pidFan.SetMode( QuickPID::Control::manual );
                    this->_isFanPidActive = false;
                    this->_vent->set( this->_fanMin );
                }
            }
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
    this->_pid.SetTunings(
        _NVM_PIDPROFNORMAL.kP,
        _NVM_PIDPROFNORMAL.kI,
        _NVM_PIDPROFNORMAL.kD,
        _NVM_PIDPROFNORMAL.pMode,
        _NVM_PIDPROFNORMAL.dMode,
        _NVM_PIDPROFNORMAL.iAwMode
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

    uint32_t ctus = 1000 * _NVM_PID.cycleTimeMS;
    this->_pid.SetSampleTimeUs(ctus);
    if ( this->_timer ) this->_timer->setOverflow(ctus, MICROSEC_FORMAT);
}


/**
 * @brief Switch to conservative profile/tuning if needed
 */
void PID_Control::_switchProfilesIfNeeded() {
    float gap = abs( this->setp - this->input );
    if ( _NVM_PID.cnsPrfErrorC > 0.0f
         && gap < _NVM_PID.cnsPrfErrorC ) {
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
                _NVM_PIDPROFNORMAL.kP,
                _NVM_PIDPROFNORMAL.kI,
                _NVM_PIDPROFNORMAL.kD,
                _NVM_PIDPROFNORMAL.pMode,
                _NVM_PIDPROFNORMAL.dMode,
                _NVM_PIDPROFNORMAL.iAwMode
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

    const char tmplt[] PROGMEM = "# PID us: %f, sp: %f, in: %f, heat: %f, fan: %f, conserv: %c, heat-pid: %f, %f, %f, sum: %f, fan-pid: %f, %f, %f, sum: %f";
    char buf[sizeof(tmplt) * 2];
    buf[sizeof(buf)-1] = 0;
    snprintf_P(buf, sizeof(buf)-1, tmplt,
        now / 1000.0f,
        setp,
        input,
        output,
        exhaustOutp,
        this->_isConservTuning ? 'Y' : 'N',
        _pid.GetPterm(), _pid.GetIterm(), _pid.GetDterm(), _pid.GetOutputSum(),
        _pidFan.GetPterm(), _pidFan.GetIterm(), _pidFan.GetDterm(), _pidFan.GetOutputSum()
    );
    Serial.println( buf );
}