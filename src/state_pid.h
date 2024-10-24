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
        enum class FanMode: uint8_t { manual, automatic };
        PID_Control(EepromSettings *nvm, ControlHeat *heat, ControlPWM *vent);
        void abort();
        bool begin();
        bool activateProfile( uint8_t profileNum, bool isConservative=false );
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
        bool updateProfileNTuning( uint8_t profile, float kP, float kI, float kD );
        void addGetLogicalChantTempC( t_Cbk_getLogicalChanTempC cbk) { getLogicalChanTempC = cbk; };
        bool selectFanProfile( uint8_t profileNum );
        FanMode getFanMode() { return this->_fanMode; }
        void setFanMode( uint8_t mode ) { this->_fanMode = (FanMode) mode; }
        void setFanMode( FanMode mode ) { this->_fanMode = mode; }
        void setFanMin( uint8_t value );
        void enablePlot( bool value ) { this->_plotPid = value; }
    
    protected:
        EepromSettings      *_nvm;
        ControlHeat         *_heat;
        ControlPWM          *_vent;
        t_Cbk_getLogicalChanTempC getLogicalChanTempC = NULL;
        HardwareTimer       *_timer;
        State               _state = State::needsInit;
        FanMode             _fanMode = FanMode::manual;
        uint8_t             _fanMin = 0;
        bool                _isConservTuning = false;
        bool                _isFanPidActive = false;
        float               input = 0;
        float               output = 0;
        float               exhaustOutp = 0.0;
        float               setp = 0;
        uint32_t            _plotStart = 0;
        bool                _plotPid = false;
        QuickPID::Control   _action = QuickPID::Control::manual;
        QuickPID            _pid = QuickPID(&input, &output, &setp);
        QuickPID            _pidFan = QuickPID(&input, &exhaustOutp, &setp);
        void _compute();
        void _syncPidSettings();
        void _switchProfilesIfNeeded();
        void _plotPidTuner();
};

#endif // _SW_STATE_PID_H
