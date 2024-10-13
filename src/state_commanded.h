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
        ControlHeat():
            ControlPWM(PIN_HEAT, PWM_FREQ_HEAT) {};
        ControlHeat(uint32_t pin=PIN_HEAT, uint32_t freq=PWM_FREQ_HEAT):
            ControlPWM(pin, freq) {};
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
        ControlDrum(
            uint16_t stepsPR = STEPPER_STEPS_PER_REV,
            uint8_t maxRPM = STEPPER_MAX_RPM,
            uint32_t pwmDRV8871 = PWM_FREQ_DRUM
            ):
                ControlPWM( PIN_STEPPER_STEP, 200 ),
                _steps_per_rev( stepsPR ),
                _max_rpm( maxRPM ),
                _drum(ControlPWM( PIN_DRUM, pwmDRV8871 ))
                {}
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
        ControlPWM   _drum;
        void virtual _setPWM(uint8_t value);
        void         _abortAction();
};
#endif // USE_STEPPER_DRUM


class PID_Control {
    public:
        PID_Control(EepromSettings *nvm, ControlHeat *heat, ControlPWM *vent):
            _nvm(nvm),
            _heat(heat),
            _vent(vent) {};
        bool begin();
        void loadProfile( uint8_t profileNum );
    
    protected:
        const EepromSettings    *_nvm;
        const ControlHeat       *_heat;
        const ControlPWM        *_vent;
        HardwareTimer           *_timer;
        bool                    isInitialized = false;
        float                   input = 0;
        float                   output = 0;
        float                   setp = 0;
        QuickPID::Control       _action = QuickPID::Control::manual;
        QuickPID                _pid = QuickPID(&input, &output, &setp);
};


class StateCommanded {
    public:
        StateCommanded(EepromSettings *nvm):
            heat(ControlHeat(PIN_HEAT, nvm->settings.pwmSSRHz)),
            vent(ControlPWM(PIN_EXHAUST, nvm->settings.pwmExhaustHz)),
#ifdef USE_STEPPER_DRUM
            drum(ControlDrum(
                nvm->settings.stepsPerRevolution,
                nvm->settings.stepsMaxRpm,
                nvm->settings.pwmDrumHz)),
#else  // USE_STEPPER_DRUM
            drum(ControlPWM(PIN_DRUM, nvm->settings.pwmDrumHz)),
#endif // USE_STEPPER_DRUM
            cool(ControlOnOff(PIN_COOL)),
            pid( PID_Control(nvm, &heat, &vent) ),
            _nvmSettings(nvm) {};
        ControlHeat heat;
        ControlPWM vent;
#ifdef USE_STEPPER_DRUM
        ControlDrum drum;
#else  // USE_STEPPER_DRUM
        ControlPWM drum;
#endif // USE_STEPPER_DRUM
        ControlOnOff cool;
        ControlBasic filter;
        PID_Control pid;

        void abort();
        bool begin();
        bool loopTick();
        void printState();
        void off();
        bool isArtisanIncontrol();
        void setControlToArtisan(bool value = true);

    protected:
        EepromSettings *_nvmSettings;
        bool _isArtisanInControl = false;
};


#endif // _SW_STATE_CMD_H
