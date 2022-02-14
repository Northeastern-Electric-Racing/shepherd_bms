#include "main.h"

#define NUMCHIPS  1     //number of chips in the daisy chain

float **cellVoltages;
uint8_t chipConfigurations[NUMCHIPS][6];
uint8_t cellTestIter = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println("Hello World!");
  LTC6804_initialize(); 
  //set_adc(MD_NORMAL, DCP_ENABLED, CELL_CH_ALL, AUX_CH_ALL);
}

void loop() {

 /*******************************************************
 * Cell Voltage Printing
 *******************************************************/

  cellTestIter++;
  if (cellTestIter >= 16)
  {
    cellTestIter = 0;
  }

  cellVoltages = segment_cvRead(cellTestIter);
  Serial.print("Voltage:\n");
  for (int c = 0; c < NUMCHIPS; c++)
  {
    for (int cell = 0; cell < 12; cell++)
    {
      Serial.print(cellVoltages[c][cell]);
      Serial.print("\t");
    }
    Serial.println();
  }
  Serial.println();

 /******************************************************
 * Current Config Printing
 ******************************************************/

  GetChipConfigurations(chipConfigurations);    //sets chipConfiguration to the current config of the chip

  Serial.print("Chip CFG:\n");
  for (int c = 0; c < NUMCHIPS; c++)
  {
    for (int byte = 0; byte < 6; byte++)
    {
      Serial.print(chipConfigurations[c][byte]);
      Serial.print("\t");
    }
    Serial.println();
  }
  Serial.println();


  /***************************************************************************************
   * 
   * MUX Communication
   * https://www.analog.com/media/en/technical-documentation/data-sheets/680412fc.pdf   --page 56 shows an example of I2C communication via LTC6804
   * 
  ****************************************************************************************/
  uint8_t commRegisterData[NUMCHIPS][6];
  uint8_t i2cWriteData[NUMCHIPS][3];

  uint8_t commReadData[NUMCHIPS][6];
  uint8_t outputData[NUMCHIPS][3];

  // Writing to the Multiplexer (Address 0x96, channel 8)
  for(int chip = 0; chip < NUMCHIPS; chip++)
  {
    i2cWriteData[chip][0] = MUX4_ADDRESS;
    i2cWriteData[chip][1] = 0x07;
    i2cWriteData[chip][2] = 0xAA;   //was "data to be stored in EEPROM" but we're not using an EEPROM rn
  }

  segment_msgSerialization(i2cWriteData, commRegisterData, MSG_OPERATION_SERIALIZE);

  LTC6804_wrcomm(NUMCHIPS, commRegisterData);
  LTC6804_rdcomm(NUMCHIPS, commReadData);    //Reading what we just wrote to the COMM register

  Serial.print("What we're writing to the LTC chip:\n");
  for (int c = 0; c < NUMCHIPS; c++)
  {
    for (int byte = 0; byte < 6; byte++)
    {
      Serial.print(commReadData[c][byte], HEX);     //printing what we just wrote to COMM register
      Serial.print("\t");
    }
    Serial.println();
  }
  Serial.println();

  LTC6804_wrcomm(NUMCHIPS, commRegisterData); //Loading LTC register with data to send just in case, not sure if necessary in this case because we already loaded it
  LTC6804_stcomm(NUMCHIPS * 2);               //Start communication by sending that data
  LTC6804_rdcomm(NUMCHIPS, commReadData);     //Read data from I2C Multiplexer

  segment_msgSerialization(outputData, commReadData, MSG_OPERATION_DESERIALIZE);

  Serial.print("Register Data from LTC chip:\n");
  for (int c = 0; c < NUMCHIPS; c++)
  {
    for (int byte = 0; byte < 6; byte++)
    {
      Serial.print(commReadData[c][byte], HEX);
      Serial.print("\t");
    }
    Serial.println();
  }
  Serial.println();

  Serial.print("Decoded Data from MUX:\n");
  for (int c = 0; c < NUMCHIPS; c++)
  {
    for (int byte = 0; byte < 3; byte++)
    {
      Serial.print(outputData[c][byte], HEX);
      Serial.print("\t");
    }
    Serial.println();
  }
  Serial.println();

  delay(1000);
}
/*****************************************************************************/
//Helper Functions:


//last two bytes of recieved index are PEC and we want to dump them
void GetChipConfigurations(uint8_t localConfig[][6])
{
  uint8_t remoteConfig[NUMCHIPS][8];
  LTC6804_rdcfg(NUMCHIPS, remoteConfig);
  for (int chip = 0; chip < NUMCHIPS; chip++)
  {
    for(int index = 0; index < 6; index++)
    {
      localConfig[chip][index] = remoteConfig[chip][index];
    }
  }
}

void SetChipConfigurations(uint8_t localConfig[][6]) 
{
  LTC6804_wrcfg(NUMCHIPS, localConfig);
}

void ConfigureDischarge(uint8_t chip, uint16_t cells) 
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
float **segment_cvRead(uint8_t cellIter)
{
  uint16_t rawCellVoltages[NUMCHIPS][12];

  float **cellVoltages = 0;
  cellVoltages = new float*[NUMCHIPS];

  ConfigureDischarge(0, cellIter);
  ConfigureDischarge(1, cellIter);
  SetChipConfigurations(chipConfigurations);

  LTC6804_adcv(); //this needs to be done before pulling from registers

  //get and print the cell voltages
  LTC6804_rdcv(0, NUMCHIPS, rawCellVoltages);
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
 * @brief Serializes or deserializes the data being written to or read from the LTC6280 register
 * 
 * @param numChips 
 * @param data 
 * @param commOutput 
 * @param operation 
 */
void segment_msgSerialization(uint8_t data[][3], uint8_t commData [][6], msg_Ser_Operation_t operation)
{
  if(operation == MSG_OPERATION_SERIALIZE)
  {
    for (int chip = 0; chip < NUMCHIPS; chip++)
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
    for (int chip = 0; chip < NUMCHIPS; chip++)
    {
      data[chip][0] = ((commData[chip][0] << 4) & 0xF0) | ((commData[chip][1] >> 4) & 0x0F);
      data[chip][1] = ((commData[chip][2] << 4) & 0xF0) | ((commData[chip][1] >> 3) & 0x0F);
      data[chip][2] = ((commData[chip][4] << 4) & 0xF0) | ((commData[chip][1] >> 5) & 0x0F);
    }

  }
  
}