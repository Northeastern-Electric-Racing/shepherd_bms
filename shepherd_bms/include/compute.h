#ifndef COMPUTE_H
#define COMPUTE_H

#include "datastructs.h"
#include "nerduino.h"
#include "canMsgHandler.h"
#include "stateMachine.h"

#define CURRENT_SENSOR_PIN_L  A1
#define CURRENT_SENSOR_PIN_H  A0
#define MEAS_5VREF_PIN        A7
#define FAULT_PIN             2
#define CHARGE_SAFETY_RELAY   4
#define CHARGE_DETECT         5
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
      * @brief Returns if charger interlock is engaged, indicating charger LV connector is plugged in
      *
      * @return true
      * @return false
      */
      bool chargerConnected();

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
      * @param bms_state
      * @param fault_status
      * @param tempAvg
      * @param tempInternal
      *
      * @return Returns a fault if we are not able to send
      */
      void sendBMSStatusMessage(int bms_state, uint32_t fault_status, int8_t avg_temp, int8_t internal_temp, bool balance);

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
      * @param high_voltage
      * @param low_voltage
      * @param avg_voltage
      *
      * @return Returns a fault if we are not able to send
      */
      void sendCellDataMessage(CriticalCellValue_t high_voltage, CriticalCellValue_t low_voltage, uint16_t avg_voltage);

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
       * @brief sends out the calculated values of currents
       *
       * @param discharge
       * @param charge
       * @param current
       */
      void sendCurrentsStatus(uint16_t discharge, uint16_t charge, uint16_t current);

        /**
       * @brief sends cell temperature message
       *
       *
       *
       * @return Returns a fault if we are not able to send
      */
      void sendCellTemp(CriticalCellValue_t max_cell_temp, CriticalCellValue_t min_cell_temp, uint16_t avg_temp);

      /**
       * @brief sends the average segment temperatures
       * 
       * 
       * 
       * @return Returns a fault if we are not able to send
       */
      void sendSegmentTemps(int8_t segmentTemps[NUM_SEGMENTS]);

      void sendDclPreFault(bool prefault);
};

extern ComputeInterface compute;

#endif
