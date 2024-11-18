#include <CRC16.h>
#include <CRC.h>
#include <EEPROM.h>
#include <logging.h>
#ifndef __DEBUG__
#include <IWatchdog.h>
#endif
#include <status-led.h>

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
    Serial.println(F("[NVM PID]"));

    const char pidHDR[] PROGMEM   = "Conserv. Prof. Gap C=%f, Fan PID gap C=%f, Chan=%d, Cycle Time=%u(ms)";
    const char pidTmplt[] PROGMEM = " profile: kP=%f, kI=%f, kD=%f, P-mode=%d, D-mode=%d, I-Aw-mode=%d";
    char buf[sizeof(pidTmplt) * 2];

    t_NvmPIDSettings *pid = &(settings.pid);
    snprintf_P(buf, sizeof(buf)-1, pidHDR,
        pid->cnsPrfErrorC,
        pid->fanSPErrorC,
        pid->chan,
        pid->cycleTimeMS
    );
    Serial.println( buf );

    struct {
        const __FlashStringHelper *name;
        t_PidTune                 *tune;
    } pidTunes[] = {
        { F(" Normal"), &(pid->tuneNormal) },
        { F(" Conservative"), &(pid->tuneConserv) },
        { F(" Fan"), &(pid->tuneFan) },
        { NULL, NULL }
    };

    for ( uint8_t i = 0; NULL != pidTunes[i].name; i++ ) {
        buf[sizeof(buf)-1] = 0;
        Serial.print( pidTunes[i].name );
        snprintf_P(buf, sizeof(buf)-1, pidTmplt,
            pidTunes[i].tune->kP,
            pidTunes[i].tune->kI,
            pidTunes[i].tune->kD,
            (uint8_t) pidTunes[i].tune->pMode,
            (uint8_t) pidTunes[i].tune->dMode,
            (uint8_t) pidTunes[i].tune->iAwMode
        );
        Serial.println(buf);
    }
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
#ifndef __DEBUG__
    IWatchdog.reload();
#endif
    memcpy_P(&settings, this->defaultSettings, sizeof(t_Settings));
    this->save();
    this->timer.reset();
}

/**
 * @brief save the eeprom container
 */
void EepromSettings::save() {
    this->settings.crc16 = calcCRC16((uint8_t *) &settings, offsetof(t_Settings, crc16));
#ifndef __DEBUG__
    IWatchdog.set( 30*1000*1000 );
    IWatchdog.reload();
#endif
    StatusLed.turnOn();
    EEPROM.put(EEPROM_SETTINGS_ADDR, this->settings);
    StatusLed.turnOff();
    isDirty = false;
#ifndef __DEBUG__
    IWatchdog.set( WATCHDOG_TIMEOUT_MS * 1000 );
    IWatchdog.reload();
#endif
}


/**
 * @brief mark the settings container dirty, prompting a save upon
 *        timer expiration
 */
void EepromSettings::markDirty() {
    this->isDirty = true;
    this->timer.reset();
}
