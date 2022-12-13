#include "calcs.h"

void calcCellTemps(AccumulatorData_t *bmsdata)
{
    for(uint8_t c = 0; c < NUM_CHIPS; c++)
    {
        for(uint8_t cell = 0; cell < NUM_CELLS_PER_CHIP; cell++)
        {
            int tempSum = 0;
            for(uint8_t therm = 0; therm < RelevantThermMap[cell].size(); therm++)
            {
                uint8_t thermNum = RelevantThermMap[cell][therm];
                tempSum += bmsdata->chipData[c].thermistorReading[thermNum];
            }
        
            //Takes the average temperature of all the relevant thermistors
            bmsdata->chipData[c].cellTemp[cell] = tempSum / RelevantThermMap[cell].size();

            //Cleansing value
            if(bmsdata->chipData[c].cellTemp[cell] > MAX_TEMP) 
                bmsdata->chipData[c].cellTemp[cell] = MAX_TEMP;
        }
    }
}

void calcPackTemps(AccumulatorData_t *bmsdata)
{
    bmsdata->maxTemp = {MIN_TEMP, 0, 0};
    bmsdata->minTemp = {MAX_TEMP, 0, 0};
    int totalTemp = 0;
    for(uint8_t c = 0; c < NUM_CHIPS; c++)
    {
        for(uint8_t therm = 1; therm < 12; therm++) {
            if (bmsdata->chipData[c].thermistorReading[therm] > bmsdata->maxTemp.val) {
                bmsdata->maxTemp = {bmsdata->chipData[c].thermistorReading[therm], c, therm};
            } else if (bmsdata->chipData[c].thermistorReading[therm] < bmsdata->minTemp.val) {
                bmsdata->minTemp = {bmsdata->chipData[c].thermistorReading[therm], c, therm};
            }
            
            totalTemp += bmsdata->chipData[c].thermistorReading[therm];
        }
    }
    bmsdata->avgTemp = totalTemp / 44;
}

void calcCellResistances(AccumulatorData_t *bmsdata)
{
    for(uint8_t c = 0; c < NUM_CHIPS; c++)
    {
        for(uint8_t cell = 0; cell < NUM_CELLS_PER_CHIP; cell++)
        {
            uint8_t cellTemp = bmsdata->chipData[c].cellTemp[cell];
            uint8_t resIndex = cellTemp / 5;  //resistance LUT increments by 5C for each index
            
            bmsdata->chipData[c].cellResistance[cell] = TEMP_TO_CELL_RES[resIndex];

            //Linear interpolation to more accurately represent cell resistances in between increments of 5C
            if(cellTemp != MAX_TEMP)
            {
                float interpolation = (TEMP_TO_CELL_RES[resIndex+1] - TEMP_TO_CELL_RES[resIndex]) / 5;
                bmsdata->chipData[c].cellResistance[cell] += (interpolation * (cellTemp % 5));
            }
        }
    }
}

void calcDCL(AccumulatorData_t *bmsdata)
{
    int16_t currentLimit = 0xFFFF;

    for(uint8_t c = 0; c < NUM_CHIPS; c++)
    {
        for(uint8_t cell = 0; cell < NUM_CELLS_PER_CHIP; cell++)
        {
            uint8_t cellTemp = bmsdata->chipData[c].cellTemp[cell];
            uint8_t resIndex = (cellTemp - MIN_TEMP) / 5;  //resistance LUT increments by 5C for each index
            
            uint8_t tmpDCL = TEMP_TO_DCL[resIndex];

            //Linear interpolation to more accurately represent current in between increments of 5C
            if(cellTemp != MAX_TEMP)
            {
                float interpolation = (TEMP_TO_DCL[resIndex+1] - TEMP_TO_DCL[resIndex]) / 5;
                bmsdata->chipData[c].cellResistance[cell] += (interpolation * (cellTemp % 5));
            }

            //Taking the minimum DCL of all the cells
            if(tmpDCL < currentLimit) currentLimit = tmpDCL;
        }
    }

    bmsdata->dischargeLimit = currentLimit;
}