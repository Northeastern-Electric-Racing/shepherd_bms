#include "segment.h"

SegmentInterface::SegmentInterface(){}

SegmentInterface::~SegmentInterface(){}

void SegmentInterface::retrieveSegmentData(ChipData_t databuf[NUM_CHIPS])
{
    segmentData = databuf;

    pullVoltages();
    pullThermistors();

    segmentData = nullptr;
}

void SegmentInterface::enableBalancing(bool balanceEnable)
{

}

void SegmentInterface::enableBalancing(uint8_t chipNum, uint8_t cellNum, bool balanceEnable)
{

}

bool SegmentInterface::isBalancing(uint8_t chipNum, uint8_t cellNum)
{

}

bool SegmentInterface::isBalancing()
{
    
}

void SegmentInterface::pullChipConfigurations()
{
    uint8_t remoteConfig[NUM_CHIPS][8];
    LTC6804_rdcfg(NUM_CHIPS, remoteConfig);

    for (int chip = 0; chip < NUM_CHIPS; chip++)
    {
        for(int index = 0; index < 6; index++)
        {
            localConfig[chip][index] = remoteConfig[chip][index];
        }
    }
}

void SegmentInterface::pushChipConfigurations()
{
    LTC6804_wrcfg(NUM_CHIPS, localConfig);
}

FaultStatus_t SegmentInterface::pullVoltages(){

    uint16_t segmentVoltages[NUM_CHIPS][12];

    LTC6804_adcv();
    uint8_t errorStatus = LTC6804_rdcv(0, NUM_CHIPS, segmentVoltages);

    for (int i = 0; i < NUM_CHIPS; i++){
        for (int j = 0; j < 12; j++){
            segmentData[i].voltageReading[j] = segmentVoltages[i][j];
        }
    }
    
}

void SegmentInterface::serializeI2CMsg(uint8_t dataToWrite[][3], uint8_t commOutput[][6])
{
    for (int chip = 0; chip < NUM_CHIPS; chip++)
    {
        commOutput[chip][0] = 0x60 | (dataToWrite[chip][0] >> 4); // START + high side of B0
        commOutput[chip][1] = (dataToWrite[chip][0] << 4) | 0x00; // low side of B0 + ACK
        commOutput[chip][2] = 0x00 | (dataToWrite[chip][1] >> 4); // BLANK + high side of B1
        commOutput[chip][3] = (dataToWrite[chip][1] << 4) | 0x00; // low side of B1 + ACK
        commOutput[chip][4] = 0x00 | (dataToWrite[chip][2] >> 4); // BLANK + high side of B2
        commOutput[chip][5] = (dataToWrite[chip][2] << 4) | 0x09; // low side of B2 + STOP & NACK
    }
}