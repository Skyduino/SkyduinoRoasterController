#ifndef _SW_NTC_H
#define _SW_NTC_H

#include <Arduino.h>

#include "ntc-data.h"

typedef struct {
    int16_t  temperatureC;
    uint16_t adcReading;
    uint16_t skwAdcBytes01;
    uint16_t skwAdcBytes23;
} NTC_Temp_Resistance_t;


/**
 * @brief Helper Class to convert ADC reading to temperature in C
 * 
 */
class NTC {
    public:
        float AdcToTempC(int32_t adcReading);
        uint32_t TempCtoSkywalkerADC(float temp);
    
    private:
        static constexpr uint32_t tableSize = NTC_TABLE_SIZE;
        static constexpr NTC_Temp_Resistance_t _tempResistanceTable[] = NTC_DATA;
};

#endif  // _SW_NTC_H