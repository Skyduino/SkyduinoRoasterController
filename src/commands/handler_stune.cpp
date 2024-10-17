#include <cstdlib>
#include <logging.h>

#include "handler_stune.h"

#define CMD_STUNE     "STUNE"
#define SUBCMD_START  "START"
#define SUBCMD_STOP   "STOP"


typedef struct {
    const char PROGMEM *subCmdName;
    void (cmndSTune::*subCmdHandler) ( CmndParser *pars ); 
} t_SubCommand;


cmndSTune::cmndSTune(State *state):
    Command( CMD_STUNE, state ) {
}


/**
 * @brief Invoke a subcommand handler
 */
void cmndSTune::_doCommand(CmndParser *pars) {
    t_SubCommand cmds[] = {
        { SUBCMD_START, &cmndSTune::_handleStart },
        { SUBCMD_STOP, &cmndSTune::_handleStop },
        { NULL, NULL }
    };

    for ( uint8_t i = 0; NULL != cmds[i].subCmdName; i++ ) {
        if ( 0 == strcmp_PF( pars->paramStr(1), cmds[i].subCmdName ) ) {
            (this->*cmds[i].subCmdHandler)( pars );
            break;
        }
    }
}


/**
 * @brief Handle STUNE;START command to start the Autotuner
 */
void cmndSTune::_handleStart(CmndParser *pars) {
    if ( 2 != pars->nTokens() ) return;

    Serial.println(F("# Starting the auto tuner"));
    this->state->pid.startAutotune();
}


/**
 * @brief Handle STUNE;STOP command to stop the Autotuner
 */
void cmndSTune::_handleStop(CmndParser *pars) {
    if ( 2 != pars->nTokens() ) return;

    Serial.println(F("# Stopping the auto tuner"));
    this->state->pid.stopAutotune();
}