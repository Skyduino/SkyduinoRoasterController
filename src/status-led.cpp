#include "status-led.h"

LEDClass StatusLed = LEDClass( PIN_LED );


/**
 * @brief Initialize LED instance
 */
void LEDClass::begin() {
    pinMode( _pin, OUTPUT );
    digitalWrite( _pin, LOW );
    this->_isOn = false;
}


/**
 * @brief turn the led off
 */
void LEDClass::turnOff() {
    digitalWrite( _pin, LOW );
    this->_isOn = false;
}


/**
 * @brief turn the led on
 */
void LEDClass::turnOn() {
    digitalWrite( _pin, HIGH );
    this->_isOn = true;
}

/**
 * @brief Toggle the LED
 */
void LEDClass::toggle() {
    if ( this->_isOn ) this->turnOff(); else this->turnOn();
}