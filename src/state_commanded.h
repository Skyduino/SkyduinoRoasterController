#ifndef _SW_STATE_CMD_PH
#define _SW_STATE_CMD_PH

#include <HardwareTimer.h>
#include <tick-timer.h>

#include "roaster.h"

// delay between SSR and heat relay transitions
#define CONTROL_HEAT_SSR_RELAY_DELAY_MS 11U

class ControlBasic {
    public:
        ControlBasic();
        bool virtual begin() {return true;};
        void virtual off();
        void virtual on();
        bool virtual isOn();
        uint8_t virtual get();
        void virtual set(uint8_t value);
        bool virtual loopTick();
        void virtual abort();

    private:
        uint8_t         value;
    
    protected:
        bool _isAborted = false;
        void virtual _setAction(uint8_t value) {};
        void virtual _abortAction();

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
        TimerMS *transitionTimer;
        ControlOnOff heatRelay = ControlOnOff(PIN_HEAT_RELAY);
        void _abortAction();
};


class StateCommanded {
    public:
        ControlHeat heat;
        ControlPWM vent     = ControlPWM(PIN_EXHAUST, PWM_FREQ_EXHAUST);
        ControlPWM drum     = ControlPWM(PIN_DRUM, PWM_FREQ_DRUM);
        ControlOnOff cool   = ControlOnOff(PIN_COOL);
        ControlBasic filter;

        void abort();
        bool begin();
        bool loopTick();
        void printState();
        void off();
        bool isArtisanIncontrol();
        void setControlToArtisan(bool value = true);

    protected:
        bool _isArtisanInControl = false;
};


#endif // _SW_STATE_CMD_H
