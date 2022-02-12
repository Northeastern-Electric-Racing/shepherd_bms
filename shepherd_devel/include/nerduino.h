#ifndef NERDUINO_H
#define NERDUINO_H

/*************************************************/
#include <Wire.h>


//Teensy Pinout
//CAN
#define CAN1_RX         23
#define CAN1_TX         22
#define CAN2_RX         30
#define CAN2_TX         31
//I2C
#define I2C_LOCAL_SDA   18
#define I2C_LOCAL_SCL   19
#define I2C1_SDA        17
#define I2C1_SCL        16
#define I2C2_SDA        25
#define I2C2_SCL        24
//SPI
#define SPI1_SCK        13
#define SPI1_CS         10
#define SPI1_MISO       11
#define SPI1_MOSI       12
#define SPI2_SCK        27
#define SPI2_CS         0
#define SPI2_MISO       26
#define SPI2_MOSI       1
//Analog IO
/* 14,15,21,20,38,39,40,41 */
//Digital Bitshift IO
/* 2,3,4,5,6,7 */
//Digital IO
/* 8,9,28,29,32,33,34,35,36,37 */

class nerduino
{
    /**
     * @todo: call all constructors of onboard chips during this intialization, maybe change hierarchy?
     */
};


//ADXL312 Accelerometer
/**
 * https://www.analog.com/media/en/technical-documentation/data-sheets/ADXL312.pdf  --Datasheet
 * https://wiki.analog.com/resources/quick-start/adxl312_quick_start_guide          --Used for implementation
 */
#define ADXL312_I2C_ADDR                0x53    //This depends on how the address pin is wired, 0x1D with Vcc and 0x53 with Gnd
#define ADXL312_POWER_CTRL_REG          0x2D
#define ADXL312_POWER_CTRL_MEASURECMD   0x08    //This tells the accelerometer to read the value, might need to change it the sleep mode if not in use

class ADXL312
{
    private:
        bool ADXL312write();
        bool ADXL312read();

    public:
        ADXL312();      //constructor
        ~ADXL312();     //destructor
        bool readXYZData();
        
};



//SHT30-DIS Humidity Sensor
/**
 * https://www.mouser.com/datasheet/2/682/Sensirion_Humidity_Sensors_SHT3x_Datasheet_digital-971521.pdf  --Datasheet
 * 
 */

class SHT30
{
    private:

    public:

};


//CAN Transceiver (Need to ask Matt McCauley about what chip we're using now)
/**
 * 
 * 
 */



//AMC6821 PWM Generator
/**
 * https://www.ti.com/lit/ds/symlink/amc6821.pdf?ts=1644706226375&ref_url=https%253A%252F%252Fwww.ti.com%252Fproduct%252FAMC6821%253Futm_source%253Dgoogle%2526utm_medium%253Dcpc%2526utm_campaign%253Dasc-sens-null-prodfolderdynamic-cpc-pf-google-wwe%2526utm_content%253Dprodfolddynamic%2526ds_k%253DDYNAMIC%2BSEARCH%2BADS%2526DCM%253Dyes%2526gclid%253DCj0KCQiA0p2QBhDvARIsAACSOOPKQVP7tfyxbaC8997ZjeHcQWZiSwAi1yblV-rFrJZ4BQS3xCwo1iYaAjmLEALw_wcB%2526gclsrc%253Daw.ds 
 * ^Datasheet
 */

class AMC6821
{
    private:

    public:

};



/*************************************************/

#endif