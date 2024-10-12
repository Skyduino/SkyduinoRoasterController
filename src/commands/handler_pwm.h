#ifndef __CMD_PWN_H
#define __CMD_PWN_H

#include "base.h"
#include "eeprom_settings.h"

class cmndPwm : public Command {
    public:
        cmndPwm(State *state);

    protected:
        void _doCommand( CmndParser *pars);
        void _setPwmDrum( int32_t value );
        void _setPwmExhaust( int32_t value );
        void _setPwmSSR( int32_t value );
};

#endif // __CMD_PWN_H