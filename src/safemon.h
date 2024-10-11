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
        SafetyMonitor(State *state,
                      int16_t maxTempC=MAX_SAFE_TEMP_C,
                      callback_t callback = NULL):
            _roasterState(state),
            _maxTemp(maxTempC),
            incCounter(callback) {};
        bool begin();
        bool loopTick();
        void triggerSafetyAction();
    
    private:
        State         *_roasterState;
        MonitorState  _monitorState = NORMAL;
        TimerMS       *timer;
        const int16_t _maxTemp;
        bool          isInitialized = false;

        bool _isTempExceedingC(float threshold);
        void _transitionToNormal();
        void _handleNormal();
        void _transitionToPreArm();
        void _handlePreArm();
        void _transitionToArmed();
        void _handleArmed();
        void _transitionToTriggered();
        void _handleTriggered();
        callback_t incCounter;
};