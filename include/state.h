#ifndef _SW_STATE_H
#define _SW_STATE_H

#include <Adafruit_MAX31855.h>
#include <filterRC.h>
#include <tick-timer.h>

#include <roaster.h>
#include "state_commanded.h"
#include "state_statistics.h"
#include "ntc.h"

class Config {
    public:
        bool isMetric;
        Config();

    protected:
};


class Reported {
    public:
        Reported(Config *config);
        bool begin();
        bool loopTick();
        uint8_t setChanFilter(uint8_t idx, uint8_t percent);
        void printState();
        void printStatistics();
        uint8_t getChanMapping(uint8_t idx);
        void setChanMapping(uint8_t idx, uint8_t mapping);
        float getChanTemp(uint8_t chan);
        uint32_t getSkywalkerADC();
    
    private:
        Adafruit_MAX31855   *tc1;
        TimerMS             *tcTimer;
        TimerMS             *ambTimer;
        Config              *config;
        filterRC<float>     filter[TEMPERATURE_CHANNELS_MAX];
        NTC                 ntc;
        uint8_t             _chanMapping[TEMPERATURE_CHANNELS_MAX];
        StatsReported       statitstics;
        float               chanTemp[TEMPERATURE_CHANNELS_MAX]; // Physical temp channels
        float               ambient;  // Ambient temperature

        void readAmbient();
        void readTemperature();
        void tcError();
        void _readNTC();
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
        Stats           stats;

        bool    begin();
        bool    loopTick();
        void    printState();
        void    printStatistics();
};

#endif  // _SW_STATE_H