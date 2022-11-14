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


void configureDischarge(uint8_t chip, uint16_t cells) 
{
  // chipConfigurations[chip][4] == chipConfigurations[Literally what chip you want][register]
  // 4 and 5 are registers to discharge chips
  chipConfigurations[chip][4] = uint8_t(cells & 0x00FF);
  // Register 5 is split in half, so we maintain the upper half and add in the bottom half to discharge cells
  chipConfigurations[chip][5] = (chipConfigurations[chip][5] & 0xF0) + uint8_t(cells >> 8);
}


void SegmentInterface::enableBalancing(bool balanceEnable)
{
    // Variable Declarations
    int NUM_CELLS = 12;
    uint16_t rawCellVoltages[NUM_CHIPS][NUM_CELLS];
    float cellVoltages[NUM_CHIPS][NUM_CELLS];
    float minCellVal = 100;
    bool balancing;
    uint16_t dischargeCommand[NUM_CHIPS] = { 0 };
    if(balanceEnable)
    {
        LTC6804_adcv(); // Do before pulling cell voltages idk why
        LTC6804_rdcv(0, NUM_CHIPS, rawCellVoltages); // getting raw cell voltages
        pullChipConfigurations();
        // Converting raw cell voltages
        for(int chip = 0; chip < NUM_CHIPS; chip++) 
        {
            for(int cell = 0; cell < NUM_CELLS; cell++)
            {
                cellVoltages[chip][cell] = float(rawCellVoltages[chip][cell]) / 10000;
            }
        }
        for(int chipNum = 0; chipNum < NUM_CHIPS; chipNum++)
        {
            for(int cellNum = 0; cellNum < NUM_CELLS; cellNum++)
            {
                balancing = (cellVoltages[chipNum][cellNum] > minCellVal + MAX_DELTA_V) && cellVoltages[chipNum][cellNum] > BAL_MIN_V;
                if(balancing[chipNum][cellNum])
                {
                    dischargeCommand[chipNum] |= 1 << cellNum;
                }
            }
        }
        configureDischarge(chipNum, dischargeCommand[chipNum]); 
        pushChipConfigurations();
    }
    else
    {
        for(int chipNum = 0; chipNum < NUM_CHIPS; chipNum++)
        {
            for(int cellNum = 0; cellNum < NUM_CELLS; cellNum++)
            {
                dischargeCommand[chipNum] &= 0 << cellNum;
            }
        }
        configureDischarge(chipNum, discharge);
        pushChipConfigurations();
    }
}

void SegmentInterface::enableBalancing(uint8_t chipNum, uint8_t cellNum, bool balanceEnable)
{
    // Variable Declarations
    int NUM_CELLS = 12;
    uint16_t rawCellVoltages[NUM_CHIPS][NUM_CELLS];
    float cellVoltages[NUM_CHIPS][NUM_CELLS];
    float minCellVal = 100;
    bool balancing;
    uint16_t dischargeCommand = 0;
    if(balanceEnable)
    {
        LTC6804_adcv(); // Do before pulling cell voltages idk why
        LTC6804_rdcv(0, NUM_CHIPS, rawCellVoltages); // getting raw cell voltages
        pullChipConfigurations();
        // Converting raw cell voltages
        for(int chip = 0; chip < NUM_CHIPS; chip++) 
        {
            for(int cell = 0; cell < NUM_CELLS; cell++)
            {
                cellVoltages[chip][cell] = float(rawCellVoltages[chip][cell]) / 10000;
            }
        }

        balancing = (cellVoltages[chipNum][cellNum] > minCellVal + MAX_DELTA_V) && cellVoltages[chipNum][cellNum] > BAL_MIN_V;
        if(balancing[chipNum][cellNum])
        {
            dischargeCommand |= 1 << cellNum;
            configureDischarge(chipNum, discharge); 
            pushChipConfigurations();
        }
    }
    else
    {
        discharge &= 0 << cellNum;
        configureDischarge(chipNum, discharge);
        pushChipConfigurations();
    }

}

bool SegmentInterface::isBalancing(uint8_t chipNum, uint8_t cellNum)
{
    pullChipConfigurations();
    
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