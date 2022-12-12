#include <nerduino.h>
#include <LTC68041.h>
#include <segment.h>
#include <compute.h>

void setup()
{
  NERduino.begin();
  delay(3000); // Allow time to connect and see boot up info
  Serial.println("Hello World!");
  
  segment.init();
}

bool dischargeEnabled = false;
uint16_t cellTestIter = 0;
bool dischargeConfig[NUM_CHIPS][NUM_CELLS_PER_CHIP] = {};

ChipData_t *testData;
Timer mainTimer;

void loop()
{
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

	delay(100);

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

		//Configures the segments to discharge based on the boolean area passed
		segment.configureBalancing(dischargeConfig);
		cellTestIter++;
		if (cellTestIter > 8) {
			cellTestIter = 0;
		}
	}
	else
	{
		//Sets all cells to not discharge
		segment.enableBalancing(false);
	}

	delay(1000);
}