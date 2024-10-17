#ifndef _SW_STATE_PID_H
#define _SW_STATE_PID_H

#include <HardwareTimer.h>
#include <QuickPID.h>
#include <sTune.h>

#include "roaster.h"
#include "eeprom_settings.h"
#include "state_commanded.h"


using t_Cbk_getLogicalChanTempC = std::function< float( uint8_t ) >;

typedef struct {
    uint32_t settleTimeSec = 10;
    uint32_t testTimeSec = 500;  // runPid interval = testTimeSec / samples
    const uint16_t samples = 500;
    const float inputSpan = 200;
    const float outputSpan = 1000;
    float outputStart = 0;
    float outputStep = 50;
    float tempLimit = 150;
    uint8_t debounce = 1;
    bool startup = true;
} t_STuneSettings;


class PID_Control {
    public:
        PID_Control(EepromSettings *nvm, ControlHeat *heat, ControlPWM *vent):
            _nvm(nvm),
            _heat(heat),
            _vent(vent) {};
        void abort();
        bool begin();
        bool activateProfile( uint8_t profileNum );
        bool isOn();
        void turnOff();
        void turnOn();
        void startAutotune();
        void stopAutotune();
        bool updateAWMode( uint8_t mode );
        bool updateChan( uint8_t chan );
        bool updateCycleTimeMs( uint32_t ct );
        bool updateDMode( uint8_t mode );
        bool updatePMode( uint8_t mode );
        void updateSetPointC( float setPointC );
        float getSetPoint() { return this->setp; };
        void updateTuning( float kP, float kI, float kD );
        void addGetLogicalChantTempC( t_Cbk_getLogicalChanTempC cbk) { getLogicalChanTempC = cbk; };
    
    protected:
        EepromSettings      *_nvm;
        ControlHeat         *_heat;
        ControlPWM          *_vent;
        t_Cbk_getLogicalChanTempC getLogicalChanTempC = NULL;
        HardwareTimer       *_timer;

        bool                isInitialized = false;
        bool                _isAborted = false;
        float               input = 0;
        float               output = 0;
        float               setp = 0;
        QuickPID::Control   _action = QuickPID::Control::manual;
        QuickPID            _pid = QuickPID(&input, &output, &setp);
        void _compute();
        void _syncPidSettings();
};


class Autotuner {
    public:
        enum class State: uint8_t { idle, running, done };

        Autotuner(
            EepromSettings *nvm,
            HardwareTimer *timer,
            t_Cbk_getLogicalChanTempC getTempC
        ):
            _nvm( nvm ),
            _timer( timer ),
            getLogicalChanTempC( getTempC ) {};
        void begin();
        void start();
        void stop();
        float getTempReadingC();

        State getState() { return this->_state; };

    protected:
        State               _state = State::idle;
        EepromSettings      *_nvm;
        HardwareTimer       *_timer;
        t_Cbk_getLogicalChanTempC getLogicalChanTempC = NULL;

        float               input = 0;
        float               output = 0;
        float               optimumOutput = 0;
        float               setp = 0;
        t_STuneSettings     stnStngs = {
            10,  // settleTimeSec
            500, // testTimeSec, runPid interval = testTimeSec / samples
            500, // samples
            200, // inputSpan
            100, // outputSpan
            0,   // outputStart
            50,  // outputStep
            250, // tempLimit
            0,   // debounce
            true // startup
        };

        sTune               tuner = sTune( &input, &output, tuner.ZN_PID, tuner.directIP, tuner.printOFF);
        QuickPID            _pid = QuickPID(&input, &output, &setp);

        void _tuneLoop();
};

#endif // _SW_STATE_PID_H
