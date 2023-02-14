#ifdef DEBUG_EVERYTHING
#define DEBUG_CHARGING
#define DEBUG_STATS
// etc etc
#endif

#include <nerduino.h>
#include <Watchdog_t4.h>
#include <LTC68041.h>
#include "segment.h"
#include "compute.h"

#include "datastructs.h"
#include "analyzer.h"

WDT_T4<WDT1> wdt;

AccumulatorData_t *prevAccData = nullptr;

uint32_t bmsFault = FAULTS_CLEAR;

uint16_t over_volt_count = 0;
uint16_t under_volt_count = 0;
uint16_t over_curr_count = 0;
uint16_t charge_over_volt = 0;
uint16_t over_chg_curr_count = 0;
uint16_t low_cell_count = 0;

/**
 * @brief Algorithm behind determining which cells we want to balance
 * @note Directly interfaces with the segments
 * 
 * @param bms_data 
 */
void balanceCells(AccumulatorData_t *bms_data)
{
	bool balanceConfig[NUM_CHIPS][NUM_CELLS_PER_CHIP];

	// For all cells of all the chips, figure out if we need to balance by comparing the difference in voltages
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
	#ifdef DEBUG_CHARGING
	Serial.println("Cell Balancing:");
	for(uint8_t c = 0; c < NUM_CHIPS; c++)
    {
        for(uint8_t cell = 0; cell < NUM_CELLS_PER_CHIP; cell++)
        {
			Serial.print(balanceConfig[c][cell]);
			Serial.print("\t");
		}
		Serial.println();
	}
	#endif

	segment.configureBalancing(balanceConfig);
}

/**
 * @brief Returns if we want to balance cells during a particular frame
 * 
 * @param bmsdata 
 * @return true 
 * @return false 
 */
bool balancingCheck(AccumulatorData_t *bmsdata)
{
	if (!compute.isCharging()) return false;
	if (bmsdata->maxTemp.val > MAX_CELL_TEMP_BAL) return false;
	if (bmsdata->maxVoltage.val <= (BAL_MIN_V * 10000)) return false;
	if(bmsdata->deltVoltage <= (MAX_DELTA_V * 10000)) return false;

	return true;
}

/**
 * @brief Returns if we want to charge cells during a particular frame
 * 
 * @param bmsdata 
 * @return true 
 * @return false 
 */
bool chargingCheck(AccumulatorData_t *bmsdata)
{
	static Timer chargeTimeout;

	if(!chargeTimeout.isTimerExpired()) return false;
	if(!compute.isCharging()) return false;
	if(bmsdata->maxVoltage.val >= (MAX_CHARGE_VOLT * 10000))
	{
		charge_over_volt++;
		if (charge_over_volt > 100) 
		{
			chargeTimeout.startTimer(CHARGE_TIMEOUT);
			return false;
		}
	} 
	else 
	{
		charge_over_volt = 0;
	}

	return true;
}

void broadcastCurrentLimit(AccumulatorData_t *bmsdata)
{
	// States for Boosting State Machine
	static enum
	{
		BOOST_STANDBY,
		BOOSTING,
		BOOST_RECHARGE
	}BoostState;

	static Timer boostTimer;
	static Timer boostRechargeTimer;

	//Transitioning out of boost
	if(boostTimer.isTimerExpired() && BoostState == BOOSTING)
	{
		BoostState = BOOST_RECHARGE;
		boostRechargeTimer.startTimer(BOOST_RECHARGE_TIME);
	}
	//Transition out of boost recharge
	if(boostRechargeTimer.isTimerExpired() && BoostState == BOOST_RECHARGE)
	{
		BoostState = BOOST_STANDBY;
	}
	//Transition to boosting
	if((bmsdata->packCurrent) > ((bmsdata->contDCL)*10) && BoostState == BOOST_STANDBY)
	{
		BoostState = BOOSTING;
		boostTimer.startTimer(BOOST_TIME);
	}

	//Currently boosting
	if(BoostState == BOOSTING || BoostState == BOOST_STANDBY)
	{
		compute.sendMCMsg(bmsdata->chargeLimit, min(bmsdata->dischargeLimit, bmsdata->contDCL * CONTDCL_MULTIPLIER));
	}
	//Currently recharging boost
	else
	{
		compute.sendMCMsg(bmsdata->chargeLimit, min(bmsdata->contDCL, bmsdata->dischargeLimit));
	}
}

#ifdef DEBUG_STATS

const void printBMSStats(AccumulatorData_t *accData)
{
	static Timer debug_statTimer;
	static const uint16_t PRINT_STAT_WAIT = 500; //ms
	
	if(!debug_statTimer.isTimerExpired()) return;

	Serial.print("Current: ");
	Serial.println((accData->packCurrent)/10);
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

	Serial.print("SoC: ");
	Serial.println(accData->soc);

	Serial.print("Is Balancing?: ");
	Serial.println(segment.isBalancing()); 

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
	}

	debug_statTimer.startTimer(PRINT_STAT_WAIT);
}

#endif

/**
 * @brief Returns any new faults or current faults that have come up
 * @note Should be bitwise OR'ed with the current fault status
 * 
 * @param accData 
 * @return uint32_t 
 */
