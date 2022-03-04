#include "nerduino.h"

nerduino::nerduino()
{
    Wire.begin();
    Serial.begin(9600);
}

nerduino::~nerduino(){}


/**
 * @brief Constructor for  SHT30 Humidity and Temp Sensor class
 */
SHT30::SHT30()
{
    reset();
    Serial.print("Heating element is enabled: ") 
    isHeaterEnabled();
}
/**
 * @brief Destructor for class
 */
SHT30::~SHT30(){}
/**
 * @brief Writing to the local I2C bus with the address of the SHT30
 * 
 * @param msg 
 * @param numBytes 
 */
void SHT30::SHT30write(uint16_t *msg, uint8_t numBytes)
{
    Wire.beginTransmission(SHT30_I2C_ADDR+SHT30_I2C_ADDR_WRITE_BIT)
    for (uint8_t i=0; i< numBytes; i++)
    {
        Wire.write(msg);
    }
    Wire.endTransmission(false);
}
/**
 * @brief Writing to the local I2C bus with the address of the SHT30
 * 
 * @param msg 
 * @param numBytes 
 */
bool SHT30::SHT30read(uint8_t *msg, uint8_t numBytes)
{
    Wire.requestFrom(SHT30_I2C_ADDR, int numBytes);
    if(Wire.available())
    {
        uint8_t dataBytes=0;
        while(Wire.available())
        {
            msg[dataBytes] = Wire.read();
            dataBytes++;
        }
        return true;
    }
    else{
        Serial.println("Reading Failed");
        return false;
    }
}
/**
 * @brief Performs a reading of the status register, from ADAFRUIT SHT30 github ino
 * returns the 16-bit status register
 */
uint16_t readStatusReg(void)                                    
{
    SHT30write(SHT30_READSTATUS, 1)
    uint8_t data[3];
    SHT30read(data, 3);
    uint16_t status = data[0];
    status <<= 8;
    status |= data[1];
    return status;
}
/**
 * @brief Performs a soft reset of the SHT30 sensor
 */
void reset(void)                                    
{
    SHT30write(SHT30_SOFTRESET, 1)
    delay(10)
    Serial.println("Successful reset")
}
/**
 * @brief 
 * Return a bool: true if heating element is enable, false is disabled
 */
bool isHeaterEnabled(); 
{
    uint16_t regVal = readStatusReg();
    return (bool) bitread(regVal, SHT30_REG_HEATER_BIT);
}
/**
 * @brief Heating element control, allows for the removal of condensation on sensor
 * 
 * @param bool{true = enable, false = disable}
 */
void heater(bool h);
{
    if (h = true)
        SHT30write(SHT30_HEATEREN, 1);
    else
        SHT30write(SHT30_HEATERDIS, 1);    
}
/**
 * @brief Writing to the local I2C bus a start measurement command, then reads the measurement
 * 
 * Returns true if successfull printing of values
 */
bool SHT30::getTempHumid(void)
{
    uint16_t cmd[1] = {SHT30_START_CMD_WCS}; 
    uint8_t *msg = new uint8_t[6];
    SHT30write(cmd, 1);
    SHT30read(msg, 6);
    float cTemp = ((((msg[0] * 256.0) + msg[1]) * 175) / 65535.0) - 45;
    float fTemp = (cTemp * 1.8) + 32;
    float humidity = ((((msg[3] * 256.0) + msg[4]) * 100) / 65535.0);
    Serial.print("Temperature C: ")
    Serial.print(cTemp);
    Serial.println(" C");
    Serial.print("Temperature C: ")
    Serial.print(fTemp);
    Serial.println(" F");
    Serial.print("Relative Humidity: ")
    Serial.print(humidity;
    Serial.println(" %RH");
    return true;
}