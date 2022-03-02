#include "segment.h"
#include "nerduino.h"
#include "canMsgHandler.h"

void canHandler_CANMSG_BMSSHUTDOWN(const CAN_message_t &msg)
{
  Serial.println("Message Received!");
}

ADXL312 accl;
nerduino nerd;
uint8_t *msg = new uint8_t[6];

void setup() {
  
}

void loop() {
  // put your main code here, to run repeatedly
}