#include "segment.h"
/** 
 * 
 * 
 * @todo Uncomment LTC6804 related lines after updating for Teensy
 * 
 * 
 * 
 */

segment::segment()
{
  //LTC6804_initialize(); 
}

segment::~segment(){}


//last two bytes of recieved index are PEC and we want to dump them
void segment::getChipConfigurations(uint8_t localConfig[][6])
{
  uint8_t remoteConfig[NUMCHIPS][8];
  //LTC6804_rdcfg(NUMCHIPS, remoteConfig);
  for (int chip = 0; chip < NUMCHIPS; chip++)
  {
    for(int index = 0; index < 6; index++)
    {
      localConfig[chip][index] = remoteConfig[chip][index];
    }
  }
}

void segment::setChipConfigurations(uint8_t localConfig[][6]) 
{
  //LTC6804_wrcfg(NUMCHIPS, localConfig);
}

void segment::configureDischarge(uint8_t chip, uint16_t cells) 
{
  chipConfigurations[chip][4] = uint8_t(cells & 0x00FF);      //takes the first byte of the cells
  chipConfigurations[chip][5] = (chipConfigurations[chip][5] & 0xF0) + uint8_t(cells >> 8);     //takes the second byte of the cells and the second 4 bits of the current fifth byte of the config
}


/**
 * @brief Reads and returns the cell voltages in a 2D array
 ****** @todo might need to modify the params of this function*****
 * https://stackoverflow.com/questions/8617683/return-a-2d-array-from-a-function  --returning a 2d array
 * @param cellIter
 * @return float* returns an array of Cell Voltages
 */
float **segment::segment_cvRead(uint8_t cellIter)
{
  uint16_t rawCellVoltages[NUMCHIPS][12];

  float **cellVoltages = 0;
  cellVoltages = new float*[NUMCHIPS];

  configureDischarge(0, cellIter);  //Not sure if these need to be configured per chip, or always twice, etc
  configureDischarge(1, cellIter);  //
  setChipConfigurations(chipConfigurations);

  //LTC6804_adcv(); //this needs to be done before pulling from registers

  //get and print the cell voltages
  //LTC6804_rdcv(0, NUMCHIPS, rawCellVoltages);
  Serial.print("Voltage:\n");

  for (int c = 0; c < NUMCHIPS; c++)
  {
    cellVoltages[c] = new float[12];
    for (int cell = 0; cell < 12; cell++)
    {
      cellVoltages[c][cell] = float(rawCellVoltages[c][cell]) / 10000;
    }
  }
  return cellVoltages;
}


/**
 * @brief Serializes the data to be written to the LTC6280 register
 * 
 * @param numChips 
 * @param data 
 * @param commOutput 
 * @param operation 
 */
void segment::msgSerialization(uint8_t numChips, uint8_t data[][3], uint8_t commData [][6], msg_Ser_Operation_t operation)
{
  if(operation == MSG_OPERATION_SERIALIZE)
  {
    for (int chip = 0; chip < numChips; chip++)
    {
      commData[chip][0] = 0x60 | (data[chip][0] >> 4);
      commData[chip][1] = (data[chip][0] << 4) | 0x08;
      commData[chip][2] = 0x00 | (data[chip][1] >> 4);
      commData[chip][3] = (data[chip][1] << 4) | 0x09;
      commData[chip][4] = 0x70 | (data[chip][2] >> 4);
      commData[chip][5] = (data[chip][2] << 4) | 0x09;
    }
  }
  if(operation == MSG_OPERATION_DESERIALIZE)
  {
    for (int chip = 0; chip < numChips; chip++)
    {
      data[chip][0] = ((commData[chip][0] << 4) & 0xF0) | ((commData[chip][1] >> 4) & 0x0F);
      data[chip][1] = ((commData[chip][2] << 4) & 0xF0) | ((commData[chip][1] >> 3) & 0x0F);
      data[chip][2] = ((commData[chip][4] << 4) & 0xF0) | ((commData[chip][1] >> 5) & 0x0F);
    }
  }
}