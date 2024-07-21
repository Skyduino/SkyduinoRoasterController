#include <logging.h>

#include "safemon.h"

#define IS_EXCEEDING(x, t) ( (!isnan(x)) && (x >= t) )
#define IS_EXCEEDING_MAX (_isTempExceedingC(MAX_SAFE_TEMP_C))
#define IS_EXCEEDING_LOW_MAX ( \
        _isTempExceedingC( \
            MAX_SAFE_TEMP_C - SAFE_TEMP_HISTERESIS_C \
        ))


bool SafetyMonitor::begin() {
    if ( !isInitialized ) {
        timer = new TimerMS(MAX_SAFE_TIMEOUT_MS);
        isInitialized = true;
    }

    return true;
}


/**
 * @brief Handle state transitions for the safety monitor
 * 
 * The logic is fairly simple: transition to ARMED state, if the exceed counter is
 * positive. Transition back to NORMAL, if the exceed counter is 0. Staying long enough
 * in ARMED state, eventually, from ARMED transition to TRIGGERED state if still
 * exceeding the temperature.
 * 
 * @return true if tick is successfuly handled (always)
 */
bool SafetyMonitor::loopTick() {
    if ( !isInitialized ) begin();

    switch ( _monitorState ) {
    case NORMAL:
        _handleNormal(); 
        break;

    case PRE_ARM:
        _handlePreArm(); 
        break;

    case ARMED:
        _handleArmed(); 
        break;

    case TRIGGERED:
        _handleTriggered(); 
        break;

    default:
        ERRORLN(F("Unknown State"));
        triggerSafetyAction();
        break;
    }

    return true;
}


/**
 * @brief Trigger abort action for the roaster commanded state
 */
void SafetyMonitor::triggerSafetyAction() {
    DEBUG(micros()); DEBUGLN(F(" Triggering Safety Action"));
    this->_roasterState->commanded.abort();
}


/**
 * @brief Check if any channel exceeds the max safety temperature
 * 
 * if none of the temperature channels are avaiable, then return True, better
 * safe than sorry. However, the NTC channel should always be there, unless
 * there is a hardware fault.
 * 
 * @return true if any temperature channels are exceeding the max temperature
 *         or if both channels are unavailable
 */
bool SafetyMonitor::_isTempExceedingC(float threshold) {
    float tcTemp = _roasterState->reported.getChanTemp(TEMPERATURE_CHANNEL_THERMOCOUPLE);
    float ntcTemp = _roasterState->reported.getChanTemp(TEMPERATURE_CHANNEL_ROASTER);

    // it temperature reading is not available at all, then better safe than sorry
    bool noTempAtAll = isnan(tcTemp) && isnan(ntcTemp);

    return noTempAtAll
           || IS_EXCEEDING( tcTemp, threshold )
           || IS_EXCEEDING( ntcTemp, threshold ); 
}


/**
 * @brief Transition to NORMAL state
 */
void SafetyMonitor::_transitionToNormal() {
    DEBUG(micros()); DEBUGLN(F(" Transitioning to Normal state"));
    _monitorState = NORMAL;
}


/**
 * @brief handle NORMAL state and possible transition to a different -- PRE_ARM
 *        state
 */
void SafetyMonitor::_handleNormal() {
    if ( IS_EXCEEDING_MAX ) _transitionToPreArm();
}


/**
 * @brief Transition to PRE_ARM state
 * 
 * Does not reset active timer, therefore if bouncing quickly between MAX and MAX-Histereris
 * may trigger the safety action.
 */
void SafetyMonitor::_transitionToPreArm() {
    DEBUG(micros()); DEBUGLN(F(" Transitioning to PRE-ARM state"));
    _monitorState = PRE_ARM;
    if ( timer->hasTicked() ) timer->reset();
}


/**
 * @brief Handle PRE_ARM state
 * 
 * May decide dropping back to NORMAL, if the temperature drops.
 * Or may transition to ARMED state if timer expires while the temp is exceeding the max safe limit.
 * 
 */
void SafetyMonitor::_handlePreArm() {
    if ( timer->hasTicked() ) {
        _transitionToArmed();
    } else if ( !IS_EXCEEDING_LOW_MAX ) {
        _transitionToNormal();
    }
}


/**
 * @brief Transition to ARMED state
 * 
 */
void SafetyMonitor::_transitionToArmed() {
    DEBUG(micros()); DEBUGLN(F(" Transitioning to ARM state"));
    _monitorState = ARMED;
    timer->reset();
}


/**
 * @brief Handle ARMED state
 * 
 * May decide dropping back to NORMAL, if the temperature drops.
 * Or may transition to ARMED state if timer expires while the temp is exceeding the max safe limit.
 * 
 */
void SafetyMonitor::_handleArmed() {
    if ( timer->hasTicked() ) {
        _transitionToTriggered();
    } else if ( !IS_EXCEEDING_LOW_MAX ) {
        _transitionToNormal();
    }
}


/**
 * @brief Transition to triggered state and effectively call safety action
 */
void SafetyMonitor::_transitionToTriggered() {
    DEBUG(micros()); DEBUGLN(F(" Transitioning to Triggered state"));
    _monitorState = TRIGGERED;
    _handleTriggered();
}


/**
 * @brief Trigger safetyAction
 * 
 * Trigger saferyAction every MAX_SAFE_TIMEOUT_MS in perpetuity,
 * until a hardware reset or power reset.
 */
void SafetyMonitor::_handleTriggered() {
    if ( timer->hasTicked() ) {
        triggerSafetyAction();
        timer->reset();
    }
}
