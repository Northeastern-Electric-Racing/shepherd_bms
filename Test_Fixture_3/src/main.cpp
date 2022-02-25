#include "main.h"
#define NUMCHIPS 1

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
  set_adc(MD_NORMAL, DCP_DISABLED, CELL_CH_ALL, AUX_CH_ALL);
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
  SetChipConfigurations(chipConfigurations);

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
  uint8_t outputData[NUMCHIPS][3];
  uint8_t commReadData[numChips][6];
  uint8_t thermistors[NUMCHIPS];




  /***************************************************************************************
   * 
   * https://www.analog.com/media/en/technical-documentation/data-sheets/680412fc.pdf   --page 56 shows an example of I2C communication via LTC6804
   * 
  ****************************************************************************************/

  Serial.println("Thermistor Values:");
  for(int i=0 ; i<8 ; i++)
  {
    GetThermistor(thermistors, 0x90, i);      //we're writing to each MUX address
    //Serial.print(thermistors[0]); 
    //Serial.print("\t");
    //delay(5);
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
    commOutput[chip][0] = 0x60 | (dataToWrite[chip][0] >> 4);   //0x60 = start
    commOutput[chip][1] = (dataToWrite[chip][0] << 4) | 0x08; 
    commOutput[chip][2] = 0x00 | (dataToWrite[chip][1] >> 4);   //proceed with next byte
    commOutput[chip][3] = (dataToWrite[chip][1] << 4) | 0x09;
    commOutput[chip][4] = 0x70 | (dataToWrite[chip][2] >> 4);   //no transmit
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


/**
 * @brief Serializes or deserializes the data being written to or read from the LTC6280 register
 * https://www.analog.com/media/en/technical-documentation/data-sheets/680412fc.pdf  --Page 31
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
      commData[chip][0] = 0x60 | (data[chip][0] >> 4);      //0x60 = I2C start
      commData[chip][1] = (data[chip][0] << 4) | 0x08;      //0x08 = Master NACK
      commData[chip][2] = 0x00 | (data[chip][1] >> 4);      //0x00 = Proceed with data sending
      commData[chip][3] = (data[chip][1] << 4) | 0x09;      //0x09 = Master NACK and Stop
      commData[chip][4] = 0x70 | (data[chip][2] >> 4);      //0x70 = No Transmit (No byte being sent)
      commData[chip][5] = (data[chip][2] << 4) | 0x09;      //0x09 = Master NACK and Stop
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


/**
 * @brief Get the thermistor reading
 * 
 * @todo Look into page 55 of polling ADC status????? 
 * 
 * @param thermisters 
 * @param MUX 
 * @param mux_channel 
 */
void GetThermistor(uint8_t thermisters[], uint8_t MUX, uint8_t mux_channel) 
{
  uint8_t commRegisterData[numChips][6];
  uint8_t i2cWriteData[numChips][3];
  for (int chip = 0; chip < numChips; chip++){
    i2cWriteData[chip][0] = MUX;
    i2cWriteData[chip][1] = mux_channel | 0x8;    //0x8 is the "enable" bit, MUX needs to be disabled after reading
    i2cWriteData[chip][2] = 0;
  }
  ConfigureCOMMRegisters(numChips, i2cWriteData, commRegisterData);
  LTC6804_wrcomm(numChips, commRegisterData);
  LTC6804_stcomm(numChips * 3);
  delay(5);
  LTC6804_adax();     //retrieves GPIO?
  delay(5);
  uint16_t aux_data[numChips][6];
  LTC6804_rdaux(0,1,aux_data);
  for (int chip = 0; chip < numChips; chip++) {
    thermisters[chip] = aux_data[chip][AUX_CH_GPIO3];     //This might be the wrong GPIO possibly?
    Serial.print(aux_data[chip][AUX_CH_GPIO1]);
    Serial.print("\t");
    Serial.print(aux_data[chip][AUX_CH_GPIO2]);
    Serial.print("\t");
    Serial.print(aux_data[chip][AUX_CH_GPIO3]);
    Serial.print("\t");
    Serial.print(aux_data[chip][AUX_CH_GPIO4]);
    Serial.print("\t");
    Serial.print(aux_data[chip][AUX_CH_GPIO5]);
    Serial.print("\t");
  }
  closeMUX(MUX);
  Serial.println();
  return;
}

void closeMUX(uint8_t MUX)
{
  uint8_t commRegisterData[numChips][6];
  uint8_t i2cWriteData[numChips][3];
  for (int chip = 0; chip < numChips; chip++){
    i2cWriteData[chip][0] = MUX;
    i2cWriteData[chip][1] = 0;    //enable bit of MUX channel set to 0, closes all channels
    i2cWriteData[chip][2] = 0;
  }
}