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
                tempSum += bmsdata->chipData[c].thermistorValue[thermNum];
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
    for(uint8_t c = 1; c < NUM_CHIPS; c += 2)
    {
        for(uint8_t therm = 17; therm < 28; therm++) {
            if (bmsdata->chipData[c].thermistorValue[therm] > bmsdata->maxTemp.val) {
                bmsdata->maxTemp = {bmsdata->chipData[c].thermistorValue[therm], c, therm};
            } else if (bmsdata->chipData[c].thermistorValue[therm] < bmsdata->minTemp.val) {
                bmsdata->minTemp = {bmsdata->chipData[c].thermistorValue[therm], c, therm};
            }
            
            totalTemp += bmsdata->chipData[c].thermistorValue[therm];
        }
    }
    bmsdata->avgTemp = totalTemp / 44;
}

void calcPackVoltageStats(AccumulatorData_t *bmsdata) {
    bmsdata->maxVoltage = {MIN_VOLT_MEAS, 0, 0};
    bmsdata->minVoltage = {MAX_VOLT_MEAS, 0, 0};
    int totalVolt = 0;
    for(uint8_t c = 0; c < NUM_CHIPS; c++)
    {
        for(uint8_t cell = 0; cell < 9; cell++) {
            if (bmsdata->chipData[c].voltageReading[cell] > bmsdata->maxVoltage.val) {
                bmsdata->maxVoltage = {bmsdata->chipData[c].voltageReading[cell], c, cell};
            } else if (bmsdata->chipData[c].voltageReading[cell] < bmsdata->minVoltage.val) {
                bmsdata->minVoltage = {bmsdata->chipData[c].voltageReading[cell], c, cell};
            }
            
            totalVolt += bmsdata->chipData[c].voltageReading[cell];
        }
    }
    bmsdata->avgVoltage = totalVolt / (NUM_CELLS_PER_CHIP * NUM_CHIPS);
    bmsdata->deltVoltage = bmsdata->maxVoltage.val - bmsdata->minVoltage.val;
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
    int16_t currentLimit = 0x7FFF;

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

void calcContDCL(AccumulatorData_t *bmsdata)
{
    uint8_t minResIndex = (bmsdata->minTemp.val - MIN_TEMP) / 5;  //resistance LUT increments by 5C for each index
    uint8_t maxResIndex = (bmsdata->maxTemp.val - MIN_TEMP) / 5;

    if (TEMP_TO_DCL[minResIndex] < TEMP_TO_DCL[maxResIndex]) {
        bmsdata->contDCL = TEMP_TO_DCL[minResIndex];
    } else {
        bmsdata->contDCL = TEMP_TO_DCL[maxResIndex];
    }
}

void calcContCCL(AccumulatorData_t *bmsdata)
{
    uint8_t minResIndex = (bmsdata->minTemp.val - MIN_TEMP) / 5;  //resistance LUT increments by 5C for each index
    uint8_t maxResIndex = (bmsdata->maxTemp.val - MIN_TEMP) / 5;

    if (TEMP_TO_CCL[minResIndex] < TEMP_TO_CCL[maxResIndex]) {
        bmsdata->chargeLimit = TEMP_TO_CCL[minResIndex];
    } else {
        bmsdata->chargeLimit = TEMP_TO_CCL[maxResIndex];
    }
}