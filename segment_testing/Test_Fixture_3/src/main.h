#ifndef MAIN_H    // To make sure you don't declare the function more than once by including the header multiple times.
#define MAIN_H

#define I2C_START 0x60
#define I2C_STOP 0x10
#define I2C_BLANK 0x00
#define I2C_NOTRANSMIT 0x70

#define I2C_ACK 0x00
#define I2C_NACK 0x80
#define I2C_NACKSTOP 0x90

#define THERM_REF 4700 // Reference resistor for thermistor measurments / voltage divider

#include <Arduino.h>
#include <LinduinoFiles/LTC68041.h> //will need to change on machine basis

void setup();
void loop();
void GetChipConfigurations(uint8_t localConfig[][6]);
void SetChipConfigurations(uint8_t localConfig[][6]);
void ConfigureDischarge(uint8_t chip, uint16_t cells);
void ConfigureCOMMRegisters(uint8_t numChips, uint8_t dataToWrite[][3], uint8_t commOutput [][6]);
void SelectTherm(uint8_t therm);
void updateAllTherms(uint8_t numChips, int out[][32]);
int8_t steinhartEq(int8_t R);
uint8_t steinhartEst(uint32_t R);
int voltToTemp(uint32_t V);

#endif