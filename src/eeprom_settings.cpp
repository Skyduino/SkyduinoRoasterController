#include <CRC16.h>
#include <CRC.h>
#include <EEPROM.h>

#include "eeprom_settings.h"


/**
 * @brief initialize the Eeprom settings container. Load from eeprom
 *        and initialize the defaults if eeprom data is garbage
 */ 
void EepromSettings::begin() {
    EEPROM.get(EEPROM_SETTINGS_ADDR, settings);
    uint16_t crc = calcCRC16((uint8_t *) &settings, sizeof(t_Settings));
    if ( (settings.crc16 != crc) || (settings.eepromMagic != EEPROM_SETTINGS_MAGIC) )
    {
        // load the defaults
        memcpy_P(&settings, this->defaultSettings, sizeof(t_Settings));
        this->save();
    }
}


/**
 * @brief check if the timer is fired and is the buffer is marked as dirty
 */
void EepromSettings::looptick() {
    if ( isDirty && this->timer.hasTicked() ) {
        this->save();
        this->timer.reset();
    }
}


/**
 * @brief save the eeprom container
 */
void EepromSettings::save() {
    this->settings.crc16 = calcCRC16((uint8_t *) &settings, sizeof(t_Settings));
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
