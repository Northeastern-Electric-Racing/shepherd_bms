#include "canMsgHandler.h"

FlexCAN_T4<CAN1, RX_SIZE_256, TX_SIZE_16> myCan;

void initializeCAN(uint8_t canLine)
{
    myCan.begin(); // needed to initialize the CAN object (must be first method called)
    myCan.setBaudRate(BAUD_RATE); // sets baud rate

    myCan.setMaxMB(MAX_MB_NUM);
    myCan.enableFIFO(); // enables the FIFO operation mode, where all received messages are received and accessed via a queue
    myCan.enableFIFOInterrupt(); // enables interrupts to be used with FIFO
    myCan.onReceive(incomingCANCallback); // sets the callback for received messages
    myCan.mailboxStatus(); // prints out mailbox config information
}


int sendMessage(uint32_t id, uint8_t len, const uint8_t *buf)
{
CAN_message_t msg;
msg.id = id;
msg.len = len;
uint8_t *buf1;

    for (int i = 0; i < 8; i++) {
        if (i < len)
        {
            buf1 = const_cast<uint8_t*>(buf + i);
            msg.buf[i] = *buf1;
        }
        else
        {
            msg.buf[i] = 0; // copies buf to message, padding with 0s if length isn't 8
        }
    }

    return myCan.write(msg);
}

void canHandler_CANMSG_CONFIGUREADDR(const CAN_message_t &msg){
    EEPROM.write(msg.buf[0],msg.buf[1]);
    readEEPROMAddrs();
}

void readEEPROMAddrs()
{
    canIDs.BMSSHUTDOWN.rawEEPROM[0] = EEPROM.read(BMSSHUTDOWN);
    canIDs.BMSSHUTDOWN.rawEEPROM[1] = EEPROM.read(BMSSHUTDOWN+1);

    canIDs.BMSDTCSTATUS.rawEEPROM[0] = EEPROM.read(BMSDTCSTATUS);
    canIDs.BMSDTCSTATUS.rawEEPROM[1] = EEPROM.read(BMSDTCSTATUS+1);

    canIDs.SET_INVERTER.rawEEPROM[0] = EEPROM.read(SET_INVERTER);
    canIDs.SET_INVERTER.rawEEPROM[1] = EEPROM.read(SET_INVERTER+1);

    canIDs.SET_CARDIRECTION.rawEEPROM[0] = EEPROM.read(SET_CARDIRECTION);
    canIDs.SET_CARDIRECTION.rawEEPROM[1] = EEPROM.read(SET_CARDIRECTION+1);

    canIDs.SET_BRAKELIGHT.rawEEPROM[0] = EEPROM.read(SET_BRAKELIGHT);
    canIDs.SET_BRAKELIGHT.rawEEPROM[1] = EEPROM.read(SET_BRAKELIGHT+1);

    canIDs.ERR_BRAKESWITCH.rawEEPROM[0] = EEPROM.read(ERR_BRAKESWITCH);
    canIDs.ERR_BRAKESWITCH.rawEEPROM[1] = EEPROM.read(ERR_BRAKESWITCH+1);

    canIDs.ERR_PEDALSENSOR.rawEEPROM[0] = EEPROM.read(ERR_PEDALSENSOR);
    canIDs.ERR_PEDALSENSOR.rawEEPROM[1] = EEPROM.read(ERR_PEDALSENSOR+1);

    canIDs.CARACCELERATION.rawEEPROM[0] = EEPROM.read(CARACCELERATION);
    canIDs.CARACCELERATION.rawEEPROM[1] = EEPROM.read(CARACCELERATION+1);

    canIDs.BRAKEFLUIDPRESSURE.rawEEPROM[0] = EEPROM.read(BRAKEFLUIDPRESSURE);
    canIDs.BRAKEFLUIDPRESSURE.rawEEPROM[1] = EEPROM.read(BRAKEFLUIDPRESSURE+1);

    canIDs.COOLINGFLOWRATE.rawEEPROM[0] = EEPROM.read(COOLINGFLOWRATE);
    canIDs.COOLINGFLOWRATE.rawEEPROM[1] = EEPROM.read(COOLINGFLOWRATE+1);

    canIDs.GPSDATA.rawEEPROM[0] = EEPROM.read(GPSDATA);
    canIDs.GPSDATA.rawEEPROM[1] = EEPROM.read(GPSDATA+1);
    
    canIDs.DIFFTEMP.rawEEPROM[0] = EEPROM.read(DIFFTEMP);
    canIDs.DIFFTEMP.rawEEPROM[1] = EEPROM.read(DIFFTEMP+1);
}


