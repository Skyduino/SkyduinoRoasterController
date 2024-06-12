#ifndef _SW_STATE_CMD_PH
#define _SW_STATE_CMD_PH


class ControlBasic {
    public:
        ControlBasic(uint8_t pin);
        bool virtual begin() {return true;};
        void virtual off();
        void virtual on();
        uint8_t virtual get();
        void virtual set(uint8_t value);
    private:
        uint8_t         value;
        const uint8_t   pin;

        void virtual _setAction(uint8_t value) {};
};

class ControlOnOff: public ControlBasic {
    public:
        bool begin();

    private:
        void setupPin();
        void _setAction(uint8_t value);
};

class ControlPWM: public ControlOnOff {
    public:
        ControlPWM(uint8_t pin, uint32_t freq);
        bool begin();
        void set(uint8_t value);
    private:
        HardwareTimer*  timer;
        uint32_t        channel;
        uint32_t        freq;
        void virtual    setPWM(uint32_t duty);
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
