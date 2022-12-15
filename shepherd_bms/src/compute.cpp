#include "compute.h"

ComputeInterface::ComputeInterface()
{
    pinMode(CURRENT_SENSOR_PIN_H, INPUT);
    pinMode(CURRENT_SENSOR_PIN_L, INPUT);
    pinMode(FAULT_PIN, OUTPUT);
    initializeCAN(CANLINE_2, CHARGER_BAUD, &(this->chargerCallback));
}

ComputeInterface::~ComputeInterface(){}

void ComputeInterface::enableCharging(bool enableCharging){

    isChargingEnabled = enableCharging ? true : false;
}

FaultStatus_t ComputeInterface::sendChargingMessage(uint8_t voltageToSet, uint8_t currentToSet)
{
    if (!isChargingEnabled)
    {
        chargerMsg.cfg.chargerControl = CHARGE_DISABLED;
        sendMessageCAN2(CANMSG_CHARGER, 8, chargerMsg.msg);
        //return isCharging() ? FAULTED : NOT_FAULTED; //return a fault if we DO detect a voltage after we stop charging
        return NOT_FAULTED;
    }

    // equations taken from TSM2500 CAN protocol datasheet
    chargerMsg.cfg.chargerVoltage = voltageToSet * 10;
    chargerMsg.cfg.chargerCurrent = currentToSet * 10 + 3200;
    chargerMsg.cfg.chargerControl = CHARGE_ENABLED;

    //todo put charger ID somewhere else
    sendMessageCAN2(CANMSG_CHARGER, 8, chargerMsg.msg);

    //return isCharging() ? NOT_FAULTED : FAULTED; //return a fault if we DON'T detect a voltage after we begin charging
    return NOT_FAULTED;
}

bool ComputeInterface::isCharging() // This is useless kinda, especially if we move to DCDC
{
    return digitalRead(CHARGE_VOLTAGE_PIN);
}

void ComputeInterface::chargerCallback(const CAN_message_t &msg)
{
    Serial.println("Callback called!");
    return;
}

void ComputeInterface::setFanSpeed(uint8_t newFanSpeed)
{
    fanSpeed = newFanSpeed;
    NERduino.setAMCDutyCycle(newFanSpeed);
}

void ComputeInterface::setFault(FaultStatus_t faultState)
{
    digitalWrite(FAULT_PIN, !faultState);
    if (FAULTED) digitalWrite(CHARGE_SAFETY_RELAY, HIGH);
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
    sendMessageCAN1(0x202, 4, mcMsg.msg);
}
