#ifndef COMPUTE_H
#define COMPUTE_H

#include "datastructs.h"
#include "nerduino.h"

#define CURRENT_SENSOR_PIN_L    A1
#define CURRENT_SENSOR_PIN_H    A0

#define MAX_ADC_RESOLUTION      1023 // 13 bit ADC

class ComputeInterface
{
    private:
        uint8_t fanSpeed;

        const int16_t current_lowChannelMax = 50; //Amps
        const int16_t current_lowChannelMin = -50; //Amps
        const int16_t current_highChannelMax = 320; //Amps
        const int16_t current_highChannelMin = -450; //Amps

        const int16_t current_lowChannelOffset = MAX_ADC_RESOLUTION * (2.5/5); //2.5V
        const int16_t current_highChannelOffset = MAX_ADC_RESOLUTION * (2.8/5); //2.8V

        const float highChannelResolution = (abs(current_lowChannelMin) + current_lowChannelMax) / MAX_ADC_RESOLUTION;
        const float lowChannelResolution = (abs(current_highChannelMin) + current_highChannelMax) / MAX_ADC_RESOLUTION;

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
         * @brief Attempts to enable/disable the charger via a second CAN line, and returns a fault if the charger is not responding
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

};
#endif