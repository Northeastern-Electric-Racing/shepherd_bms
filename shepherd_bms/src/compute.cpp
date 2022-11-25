#include "compute.h"

ComputeInterface::ComputeInterface(){}

ComputeInterface::~ComputeInterface(){}

FaultStatus_t ComputeInterface::enableCharging(bool enableCharging){

    isChargingEnabled = enableCharging ? true : false;
}


int ComputeInterface::sendChargingMessage(int voltageToSet, int currentToSet){

    uint8_t voltageByte2;
    uint8_t voltageByte3;
    uint8_t currentByte4;
    uint8_t currentByte5;

    if (isChargingEnabled){

        // equations taken from TSM2500 CAN protocol datasheet
        voltageByte3 = (10 * voltageToSet) / 256;
        voltageByte2 = (10 * voltageToSet) - (256 * voltageByte3);

        currentByte5 = (10 * (currentToSet + 3200)) / 256;
        currentByte4 = (10 * (currentToSet + 3200)) - (256 * currentByte5);

        startCharging.chargerData.chargerVoltageByte2 = voltageByte2;
        startCharging.chargerData.chargerVoltageByte3 = voltageByte3;
        startCharging.chargerData.chargerCurrentByte4 = currentByte4;
        startCharging.chargerData.chargerCurrentByte5 = currentByte5;

        startCharging.chargerData.chargerControl = 252; // 111111 - reserved + 00 = start charging

        sendMessage(0x18E54024, 8, startCharging.chargerMsg); // first param is charger control address

        return 0; // no fault return value 

        
    }


    else{

        return 1; // fault return value

    }

}

bool ComputeInterface::isCharging(){}

void ComputeInterface::setFanSpeed(uint8_t newFanSpeed){}

int16_t ComputeInterface::getPackCurrent(){}

