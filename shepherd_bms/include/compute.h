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


#define MAX_ADC_RESOLUTION    1023 // 13 bit ADC

class ComputeInterface
{
   private:
      uint8_t fan_speed_;
      bool is_charging_enabled_;

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
      * @param is_enabled
      */
      void enableCharging(bool enable_charging);

      /**
      * @brief sends charger message
      * 
      * @param voltage_to_set
      * @param currentToSet
      *
      * @return Returns a fault if we are not able to communicate with charger
      */
      FaultStatus_t sendChargingMessage(uint16_t voltage_to_set, AccumulatorData_t *bms_data);

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
      * @param new_fan_speed
      */
      void setFanSpeed(uint8_t new_fan_speed);

      /**
      * @brief Returns the pack current sensor reading
      *
      * @return int16_t
      */
      int16_t getPackCurrent();

      /**
      * @brief sends max charge/discharge current to Motor Controller
      *
      * @param max_charge
      * @param max_discharge
      */
      void sendMCMsg(uint16_t max_charge, uint16_t max_discharge);

      /**
      * @brief updates fault relay
      *
      * @param fault_state
      */
      void setFault(FaultStatus_t fault_state);

      /**
      * @brief sends acc status message
      * 
      * @param voltage
      * @param current
      * @param ah
      * @param soc
      * @param health
      *
      * @return Returns a fault if we are not able to send
      */
      void sendAccStatusMessage(uint16_t voltage, int16_t current, uint16_t ah, uint8_t soc, uint8_t health);

      /**
      * @brief sends BMS status message 
      * 
      * @param failsafe
      * @param dtc_1
      * @param dtc_2
      * @param current_limit
      * @param avg_temp
      * @param internal_temp
      * 
      * @return Returns a fault if we are not able to send
      */
      void sendBMSStatusMessage(uint8_t failsafe, uint8_t dtc_1, uint16_t dtc_2, uint16_t current_limit, int8_t avg_temp, int8_t internal_temp);

      /**
      * @brief sends shutdown control message
      * 
      * @param mpe_state
      *
      * @return Returns a fault if we are not able to send
      */
      void sendShutdownControlMessage(uint8_t mpe_state);

      /**
      * @brief sends cell data message
      * 
      * @param hv
      * @param hv_id
      * @param lv
      * @param lv_id
      * @param avg_volt
      *
      * @return Returns a fault if we are not able to send
      */
      void sendCellDataMessage(uint16_t hv, uint8_t hv_id, uint16_t lv, uint8_t lv_id, uint16_t avg_volt);

      /**
      * @brief sends cell voltage message
      * 
      * @param cell_id
      * @param instant_volt
      * @param internal_res
      * @param shunted
      * @param open_voltage
      *
      * @return Returns a fault if we are not able to send
      */
      void sendCellVoltageMessage(uint8_t cell_id, uint16_t instant_volt, uint16_t internal_res, uint8_t shunted, uint16_t open_voltage);

      /**
      * @brief sends "is charging" message
      * 
      * @param charging_status
      *
      * @return Returns a fault if we are not able to send
      */
      void sendChargingStatus(bool charging_status);

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