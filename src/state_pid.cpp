#include <logging.h>

#include "state_pid.h"

#define _NVM_GETPIDPROF(x) (this->_nvm->settings.pidProfiles[ x ])
#define _NVM_PIDPROFCURRENT _NVM_GETPIDPROF( this->_nvm->settings.pidCurrentProfile )


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
    this->_pid.SetOutputLimits(0, 100);
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
 * @brief loop process
 */
bool PID_Control::loopTick() {
    if ( NULL != this->_tuner ) {
        if ( this->_tuner->getState() != Autotuner::State::running ) {
            // Cleanup the tuner.
            this->_tuner->stop();
            this->stopAutotune();
        }
    }
    return true;
}


/**
 * @brief Turn off the PID controller
 */
void PID_Control::turnOff() {
    if ( getState() == this->State::needsInit
         || getState() == this->State::autotune ) return;
    this->_pid.SetMode(QuickPID::Control::manual);
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

    this->_pid.Initialize();
    this->_pid.SetMode(QuickPID::Control::timer);
    this->_timer->resume();
    this->_state = this->State::on;
}


/**
 * @brief start auto tune process
 */
void PID_Control::startAutotune() {
    if ( this->getState() == this->State::needsInit
         || this->getState() == this->State::autotune
         || this->getState() == this->State::aborted ) return;

    this->turnOff();
    this->_tuner = new Autotuner( this->_nvm, this->_timer, this->getLogicalChanTempC );
    this->_tuner->begin();
    this->_tuner->start();
    this->_state = this->State::autotune;
}


/**
 * @brief stop auto tune process
 */
void PID_Control::stopAutotune() {
    if ( getState() == this->State::autotune ) {
        this->_tuner->stop();
        delete this->_tuner;
        this->_tuner = NULL;
        this->_state = this->State::off;
        this->_timer->detachInterrupt();
        this->_timer->attachInterrupt(
            std::bind(&PID_Control::_compute, this)
        );
        this->_syncPidSettings();
    }
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
 * @brief Do the PID calculation here
 */
void PID_Control::_compute() {
    float tempC = this->getTempReadingC();

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


/**
 * @brief prepare auto tuner
 */
void Autotuner::begin() {
    pinMode( PIN_HEAT, OUTPUT );
    digitalWrite( PIN_HEAT, LOW );
    this->output = 0;
    tuner.Configure(
        stnStngs.inputSpan,
        stnStngs.outputSpan,
        stnStngs.outputStart,
        stnStngs.outputStep,
        stnStngs.testTimeSec,
        stnStngs.settleTimeSec,
        stnStngs.samples
    );
    tuner.SetEmergencyStop( stnStngs.tempLimit );
    this->_state = State::idle;

    uint32_t ctus = _NVM_PIDPROFCURRENT.cycleTimeMS * 100;
    this->_timer->setOverflow(ctus, MICROSEC_FORMAT);
    this->_timer->detachInterrupt();
}


/**
 * @brief Start the auto tuner
 */
void Autotuner::start() {
    float curTemp = getTempReadingC();
    if ( curTemp < ( this->_nvm->settings.maxSafeTempC - 60 - 5 ) ) {
        this->setp = curTemp + 60.0f;
    };
    DEBUG(millis()); DEBUG(F("Starting Autotune with setpoint C: "));
    DEBUGLN( this->setp );
    this->_state = Autotuner::State::running;
    this->_timer->refresh();
    this->_timer->attachInterrupt( std::bind( &Autotuner::_tuneLoop, this ) );
    this->_timer->resume();
}


/**
 * @brief Stop the auto tuner
 */
void Autotuner::stop() {
    DEBUG(millis()); DEBUGLN("Stoping autotuner and Detaching the interrupt");
    this->_timer->pause();
    this->_timer->detachInterrupt();
    digitalWrite( PIN_HEAT, LOW );
}


/**
 * @brief Get current logical channel temperature
 * @return temperature C
 */
float Autotuner::getTempReadingC() {
    if ( NULL == getLogicalChanTempC ) {
        ERRORLN(F("No callback for getLogicalChanTempC"));
        return NAN;
    }
    return this->getLogicalChanTempC( _NVM_PIDPROFCURRENT.chan );
}


/**
 * @brief interrupt handler, auto tuner 
 */
void Autotuner::_tuneLoop() {
    if ( Autotuner::State::running != this->getState() ) {
        DEBUGLN( "Ignoring tuner loop, not running");
        return;
    }
    optimumOutput = tuner.softPwm( PIN_HEAT & 0xFF, input, output, setp, stnStngs.outputSpan, stnStngs.debounce);
 
    switch (tuner.Run()) {
        case tuner.sample: // active once per sample during test
            DEBUG(millis()); DEBUGLN(F("Autotune sample"));
            input = this->getTempReadingC();
            tuner.plotter(input, output, setp, 0.5f, 3); // output scale 0.5, plot every 3rd sample
            break;
 
        case tuner.tunings: // active just once when sTune is done
            DEBUG(millis()); DEBUGLN(F("Autotune complete"));
            tuner.printTunings();
            tuner.GetAutoTunings(
                &_NVM_PIDPROFCURRENT.kP,
                &_NVM_PIDPROFCURRENT.kI,
                &_NVM_PIDPROFCURRENT.kD
            ); // sketch variables updated by sTune
            this->_nvm->markDirty();
            _pid.SetOutputLimits(0, stnStngs.outputSpan * 0.1);
            _pid.SetSampleTimeUs((stnStngs.outputSpan - 1) * 1000);
            stnStngs.debounce = 0; // ssr mode
            output = stnStngs.outputStep;
            _pid.SetMode(_pid.Control::automatic); // the PID is turned on
            _pid.SetProportionalMode(_pid.pMode::pOnMeas);
            _pid.SetAntiWindupMode(_pid.iAwMode::iAwClamp);
            _pid.SetTunings(
                _NVM_PIDPROFCURRENT.kP,
                _NVM_PIDPROFCURRENT.kI,
                _NVM_PIDPROFCURRENT.kD
            ); // update PID with the new tunings
            this->_state = Autotuner::State::done;
            break;
 
        case tuner.runPid: // active once per sample after tunings
            if (stnStngs.startup && input > setp - 5) { // reduce overshoot
                stnStngs.startup = false;
                output -= 9;
                _pid.SetMode(_pid.Control::manual);
                _pid.SetMode(_pid.Control::automatic);
            }
            input = this->getTempReadingC();
            _pid.Compute();
            tuner.plotter(input, optimumOutput, setp, 0.5f, 3);
            break;
   }
}