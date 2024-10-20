#ifndef _SW_STATE_PID_H
#define _SW_STATE_PID_H

#include <HardwareTimer.h>

#include "roaster.h"
#include "eeprom_settings.h"
#include "state_commanded.h"


using t_Cbk_getLogicalChanTempC = std::function< float( uint8_t ) >;

class PID_Control {
    public:
        enum class State: uint8_t { needsInit, off, on, autotune, aborted };
        PID_Control(EepromSettings *nvm, ControlHeat *heat, ControlPWM *vent):
            _nvm(nvm),
            _heat(heat),
            _vent(vent) {};
        void abort();
        bool begin();
        bool activateProfile( uint8_t profileNum, bool isConservative=false);
        State getState() { return this->_state; };
        float getTempReadingC();
        bool isOn();
        void turnOff();
        void turnOn();
        void print();
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
        State               _state = State::needsInit;
        bool                _isConservTuning = false;
        float               input = 0;
        float               output = 0;
        float               setp = 0;
        QuickPID::Control   _action = QuickPID::Control::manual;
        QuickPID            _pid = QuickPID(&input, &output, &setp);
        void _compute();
        void _syncPidSettings();
        void _switchProfilesIfNeeded();
};

#endif // _SW_STATE_PID_H
