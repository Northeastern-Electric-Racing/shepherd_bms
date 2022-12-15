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

        const float current_lowChannelOffset = 2.58; // Calibrated with current = 0A
        const float current_highChannelOffset = 2.57; // Calibrated with current = 0A

        const float highChannelGain = 1 / 0.0040;
        const float lowChannelGain = 1 / 0.0267;

        /**
         * @todo These might need to be changed depending on the charging ticket
         */
        const uint8_t startChargingMsg[];
        const uint8_t endChargingMsg[];

        union 
        {
           uint8_t msg[8] = {0, 0, 0, 0, 0, 0, 0xFF, 0xFF};

           struct
           {
                bool chargerControl         :8;
                uint16_t chargerVoltage     :16;    //Note the charger voltage sent over should be 10*desired voltage
                uint16_t chargerCurrent     :16;    //Note the charge current sent over should be 10*desired current + 3200
                uint8_t chargerLEDs         :8;
                uint8_t reserved1           :8;
                uint8_t reserved2           :8;
           } cfg;
           
        } chargerMsg;

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
        FaultStatus_t sendChargingMessage(uint8_t voltageToSet, uint8_t currentToSet);

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

};

#endif