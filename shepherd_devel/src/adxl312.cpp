#include "adxl312.h"


ADXL312::ADXL312()
{
    if(verifyFunctionality())
    {
        configureForMeasurement();
        return;
    }
    Serial.println("~~~~~~~~~~~~~~~~WARNING: Unable to verify functionality of ADXL312~~~~~~~~~~~~~~~~~~~~~~~");
}

ADXL312::~ADXL312(){}


void ADXL312::ADXL312write(uint8_t *cmd, uint8_t numBytes)
{
    Wire.beginTransmission(ADXL312_I2C_ADDR);
    for(uint8_t i=0; i<numBytes;i++)
    {
        Wire.write(cmd[i]);
    }
    Wire.endTransmission(false);
}


bool ADXL312::ADXL312read(uint8_t *msg, uint8_t numBytes)
{
    Wire.requestFrom(ADXL312_I2C_ADDR, (int)numBytes);

    if (Wire.available())
    {
        uint8_t i2cByte=0;
        while(Wire.available())
        {
            msg[i2cByte] = Wire.read();
            i2cByte++;
        }
        return true;
    }

    return false;
}


bool ADXL312::verifyFunctionality()
{
    uint8_t cmd[1] = {ADXL312_DEVID_REG}; 
    uint8_t msg[1];
    ADXL312write(cmd, 1);
    if(ADXL312read(msg, 1))
    {
        Serial.println(msg[0]);
        if(msg[0] == ADXL312_DEVID)
        {
            Serial.println("ADXL312 connection online...");
            return true;
        }
    }
    Serial.println("ERROR: Connection to ADXL312 is not available!");
    return false;
}


void ADXL312::configureForMeasurement()
{
    uint8_t cmd[2] = {ADXL312_POWER_CTRL_REG, ADXL312_POWER_CTRL_MEASURECMD}; 
    ADXL312write(cmd, 1);
}


void ADXL312::getXYZ(uint8_t *msg)
{
    uint8_t cmd[1] = {ADXL312_XYZDATA_REG_OFFSET}; 
    ADXL312write(cmd, 1);
    if(ADXL312read(msg, 6))
    {
        return;
    }
    Serial.println("ERROR: XYZ Data not available!");
    return;
}