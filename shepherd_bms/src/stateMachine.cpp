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
    prevAccData = nullptr;
    bmsFault = FAULTS_CLEAR;
    overVoltCount = 0;
    underVoltCount = 0;
    overCurrCount = 0;
    chargeOverVolt = 0;
    overChgCurrCount = 0;
    lowCellCount = 0;
    analyzer.push(prevAccData);
}

void StateMachine::handleBoot(AccumulatorData_t *bmsdata)
{
    return;
}

void StateMachine::initReady()
{
    segment.enableBalancing(false);
    compute.enableCharging(false);
    analyzer.push(prevAccData);
}

void StateMachine::handleReady(AccumulatorData_t *bmsdata)
{
    return;
}

void StateMachine::initCharging()
{
    compute.enableCharging(true);
    segment.enableBalancing(false);
}

void StateMachine::handleCharging(AccumulatorData_t *bmsdata)
{
    return;
}

void StateMachine::initFaulted()
{
    segment.enableBalancing(false);
    compute.enableCharging(false);
    bmsFault = prevAccData->faultCode;
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