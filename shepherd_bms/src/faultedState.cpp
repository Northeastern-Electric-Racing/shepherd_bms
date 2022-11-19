#include "states.h"

extern FaultedState faultedState;

void FaultedState::handleState()
{
    //do something for faulted
}

void FaultedState::initialize()
{
    //initialize the BMS to be faulted (i.e. turn everything off and make it safe)
}