#include "canMsgHandler.h"

FlexCAN_T4<CAN1, RX_SIZE_256, TX_SIZE_16> myCan;

/**
 * @brief Initializes a CAN object for whichever line we are choosing
 * 
 * @param canLine   which CAN transceiver to use we want to use = NOT currently being used, we will probably need this eventually if we need to broadcast at different rates
 */
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

/**
 * @brief Sends CAN message
 * 
 * @param id 
 * @param len 
 * @param buf 
 * @return int 
 */
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

  //Need to figure out the contents of the command
    for(uint8_t i; i<NUM_CONFIGURABLECANMSG; i++)
    {
        EEPROM.write(i,msg.buf[i]);
    }
  //EEPROM.write(LED_pin_location, configmsg[0]);         //reconfigure EEPROM values
  //EEPROM.write(sensor_param_location, configmsg[1]);
    readEEPROMAddrs();

}

void readEEPROMAddrs()
{
    canmsgAddr.BMSSHUTDOWN = EEPROM.read(0);
    canmsgAddr.BMSDTCSTATUS = EEPROM.read(1);
    canmsgAddr.SET_INVERTER = EEPROM.read(2);
    canmsgAddr.SET_CARDIRECTION = EEPROM.read(3);
    canmsgAddr.SET_BRAKELIGHT = EEPROM.read(4);
    canmsgAddr.ERR_BRAKESWITCH = EEPROM.read(5);
    canmsgAddr.ERR_PEDALSENSOR = EEPROM.read(6);
    canmsgAddr.CARACCELERATION = EEPROM.read(7);
    canmsgAddr.BRAKEFLUIDPRESSURE = EEPROM.read(8);
    canmsgAddr.COOLINGFLOWRATE = EEPROM.read(9);
    canmsgAddr.GPSDATA = EEPROM.read(10);
    canmsgAddr.DIFFTEMP = EEPROM.read(11);
}

/**
 * @brief Processes all CAN messages
 * @note  ID filtering should happen beforehand, maybe add relevent ID's to each .cpp file?
 * 
 * @param msg 
 */
void incomingCANCallback(const CAN_message_t &msg)
{
    if(!SDWrite())
    {
        Serial.println("Error logging to SD Card!");
    }

    switch(msg.id)
    {
        case CANMSG_CONFIGUREADDR:
            canHandler_CANMSG_CONFIGUREADDR(msg);
            break;
        case CANMSG_BMSSHUTDOWN:
            canHandler_CANMSG_BMSSHUTDOWN(msg);
            break;
        case CANMSG_BMSDTCSTATUS            :
            canHandler_CANMSG_BMSDTCSTATUS(msg);
            break;
        case CANMSG_SET_INVERTER            :
            canHandler_CANMSG_SET_INVERTER(msg);
            break;
        case CANMSG_SET_CARDIRECTION        :
            canHandler_CANMSG_SET_CARDIRECTION(msg);
            break;
        case CANMSG_SET_BRAKELIGHT          :
            canHandler_CANMSG_SET_BRAKELIGHT(msg);
            break;
        case CANMSG_ERR_BRAKESWITCH         :
            canHandler_CANMSG_ERR_BRAKESWITCH(msg);
            break;
        case CANMSG_ERR_PEDALSENSOR         :
            canHandler_CANMSG_ERR_PEDALSENSOR(msg);
            break;
        case CANMSG_CARACCELERATION         :
            canHandler_CANMSG_CARACCELERATION(msg);
            break;
        case CANMSG_BRAKEFLUIDPRESSURE      :
            canHandler_CANMSG_BRAKEFLUIDPRESSURE(msg);
            break;
        case CANMSG_COOLINGFLOWRATE         :
            canHandler_CANMSG_COOLINGFLOWRATE(msg);
            break;
        case CANMSG_GPSDATA                 :
            canHandler_CANMSG_GPSDATA(msg);
            break;
        case CANMSG_DIFFTEMP                :
            canHandler_CANMSG_DIFFTEMP(msg);
            break;

        //Predefined CAN ID's
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
        case CANMSG_CHARGER_TO_BMS          :
            canHandler_CANMSG_CHARGER_TO_BMS(msg);
            break;
        case CANMSG_BMS_TO_CHARGER          :
            canHandler_CANMSG_BMS_TO_CHARGER(msg);
            break;
        default:
            Serial.println("CAN ID Invalid!");
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
__attribute__((weak)) void canHandler_CANMSG_CHARGER_TO_BMS       (const CAN_message_t &msg){return;}
__attribute__((weak)) void canHandler_CANMSG_BMS_TO_CHARGER       (const CAN_message_t &msg){return;}

//For SD logging in the TCU, isn't used anywhere else
__attribute__((weak)) bool SDWrite(){return true;}