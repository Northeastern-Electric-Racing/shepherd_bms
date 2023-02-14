#include "stateMachine.h"

StateMachine::StateMachine()
{
    current_state = BOOT_STATE;
    initBoot();
}

StateMachine::~StateMachine()
{}

void StateMachine::initBoot()
{
    return;
}

void StateMachine::handleBoot(AccumulatorData_t *bmsdata)
{
    return;
}

void StateMachine::initReady()
{
    return;
}

void StateMachine::handleReady(AccumulatorData_t *bmsdata)
{
    return;
}

void StateMachine::initCharging()
{
    return;
}

void StateMachine::handleCharging(AccumulatorData_t *bmsdata)
{
    return;
}

void StateMachine::initFaulted()
{
    return;
}

void StateMachine::handleFaulted(AccumulatorData_t *bmsdata)
{
    return;
}

void StateMachine::handleState(AccumulatorData_t *bmsdata)
{
    (this->*handlerLUT[current_state])(bmsdata);
}

void StateMachine::requestTransition(BMSState_t next_state)
{
    if(current_state == next_state) return;

    if(!validTransitionFromTo[current_state][next_state]) return;

    (this->*initLUT[next_state])();
}