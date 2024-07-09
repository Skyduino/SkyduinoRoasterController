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
        float AdcToTempC(int32_t adcReading);
    
    private:
        static constexpr uint32_t tableSize = NTC_TABLE_SIZE;
        static constexpr NTC_Temp_Resistance_t _tempResistanceTable[] = NTC_DATA;
};

#endif  // _SW_NTC_H