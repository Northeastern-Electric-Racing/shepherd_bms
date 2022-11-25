#include "compute.h"

ComputeInterface::ComputeInterface(){}

ComputeInterface::~ComputeInterface(){}

FaultStatus_t ComputeInterface::enableCharging(bool enableCharging){

    isChargingEnabled = enableCharging ? true : false;
}


FaultStatus_t ComputeInterface::sendChargingMessage(uint8_t voltageToSet, uint8_t currentToSet){

    if (!isChargingEnabled)
    {
        chargerMsg.cfg.chargerControl = CHARGE_DISABLED;
        sendMessageCAN2(CANMSG_CHARGER, 8, chargerMsg.msg);
        return isCharging() ? FAULTED : NOT_FAULTED; //return a fault if we DO detect a voltage after we stop charging
    }

    // equations taken from TSM2500 CAN protocol datasheet
    chargerMsg.cfg.chargerVoltage = voltageToSet * 10;
    chargerMsg.cfg.chargerCurrent = (currentToSet - 3200) * 10; //todo double check order of ops
    chargerMsg.cfg.chargerControl = CHARGE_ENABLED;

    //todo put charger ID somewhere else
    sendMessageCAN2(CANMSG_CHARGER, 8, chargerMsg.msg);

    return isCharging() ? NOT_FAULTED : FAULTED; //return a fault if we DON'T detect a voltage after we begin charging
}

bool ComputeInterface::isCharging()
{
    return digitalRead(CHARGE_VOLTAGE_PIN);
}

void ComputeInterface::setFanSpeed(uint8_t newFanSpeed){}

int16_t ComputeInterface::getPackCurrent(){}

