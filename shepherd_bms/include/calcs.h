#ifndef CALCS_H
#define CALCS_H

#include "datastructs.h"

using namespace std;

//In this case, the discharge rate is given by the battery capacity (in Ah) divided by the number of hours it takes to charge/discharge the battery.

const uint16_t TEMP_TO_CELL_RES[60] = 
{
    
};

/**
 * @brief Mapping the Relevant Thermistors for each cell based on cell #
 * 
 */
const std::vector<int> RelevantThermMap[NUM_CELLS_PER_CHIP] = 
{
    {1,2},
    {1,2},
    {1,2,3,4},
    {3,4},
    {3,4,5,6,7},
    {5,6,7,8,9},
    {8,9},
    {8,9,10,11},
    {10,11}
};

void calcCellResistances(AccumulatorData_t *bmsdata);

void calcCellTemps(AccumulatorData_t *bmsdata);

int16_t calcCurrentLimit(AccumulatorData_t *bmsdata);

#endif