
#include "nerduino.h"

nerduino::nerduino()
{
    Wire.begin();
    Serial.begin(9600);
}

nerduino::~nerduino(){}

/**
 * @brief fills a buffer of data type XYZData_t with XYZ accelerometer data
 * @note size of buffer is determined by NUM_ADXL312_SAMPLES macro
 * 
 * @param xyzbuf
 */
void nerduino::getADXLdata(XYZData_t *xyzbuf)
{
    uint8_t *msg = new uint8_t[6];
    for(uint8_t i=0;i<NUM_ADXL312_SAMPLES;i++)
    {
        adxl312.getXYZ(msg);

        xyzbuf[i].XData.rawdata[0] = msg[0];
        xyzbuf[i].XData.rawdata[1] = msg[1];
        xyzbuf[i].YData.rawdata[0] = msg[2];
        xyzbuf[i].YData.rawdata[1] = msg[3];
        xyzbuf[i].ZData.rawdata[0] = msg[4];
        xyzbuf[i].ZData.rawdata[1] = msg[5];
        delay(5);
    }
}

