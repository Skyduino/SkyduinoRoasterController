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