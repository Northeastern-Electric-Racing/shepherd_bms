#include "analyzer.h"

Analyzer analyzer;

Analyzer::Analyzer(){}

Analyzer::~Analyzer(){}

void Analyzer::push(AccumulatorData_t *data)
{
    if(prevbmsdata != nullptr)
        delete prevbmsdata;

    prevbmsdata = bmsdata;
    bmsdata = data;

    disableTherms();

    calcCellTemps();
	calcPackTemps();
	calcPackVoltageStats();
	calcOpenCellVoltage();
	calcCellResistances();
	calcDCL();
	calcContDCL();
    calcCCL();
	calcContCCL();
	calcStateOfCharge();

    isFirstReading = false;
}

void Analyzer::calcCellTemps()
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
            {
                bmsdata->chipData[c].cellTemp[cell] = MAX_TEMP;
            }
        }
    }
}

void Analyzer::calcPackTemps()
{
    bmsdata->maxTemp = {MIN_TEMP, 0, 0};
    bmsdata->minTemp = {MAX_TEMP, 0, 0};
    int totalTemp = 0;
    for(uint8_t c = 1; c < NUM_CHIPS; c += 2)
    {
        for(uint8_t therm = 17; therm < 28; therm++) 
        {
            // finds out the maximum cell temp and location
            if (bmsdata->chipData[c].thermistorValue[therm] > bmsdata->maxTemp.val) 
            {
                bmsdata->maxTemp = {bmsdata->chipData[c].thermistorValue[therm], c, therm};
            }

            // finds out the minimum cell temp and location
            if (bmsdata->chipData[c].thermistorValue[therm] < bmsdata->minTemp.val) 
            {
                bmsdata->minTemp = {bmsdata->chipData[c].thermistorValue[therm], c, therm};
            }
            
            totalTemp += bmsdata->chipData[c].thermistorValue[therm];
        }
    }

    // takes the average of all the cell temperatures
    bmsdata->avgTemp = totalTemp / 44;
}

void Analyzer::calcPackVoltageStats() {
    bmsdata->maxVoltage = {MIN_VOLT_MEAS, 0, 0};
    bmsdata->minVoltage = {MAX_VOLT_MEAS, 0, 0};
    uint32_t totalVolt = 0;
    for(uint8_t c = 0; c < NUM_CHIPS; c++)
    {
        for(uint8_t cell = 0; cell < 9; cell++) 
        {
            // fings out the maximum cell voltage and location
            if (bmsdata->chipData[c].voltageReading[cell] > bmsdata->maxVoltage.val)
            {
                bmsdata->maxVoltage = {bmsdata->chipData[c].voltageReading[cell], c, cell};
            }

            //finds out the minimum cell voltage and location
            else if (bmsdata->chipData[c].voltageReading[cell] < bmsdata->minVoltage.val) 
            {
                bmsdata->minVoltage = {bmsdata->chipData[c].voltageReading[cell], c, cell};
            }
            
            totalVolt += bmsdata->chipData[c].voltageReading[cell];
        }
    }

    // calculate some voltage stats
    bmsdata->avgVoltage = totalVolt / (NUM_CELLS_PER_CHIP * NUM_CHIPS);
    bmsdata->packVoltage = totalVolt / 1000; // convert to voltage * 10
    bmsdata->deltVoltage = bmsdata->maxVoltage.val - bmsdata->minVoltage.val;
}

