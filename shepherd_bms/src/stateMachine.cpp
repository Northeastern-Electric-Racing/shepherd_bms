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
    AccumulatorData_t *prevAccData = nullptr;
    uint32_t bmsFault = FAULTS_CLEAR;
    uint16_t overVoltCount = 0;
    uint16_t underVoltCount = 0;
    uint16_t overCurrCount = 0;
    uint16_t chargeOverVolt = 0;
    uint16_t overChgCurrCount = 0;
    uint16_t lowCellCount = 0;
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
    static Timer chargeMessageTimer;
	static const uint16_t CHARGE_MESSAGE_WAIT = 250; //ms
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