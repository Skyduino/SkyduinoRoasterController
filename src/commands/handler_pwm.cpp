#include <cstdlib>

#include "handler_pwm.h"

#define CMD_PWM "PWM"
#define MIN_PWM 1
#define MAX_PWM 30000


cmndPwm::cmndPwm(State *state):
    Command( CMD_PWM, state ) {
}


void cmndPwm::_doCommand(CmndParser *pars) {
    int32_t value = atoi( pars->paramStr(2) );
    if (( value < MIN_PWM ) || ( value > MAX_PWM )) return;

    if ( strcmp( pars->paramStr(1), "DRUM" ) == 0 ) {
        this->_setPwmDrum( value );
    } else if ( strcmp( pars->paramStr(1), "EXHAUST" ) == 0 ) {
        this->_setPwmExhaust( value );
    } else if ( strcmp( pars->paramStr(1), "SSR" ) == 0 ) {
        this->_setPwmSSR( value );
    }
}


/**
 * @brief Set PWM frequency for the Drum control
 */
void cmndPwm::_setPwmDrum( int32_t value ) {
    state->nvmSettings->settings.pwmDrumHz = value;
    state->nvmSettings->markDirty();
    Serial.print(F("Setting Drum PWM freq to "));
    Serial.println(value);
}


/**
 * @brief Set PWM frequency for the Exhaust fan
 */
void cmndPwm::_setPwmExhaust( int32_t value ) {
    state->nvmSettings->settings.pwmExhaustHz = value;
    state->nvmSettings->markDirty();
    Serial.print(F("Setting Exhaust FAN PWM freq to "));
    Serial.println(value);
}


/**
 * @brief Set PWM frequency for SSR
 */
void cmndPwm::_setPwmSSR( int32_t value ) {
    if ( value > 120 ) return;

    state->nvmSettings->settings.pwmSSRHz = value;
    state->nvmSettings->markDirty();
    Serial.print(F("Setting SSR PWM freq to "));
    Serial.println(value);
}