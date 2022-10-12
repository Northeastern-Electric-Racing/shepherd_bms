#include "segment.h"

SegmentInterface::SegmentInterface(){}

SegmentInterface::~SegmentInterface(){}

int* SegmentInterface::retrieveSegmentData()
{
    pullVoltages();
    pullThermistors();
    return segmentData;
}

void SegmentInterface::ConfigureCOMMRegisters(uint8_t numChips, uint8_t dataToWrite[][3], uint8_t commOutput [][6])
{
  for (int chip = 0; chip < numChips; chip++)
  {
    commOutput[chip][0] = 0x60 | (dataToWrite[chip][0] >> 4); // START + high side of B0
    commOutput[chip][1] = (dataToWrite[chip][0] << 4) | 0x00; // low side of B0 + ACK
    commOutput[chip][2] = 0x00 | (dataToWrite[chip][1] >> 4); // BLANK + high side of B1
    commOutput[chip][3] = (dataToWrite[chip][1] << 4) | 0x00; // low side of B1 + ACK
    commOutput[chip][4] = 0x00 | (dataToWrite[chip][2] >> 4); // BLANK + high side of B2
    commOutput[chip][5] = (dataToWrite[chip][2] << 4) | 0x09; // low side of B2 + STOP & NACK
  }
}

void SegmentInterface::SelectTherm(uint8_t therm) 
{
  // Exit if out of range values
  if (therm < 0 || therm > 32) {
    return;
  }
  uint8_t i2cWriteData[NUM_CHIPS][3];
  uint8_t commRegData[NUM_CHIPS][6];
  if (therm <= 8) {
    // Turn off competing multiplexor (therms 9-16)
    for(int chip = 0; chip < NUM_CHIPS; chip++) {
      i2cWriteData[chip][0] = 0x92;
      i2cWriteData[chip][1] = 0x00;
      i2cWriteData[chip][2] = 0x00;
    }
    ConfigureCOMMRegisters(NUM_CHIPS, i2cWriteData, commRegData);
    LTC6804_wrcomm(NUM_CHIPS, commRegData);
    LTC6804_stcomm(24);

    // Turn on desired thermistor
    for(int chip = 0; chip < NUM_CHIPS; chip++) {
      i2cWriteData[chip][0] = 0x90;
      i2cWriteData[chip][1] = 0x08 + (therm - 1);
      i2cWriteData[chip][2] = 0x00;
    }
    ConfigureCOMMRegisters(NUM_CHIPS, i2cWriteData, commRegData);
    LTC6804_wrcomm(NUM_CHIPS, commRegData);
    LTC6804_stcomm(24);
  } else if (therm <= 16) {
    // Turn off competing multiplexor (therms 1-8)
    for(int chip = 0; chip < NUM_CHIPS; chip++) {
      i2cWriteData[chip][0] = 0x90;
      i2cWriteData[chip][1] = 0x00;
      i2cWriteData[chip][2] = 0x00;
    }
    ConfigureCOMMRegisters(NUM_CHIPS, i2cWriteData, commRegData);
    LTC6804_wrcomm(NUM_CHIPS, commRegData);
    LTC6804_stcomm(24);

    // Turn on desired thermistor
    for(int chip = 0; chip < NUM_CHIPS; chip++) {
      i2cWriteData[chip][0] = 0x92;
      i2cWriteData[chip][1] = 0x08 + (therm - 9);
      i2cWriteData[chip][2] = 0x00;
    }
    ConfigureCOMMRegisters(NUM_CHIPS, i2cWriteData, commRegData);
    LTC6804_wrcomm(NUM_CHIPS, commRegData);
    LTC6804_stcomm(24);
  } else if (therm <= 24) {
    // Turn off competing multiplexor (therms 25-32)
    for(int chip = 0; chip < NUM_CHIPS; chip++) {
      i2cWriteData[chip][0] = 0x96;
      i2cWriteData[chip][1] = 0x00;
      i2cWriteData[chip][2] = 0x00;
    }
    ConfigureCOMMRegisters(NUM_CHIPS, i2cWriteData, commRegData);
    LTC6804_wrcomm(NUM_CHIPS, commRegData);
    LTC6804_stcomm(24);

    // Turn on desired thermistor
    for(int chip = 0; chip < NUM_CHIPS; chip++) {
      i2cWriteData[chip][0] = 0x94;
      i2cWriteData[chip][1] = 0x08 + (therm - 17);
      i2cWriteData[chip][2] = 0x00;
    }
    ConfigureCOMMRegisters(NUM_CHIPS, i2cWriteData, commRegData);
    LTC6804_wrcomm(NUM_CHIPS, commRegData);
    LTC6804_stcomm(24);
  } else {
    // Turn off competing multiplexor (therms 17-24)
    for(int chip = 0; chip < NUM_CHIPS; chip++) {
      i2cWriteData[chip][0] = 0x94;
      i2cWriteData[chip][1] = 0x00;
      i2cWriteData[chip][2] = 0x00;
    }
    ConfigureCOMMRegisters(NUM_CHIPS, i2cWriteData, commRegData);
    LTC6804_wrcomm(NUM_CHIPS, commRegData);
    LTC6804_stcomm(24);

    // Turn on desired thermistor
    for(int chip = 0; chip < NUM_CHIPS; chip++) {
      i2cWriteData[chip][0] = 0x96;
      i2cWriteData[chip][1] = 0x08 + (therm - 25);
      i2cWriteData[chip][2] = 0x00;
    }
    ConfigureCOMMRegisters(NUM_CHIPS, i2cWriteData, commRegData);
    LTC6804_wrcomm(NUM_CHIPS, commRegData);
    LTC6804_stcomm(24);
  }
}

