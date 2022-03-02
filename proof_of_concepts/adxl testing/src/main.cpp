#include <Arduino.h>
#include <Wire.h>

#define ADXL312_I2C_ADDR                0x53    //This depends on how the address pin is wired, 0x1D with Vcc and 0x53 with Gnd
#define ADXL312_DEVID_REG               0x00
#define ADXL312_DEVID                   0xE5
#define ADXL312_POWER_CTRL_REG          0x2D
#define ADXL312_POWER_CTRL_MEASURECMD   0x08    //This tells the accelerometer to read the value, might need to change it the sleep mode if not in use
#define ADXL312_XYZDATA_REG_OFFSET      0x32 


void setup() {
  Serial.begin(9600);
  Wire.begin();
  Serial.print("Initializing...");
  Wire.beginTransmission(ADXL312_I2C_ADDR);
  Wire.write(ADXL312_POWER_CTRL_REG);
  Wire.write(ADXL312_POWER_CTRL_MEASURECMD);
  Wire.endTransmission(false);
}

void loop() {
  uint8_t idmsg[1]={0};
  Wire.beginTransmission(ADXL312_I2C_ADDR);
  Wire.write(ADXL312_DEVID_REG);
  Wire.endTransmission(false);
  Wire.requestFrom(ADXL312_I2C_ADDR, 1);

  if (Wire.available())
  {
      uint8_t i2cByte=0;

      while(Wire.available())
      {
         idmsg[i2cByte] = Wire.read();
      }
  }

  Serial.print("Device ID:\t");
  Serial.println(idmsg[0],HEX);

  uint8_t *msg = new uint8_t[6];

  Wire.beginTransmission(ADXL312_I2C_ADDR);
  Wire.write(ADXL312_XYZDATA_REG_OFFSET);
  Wire.endTransmission(false);
  Wire.requestFrom(ADXL312_I2C_ADDR, 6);

  if (Wire.available())
  {
      uint8_t i2cByte=0;

      while(Wire.available())
      {
          msg[i2cByte] = Wire.read();
          i2cByte++;
      }
  }

  Serial.print("XYZ Data:\t");
  int16_t XYZDATA;
  for(int i=0;i<6;i+=2)
  {
    XYZDATA = (msg[i]) | (msg[i+1] << 8);
    Serial.println(XYZDATA,DEC);
  }

  Serial.println("cycle...");
  delay(5);
}