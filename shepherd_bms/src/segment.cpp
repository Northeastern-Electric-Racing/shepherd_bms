#include "segment.h"

SegmentInterface::SegmentInterface(){}

SegmentInterface::~SegmentInterface(){}

int* SegmentInterface::retrieveSegmentData()
{
    pullVoltages();
    pullThermistors();
    return segmentData;
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

void SegmentInterface::writeChipConfigurations()
{
    LTC6804_wrcfg(NUM_CHIPS, localConfig);
}