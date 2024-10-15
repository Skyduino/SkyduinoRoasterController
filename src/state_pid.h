#ifndef _SW_STATE_PID_H
#define _SW_STATE_PID_H

#include <HardwareTimer.h>

#include "roaster.h"
#include "eeprom_settings.h"
#include "state_commanded.h"


using t_Cbk_getLogicalChanTempC = std::function< float( uint8_t ) >;

class PID_Control {
    public:
        PID_Control(EepromSettings *nvm, ControlHeat *heat, ControlPWM *vent):
            _nvm(nvm),
            _heat(heat),
            _vent(vent) {};
        bool begin();
        void activateProfile( uint8_t profileNum );
        bool isOn();
        void turnOff();
        void turnOn();
        void updateChan( uint8_t chan );
        void updateCycleTimeMs( uint32_t ct );
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
        float               input = 0;
        float               output = 0;
        float               setp = 0;
        QuickPID::Control   _action = QuickPID::Control::manual;
        QuickPID            _pid = QuickPID(&input, &output, &setp);
        void _compute();
        void _syncPidSettings();
};

#endif // _SW_STATE_PID_H
