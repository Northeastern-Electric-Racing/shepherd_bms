/**
 * https://www.ti.com/lit/ds/symlink/amc6821.pdf?ts=1644706226375&ref_url=https%253A%252F%252Fwww.ti.com%252Fproduct%252FAMC6821%253Futm_source%253Dgoogle%2526utm_medium%253Dcpc%2526utm_campaign%253Dasc-sens-null-prodfolderdynamic-cpc-pf-google-wwe%2526utm_content%253Dprodfolddynamic%2526ds_k%253DDYNAMIC%2BSEARCH%2BADS%2526DCM%253Dyes%2526gclid%253DCj0KCQiA0p2QBhDvARIsAACSOOPKQVP7tfyxbaC8997ZjeHcQWZiSwAi1yblV-rFrJZ4BQS3xCwo1iYaAjmLEALw_wcB%2526gclsrc%253Daw.ds
 * ^Datasheet
 * 
 * @note Look at pages 16 and 17 for PWM operation
 */

#include <Arduino.h>
#include <Wire.h> //I2C library 


#define AMC6821_I2C_ADDR                    0x18
#define AMC6821_DEVID_REG                   0x3D
#define AMC6821_DEVID                       0x21
#define AMC6821_CONFIG1_REG                 0x00
#define AMC6821_CONFIG2_REG                 0x01
#define AMC6821_CONFIG3_REG                 0x3F
#define AMC6821_CONFIG4_REG                 0x04


/**
 * @note Duty Cycle is variable corresponding to 1/255 increments 
 *       (0x00 corresponds to 0% duty cycle and 0xFF(255) corresponds to 100% duty cycle)
 * @ref Refer to page 40 of datasheet
 */
#define AMC6821_DUTYCYCLE_REG               0x22


/**
 * @brief PWM I2C Characteristics Command Contents  (8 bits)
 * @ref Refer to page 41 of datasheet
 *                _______________________________________________________________________________________________________________
 * bit numbers:   |            7            |      6      |           5   4   3               |            2   1   0            |
 *                |-------------------------|-------------|-----------------------------------|---------------------------------|
 * contents:      |   Fan spin up disable   |   Reserved  |   PWM Frequency(see AMC macros)   |   Spin up time(see AMC macros)  |
 *                |_________________________|_____________|___________________________________|_________________________________|
 * 
 */
#define AMC6821_CHARACTERISTICS_REG         0x20
//PWM Frequencies
#define AMC6821_CHARACTERISTICS_1KHZ        0x0
#define AMC6821_CHARACTERISTICS_10KHZ       0x1
#define AMC6821_CHARACTERISTICS_20KHZ       0x2
#define AMC6821_CHARACTERISTICS_25KHZ       0x3    //(default) This frequency is what our fans operate at
#define AMC6821_CHARACTERISTICS_30KHZ       0x4
#define AMC6821_CHARACTERISTICS_40KHZ       0x5    //0x06 and 0x07 also result in 40kHz duty cycle
//Fan spin up times
#define AMC6821_CHARACTERISTICS_SPINUP_02   0x0    //02 means 0.2 seconds
#define AMC6821_CHARACTERISTICS_SPINUP_04   0x1    //04 means 0.4 seconds
#define AMC6821_CHARACTERISTICS_SPINUP_06   0x2    //...
#define AMC6821_CHARACTERISTICS_SPINUP_08   0x3
#define AMC6821_CHARACTERISTICS_SPINUP_1    0x4
#define AMC6821_CHARACTERISTICS_SPINUP_2    0x5    //(default)
#define AMC6821_CHARACTERISTICS_SPINUP_4    0x6
#define AMC6821_CHARACTERISTICS_SPINUP_8    0x7


void AMC6821write(uint8_t *msg, uint8_t numBytes);
bool AMC6821read(uint8_t *cmd, uint8_t numBytes);
void resetChip();
void writeConfig(uint8_t configNum, uint8_t config);

/******************************************************************************************************************/

