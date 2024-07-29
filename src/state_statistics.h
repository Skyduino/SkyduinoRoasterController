#ifndef _SW_STATISTICS_H
#define _SW_STATISTICS_H

#include <Arduino.h>

class Stats {
    public:
        void print();
        void loopStart();
        void loopEnd();
    
    protected:
        uint32_t _loopStartTime;
        uint32_t _count = 0;
        uint32_t _loopMin = 0xFFFFFFFF;
        uint32_t _loopMax = 0;
        uint32_t _loopAvg = 0;
};


class StatsReported {
    public:
        uint32_t tc_read_attempts_total;
        uint32_t tc_read_attempts_retries;
        uint32_t tc_read_attempts_failures;

        void print();
};

#endif // _SW_STATISTICS_H