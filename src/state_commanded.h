#ifndef _SW_STATE_CMD_PH
#define _SW_STATE_CMD_PH

#include <HardwareTimer.h>

class ControlBasic {
    public:
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
        uint8_t heat;
        uint8_t vent;
        uint8_t cool;
        uint8_t filter;
        uint8_t drum;

        StateCommanded();
        bool begin();
        bool loopTick();
};


#endif // _SW_STATE_CMD_H
