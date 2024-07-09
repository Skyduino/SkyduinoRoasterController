#include "state.h"

class SafetyMonitor {
    public:
        SafetyMonitor(State *state): _state(state) {};
        bool loopTick();

    
    private:
        State *_state;
};