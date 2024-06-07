#include "logging.h"
#include "state.h"

bool Reported::begin() {
    bool isSuccess = true;
    isSuccess &= tcHandler.begin();
    if ( !isSuccess ) {
        ERRORLN(F("Couldn't not initialize MAX3185"));
    }
    return isSuccess;
}

bool Reported::loopTick() {
    tcHandler.loopTick();

    return true;
}
