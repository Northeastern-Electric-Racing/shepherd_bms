#include "main.h"
#include "config.h"

uint16_t rawCellVoltages[CHIPS][12];
float cellVoltages[CHIPS][12];
uint16_t rawTempVoltages[CHIPS][6];
int temps[CHIPS][32];
uint8_t chipConfigurations[CHIPS][6];
uint16_t cellTestIter = 0;
char keyPress = '0';
bool discharge = true;
bool balance = true;
char serialBuf[40];
uint8_t commRegData[CHIPS][6];
uint8_t i2cWriteData[CHIPS][3];
bool balancing[CHIPS][CELLS_S];
uint16_t dischargeCommand[CHIPS];

float minCellVal = 100;
int minCell = 0;
float maxCellVal = 0;
int maxCell = 0;
float deltaV = 0;

uint64_t currTime = 0;
uint64_t lastPrintTime = 0;
uint64_t lastVoltTime = 0;
uint64_t lastTempTime = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  delay(3000); // Allow time to connect and see boot up info
  Serial.println("Hello World!");
  LTC6804_initialize();

  // Turn OFF GPIO 1 & 2 pull downs
  GetChipConfigurations(chipConfigurations);
  for (int c = 0; c < CHIPS; c++)
  {
    chipConfigurations[c][0] |= 0x18;
  }
  SetChipConfigurations(chipConfigurations);

  Serial.print("Chip CFG:\n");
  for (int c = 0; c < CHIPS; c++)
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
    if (keyPress == ' ') {
      discharge = !discharge; // Toggle discharging
      Serial.println("TOGGLE DISCHARGE");
    } else if (keyPress == 'b') {
      balance = !balance;
      Serial.println("TOGGLE DISCHARGE MODE");
    }
  }

  // MEASURE VOLTAGES
  if (lastVoltTime + VOLT_DELAY < currTime) {
    // Run ADC on cell taps
    LTC6804_adcv(); //this needs to be done before pulling from registers

    // Pull and print the cell voltages from registers
    LTC6804_rdcv(0, CHIPS, rawCellVoltages);
  }

  // MEASURE TEMPS
  if (lastTempTime + TEMP_DELAY < currTime) {
    // Ensuring GPIO 1 & 2 pull downs are OFF
    GetChipConfigurations(chipConfigurations);
    for (int c = 0; c < CHIPS; c++)
    {
      chipConfigurations[c][0] |= 0x18;
    }
    SetChipConfigurations(chipConfigurations);

    updateAllTherms(2, temps);
  }

  // PRINT VOLTAGES AND TEMPS
  if (lastPrintTime + PRINT_DELAY < currTime) {
    lastPrintTime = currTime;

    Serial.print("Voltage:\n");
    minCellVal = 100;
    maxCellVal = 0;
    for (int c = 0; c < CHIPS; c++)
    {
      for (int cell = 0; cell < CELLS_S; cell++)
      {
        cellVoltages[c][cell] = float(rawCellVoltages[c][cell]) / 10000;
        if (cellVoltages[c][cell] < minCellVal) {
          minCellVal = cellVoltages[c][cell];
          minCell = c * CELLS_S + cell;
        }
        if (cellVoltages[c][cell] > maxCellVal) {
          maxCellVal = cellVoltages[c][cell];
          maxCell = c * CELLS_S + cell;
        }
        deltaV = maxCellVal - minCellVal;
        dtostrf(cellVoltages[c][cell], 6, 4, serialBuf);
        sprintf(serialBuf, "%sV\t", serialBuf);
        if(balancing[c][cell] && discharge) {
          Serial.print("\033[31m");
          Serial.print(serialBuf);
          Serial.print("\033[37m");
        } else {
          Serial.print(serialBuf);
        }
        // This would work, but arduino stupidly does not support floats in formatting :/
        // Keeping here for when we move to teensy (which I think can do this )
        // sprintf(serialBuf, "%1.4fV\t", cellVoltages[c][cell]);
        // Serial.print(serialBuf);
      }
      Serial.println();
    }
    Serial.println();

    Serial.print("Max Voltage: ");
    Serial.print(maxCellVal);
    Serial.print(", ");
    Serial.println(maxCell + 1);

    Serial.print("Min Voltage: ");
    Serial.print(minCellVal);
    Serial.print(", ");
    Serial.println(minCell + 1);

    Serial.print("Cell Delta: ");
    Serial.println(deltaV);
    Serial.println();
    
    Serial.print("ALL Temps:\n");
    for (int c = 0; c < 2; c++)
    {
      for (int i = 0; i < THERMISTORS; i++)
      {
        Serial.print(temps[c][i]);
        Serial.print("\t");
      }
      Serial.println();
    }
    Serial.println();
  }

  // DISCHARGE
  if (discharge) {
    if (balance) {
      delay(1000);
      // First attempt balancing algorithm
      for (int c = 0; c < CHIPS; c++) {
        dischargeCommand[c] = 0;
      }
      for (int c = 0; c < CHIPS; c++) {
        for (int cell = 0; cell < CELLS_S; cell ++) {
          balancing[c][cell] = (cellVoltages[c][cell] > minCellVal + MAX_DELTA_V);
          if (balancing[c][cell]) {
            dischargeCommand[c] |= 1 << cell;
          }
        }
      }
    } else {
      delay(200);
       // Just a counter for testing
      cellTestIter++;
      if (cellTestIter >= 512) {
        cellTestIter = 0;
      }
      for (int c = 0; c < CHIPS; c++) {
        dischargeCommand[c] = cellTestIter;
      }
    }

    GetChipConfigurations(chipConfigurations);
    for (int c = 0; c < CHIPS; c++) {
      ConfigureDischarge(c, dischargeCommand[c]);
    }
    SetChipConfigurations(chipConfigurations);
  } else {
    GetChipConfigurations(chipConfigurations);
    for (int c = 0; c < CHIPS; c++) {
      ConfigureDischarge(c, 0);
    }
    SetChipConfigurations(chipConfigurations);
  }
}



