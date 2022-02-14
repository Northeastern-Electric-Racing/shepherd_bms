#include "nerduino.h"

nerduino::nerduino()
{
    Wire.begin();
    Serial.begin(9600);
}

nerduino::~nerduino(){}
/**************************************/

/**
 * @brief Construct a new ADXL312 object and verify and configure the functionality of the chip
 * 
 */
ADXL312::ADXL312()
{
    if(verifyFunctionality())
    {
        configureForMeasurement();
        return;
    }
}

ADXL312::~ADXL312(){}

/**
 * @brief Writing to the local I2C bus with the address of the ADXL312
 * 
 * @param msg 
 * @param numBytes 
 */
void ADXL312::ADXL312write(uint8_t *msg, uint8_t numBytes)
{
    Wire.beginTransmission(ADXL312_I2C_ADDR);
    for(uint8_t i=0; i<numBytes;i++)
    {
        Wire.write(msg[i]);
    }
    Wire.endTransmission(false);
}

/**
 * @brief Requesting data to read in from the ADXL312
 * 
 * @param msg 
 * @param numBytes 
 * @return true 
 * @return false 
 */
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

/**
 * @brief Verify the functionality of the onboard ADXL312
 * 
 * @return true 
 * @return false 
 */
bool ADXL312::verifyFunctionality()
{
    uint8_t cmd[1] = {ADXL312_DEVID_REG}; 
    uint8_t msg[1];
    ADXL312write(cmd, 1);
    if(ADXL312read(msg, 1))
    {
        if(msg[0] == ADXL312_DEVID)
        {
            Serial.println("ADXL312 connection online...");
            return true;
        }
    }
    Serial.println("ERROR: Connection to ADXL312 is not available!");
    return false;
}

/**
 * @brief Setting the ADXL312 to actively measure the XYZ data
 * 
 */
void ADXL312::configureForMeasurement()
{
    uint8_t cmd[2] = {ADXL312_POWER_CTRL_REG, ADXL312_POWER_CTRL_MEASURECMD}; 
    ADXL312write(cmd, 1);
} 

/**
 * @brief Retrieves the XYZ data from the ADXL312
 * 
 * @return uint8_t* msg
 */
uint8_t *ADXL312::getXYZ()
{
    uint8_t cmd[1] = {ADXL312_XYZDATA_REG_OFFSET}; 
    uint8_t *msg = new uint8_t[6];
    ADXL312write(cmd, 1);
    if(ADXL312read(msg, 6))
    {
        return msg;
    }
    Serial.println("ERROR: XYZ Data not available!");
    return msg;
}
