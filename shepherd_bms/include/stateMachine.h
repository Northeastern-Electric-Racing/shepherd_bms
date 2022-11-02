#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

#include "states.h"

class StateMachine
{
    //Maybe each state should access the same state machine hmmm
    private:
        BaseState currentState;

        BootState bootState;
        IdleState idleState;
        ActiveState activeState;
        ChargingState chargingState;
        FaultedState faultedState;

    public:
        StateMachine();

        virtual ~StateMachine();

        void handleState();

        void switchState(BaseState nextState);

}BMSStateMachine;


#endif