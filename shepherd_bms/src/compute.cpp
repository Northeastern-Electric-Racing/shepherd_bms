#include "compute.h"

ComputeInterface compute;

ComputeInterface::ComputeInterface()
{
    pinMode(CURRENT_SENSOR_PIN_H, INPUT);
    pinMode(CURRENT_SENSOR_PIN_L, INPUT);
    pinMode(MEAS_5VREF_PIN, INPUT);
    pinMode(FAULT_PIN, OUTPUT);
    pinMode(CHARGE_DETECT, INPUT);
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

bool ComputeInterface::chargerConnected()
{
    return !(digitalRead(CHARGE_DETECT) == HIGH);
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
    // static const float CURRENT_SUPPLY_VOLTAGE = 5.038;
    static const float CURRENT_ADC_RESOLUTION = 5.0 / MAX_ADC_RESOLUTION;

    static const float CURRENT_LOWCHANNEL_OFFSET = 2.517; // Calibrated with current = 0A
    static const float CURRENT_HIGHCHANNEL_OFFSET = 2.520; // Calibrated with current = 0A

    static const float HIGHCHANNEL_GAIN = 1 / 0.004; // Calibrated with  current = 5A, 10A, 20A
    static const float LOWCHANNEL_GAIN = 1 / 0.0267;

    static const float REF5V_DIV = 19.02 / (19.08 + 19.02); // Resistive divider in kOhm
    static const float REF5V_CONV = 1 / REF5V_DIV; // Converting from reading to real value

    float ref_5V = analogRead(MEAS_5VREF_PIN) * (3.3 / MAX_ADC_RESOLUTION) * REF5V_CONV;
    int16_t high_current = 10 * (((5 / ref_5V) * (analogRead(CURRENT_SENSOR_PIN_L) * CURRENT_ADC_RESOLUTION)) - CURRENT_HIGHCHANNEL_OFFSET) * HIGHCHANNEL_GAIN; // Channel has a large range with low resolution
    int16_t low_current = 10 * (((5 / ref_5V) * (analogRead(CURRENT_SENSOR_PIN_H) * CURRENT_ADC_RESOLUTION)) - CURRENT_LOWCHANNEL_OFFSET) * LOWCHANNEL_GAIN; // Channel has a small range with high resolution
    
    // Serial.print("High: ");
    // Serial.println(-high_current);
    // Serial.print("Low: ");
    // Serial.println(-low_current);
    // Serial.print("5V: ");
    // Serial.println(ref_5V);

    // If the current is scoped within the range of the low channel, use the low channel
    if(low_current < CURRENT_LOWCHANNEL_MAX - 5.0 || low_current > CURRENT_LOWCHANNEL_MIN + 5.0)
    {
        return -low_current;
    }

    return -high_current;
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

void ComputeInterface::sendBMSStatusMessage(int bms_state, uint32_t fault_status, int8_t avg_temp, int8_t internal_temp)
{
    union
    {
        uint8_t msg[8] = {0, 0, 0, 0, 0, 0, 0, 0};

        struct
        {
            uint8_t state;
            uint32_t fault; 
            int8_t temp_avg;
            uint8_t temp_internal;
        } cfg;
    } bmsStatusMsg;

    bmsStatusMsg.cfg.temp_avg = static_cast<int8_t>(avg_temp);
    bmsStatusMsg.cfg.state = static_cast<uint8_t>(bms_state);
    bmsStatusMsg.cfg.fault = fault_status;
    bmsStatusMsg.cfg.temp_internal = static_cast<uint8_t>(internal_temp);

    uint8_t msg[8] = {
                         bmsStatusMsg.cfg.state, 
                        (fault_status & 0xff000000),
                        (fault_status & 0x00ff0000), 
                        (fault_status & 0x0000ff00), 
                        (fault_status & 0x000000ff), 
                         bmsStatusMsg.cfg.temp_avg
                     };
    

    sendMessageCAN1(CANMSG_BMSDTCSTATUS, 8, msg);
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

void ComputeInterface::sendCellDataMessage(CriticalCellValue_t high_voltage, CriticalCellValue_t low_voltage, uint16_t avg_voltage)
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

    cellDataMsg.cfg.highCellVoltage = high_voltage.val;
    cellDataMsg.cfg.highCellID = (high_voltage.chipIndex << 4) | high_voltage.cellNum;
    cellDataMsg.cfg.lowCellVoltage = low_voltage.val;
    cellDataMsg.cfg.lowCellID = (low_voltage.chipIndex << 4) | low_voltage.cellNum;
    cellDataMsg.cfg.voltAvg = avg_voltage;

    uint8_t msg[8] = {
                        ( cellDataMsg.cfg.highCellVoltage & 0x00ff), 
                        ((cellDataMsg.cfg.highCellVoltage & 0xff00)>>8), 
                          cellDataMsg.cfg.highCellID, 
                        ( cellDataMsg.cfg.lowCellVoltage & 0x00ff), 
                        ((cellDataMsg.cfg.lowCellVoltage & 0xff00)>>8), 
                          cellDataMsg.cfg.lowCellID, 
                        ( cellDataMsg.cfg.voltAvg & 0x00ff), 
                        ((cellDataMsg.cfg.voltAvg & 0xff00)>>8)
                     };

    sendMessageCAN1(CANMSG_BMSCELLDATA, 8, msg);
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

void ComputeInterface::sendCellTemp(CriticalCellValue_t max_cell_temp, CriticalCellValue_t min_cell_temp, uint16_t avg_temp)
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

    cellTempMsg.cfg.maxCellTemp = max_cell_temp.val;
    cellTempMsg.cfg.maxCellID = (max_cell_temp.chipIndex << 4) | (max_cell_temp.cellNum - 17);
    cellTempMsg.cfg.minCellTemp = min_cell_temp.val;
    cellTempMsg.cfg.minCellID = (min_cell_temp.chipIndex << 4) | (min_cell_temp.cellNum - 17);
    cellTempMsg.cfg.averageTemp = avg_temp;
    
    uint8_t msg[8] = {
                        ( cellTempMsg.cfg.maxCellTemp & 0x00ff), 
                        ((cellTempMsg.cfg.maxCellTemp & 0xff00)>>8), 
                          cellTempMsg.cfg.maxCellID, 
                        ( cellTempMsg.cfg.minCellTemp & 0x00ff),
                        ((cellTempMsg.cfg.minCellTemp & 0xff00)>>8), 
                          cellTempMsg.cfg.minCellID, 
                        ( cellTempMsg.cfg.averageTemp & 0x00ff), 
                        ((cellTempMsg.cfg.averageTemp & 0xff00)>>8)
                     };

    sendMessageCAN1(0x08, 8, msg);
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
