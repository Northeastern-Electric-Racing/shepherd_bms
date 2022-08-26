#include "main.h"

const uint8_t numChips = 2;
uint16_t rawCellVoltages[numChips][12];
float cellVoltages[numChips][12];
uint16_t rawTempVoltages[numChips][6];
float temps[numChips][6];
uint8_t chipConfigurations[numChips][6];
uint16_t cellTestIter = 0;
char key_press = '0';
bool discharge = true;
char serialBuf[20];

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println("Hello World!");
  LTC6804_initialize(); 
  //set_adc(MD_NORMAL, DCP_ENABLED, CELL_CH_ALL, AUX_CH_ALL);
  GetChipConfigurations(chipConfigurations);
  for (int c = 0; c < numChips; c++)
  {
    chipConfigurations[c][0] |= 0x18;
  }
  SetChipConfigurations(chipConfigurations);
}

void loop() {
  if (Serial.available()) {
    key_press = Serial.read();
    if ((key_press == ' ')) {
      discharge = !discharge;
    }
  }

  if (discharge) {
    cellTestIter++;
    if (cellTestIter >= 16)
    {
      cellTestIter = 0;
    }

    ConfigureDischarge(0, cellTestIter);
    ConfigureDischarge(1, cellTestIter);
    SetChipConfigurations(chipConfigurations);

    Serial.print("Discharge: ");
    Serial.println(cellTestIter, BIN);
  } else {
    ConfigureDischarge(0, 0);
    ConfigureDischarge(1, 0);
    SetChipConfigurations(chipConfigurations);
  }

  // Run ADC on cell taps
  LTC6804_adcv(); //this needs to be done before pulling from registers

  // Pull and print the cell voltages from registers
  LTC6804_rdcv(0, numChips, rawCellVoltages);
  Serial.print("Voltage:\n");
  for (int c = 0; c < numChips; c++)
  {
    for (int cell = 0; cell < 12; cell++)
    {
      cellVoltages[c][cell] = float(rawCellVoltages[c][cell]) / 10000;
      dtostrf(cellVoltages[c][cell], 6, 4, serialBuf);
      sprintf(serialBuf, "%sV\t", serialBuf);
      Serial.print(serialBuf);
      // This would work, but arduino stupidly does not support floats in formatting :/
      // Keeping here for when we move to teensy (which I think can do this )
      // sprintf(serialBuf, "%1.4fV\t", cellVoltages[c][cell]);
      // Serial.print(serialBuf);
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
      Serial.print(chipConfigurations[c][byte], HEX);
      Serial.print("\t");
    }
    Serial.println();
  }
  Serial.println();

  for (int c = 0; c < numChips; c++)
  {
    chipConfigurations[c][0] |= 0x18;
  }
  SetChipConfigurations(chipConfigurations);

  Serial.print("NEW Chip CFG:\n");
  for (int c = 0; c < numChips; c++)
  {
    for (int byte = 0; byte < 6; byte++)
    {
      Serial.print(chipConfigurations[c][byte], HEX);
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
    i2cWriteData[chip][0] = 0x92;
    i2cWriteData[chip][1] = 0x00;
    i2cWriteData[chip][2] = 0x00;
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
  LTC6804_stcomm(24);                             //Start communication by sending that data
  LTC6804_rdcomm(numChips, commReadData);        //Read data from I2C Multiplexer



  // Writing to the Multiplexer (Address 0x96, )
  for(int chip = 0; chip < numChips; chip++)
  {
    i2cWriteData[chip][0] = 0x90;
    i2cWriteData[chip][1] = 0x08;
    i2cWriteData[chip][2] = 0x00;
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
  LTC6804_stcomm(24);                             //Start communication by sending that data
  LTC6804_rdcomm(numChips, commReadData);        //Read data from I2C Multiplexer



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

  

  // Reading IO
  LTC6804_adax();
  //get and print the GPIO voltages
  LTC6804_rdaux(1, numChips, rawTempVoltages);
  Serial.print("Temps:\n");
  for (int c = 0; c < numChips; c++)
  {
    for (int temp = 0; temp < 6; temp++)
    {
      temps[c][temp] = float(rawTempVoltages[c][temp]) / 10000;
      Serial.print(temps[c][temp]);
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
    commOutput[chip][0] = 0x60 | (dataToWrite[chip][0] >> 4); // START + high side of B0
    commOutput[chip][1] = (dataToWrite[chip][0] << 4) | 0x00; // low side of B0 + NACK
    commOutput[chip][2] = 0x00 | (dataToWrite[chip][1] >> 4); // BLANK + high side of B1
    commOutput[chip][3] = (dataToWrite[chip][1] << 4) | 0x00; // low side of B1 + NACK
    commOutput[chip][4] = 0x00 | (dataToWrite[chip][2] >> 4); // BLANK + high side of B2
    commOutput[chip][5] = (dataToWrite[chip][2] << 4) | 0x09; // low side of B2 + STOP & NACK
  }
}