//last two bytes of recieved index are PEC and we want to dump them
void GetChipConfigurations(uint8_t localConfig[][6]) 
{ 
  uint8_t remoteConfig[CHIPS][8];
  LTC6804_rdcfg(CHIPS, remoteConfig);
  for (int chip = 0; chip < CHIPS; chip++)
  {
    for(int index = 0; index < 6; index++)
    {
      localConfig[chip][index] = remoteConfig[chip][index];
    }

  }

}

void SetChipConfigurations(uint8_t localConfig[][6]) 
{
  LTC6804_wrcfg(CHIPS, localConfig);
}

void ConfigureDischarge(uint8_t chip, uint16_t cells) 
{
  chipConfigurations[chip][4] = uint8_t(cells & 0x00FF);
  chipConfigurations[chip][5] = (chipConfigurations[chip][5] & 0xF0) + uint8_t(cells >> 8);
}


/**
 * @brief This takes the desired I2C command and serializes it to the 6 COMM registers of the LTC6804, might need to double check calculations in the future
 * 
 * @param CHIPS 
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
    for(int chip = 0; chip < CHIPS; chip++) {
      i2cWriteData[chip][0] = 0x92;
      i2cWriteData[chip][1] = 0x00;
      i2cWriteData[chip][2] = 0x00;
    }
    ConfigureCOMMRegisters(CHIPS, i2cWriteData, commRegData);
    LTC6804_wrcomm(CHIPS, commRegData);
    LTC6804_stcomm(24);

    // Turn on desired thermistor
    for(int chip = 0; chip < CHIPS; chip++) {
      i2cWriteData[chip][0] = 0x90;
      i2cWriteData[chip][1] = 0x08 + (therm - 1);
      i2cWriteData[chip][2] = 0x00;
    }
    ConfigureCOMMRegisters(CHIPS, i2cWriteData, commRegData);
    LTC6804_wrcomm(CHIPS, commRegData);
    LTC6804_stcomm(24);
  } else if (therm <= 16) {
    // Turn off competing multiplexor (therms 1-8)
    for(int chip = 0; chip < CHIPS; chip++) {
      i2cWriteData[chip][0] = 0x90;
      i2cWriteData[chip][1] = 0x00;
      i2cWriteData[chip][2] = 0x00;
    }
    ConfigureCOMMRegisters(CHIPS, i2cWriteData, commRegData);
    LTC6804_wrcomm(CHIPS, commRegData);
    LTC6804_stcomm(24);

    // Turn on desired thermistor
    for(int chip = 0; chip < CHIPS; chip++) {
      i2cWriteData[chip][0] = 0x92;
      i2cWriteData[chip][1] = 0x08 + (therm - 9);
      i2cWriteData[chip][2] = 0x00;
    }
    ConfigureCOMMRegisters(CHIPS, i2cWriteData, commRegData);
    LTC6804_wrcomm(CHIPS, commRegData);
    LTC6804_stcomm(24);
  } else if (therm <= 24) {
    // Turn off competing multiplexor (therms 25-32)
    for(int chip = 0; chip < CHIPS; chip++) {
      i2cWriteData[chip][0] = 0x96;
      i2cWriteData[chip][1] = 0x00;
      i2cWriteData[chip][2] = 0x00;
    }
    ConfigureCOMMRegisters(CHIPS, i2cWriteData, commRegData);
    LTC6804_wrcomm(CHIPS, commRegData);
    LTC6804_stcomm(24);

    // Turn on desired thermistor
    for(int chip = 0; chip < CHIPS; chip++) {
      i2cWriteData[chip][0] = 0x94;
      i2cWriteData[chip][1] = 0x08 + (therm - 17);
      i2cWriteData[chip][2] = 0x00;
    }
    ConfigureCOMMRegisters(CHIPS, i2cWriteData, commRegData);
    LTC6804_wrcomm(CHIPS, commRegData);
    LTC6804_stcomm(24);
  } else {
    // Turn off competing multiplexor (therms 17-24)
    for(int chip = 0; chip < CHIPS; chip++) {
      i2cWriteData[chip][0] = 0x94;
      i2cWriteData[chip][1] = 0x00;
      i2cWriteData[chip][2] = 0x00;
    }
    ConfigureCOMMRegisters(CHIPS, i2cWriteData, commRegData);
    LTC6804_wrcomm(CHIPS, commRegData);
    LTC6804_stcomm(24);

    // Turn on desired thermistor
    for(int chip = 0; chip < CHIPS; chip++) {
      i2cWriteData[chip][0] = 0x96;
      i2cWriteData[chip][1] = 0x08 + (therm - 25);
      i2cWriteData[chip][2] = 0x00;
    }
    ConfigureCOMMRegisters(CHIPS, i2cWriteData, commRegData);
    LTC6804_wrcomm(CHIPS, commRegData);
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