#include "nerduino.h"
#include "canMsgHandler.h"

void canHandler_CANMSG_BMSSHUTDOWN(const CAN_message_t &msg)
{
  Serial.println("Message Received!");
}

nerduino nerd;

void setup() {
  
}

void loop()
{
  XYZData_t xyzbuf[16];
  nerd.getADXLdata(xyzbuf);

  for(uint8_t i=0; i<NUM_ADXL312_SAMPLES; i++)
  {
    Serial.print(xyzbuf[i].XData.data);
    Serial.print("\t");
    Serial.print(xyzbuf[i].YData.data);
    Serial.print("\t");
    Serial.print(xyzbuf[i].ZData.data);
    Serial.print("\t");
    Serial.println();
  }
 
  Serial.println("cycle...");

  delay(500);
}