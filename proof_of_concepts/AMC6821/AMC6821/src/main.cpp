#include <Arduino.h>
#include <Wire.h> //I2C library 

void setup() {
  // put your setup code here, to run once:

  Serial.begin(9600);
  Wire.begin();
}

void loop() {
  // put your main code here, to run repeatedly:
  int idmsg[1];

 
  Wire.beginTransmission(0x18);
  Wire.write(0x3D);
  Wire.endTransmission(false);
  Wire.requestFrom(0x18, 1);

  if (Wire.available())
  {
      uint8_t i2cByte=0;

      while(Wire.available())
      {
         idmsg[i2cByte] = Wire.read();
      }
  }

  Serial.println(idmsg[0], HEX);
  delay(1000);
}



//page 8 of datasheet talks about addresses 
//address written in hex 


//TODO: learn what data needs to be sent. Wire lib is very simple - just need data