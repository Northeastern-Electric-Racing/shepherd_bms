#ifndef COMPUTE_H
#define COMPUTE_H

#include "datastructs.h"
#include "nerduino.h"

#define CHARGE_VOLTAGE_PIN 3

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
           uint8_t msg[8] = {0, 0, 0, 0, 0, 0, 0xFF, 0xFF};

           struct
           {
                bool chargerControl         :8;
                uint16_t chargerVoltage     :16;    //Note the charger voltage sent over should be 10*desired voltage
                uint16_t chargerCurrent     :16;    //Note the charge current sent over should be -3200+desired current*10
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
         * @return FaultStatus_t
         */
        FaultStatus_t enableCharging(bool isEnabled);

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
};
#endif