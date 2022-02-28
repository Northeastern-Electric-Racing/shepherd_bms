#include "canMsgHandler.h"

FlexCAN_T4<CAN1, RX_SIZE_256, TX_SIZE_16> myCan;

/**
 * @brief Initializes a CAN object for whichever line we are choosing
 * 
 * @param canLine   which CAN transceiver to use we want to use 
 */
void initializeCAN(uint8_t canLine)
{

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

/**
 * @brief Processes all CAN messages
 * @note  ID filtering should happen beforehand, maybe add relevent ID's to each .cpp file?
 * 
 * @param msg 
 */
void incomingCANCallback(const CAN_message_t &msg)
{
    switch(msg.id)
    {
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
            canHandler_CANMSG_MOTORTEMP1(msg);          //Do all motor temp messages get handled the same way?
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
/***********************************************************/
