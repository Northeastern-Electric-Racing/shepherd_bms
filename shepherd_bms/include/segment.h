#ifndef SEGMENT_H
#define SEGMENT_H

#include <LTC68041.h>
#include <nerduino.h>
#include "datastructs.h"
#include "bmsConfig.h"

#define THERM_WAIT_TIME     500 //ms
<<<<<<< HEAD
#define VOLTAGE_WAIT_TIME   250 //ms
=======
#define VOLTAGE_WAIT_TIME   100 //ms
>>>>>>> main
#define THERM_AVG           15 // Number of values to average

#define MAX_VOLT_DELTA      2500
#define MAX_VOLT_DELTA_COUNT    10

/**
 * @brief This class serves as the interface for all of the segment boards
 */
class SegmentInterface
{
    private:

        Timer therm_timer;
        Timer voltage_reading_timer;
<<<<<<< HEAD
=======
        Timer variance_timer;
>>>>>>> main

        FaultStatus_t voltage_error = NOT_FAULTED;
        FaultStatus_t therm_error = NOT_FAULTED;

        uint16_t therm_settle_time_ = 0;

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

        ChipData_t *segment_data = nullptr;

        ChipData_t previous_data[NUM_CHIPS];

        uint8_t local_config[NUM_CHIPS][6] = {};

        uint16_t discharge_commands[NUM_CHIPS] = {};

        void pullChipConfigurations();

        void pushChipConfigurations();

        void SelectTherm(uint8_t therm);

        FaultStatus_t pullThermistors();

        FaultStatus_t pullVoltages();

        int8_t steinhartEst(uint16_t V);

        void serializeI2CMsg(uint8_t data_to_write[][3], uint8_t comm_output[][6]);

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
         * @param balance_enable
         */
        void enableBalancing(bool balance_enable);

        /**
         * @brief Enables/disables balancing for a specific cell
         *
         * @param chip_num
         * @param cell_num
         * @param balance_enable
         */
        void enableBalancing(uint8_t chip_num, uint8_t cell_num, bool balance_enable);

        /**
         * @brief Sets each cell to whatever state is passed in the boolean config area
         *
         * @param discharge_config
         */
        void configureBalancing(bool discharge_config[NUM_CHIPS][NUM_CELLS_PER_CHIP]);

        /**
         * @brief Returns if a specific cell is balancing
         *
         * @param chip_num
         * @return true
         * @return false
         */
        bool isBalancing(uint8_t chip_num, uint8_t cell_num);

        /**
         * @brief Returns if any cells are balancing
         *
         * @return true
         * @return false
         */
        bool isBalancing();

        void averagingThermCheck();

        void standardDevThermCheck();

        int8_t calcThermStandardDev(int16_t avg_temp);

        int16_t calcAverage();

        void varianceThermCheck();

        void discardNeutrals();

};

extern SegmentInterface segment;

#endif