#include "safemon.h"

#define IS_EXCEEDING(x) ( (!isnan(x)) && (x >= MAX_SAFE_TEMP_C) )

bool SafetyMonitor::begin() {
    if ( !isInitialized ) {
        timer = new TimerMS(MAX_SAFE_TIMEOUT_MS);
        isInitialized = true;
    }

    exceedCounter = 0;
    return true;
}

bool SafetyMonitor::loopTick() {
    if ( !isInitialized ) begin();


    return true;
}

void SafetyMonitor::triggerSafetyAction() {
}

bool SafetyMonitor::_isTempExceeded() {
    float tcTemp = _state->reported.getChanTemp(TEMPERATURE_CHANNEL_THERMOCOUPLE);
    float ntcTemp = _state->reported.getChanTemp(TEMPERATURE_CHANNEL_ROASTER);

    // it temperature reading is not available at all, then better safe than sorry
    bool noTempAtAll = isnan(tcTemp) && isnan(ntcTemp);

    return noTempAtAll || IS_EXCEEDING( tcTemp ) || IS_EXCEEDING( ntcTemp ); 
}
