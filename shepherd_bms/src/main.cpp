#include <nerduino.h>
#include <LTC68041.h>
#include "segment.h"
#include "compute.h"
#include "calcs.h"
#include "datastructs.h"

int currTime = 0;
int lastPackCurr = 0;
int lastVoltTemp = 0;

bool dischargeEnabled = false;
uint16_t cellTestIter = 0;
bool dischargeConfig[NUM_CHIPS][NUM_CELLS_PER_CHIP] = {};

ChipData_t *testData;
Timer mainTimer;
ComputeInterface compute;

BMSFault_t bmsFault = FAULTS_CLEAR;

void testSegments()
{
	currTime = millis();
	if (lastVoltTemp + 1000 < currTime) {
		lastVoltTemp = currTime;
		//Handle Segment data collection test logic
		testData = new ChipData_t[NUM_CHIPS];

		//Retrieve ALL segment data (therms and voltage)
		segment.retrieveSegmentData(testData);

		for (int chip = 0; chip < NUM_CHIPS; chip++)
		{
			for (int cell=0; cell < NUM_CELLS_PER_CHIP; cell++)
			{
				Serial.print(testData[chip].voltageReading[cell]);
				Serial.print("\t");
			}

			Serial.println(); //newline

			for (int therm=17; therm < 28; therm++)
			{
				Serial.print(testData[chip].thermistorReading[therm]);
				Serial.print("\t");
				if (therm == 15) Serial.println();
			}
			Serial.println(); //newline
		}
		Serial.println(); //newline
		delete[] testData;
		testData = nullptr;

		//Handle discharge test logic
		if (Serial.available()) 
		{ // Check for key presses
			char keyPress = Serial.read(); // Read key
			if (keyPress == ' ') 
			{
				Serial.println(dischargeEnabled ? "STOPPING DISCHARGE COUNTING..." : "STARTING DISCHARGE COUNTING...");
				dischargeEnabled = !dischargeEnabled;
			}
		}
		
		if(dischargeEnabled)
		{ 	
			for (uint8_t c = 0; c < NUM_CHIPS; c++)
			{
				for (uint8_t i = 0; i < NUM_CELLS_PER_CHIP; i++){
					dischargeConfig[c][i] = false;
				}

				dischargeConfig[c][cellTestIter % NUM_CELLS_PER_CHIP] = true;
			}
      	}

		//Configures the segments to discharge based on the boolean area passed
		segment.configureBalancing(dischargeConfig);
		cellTestIter++;

		if (cellTestIter > 8) {
			cellTestIter = 0;
		}
		else
		{
			//Sets all cells to not discharge
			segment.enableBalancing(false);
		}
		Serial.println(); //newline
  	}
	Serial.println(); //newline
    delete[] testData;
    testData = nullptr;
	delay(1000);

	uint16_t tempMaxCharge = 0;			// to be changed when the actual values are calculated
	uint16_t tempMaxDischarge = 0;
	compute.sendMCMsg(tempMaxCharge, tempMaxDischarge);
	
	if (lastPackCurr + 100 < currTime) {
		lastPackCurr = currTime;
		// Get pack current and print
		Serial.println(compute.getPackCurrent());
	}
	delay(1000);
}

void shepherdMain()
{
	//Implement some simple controls and calcs behind shepherd

	//Create a dynamically allocated structure
	//@note this will move to a specialized container with a list of these structs
	AccumulatorData_t *accData = new AccumulatorData_t;
	

	//Collect all the segment data needed to perform analysis
	//Not state specific
	segment.retrieveSegmentData(accData->chipData);

	int16_t current = compute.getPackCurrent();
	Serial.print("Current: ");
	Serial.println(current);
	//compute.getTSGLV();
	//etc

	//Perform calculations on the data in the frame
	//Some calculations might be state dependent
	calcCellTemps(accData);
	calcPackTemps(accData);
	calcPackVoltageStats(accData);
	Serial.print("Min, Max, Avg Temps: ");
	Serial.print(accData->minTemp.val);
	Serial.print(",  ");
	Serial.print(accData->maxTemp.val);
	Serial.print(",  ");
	Serial.println(accData->avgTemp);
	Serial.print("Min, Max, Avg, Delta Voltages: ");
	Serial.print(accData->minVoltage.val);
	Serial.print(",  ");
	Serial.print(accData->maxVoltage.val);
	Serial.print(",  ");
	Serial.print(accData->avgVoltage);
	Serial.print(",  ");
	Serial.println(accData->deltVoltage);
	calcCellResistances(accData);
	calcDCL(accData);
	calcContDCL(accData);
	calcContCCL(accData);
	Serial.print("DCL: ");
	Serial.println(accData->dischargeLimit);

	Serial.print("CCL: ");
	Serial.println(accData->dischargeLimit);

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

	uint16_t overVoltCount = 0;

	// ACTIVE/NORMAL STATE
	if (bmsFault == FAULTS_CLEAR) {
		compute.sendMCMsg(0, accData->dischargeLimit);

		// Check for fuckies
		if (current > accData->contDCL) {
			bmsFault = DISCHARGE_LIMIT_ENFORCEMENT_FAULT;
		}
		if (current < 0 && abs(current) > accData->chargeLimit) {
			bmsFault = CHARGE_LIMIT_ENFORCEMENT_FAULT;
		}
		if (accData->minVoltage.val < MIN_VOLT) {
			bmsFault = CELL_VOLTAGE_TOO_LOW;
		}
		if (accData->maxVoltage.val > MAX_VOLT) { // Needs to be reimplemented with a flag for every cell in case multiple go over
			overVoltCount++;
			if (overVoltCount > 1000) { // 10 seconds @ 100Hz rate
				bmsFault = CELL_VOLTAGE_TOO_HIGH;
			}
		} else {
			overVoltCount = 0;
		}
		if (accData->maxTemp.val > MAX_CELL_TEMP) {
			bmsFault = PACK_TOO_HOT;
		}
		if (accData->minVoltage.val < 900) { // 90mV
			bmsFault = LOW_CELL_VOLTAGE;
		}
	}

	// FAULT STATE
	if (bmsFault != FAULTS_CLEAR) {
		compute.setFault(FAULTED);
		Serial.print("BMS FAULT: ");
		Serial.println(bmsFault);
		Serial.println("Hit Spacebar to clear");
		delay(3000);
		if (Serial.available()) 
		{ // Check for key presses
			char keyPress = Serial.read(); // Read key
			if (keyPress == ' ') 
			{
				bmsFault = FAULTS_CLEAR;
			}
		}
	}

	//compute.sendChargerMsg();
	//sendCanMsg(all the data we wanna send out)
	//etc

	delete accData;
}

void setup()
{
  NERduino.begin();
  initializeCAN(1);
  delay(3000); // Allow time to connect and see boot up info
  Serial.println("Hello World!");
  
  segment.init();

  compute.setFault(NOT_FAULTED);
}

void loop()
{
	/**
	 * @brief These are two functions that can determine the mode that 
	 * 		we are operating in (either testing the segments or actually running the car)
	 * @note eventually, we'll need to find a formal place for the testSegments() function,
	 * 		probably in some HIL automated testing, **THIS IS A TEMPORARY FIX**
	 */
	//testSegments();
	shepherdMain();
	
	delay(10);
}