#include "stateMachine.h"

StateMachine::StateMachine()
{
    currentState = BOOT_STATE;
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
    (this->*handlerLUT[currentState])(bmsdata);
}

void StateMachine::requestTransition(BMSState_t nextState)
{
    if(currentState == nextState) return;

    if(!validTransitionFromTo[currentState][nextState]) return;

    (this->*initLUT[nextState])();
}