void incomingCANCallback(const CAN_message_t &msg)
{
    if(!SDWrite())
    {
        Serial.println("Error logging to SD Card!");
    }

    /**
     * Using a switch statement to rule out set CAN IDs first, and then using if
     * elses within the default case because the switch statement is more efficient
     */
    switch(msg.id)
    {                                                   //using a switch statement for set CAN IDs
        case CANMSG_CONFIGUREADDR           :
            canHandler_CANMSG_CONFIGUREADDR(msg);
            break;
        case CANMSG_ACCELERATIONCTRLINFO    :
            canHandler_CANMSG_ACCELERATIONCTRLINFO(msg);
            break;
        case CANMSG_MOTORTEMP1              :
            canHandler_CANMSG_MOTORTEMP1(msg);          //Do all motor temp messages get handled the same way? if so we can make all them go to the same case
            break;
        case CANMSG_MOTORTEMP2              :
            canHandler_CANMSG_MOTORTEMP2(msg);
            break;
        case CANMSG_MOTORETEMP3             :
            canHandler_CANMSG_MOTORETEMP3(msg);
            break;
        case CANMSG_MOTORMOTION             :
            canHandler_CANMSG_MOTORMOTION(msg);
            break;
        case CANMSG_MOTORCURRENT            :
            canHandler_CANMSG_MOTORCURRENT(msg);
            break;
        case CANMSG_MOTORVOLTAGE            :
            canHandler_CANMSG_MOTORVOLTAGE(msg);
            break;
        case CANMSG_MCVEHICLESTATE          :
            canHandler_CANMSG_MCVEHICLESTATE(msg);
            break;
        case CANMSG_ERR_MCFAULT             :
            canHandler_CANMSG_ERR_MCFAULT(msg);
            break;
        case CANMSG_MOTORTORQUETIMER        :
            canHandler_CANMSG_MOTORTORQUETIMER(msg);
            break;
        case CANMSG_BMSSTATUS2              :
            canHandler_CANMSG_BMSSTATUS2(msg);
            break;
        case CANMSG_BMSCHARGEDISCHARGE      :
            canHandler_CANMSG_BMSCHARGEDISCHARGE(msg);
            break;
        case CANMSG_MC_BMS_INTEGRATION      :
            canHandler_CANMSG_MC_BMS_INTEGRATION(msg);
            break;
        default:                                        //using if statements for configurable CAN IDs
            if (msg.id == canIDs.GPSDATA.canID)
            {
                canHandler_CANMSG_GPSDATA(msg);
            }
            else if (msg.id == canIDs.CARACCELERATION.canID)
            {
                canHandler_CANMSG_CARACCELERATION(msg);
            }
            else if (msg.id == canIDs.BRAKEFLUIDPRESSURE.canID)
            {
                canHandler_CANMSG_BRAKEFLUIDPRESSURE(msg);
            }
            else if (msg.id == canIDs.BMSSHUTDOWN.canID)
            {
                canHandler_CANMSG_BMSSHUTDOWN(msg);
            }
            else if (msg.id == canIDs.SET_INVERTER.canID)
            {
                canHandler_CANMSG_SET_INVERTER(msg);
            }
            else if (msg.id == canIDs.COOLINGFLOWRATE.canID)
            {
                canHandler_CANMSG_COOLINGFLOWRATE(msg);
            }
            else if (msg.id == canIDs.BMSDTCSTATUS.canID)
            {
                canHandler_CANMSG_BMSDTCSTATUS(msg);
            }
            else if (msg.id == canIDs.SET_CARDIRECTION.canID)
            {
                canHandler_CANMSG_SET_CARDIRECTION(msg);
            }
            else if (msg.id == canIDs.SET_BRAKELIGHT.canID)
            {
                canHandler_CANMSG_SET_BRAKELIGHT(msg);
            }
            else if (msg.id == canIDs.ERR_BRAKESWITCH.canID)
            {
                canHandler_CANMSG_ERR_BRAKESWITCH(msg);
            }
            else if (msg.id == canIDs.ERR_PEDALSENSOR.canID)
            {
                canHandler_CANMSG_ERR_PEDALSENSOR(msg);
            }
            else if (msg.id == canIDs.DIFFTEMP.canID)
            {
                canHandler_CANMSG_DIFFTEMP(msg);
            }
            else
            {
                Serial.println("CAN ID Invalid!");
            }
            break;
    }
}

