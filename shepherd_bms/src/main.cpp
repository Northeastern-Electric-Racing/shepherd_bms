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

void setup()
{
  NERduino.begin();
  LTC6804_initialize();

  // Turn OFF GPIO 1 & 2 pull downs
  GetChipConfigurations(chipConfigurations);
  for (int c = 0; c < NUM_CHIPS; c++)
  {
    chipConfigurations[c][0] |= 0x18;
  }
  SetChipConfigurations(chipConfigurations);

  GetChipConfigurations(chipConfigurations);

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
  Serial.println("Done");
  delay(3000);
}

void loop()
{
	/**
	 * @brief Sample flow of how contorl code should flow
	 *
	 * 1. Read Cell Data from Segment Interface
	 * 2. Read other peripheral data from Compute Interface
	 * 3. Send data to a large container of AccumulaterData structs
	 * 4. Call the StateMachine.handleState()
	 * 5. Replenish watchdog
	 *
	 */

    // Run ADC on cell taps
    LTC6804_adcv(); //this needs to be done before pulling from registers

    // Pull and print the cell voltages from registers
    LTC6804_rdcv(0, NUM_CHIPS, rawCellVoltages);

    Serial.print("Voltage:\n");
    for (int chip = 0; chip < NUM_CHIPS; chip++)
    {
      for (int cell=0; cell < NUM_CELLS_PER_CHIP; cell++)
      {
            Serial.print(rawCellVoltages[chip][cell]);
            Serial.print("\t");
      }
		Serial.println(); //newline
	}
  delay(1000);
}