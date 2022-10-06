#ifndef DATASTRUCTS_H
#define DATASTRUCTS_H

#include <nerduino.h>

/*individual segment data*/
typedef struct ChipData_t
{
double VoltageReading[12];          //store voltage readings from each chip
double thermistorReading[32];       //store all therm readings from each chip
bool discharge[12];
};

/*One frame of data from the accumulator*/
struct AccumulatorData_t
{
    ChipData_t ChipData[8];

    /*max and min thermistor readings*/
    double maxTemp;
    double minTemp;

    /*max, min, and avg voltage of the cells*/
    double maxVoltage;
    double minVoltage;
    double avgVoltage;

    FaultStatus_t faulted = NOT_FAULTED;


    double currentReading;
};


#endif