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

FaultStatus_t ComputeInterface::sendChargingMessage(uint16_t voltageToSet, uint16_t currentToSet, uint8_t state_of_charge, AccumulatorData_t *bms_data)
{
    if (!isChargingEnabled)
    {
        chargerMsg.cfg.chargerControl = 0b101;
        sendMessageCAN2(CANMSG_CHARGER, 8, chargerMsg.msg);
        //return isCharging() ? FAULTED : NOT_FAULTED; //return a fault if we DO detect a voltage after we stop charging
        return NOT_FAULTED;
    }

    // equations taken from TSM2500 CAN protocol datasheet
    chargerMsg.cfg.chargerControl = 0xFC;
    chargerMsg.cfg.chargerVoltage = voltageToSet * 10;
    if (currentToSet > 10) 
    {
        currentToSet = 10;
    }
    chargerMsg.cfg.chargerCurrent = currentToSet * 10 + 3200;
    
    
    chargerMsg.cfg.chargerLEDs = calcChargerLEDState(state_of_charge, bms_data);
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

void ComputeInterface::sendBMSStatusMessage(uint8_t failsafe, uint8_t dtc1, uint16_t dtc2, uint16_t currentLimit, int8_t tempAvg, int8_t tempInternal)
{
    BMSStatusMsg.cfg.fsStatus = failsafe;
    BMSStatusMsg.cfg.dtcStatus1 = dtc1;
    BMSStatusMsg.cfg.dtcStatus2 = __builtin_bswap16(dtc2);
    BMSStatusMsg.cfg.currentLimit = __builtin_bswap16(currentLimit);
    BMSStatusMsg.cfg.tempAvg = static_cast<uint8_t>(tempAvg);
    BMSStatusMsg.cfg.tempInternal = static_cast<uint8_t>(tempInternal);

    
    sendMessageCAN1(0x02, 8, BMSStatusMsg.msg);
}

void ComputeInterface::sendShutdownControlMessage(uint8_t mpeState)
{
    shutdownControlMsg.cfg.mpeState = mpeState;
    
    sendMessageCAN1(0x03, 1, shutdownControlMsg.msg);
}

void ComputeInterface::sendCellDataMessage(uint16_t hv, uint8_t hvID, uint16_t lv, uint8_t lvID, uint16_t voltAvg)
{
    cellDataMsg.cfg.highCellVoltage = __builtin_bswap16(hv);
    cellDataMsg.cfg.highCellID = hvID;
    cellDataMsg.cfg.lowCellVoltage = __builtin_bswap16(lv);
    cellDataMsg.cfg.lowCellID = lvID;
    cellDataMsg.cfg.voltAvg = __builtin_bswap16(voltAvg);

    sendMessageCAN1(0x04, 8, cellDataMsg.msg);
}

void ComputeInterface::sendCellVoltageMessage(uint8_t cellID, uint16_t instantVoltage, uint16_t internalResistance, uint8_t shunted, uint16_t openVoltage)
{
    cellVoltageMsg.cfg.cellID = cellID;
    cellVoltageMsg.cfg.instantVoltage = __builtin_bswap16(instantVoltage);
    cellVoltageMsg.cfg.internalResistance = __builtin_bswap16(internalResistance);
    cellVoltageMsg.cfg.shunted = shunted;
    cellVoltageMsg.cfg.openVoltage = __builtin_bswap16(openVoltage);

    sendMessageCAN1(0x07, 8, cellVoltageMsg.msg);
}

void ComputeInterface::sendCurrentsStatus(uint16_t discharge, uint16_t charge, uint16_t current)
{
    currentsStatusMsg.cfg.DCL = discharge;
    currentsStatusMsg.cfg.CCL = charge;
    currentsStatusMsg.cfg.packCurr = current;

    sendMessageCAN1(0x06, 8, currentsStatusMsg.msg);
}

void ComputeInterface::sendChargingStatus(bool chargingStatus)
{
    uint8_t chargingArray[1] = {chargingStatus};

    sendMessageCAN1(0x05, 1, chargingArray);
}

void ComputeInterface::MCCallback(const CAN_message_t &msg)
{
    return;
}

uint8_t ComputeInterface::calcChargerLEDState(uint8_t state_of_charge, AccumulatorData_t *bms_data)
{
  enum LED_state
  {
    RED_BLINKING =       0x00,
    RED_CONSTANT =       0x01,
    YELLOW_BLINKING =    0x02,
    YELLOW_CONSTANT =    0x03,
    GREEN_BLINKING =     0x04,
    GREEN_CONSTANT =     0x05,
    RED_GREEN_BLINKING = 0x06
  };

  if((state_of_charge < 80) && (bms_data->packCurrent > .7 * 10))
  {
    return RED_BLINKING;
  }
  else if((state_of_charge < 80) && (bms_data->packCurrent <= .7 * 10))
  {
    return RED_CONSTANT;
  }
  else if((state_of_charge >= 80 && state_of_charge < 95) && (bms_data->packCurrent > .7 * 10))
  {
    return YELLOW_BLINKING;
  }
  else if((state_of_charge >= 80 && state_of_charge < 95) && (bms_data->packCurrent <= .7 * 10))
  {
    return YELLOW_CONSTANT;
  }
  else if((state_of_charge >= 95) && (bms_data->packCurrent > .7 * 10))
  {
    return GREEN_BLINKING;
  }
  else if((state_of_charge >= 95) && (bms_data->packCurrent <= .7 * 10))
  {
    return GREEN_BLINKING;
  }
  else
  {
    return RED_GREEN_BLINKING;
  }
  
}