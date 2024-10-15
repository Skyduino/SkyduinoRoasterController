#ifndef __SW_EEPROM_SETTINGS_H
#define __SW_EEPROM_SETTINGS_H

#include <avr/pgmspace.h>
#include <QuickPID.h>
#include <tick-timer.h>
#include <roaster.h>


typedef struct {
    uint32_t powerOnResets;
    uint32_t watchdogResets;
    uint32_t softResets;
    uint32_t safetyTriggers;
} t_Counters;


typedef struct {
    float   kP;
    float   kI;
    float   kD;
    QuickPID::pMode   pMode;
    QuickPID::dMode   dMode;
    QuickPID::iAwMode iAwMode;
    uint8_t chan;
    uint32_t cycleTimeMS;
} t_NvmPIDSettings;


typedef struct {
#ifdef USE_STEPPER_DRUM
    uint16_t   stepsPerRevolution;
    uint8_t    stepsMaxRpm;
#endif  // USE_STEPPER_DRUM

    int16_t    maxSafeTempC;
    uint16_t   pwmCoolHz;
    uint16_t   pwmDrumHz;
    uint16_t   pwmExhaustHz;
    uint16_t   pwmSSRHz;
    uint16_t   pwmLedHz;
    t_Counters counters;
    uint8_t    pidCurrentProfile;
    t_NvmPIDSettings pidProfiles[PID_NUM_PROFILES];

    uint32_t   eepromMagic;
    uint16_t   crc16;
} t_Settings;


class EepromSettings {
    public:
        t_Settings settings;

        EepromSettings(const t_Settings *eeprom);
        bool loopTick();
        void markDirty();
        void print();
        void incSafetyCounter();
        void loadDefaults();

    private:
        TimerMS          timer = TimerMS(EEPROM_SAVE_TIME_MS); 
        bool             isDirty = false;
        const t_Settings *defaultSettings;

        void save();
};

#endif  // __SW_EEPROM_SETTINGS_H
