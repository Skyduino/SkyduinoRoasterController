#include "roaster.h"
#include "state.h"


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

    tc1 = new Adafruit_MAX31855(SPI_BTCS);
    tcTimer = new TimerMS(THERMOCOUPLE_UPDATE_INTERVAL_MS);
    ambTimer = new TimerMS(THERMOCOUPLE_UPDATE_INTERVAL_MS << 3);

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
