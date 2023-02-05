#ifndef COMPUTE_H
#define COMPUTE_H

#include "datastructs.h"
#include "nerduino.h"
#include "canMsgHandler.h"

#define CURRENT_SENSOR_PIN_L  A1
#define CURRENT_SENSOR_PIN_H  A0
#define FAULT_PIN             RELAY_PIN
#define CHARGE_VOLTAGE_PIN    3
#define CHARGE_SAFETY_RELAY   6
#define CHARGE_DETECT         7
#define CHARGER_BAUD          250000U
#define MC_BAUD               1000000U

#define CHARGER_CAN_ID        0x18E54024

#define MAX_ADC_RESOLUTION    1023 // 13 bit ADC

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

      /**
      * @brief Determines state of the charger LEDs based on the battery charge percentage
      * 
      * @param bms_data
      * 
      * @return uint8_t Value to be used for setting LED bits for charger message
      * 
      */
      uint8_t calcChargerLEDState(AccumulatorData_t *bms_data);

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
      FaultStatus_t sendChargingMessage(uint16_t voltageToSet, uint16_t currentToSet, AccumulatorData_t *bms_data);

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

      /**
       * @brief sends out the calculated values of currents
       * 
       * @param discharge 
       * @param charge 
       * @param current 
       */
      void sendCurrentsStatus(uint16_t discharge, uint16_t charge, uint16_t current);
};

extern ComputeInterface compute;

#endif