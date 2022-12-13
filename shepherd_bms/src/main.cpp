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
	}
	
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
	compute.getPackCurrent();
	//compute.getTSGLV();
	//etc

	//Perform calculations on the data in the frame
	//Some calculations might be state dependent
	calcCellTemps(accData);
	calcCellResistances(accData);
	calcDCL(accData);

	//Send out what needs to happen now (depends on state)
	//compute.sendMCMsg(CCL, DCL);
	//compute.sendChargerMsg();
	//sendCanMsg(all the data we wanna send out)
	//etc

	delete accData;
}

void setup()
{
  NERduino.begin();
  delay(3000); // Allow time to connect and see boot up info
  Serial.println("Hello World!");
  
  segment.init();
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
}