void Analyzer::calcCellResistances()
{
    for(uint8_t c = 0; c < NUM_CHIPS; c++)
    {
        for(uint8_t cell = 0; cell < NUM_CELLS_PER_CHIP; cell++)
        {
            uint8_t cellTemp = bmsdata->chipData[c].cellTemp[cell];
            uint8_t resIndex = (cellTemp - MIN_TEMP) / 5;  //resistance LUT increments by 5C for each index
            
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

void Analyzer::calcDCL()
{
    int16_t currentLimit = 0x7FFF;

    for(uint8_t c = 0; c < NUM_CHIPS; c++)
    {
        for(uint8_t cell = 0; cell < NUM_CELLS_PER_CHIP; cell++)
        {   
            // Apply equation
            uint16_t tmpDCL = (bmsdata->chipData[c].openCellVoltage[cell] - ((MIN_VOLT + VOLT_SAG_MARGIN) * 10000)) / (bmsdata->chipData[c].cellResistance[cell] * 10);
            // Multiplying resistance by 10 to convert from mOhm to Ohm and then to Ohm * 10000 to account for the voltage units

            //Taking the minimum DCL of all the cells
            if(tmpDCL < currentLimit) currentLimit = tmpDCL;
        }
    }

    // ceiling for current limit
    if (currentLimit > MAX_CELL_CURR) 
    {
        bmsdata->dischargeLimit = MAX_CELL_CURR;
    }
    else 
    {
        bmsdata->dischargeLimit = currentLimit;
    }
}

void Analyzer::calcContDCL()
{
    uint8_t minResIndex = (bmsdata->minTemp.val - MIN_TEMP) / 5;  //resistance LUT increments by 5C for each index
    uint8_t maxResIndex = (bmsdata->maxTemp.val - MIN_TEMP) / 5;

    if (TEMP_TO_DCL[minResIndex] < TEMP_TO_DCL[maxResIndex])
    {
        bmsdata->contDCL = TEMP_TO_DCL[minResIndex];
    }
    else
    {
        bmsdata->contDCL = TEMP_TO_DCL[maxResIndex];
    }
}

void Analyzer::calcCCL()
{
    int16_t currentLimit = 0x7FFF;

    for(uint8_t c = 0; c < NUM_CHIPS; c++)
    {
        for(uint8_t cell = 0; cell < NUM_CELLS_PER_CHIP; cell++)
        {   
            // Apply equation
            uint16_t tmpCCL = (MAX_VOLT - ((bmsdata->chipData[c].openCellVoltage[cell] + VOLT_SAG_MARGIN) * 10000)) / (bmsdata->chipData[c].cellResistance[cell] * 10);
            // Multiplying resistance by 10 to convert from mOhm to Ohm and then to Ohm * 10000 to account for the voltage units

            //Taking the minimum DCL of all the cells
            if(tmpCCL < currentLimit) currentLimit = tmpCCL;
        }
    }

    // ceiling for current limit
    if (currentLimit > MAX_CELL_CURR) 
    {
        bmsdata->dischargeLimit = MAX_CELL_CURR;
    }
    else 
    {
        bmsdata->dischargeLimit = currentLimit;
    }
}

void Analyzer::calcContCCL()
{
    uint8_t minResIndex = (bmsdata->minTemp.val - MIN_TEMP) / 5;  //resistance LUT increments by 5C for each index
    uint8_t maxResIndex = (bmsdata->maxTemp.val - MIN_TEMP) / 5;

    if (TEMP_TO_CCL[minResIndex] < TEMP_TO_CCL[maxResIndex])
    {
        bmsdata->chargeLimit = TEMP_TO_CCL[minResIndex];
    }
    else
    {
        bmsdata->chargeLimit = TEMP_TO_CCL[maxResIndex];
    }
}

void Analyzer::calcOpenCellVoltage() 
{
    // if there is no previous data point, set inital open cell voltage to current reading
    if (isFirstReading)
    {
        for (uint8_t chip = 0; chip < NUM_CHIPS; chip++) 
        {
            for (uint8_t cell = 0; cell < NUM_CELLS_PER_CHIP; cell++) 
            {
                bmsdata->chipData[chip].openCellVoltage[cell] = bmsdata->chipData[chip].voltageReading[cell];
            }
        }
    }
    // If we are within the current threshold for open voltage measurments
     else if (bmsdata->packCurrent < (OCV_CURR_THRESH * 10) && bmsdata->packCurrent > (-OCV_CURR_THRESH * 10)) 
    {
        for (uint8_t chip = 0; chip < NUM_CHIPS; chip++) 
        {
            for (uint8_t cell = 0; cell < NUM_CELLS_PER_CHIP; cell++) 
            {
                // Sets open cell voltage to a moving average of OCV_AVG values
                bmsdata->chipData[chip].openCellVoltage[cell] = (uint32_t(bmsdata->chipData[chip].voltageReading[cell]) + (uint32_t(prevbmsdata->chipData[chip].openCellVoltage[cell])  * (OCV_AVG - 1))) / OCV_AVG;
            }
        }
    } 
    else
    {
        for (uint8_t chip = 0; chip < NUM_CHIPS; chip++) 
        {
            for (uint8_t cell = 0; cell < NUM_CELLS_PER_CHIP; cell++) 
            {
                // Set OCV to the previous/existing OCV
                bmsdata->chipData[chip].openCellVoltage[cell] = prevbmsdata->chipData[chip].openCellVoltage[cell];
            }
        }
    }
}

uint8_t Analyzer::calcFanPWM()
{
    // Resistance LUT increments by 5C for each index, plus we account for negative minimum
    uint8_t minResIndex = (bmsdata->maxTemp.val - MIN_TEMP) / 5;
    // Ints are roounded down, so this would be the value if rounded up
    uint8_t maxResIndex = (bmsdata->maxTemp.val - MIN_TEMP) / 5 + 1;
    // Determine how far into the 5C interval the temp is
    uint8_t partOfIndex = (bmsdata->maxTemp.val - MIN_TEMP) % 5;

    // Uses fan LUT and finds low and upper end. Then takes average, weighted to how far into the interval the exact temp is
    return ((FAN_CURVE[maxResIndex] * partOfIndex) + (FAN_CURVE[minResIndex] * (5 - partOfIndex))) / (2 * 5);
}

void Analyzer::disableTherms()
{
    int8_t tempRepl = 25; // Iniitalize to room temp (necessary to stabilize when the BMS first boots up/has null values)
    if (!isFirstReading) tempRepl = prevbmsdata->avgTemp; // Set to actual average temp of the pack

    for(uint8_t c = 1; c < NUM_CHIPS; c+= 2)
    {
        for(uint8_t therm = 17; therm < 28; therm++)
        {
            // If 2D LUT shows therm should be disable
            if (THERM_DISABLE[(c - 1) / 2][therm - 17])
            {
                // Nullify thermistor by setting to pack average
                bmsdata->chipData[c].thermistorValue[therm] = tempRepl;
            }
        }
    }
}

void Analyzer::calcStateOfCharge()
{
    int index = (((bmsdata->minVoltage.val) - MIN_VOLT) / .1);

    // .1 = 1.3V range / 13 datapoints on curve
    if (index >= 13) 
        bmsdata->soc = 100;
    else
    {
        float distance_from_higher = (bmsdata->minVoltage.val) - ((index / 10) + 2.9);
        bmsdata->soc = ((distance_from_higher*STATE_OF_CHARGE_CURVE[index+1]) + ((1-distance_from_higher)*STATE_OF_CHARGE_CURVE[index]));
    }
}