#include "compute.h"

ComputeInterface::ComputeInterface()
{
    pinMode(CURRENT_SENSOR_PIN_H, INPUT);
    pinMode(CURRENT_SENSOR_PIN_L, INPUT);
    pinMode(FAULT_PIN, OUTPUT);
    initializeCAN(CANLINE_2, CHARGER_BAUD, &(this->chargerCallback));
    initializeCAN(CANLINE_1, MC_BAUD, &(this->MCCallback));
}

ComputeInterface::~ComputeInterface(){}

void ComputeInterface::enableCharging(bool enableCharging){
    isChargingEnabled = enableCharging;
}

FaultStatus_t ComputeInterface::sendChargingMessage(uint16_t voltageToSet, uint16_t currentToSet)
{
    if (!isChargingEnabled)
    {
        chargerMsg.cfg.chargerControl = 0b101;
        sendMessageCAN2(CANMSG_CHARGER, 8, chargerMsg.msg);
        Serial.println("DISABLED!");
        //return isCharging() ? FAULTED : NOT_FAULTED; //return a fault if we DO detect a voltage after we stop charging
        return NOT_FAULTED;
    }

    // equations taken from TSM2500 CAN protocol datasheet
    chargerMsg.cfg.chargerControl = 0xFC;
    chargerMsg.cfg.chargerVoltage = voltageToSet * 10;
    if (currentToSet > 10) {
        currentToSet = 10;
    }
    chargerMsg.cfg.chargerCurrent = currentToSet * 10 + 3200;
    chargerMsg.cfg.chargerLEDs = 0x01;
    chargerMsg.cfg.reserved2_3 = 0xFFFF;

    uint8_t msg[8] = {chargerMsg.cfg.chargerControl, static_cast<uint8_t>(chargerMsg.cfg.chargerVoltage), chargerMsg.cfg.chargerVoltage >> 8, static_cast<uint8_t>(chargerMsg.cfg.chargerCurrent), chargerMsg.cfg.chargerCurrent >> 8, chargerMsg.cfg.chargerLEDs, 0xFF, 0xFF};

    //todo put charger ID somewhere else
    sendMessageCAN2(0x18E54024, 8, msg);

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
    Serial.print("ID:\t");
    Serial.println(msg.id, HEX);
    Serial.print("DATA:");
    for (int i = 0; i < msg.len; i++) {
        Serial.print("\t");
        Serial.print(msg.buf[i], HEX);
    }
    Serial.println();
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

void ComputeInterface::sendAccStatusMessage(uint16_t voltage, int16_t current, uint16_t AH, uint8_t SoC, uint8_t health)
{
    accStatusMsg.cfg.packVolt = __builtin_bswap16(voltage);
    accStatusMsg.cfg.packCurrent = __builtin_bswap16(static_cast<uint16_t>(current)); // convert with 2s complement
    accStatusMsg.cfg.packAH = __builtin_bswap16(AH);
    accStatusMsg.cfg.packSoC = SoC;
    accStatusMsg.cfg.packHealth = health;

    //todo put ID somewhere else
    sendMessageCAN1(0x01, 8, accStatusMsg.msg);
}

void ComputeInterface::MCCallback(const CAN_message_t &msg)
{
    return;
}
