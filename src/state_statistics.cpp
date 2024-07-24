#include "state_statistics.h"


/**
 * @brief Print Reporte State Class statistics
 */
void StatsReported::print() {
    Serial.print(F("Thermocouple Read Attempts Total/Retries/Failed: "));
    
    char buf[255];
    const char format[] = "%lu/%lu/%lu";
    sprintf(buf, format, tc_read_attempts_total, tc_read_attempts_retries, tc_read_attempts_failures);
    Serial.println(buf);
}


/**
 * @brief Print general stats
 */
void Stats::print() {
    Serial.print(F("Loop timing min/avg/max: "));
    
    char buf[255];
    const char format[] = "%3.2fms / %3.2fms / %3.2fms";
    sprintf(buf, format, (float) _loopMin/1000, (float) _loopAvg/1000, (float) _loopMax/1000);
    Serial.println(buf);
}


/**
 * @brief Mark loop start and store the tick for later loop time min/avg/max calculation
 */
void Stats::loopStart()
{
    this->_loopStartTime = micros();
}

/**
 * @brief Mark loop end and calculate loop min/avg/max times
 */
void Stats::loopEnd() {
    uint32_t loopDuration = micros() - this->_loopStartTime;

    if ( loopDuration < this->_loopMin ) {
        this->_loopMin = loopDuration;
    } else if ( loopDuration > this->_loopMax ) {
        this->_loopMax = loopDuration;
    }

    // at certain point this is going to overflow and it will reset the avg
    if ( 0 == this->_count) {
        this->_loopAvg = loopDuration;
    } else {
        this->_loopAvg += (loopDuration - _loopAvg) / _count;
    }
    this->_count++;
}
