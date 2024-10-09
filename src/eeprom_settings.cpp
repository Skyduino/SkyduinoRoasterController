#include <CRC16.h>
#include <CRC.h>
#include <EEPROM.h>
#include <logging.h>

#include "eeprom_settings.h"


/**
 * @brief initialize the Eeprom settings container. Load from eeprom
 *        and initialize the defaults if eeprom data is garbage
 */ 
bool EepromSettings::begin() {
    EEPROM.get(EEPROM_SETTINGS_ADDR, settings);
    uint16_t crc = calcCRC16((uint8_t *) &settings, offsetof(t_Settings, crc16));
    if ( (settings.crc16 != crc) || (settings.eepromMagic != EEPROM_SETTINGS_MAGIC) )
    {
        // load the defaults
        DEBUG(micros()); DEBUGLN(F(" Loading NVM Settings defaults"));
        memcpy_P(&settings, this->defaultSettings, sizeof(t_Settings));
        this->save();
    }

    return true;
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
