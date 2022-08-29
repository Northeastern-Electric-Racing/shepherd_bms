#include "main.h"

const uint8_t numChips = 2;
uint16_t rawCellVoltages[numChips][12];
float cellVoltages[numChips][12];
uint16_t rawTempVoltages[numChips][6];
int temps[numChips][32];
uint8_t chipConfigurations[numChips][6];
uint16_t cellTestIter = 0;
char keyPress = '0';
bool discharge = false;
char serialBuf[20];
uint8_t commRegData[numChips][6];
uint8_t i2cWriteData[numChips][3];
const uint32_t TEMP_CONV[] = {33200, 31500, 29900, 28400, 26900, 25600, 24300, 23100, 21900, 20900, 19900, 18900, 18000, 17100, 16300, 15500, 14800, 14100, 13500, 12900, 12300, 11700, 11200, 10700, 10200, 9780, 9350, 8940, 8560, 8190, 7840, 7510, 7190, 6890, 6610, 6340, 6080, 5830, 5590, 5370, 5150, 4950, 4750, 4570, 4390, 4220, 4060, 3900, 3750, 3610, 3470, 0};
const uint32_t VOLT_TEMP_CONV[] = {45140, 44890, 44640, 44370, 44060, 43800, 43510, 43210, 42900, 42560, 42240, 41900, 41550, 41170, 40820, 40450, 40040, 39650, 39220, 38810, 38370, 37950, 37500, 37090, 36650, 36180, 35670, 35220, 34740, 34230, 33770, 33270, 32770, 32280, 31770, 31260, 30750, 30240, 29720, 29220, 28710, 28200, 27680, 27160, 26660, 26140, 25650, 25130, 24650, 24150, 23660, 23170, 22670, 22190, 21720, 21240, 0};
uint64_t currTime = 0;
uint64_t lastPrintTime = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(96000);
  delay(3000); // Allow time to connect and see boot up info
  Serial.println("Hello World!");
  LTC6804_initialize();

  // Turn OFF GPIO 1 & 2 pull downs
  GetChipConfigurations(chipConfigurations);
  for (int c = 0; c < numChips; c++)
  {
    chipConfigurations[c][0] |= 0x18;
  }
  SetChipConfigurations(chipConfigurations);

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
}

void loop() {
  currTime = millis();
  // Keypress processing
  if (Serial.available()) { // Check for key presses
    keyPress = Serial.read(); // Read key
    if ((keyPress == ' ')) {
      discharge = !discharge; // Toggle discharging
    }
  }

  if (discharge) {
    // Just a counter for testing
    cellTestIter++;
    if (cellTestIter >= 512)
    {
      cellTestIter = 0;
    }

    GetChipConfigurations(chipConfigurations);
    ConfigureDischarge(0, cellTestIter);
    ConfigureDischarge(1, cellTestIter);
    SetChipConfigurations(chipConfigurations);
  } else {
    GetChipConfigurations(chipConfigurations);
    ConfigureDischarge(0, 0);
    ConfigureDischarge(1, 0);
    SetChipConfigurations(chipConfigurations);
  }

  if (lastPrintTime + 2000 < currTime) {
    lastPrintTime = currTime;

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

    // Ensuring GPIO 1 & 2 pull downs are OFF
    GetChipConfigurations(chipConfigurations);
    for (int c = 0; c < numChips; c++)
    {
      chipConfigurations[c][0] |= 0x18;
    }
    SetChipConfigurations(chipConfigurations);
    /*
    SelectTherm(1);
    SelectTherm(17);
    LTC6804_adax(); // Run ADC for AUX (GPIOs and refs)
    LTC6804_rdaux(0, numChips, rawTempVoltages); // Fetch ADC results from AUX registers
    for (int c = 0; c < numChips; c++)
    {
      Serial.print(rawTempVoltages[c][0]);
      Serial.print(" ");
      Serial.print(rawTempVoltages[c][1]);
      Serial.print(" ");
      Serial.println(rawTempVoltages[c][2]);
      temps[c][0] = THERM_REF * (float(rawTempVoltages[c][0]) / 10000) / ((float(rawTempVoltages[c][2]) / 10000) - (float(rawTempVoltages[c][0]) / 10000));
      temps[c][16] = THERM_REF * (float(rawTempVoltages[c][1]) / 10000) / ((float(rawTempVoltages[c][2]) / 10000) - (float(rawTempVoltages[c][1]) / 10000));
      Serial.print(temps[c][0]);
      Serial.print(" ");
      Serial.println(temps[c][16]);
      Serial.println();
    }*/

    updateAllTherms(2, temps);
    
    Serial.print("ALL Temps:\n");
    for (int c = 0; c < 2; c++)
    {
      for (int i = 0; i < 32; i++)
      {
        Serial.print(temps[c][i]);
        Serial.print("\t");
      }
      Serial.println();
    }
    Serial.println();
  }

  delay(150);
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
    commOutput[chip][1] = (dataToWrite[chip][0] << 4) | 0x00; // low side of B0 + ACK
    commOutput[chip][2] = 0x00 | (dataToWrite[chip][1] >> 4); // BLANK + high side of B1
    commOutput[chip][3] = (dataToWrite[chip][1] << 4) | 0x00; // low side of B1 + ACK
    commOutput[chip][4] = 0x00 | (dataToWrite[chip][2] >> 4); // BLANK + high side of B2
    commOutput[chip][5] = (dataToWrite[chip][2] << 4) | 0x09; // low side of B2 + STOP & NACK
  }
}

