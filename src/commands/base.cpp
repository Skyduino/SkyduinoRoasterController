#include "base.h"

Command::Command(const char* cmdName): CmndBase(cmdName) {
}

bool Command::begin(void) {
    return true;
}