#ifndef COMPUTE_H
#define COMPUTE_H

#include "datastructs.h"
#include "nerduino.h"

using namespace std;

class ComputeInterface
{
    private:
        uint8_t fanSpeed;
        bool isChargingEnabled;

        union 
        {
           uint8_t chargerMsg[8] = {0, 0, 0, 0, 0, 0, 0xFF, 0xFF};

           struct
           {
                uint8_t chargerControl       :8;
                uint8_t chargerVoltageByte2  :8;
                uint8_t chargerVoltageByte3  :8;
                uint8_t chargerCurrentByte4  :8;
                uint8_t chargerCurrentByte5  :8;
                uint8_t chargerLEDs          :8;
                uint8_t reserved1            :8;
                uint8_t reserved2            :8;
           } chargerData;
           
        }startCharging;
        

        /**
         * @todo These might need to be changed depending on the charging ticket
         */
        const uint8_t startChargingMsg[];
        const uint8_t endChargingMsg[];

        void startChargerComms();

        void endChargerComms();

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

         /**
         * @brief sends charger message
         * 
         * @param voltageToSet
         * @param currentToSet
         *
         * @return int (error code 0 = message sent, 1 = not sent)
         */
        int sendChargingMessage(int voltageToSet, int currentToSet);



};
#endif