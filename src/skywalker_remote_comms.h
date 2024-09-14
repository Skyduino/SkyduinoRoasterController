#ifndef _SW_REMOTE_COMMS_H
#define _SW_REMOTE_COMMS_H

#include <skywalker-protocol.h>
#include <tick-timer.h>

#include <roaster.h>
#include <state.h>


class SkywalkerRemoteComm {
    public:
        SkywalkerRemoteComm(State *state);
        bool begin();
        bool loopTick();

    protected:
        State       *_state;
        TimerMS     *_commTimer;
        bool        isTxCycle      = false;
        SWRoasterTx _toRemote      = SWRoasterTx(PIN_DATA_TO_RMT);
        SWRemoteRx  _fromRemote    = SWRemoteRx(PIN_DATA_FROM_RMT);
        uint8_t     _oldAirFan     = 255;
        uint8_t     _oldCoolingFan = 255;
        uint8_t     _oldDrumSpeed  = 255;
        uint8_t     _oldHeat       = 255;

        void _handleRx();
        void _handleTx();
};

#endif //_SW_REMOTE_COMMS_H