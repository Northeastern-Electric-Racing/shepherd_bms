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
#include "stateMachine.h"

WDT_T4<WDT1> wdt;

AccumulatorData_t *prevAccData = nullptr;

StateMachine stateMachine;

#ifdef DEBUG_STATS
int pack_res_arr[10][NUM_CHIPS][NUM_CELLS_PER_CHIP];
int ind = 0;
void storePackResistances(AccumulatorData_t *accData, int (&packResArr)[10][NUM_CHIPS][NUM_CELLS_PER_CHIP], int &index)
{
	if((accData->pack_current > 100) && (index < 10))
	{
		for(int chip = 0; chip < NUM_CHIPS; chip++)
		{
			for(int cell = 0; cell < NUM_CELLS_PER_CHIP; cell++)
			{
				packResArr[index][chip][cell] = accData->chip_data[chip].cell_resistance[cell];
			}
		}
		index++;
	}
}

void printPackResistances(int (&packResArr)[10][NUM_CHIPS][NUM_CELLS_PER_CHIP])
{
	for(int i = 0; i < 10; i++)
	{
		for(int chip = 0; chip < NUM_CHIPS; chip++)
		{
			for(int cell = 0; cell < NUM_CELLS_PER_CHIP; cell++)
			{
				Serial.print(packResArr[i][chip][cell]);
				Serial.print("	");
			}
			Serial.println("\n");
		}
		Serial.println("\n \n \n");
	}
}

const void printBMSStats(AccumulatorData_t *accData)
{
	static Timer debug_statTimer;
	static const uint16_t PRINT_STAT_WAIT = 500; //ms

	if(!debug_statTimer.isTimerExpired()) return;

	Serial.print("Prev Fault: ");
	Serial.println(stateMachine.previousFault);
	Serial.print("Current: ");
	Serial.println((accData->pack_current)/10);
	Serial.print("Min, Max, Avg Temps: ");
	Serial.print(accData->min_temp.val);
	Serial.print(",  ");
	Serial.print(accData->max_temp.val);
	Serial.print(",  ");
	Serial.println(accData->avg_temp);
	Serial.print("Min, Max, Avg, Delta Voltages: ");
	Serial.print(accData->min_voltage.val);
	Serial.print(",  ");
	Serial.print(accData->max_voltage.val);
	Serial.print(",  ");
	Serial.print(accData->avg_voltage);
	Serial.print(",  ");
	Serial.println(accData->delt_voltage);

	Serial.print("DCL: ");
	Serial.println(accData->discharge_limit);

	Serial.print("CCL: ");
	Serial.println(accData->charge_limit);

	Serial.print("SoC: ");
	Serial.println(accData->soc);

	Serial.print("Is Balancing?: ");
	Serial.println(segment.isBalancing());

	Serial.print("State: ");
	if (stateMachine.current_state == 0) Serial.println("BOOT");
	else if (stateMachine.current_state == 1) Serial.println("READY");
	else if (stateMachine.current_state == 2) Serial.println("CHARGING");
	else if (stateMachine.current_state == 1) Serial.println("FAULTED");

	/*Serial.println("Raw Cell Voltage:");
	for(uint8_t c = 0; c < NUM_CHIPS; c++)
	{
		for(uint8_t cell = 0; cell < NUM_CELLS_PER_CHIP; cell++)
		{
			Serial.print(accData->chipData[c].voltageReading[cell]);
			Serial.print("\t");
		}
		Serial.println();
	}*/

	Serial.println("Open Cell Voltage:");
	for(uint8_t c = 0; c < NUM_CHIPS; c++)
	{
		for(uint8_t cell = 0; cell < NUM_CELLS_PER_CHIP; cell++)
		{
			Serial.print(accData->chip_data[c].open_cell_voltage[cell]);
			Serial.print("\t");
		}
		Serial.println();
	}

	Serial.println("Cell Temps:");
	for(uint8_t c = 0; c < NUM_CHIPS; c++)
	{
		for(uint8_t cell = 17; cell < 28; cell++)
		{
			Serial.print(accData->chip_data[c].thermistor_reading[cell]);
			Serial.print("\t");
		}
		Serial.println();
	}

	Serial.println("Avg Cell Temps:");
	for(uint8_t c = 0; c < NUM_CHIPS; c++)
	{
		for(uint8_t cell = 17; cell < 28; cell++)
		{
			Serial.print(accData->chip_data[c].thermistor_value[cell]);
			Serial.print("\t");
		}
		Serial.println();
	}


	debug_statTimer.startTimer(PRINT_STAT_WAIT);
}


#endif


void setup()
{
  WDT_timings_t config;
  config.trigger = 5;         /* in seconds, 0->128 */
  config.timeout = 15;        /* in seconds, 0->128 */
  wdt.begin(config);
  NERduino.begin();
  compute.setFault(NOT_FAULTED);
  segment.init();
}

void loop()
{
	//Create a dynamically allocated structure
	AccumulatorData_t *accData = new AccumulatorData_t;

	//accData->faultCode = FAULTS_CLEAR;

	//Collect all the segment data needed to perform analysis
	//Not state specific
	segment.retrieveSegmentData(accData->chip_data);
	accData->pack_current = compute.getPackCurrent();

	//Perform calculations on the data in the frame
	analyzer.push(accData);

	stateMachine.handleState(accData);

	#ifdef DEBUG_STATS
	printBMSStats(analyzer.bmsdata);
	storePackResistances(accData, pack_res_arr, ind);
	if(ind == 10)
	{
		printPackResistances(pack_res_arr);
		ind = 0;
	}
	#endif

	wdt.feed();
	delay(10); // not sure if we need this in, it was in before
}
