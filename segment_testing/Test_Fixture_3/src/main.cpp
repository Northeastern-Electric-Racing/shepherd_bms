#include "main.h"

const uint8_t numChips = 1;
uint16_t rawCellVoltages[numChips][12];
float cellVoltages[numChips][12];
uint8_t chipConfigurations[numChips][6];
uint16_t cellTestIter = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println("Hello World!");
  LTC6804_initialize(); 
  //set_adc(MD_NORMAL, DCP_ENABLED, CELL_CH_ALL, AUX_CH_ALL);
}

void loop() {

  cellTestIter++;
  if (cellTestIter >= 16)
  {
    cellTestIter = 0;
  }

  ConfigureDischarge(0, cellTestIter);
  ConfigureDischarge(1, cellTestIter);
  SetChipConfigurations(chipConfigurations);

  LTC6804_adcv(); //this needs to be done before pulling from registers

  //get and print the cell voltages
  LTC6804_rdcv(0, numChips, rawCellVoltages);
  Serial.print("Voltage:\n");
  for (int c = 0; c < numChips; c++)
  {
    for (int cell = 0; cell < 12; cell++)
    {
      cellVoltages[c][cell] = float(rawCellVoltages[c][cell]) / 10000;
      Serial.print(cellVoltages[c][cell]);
      Serial.print("\t");
    }
    Serial.println();
  }
  Serial.println();

  GetChipConfigurations(chipConfigurations);

  Serial.print("Chip CFG:\n");
  for (int c = 0; c < numChips; c++)
  {
    for (int byte = 0; byte < 6; byte++)
    {
      Serial.print(chipConfigurations[c][byte]);
      Serial.print("\t");
    }
    Serial.println();
  }
  Serial.println();

  uint8_t commRegisterData[numChips][6];
  uint8_t i2cWriteData[numChips][3];

  uint8_t commReadData[numChips][6];




  /***************************************************************************************
   * 
   * https://www.analog.com/media/en/technical-documentation/data-sheets/680412fc.pdf   --page 56 shows an example of I2C communication via LTC6804
   * 
  ****************************************************************************************/

  // Writing to the Multiplexer (Address 0x96, )
  for(int chip = 0; chip < numChips; chip++)
  {
    i2cWriteData[chip][0] = 0x96;
    i2cWriteData[chip][1] = 0x08;
    i2cWriteData[chip][2] = 0xAA;   //was "data to be stored in EEPROM" but we're not using an EEPROM rn
  }

  ConfigureCOMMRegisters(numChips, i2cWriteData, commRegisterData);

  LTC6804_wrcomm(numChips, commRegisterData);
  LTC6804_rdcomm(numChips, commReadData);    //Reading what we just wrote to the COMM register

  Serial.print("What we're writing to the LTC chip:\n");
  for (int c = 0; c < numChips; c++)
  {
    for (int byte = 0; byte < 6; byte++)
    {
      Serial.print(commReadData[c][byte], HEX);     //printing what we just wrote to COMM register
      Serial.print("\t");
    }
    Serial.println();
  }
  Serial.println();

  /*****************************************************************/

  LTC6804_wrcomm(numChips, commRegisterData); //Loading LTC register with data to send just in case, not sure if necessary in this case because we already loaded it
  LTC6804_stcomm(numChips * 2);               //Start communication by sending that data
  LTC6804_rdcomm(numChips, commReadData);     //Read data from I2C Multiplexer

  Serial.print("Data from the Multiplexer?:\n");
  for (int c = 0; c < numChips; c++)
  {
    for (int byte = 0; byte < 6; byte++)
    {
      Serial.print(commReadData[c][byte], HEX);
      Serial.print("\t");
    }
    Serial.println();
  }
  Serial.println();


  delay(1000);
}

//last two bytes of recieved index are PEC and we want to dump them
void GetChipConfigurations(uint8_t localConfig[][6]) 
{ 
  uint8_t remoteConfig[numChips][8];
  LTC6804_rdcfg(numChips, remoteConfig);
  for (int chip = 0; chip < numChips; chip++)
  {
    for(int index = 0; index < 6; index++)
    {
      localConfig[chip][index] = remoteConfig[chip][index];
    }

  }

}

void SetChipConfigurations(uint8_t localConfig[][6]) 
{
  LTC6804_wrcfg(numChips, localConfig);
}

void ConfigureDischarge(uint8_t chip, uint16_t cells) 
{
  chipConfigurations[chip][4] = uint8_t(cells & 0x00FF);
  chipConfigurations[chip][5] = (chipConfigurations[chip][5] & 0xF0) + uint8_t(cells >> 8);
}


/**
 * @brief This takes the desired I2C command and serializes it to the 6 COMM registers of the LTC6804, might need to double check calculations in the future
 * 
 * @param numChips 
 * @param dataToWrite 
 * @param commOutput 
 */
void ConfigureCOMMRegisters(uint8_t numChips, uint8_t dataToWrite[][3], uint8_t commOutput [][6])
{
  for (int chip = 0; chip < numChips; chip++)
  {
    commOutput[chip][0] = 0x60 | (dataToWrite[chip][0] >> 4);
    commOutput[chip][1] = (dataToWrite[chip][0] << 4) | 0x08;
    commOutput[chip][2] = 0x00 | (dataToWrite[chip][1] >> 4);
    commOutput[chip][3] = (dataToWrite[chip][1] << 4) | 0x09;
    commOutput[chip][4] = 0x70 | (dataToWrite[chip][2] >> 4);
    commOutput[chip][5] = (dataToWrite[chip][2] << 4) | 0x09;

    /*
    commOutput[chip][0] = 0x60 + (dataToWrite[chip][0] >> 4);
    commOutput[chip][1] = (dataToWrite[chip][0] << 4) + 0x08;
    commOutput[chip][2] = 0x60 + (dataToWrite[chip][1] >> 4);
    commOutput[chip][3] = (dataToWrite[chip][1] << 4) + 0x08;
    commOutput[chip][4] = 0x60 + (dataToWrite[chip][2] >> 4);
    commOutput[chip][5] = (dataToWrite[chip][2] << 4) + 0x08;
    */
  }
}