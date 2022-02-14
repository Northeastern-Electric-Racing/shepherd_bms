#ifndef NERDUINO_H
#define NERDUINO_H

/*************************************************/
#include <Wire.h>
#include <Arduino.h>

//Teensy Pinout
#define RELAY_PIN       36          //implementation of RELAY_PIN is as simple as digitalWrite(RELAY_PIN,HIGH or LOW);
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
/* 8,9,28,29,32,33,34,35,37 */

class nerduino
{
    private:

    public:
        nerduino();
        ~nerduino();
    
    /**
     * @todo: call all constructors of onboard chips during this intialization, maybe change hierarchy?
     *
     */
};


//ADXL312 Accelerometer
/**
 * https://www.analog.com/media/en/technical-documentation/data-sheets/ADXL312.pdf  --Datasheet
 * https://wiki.analog.com/resources/quick-start/adxl312_quick_start_guide          --Used for implementation
 */
#define ADXL312_I2C_ADDR                0x53    //This depends on how the address pin is wired, 0x1D with Vcc and 0x53 with Gnd
#define ADXL312_DEVID_REG               0x00
#define ADXL312_DEVID                   0xE5
#define ADXL312_POWER_CTRL_REG          0x2D
#define ADXL312_POWER_CTRL_MEASURECMD   0x08    //This tells the accelerometer to read the value, might need to change it the sleep mode if not in use
#define ADXL312_XYZDATA_REG_OFFSET      0x32    //The XYZ data registers are 0x32-0x37

class ADXL312
{
    private:
        void ADXL312write(uint8_t *msg, uint8_t numBytes);
        bool ADXL312read(uint8_t *msg,uint8_t numBytes);

    public:
        ADXL312();      //constructor
        ~ADXL312();     //destructor
        bool verifyFunctionality();
        void configureForMeasurement();
        uint8_t* getXYZ();
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


//SN65HVD256D CAN Transceiver (Might want to move to a few seperate files to create a master queue using __attribute__((weak)) )
/**
 * https://www.ti.com/lit/ds/symlink/sn65hvd255.pdf?HQS=dis-dk-null-digikeymode-dsf-pf-null-wwe&ts=1644803460205&ref_url=https%253A%252F%252Fwww.ti.com%252Fgeneral%252Fdocs%252Fsuppproductinfo.tsp%253FdistId%253D10%2526gotoUrl%253Dhttps%253A%252F%252Fwww.ti.com%252Flit%252Fgpn%252Fsn65hvd255
 * ^Datasheet
 * 
 * https://github.com/Northeastern-Electric-Racing/NER/blob/master/Embedded%20Software/CAN%20Latency/TeensyCanTest/src/main.cpp  --Implementation of CAN on Teensy
 */

class SN65HVD
{
    private:

    public:
    /**
     * @todo create a master CAN process to process CAN queue and intialize using desired parameters, this process will need to be a bit more complex than
     *       just the chip implementations, so a knowledge of the CAN network and systems will be necessary
     */
};


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