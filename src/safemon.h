#include <tick-timer.h>

#include "state.h"

enum MonitorState {
    NORMAL,
    PRE_ARM,
    ARMED,
    TRIGGERED
};


class SafetyMonitor {
    public:
        SafetyMonitor(State *state): _roasterState(state) {};
        bool begin();
        bool loopTick();
        void triggerSafetyAction();
    
    private:
        State        *_roasterState;
        MonitorState _monitorState = NORMAL;
        TimerMS      *timer;
        bool         isInitialized = false;

        bool _isTempExceedingC(float threshold);
        void _transitionToNormal();
        void _handleNormal();
        void _transitionToPreArm();
        void _handlePreArm();
        void _transitionToArmed();
        void _handleArmed();
        void _transitionToTriggered();
        void _handleTriggered();
};