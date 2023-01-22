#ifndef SEGMENT_H
#define SEGMENT_H

#include <LTC68041.h>
#include <nerduino.h>
#include "datastructs.h"
#include "bmsConfig.h"

#define THERM_WAIT_TIME     500 //ms
#define VOLTAGE_WAIT_TIME   250 //ms

#define THERM_AVG           15 // Number of values to average

/**
 * @brief This class serves as the interface for all of the segment boards
 */
class SegmentInterface
{
    private:
    
        Timer thermTimer;
        Timer voltageReadingTimer;

        FaultStatus_t voltageError = NOT_FAULTED;
        FaultStatus_t thermError = NOT_FAULTED;

        uint16_t thermSettleTime = 0;

        const uint32_t VOLT_TEMP_CONV[91] = 
        {
            44260, 43970, 43670, 43450, 43030, 42690, 42340, 41980, 41620, 41240,
            40890, 40460, 40040, 39580, 39130, 38660, 38210, 37710, 37210, 36190,
            35670, 35160, 34620, 34080, 33550, 32990, 32390, 31880, 31270, 30690, 
            30160, 29590, 28990, 28450, 27880, 27270, 26740, 26080, 25610, 25000,
            24440, 23880, 23330, 22780, 22240, 21700, 21180, 20660, 20150, 19640, 
            19140, 18650, 18170, 17700, 17230, 16780, 16330, 15890, 15470, 15030, 
            14640, 14230, 13850, 13450, 13070, 12710, 11490, 11170, 10850, 10550,
            10250, 9960, 9670, 9400, 9130, 8870, 8620, 8370, 8130, 7900, 
            0
        };

        const int32_t VOLT_TEMP_CALIB_OFFSET = 0;

        ChipData_t *segmentData = nullptr;

        ChipData_t previousData[NUM_CHIPS];

        uint8_t localConfig[NUM_CHIPS][6] = {};

        uint16_t dischargeCommands[NUM_CHIPS] = {};

        void pullChipConfigurations();

        void pushChipConfigurations();

        void SelectTherm(uint8_t therm);

        FaultStatus_t pullThermistors();

        FaultStatus_t pullVoltages();

        uint8_t steinhartEst(uint16_t V);

        void serializeI2CMsg(uint8_t dataToWrite[][3], uint8_t commOutput[][6]);

        void configureDischarge(uint8_t chip, uint16_t cells);

        void disableGPIOPulldowns();

    public:
        SegmentInterface();

        ~SegmentInterface();
        /**
         * @brief Initializes the segments
         * 
         */
        void init();

        /**
         * @brief Pulls all cell data from the segments and returns all cell data
         * 
         * @todo make sure that retrieving cell data doesn't block code too much
         * 
         * @return int*
         */
        void retrieveSegmentData(ChipData_t databuf[NUM_CHIPS]);

        /**
         * @brief Enables/disables balancing for all cells
         * 
         * @param balanceEnable
         */
        void enableBalancing(bool balanceEnable);

        /**
         * @brief Enables/disables balancing for a specific cell
         * 
         * @param chipNum 
         * @param cellNum 
         * @param balanceEnable 
         */
        void enableBalancing(uint8_t chipNum, uint8_t cellNum, bool balanceEnable);

        /**
         * @brief Sets each cell to whatever state is passed in the boolean config area
         * 
         * @param dischargeConfig 
         */
        void configureBalancing(bool dischargeConfig[NUM_CHIPS][NUM_CELLS_PER_CHIP]);

        /**
         * @brief Returns if a specific cell is balancing
         * 
         * @param chipNum 
         * @return true 
         * @return false 
         */
        bool isBalancing(uint8_t chipNum, uint8_t cellNum);

        /**
         * @brief Returns if any cells are balancing
         * 
         * @return true 
         * @return false 
         */
        bool isBalancing();
};

extern SegmentInterface segment;

#endif