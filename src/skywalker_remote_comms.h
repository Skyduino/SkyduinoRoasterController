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
        bool        isTxCycle = false;
        SWRoasterTx _toRemote   = SWRoasterTx(PIN_DATA_TO_RMT);
        SWRemoteRx  _fromRemote = SWRemoteRx(PIN_DATA_FROM_RMT);
};

#endif //_SW_REMOTE_COMMS_H