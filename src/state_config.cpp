#include "state.h"


uint8_t Config::getChanMapping(uint8_t idx) {
    return chanMapping[idx];
}

void Config::setChanMapping(uint8_t idx, uint8_t mapping) {
    this->chanMapping[idx] = mapping;
}
