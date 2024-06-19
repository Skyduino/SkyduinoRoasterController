#include "roaster.h"
#include "state.h"


Config::Config() {
    isMetric = false;
}

Status::Status() {
    tcStatus = 1;
}

bool State::begin() {
    bool isSuccess = true;

    isSuccess &= commanded.begin();
    isSuccess &= reported.begin();

    return isSuccess;
}

bool State::loopTick() {
    bool isSuccess = true;

    isSuccess &= commanded.loopTick();
    isSuccess &= reported.loopTick();

    return true;
    return isSuccess;
}


/**
 * Outputs to serial the current roaster state in response to READ command
 * Print out comma separated values for ambient temp, channel temps, OT1, OT2
*/
void State::printState() {
    reported.printState();
    commanded.printState();
}
