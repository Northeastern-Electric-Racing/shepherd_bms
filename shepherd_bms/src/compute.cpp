#include "compute.h"

ComputeInterface compute;

ComputeInterface::ComputeInterface()
{
    pinMode(CURRENT_SENSOR_PIN_H, INPUT);
    pinMode(CURRENT_SENSOR_PIN_L, INPUT);
    pinMode(FAULT_PIN, OUTPUT);
    initializeCAN(CANLINE_2, CHARGER_BAUD, &(this->chargerCallback));
    initializeCAN(CANLINE_1, MC_BAUD, &(this->MCCallback));
}

ComputeInterface::~ComputeInterface(){}

void ComputeInterface::enableCharging(bool enableCharging)
{
    isChargingEnabled = enableCharging;
}

FaultStatus_t ComputeInterface::sendChargingMessage(uint16_t voltageToSet, uint16_t currentToSet)
{
    static union 
    {
        uint8_t msg[8] = {0, 0, 0, 0, 0, 0, 0xFF, 0xFF};

        struct
        {
            uint8_t chargerControl;
            uint16_t chargerVoltage;    //Note the charger voltage sent over should be 10*desired voltage
            uint16_t chargerCurrent;    //Note the charge current sent over should be 10*desired current + 3200
            uint8_t chargerLEDs;
            uint16_t reserved2_3;
        } cfg;
    } chargerMsg;

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
    static const float current_lowChannelMax = 75.0; //Amps
    static const float current_lowChannelMin = -75.0; //Amps
    static const float current_supplyVoltage = 5.038;
    static const float current_ADCResolution = 5.0 / MAX_ADC_RESOLUTION;

    static const float current_lowChannelOffset = 2.530; // Calibrated with current = 0A
    static const float current_highChannelOffset = 2.57; // Calibrated with current = 0A

    static const float highChannelGain = 1 / 0.0040;
    static const float lowChannelGain = 1 / 0.0267;


    uint16_t highCurrent = (5 / current_supplyVoltage) * (analogRead(CURRENT_SENSOR_PIN_H) * current_ADCResolution - current_highChannelOffset) * highChannelGain; // Channel has a large range with low resolution
    float lowCurrent = (5 / current_supplyVoltage) * (analogRead(CURRENT_SENSOR_PIN_L) * current_ADCResolution - current_lowChannelOffset) * lowChannelGain; // Channel has a small range with high resolution

    // If the current is scoped within the range of the low channel, use the low channel
    if(lowCurrent < current_lowChannelMax - 5.0 || lowCurrent > current_lowChannelMin + 5.0)
    {
        return lowCurrent;
    }

    return highCurrent;
}

void ComputeInterface::sendMCMsg(uint16_t userMaxCharge, uint16_t userMaxDischarge)
{
    static union 
    {
        uint8_t msg[4] = {0,0,0,0};

        struct
        {
            uint16_t maxDischarge;
            uint16_t maxCharge;

        }config;
    }mcMsg;

    mcMsg.config.maxCharge = userMaxCharge;
    mcMsg.config.maxDischarge = userMaxDischarge;
    sendMessageCAN1(0x202, 4, mcMsg.msg);
}

void ComputeInterface::sendAccStatusMessage(uint16_t voltage, int16_t current, uint16_t AH, uint8_t SoC, uint8_t health)
{
    static union 
    {
        uint8_t msg[8] = {0, 0, 0, 0, 0, 0, 0, 0};

        struct
        {
            uint16_t packVolt;
            uint16_t packCurrent;
            uint16_t packAH;
            uint8_t packSoC;
            uint8_t packHealth;
        } cfg;
    } accStatusMsg;

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
    static union 
    {
        uint8_t msg[8] = {0, 0, 0, 0, 0, 0, 0, 0};

        struct
        {
            uint8_t fsStatus;
            uint8_t dtcStatus1;
            uint16_t dtcStatus2;
            uint16_t currentLimit;
            uint8_t tempAvg;
            uint8_t tempInternal;
        } cfg;   
    } BMSStatusMsg;

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
    static union 
    {
        uint8_t msg[1] = {0};

        struct
        {
            uint8_t mpeState;
            
        } cfg;   
    } shutdownControlMsg;

    shutdownControlMsg.cfg.mpeState = mpeState;
    
    sendMessageCAN1(0x03, 1, shutdownControlMsg.msg);
}

void ComputeInterface::sendCellDataMessage(uint16_t hv, uint8_t hvID, uint16_t lv, uint8_t lvID, uint16_t voltAvg)
{
    static union 
    {
        uint8_t msg[8] = {0, 0, 0, 0, 0, 0, 0, 0};

        struct
        {
            uint16_t highCellVoltage;
            uint8_t highCellID;
            uint16_t lowCellVoltage;
            uint8_t lowCellID;
            uint16_t voltAvg;
        } cfg;   
    } cellDataMsg;

    cellDataMsg.cfg.highCellVoltage = __builtin_bswap16(hv);
    cellDataMsg.cfg.highCellID = hvID;
    cellDataMsg.cfg.lowCellVoltage = __builtin_bswap16(lv);
    cellDataMsg.cfg.lowCellID = lvID;
    cellDataMsg.cfg.voltAvg = __builtin_bswap16(voltAvg);

    sendMessageCAN1(0x04, 8, cellDataMsg.msg);
}

void ComputeInterface::sendCellVoltageMessage(uint8_t cellID, uint16_t instantVoltage, uint16_t internalResistance, uint8_t shunted, uint16_t openVoltage)
{
    static union 
    {
        uint8_t msg[8] = {0, 0, 0, 0, 0, 0, 0, 0};

        struct
        {
            uint8_t cellID;
            uint16_t instantVoltage;
            uint16_t internalResistance;
            uint8_t shunted;
            uint16_t openVoltage;
        } cfg;   
    } cellVoltageMsg;

    cellVoltageMsg.cfg.cellID = cellID;
    cellVoltageMsg.cfg.instantVoltage = __builtin_bswap16(instantVoltage);
    cellVoltageMsg.cfg.internalResistance = __builtin_bswap16(internalResistance);
    cellVoltageMsg.cfg.shunted = shunted;
    cellVoltageMsg.cfg.openVoltage = __builtin_bswap16(openVoltage);

    sendMessageCAN1(0x07, 8, cellVoltageMsg.msg);
}

void ComputeInterface::sendCurrentsStatus(uint16_t discharge, uint16_t charge, uint16_t current)
{
    static union 
    {
        uint8_t msg[6] = {0, 0, 0, 0, 0, 0};

        struct
        {
            uint16_t DCL;
            uint16_t CCL;
            uint16_t packCurr;
        } cfg;   
    } currentsStatusMsg;
    
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

void ComputeInterface::sendCellTemp(uint16_t mCellTemp, uint8_t mCellID, uint16_t minCellTemp, uint8_t minCellID, uint16_t tempAvg)
{
    static union
    {
        uint8_t msg[8] = {0, 0, 0, 0, 0, 0, 0, 0};

        struct 
        {
            uint16_t maxCellTemp;
            uint8_t maxCellID;
            uint16_t minCellTemp;
            uint8_t minCellID;
            uint16_t averageTemp;
        } cfg;
    } cellTempMsg;
    
    cellTempMsg.cfg.maxCellTemp = mCellTemp;
    cellTempMsg.cfg.maxCellID = mCellID;
    cellTempMsg.cfg.minCellTemp = minCellTemp;
    cellTempMsg.cfg.minCellID = minCellID;
    cellTempMsg.cfg.averageTemp = tempAvg;

    sendMessageCAN1(0x08, 8, cellTempMsg.msg);
}