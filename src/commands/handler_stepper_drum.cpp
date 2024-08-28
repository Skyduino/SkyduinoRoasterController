#include "state.h"
#include "handler_stepper_drum.h"

#define CMD_STEPS  "STPR"
#define CMD_MAXRPM "MXRPM"
#define MIN_RPM    10
#define MAX_RPM    120

cmndSteps::cmndSteps(State *state):
    ControlCommand(CMD_STEPS, state) {
}


void cmndSteps::_handleValue(int32_t value) {
    if ( value > 0 ) {
        state->commanded.drum.setStepsPerRevolution( value );
        Serial.print(F("Setting ")); Serial.print(value);
        Serial.println(F(" steps per revolution"));
    }
}


cmndMaxRPM::cmndMaxRPM(State *state):
    ControlCommand(CMD_MAXRPM, state) {
}


void cmndMaxRPM::_handleValue(int32_t value) {
    if ( value < MIN_RPM ) {
        Serial.print(F("Ignoring Minimum rpm, it's below ")); Serial.println(MIN_RPM);
        return;
    } 
    if ( value > MAX_RPM ) {
        Serial.print(F("Ignoring Maximum rpm, it's above ")); Serial.println(MAX_RPM);
        return;
    }

    state->commanded.drum.setMaxRPM( value );
    Serial.print(F("Setting MAX rpm to ")); Serial.println( value );
}
