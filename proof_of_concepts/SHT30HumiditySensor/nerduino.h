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

//SHT30-DIS Humidity Sensor
/**
 * https://www.mouser.com/datasheet/2/682/Sensirion_Humidity_Sensors_SHT3x_Datasheet_digital-971521.pdf  --Datasheet
 * 
 */

#define SHT30_I2C_ADDR                 0x44     //If ADDR (pin2) is connected to VDD, 0x45
#define SHT30_I2C_ADDR_WRITE_BIT       0x00     //This is the write bit to add to ADDR
#define SHT30_START_CMD_WCS            0x2C06   //Start measurement command with clock streching enabled and high repeatability
#define SHT30_START_CMD_NCS            0x2400   //Start measurement command with clock streching disabled and high repeatability 
#define SHT30_READSTATUS               0xF32D   /**< Read Out of Status Register */
#define SHT30_CLEARSTATUS              0x3041   /**< Clear Status */
#define SHT30_SOFTRESET                0x30A2   /**< Soft Reset */
#define SHT30_HEATEREN                 0x306D   /**< Heater Enable */
#define SHT30_HEATERDIS                0x3066   /**< Heater Disable */
#define SHT30_REG_HEATER_BIT           0x0d     /**< Status Register Heater Bit */
class SHT30
{
    private:
        void SHT30write(uint16_t *msg, uint8_t numBytes);    //Allows for private I2C writing with SHT30
        bool SHT30read(uint8_t *msg, uint8_t numBytes);     //Allows for private I2C reading with SHT30
    public:
        SHT30();                                            //Constructor for the SHT30 class
        ~SHT30();                                           //Destructor 
        uint16_t readStatusReg(void);                       //Reads the 16-bit status register
        void reset(void);                                   //Allows for a soft reset upon start up
        bool isHeaterEnabled();                             //Check if the heating element is turned on
        void heater(bool h);                                //Allows for heating element control, bool = true->enables, false->disable. Clears built up condensation
        bool getTempHumid(void);                            //Public function to start a measurement and print values
};

#endif