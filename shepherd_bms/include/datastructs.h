#ifndef DATASTRUCTS_H
#define DATASTRUCTS_H

#include <nerduino.h>

/**
 * @brief Individual chip data
 * @note stores thermistor values, voltage readings, and the discharge status
 */
typedef struct ChipData_t
{
    uint16_t VoltageReading[12];          //store voltage readings from each chip
    uint16_t thermistorReading[32];       //store all therm readings from each chip
    bool discharge[12];
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
    CELL_ASIC_FAULT                     = 0x8,      //internal fault with the cell voltage measurement circuitry
    PACK_TOO_HOT                        = 0x10,
    CHARGE_INTERLOCK_FAULT              = 0x20,     //both Charge Power (pin 3) and Ready Power (pin 2) are energized at the same time
    OPEN_WIRING_FAULT                   = 0x40,     //cell tap wire is either weakly connected or not connected 
    PACK_VOLTAGE_MISMATCH               = 0x80,     //total pack voltage sensor did not match the sum of the individual cell voltage measurements
    INTERNAL_SOFTWARE_FAULT             = 0x100,
    INTERNAL_HEATSINK_THERMISTOR        = 0x200,    //internal hardware fault has occurredwith the internal thermistors that monitor the unit temperature
    INTERNAL_HARDWARE_FAULT             = 0x400,    
    CELL_BALANCING_STUCK_OFF            = 0x800,
    CURRENT_SENSOR_FAULT                = 0x1000,
    INTERNAL_CELL_COMM_FAULT            = 0x2000,
    LOW_CELL_VOLTAGE                    = 0x4000,   //voltage of a cell falls below 90 mV
    HIGH_VOLTAGE_ISOLATION_FAULT        = 0x8000,   //isolation breakdown between the high voltage battery and the BMS low voltage power ground
    PACK_VOLTAGE_SENSOR_FAULT           = 0x10000,  //fault code is set if the total pack voltage sensor reads zero volts
    WEAK_PACK_FAULT                     = 0x20000,
    FAN_MONITOR_FAULT                   = 0x40000,
    EXTERNAL_COMMUNICATION_FAULT        = 0x80000,
    DISCHARGE_LIMIT_ENFORCEMENT_FAULT   = 0x100000,
    CHARGER_SAFETY_RELAY                = 0x200000,
    BATTERY_THERMISTOR                  = 0x400000,

    MAX_FAULTS                          = 0x80000000 //Maximum allowable fault code
};


/**
 * @brief Stores critical values for the pack, and where that critical value can be found
 * 
 */
struct CrticalCellValue_t
{
    uint16_t val;
    uint8_t chipIndex;
    uint8_t cellNum;
};

/**
 * @brief Represents one "frame" of BMS data
 * 
 */
struct AccumulatorData_t
{
    /*Array of data from all chips in the system*/
    ChipData_t ChipData[8];

    FaultStatus_t faultStatus = NOT_FAULTED;

    uint16_t currentReading;

    /**
     * @brief Note that this is a 32 bit integer, so there are 32 max possible fault codes
     * 
     */
    uint32_t faultCode;

    /*Max and min thermistor readings*/
    CrticalCellValue_t maxTemp;
    CrticalCellValue_t minTemp;

    /*Max, min, and avg voltage of the cells*/
    CrticalCellValue_t maxVoltage;
    CrticalCellValue_t minVoltage;
    uint16_t avgVoltage;
};

#endif