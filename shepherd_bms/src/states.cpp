#include "states.h"

extern StateMachine bmsDirector;

bool operator==(const BaseState &lhs, const BaseState &rhs)
{
    return lhs.state == rhs.state;
}

StateMachine::StateMachine()
{}

StateMachine::~StateMachine()
{}

void StateMachine::handleState()
{
    currentState.handleState();
}

void StateMachine::switchState(BaseState &nextState)
{
    currentState = nextState;
    currentState.initialize();
}

void StateMachine::requestTransition(BaseState &nextState)
{
    if(currentState == nextState) return;

    //defining the map of state transitions within a LUT
    switch(currentState.state)
    {
        case(BOOT):
            switch(nextState.state)
            {
                case(IDLE):
                case(FAULTED):
                    switchState(nextState);
                    return;
                default:
                    return;
            }
        case(IDLE):
            switch(nextState.state)
            {
                case(ACTIVE):
                case(CHARGING):
                case(FAULTED):
                    switchState(nextState);
                    return;
                default:
                    return;
            }
            return;
        case(ACTIVE):
            switch(nextState.state)
            {
                case(IDLE):
                case(FAULTED):
                    switchState(nextState);
                    return;
                default:
                    return;
            }
            return;
        case(CHARGING):
            switch(nextState.state)
            {
                case(IDLE):
                case(FAULTED):
                    switchState(nextState);
                    return;
                default:
                    return;
            }
            return;
        case(FAULTED):
            switch(nextState.state)
            {
                case(BOOT):
                    switchState(nextState);
                    return;
                default:
                    return;
            }
            return;
        default:
            return;
    }
}
