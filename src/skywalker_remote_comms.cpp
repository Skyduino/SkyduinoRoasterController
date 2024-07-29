#include "skywalker_remote_comms.h"

SkywalkerRemoteComm::SkywalkerRemoteComm(State *state): _state(state) {
}


/**
 * @brief Initialize communication with the remote;
 */
bool SkywalkerRemoteComm::begin() {
    this->_toRemote.begin();
    this->_fromRemote.begin();

    return true;
}


/**
 * @brief handle loop iteration. Communicate with the remote if the timer is up.
 *        Cycle between data transmit and data receive.
 */
bool SkywalkerRemoteComm::loopTick() {
    if ( isTxCycle ) {
        this->_toRemote.loopTick();
    } else {
        this->_fromRemote.loopTick();
    }

    isTxCycle = !isTxCycle;

    return true;
}
