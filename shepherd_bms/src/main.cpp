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
ChipData_t *testData;
Timer mainTimer;

void loop()
{
	testData = new ChipData_t[NUM_CHIPS];
	// Run ADC on cell taps
	segment.retrieveSegmentData(testData);

	for (int chip = 0; chip < NUM_CHIPS; chip++)
	{
		for (int cell=0; cell < NUM_CELLS_PER_CHIP; cell++)
		{
			Serial.print(testData[chip].voltageReading[cell]);
			Serial.print("\t");
		}

	  	Serial.println(); //newline

		for (int therm=0; therm < NUM_THERMS_PER_CHIP; therm++)
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
	delay(1000);

	uint16_t tempMaxCharge = 0;			// to be changed when the actual values are calculated
	uint16_t tempMaxDischarge = 0;
	compute.sendMCMsg(tempMaxCharge, tempMaxDischarge);

}