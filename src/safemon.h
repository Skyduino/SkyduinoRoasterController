#include <tick-timer.h>

#include "state.h"

class SafetyMonitor {
    public:
        SafetyMonitor(State *state): _state(state) {};
        bool begin();
        bool loopTick();
        void triggerSafetyAction();
    
    private:
        State   *_state;
        TimerMS *timer;
        bool    isInitialized = false;
        unsigned int exceedCounter = 0;

        bool _isTempExceeded();
};