#include "compute.h"

ComputeInterface::ComputeInterface()
{
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

void ComputeInterface::setFanSpeed(uint8_t newFanSpeed){}

int16_t ComputeInterface::getPackCurrent(){}

