#ifndef __CMD_PID_H
#define __CMD_PID_H

#include "base.h"

class cmndPid : public Command {
    public:
        cmndPid(State *state);

    protected:
        void _doCommand( CmndParser *pars );
        void _handleAwMode( CmndParser *pars );
        void _handleChan( CmndParser *pars );
        void _handleChngPrfl( CmndParser *pars );
        void _handleConsrvPrfl( CmndParser *pars );
        void _handleCT( CmndParser *pars );
        void _handleDMode( CmndParser *pars );
        void _handleOff( CmndParser *pars );
        void _handleOn( CmndParser *pars );
        void _handlePMode( CmndParser *pars );
        void _handleSV( CmndParser *pars );
        void _handleT( CmndParser *pars );
        void _handleTPOM( CmndParser *pars );
        void __handlePidTune( CmndParser *pars, QuickPID::pMode pMode );
};

#endif // __CMD_PID_H