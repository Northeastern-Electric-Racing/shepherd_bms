#include "segment.h"

SegmentInterface::SegmentInterface(){}

SegmentInterface::~SegmentInterface(){}

void SegmentInterface::retrieveSegmentData(ChipData_t databuf[NUM_CHIPS])
{
    segmentData = databuf;

    pullVoltages();
    pullThermistors();

    segmentData = nullptr;
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
    serializeI2CMsg(i2cWriteData, commRegData);
    LTC6804_wrcomm(NUM_CHIPS, commRegData);
    LTC6804_stcomm(24);

    // Turn on desired thermistor
    for(int chip = 0; chip < NUM_CHIPS; chip++) {
      i2cWriteData[chip][0] = 0x90;
      i2cWriteData[chip][1] = 0x08 + (therm - 1);
      i2cWriteData[chip][2] = 0x00;
    }
    serializeI2CMsg(i2cWriteData, commRegData);
    LTC6804_wrcomm(NUM_CHIPS, commRegData);
    LTC6804_stcomm(24);
  } else if (therm <= 16) {
    // Turn off competing multiplexor (therms 1-8)
    for(int chip = 0; chip < NUM_CHIPS; chip++) {
      i2cWriteData[chip][0] = 0x90;
      i2cWriteData[chip][1] = 0x00;
      i2cWriteData[chip][2] = 0x00;
    }
    serializeI2CMsg(i2cWriteData, commRegData);
    LTC6804_wrcomm(NUM_CHIPS, commRegData);
    LTC6804_stcomm(24);

    // Turn on desired thermistor
    for(int chip = 0; chip < NUM_CHIPS; chip++) {
      i2cWriteData[chip][0] = 0x92;
      i2cWriteData[chip][1] = 0x08 + (therm - 9);
      i2cWriteData[chip][2] = 0x00;
    }
    serializeI2CMsg(i2cWriteData, commRegData);
    LTC6804_wrcomm(NUM_CHIPS, commRegData);
    LTC6804_stcomm(24);
  } else if (therm <= 24) {
    // Turn off competing multiplexor (therms 25-32)
    for(int chip = 0; chip < NUM_CHIPS; chip++) {
      i2cWriteData[chip][0] = 0x96;
      i2cWriteData[chip][1] = 0x00;
      i2cWriteData[chip][2] = 0x00;
    }
    serializeI2CMsg(i2cWriteData, commRegData);
    LTC6804_wrcomm(NUM_CHIPS, commRegData);
    LTC6804_stcomm(24);

    // Turn on desired thermistor
    for(int chip = 0; chip < NUM_CHIPS; chip++) {
      i2cWriteData[chip][0] = 0x94;
      i2cWriteData[chip][1] = 0x08 + (therm - 17);
      i2cWriteData[chip][2] = 0x00;
    }
    serializeI2CMsg(i2cWriteData, commRegData);
    LTC6804_wrcomm(NUM_CHIPS, commRegData);
    LTC6804_stcomm(24);
  } else {
    // Turn off competing multiplexor (therms 17-24)
    for(int chip = 0; chip < NUM_CHIPS; chip++) {
      i2cWriteData[chip][0] = 0x94;
      i2cWriteData[chip][1] = 0x00;
      i2cWriteData[chip][2] = 0x00;
    }
    serializeI2CMsg(i2cWriteData, commRegData);
    LTC6804_wrcomm(NUM_CHIPS, commRegData);
    LTC6804_stcomm(24);

    // Turn on desired thermistor
    for(int chip = 0; chip < NUM_CHIPS; chip++) {
      i2cWriteData[chip][0] = 0x96;
      i2cWriteData[chip][1] = 0x08 + (therm - 25);
      i2cWriteData[chip][2] = 0x00;
    }
    serializeI2CMsg(i2cWriteData, commRegData);
    LTC6804_wrcomm(NUM_CHIPS, commRegData);
    LTC6804_stcomm(24);
  }
}

uint8_t SegmentInterface::steinhartEst(uint16_t V) {
  int i = 0;
  while (V < VOLT_TEMP_CONV[i]) i++;
  return i - 5;
}

FaultStatus_t SegmentInterface::pullThermistors()
{
	//Ensure that GPIO 1 & 2 pull downs are OFF
	pullChipConfigurations();
    for (int c = 0; c < NUM_CHIPS; c++)
    {
      localConfig[c][0] |= 0x18;
    }
    pushChipConfigurations();

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
            segmentData[c].thermistorReading[therm - 1] = steinhartEst(uint32_t(rawTempVoltages[c][0] * (float(rawTempVoltages[c][2]) / 50000)));
            segmentData[c].thermistorReading[therm + 15] = steinhartEst(uint32_t(rawTempVoltages[c][1] * (float(rawTempVoltages[c][2]) / 50000)));
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