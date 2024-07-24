#ifndef _SW_STATISTICS_H
#define _SW_STATISTICS_H

#include <Arduino.h>

class StatsReported {
    public:
        uint32_t tc_read_attempts_total;
        uint32_t tc_read_attempts_retries;
        uint32_t tc_read_attempts_failures;

        void print();
};

#endif // _SW_STATISTICS_H