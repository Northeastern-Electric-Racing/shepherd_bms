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

	Serial.print("Voltage:\n");
	for (int chip = 0; chip < NUM_CHIPS; chip++)
	{
		for (int cell=0; cell < NUM_CELLS_PER_CHIP; cell++)
		{
			Serial.print(testData[chip].voltageReading[cell]);
			Serial.print("\t");
		}
	Serial.println(); //newline
      }
    delete[] testData;
    testData = nullptr;
}