#include <nerduino.h>
#include <LTC68041.h>
#include <segment.h>
#include <compute.h>

uint16_t rawCellVoltages[NUM_CHIPS][12];
float cellVoltages[NUM_CHIPS][12];
uint8_t chipConfigurations[NUM_CHIPS][6];

//last two bytes of recieved index are PEC and we want to dump them
void GetChipConfigurations(uint8_t localConfig[][6])
{
  uint8_t remoteConfig[NUM_CHIPS][8];
  LTC6804_rdcfg(NUM_CHIPS, remoteConfig);
  for (int chip = 0; chip < NUM_CHIPS; chip++)
  {
    for(int index = 0; index < 6; index++)
    {
      localConfig[chip][index] = remoteConfig[chip][index];
    }
  }
}

void SetChipConfigurations(uint8_t localConfig[][6])
{
  LTC6804_wrcfg(NUM_CHIPS, localConfig);
}

void ConfigureDischarge(uint8_t chip, uint16_t cells)
{
  chipConfigurations[chip][4] = uint8_t(cells & 0x00FF);
  chipConfigurations[chip][5] = (chipConfigurations[chip][5] & 0xF0) + uint8_t(cells >> 8);
}

void setup()
{
  NERduino.begin();
  // put your setup code here, to run once:
  delay(3000); // Allow time to connect and see boot up info
  Serial.println("Hello World!");
  LTC6804_initialize();

  // Turn OFF GPIO 1 & 2 pull downs
  GetChipConfigurations(chipConfigurations);
  for (int c = 0; c < NUM_CHIPS; c++)
  {
    chipConfigurations[c][0] |= 0x18;
    ConfigureDischarge(c, 0);
  }
  SetChipConfigurations(chipConfigurations);

  Serial.print("Chip CFG:\n");
  for (int c = 0; c < NUM_CHIPS; c++)
  {
    for (int byte = 0; byte < 6; byte++)
    {
      Serial.print(chipConfigurations[c][byte], HEX);
      Serial.print("\t");
    }
    Serial.println();
  }
  Serial.println();
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