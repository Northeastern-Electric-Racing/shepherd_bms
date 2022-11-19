#include "segment.h"

SegmentInterface segment;

SegmentInterface::SegmentInterface(){}

SegmentInterface::~SegmentInterface(){}

void SegmentInterface::retrieveSegmentData(ChipData_t databuf[NUM_CHIPS])
{
    segmentData = databuf;

    /**
     * Pull voltages and thermistors and indiacte if there was a problem during retrieval
     */
    voltageError = pullVoltages();
    pullThermistors();

    /**
     * Save the contents of the reading so that we can use it to fill in missing data
     */
    memcpy(previousData, segmentData, sizeof(ChipData_t)*NUM_CHIPS);

    segmentData = nullptr;
}

void SegmentInterface::configureDischarge(uint8_t chip, uint16_t cells) 
{
  // chipConfigurations[chip][4] == chipConfigurations[Literally what chip you want][register]
  // 4 and 5 are registers to discharge chips
  localConfig[chip][4] = uint8_t(cells & 0x00FF);
  // Register 5 is split in half, so we maintain the upper half and add in the bottom half to discharge cells
  localConfig[chip][5] = (localConfig[chip][5] & 0xF0) + uint8_t(cells >> 8);
}

void SegmentInterface::enableBalancing(bool balanceEnable)
{
    //Discharging all cells in series
    static const uint16_t dischargeAllCommand = 0xFFFF >> (16-9); // Shifting 0xFFFF right by 16 minus however many cells in series

    pullChipConfigurations();

    if(balanceEnable)
    {
        for (int c = 0; c < NUM_CHIPS; c++)
        {
            configureDischarge(c, 0);
        }
        pushChipConfigurations();
    }
    else
    {
        for(int c = 0; c < NUM_CHIPS; c++)
        {
            for(int cellNum = 0; cellNum < 9; cellNum++)
            {
                configureDischarge(c, dischargeAllCommand);
            }
        }
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

FaultStatus_t SegmentInterface::pullVoltages()
{
    /**
     * If we haven't waited long enough between pulling voltage data
     * just copy over the contents of the last good reading and the fault status from the most recent attempt
     */
    if(!voltageReadingTimer.isTimerExpired())
    {
        for(uint8_t i=0; i<NUM_CHIPS; i++)
        {
            memcpy(segmentData[i].voltageReading, previousData[i].voltageReading, sizeof(segmentData[i].voltageReading));
        }
        return voltageError;
    }

    uint16_t segmentVoltages[NUM_CHIPS][12];

    LTC6804_adcv();

    /**
     * If we received an incorrect PEC indicating a bad read
     * copy over the data from the last good read and indicate an error
     */
    if(LTC6804_rdcv(0, NUM_CHIPS, segmentVoltages) == -1)
    {
        for(uint8_t i=0; i<NUM_CHIPS; i++)
        {
            memcpy(segmentData[i].voltageReading, previousData[i].voltageReading, sizeof(segmentData[i].voltageReading));
        }
        return FAULTED;
    }

    /**
     * If the read was successful, copy the voltage data
     */
    for (uint8_t i = 0; i < NUM_CHIPS; i++)
    {
        for (uint8_t j = 0; j < NUM_CELLS_PER_CHIP; j++)
        {
            segmentData[i].voltageReading[j] = segmentVoltages[i][j];
        }
    }
    
    /**
     * Start the timer between readings if successful
     */
    voltageReadingTimer.startTimer(VOLTAGE_WAIT_TIME);
    return NOT_FAULTED;
}

FaultStatus_t SegmentInterface::pullThermistors()
{

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