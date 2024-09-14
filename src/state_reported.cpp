#include <cmath>
#include <SPI.h>

#include "roaster.h"
#include "logging.h"
#include "state.h"

#define TC_MAX_READ_ATTEMPTS 10


Reported::Reported(Config *config) {
    this->config = config;

    _chanMapping[0] = TEMPERATURE_CHANNEL_ROASTER+1;
    _chanMapping[1] = TEMPERATURE_CHANNEL_THERMOCOUPLE+1;
    _chanMapping[2] = 0;
    _chanMapping[3] = 0;

    for (uint8_t i = 0; i < TEMPERATURE_CHANNELS_MAX; i++) {
        chanTemp[i] = 0;
    }
    ambient = 0;

    tc1 = new Adafruit_MAX31855(SPI_BTCS);
    tcTimer = new TimerMS(THERMOCOUPLE_UPDATE_INTERVAL_MS);
    ambTimer = new TimerMS(THERMOCOUPLE_UPDATE_INTERVAL_MS << 3);
}


bool Reported::begin() {
    pinMode(PIN_NTC, INPUT_ANALOG);
    analogReadResolution(12);

    bool isSuccess = true;
    isSuccess &= tc1->begin();
    if ( !isSuccess ) {
        ERRORLN(F("Couldn't not initialize MAX3185"));
    }
    return isSuccess;
}

bool Reported::loopTick() {
    if (tcTimer->hasTicked()) {
        readTemperature();
        _readNTC();
        tcTimer->reset();
    }

    if (ambTimer->hasTicked()) {
        readAmbient();
        ambTimer->reset();
    }

    return true;
}


/**
 * Outputs to serial comma separated values for ambient and channel temps
 * part of the response to READ command
*/
void Reported::printState() {
    Serial.print( config->isMetric ? this->ambient : CONVERT_C_TO_F( this->ambient ));
    uint8_t mapping = 0;
    float temp;
    for (uint8_t i = 0; i < TEMPERATURE_CHANNELS_MAX; i++) {
        mapping = this->_chanMapping[i];
        if ((mapping >= 1)
            && (mapping <= TEMPERATURE_CHANNELS_MAX)) {
            Serial.print(F(","));
            temp = this->chanTemp[mapping - 1];
            Serial.print( config->isMetric ? temp : CONVERT_C_TO_F( temp ) );
        }
    }
}


void Reported::printStatistics() {
    this->statitstics.print();
}


uint8_t Reported::setChanFilter(uint8_t idx, uint8_t percent) {
    // convert from logical to phiscial channel
    uint8_t chan = this->_chanMapping[idx];

    if( chan > 0 ) { // is the physical channel active?
        --chan;
        this->filter[chan].init( percent );
        Serial.print(F("# Physical channel ")); Serial.print( chan );
        Serial.print(F(" filter set to ")); Serial.println( percent );
    }

    return chan;
}


/*
 * Protected and private implementations
 */
void Reported::readAmbient() {
    ambient = tc1->readInternal();
}

void Reported::readTemperature() {
    float temp = NAN;
    
    this->statitstics.tc_read_attempts_total++;
    uint8_t attempts = TC_MAX_READ_ATTEMPTS;
    
    while ( attempts && isnan(temp = tc1->readCelsius()) ) {
        attempts--;
        this->statitstics.tc_read_attempts_retries++;
    }
 
    if ( !isnan(temp)) {
        temp = filter[TEMPERATURE_CHANNEL_THERMOCOUPLE].doFilter(temp);
        TEMPERATURE_TC(chanTemp) = temp;
    } else {
        this->statitstics.tc_read_attempts_failures++;
        this->tcError();
    }
}

void Reported::tcError() {
    uint8_t e = tc1->readError();

    WARN(F("TC Fault: "));
    if (e & MAX31855_FAULT_OPEN) {
        WARNLN(F("FAULT: Thermocouple is open - no connections."));
        TEMPERATURE_TC(chanTemp) = NAN;
    }
    if (e & MAX31855_FAULT_SHORT_GND) WARNLN(F("FAULT: Thermocouple is short-circuited to GND."));
    if (e & MAX31855_FAULT_SHORT_VCC) WARNLN(F("FAULT: Thermocouple is short-circuited to VCC."));
}


void Reported::setChanMapping(uint8_t idx, uint8_t mapping) {
    this->_chanMapping[idx] = mapping;
}

float Reported::getChanTemp(uint8_t chan) {
    if ( chan >= TEMPERATURE_CHANNELS_MAX ) {
        // this should not happen, but as a precaution
        // for wrong channels return temperature exceeding safety limits
        return (MAX_SAFE_TEMP_C + 1);
    }
    
    return this->chanTemp[ chan ];
}

void Reported::_readNTC() {
    float ntcTemp = ntc.AdcToTempC( analogRead(PIN_NTC) );
    if ( !isnan(ntcTemp) ) {
        ntcTemp = filter[TEMPERATURE_CHANNEL_ROASTER].doFilter(ntcTemp);
    }

    TEMPERATURE_ROASTER(chanTemp) = ntcTemp;
}


/**
 * @brief Get Skywalker ADC data, return 2x 10bit ADC values packed into 32 bits
 *        The BT is Channel 1 so get the reading which is mapped to this channel
 *        depending how Artisan/User set ups channel mapping, this could be the
 *        NTC or the thermocouple reading.
 *
 * @return uint32_t of two ADC values for the Skywalker Remote
 */
uint32_t Reported::getSkywalkerADC() {
    float tempC;

    uint8_t mapping = this->_chanMapping[1];
    if ((mapping >= 1)
        && (mapping <= TEMPERATURE_CHANNELS_MAX)) {
        tempC = this->chanTemp[mapping - 1];
    } else {
        tempC = 0;
    }

    return this->ntc.TempCtoSkywalkerADC(tempC);
}