#include "state.h"

StateCommanded::StateCommanded() {
    heat    = 0;
    vent    = 0;
    cool    = 0;
    filter  = 0;
    drum    = 0;
}

Reported::Reported() {
    for (uint8_t i = 0; i < TEMPERATURE_CHANNELS_MAX; i++) {
        chanTemp[i] = 0;
    }
    ambient = 0;
}

Status::Status() {
    timer = TimerMS(TC4_COMM_TIMEOUT_MS);      // tc4ComTimeOut
    tcStatus = 1;
}

