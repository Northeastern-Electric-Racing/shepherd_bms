#ifndef DATASTRUCTS_H
#define DATASTRUCTS_H

#include <nerduino.h>



/*Individual chip data*/
typedef struct ChipData_t
{
    uint16_t VoltageReading[12];          //store voltage readings from each chip
    uint16_t thermistorReading[32];       //store all therm readings from each chip
    bool discharge[12];
};

typedef enum BMSFault_t
{
    //Orion BMS faults
    CELLS_NOT_BALANCING;
    CELL_VOLTAGE_TOO_HIGH;
    CELL_VOLTAGE_TOO_LOW;
    CELL_ASIC_FAULT;                    //internal fault with the cell voltage measurement circuitry
    PACK_TOO_HOT;
    CHARGE_INTERLOCK_FAULT;             //both Charge Power (pin 3) and Ready Power (pin 2) are energized at the same time
    OPEN_WIRING_FAULT;                  //cell tap wire is either weakly connected or not connected 
    PACK_VOLTAGE_MISMATCH;              //total pack voltage sensor did not match the sum of the individual cell voltage measurements
    INTERNAL_SOFTWARE_FAULT;
    INTERNAL_HEATSINK_THERMISTOR;       //internal hardware fault has occurredwith the internal thermistors that monitor the unit temperature
    INTERNAL_HARDWARE_FAULT;
    CELL_BALANCING_STUCK_OFF;
    CURRENT_SENSOR_FAULT;
    INTERNAL_CELL_COMM_FAULT;
    LOW_CELL_VOLTAGE;                   //voltage of a cell falls below 90 mV
    HIGH_VOLTAGE_ISOLATION_FAULT;       //isolation breakdown between the high voltage battery and the BMS low voltage power ground
    PACK_VOLTAGE_SENSOR_FAULT;          //fault code is set if the total pack voltage sensor reads zero volts
    WEAK_PACK_FAULT;
    FAN_MONITOR_FAULT;
    EXTERNAL_COMMUNICATION_FAULT;
    DISCHARGE_LIMIT_ENFORCEMENT_FAULT;
    CHARGER_SAFETY_RELAY;
    BATTERY_THERMISTOR;
};

/*One frame of data from the accumulator*/
struct AccumulatorData_t
{
    /*Array of data from all chips in the system*/
    ChipData_t ChipData[8];

    FaultStatus_t faultStatus = NOT_FAULTED;

    uint16_t currentReading;

    BMSFault_t;

    /*Max and min thermistor readings*/
    uint16_t maxTemp;
    uint16_t minTemp;

    /*Max, min, and avg voltage of the cells*/
    uint16_t maxVoltage;
    uint16_t minVoltage;
    uint16_t avgVoltage;
};

#endif