#include "roaster.h"
#include "state.h"

StateCommanded::StateCommanded() {
    heat    = 0;
    vent    = 0;
    cool    = 0;
    filter  = 0;
    drum    = 0;
}


Config::Config() {
    chanMapping[0] = TEMPERATURE_CHANNEL_ROASTER+1;
    chanMapping[1] = TEMPERATURE_CHANNEL_THERMOCOUPLE+1;
    chanMapping[2] = 0;
    chanMapping[3] = 0;

    isMetric = false;
}

Reported::Reported(Config *config) {
    this->config = config;

    for (uint8_t i = 0; i < TEMPERATURE_CHANNELS_MAX; i++) {
        chanTemp[i] = 0;
    }
    ambient = 0;
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
