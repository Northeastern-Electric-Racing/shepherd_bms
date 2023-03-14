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

void ComputeInterface::enableCharging(bool enable_charging)
{
    is_charging_enabled_ = enable_charging;
}

FaultStatus_t ComputeInterface::sendChargingMessage(uint16_t voltage_to_set, AccumulatorData_t *bms_data)
{
    union
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

    uint16_t current_to_set = bms_data->charge_limit;

    if (!is_charging_enabled_)
    {
        chargerMsg.cfg.chargerControl = 0b101;
        sendMessageCAN2(CANMSG_CHARGER, 8, chargerMsg.msg);
        //return isCharging() ? FAULTED : NOT_FAULTED; //return a fault if we DO detect a voltage after we stop charging
        return NOT_FAULTED;
    }

    // equations taken from TSM2500 CAN protocol datasheet
    chargerMsg.cfg.chargerControl = 0xFC;
    chargerMsg.cfg.chargerVoltage = voltage_to_set * 10;
    if (current_to_set > 10) 
    {
        current_to_set = 10;
    }
    chargerMsg.cfg.chargerCurrent = current_to_set * 10 + 3200;
    chargerMsg.cfg.chargerLEDs = calcChargerLEDState(bms_data);
    chargerMsg.cfg.reserved2_3 = 0xFFFF;

    uint8_t msg[8] = {chargerMsg.cfg.chargerControl, static_cast<uint8_t>(chargerMsg.cfg.chargerVoltage), chargerMsg.cfg.chargerVoltage >> 8, static_cast<uint8_t>(chargerMsg.cfg.chargerCurrent), chargerMsg.cfg.chargerCurrent >> 8, chargerMsg.cfg.chargerLEDs, 0xFF, 0xFF};

    sendMessageCAN2(CANMSG_CHARGER, 8, msg);

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

void ComputeInterface::setFanSpeed(uint8_t new_fan_speed)
{
    fan_speed_ = new_fan_speed;
    NERduino.setAMCDutyCycle(new_fan_speed);
}

void ComputeInterface::setFault(FaultStatus_t fault_state)
{
    digitalWrite(FAULT_PIN, !fault_state);
    if (FAULTED) digitalWrite(CHARGE_SAFETY_RELAY, HIGH);
}

int16_t ComputeInterface::getPackCurrent()
{
    static const float CURRENT_LOWCHANNEL_MAX = 75.0; //Amps
    static const float CURRENT_LOWCHANNEL_MIN = -75.0; //Amps
    static const float CURRENT_SUPPLY_VOLTAGE = 5.038;
    static const float CURRENT_ADC_RESOLUTION = 5.0 / MAX_ADC_RESOLUTION;

    static const float CURRENT_LOWCHANNEL_OFFSET = 2.530; // Calibrated with current = 0A
    static const float CURRENT_HIGHCHANNEL_OFFSET = 2.57; // Calibrated with current = 0A

    static const float HIGHCHANNEL_GAIN = 1 / 0.0040;
    static const float LOWCHANNEL_GAIN = 1 / 0.0267;


    int16_t high_current = 10 * (5 / CURRENT_SUPPLY_VOLTAGE) * (analogRead(CURRENT_SENSOR_PIN_H) * CURRENT_ADC_RESOLUTION - CURRENT_HIGHCHANNEL_OFFSET) * HIGHCHANNEL_GAIN; // Channel has a large range with low resolution
    int16_t low_current = 10 * (5 / CURRENT_SUPPLY_VOLTAGE) * (analogRead(CURRENT_SENSOR_PIN_L) * CURRENT_ADC_RESOLUTION - CURRENT_LOWCHANNEL_OFFSET) * LOWCHANNEL_GAIN; // Channel has a small range with high resolution

    // If the current is scoped within the range of the low channel, use the low channel
    if(low_current < CURRENT_LOWCHANNEL_MAX - 5.0 || low_current > CURRENT_LOWCHANNEL_MIN + 5.0)
    {
        return low_current;
    }

    return high_current;
}

void ComputeInterface::sendMCMsg(uint16_t user_max_charge, uint16_t user_max_discharge)
{
    union 
    {
        uint8_t msg[4] = {0,0,0,0};

        struct
        {
            uint16_t maxDischarge;
            uint16_t maxCharge;

        }config;
    }mcMsg;

    mcMsg.config.maxCharge = user_max_charge;
    mcMsg.config.maxDischarge = user_max_discharge;
    sendMessageCAN1(CANMSG_BMSCURRENTLIMITS, 4, mcMsg.msg);
}

void ComputeInterface::sendAccStatusMessage(uint16_t voltage, int16_t current, uint16_t ah, uint8_t soc, uint8_t health)
{
    union 
    {
        uint8_t msg[8] = {0, 0, 0, 0, 0, 0, 0, 0};

        struct
        {
            uint16_t packVolt;
            uint16_t pack_current;
            uint16_t packAH;
            uint8_t packSoC;
            uint8_t packHealth;
        } cfg;
    } accStatusMsg;

    accStatusMsg.cfg.packVolt = __builtin_bswap16(voltage);
    accStatusMsg.cfg.pack_current = __builtin_bswap16(static_cast<uint16_t>(current)); // convert with 2s complement
    accStatusMsg.cfg.packAH = __builtin_bswap16(ah);
    accStatusMsg.cfg.packSoC = soc;
    accStatusMsg.cfg.packHealth = health;

    sendMessageCAN1(CANMSG_BMSACCSTATUS, 8, accStatusMsg.msg);
}

void ComputeInterface::sendBMSStatusMessage(BMSState_t bms_state, BMSFault_t fault_status, int8_t avg_temp, int8_t internal_temp)
{
    union 
    {
        uint8_t msg[8] = {0, 0, 0, 0, 0, 0, 0, 0};

        struct
        {
            uint8_t state;
            uint32_t fault;
            uint8_t temp_avg;
            uint8_t temp_internal;
        } cfg;   
    } bmsStatusMsg;

    bmsStatusMsg.cfg.state = bms_state;
    bmsStatusMsg.cfg.fault = fault_status;
    bmsStatusMsg.cfg.temp_avg = static_cast<uint8_t>(avg_temp);
    bmsStatusMsg.cfg.temp_internal = static_cast<uint8_t>(internal_temp);

    
    sendMessageCAN1(CANMSG_BMSDTCSTATUS, 8, bmsStatusMsg.msg);
}

void ComputeInterface::sendShutdownControlMessage(uint8_t mpe_state)
{
    union 
    {
        uint8_t msg[1] = {0};

        struct
        {
            uint8_t mpeState;
            
        } cfg;   
    } shutdownControlMsg;

    shutdownControlMsg.cfg.mpeState = mpe_state;
    
    sendMessageCAN1(0x03, 1, shutdownControlMsg.msg);
}

void ComputeInterface::sendCellDataMessage(uint16_t hv, uint8_t hv_id, uint16_t lv, uint8_t lv_id, uint16_t avg_volt)
{
    union 
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
    cellDataMsg.cfg.highCellID = hv_id;
    cellDataMsg.cfg.lowCellVoltage = __builtin_bswap16(lv);
    cellDataMsg.cfg.lowCellID = lv_id;
    cellDataMsg.cfg.voltAvg = __builtin_bswap16(avg_volt);

    sendMessageCAN1(CANMSG_BMSCELLDATA, 8, cellDataMsg.msg);
}

void ComputeInterface::sendCellVoltageMessage(uint8_t cell_id, uint16_t instant_voltage, uint16_t internal_Res, uint8_t shunted, uint16_t open_voltage)
{
    union 
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

    cellVoltageMsg.cfg.cellID = cell_id;
    cellVoltageMsg.cfg.instantVoltage = __builtin_bswap16(instant_voltage);
    cellVoltageMsg.cfg.internalResistance = __builtin_bswap16(internal_Res);
    cellVoltageMsg.cfg.shunted = shunted;
    cellVoltageMsg.cfg.openVoltage = __builtin_bswap16(open_voltage);

    sendMessageCAN1(0x07, 8, cellVoltageMsg.msg);
}

void ComputeInterface::sendCurrentsStatus(uint16_t discharge, uint16_t charge, uint16_t current)
{
    union 
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

    sendMessageCAN1(CANMSG_BMSCURRENTS, 8, currentsStatusMsg.msg);
}

void ComputeInterface::MCCallback(const CAN_message_t &msg)
{
    return;
}

void ComputeInterface::sendCellTemp(uint16_t m_cell_temp, uint8_t m_cell_id, uint16_t min_cell_temp, uint8_t min_cell_id, uint16_t avg_temp)
{
    union
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
    
    cellTempMsg.cfg.maxCellTemp = m_cell_temp;
    cellTempMsg.cfg.maxCellID = m_cell_id;
    cellTempMsg.cfg.minCellTemp = min_cell_temp;
    cellTempMsg.cfg.minCellID = min_cell_id;
    cellTempMsg.cfg.averageTemp = avg_temp;

    sendMessageCAN1(0x08, 8, cellTempMsg.msg);
}
uint8_t ComputeInterface::calcChargerLEDState(AccumulatorData_t *bms_data)
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

  if((bms_data->soc < 80) && (bms_data->pack_current > .5 * 10))
  {
    return RED_BLINKING;
  }
  else if((bms_data->soc < 80) && (bms_data->pack_current <= .5 * 10))
  {
    return RED_CONSTANT;
  }
  else if((bms_data->soc >= 80 && bms_data->soc < 95) && (bms_data->pack_current > .5 * 10))
  {
    return YELLOW_BLINKING;
  }
  else if((bms_data->soc >= 80 && bms_data->soc < 95) && (bms_data->pack_current <= .5 * 10))
  {
    return YELLOW_CONSTANT;
  }
  else if((bms_data->soc >= 95) && (bms_data->pack_current > .5 * 10))
  {
    return GREEN_BLINKING;
  }
  else if((bms_data->soc >= 95) && (bms_data->pack_current <= .5 * 10))
  {
    return GREEN_BLINKING;
  }
  else
  {
    return RED_GREEN_BLINKING;
  }

}
