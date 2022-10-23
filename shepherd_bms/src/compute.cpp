#include "compute.h"

ComputeInterface::ComputeInterface(){}

ComputeInterface::~ComputeInterface(){}

FaultStatus_t ComputeInterface::enableCharging(bool isEnabled){}

bool ComputeInterface::isCharging(){}

void ComputeInterface::setFanSpeed(uint8_t newFanSpeed){}

int16_t ComputeInterface::getPackCurrent(){}

