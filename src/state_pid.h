#ifndef _SW_STATE_PID_H
#define _SW_STATE_PID_H

#include <HardwareTimer.h>

#include "roaster.h"
#include "state_commanded.h"


class PID_Control {
    public:
        PID_Control(EepromSettings *nvm, ControlHeat *heat, ControlPWM *vent, std::function<float(uint8_t)> getLogicalChT):
            _nvm(nvm),
            _heat(heat),
            _vent(vent),
            getLogicalChanTemp(getLogicalChT) {};
        bool begin();
        void loadProfile( uint8_t profileNum );
        bool isOn();
        void turnOff();
        void turnOn();
        void updateCycleTimeMs( uint32_t ct );
        void updateSetPointC( float setPointC );
        float getSetPoint() { return this->setp; };
        void updateTuning( float kP, float kI, float kD );
    
    protected:
        EepromSettings      *_nvm;
        ControlHeat         *_heat;
        ControlPWM          *_vent;
        std::function<float(uint8_t)> getLogicalChanTemp;
        HardwareTimer       *_timer;
        bool                isInitialized = false;
        float               input = 0;
        float               output = 0;
        float               setp = 0;
        QuickPID::Control   _action = QuickPID::Control::manual;
        QuickPID            _pid = QuickPID(&input, &output, &setp);
        void _compute();
};

#endif // _SW_STATE_PID_H
