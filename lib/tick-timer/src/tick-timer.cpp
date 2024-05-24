#include <Arduino.h>
#include "tick-timer.h"


_Timer::_Timer(uint32_t delay, uint32_t (*tickFunc)()) {
    this->getTick = tickFunc;
    this->delay = delay;
    this->nextTick = this->getTick() + this->delay;
    this->ticked = false;
}

const bool _Timer::hasTicked() {
    if (this->ticked)
        return true;

    if (this->getTick() >= this->nextTick) {
        this->ticked = true;
        return true;
    }

    return false;
}

void _Timer::reset() {
    this->nextTick = this->getTick() + this->delay;
    this->ticked = false;
}