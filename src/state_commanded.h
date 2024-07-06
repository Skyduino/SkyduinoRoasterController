#ifndef _SW_STATE_CMD_PH
#define _SW_STATE_CMD_PH

#include <HardwareTimer.h>
#include "roaster.h"

class ControlBasic {
    public:
        ControlBasic();
        bool virtual begin() {return true;};
        void virtual off();
        void virtual on();
        uint8_t virtual get();
        void virtual set(uint8_t value);
        bool virtual loopTick();

    private:
        uint8_t         value;
    
    protected:
        void virtual _setAction(uint8_t value) {};
};

class ControlOnOff: public ControlBasic {
    public:
        ControlOnOff(uint8_t pin): pin(pin) {};
        bool begin();

    protected:
        const uint8_t   pin;
        void _setAction(uint8_t value);
};

class ControlPWM: public ControlOnOff {
    public:
        ControlPWM(uint8_t pin, uint32_t freq);
        bool begin();

    private:
        HardwareTimer*  timer;
        uint32_t        channel;
        uint32_t        freq;
    
    protected:
        void _setAction(uint8_t value);
};

class ControlHeat: public ControlPWM {
    public:
        ControlHeat(): ControlPWM(PIN_HEAT, PWM_FREQ_HEAT) {};
        bool begin();
        bool loopTick();

    protected:
        void _setAction(uint8_t value);

    private:
        bool isTransitioning = false;
        uint8_t oldValue = 0;
        ControlOnOff heatRelay = ControlOnOff(PIN_HEAT_RELAY);
};


class StateCommanded {
    public:
        ControlHeat heat;
        ControlPWM vent     = ControlPWM(PIN_EXHAUST, PWM_FREQ_EXHAUST);
        ControlPWM drum     = ControlPWM(PIN_DRUM, PWM_FREQ_DRUM);
        ControlOnOff cool   = ControlOnOff(PIN_COOL);
        ControlBasic filter;

        bool begin();
        bool loopTick();
        void printState();
};


#endif // _SW_STATE_CMD_H
