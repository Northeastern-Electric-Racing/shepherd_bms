#ifndef NERDUINO_H
#define NERDUINO_H

/*************************************************/

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


//ADXL312 Accelerometer
/**
 * https://www.analog.com/media/en/technical-documentation/data-sheets/ADXL312.pdf
 */
#define ADXL312_I2C_ADDR                0x53    //This depends on how the address pin is wired, 0x1D with Vcc and 0x53 with Gnd
#define ADXL312_POWER_CTRL_REG          0x2D
#define ADXL312_POWER_CTRL_MEASURECMD   0x


class ADXL312
{
    private:

    public:

};

/*************************************************/

#endif