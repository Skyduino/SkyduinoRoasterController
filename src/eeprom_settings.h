#ifndef __SW_EEPROM_SETTINGS_H
#define __SW_EEPROM_SETTINGS_H

#include <avr/pgmspace.h>
#include <tick-timer.h>
#include <roaster.h>


typedef struct {
    uint32_t powerOnResets;
    uint32_t safetyTriggers;
} t_Counters;

typedef struct {
#ifdef USE_STEPPER_DRUM
    uint16_t   stepsPerRevolution;
    uint8_t    stepsMaxRpm;
#endif  // USE_STEPPER_DRUM

    uint16_t   maxSafeTempC;
    t_Counters counters;

    uint32_t   eepromMagic;
    uint16_t   crc16;
} t_Settings;


class EepromSettings {
    public:
        t_Settings settings;

        EepromSettings(const t_Settings *eeprom): defaultSettings(eeprom) {};
        void begin();
        void looptick();

    private:
        TimerMS          timer = TimerMS(EEPROM_SAVE_TIME_MS); 
        bool             isDirty = false;
        const t_Settings *defaultSettings;

        void save();
        void markDirty();
};

const t_Settings defaultSettings = {
#ifdef USE_STEPPER_DRUM
    STEPPER_STEPS_PER_REV, // stepsPerRevolution;
    STEPPER_MAX_RPM, // Max RPM for stepper drum driver
#endif  // USE_STEPPER_DRUM
    MAX_SAFE_TEMP_C,
    // Counters
    0, // power on resets
    0, // safetyTriggers
    EEPROM_SETTINGS_MAGIC, // EEPROM MAGIC number
    0 // CRC
};

#endif  // __SW_EEPROM_SETTINGS_H
