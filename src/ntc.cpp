#include <cmath>

#include "ntc.h"

float NTC::AdcToTempC(int32_t adcReading) {
    // check if the value is outside of lookup range
    if ( adcReading >  _tempResistanceTable[0].adcReading
         || adcReading < _tempResistanceTable[tableSize-1].adcReading) {
        return NAN;
    }

    uint32_t lowBound = 0;
    uint32_t highBound = tableSize - 1;
    uint32_t attempts = tableSize - 1;
    uint32_t midpoint;
    NTC_Temp_Resistance_t *entry;

    while ( ((highBound - lowBound) > 1) && attempts ) {
        attempts--;
        midpoint = (highBound + lowBound) / 2;
        entry = &(_tempResistanceTable[ midpoint ]);
        if ( entry->adcReading == adcReading ) {
            return entry->temperatureC; 
        } else if ( entry->adcReading < adcReading ) {
            highBound = midpoint;
        } else {
            lowBound = midpoint;
        }
    }

    // we narrowed it down to two entries, check if the upper one matches exactly
    NTC_Temp_Resistance_t *entry_l = &(_tempResistanceTable[ lowBound]);
    NTC_Temp_Resistance_t *entry_h = &(_tempResistanceTable[ highBound]);

    if (entry_l->adcReading == adcReading) {
        return entry_l->temperatureC;
    } else if (entry_h->adcReading == adcReading) {
        return entry_h->temperatureC;
    }

    return -273;
}