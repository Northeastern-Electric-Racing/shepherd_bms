#include "compute.h"

ComputeInterface::ComputeInterface()
{
    pinMode(CURRENT_SENSOR_PIN_H, INPUT);
    pinMode(CURRENT_SENSOR_PIN_L, INPUT);
}

ComputeInterface::~ComputeInterface(){}

FaultStatus_t ComputeInterface::enableCharging(bool isEnabled){}

bool ComputeInterface::isCharging(){}

void ComputeInterface::setFanSpeed(uint8_t newFanSpeed)
{
    fanSpeed = newFanSpeed;
    NERduino.setAMCDutyCycle(newFanSpeed);
}

int16_t ComputeInterface::getPackCurrent()
{
    uint16_t highCurrent = analogRead(CURRENT_SENSOR_PIN_H) * highChannelResolution - current_highChannelOffset; // Channel has a large range with low resolution
    uint16_t lowCurrent = analogRead(CURRENT_SENSOR_PIN_L) * lowChannelResolution - current_lowChannelOffset; // Channel has a small range with high resolution

    // If the current is scoped within the range of the low channel, use the low channel
    if(lowCurrent < current_lowChannelMax || lowCurrent > current_lowChannelMin)
    {
        return lowCurrent;
    }

    return highCurrent;
}

