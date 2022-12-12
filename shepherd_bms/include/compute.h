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