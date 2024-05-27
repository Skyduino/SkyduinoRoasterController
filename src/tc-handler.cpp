/* The TC handler is responsible to update the state structure with
 * thermocouple temperature. MAX31855 also provides the internal temp
 * so use that as the ambient temperature provider, although it is really
 * going to be skewed by the internal heat.
 */

#include "logging.h"
#include "roaster.h"
#include "state.h"
#include "tc-handler.h"

TCHandler::TCHandler(t_State *state)
{
    this->state = state;
}

bool TCHandler::begin()
{
    return this->tc1.begin();
}

bool TCHandler::loopTick() {
    if (tcTimer.hasTicked()) {
        readTemperature();
        tcTimer.reset();
    }

    if (ambTimer.hasTicked()) {
        readAmbient();
        ambTimer.reset();
    }

    return true;
}

void TCHandler::readAmbient() {
    double temp = tc1.readInternal();
 
    if (isnan(temp)) {
        this->tcError();
    } else {
        if ( !(state->cfg.isMetric) ) {
            temp *= 9.0;
            temp /= 5.0;
            temp += 32;
        }
        state->reported.ambient = temp;
    }
}

void TCHandler::readTemperature() {
    double temp = state->cfg.isMetric ? tc1.readCelsius() : tc1.readFahrenheit();
 
    if (isnan(temp)) {
        return;
        this->tcError();
    } else {
        temp = state->cfg.filter[TEMPERATURE_CHANNEL_THERMOCOUPLE].doFilter(temp);
        TEMPERATURE_TC(state->reported) = temp;
    }
}

void TCHandler::tcError() {
    uint8_t e = tc1.readError();

    WARN(F("TC Fault: "));
    if (e & MAX31855_FAULT_OPEN) WARNLN(F("FAULT: Thermocouple is open - no connections."));
    if (e & MAX31855_FAULT_SHORT_GND) WARNLN(F("FAULT: Thermocouple is short-circuited to GND."));
    if (e & MAX31855_FAULT_SHORT_VCC) WARNLN(F("FAULT: Thermocouple is short-circuited to VCC."));
}
