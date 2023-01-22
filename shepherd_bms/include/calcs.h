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
const float TEMP_TO_CELL_RES[16] = 
{
    5.52, 5.52, 5.52, 4.84, 4.27, 3.68, 3.16, 2.74, 2.4,
    2.12, 1.98, 1.92, 1.90, 1.90, 1.90, 1.90
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
const uint8_t TEMP_TO_DCL[16] =
{
    0, 40, 110, 125, 140, 140, 140, 140, 
    140, 140, 140, 100, 60, 20, 0, 0
};

/**
 * @brief Mapping Cell temperatue to the charge current limit based on the 
 *      temperature charge limit curve profile of the Samsung 186500 INR
 *      in the Orion BMS software utility app
 * 
 * @note Units are in Amps and indicies are in (degrees C)/5, stops at 65C
 * @note Limit should be *interpolated* from these values (i.e. if we are
 *      at 27C, we should take the limit that is halfway between 25C and 30C)
 * 
 */
const uint8_t TEMP_TO_CCL[16] =
{
    0, 0, 0, 25, 25, 25, 25, 25, 25, 25,
    20, 15, 10, 5, 1, 1
};

/**
 * @brief Lookup table for State of Charge
 * 
 * @note Each index corresponds to 10% charge
 * 
 * @note Assuming linear curve
 */
const uint8_t STATE_OF_CHARGE_CURVE[13] =
{
    0, 8, 16, 24, 32, 40, 48, 56, 64, 72, 80, 88, 100
};

/**
 * @brief Mapping desired fan speed PWM to the cell temperature
 * 
 * @note Units are in PWM out of 255 and indicies are in (degrees C)/5, stops at 65C
 * @note Limit should be *interpolated* from these values (i.e. if we are
 *      at 27C, we should take the limit that is halfway between 25C and 30C)
 * 
 */
const uint8_t FAN_CURVE[16] =
{
    0, 0, 0, 0, 0, 0, 0, 0, 32, 64,
    128, 255, 255, 255, 255, 255
};

/**
 * @brief Selecting thermistors to ignore
 * 
 * @note True / 1 will disable the thermistor
 * 
 */
const uint8_t THERM_DISABLE[4][11] =
{
    {0,0,0,0,1,0,0,0,0,0,0},
    {1,0,0,0,0,0,0,0,0,0,0},
    {1,1,0,0,0,0,0,0,0,0,0},
    {1,1,0,0,0,0,0,0,0,0,0},
};

/**
 * @brief Mapping the Relevant Thermistors for each cell based on cell #
 * 
 */
const std::vector<int> RelevantThermMap[NUM_CELLS_PER_CHIP] = 
{
    {17,18},
    {17,18},
    {17,18,19,20},
    {19,20},
    {19,20,21,22,23},
    {21,22,23,24,25},
    {24,25},
    {24,25,26,27},
    {26,27}
};

void calcPackVoltageStats(AccumulatorData_t *bmsdata);

void calcCellResistances(AccumulatorData_t *bmsdata);

void calcCellTemps(AccumulatorData_t *bmsdata);

void calcPackTemps(AccumulatorData_t *bmsdata);

void calcDCL(AccumulatorData_t *bmsdata);

void calcContDCL(AccumulatorData_t *bmsdata);

void calcContCCL(AccumulatorData_t *bmsdata);

void calcOpenCellVoltage(AccumulatorData_t *bmsdata, AccumulatorData_t *prevbmsdata);

uint8_t calcFanPWM(AccumulatorData_t *bmsdata);

void disableTherms(AccumulatorData_t *bmsdata, AccumulatorData_t *prevbmsdata);

uint8_t calcStateOfCharge(AccumulatorData_t *bmsdata);

#endif