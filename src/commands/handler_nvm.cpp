#include <cstdlib>
#include <logging.h>

#include "handler_nvm.h"

#define CMD_NVM       "NVM"
#define SUBCMD_SAVE   "SAVE"


typedef struct {
    const char *subCmdName;
    void (cmndNvm::*subCmdHandler) ( CmndParser *pars );
} t_SubCommand;


cmndNvm::cmndNvm(State *state):
    Command( CMD_NVM, state ) {
}

void cmndNvm::_doCommand(CmndParser *pars) {
    // Order of the commands does matter, shorter commands are checked last
    t_SubCommand cmds[] = {
        { SUBCMD_SAVE, &cmndNvm::_handleSave },
        { NULL, NULL }
    };

    for ( uint8_t i = 0; NULL != cmds[i].subCmdName; i++ ) {
        if ( 0 == strncmp( pars->paramStr(1), cmds[i].subCmdName, strlen(cmds[i].subCmdName) ) ) {
            (this->*cmds[i].subCmdHandler)( pars );
            break;
        }
    }
}


/**
 * @brief Handle NVM;SAVE command to store setting in NVRAM
 */
void cmndNvm::_handleSave(CmndParser *pars) {
    if ( 2 != pars->nTokens() ) return;

    uint32_t start = millis();
    this->state->nvmSettings->save();
    float durS = ( millis() - start ) / 1000.0f;
    Serial.print(F("# Saved NVM in "));
    Serial.print( durS );
    Serial.println(F("s"));
}
