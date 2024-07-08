#ifndef _SW_NTC_H
#define _SW_NTC_H

#include <Arduino.h>

#include "ntc-data.h"

typedef struct {
    int      temperatureC;
    uint16_t adcReading;
} NTC_Temp_Resistance_t;


/**
 * @brief Helper Class to convert ADC reading to temperature in C
 * 
 */
class NTC {
    public:
        double AdcToTempC(int32_t adcReading);
    
    private:
        int32_t lastLookup = -1;
        const uint32_t tableSize = NTC_TABLE_SIZE;
        NTC_Temp_Resistance_t _tempResistanceTable[NTC_TABLE_SIZE] = NTC_DATA;
};

#endif  // _SW_NTC_H