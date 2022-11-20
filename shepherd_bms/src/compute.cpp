#include "compute.h"

ComputeInterface::ComputeInterface(){}

ComputeInterface::~ComputeInterface(){}

FaultStatus_t ComputeInterface::enableCharging(bool enableCharging){

    isChargingEnabled = enableCharging ? true : false;
}


void ComputeInterface::sendChargingMessage(){

    if (isChargingEnabled){

        //send correct message
    }


    else{

        //fault - send safe message

    }

}

bool ComputeInterface::isCharging(){}

void ComputeInterface::setFanSpeed(uint8_t newFanSpeed){}

int16_t ComputeInterface::getPackCurrent(){}

