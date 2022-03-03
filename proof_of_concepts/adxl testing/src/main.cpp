#include <Arduino.h>
#include <Wire.h>

#define ADXL312_I2C_ADDR                0x53    //This depends on how the address pin is wired, 0x1D with Vcc and 0x53 with Gnd
#define ADXL312_DEVID_REG               0x00
#define ADXL312_DEVID                   0xE5
#define ADXL312_POWER_CTRL_REG          0x2D
#define ADXL312_POWER_CTRL_MEASURECMD   0x08    //This tells the accelerometer to read the value, might need to change it the sleep mode if not in use
#define ADXL312_XYZDATA_REG_OFFSET      0x32 


bool verifyFunctionality();
void configureForMeasurement();
void getXYZ(uint8_t *msg);


void setup() {
  Serial.begin(9600);
  Wire.begin();
  Serial.print("Initializing...");
  configureForMeasurement();
}

void loop() {
  if(verifyFunctionality())
  {
    Serial.println("ahhh");
  }

  uint8_t *msg = new uint8_t[6];
  int16_t XYZDATA;

  getXYZ(msg);
  for(int i=0;i<6;i+=2)
  {
    XYZDATA = (msg[i]) | (msg[i+1] << 8);
    Serial.println(XYZDATA,DEC);
  }
  
  Serial.println("cycle...");

  delay(500);
}

/**
 * @brief Requesting data to read in from the ADXL312
 * 
 * @param msg 
 * @param numBytes 
 * @return true 
 * @return false 
 */
bool ADXL312read(uint8_t *msg, uint8_t numBytes)
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
 * @brief Writing to the local I2C bus with the address of the ADXL312
 * 
 * @param msg 
 * @param numBytes
 */
void ADXL312write(uint8_t *msg, uint8_t numBytes)
{
    Wire.beginTransmission(ADXL312_I2C_ADDR);
    for(uint8_t i=0; i<numBytes;i++)
    {
        Wire.write(msg[i]);
    }
    Wire.endTransmission(false);
}


/**
 * @brief Verify the functionality of the onboard ADXL312
 * 
 * @return true 
 * @return false 
 */
bool verifyFunctionality()
{
    uint8_t cmd[1] = {ADXL312_DEVID_REG}; 
    uint8_t msg[1];
    ADXL312write(cmd, 1);
    if(ADXL312read(msg, 1))
    {
        Serial.println(msg[0], HEX);
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
void configureForMeasurement()
{
    uint8_t cmd[2] = {ADXL312_POWER_CTRL_REG, ADXL312_POWER_CTRL_MEASURECMD}; 
    ADXL312write(cmd, 1);
}

/**
 * @brief Retrieves the XYZ data from the ADXL312
 * 
 * @return uint8_t* msg
 */
void getXYZ(uint8_t *msg)
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