#include <logging.h>

#include "skywalker_remote_comms.h"

typedef uint8_t (SWRemoteRx::* t_GetNewValueMethod)();

/**
 * @brief helper type to iterate through the new remote message and
 *        asses what has changed
 */
typedef struct {
    uint8_t *oldValue;
    t_GetNewValueMethod getNewValue;
    const __FlashStringHelper *valueName;
} t_RemoteWhatHasChanged;


SkywalkerRemoteComm::SkywalkerRemoteComm(State *state): _state(state) {
    this->_commTimer = new TimerMS(SKW_REMOTE_COMM_PERIOD_MS);
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
    if ( !(this->_commTimer->hasTicked()) ) return true;

    if ( isTxCycle ) {
        uint32_t skwrADC = this->_state->reported.getSkywalkerADC();
        this->_toRemote.setHighTempADC( skwrADC & 0xFFFF );
        this->_toRemote.setLowTempADC( skwrADC >> 16 );
        this->_toRemote.sendMessage();
    } else {
        // really care about remote control messages if TC4 is not in use
        if ( !(this->_state->commanded.isArtisanIncontrol()) ) {
            this->_handleRx();
        }
    }

    isTxCycle = !isTxCycle;
    this->_commTimer->reset();

    return true;
}


/**
 * @brief take care of receiving the commands from the remote
 */
void SkywalkerRemoteComm::_handleRx() {
    bool isSuccess = this->_fromRemote.getMessage();
    if ( !isSuccess ) {
        DEBUG(micros()); DEBUGLN(F(" Failed to received message from the remote"));
        return;
    }

    // we have received a control command from the remote,
    // let's check if there's anything
    t_RemoteWhatHasChanged valueHandlers[] = {
        { &_oldAirFan, &SWRemoteRx::getAirFan, F("Air Fan Speed") },
        { &_oldCoolingFan, &SWRemoteRx::getCoolingFan, F("Cooling Fan Speed") },
        { &_oldDrumSpeed, &SWRemoteRx::getDrumSpeed, F("Drum Speed") },
        { &_oldHeat, &SWRemoteRx::getHeat, F("Heater power") }
    };

    uint8_t count = sizeof(valueHandlers) / sizeof(valueHandlers[0]);
    for (uint8_t i = 0; i < count; i++) {
        t_RemoteWhatHasChanged *handler = &(valueHandlers[i]);

        uint8_t newValue = (_fromRemote.*handler->getNewValue) ();
        if ( newValue != *(handler->oldValue) ) {
            DEBUG(micros()); DEBUG(handler->valueName);
            DEBUG(F(" has changed to "));
            DEBUGLN(newValue);
            *(handler->oldValue) = newValue;
        }
    }
}
