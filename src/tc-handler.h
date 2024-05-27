#ifndef __SW_THERMOCOUPLE_H
#define __SW_THERMOCOUPLE_H

#include <Adafruit_MAX31855.h>
#include <tick-timer.h>

#include "state.h"


class TCHandler {
    public:
        TCHandler(t_State* state);
        bool begin();
        bool loopTick();

    private:
        Adafruit_MAX31855   tc1 = Adafruit_MAX31855(SPI_BTCS);
        TimerMS             tcTimer = TimerMS(THERMOCOUPLE_UPDATE_INTERVAL_MS);
        TimerMS             ambTimer = TimerMS(THERMOCOUPLE_UPDATE_INTERVAL_MS << 3);
        t_State             *state;

        void readAmbient();
        void readTemperature();
        void tcError();
};

#endif  // __SW_THERMOCOUPLE_H