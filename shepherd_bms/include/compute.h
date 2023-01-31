#ifndef COMPUTE_H
#define COMPUTE_H

#include "datastructs.h"
#include "nerduino.h"
#include "canMsgHandler.h"

#define CURRENT_SENSOR_PIN_L    A1
#define CURRENT_SENSOR_PIN_H    A0
#define FAULT_PIN               RELAY_PIN
#define CHARGE_VOLTAGE_PIN 3
#define CHARGE_SAFETY_RELAY 6
#define CHARGE_DETECT 7
#define CHARGER_BAUD 250000U
#define MC_BAUD 1000000U

#define MAX_ADC_RESOLUTION      1023 // 13 bit ADC

class ComputeInterface
{
    private:
        uint8_t fanSpeed;
        bool isChargingEnabled;

        enum
        {
            CHARGE_ENABLED,
            CHARGE_DISABLED
        };

        union 
        {
            uint8_t msg[4] = {0,0,0,0};

            struct
            {
                uint16_t maxDischarge;
                uint16_t maxCharge;

            }config;
        }mcMsg;

        const float current_lowChannelMax = 75.0; //Amps
        const float current_lowChannelMin = -75.0; //Amps
        const int16_t current_highChannelMax = 500; //Amps
        const int16_t current_highChannelMin = -500; //Amps
        const float current_supplyVoltage = 5.038;
        const float current_ADCResolution = 5.0 / MAX_ADC_RESOLUTION;

        const float current_lowChannelOffset = 2.530; // Calibrated with current = 0A
        const float current_highChannelOffset = 2.57; // Calibrated with current = 0A

        const float highChannelGain = 1 / 0.0040;
        const float lowChannelGain = 1 / 0.0267;

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

        union 
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


        union 
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


        union 
        {
           uint8_t msg[1] = {0};

           struct
           {
                uint8_t mpeState;
                
           } cfg;   
        } shutdownControlMsg;


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

    public:
        ComputeInterface();

        ~ComputeInterface();

        /**
         * @brief sets safeguard bool to check whether charging is enabled or disabled
         *
         * @param isEnabled
         */
        void enableCharging(bool isEnabled);

        /**
         * @brief sends charger message
         * 
         * @param voltageToSet
         * @param currentToSet
         *
         * @return Returns a fault if we are not able to communicate with charger
         */
        FaultStatus_t sendChargingMessage(uint16_t voltageToSet, uint16_t currentToSet, uint8_t state_of_charge);

        /**
         * @brief Returns if we are detecting a charging voltage
         *
         * @return true
         * @return false
         */
        bool isCharging();

        /**
         * @brief Handle any messages received from the charger
         * 
         * @param msg 
         */
        static void chargerCallback(const CAN_message_t &msg);

        static void MCCallback(const CAN_message_t &msg);

        /**
         * @brief Sets the desired fan speed
         *
         * @param newFanSpeed
         */
        void setFanSpeed(uint8_t newFanSpeed);

        /**
         * @brief Returns the pack current sensor reading
         *
         * @return int16_t
         */
        int16_t getPackCurrent();

        /**
         * @brief sends max charge/discharge current to Motor Controller
         *
         * @param maxCharge
         * @param maxDischarge
         */
        void sendMCMsg(uint16_t maxCharge, uint16_t maxDischarge);

        /**
         * @brief updates fault relay
         *
         * @param faultState
         */
        void setFault(FaultStatus_t faultState);

        /**
         * @brief sends acc status message
         * 
         * @param voltage
         * @param current
         * @param AH
         * @param SoC
         * @param health
         *
         * @return Returns a fault if we are not able to send
         */
        void sendAccStatusMessage(uint16_t voltage, int16_t current, uint16_t AH, uint8_t SoC, uint8_t health);

        /**
         * @brief sends BMS status message 
         * 
         * @param failsafe
         * @param dtc1
         * @param dtc2
         * @param currentLimit
         * @param tempAvg
         * @param tempInternal
         * 
         * @return Returns a fault if we are not able to send
         */
        void sendBMSStatusMessage(uint8_t failsafe, uint8_t dtc1, uint16_t dtc2, uint16_t currentLimit, int8_t tempAvg, int8_t tempInternal);

         /**
         * @brief sends shutdown control message
         * 
         * @param mpeState
         *
         * @return Returns a fault if we are not able to send
         */
        void sendShutdownControlMessage(uint8_t mpeState);

        /**
         * @brief sends cell data message
         * 
         * @param hv
         * @param hvID
         * @param lv
         * @param lvID
         * @param voltAvg
         *
         * @return Returns a fault if we are not able to send
         */
        void sendCellDataMessage(uint16_t hv, uint8_t hvID, uint16_t lv, uint8_t lvID, uint16_t voltAvg);

        /**
         * @brief sends cell voltage message
         * 
         * @param cellID
         * @param instantVoltage
         * @param internalResistance
         * @param shunted
         * @param openVoltage
         *
         * @return Returns a fault if we are not able to send
         */
        void sendCellVoltageMessage(uint8_t cellID, uint16_t instantVoltage, uint16_t internalResistance, uint8_t shunted, uint16_t openVoltage);

        /**
         * @brief sends "is charging" message
         * 
         * @param chargingStatus
         *
         * @return Returns a fault if we are not able to send
         */
        void sendChargingStatus(bool chargingStatus);

        void sendCurrentsStatus(uint16_t discharge, uint16_t charge, uint16_t current);

        /**
         * @brief Determines state of the charger LEDs based on the battery charge percentage
         * 
         * @param state_of_charge 8 bit integer representing battery charge
         * @param bms_data Accumulator data
         * 
         * @return uint8_t Value to be used for setting LED bits for charger message
         * 
         */
        uint8_t calcChargerLEDState(uint8_t state_of_charge, AccumulatorData_t bms_data);
};

#endif