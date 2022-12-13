#ifndef CALCS_H
#define CALCS_H

#include "datastructs.h"

using namespace std;


/**
 * @brief Mapping Cell temperature to the cell resistance based on the 
 *      nominal cell resistance curve profile of the Samsung 186500 INR in the 
 *      Orion BMS software utility app
 * 
 * @note Units are in mOhms and indicies are in (degrees C)/5, stops at 65C
 * @note Resistance should be *interpolated* from these values (i.e. if we are
 *      at 27C, we should take the resistance that is halfway between 25C and 30C)
 */
const float TEMP_TO_CELL_RES[14] = 
{
    10.17, 9.34, 8.78, 8.44, 8.25, 8.18, 8.17,
    8.16, 8.14, 8.06, 7.83, 7.42, 6.8, 6.8
};

/**
 * @brief Mapping Cell temperatue to the discharge current limit based on the 
 *      temperature discharge limit curve profile of the Samsung 186500 INR
 *      in the Orion BMS software utility app
 * 
 * @note Units are in Amps and indicies are in (degrees C)/5, stops at 65C
 * @note Limit should be *interpolated* from these values (i.e. if we are
 *      at 27C, we should take the limit that is halfway between 25C and 30C)
 * 
 */
const uint8_t TEMP_TO_DCL[14] =
{
    65, 85, 105, 105, 105, 105, 
    105, 105, 105, 55, 5, 0, 0, 0
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

void calcPackTemps(AccumulatorData_t *bmsdata);

void calcDCL(AccumulatorData_t *bmsdata);

#endif