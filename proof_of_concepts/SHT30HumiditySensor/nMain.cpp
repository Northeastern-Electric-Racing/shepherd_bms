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
        nerduino()
        {
        Wire.begin();
        Serial.begin(9600);
        }
        ~nerduino(){}
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
        void SHT30write(uint8_t *msg, uint8_t numBytes)    //Allows for private I2C writing with SHT30
        {
            Wire.beginTransmission(SHT30_I2C_ADDR+SHT30_I2C_ADDR_WRITE_BIT);
            for(uint8_t i=0; i< numBytes; i++)
            {
                Wire.write(msg[i]);
            }
            Wire.endTransmission(false);
        }
        bool SHT30read(uint8_t *msg, uint8_t numBytes)     //Allows for private I2C reading with SHT30
        {
        Wire.requestFrom(SHT30_I2C_ADDR, numBytes);
        if(Wire.available())
        {
            uint8_t dataBytes=0;
            while(Wire.available())
            {
                msg[dataBytes] = Wire.read();
                dataBytes++;
            }
            return true;
            }
        else{
            Serial.println("Reading Failed");
            return false;
            }
        }
    public:
        SHT30()                                            //Constructor for the SHT30 class
            {
                reset();
                Serial.println("SHT30 is reset.");
                Serial.println("Heating element is enabled: ");
                isHeaterEnabled();
            }
        ~SHT30()                                           //Destructor 
        {}
        uint16_t readStatusReg()                       //Reads the 16-bit status register
            {
                uint8_t msg[2];
                msg[0] = SHT30_READSTATUS>>8;
                msg[1] = SHT30_READSTATUS & 0x00FF;
                SHT30write(msg, 2);
                uint8_t data[3];
                SHT30read(data, 3);
                uint16_t status = data[0];
                status <<= 8;
                status |= data[1];
                return status;
            }
        void reset(void)                                    //Allows for a soft reset upon start up
            {
                uint8_t msg[2];
                msg[0] = SHT30_SOFTRESET>>8;
                msg[1] = SHT30_SOFTRESET & 0x00FF;
                SHT30write(msg, 2);
                Serial.println("Successful reset");
            }
        bool isHeaterEnabled()                             //Check if the heating element is turned on
            {
            uint16_t regVal = readStatusReg();
            return (bool) bitRead(regVal, SHT30_REG_HEATER_BIT);
            }
        void heater(bool h)                                 //Allows for heating element control, bool = true->enables, false->disable. Clears built up condensation
            {
                    uint8_t msg1[2];
                    msg1[0] = SHT30_HEATEREN>>8;
                    msg1[1] = SHT30_HEATEREN & 0x00FF;
                    uint8_t msg2[2];
                    msg2[0] = SHT30_HEATERDIS>>8;
                    msg2[1] = SHT30_HEATERDIS & 0x00FF;
                if (h = true)
                    SHT30write(msg1, 2);
                else
                    SHT30write(msg2, 2);    
            }   
        bool getTempHumid(void)                             //Public function to start a measurement and print values
            {
                uint8_t cmd[2];
                cmd[0] = SHT30_START_CMD_WCS >> 8; 
                cmd[1] = SHT30_START_CMD_WCS & 0xFF;
                uint8_t msg[6];
                SHT30write(cmd, 2);
                SHT30read(msg, 6);
                float cTemp = ((((msg[0] * 256.0) + msg[1]) * 175) / 65535.0) - 45;
                float fTemp = (cTemp * 1.8) + 32;
                float humidity = ((((msg[3] * 256.0) + msg[4]) * 100) / 65535.0);
                Serial.print("Temperature C: ");
                Serial.print(cTemp);
                Serial.println(" C");
                Serial.print("Temperature F: ");
                Serial.print(fTemp);
                Serial.println(" F");
                Serial.print("Relative Humidity: ");
                Serial.print(humidity);
                Serial.println(" %RH");
                return true;
            }
};

void setup(){

}
nerduino nerd;
SHT30 tempHumidSensor;

void loop()
{
tempHumidSensor.getTempHumid();
delay (500);
}