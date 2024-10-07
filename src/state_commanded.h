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
        ControlOnOff(uint32_t pin): pin(pin) {};
        bool begin();

    protected:
        const uint32_t pin;
        void _setAction(uint8_t value);
};

class ControlPWM: public ControlOnOff {
    public:
        ControlPWM(uint32_t pin, uint32_t freq);
        bool begin();

    protected:
        HardwareTimer *timer;
        uint32_t      channel;
        uint32_t      freq;
        void          _setAction(uint8_t value);
        void virtual  _setPWM(uint8_t value);

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

#ifdef USE_STEPPER_DRUM
class ControlDrum : public ControlPWM {
    public:
        ControlDrum(): ControlPWM( PIN_STEPPER_STEP, 200 ),
            _steps_per_rev( STEPPER_STEPS_PER_REV ),
            _max_rpm( STEPPER_MAX_RPM ) {};
        bool     begin();
        uint32_t durationFromValue(uint8_t value);
        uint32_t frequencyFromValue(uint8_t value);
        void     setStepsPerRevolution(uint16_t steps);
        void     setMaxRPM(uint8_t rpm);

    protected:
        void _setAction(uint8_t value);
    
    private:
        uint16_t     _steps_per_rev;
        uint8_t      _max_rpm;
        ControlOnOff _enable = ControlOnOff( PIN_STEPPER_EN );
        ControlPWM   _drum   = ControlPWM( PIN_DRUM, PWM_FREQ_DRUM );
        void virtual _setPWM(uint8_t value);
        void         _abortAction();
};
#endif // USE_STEPPER_DRUM

class StateCommanded {
    public:
        ControlHeat heat;
        ControlPWM vent     = ControlPWM(PIN_EXHAUST, PWM_FREQ_EXHAUST);
#ifdef USE_STEPPER_DRUM
        ControlDrum drum;
#else  // USE_STEPPER_DRUM
        ControlPWM drum     = ControlPWM(PIN_DRUM, PWM_FREQ_DRUM);
#endif // USE_STEPPER_DRUM
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
