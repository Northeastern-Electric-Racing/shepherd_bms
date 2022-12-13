#include "compute.h"

ComputeInterface::ComputeInterface()
{
    pinMode(CURRENT_SENSOR_PIN_H, INPUT);
    pinMode(CURRENT_SENSOR_PIN_L, INPUT);
    pinMode(FAULT_PIN, OUTPUT);
}

ComputeInterface::~ComputeInterface(){}

FaultStatus_t ComputeInterface::enableCharging(bool isEnabled){}

bool ComputeInterface::isCharging(){}

void ComputeInterface::setFanSpeed(uint8_t newFanSpeed)
{
    fanSpeed = newFanSpeed;
    NERduino.setAMCDutyCycle(newFanSpeed);
}

void ComputeInterface::setFault(bool faultState)
{
    digitalWrite(FAULT_PIN, faultState);
}

int16_t ComputeInterface::getPackCurrent()
{
    uint16_t highCurrent = (5 / current_supplyVoltage) * (analogRead(CURRENT_SENSOR_PIN_H) * current_ADCResolution - current_highChannelOffset) * highChannelGain; // Channel has a large range with low resolution
    float lowCurrent = (5 / current_supplyVoltage) * (analogRead(CURRENT_SENSOR_PIN_L) * current_ADCResolution - current_lowChannelOffset) * lowChannelGain; // Channel has a small range with high resolution

    // If the current is scoped within the range of the low channel, use the low channel
    if(lowCurrent < current_lowChannelMax - 5.0 || lowCurrent > current_lowChannelMin + 5.0)
    {
        return lowCurrent;
    }

    return highCurrent;
}

void ComputeInterface::sendMCMsg(uint16_t userMaxCharge, uint16_t userMaxDischarge){

    mcMsg.config.maxCharge = userMaxCharge;
    mcMsg.config.maxDischarge = userMaxDischarge;
    sendMessage(0x202, 4, mcMsg.msg);

}

