#include <nerduino.h>
#include <LTC68041.h>
#include "segment.h"
#include "compute.h"
#include "datastructs.h"
#include "analyzer.h"
#include "stateMachine.h"

int currTime = 0;
int lastPackCurr = 0;
int lastVoltTemp = 0;

bool dischargeEnabled = false;
uint16_t cellTestIter = 0;
bool dischargeConfig[NUM_CHIPS][NUM_CELLS_PER_CHIP] = {};

ChipData_t *testData;
Timer mainTimer;

uint32_t bmsFault = FAULTS_CLEAR;

StateMachine bmsDirector;

bool isCharging = false;

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
	AccumulatorData_t accData;

	//Collect all the segment data needed to perform analysis
	//Not state specific
	segment.retrieveSegmentData(accData.chipData);

	int16_t current = compute.getPackCurrent();
	Serial.print("Current: ");
	Serial.println(current);
	//compute.getTSGLV();
	//etc

	//Perform calculations on the data in the frame
	analyzer.push(accData);

	uint16_t overVoltCount = 0;

	// ACTIVE/NORMAL STATE
	if (bmsFault == FAULTS_CLEAR) {
		compute.sendMCMsg(0, analyzer.bmsdata->dischargeLimit);

		// Check for fuckies
		if (current > analyzer.bmsdata->contDCL) {
			bmsFault |= DISCHARGE_LIMIT_ENFORCEMENT_FAULT;
		}
		if (current < 0 && abs(current) > analyzer.bmsdata->chargeLimit) {
			bmsFault |= CHARGE_LIMIT_ENFORCEMENT_FAULT;
		}
		if (analyzer.bmsdata->minVoltage.val < MIN_VOLT) {
			bmsFault |= CELL_VOLTAGE_TOO_LOW;
		}
		if (analyzer.bmsdata->maxVoltage.val > MAX_VOLT) { // Needs to be reimplemented with a flag for every cell in case multiple go over
			overVoltCount++;
			if (overVoltCount > 1000) { // 10 seconds @ 100Hz rate
				bmsFault |= CELL_VOLTAGE_TOO_HIGH;
			}
		} else {
			overVoltCount = 0;
		}
		if (analyzer.bmsdata->maxTemp.val > MAX_CELL_TEMP) {
			bmsFault |= PACK_TOO_HOT;
		}
		if (analyzer.bmsdata->minVoltage.val < 900) { // 90mV
			bmsFault |= LOW_CELL_VOLTAGE;
		}
	}

	// FAULT STATE
	if (bmsFault != FAULTS_CLEAR) {
		compute.setFault(FAULTED);
		Serial.print("BMS FAULT: ");
		Serial.println(bmsFault);
		Serial.println("Hit Spacebar to clear");
		delay(1000);
		if (Serial.available()) 
		{ // Check for key presses
			char keyPress = Serial.read(); // Read key
			if (keyPress == ' ') 
			{
				bmsFault = FAULTS_CLEAR;
			}
		}
	}

	uint16_t packChargeVolt = 300;

	Serial.println("Hit Spacebar to enable charging");
	// CHARGE STATE
	if (Serial.available())
	{ // Check for key presses
		char keyPress = Serial.read(); // Read key
		if (keyPress == ' ')
		{
			isCharging = !isCharging;
		}
	} 

	if (digitalRead(CHARGE_DETECT) == LOW && bmsFault == FAULTS_CLEAR) 
	{
		digitalWrite(CHARGE_SAFETY_RELAY, HIGH);
		compute.enableCharging(true);
		compute.sendChargingMessage(packChargeVolt, analyzer.bmsdata->chargeLimit);
	}
	else if (bmsFault == FAULTS_CLEAR) 
	{
		digitalWrite(CHARGE_SAFETY_RELAY, LOW);
	}

	//sendCanMsg(all the data we wanna send out)
	//etc
}

void setup()
{
  NERduino.begin();
  delay(3000); // Allow time to connect and see boot up info
  Serial.println("Hello World!");

  initializeCAN(CANLINE_1, MC_BAUD, incomingCANCallback);
  
  segment.init();

  compute.setFault(NOT_FAULTED);

  analyzer.resize(MAX_SIZE_OF_HIST_QUEUE / ACCUMULATOR_FRAME_SIZE);
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