uint32_t faultCheck(AccumulatorData_t *accData)
{
	// FAULT CHECK
	// Check for fuckies
	uint32_t faultStatus = 0;

	// Over current fault for discharge
	if ((accData->packCurrent) > ((accData->dischargeLimit)*10)) 
	{
		over_curr_count++;
		if (over_curr_count > 10) 
		{ // 0.10 seconds @ 100Hz rate
			faultStatus |= DISCHARGE_LIMIT_ENFORCEMENT_FAULT;
		}
	} else 
	{
		over_curr_count = 0;
	}

	// Over current fault for charge
	if ((accData->packCurrent) < 0 && abs((accData->packCurrent)) > ((accData->chargeLimit)*10))
	{
		over_chg_curr_count++;
		if (over_chg_curr_count > 100) 
		{ // 1 seconds @ 100Hz rate
			faultStatus |= CHARGE_LIMIT_ENFORCEMENT_FAULT;
		}
	} 
	else 
	{
		over_chg_curr_count = 0;
	} 

	// Low cell voltage fault
	if (accData->minVoltage.val < MIN_VOLT * 10000) 
	{

		under_volt_count++;
		if (under_volt_count > 900)
		{ // 9 seconds @ 100Hz rate
			faultStatus |= CELL_VOLTAGE_TOO_LOW;
		}
	} 
	else 
	{
		under_volt_count = 0;
	}

	// High cell voltage fault
	if (((accData->maxVoltage.val > MAX_VOLT * 10000) && digitalRead(CHARGE_DETECT) == HIGH) || (accData->maxVoltage.val > MAX_CHARGE_VOLT * 10000)) 
	{ // Needs to be reimplemented with a flag for every cell in case multiple go over
		over_volt_count++;
		if (over_volt_count > 900) { // 9 seconds @ 100Hz rate
			faultStatus |= CELL_VOLTAGE_TOO_HIGH;
		}
	} 
	else 
	{
		over_volt_count = 0;
	}

	// High Temp Fault
	if (accData->maxTemp.val > MAX_CELL_TEMP) {
		faultStatus |= PACK_TOO_HOT;
	}

	// Extremely low cell voltage fault
	if (accData->minVoltage.val < 900) 
	{ // 90mV
		low_cell_count++;
		if (low_cell_count > 100) { // 1 seconds @ 100Hz rate
			faultStatus |= LOW_CELL_VOLTAGE;
		}
	} 
	else 
	{
		low_cell_count = 0;
	}

	return faultStatus;
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
	accData->packCurrent = compute.getPackCurrent();
	//compute.getTSGLV();
	//etc

	//Perform calculations on the data in the frame
	//Some calculations might be state dependent
	analyzer.push(accData);

	#ifdef DEBUG_STATS
	printBMSStats(analyzer.bmsdata);
	#endif

	// Check for faults
	bmsFault |= faultCheck(analyzer.bmsdata);

	// ACTIVE/NORMAL STATE
	if (bmsFault == FAULTS_CLEAR) 
	{
		compute.setFault(NOT_FAULTED);
	}
	else
	{
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

	/**
	 * @todo move to charging state in SM
	 */
	{
		static Timer chargeMessageTimer;
		static const uint16_t CHARGE_MESSAGE_WAIT = 250; //ms

		// CHARGE STATE
		if (digitalRead(CHARGE_DETECT) == LOW && bmsFault == FAULTS_CLEAR) 
		{
			// Check if we should charge
			if (chargingCheck(analyzer.bmsdata)) 
			{
				digitalWrite(CHARGE_SAFETY_RELAY, HIGH);
				compute.enableCharging(true);
				compute.sendChargingStatus(true);
			} 
			else 
			{
				digitalWrite(CHARGE_SAFETY_RELAY, LOW);
				compute.enableCharging(false);
				compute.sendChargingStatus(false);
			}

			// Check if we should balance
			if (balancingCheck(analyzer.bmsdata)) 
			{
				balanceCells(analyzer.bmsdata);
			} 
			else 
			{
				segment.enableBalancing(false);
			}
			
			// Send CAN message, but not too often
			if (chargeMessageTimer.isTimerExpired()) 
			{
				compute.sendChargingMessage((MAX_CHARGE_VOLT * NUM_CELLS_PER_CHIP * NUM_CHIPS), analyzer.bmsdata);
				chargeMessageTimer.startTimer(CHARGE_MESSAGE_WAIT);
			}
		} 
		else if (bmsFault == FAULTS_CLEAR) 
		{
			digitalWrite(CHARGE_SAFETY_RELAY, LOW);
		}
	}


	broadcastCurrentLimit(analyzer.bmsdata);
	compute.sendAccStatusMessage(analyzer.bmsdata->packVoltage, analyzer.bmsdata->packCurrent, 0, 0, 0);
	compute.sendCurrentsStatus(analyzer.bmsdata->dischargeLimit, analyzer.bmsdata->chargeLimit, analyzer.bmsdata->packCurrent);
	compute.setFanSpeed(analyzer.calcFanPWM(analyzer.bmsdata));
}

void setup()
{
  WDT_timings_t config;
  config.trigger = 5;         /* in seconds, 0->128 */
  config.timeout = 15;        /* in seconds, 0->128 */
  wdt.begin(config);
  NERduino.begin();
  
  segment.init();

  compute.setFault(NOT_FAULTED);
}

void loop()
{
	shepherdMain();
	wdt.feed();
	delay(10); // not sure if we need this in, it was in before
}