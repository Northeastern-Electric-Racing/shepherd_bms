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


/**
 * @note Duty Cycle is variable corresponding to 1/255 increments 
 *       (0x00 corresponds to 0% duty cycle and 0xFF corresponds to 100% duty cycle)
 * @ref Refer to page 40 of datasheet
 */
#define AMC6821_DUTYCYCLE_REG               0x22


/**
 * @brief PWM I2C Characteristics Command Contents  (8 bits)
 * @ref Refer to page 41 of datasheet
 *                _______________________________________________________________________________________________________________
 * bit numbers:   |            7            |      6      |           5   4   3               |    2   1   0                    |
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


/******************************************************************************************************************/

void setup() {
  // put your setup code here, to run once:

  Serial.begin(9600);
  Wire.begin();
}

void loop() {
  // put your main code here, to run repeatedly:
  int idmsg[1];

 
  Wire.beginTransmission(AMC6821_I2C_ADDR);
  Wire.write(AMC6821_DEVID_REG);
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

  Serial.println(idmsg[0], HEX);
  delay(1000);
}



//page 8 of datasheet talks about addresses 
//address written in hex 


//TODO: learn what data needs to be sent. Wire lib is very simple - just need data