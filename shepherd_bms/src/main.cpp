#include <nerduino.h>
#include <Watchdog_t4.h>
#include <LTC68041.h>
#include "segment.h"
#include "compute.h"
#include "calcs.h"
#include "datastructs.h"

int currTime = 0;
int lastPackCurr = 0;
int lastVoltTemp = 0;
int lastChargeMsg = 0;
int lastStatMsg = 0;

bool dischargeEnabled = false;
uint16_t cellTestIter = 0;
bool dischargeConfig[NUM_CHIPS][NUM_CELLS_PER_CHIP] = {};

ChipData_t *testData;
Timer mainTimer;
ComputeInterface compute;
WDT_T4<WDT1> wdt;

Timer chargeTimeout;

AccumulatorData_t *prevAccData = nullptr;

uint32_t bmsFault = FAULTS_CLEAR;

uint16_t overVoltCount = 0;
uint16_t underVoltCount = 0;
uint16_t overCurrCount = 0;
uint16_t chargeOverVolt = 0;
uint16_t overChgCurrCount = 0;

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

void chargeBalancing(AccumulatorData_t *bms_data)
{
	bool balanceConfig[NUM_CHIPS][NUM_CELLS_PER_CHIP];

    for(uint8_t chip = 0; chip < NUM_CHIPS; chip++)
    {
		for(uint8_t cell = 0; cell < NUM_CELLS_PER_CHIP; cell++)
		{
			uint16_t delta = bms_data->chipData[chip].voltageReading[cell] - (uint16_t)bms_data-> minVoltage.val;
			if(delta > MAX_DELTA_V * 10000)
				balanceConfig[chip][cell] = true;
			else
				balanceConfig[chip][cell] = false;
        }
    }
	
	/*Serial.println("Cell Balancing:");
	for(uint8_t c = 0; c < NUM_CHIPS; c++)
    {
        for(uint8_t cell = 0; cell < NUM_CELLS_PER_CHIP; cell++)
        {
			Serial.print(balanceConfig[c][cell]);
			Serial.print("\t");
		}
		Serial.println();
	}*/
	segment.configureBalancing(balanceConfig);
}

bool balancingCheck(AccumulatorData_t *bmsdata)
{

	if (!compute.isCharging()) return false;
	if (bmsdata->maxVoltage.val <= (BAL_MIN_V * 10000)) return false;
	if(bmsdata->deltVoltage <= (MAX_DELTA_V * 10000)) return false;

	return true;
}

bool chargingCheck(AccumulatorData_t *bmsdata)
{
	if(!chargeTimeout.isTimerExpired()) return false;
	if(!compute.isCharging()) return false;
	if(bmsdata->maxVoltage.val >= (MAX_CHARGE_VOLT * 10000))
	{
		chargeOverVolt++;
		if (chargeOverVolt > 100) {
			chargeTimeout.startTimer(CHARGE_TIMEOUT);
			return false;
		}
	} else {
		chargeOverVolt = 0;
	}

	return true;
}

