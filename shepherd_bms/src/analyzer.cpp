#include "analyzer.h"

Analyzer analyzer;

Analyzer::Analyzer(){}

Analyzer::Analyzer(uint16_t newSize)
{
    resize(newSize);
}

Analyzer::~Analyzer()
{
    while(tail != nullptr)
    {
        //remove the last item
        node *tmp = tail;
        tail = tail->next;
        delete tmp;
        tmp = nullptr;
    }
}

void Analyzer::push(AccumulatorData_t data)
{
    //make sure we have waited long enough
    if(!analysisTimer.isTimerExpired())
    {
        //add in a new data point
        enQueue(data);

        //perform calculations on the dataset
        calcCellTemps();
        calcPackTemps();
        calcPackVoltageStats();
        calcCellResistances();
        calcDCL();
        calcContDCL();
        calcContCCL();

        printData();

        //start the timer to make sure we aren't doing repeat analysis on exactly the same data
        analysisTimer.startTimer(ANALYSIS_INTERVAL);
    }
}

void Analyzer::printData()
{
    Serial.print("Min, Max, Avg Temps: ");
	Serial.print(bmsdata->minTemp.val);
	Serial.print(",  ");
	Serial.print(bmsdata->maxTemp.val);
	Serial.print(",  ");
	Serial.println(bmsdata->avgTemp);
	Serial.print("Min, Max, Avg, Delta Voltages: ");
	Serial.print(bmsdata->minVoltage.val);
	Serial.print(",  ");
	Serial.print(bmsdata->maxVoltage.val);
	Serial.print(",  ");
	Serial.print(bmsdata->avgVoltage);
	Serial.print(",  ");
	Serial.println(bmsdata->deltVoltage);
	Serial.print("DCL: ");
	Serial.println(bmsdata->dischargeLimit);
	Serial.print("CCL: ");
	Serial.println(bmsdata->chargeLimit);

    /*
	Serial.println("Cell Temps:");
	for(uint8_t c = 0; c < NUM_CHIPS; c++)
    {
        for(uint8_t cell = 17; cell < 28; cell++)
        {
			Serial.print(accData->chipData[c].thermistorReading[cell]);
			Serial.print("\t");
		}
		Serial.println();
	}

	Serial.println("Cell Temps Avg:");
	for(uint8_t c = 0; c < NUM_CHIPS; c++)
    {
        for(uint8_t cell = 17; cell < 28; cell++)
        {
			Serial.print(accData->chipData[c].thermistorValue[cell]);
			Serial.print("\t");
		}
		Serial.println();
	}*/
}

void Analyzer::resize(uint16_t newSize)
{
    size = newSize;

    if(head == nullptr || numNodes < size) return;

    //if the queue is longer than the new size, we need to delete nodes
    while(numNodes > size)
    {
        //remove the last item
        node *tmp = tail;
        tail = tail->next;
        delete tmp;
        tmp = nullptr;
        numNodes--;
    }
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
                bmsdata->chipData[c].cellTemp[cell] = MAX_TEMP;
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

void Analyzer::calcPackVoltageStats() {
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

void Analyzer::calcCellResistances()
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

void Analyzer::calcDCL()
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

void Analyzer::calcContDCL()
{
    uint8_t minResIndex = (bmsdata->minTemp.val - MIN_TEMP) / 5;  //resistance LUT increments by 5C for each index
    uint8_t maxResIndex = (bmsdata->maxTemp.val - MIN_TEMP) / 5;

    if (TEMP_TO_DCL[minResIndex] < TEMP_TO_DCL[maxResIndex]) {
        bmsdata->contDCL = TEMP_TO_DCL[minResIndex];
    } else {
        bmsdata->contDCL = TEMP_TO_DCL[maxResIndex];
    }
}

void Analyzer::calcContCCL()
{
    uint8_t minResIndex = (bmsdata->minTemp.val - MIN_TEMP) / 5;  //resistance LUT increments by 5C for each index
    uint8_t maxResIndex = (bmsdata->maxTemp.val - MIN_TEMP) / 5;

    if (TEMP_TO_CCL[minResIndex] < TEMP_TO_CCL[maxResIndex]) {
        bmsdata->chargeLimit = TEMP_TO_CCL[minResIndex];
    } else {
        bmsdata->chargeLimit = TEMP_TO_CCL[maxResIndex];
    }
}

void Analyzer::enQueue(AccumulatorData_t data)
{
    node *newNode = new node;
    newNode->data = data;

    //if list is empty
    if (head == nullptr)
    {
        newNode->next = nullptr;
        head = newNode;
        tail = newNode;
        numNodes++;
        return;
    }

    //if the list is not full yet
    if(numNodes < size)
    {
        //add in item
        head->next = newNode;
        head = newNode;
        numNodes++;
    }
    //else if the list is full
    else
    {
        //add in item
        head->next = newNode;
        head = newNode;

        //remove the last item
        node *tmp = tail;
        tail = tail->next;
        delete tmp;
        tmp = nullptr;
    }
}