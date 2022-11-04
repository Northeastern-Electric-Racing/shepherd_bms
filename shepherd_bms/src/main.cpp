#include <nerduino.h>
#include <LTC68041.h>
#include <segment.h>
#include <compute.h>

void setup()
{
  	NERduino.begin();
  	LTC6804_initialize();
}

ChipData_t testData[NUM_CHIPS];

void loop()
{
  	segmentInterface.retrieveSegmentData(testData);

	for(int i=0; i< NUM_CHIPS; i++)
	{
		Serial.println(testData[i].thermistorReading[0]);
	}
}