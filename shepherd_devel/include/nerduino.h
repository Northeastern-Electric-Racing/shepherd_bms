/**
 * @file nerduino.h
 * @author Nick DePatie
 * @brief 
 * @date 2022-03-07
 */
#ifndef NERDUINO_H
#define NERDUINO_H

/*************************************************/
#include <Wire.h>
#include <EEPROM.h>
#include <Arduino.h>
#include "canMsgHandler.h"
#include "adxl312.h"
#include "sht30.h"
#include "amc6821.h"

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

/**********************************************************************************/
//Message Structures
#define NUM_ADXL312_SAMPLES 16
/**
 * @brief This is the message type for retrieving XYZ data as a buffer
 * 
 */
typedef struct
{
    union XData_t
    {
        uint8_t rawdata[2];
        int16_t data;
    }XData;

    union YData_t
    {
        uint8_t rawdata[2];
        int16_t data;
    }YData;

    union ZData_t
    {
        uint8_t rawdata[2];
        int16_t data;
    }ZData;
    
}XYZData_t;

/*********************************************/

class nerduino
{
    private:
        ADXL312 adxl312;
        AMC6821 amc6821;
        SHT30 sht30;

    public:
        nerduino();
        ~nerduino();

        /**
         * @brief fills a buffer of data type XYZData_t with XYZ accelerometer data
         * @note size of buffer is determined by NUM_ADXL312_SAMPLES macro
         * 
         * @param xyzbuf 
         */
        void getADXLdata(XYZData_t *xyzbuf);
};

/*************************************************/

#endif