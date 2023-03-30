#ifndef DATASTRUCTS_H
#define DATASTRUCTS_H

#include <nerduino.h>
#include "bmsConfig.h"
#include <vector>

/**
 * @brief Individual chip data
 * @note stores thermistor values, voltage readings, and the discharge status
 */
struct ChipData_t
{
    //These are retrieved from the initial LTC comms
    uint16_t voltage_reading[NUM_CELLS_PER_CHIP];          //store voltage readings from each chip
    int8_t thermistor_reading[NUM_THERMS_PER_CHIP];       //store all therm readings from each chip
    int8_t thermistor_value[NUM_THERMS_PER_CHIP];
    FaultStatus_t error_reading;

    //These are calculated during the analysis of data
    int8_t cell_temp[NUM_CELLS_PER_CHIP];
    float cell_resistance[NUM_CELLS_PER_CHIP];
    uint16_t open_cell_voltage[NUM_CELLS_PER_CHIP];
};

/**
 * @brief Enuemrated possible fault codes for the BMS
 * @note  the values increase at powers of two to perform bitwise operations on a main fault code
 *          to set or get the error codes
 */
enum BMSFault_t
{
    FAULTS_CLEAR                        = 0x0,

    //Orion BMS faults
    CELLS_NOT_BALANCING                 = 0x1,
    CELL_VOLTAGE_TOO_HIGH               = 0x2,
    CELL_VOLTAGE_TOO_LOW                = 0x4,
    PACK_TOO_HOT                        = 0x8,
    OPEN_WIRING_FAULT                   = 0x10,    //cell tap wire is either weakly connected or not connected
    INTERNAL_SOFTWARE_FAULT             = 0x20,    //general software fault
    INTERNAL_THERMAL_ERROR              = 0x40,    //internal hardware fault reulting from too hot of a temperature reading from NERduino
    INTERNAL_CELL_COMM_FAULT            = 0x80,    //this is due to an invalid CRC from retrieving values
    CURRENT_SENSOR_FAULT                = 0x100,
    CHARGE_READING_MISMATCH             = 0x200,   //we detect that there is a charge voltage, but we are not supposed to be charging
    LOW_CELL_VOLTAGE                    = 0x400,   //voltage of a cell falls below 90 mV
    WEAK_PACK_FAULT                     = 0x800,
    EXTERNAL_CAN_FAULT                  = 0x1000,
    DISCHARGE_LIMIT_ENFORCEMENT_FAULT   = 0x2000,
    CHARGER_SAFETY_RELAY                = 0x4000,
    BATTERY_THERMISTOR                  = 0x8000,
    CHARGER_CAN_FAULT                   = 0x10000,
    CHARGE_LIMIT_ENFORCEMENT_FAULT      = 0x20000,

    MAX_FAULTS                          = 0x80000000 //Maximum allowable fault code
};

/**
 * @brief Stores critical values for the pack, and where that critical value can be found
 *
 */
struct CriticalCellValue_t
{
    int32_t val;
    uint8_t chipIndex;
    uint8_t cellNum;
};

/**
 * @brief Represents one "frame" of BMS data
 * @note the size of this structure is **9752 bits** (~1.3k bytes), as of October 22, 2022
 */
#define ACCUMULATOR_FRAME_SIZE sizeof(AccumulatorData_t);

struct AccumulatorData_t
{
    /*Array of data from all chips in the system*/
    ChipData_t chip_data[NUM_CHIPS];

    FaultStatus_t fault_status = NOT_FAULTED;

    int16_t pack_current;                        // this value is multiplied by 10 to account for decimal precision
    uint16_t pack_voltage;
    uint16_t pack_res;

    uint16_t discharge_limit;
    uint16_t charge_limit;
    uint16_t cont_DCL;
    uint8_t soc;

    int8_t segment_average_temps[NUM_SEGMENTS];

    /**
     * @brief Note that this is a 32 bit integer, so there are 32 max possible fault codes
     */
    uint32_t fault_code;

    /*Max, min, and avg thermistor readings*/
    CriticalCellValue_t max_temp;
    CriticalCellValue_t min_temp;
    uint8_t avg_temp;

    /*Max and min cell resistances*/
    CriticalCellValue_t max_res;
    CriticalCellValue_t min_res;

    /*Max, min, and avg voltage of the cells*/
    CriticalCellValue_t max_voltage;
    CriticalCellValue_t min_voltage;
    uint16_t avg_voltage;
    uint16_t delt_voltage;
};

#endif