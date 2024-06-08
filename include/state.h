#ifndef _SW_STATE_H
#define _SW_STATE_H

#include <Adafruit_MAX31855.h>
#include <filterRC.h>

#include <roaster.h>
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

class Config {
    public:
        uint8_t chanMapping[TEMPERATURE_CHANNELS_MAX];
        filterRC<double> filter[TEMPERATURE_CHANNELS_MAX];
        bool isMetric;
        Config();
};

class Reported {
    public:
        Reported(Config *config);
        bool begin();
        bool loopTick();
        double getAmbient();
        double getChanTemp(uint8_t chan);
    
    private:
        Adafruit_MAX31855   *tc1;
        TimerMS             *tcTimer;
        TimerMS             *ambTimer;
        Config              *config;
        double              chanTemp[TEMPERATURE_CHANNELS_MAX]; // Physical temp channels
        double              ambient;  // Ambient temperature

        void readAmbient();
        void readTemperature();
        void tcError();
};

class Status {
    public:
        uint8_t tcStatus;
        Status();
};

class State {
    public:
        StateCommanded  commanded;
        Reported        reported = Reported(&cfg);
        Config          cfg;
        Status          status;

        bool    begin();
        bool    loopTick();
};

#endif  // _SW_STATE_H