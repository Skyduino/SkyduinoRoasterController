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

    private:
        uint8_t         value;
        void virtual _setAction(uint8_t value) {};
};

class ControlOnOff: public ControlBasic {
    public:
        ControlOnOff(uint8_t pin): pin(pin) {};
        bool begin();

    protected:
        const uint8_t   pin;

    private:
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
        void _setAction(uint8_t value);
};


class StateCommanded {
    public:
        ControlPWM heat     = ControlPWM(PIN_HEAT, PWM_FREQ_HEAT);
        ControlPWM vent     = ControlPWM(PIN_EXHAUST, PWM_FREQ_EXHAUST);
        ControlPWM drum     = ControlPWM(PIN_DRUM, PWM_FREQ_DRUM);
        ControlOnOff cool   = ControlOnOff(PIN_COOL);
        ControlBasic filter;

        bool begin();
        bool loopTick();
        void printState();
};


#endif // _SW_STATE_CMD_H
