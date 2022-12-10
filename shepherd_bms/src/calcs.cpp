#include "calcs.h"

void calcCellResistances(AccumulatorData_t *bmsdata)
{
    for(uint8_t c = 0; c < NUM_CHIPS; c++)
    {
        for(uint8_t cell = 0; cell < NUM_CELLS_PER_CHIP; cell++)
        {
            uint16_t cellTemp = bmsdata->ChipData[c].cellTemp[cell];
            uint16_t currentDraw = bmsdata->packCurrent;

            //TODO implement LUT for cell resistance
            //bmsdata->ChipData[c].cellResistance[cell] = LUT(cellTemp, currentDraw);
        }
    }
}

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
                tempSum += bmsdata->ChipData[c].thermistorReading[thermNum];
            }

            bmsdata->ChipData[c].cellTemp[cell] = tempSum / RelevantThermMap[cell].size();
        }
    }
}

int16_t calcCurrentLimit(AccumulatorData_t *bmsdata)
{
    int16_t currentLimit = 0;

    calcCellTemps(bmsdata);
    calcCellResistances(bmsdata);

    for(uint8_t c = 0; c < NUM_CHIPS; c++)
    {

    }

    return currentLimit;
}