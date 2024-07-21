#ifndef _SW_STATE_H
#define _SW_STATE_H

#include <Adafruit_MAX31855.h>
#include <filterRC.h>
#include <tick-timer.h>

#include <roaster.h>
#include "state_commanded.h"
#include "ntc.h"

class Config {
    public:
        bool isMetric;
        Config();

    protected:
};

class Stats {
    public:
        uint32_t tc_read_attempts_total;
        uint32_t tc_read_attempts_retries;
        uint32_t tc_read_attempts_failures;

        void print();
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
    
    private:
        Adafruit_MAX31855   *tc1;
        TimerMS             *tcTimer;
        TimerMS             *ambTimer;
        Config              *config;
        filterRC<float>     filter[TEMPERATURE_CHANNELS_MAX];
        NTC                 ntc;
        uint8_t             _chanMapping[TEMPERATURE_CHANNELS_MAX];
        Stats               statitstics;
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

        bool    begin();
        bool    loopTick();
        void    printState();

};

#endif  // _SW_STATE_H