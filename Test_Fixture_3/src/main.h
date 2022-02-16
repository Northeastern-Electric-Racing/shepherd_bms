#ifndef MAIN_H    // To make sure you don't declare the function more than once by including the header multiple times.
#define MAIN_H

#define I2C_START 0x60
#define I2C_STOP 0x10
#define I2C_BLANK 0x00
#define I2C_NOTRANSMIT 0x70

#define I2C_ACK 0x00
#define I2C_NACK 0x80
#define I2C_NACKSTOP 0x90

#include <Arduino.h>
#include <.\LinduinoFiles\LTC68041.h> //will need to change on machine basis

//Boolean type to designate whether to serialize or deserialize data
typedef enum
{
    MSG_OPERATION_SERIALIZE,
    MSG_OPERATION_DESERIALIZE
} msg_Ser_Operation_t;

void setup();
void loop();
void GetChipConfigurations(uint8_t localConfig[][6]);
void SetChipConfigurations(uint8_t localConfig[][6]);
void ConfigureDischarge(uint8_t chip, uint16_t cells);
void ConfigureCOMMRegisters(uint8_t numChips, uint8_t dataToWrite[][3], uint8_t commOutput [][6]);

void segment_msgSerialization(uint8_t data[][3], uint8_t commData [][6], msg_Ser_Operation_t operation);

#endif