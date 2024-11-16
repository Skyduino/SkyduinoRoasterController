#ifndef _SW_LED_H
#define _SW_LED_H

#include <Arduino.h>
#include <roaster.h>

#include "state_commanded.h"


/**
 * @brief Status LED Class
 *
 */
class LEDClass {
    public:
        LEDClass( uint32_t pin = PIN_LED ): _pin( pin ) { begin(); }
        void begin();
        void turnOff();
        void turnOn();
        void toggle();
    private:
        const uint32_t _pin;
        bool _isOn = false;
};

extern LEDClass StatusLed;
#endif  // _SW_LED_H