#include "compute.h"

ComputeInterface compute;

ComputeInterface::ComputeInterface(){}

ComputeInterface::~ComputeInterface(){}

FaultStatus_t ComputeInterface::enableCharging(bool isEnabled){}

bool ComputeInterface::isCharging(){}

void ComputeInterface::setFanSpeed(uint8_t newFanSpeed)
{
    fanSpeed = newFanSpeed;
    NERduino.setAMCDutyCycle(newFanSpeed);
}

int16_t ComputeInterface::getPackCurrent(){}

void ComputeInterface::sendMCMsg(uint16_t userMaxCharge, uint16_t userMaxDischarge){

    mcMsg.config.maxCharge = userMaxCharge;
    mcMsg.config.maxDischarge = userMaxDischarge;
    sendMessage(0x202, 4, mcMsg.msg);

}

