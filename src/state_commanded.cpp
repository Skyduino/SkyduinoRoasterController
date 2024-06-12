#include "state.h"
#include "state_commanded.h"

bool StateCommanded::begin() {
    return true;
}

bool StateCommanded::loopTick() {
    return true;
}

void ControlBasic::on() {
    this->set(100);
}

void ControlBasic::off() {
    this->set(0);
}

uint8_t ControlBasic::get() {
    return this->value;
}

void ControlBasic::set(uint8_t value) {
    this->value = value;
    this->_setAction(value);
}
