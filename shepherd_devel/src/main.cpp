#include "segment.h"
#include "nerduino.h"
#include "canMsgHandler.h"

void canHandler_CANMSG_BMSSHUTDOWN(const CAN_message_t &msg)
{
  Serial.println("Message Received!");
}

void setup() {
  nerduino nerduino;
  ADXL312 accelerometer;
}

void loop() {
  // put your main code here, to run repeatedly:


}