const uint32_t VOLT_TEMP_CONV[] = {45140, 44890, 44640, 44370, 44060, 43800, 43510, 43210, 42900, 42560, 42240, 41900, 41550, 41170, 40820, 40450, 40040, 39650, 39220, 38810, 38370, 37950, 37500, 37090, 36650, 36180, 35670, 35220, 34740, 34230, 33770, 33270, 32770, 32280, 31770, 31260, 30750, 30240, 29720, 29220, 28710, 28200, 27680, 27160, 26660, 26140, 25650, 25130, 24650, 24150, 23660, 23170, 22670, 22190, 21720, 21240, 0};
int voltToTemp(uint32_t V) {
  int i = 0;
  while (V < VOLT_TEMP_CONV[i]) i++;
  return i - 5;
}

void SegmentInterface::pullThermistors()
{
    uint16_t rawTempVoltages[NUM_CHIPS][6];
    for (int therm = 1; therm <= 16; therm++) {
        SelectTherm(therm);
        delay(5);
        SelectTherm(therm + 16);
        delay(10);
        LTC6804_adax(); // Run ADC for AUX (GPIOs and refs)
        delay(10);
        LTC6804_rdaux(0, NUM_CHIPS, rawTempVoltages); // Fetch ADC results from AUX registers
        for (int c = 0; c < NUM_CHIPS; c++) {
            thermisterTemp[c][therm - 1] = voltToTemp(uint32_t(rawTempVoltages[c][0] * (float(rawTempVoltages[c][2]) / 50000)));
            thermisterTemp[c][therm + 15] = voltToTemp(uint32_t(rawTempVoltages[c][1] * (float(rawTempVoltages[c][2]) / 50000)));
        }
    }
}

void SegmentInterface::enableBalancing(bool balanceEnable)
{

}

void SegmentInterface::enableBalancing(uint8_t chipNum, uint8_t cellNum, bool balanceEnable)
{

}

bool SegmentInterface::isBalancing(uint8_t chipNum, uint8_t cellNum)
{

}

bool SegmentInterface::isBalancing()
{
    
}

void SegmentInterface::pullChipConfigurations()
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

void SegmentInterface::pushChipConfigurations()
{
    LTC6804_wrcfg(NUM_CHIPS, localConfig);
}

void SegmentInterface::serializeI2CMsg(uint8_t dataToWrite[][3], uint8_t commOutput[][6])
{
    for (int chip = 0; chip < NUM_CHIPS; chip++)
    {
        commOutput[chip][0] = 0x60 | (dataToWrite[chip][0] >> 4); // START + high side of B0
        commOutput[chip][1] = (dataToWrite[chip][0] << 4) | 0x00; // low side of B0 + ACK
        commOutput[chip][2] = 0x00 | (dataToWrite[chip][1] >> 4); // BLANK + high side of B1
        commOutput[chip][3] = (dataToWrite[chip][1] << 4) | 0x00; // low side of B1 + ACK
        commOutput[chip][4] = 0x00 | (dataToWrite[chip][2] >> 4); // BLANK + high side of B2
        commOutput[chip][5] = (dataToWrite[chip][2] << 4) | 0x09; // low side of B2 + STOP & NACK
    }
}