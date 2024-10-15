#include <CRC16.h>
#include <CRC.h>
#include <EEPROM.h>
#include <logging.h>

#include "eeprom_settings.h"


/**
 * @brief initialize the Eeprom settings container. Load from eeprom
 *        and initialize the defaults if eeprom data is garbage
 */
EepromSettings::EepromSettings(const t_Settings *eeprom): defaultSettings(eeprom) {
    EEPROM.get(EEPROM_SETTINGS_ADDR, settings);
    uint16_t crc = calcCRC16((uint8_t *) &settings, offsetof(t_Settings, crc16));
    if ( (settings.crc16 != crc) || (settings.eepromMagic != EEPROM_SETTINGS_MAGIC) )
    {
        this->loadDefaults();
    }
}


/**
 * @brief check if the timer is fired and is the buffer is marked as dirty
 */
bool EepromSettings::loopTick() {
    if ( isDirty && this->timer.hasTicked() ) {
        this->save();
        this->timer.reset();
    }

    return true;
}


/**
 * @brief print current eeprom settings
 */
void EepromSettings::print() {
    Serial.print(F("NVM Power On count: "));
    Serial.println(this->settings.counters.powerOnResets);
    Serial.print(F("NVM Watchdog Reset count: "));
    Serial.println(this->settings.counters.watchdogResets);
    Serial.print(F("NVM Software Reset count: "));
    Serial.println(this->settings.counters.softResets);
    Serial.print(F("NVM Safety triggered count: "));
    Serial.println(this->settings.counters.safetyTriggers);
#ifdef USE_STEPPER_DRUM
    Serial.print(F("NVM Stepper driver steps per revolution: "));
    Serial.println(this->settings.stepsPerRevolution);
    Serial.print(F("NVM Stepper driver Max RPM: "));
    Serial.println(this->settings.stepsMaxRpm);
#endif  // USE_STEPPER_DRUM
    Serial.print(F("NVM Safety Temperature Threshold C: "));
    Serial.println(this->settings.maxSafeTempC);
    Serial.print(F("NVM PWM frequency Drum Hz: "));
    Serial.println(this->settings.pwmDrumHz);
    Serial.print(F("NVM PWM frequency Exhaust fan Hz: "));
    Serial.println(this->settings.pwmExhaustHz);
    Serial.print(F("NVM PWM frequency SSR Hz: "));
    Serial.println(this->settings.pwmSSRHz);
    Serial.println(F("---"));
}


/**
 * @brief Increment safety counter
 */
void EepromSettings::incSafetyCounter() {
    this->settings.counters.safetyTriggers++;
    this->markDirty();
}


/**
 * @brief Reset settings to default
 */
void EepromSettings::loadDefaults() {
    // load the defaults
    DEBUG(micros()); DEBUGLN(F(" Loading NVM Settings defaults"));
    memcpy_P(&settings, this->defaultSettings, sizeof(t_Settings));
    this->save();
    this->timer.reset();
}

/**
 * @brief save the eeprom container
 */
void EepromSettings::save() {
    DEBUG(micros()); DEBUGLN(F(" Saving NVM Settings"));
    this->settings.crc16 = calcCRC16((uint8_t *) &settings, offsetof(t_Settings, crc16));
    EEPROM.put(EEPROM_SETTINGS_ADDR, this->settings);
    isDirty = false;
}


/**
 * @brief mark the settings container dirty, prompting a save upon
 *        timer expiration
 */
void EepromSettings::markDirty() {
    this->isDirty = true;
    this->timer.reset();
}
