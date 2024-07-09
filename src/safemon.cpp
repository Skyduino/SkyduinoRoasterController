#include "safemon.h"

bool SafetyMonitor::begin() {
    if ( !isInitialized ) {
        timer = new TimerMS(MAX_SAFE_TIMEOUT_MS);
        isInitialized = true;
    }

    return true;
}

bool SafetyMonitor::loopTick() {
    if ( !isInitialized ) begin();

    return true;
}

void SafetyMonitor::triggerSafetyAction() {
}