/**
 * @brief Configures multiplexors to expose the desired therrmistor for measurment
 * 
 * @param therm Number thermistor requested
 */
void SelectTherm(uint8_t therm) {
  // Exit if out of range values
  if (therm < 0 || therm > 32) {
    return;
  }
  if (therm <= 8) {
    // Turn off competing multiplexor (therms 9-16)
    for(int chip = 0; chip < numChips; chip++) {
      i2cWriteData[chip][0] = 0x92;
      i2cWriteData[chip][1] = 0x00;
      i2cWriteData[chip][2] = 0x00;
    }
    ConfigureCOMMRegisters(numChips, i2cWriteData, commRegData);
    LTC6804_wrcomm(numChips, commRegData);
    LTC6804_stcomm(24);

    // Turn on desired thermistor
    for(int chip = 0; chip < numChips; chip++) {
      i2cWriteData[chip][0] = 0x90;
      i2cWriteData[chip][1] = 0x08 + (therm - 1);
      i2cWriteData[chip][2] = 0x00;
    }
    ConfigureCOMMRegisters(numChips, i2cWriteData, commRegData);
    LTC6804_wrcomm(numChips, commRegData);
    LTC6804_stcomm(24);
  } else if (therm <= 16) {
    // Turn off competing multiplexor (therms 1-8)
    for(int chip = 0; chip < numChips; chip++) {
      i2cWriteData[chip][0] = 0x90;
      i2cWriteData[chip][1] = 0x00;
      i2cWriteData[chip][2] = 0x00;
    }
    ConfigureCOMMRegisters(numChips, i2cWriteData, commRegData);
    LTC6804_wrcomm(numChips, commRegData);
    LTC6804_stcomm(24);

    // Turn on desired thermistor
    for(int chip = 0; chip < numChips; chip++) {
      i2cWriteData[chip][0] = 0x92;
      i2cWriteData[chip][1] = 0x08 + (therm - 9);
      i2cWriteData[chip][2] = 0x00;
    }
    ConfigureCOMMRegisters(numChips, i2cWriteData, commRegData);
    LTC6804_wrcomm(numChips, commRegData);
    LTC6804_stcomm(24);
  } else if (therm <= 24) {
    // Turn off competing multiplexor (therms 25-32)
    for(int chip = 0; chip < numChips; chip++) {
      i2cWriteData[chip][0] = 0x96;
      i2cWriteData[chip][1] = 0x00;
      i2cWriteData[chip][2] = 0x00;
    }
    ConfigureCOMMRegisters(numChips, i2cWriteData, commRegData);
    LTC6804_wrcomm(numChips, commRegData);
    LTC6804_stcomm(24);

    // Turn on desired thermistor
    for(int chip = 0; chip < numChips; chip++) {
      i2cWriteData[chip][0] = 0x94;
      i2cWriteData[chip][1] = 0x08 + (therm - 17);
      i2cWriteData[chip][2] = 0x00;
    }
    ConfigureCOMMRegisters(numChips, i2cWriteData, commRegData);
    LTC6804_wrcomm(numChips, commRegData);
    LTC6804_stcomm(24);
  } else {
    // Turn off competing multiplexor (therms 17-24)
    for(int chip = 0; chip < numChips; chip++) {
      i2cWriteData[chip][0] = 0x94;
      i2cWriteData[chip][1] = 0x00;
      i2cWriteData[chip][2] = 0x00;
    }
    ConfigureCOMMRegisters(numChips, i2cWriteData, commRegData);
    LTC6804_wrcomm(numChips, commRegData);
    LTC6804_stcomm(24);

    // Turn on desired thermistor
    for(int chip = 0; chip < numChips; chip++) {
      i2cWriteData[chip][0] = 0x96;
      i2cWriteData[chip][1] = 0x08 + (therm - 25);
      i2cWriteData[chip][2] = 0x00;
    }
    ConfigureCOMMRegisters(numChips, i2cWriteData, commRegData);
    LTC6804_wrcomm(numChips, commRegData);
    LTC6804_stcomm(24);
  }
}

void updateAllTherms(uint8_t numChips, int out[][32]) {
  for (int therm = 1; therm <= 16; therm++) {
    SelectTherm(therm);
    delay(5);
    SelectTherm(therm + 16);
    delay(10);
    LTC6804_adax(); // Run ADC for AUX (GPIOs and refs)
    delay(10);
    LTC6804_rdaux(0, numChips, rawTempVoltages); // Fetch ADC results from AUX registers
    for (int c = 0; c < numChips; c++) {
      out[c][therm - 1] = voltToTemp(uint32_t(rawTempVoltages[c][0] * (float(rawTempVoltages[c][2]) / 50000)));
      out[c][therm + 15] = voltToTemp(uint32_t(rawTempVoltages[c][1] * (float(rawTempVoltages[c][2]) / 50000)));
    }
  }
}

int8_t steinhartEq(int8_t R) {
  return int8_t(1 / (0.001125308852122 + (0.000234711863267 * log(R)) + (0.000000085663516 * (pow(log(R), 3)))));
}

/* Cause im lazy: https://www.lasercalculator.com/ntc-thermistor-calculator/
Currently only plots to 0-50. All values outside are binned to 0 or 50
*/
uint8_t steinhartEst(uint32_t R) {
  int i = 0;
  while (R < TEMP_CONV[i]) i++;
  return i;
}

int voltToTemp(uint32_t V) {
  int i = 0;
  while (V < VOLT_TEMP_CONV[i]) i++;
  return i - 5;
}