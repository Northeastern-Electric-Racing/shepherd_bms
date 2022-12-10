#ifndef CALCS_H
#define CALCS_H

#include "datastructs.h"

using namespace std;

//In this case, the discharge rate is given by the battery capacity (in Ah) divided by the number of hours it takes to charge/discharge the battery.

const uint16_t TEMP_TO_CELL_RES[60] = 
{
    
};

void calcCellResistances(AccumulatorData_t *bmsdata);

void calcCellTemps(AccumulatorData_t *bmsdata);

int16_t calcCurrentLimit(AccumulatorData_t *bmsdata);

#endif