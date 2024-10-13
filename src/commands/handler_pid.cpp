#include <cstdlib>

#include "handler_pid.h"

#define CMD_PID "PID"
#define SUBCMD_AWMODE "AWMODE"
#define SUBCMD_CHAN   "CHAN"
#define SUBCMD_CT     "CT"
#define SUBCMD_DMODE  "DMODE"
#define SUBCMD_OFF    "OFF"
#define SUBCMD_ON     "ON"
#define SUBCMD_PMODE  "PMODE"
#define SUBCMD_SV     "SV"
#define SUBCMD_T      "T"


typedef struct {
    const char *subCmdName;
    void (cmndPid::*subCmdHandler) ( CmndParser *pars ); 
} t_SubCommand;


cmndPid::cmndPid(State *state):
    Command( CMD_PID, state ) {
}

void cmndPid::_doCommand(CmndParser *pars) {
    t_SubCommand cmds[] = {
        { SUBCMD_AWMODE, &cmndPid::_handleAwMode },
        { SUBCMD_CHAN, &cmndPid::_handleChan },
        { SUBCMD_CT, &cmndPid::_handleCT },
        { SUBCMD_DMODE, &cmndPid::_handleDMode },
        { SUBCMD_OFF, &cmndPid::_handleOff },
        { SUBCMD_ON, &cmndPid::_handleOn },
        { SUBCMD_PMODE, &cmndPid::_handlePMode },
        { SUBCMD_SV, &cmndPid::_handleSV },
        { SUBCMD_T, &cmndPid::_handleT },
        { NULL, NULL }
    };

    for ( uint8_t i = 0; NULL != cmds[i].subCmdName; i++ ) {
        if ( 0 == strcmp( pars->paramStr(1), cmds[i].subCmdName ) ) {
            (this->*cmds[i].subCmdHandler)( pars );
            break;
        }
    }
}


/**
 * @brief Handle PID;AWMOD;i command to update Anti Windup Mode,
 *        where i is 0 -- set anti-windup to Condition, 1 -- set aw to Clamp,
 *        2 - turn off the anti windup
 */
void cmndPid::_handleAwMode(CmndParser *pars) {
    Serial.println(F("handling AW Mode"));
}


/**
 * @brief Handle PID;CHAN;i command to update the input channel
 */
void cmndPid::_handleChan(CmndParser *pars) {
}


/**
 * @brief Handle PID;CT;ssss command, to change the PID cycle time
 */
void cmndPid::_handleCT(CmndParser *pars) {
}


/**
 * @brief Handle PID;DMOD;i command to update the D-term mode
 *        where i is 0 -- dterm on error, 1 -- dterm on measurement
 */
void cmndPid::_handleDMode(CmndParser *pars) {
}


/**
 * @brief Handle PID;OFF command to deactivate the PID
 */
void cmndPid::_handleOff(CmndParser *pars) {
}


/**
 * @brief Handle PID;ON command to activate the PID
 */
void cmndPid::_handleOn(CmndParser *pars) {
}


/**
 * @brief Handle PID;PMOD;i command to update the P-term mode
 *        where i is 0 -- pterm on error, 1 -- pterm on measurement,
 *        2 -- pterm on both error & measurement
 */
void cmndPid::_handlePMode(CmndParser *pars) {
}


/**
 * @brief Handle PID;SV;xxx command to change the setpoint
 */
void cmndPid::_handleSV(CmndParser *pars) {
}


/**
 * @brief Handle PID;T;ppp;iii;ddd command to change PID tuning parameters
 */
void cmndPid::_handleT(CmndParser *pars) {
}
