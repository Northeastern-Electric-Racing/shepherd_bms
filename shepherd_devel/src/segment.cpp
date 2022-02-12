#include "segment.h"
/** 
 * @todo Literally everything lmao
 */

/**
 * @brief Serializes the data to be written to the LTC6280 register
 * 
 * @param numChips 
 * @param data 
 * @param commOutput 
 * @param operation 
 */
void segment::msgSerialization(uint8_t numChips, uint8_t data[][3], uint8_t commData [][6], msg_Ser_Operation_t operation)
{
  if(operation == MSG_OPERATION_SERIALIZE)
  {
    for (int chip = 0; chip < numChips; chip++)
    {
      commData[chip][0] = 0x60 | (data[chip][0] >> 4);
      commData[chip][1] = (data[chip][0] << 4) | 0x08;
      commData[chip][2] = 0x00 | (data[chip][1] >> 4);
      commData[chip][3] = (data[chip][1] << 4) | 0x09;
      commData[chip][4] = 0x70 | (data[chip][2] >> 4);
      commData[chip][5] = (data[chip][2] << 4) | 0x09;
    }
  }
  if(operation == MSG_OPERATION_DESERIALIZE)
  {
    for (int chip = 0; chip < numChips; chip++)
    {
      data[chip][0] = ((commData[chip][0] << 4) & 0xF0) | ((commData[chip][1] >> 4) & 0x0F);
      data[chip][1] = ((commData[chip][2] << 4) & 0xF0) | ((commData[chip][1] >> 3) & 0x0F);
      data[chip][2] = ((commData[chip][4] << 4) & 0xF0) | ((commData[chip][1] >> 5) & 0x0F);
    }
  }
}