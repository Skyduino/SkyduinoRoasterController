#ifndef _SW_STATE_H
#define _SW_STATE_H

#include <filterRC.h>

#include <roaster.h>
#include "tc-handler.h"
#include "tick-timer.h"

class StateCommanded {
    public:
        uint8_t heat;
        uint8_t vent;
        uint8_t cool;
        uint8_t filter;
        uint8_t drum;

        StateCommanded();
        bool begin();
        bool loopTick();
};

class Reported {
    public:
        double chanTemp[TEMPERATURE_CHANNELS_MAX]; // Physical temp channels
        double ambient;  // Ambient temperature

        Reported(Config *config);
        bool begin();
        bool loopTick();
    
    private:
        Adafruit_MAX31855   tc1 = Adafruit_MAX31855(SPI_BTCS);
        TimerMS             tcTimer = TimerMS(THERMOCOUPLE_UPDATE_INTERVAL_MS);
        TimerMS             ambTimer = TimerMS(THERMOCOUPLE_UPDATE_INTERVAL_MS << 3);
        Config              *config;

        void readAmbient();
        void readTemperature();
        void tcError();
};

class Config {
    public:
        uint8_t chanMapping[TEMPERATURE_CHANNELS_MAX];
        filterRC<double> filter[TEMPERATURE_CHANNELS_MAX];
        bool isMetric;
        Config();
};

class Status {
    public:
        uint8_t tcStatus;
        Status();
};

class State {
    public:
        StateCommanded  commanded;
        Reported        reported;
        Config          cfg;
        Status          status;

        bool    begin();
        bool    loopTick();
};

#endif  // _SW_STATE_H