void setup() {
  delay(3000);

  uint8_t msg[1];

  pinMode(13,OUTPUT);
  digitalWrite(13,HIGH);
  Serial.begin(9600);
  Wire.begin();

  //resetChip();
  delay(1000);

  writeConfig(4, 0x88);
  delay(100);
  writeConfig(2, 0x03);
  delay(100);
  writeConfig(3, 0x02);
  delay(100);
  writeConfig(1, 0x94);
  delay(100);

////////////////////////////////////////////////////
  Wire.beginTransmission(AMC6821_I2C_ADDR);
  Wire.write(AMC6821_CHARACTERISTICS_REG);
  Wire.write(0x1D);
  Wire.endTransmission();
////////////////////////////////////////////
////////////////////////////////////////////
  uint8_t cmd[1];

  cmd[0]=AMC6821_CONFIG1_REG;
  AMC6821write(cmd,1);
  if(AMC6821read(msg,1))
  {
    Serial.print("Config Reg 1:\t");
    Serial.println(msg[0],HEX);
  }

  delay(10);

  cmd[0]=AMC6821_CONFIG2_REG;
  AMC6821write(cmd,1);
  if(AMC6821read(msg,1))
  {
    Serial.print("Config Reg 2:\t");
    Serial.println(msg[0],HEX);
  }

  delay(10);

  cmd[0]=AMC6821_CONFIG3_REG;
  AMC6821write(cmd,1);
  if(AMC6821read(msg,1))
  {
    Serial.print("Config Reg 3:\t");
    Serial.println(msg[0],HEX);
  }

  delay(10);

  cmd[0]=AMC6821_CONFIG4_REG;
  AMC6821write(cmd,1);
  if(AMC6821read(msg,1))
  {
    Serial.print("Config Reg 4:\t");
    Serial.println(msg[0],HEX);
  }

  delay(10);

  cmd[0]=AMC6821_DEVID_REG;
  AMC6821write(cmd,1);
  if(AMC6821read(msg,1))
  {
    Serial.print("Device ID:\t");
    Serial.println(msg[0],HEX);
  }

  delay(10);

  cmd[0]=AMC6821_CHARACTERISTICS_REG;
  AMC6821write(cmd,1);
  if(AMC6821read(msg,1))
  {
    Serial.print("Characteristic Reg:\t");
    Serial.println(msg[0],HEX);
  }

  delay(10);

}

uint8_t fanspeed = 0;

void loop() {
  // put your main code here, to run repeatedly:
  uint8_t idmsg[1];
 
  Wire.beginTransmission(AMC6821_I2C_ADDR);
  Wire.write(AMC6821_DUTYCYCLE_REG);
  Wire.write(0xFF);
  Wire.endTransmission(false);

  delay(10);

  Wire.beginTransmission(AMC6821_I2C_ADDR);
  Wire.write(AMC6821_DUTYCYCLE_REG);
  Wire.endTransmission(false);
  Wire.requestFrom(AMC6821_I2C_ADDR, 1);

  if (Wire.available())
  {
      uint8_t i2cByte=0;

      while(Wire.available())
      {
         idmsg[i2cByte] = Wire.read();
      }
  }

  Serial.print("PWM Level =\t");
  Serial.println(idmsg[0], HEX);
  delay(1000);
}

/**
 * @brief Writing to the local I2C bus with the address of the ADXL312
 * 
 * @param msg 
 * @param numBytes
 */
void AMC6821write(uint8_t *msg, uint8_t numBytes)
{
    Wire.beginTransmission(AMC6821_I2C_ADDR);
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
bool AMC6821read(uint8_t *msg, uint8_t numBytes)
{
    Wire.requestFrom(AMC6821_I2C_ADDR, (int)numBytes);

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


void resetChip()
{
  uint8_t cmd[2] = {AMC6821_CONFIG2_REG, 0x80};
  AMC6821write(cmd,2);

  uint8_t cmd2[1];
  uint8_t msg[1];
  cmd[0]=AMC6821_CONFIG1_REG;
  AMC6821write(cmd2,1);
  if(AMC6821read(msg,1))
  {}

  delay(10);

}

void writeConfig(uint8_t configNum, uint8_t config)
{
  uint8_t cmd[2];
  cmd[1] = config;

  switch(configNum)
  {
    case 0x01:
      cmd[0] =  AMC6821_CONFIG1_REG;
      break;
    case 0x02:
      cmd[0] =  AMC6821_CONFIG2_REG;
      break;
    case 0x03:
      cmd[0] =  AMC6821_CONFIG3_REG;
      break;
    case 0x04:
      cmd[0] =  AMC6821_CONFIG4_REG;
      break;
    default:
      Serial.println("Unidentified Config #!");
      break;
  }
  
  AMC6821write(cmd,2);
}

//page 8 of datasheet talks about addresses 
//address written in hex 