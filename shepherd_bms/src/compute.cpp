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
    uint16_t highCurrent = analogRead(CURRENT_SENSOR_PIN_H); //Channel has a large range with low resolution
    uint16_t lowCurrent = analogRead(CURRENT_SENSOR_PIN_L); //Channel has a small range with high resolution

    //If the current is scoped within the range of the low channel, use the low channel
    if(highCurrent < current_lowChannelMax || highCurrent > current_lowChannelMin)
    {
        return (lowCurrent - current_lowChannelOffset) * lowChannelResolution;
    }

    return (highCurrent - current_highChannelOffset) * highChannelResolution;
}