void shepherdMain()
{
	currTime = millis();
	//Implement some simple controls and calcs behind shepherd

	//Create a dynamically allocated structure
	//@note this will move to a specialized container with a list of these structs
	AccumulatorData_t *accData = new AccumulatorData_t;

	//Collect all the segment data needed to perform analysis
	//Not state specific
	segment.retrieveSegmentData(accData->chipData);
	accData->packCurrent = compute.getPackCurrent();
	disableTherms(accData, prevAccData);
	//compute.getTSGLV();
	//etc

	//Perform calculations on the data in the frame
	//Some calculations might be state dependent
	calcCellTemps(accData);
	calcPackTemps(accData);
	calcPackVoltageStats(accData);
	calcOpenCellVoltage(accData, prevAccData);
	
	calcCellResistances(accData);
	calcDCL(accData);
	calcContDCL(accData);
	calcContCCL(accData);

	if (currTime > lastStatMsg + 500) {
		lastStatMsg = currTime;
		Serial.print("Current: ");
		Serial.println(accData->packCurrent);
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
		
		Serial.print("DCL: ");
		Serial.println(accData->dischargeLimit);

		Serial.print("CCL: ");
		Serial.println(accData->chargeLimit);

		Serial.print("Is Balancing?: ");
		Serial.println(segment.isBalancing());

		/*
		Serial.println("Open Cell Voltage:");
		for(uint8_t c = 0; c < NUM_CHIPS; c++)
		{
			for(uint8_t cell = 0; cell < NUM_CELLS_PER_CHIP; cell++)
			{
				Serial.print(accData->chipData[c].openCellVoltage[cell]);
				Serial.print("\t");
			}
			Serial.println();
		}

		Serial.println("Cell Temps:");
		for(uint8_t c = 1; c < NUM_CHIPS; c+= 2)
		{
			for(uint8_t cell = 17; cell < 28; cell++)
			{
				Serial.print(accData->chipData[c].thermistorReading[cell]);
				Serial.print("\t");
			}
			Serial.println();
		}

		Serial.println("Avg Cell Temps:");
		for(uint8_t c = 1; c < NUM_CHIPS; c+= 2)
		{
			for(uint8_t cell = 17; cell < 28; cell++)
			{
				Serial.print(accData->chipData[c].thermistorValue[cell]);
				Serial.print("\t");
			}
			Serial.println();
		} */
	}

	// FAULT CHECK
	// Check for fuckies
	if (accData->packCurrent > accData->dischargeLimit) {
		overCurrCount++;
		if (overCurrCount > 10) { // 0.10 seconds @ 100Hz rate
			bmsFault |= DISCHARGE_LIMIT_ENFORCEMENT_FAULT;
		}
	} else {
		overCurrCount = 0;
	}
	if (accData->packCurrent < 0 && abs(accData->packCurrent) > accData->chargeLimit) {
		overChgCurrCount++;
		if (overChgCurrCount > 100) { // 1 seconds @ 100Hz rate
			bmsFault |= CHARGE_LIMIT_ENFORCEMENT_FAULT;
		}
	} else {
		overChgCurrCount = 0;
	}
	if (accData->minVoltage.val < MIN_VOLT * 10000) {
		underVoltCount++;
		if (underVoltCount > 900) { // 9 seconds @ 100Hz rate
			bmsFault |= CELL_VOLTAGE_TOO_LOW;
		}
	} else {
		underVoltCount = 0;
	}
	if (((accData->maxVoltage.val > MAX_VOLT * 10000) && digitalRead(CHARGE_DETECT) == HIGH) || (accData->maxVoltage.val > MAX_CHARGE_VOLT * 10000)) { // Needs to be reimplemented with a flag for every cell in case multiple go over
		overVoltCount++;
		if (overVoltCount > 900) { // 9 seconds @ 100Hz rate
			bmsFault |= CELL_VOLTAGE_TOO_HIGH;
		}
	} else {
		overVoltCount = 0;
	}
	if (accData->maxTemp.val > MAX_CELL_TEMP) {
		bmsFault |= PACK_TOO_HOT;
	}
	if (accData->minVoltage.val < 900) { // 90mV
		bmsFault |= LOW_CELL_VOLTAGE;
	}

	// ACTIVE/NORMAL STATE
	if (bmsFault == FAULTS_CLEAR) {
		compute.setFault(NOT_FAULTED);
	}

	// FAULT STATE
	if (bmsFault != FAULTS_CLEAR) {
		compute.setFault(FAULTED);

		segment.enableBalancing(false);
		digitalWrite(CHARGE_SAFETY_RELAY, LOW);
		compute.enableCharging(false);

		Serial.print("BMS FAULT: ");
		Serial.println(bmsFault, HEX);
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

	// CHARGE STATE
	if (digitalRead(CHARGE_DETECT) == LOW && bmsFault == FAULTS_CLEAR) {
		// Check if we should charge
		if (chargingCheck(accData)) {
			digitalWrite(CHARGE_SAFETY_RELAY, HIGH);
			compute.enableCharging(true);
			compute.sendChargingStatus(true);
		} else {
			digitalWrite(CHARGE_SAFETY_RELAY, LOW);
			compute.enableCharging(false);
			compute.sendChargingStatus(false);
		}

		// Check if we should balance
		if (balancingCheck(accData)) {
			chargeBalancing(accData);
			segment.enableBalancing(true);
		} else {
			segment.enableBalancing(false);
		}
		
		// Send CAN message, but not too often
		if (currTime > lastChargeMsg + 250) {
			lastChargeMsg = currTime;
			compute.sendChargingMessage(MAX_CHARGE_VOLT * NUM_CELLS_PER_CHIP * NUM_CHIPS, accData->chargeLimit);
		}
	} else if (bmsFault == FAULTS_CLEAR) {
		digitalWrite(CHARGE_SAFETY_RELAY, LOW);
	}

	compute.sendMCMsg(0, accData->dischargeLimit);
	compute.sendAccStatusMessage(accData->packVoltage, accData->packCurrent, 0, 0, 0);
	compute.sendCurrentsStatus(accData->dischargeLimit, accData->chargeLimit, accData->packCurrent);

	compute.setFanSpeed(calcFanPWM(accData));

	prevAccData = accData;
	delete accData;
}

void setup()
{
  NERduino.begin();
  delay(3000); // Allow time to connect and see boot up info
  Serial.println("Hello World!");
  
  segment.init();

  compute.setFault(NOT_FAULTED);

  WDT_timings_t config;
  config.trigger = 5;         /* in seconds, 0->128 */
  config.timeout = 15;        /* in seconds, 0->128 */
  wdt.begin(config);
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
	wdt.feed();
	
	delay(10);
}