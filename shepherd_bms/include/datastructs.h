#ifndef DATASTRUCTS_H
#define DATASTRUCTS_H

#include <nerduino.h>

/**
 * @brief Individual chip data
 * @note stores thermistor values, voltage readings, and the discharge status
 */
struct ChipData_t
{
    //These are retrieved from the initial LTC comms
    uint16_t voltageReading[12];          //store voltage readings from each chip
    uint16_t thermistorReading[32];       //store all therm readings from each chip
    bool discharge[12];
    FaultStatus_t errorReading;

    //These are calculated during the analysis of data
    uint16_t cellResistance[12];
    uint16_t openCellVoltage[12];

    bool thermsUpdated;
    bool voltagesUpdated;
};

/**
 * @brief Enuemrated possible fault codes for the BMS
 * @note  the values increase at powers of two to perform bitwise operations on a main fault code
 *          to set or get the error codes
 */
enum BMSFault_t
{
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

    MAX_FAULTS                          = 0x80000000 //Maximum allowable fault code
};

/**
 * @brief Stores critical values for the pack, and where that critical value can be found
 * 
 */
struct CriticalCellValue_t
{
    int16_t val;
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
    ChipData_t ChipData[8];

    FaultStatus_t faultStatus = NOT_FAULTED;

    int16_t packCurrent;
    uint16_t packVoltage;
    uint16_t packRes;

    /**
     * @brief Note that this is a 32 bit integer, so there are 32 max possible fault codes
     */
    uint32_t faultCode;

    /*Max and min thermistor readings*/
    CriticalCellValue_t maxTemp;
    CriticalCellValue_t minTemp;

    /*Max and min cell resistances*/
    CriticalCellValue_t maxRes;
    CriticalCellValue_t minRes;

    /*Max, min, and avg voltage of the cells*/
    CriticalCellValue_t maxVoltage;
    CriticalCellValue_t minVoltage;
};

#endif