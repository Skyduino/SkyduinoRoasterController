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
    QuickPID::pMode   pMode;
    QuickPID::dMode   dMode;
    QuickPID::iAwMode iAwMode;
    float   kP;
    float   kI;
    float   kD;
} t_PidTune;

typedef struct {
    float     fanSPErrorC;
    float     cnsPrfErrorC;
    uint16_t  cycleTimeMS;
    uint8_t   chan;
    t_PidTune tuneNormal, tuneConserv, tuneFan;
} t_NvmPIDSettings;


typedef struct {
    uint16_t   stepsPerRevolution;
    uint8_t    stepsMaxRpm;

    int16_t    maxSafeTempC;
    uint16_t   pwmCoolHz;
    uint16_t   pwmDrumHz;
    uint16_t   pwmExhaustHz;
    uint16_t   pwmSSRHz;
    t_Counters counters;
    t_NvmPIDSettings pid;

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
        void loadDefaults(bool saveImmediatly);
        void save();
        bool skipWatchdog() { return _skipWdg; }

    private:
        TimerMS          timer = TimerMS(EEPROM_SAVE_TIME_MS); 
        bool             isDirty = false;
        const t_Settings *defaultSettings;
        bool             _skipWdg = false;
};

#endif  // __SW_EEPROM_SETTINGS_H