/**************************************************************/
/**
 * @brief CAN Message Handle Commands
 * __attribute__((weak)) indicates that if the compiler doesn't find any other function with this same name, then it will default to these which just do nothing
 * This means we can use a master CAN processing command amongst all devices
 * 
 */

__attribute__((weak)) void canHandler_CANMSG_BMSSHUTDOWN          (const CAN_message_t &msg){return;}
__attribute__((weak)) void canHandler_CANMSG_BMSDTCSTATUS         (const CAN_message_t &msg){return;}
__attribute__((weak)) void canHandler_CANMSG_SET_INVERTER         (const CAN_message_t &msg){return;}
__attribute__((weak)) void canHandler_CANMSG_SET_CARDIRECTION     (const CAN_message_t &msg){return;}
__attribute__((weak)) void canHandler_CANMSG_SET_BRAKELIGHT       (const CAN_message_t &msg){return;}
__attribute__((weak)) void canHandler_CANMSG_ERR_BRAKESWITCH      (const CAN_message_t &msg){return;}
__attribute__((weak)) void canHandler_CANMSG_ERR_PEDALSENSOR      (const CAN_message_t &msg){return;}
__attribute__((weak)) void canHandler_CANMSG_CARACCELERATION      (const CAN_message_t &msg){return;}
__attribute__((weak)) void canHandler_CANMSG_BRAKEFLUIDPRESSURE   (const CAN_message_t &msg){return;}
__attribute__((weak)) void canHandler_CANMSG_COOLINGFLOWRATE      (const CAN_message_t &msg){return;}
__attribute__((weak)) void canHandler_CANMSG_GPSDATA              (const CAN_message_t &msg){return;}
__attribute__((weak)) void canHandler_CANMSG_DIFFTEMP             (const CAN_message_t &msg){return;}

//Predefined CAN Messages
__attribute__((weak)) void canHandler_CANMSG_ACCELERATIONCTRLINFO (const CAN_message_t &msg){return;}
__attribute__((weak)) void canHandler_CANMSG_MOTORTEMP1           (const CAN_message_t &msg){return;}
__attribute__((weak)) void canHandler_CANMSG_MOTORTEMP2           (const CAN_message_t &msg){return;}
__attribute__((weak)) void canHandler_CANMSG_MOTORETEMP3          (const CAN_message_t &msg){return;}
__attribute__((weak)) void canHandler_CANMSG_MOTORMOTION          (const CAN_message_t &msg){return;}
__attribute__((weak)) void canHandler_CANMSG_MOTORCURRENT         (const CAN_message_t &msg){return;}
__attribute__((weak)) void canHandler_CANMSG_MOTORVOLTAGE         (const CAN_message_t &msg){return;}
__attribute__((weak)) void canHandler_CANMSG_MCVEHICLESTATE       (const CAN_message_t &msg){return;}
__attribute__((weak)) void canHandler_CANMSG_ERR_MCFAULT          (const CAN_message_t &msg){return;}
__attribute__((weak)) void canHandler_CANMSG_MOTORTORQUETIMER     (const CAN_message_t &msg){return;}
__attribute__((weak)) void canHandler_CANMSG_BMSSTATUS2           (const CAN_message_t &msg){return;}
__attribute__((weak)) void canHandler_CANMSG_BMSCHARGEDISCHARGE   (const CAN_message_t &msg){return;}
__attribute__((weak)) void canHandler_CANMSG_MC_BMS_INTEGRATION   (const CAN_message_t &msg){return;}

//For SD logging in the TCU, isn't used anywhere else
__attribute__((weak)) bool SDWrite(){return true;}

