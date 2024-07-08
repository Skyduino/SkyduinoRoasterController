#include "ntc.h"

double NTC::AdcToTempC(int32_t adcReading) {
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

    // we came close to two entries, check if any matches exactly
    if (_tempResistanceTable[lowBound].adcReading == adcReading) {
        return _tempResistanceTable[lowBound].temperatureC;
    } else if (_tempResistanceTable[highBound].adcReading == adcReading) {
        return _tempResistanceTable[highBound].temperatureC;
    } else {
        return 0;
    }
}