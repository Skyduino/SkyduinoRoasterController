#include <SPI.h>

#include "logging.h"
#include "state.h"

bool Reported::begin() {
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
        tcTimer->reset();
    }

    if (ambTimer->hasTicked()) {
        readAmbient();
        ambTimer->reset();
    }

    return true;
}

double Reported::getAmbient() {
    return this->ambient;
}

double Reported::getChanTemp(uint8_t chan) {
    return this->chanTemp[chan];
}

/*
 * Protected and private implementations
 */
void Reported::readAmbient() {
    double temp = tc1->readInternal();
 
    if (isnan(temp)) {
        this->tcError();
    } else {
        if ( !(config->isMetric) ) {
            temp *= 9.0;
            temp /= 5.0;
            temp += 32;
        }
        ambient = temp;
    }
}

void Reported::readTemperature() {
    double temp = config->isMetric ? tc1->readCelsius() : tc1->readFahrenheit();
 
    if (isnan(temp)) {
        return;
        this->tcError();
    } else {
        temp = filter[TEMPERATURE_CHANNEL_THERMOCOUPLE].doFilter(temp);
        TEMPERATURE_TC(chanTemp) = temp;
    }
}

void Reported::tcError() {
    uint8_t e = tc1->readError();

    WARN(F("TC Fault: "));
    if (e & MAX31855_FAULT_OPEN) WARNLN(F("FAULT: Thermocouple is open - no connections."));
    if (e & MAX31855_FAULT_SHORT_GND) WARNLN(F("FAULT: Thermocouple is short-circuited to GND."));
    if (e & MAX31855_FAULT_SHORT_VCC) WARNLN(F("FAULT: Thermocouple is short-circuited to VCC."));
}


uint8_t Reported::setChanFilter(uint8_t idx, uint8_t percent) {
    // convert from logical to phiscial channel
    uint8_t chan = config->getChanMapping(idx);

    if( chan > 0 ) { // is the physical channel active?
        --chan;
        this->filter[chan].init( percent );
        Serial.print(F("# Physical channel ")); Serial.print( chan );
        Serial.print(F(" filter set to ")); Serial.println( percent );
    }

    return chan;
}