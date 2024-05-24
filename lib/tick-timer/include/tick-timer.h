#ifndef _TICK_TIMER_H
#define _TICK_TIMER_H


#include <Arduino.h>
#include <stdint.h>


class _Timer {
    private:
        uint32_t nextTick;
        uint32_t delay;
        uint32_t (*getTick)();
        bool ticked;
    public:
        _Timer(uint32_t delay, uint32_t (*tickFunc)());
        const bool hasTicked();
        void reset();
};

class TimerMS: public _Timer {
    public:
        TimerMS(uint32_t delay): _Timer(delay, millis) {};
};

class TimerUS: public _Timer {
    public:
        TimerUS(uint32_t delay): _Timer(delay, micros) {};
};


#endif