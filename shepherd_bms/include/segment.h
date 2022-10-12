#ifndef SEGMENT_H
#define SEGMENT_H

#include <LTC68041.h>
#include <nerduino.h>
#include "datastructs.h"

#define NUM_SEGMENTS    4
#define NUM_CHIPS       NUM_SEGMENTS*2

#define MIN_VOLT        2.9
#define MAX_VOLT        4.2
#define MAX_DELTA_V     0.02
#define BAL_MIN_V       4.00


/**
 * @brief This class serves as the interface for all of the segment boards
 */
class SegmentInterface
{
    private:

        const uint32_t VOLT_TEMP_CONV[57] = 
        {
            45140, 44890, 44640, 44370, 44060, 43800, 43510, 43210, 42900, 42560,
            42240, 41900, 41550, 41170, 40820, 40450, 40040, 39650, 39220, 38810, 
            38370, 37950, 37500, 37090, 36650, 36180, 35670, 35220, 34740, 34230,
            33770, 33270, 32770, 32280, 31770, 31260, 30750, 30240, 29720, 29220, 
            28710, 28200, 27680, 27160, 26660, 26140, 25650, 25130, 24650, 24150, 
            23660, 23170, 22670, 22190, 21720, 21240, 0
        };

        uint32_t thermisterTemp[NUM_CHIPS][32];

        int segmentData[1] = {0}; //placeholder for internal segment data type

        uint8_t localConfig[NUM_CHIPS][6] = {};

        uint8_t chipConfigurations[NUM_CHIPS][6];

        void pullChipConfigurations();

        void pushChipConfigurations();

        void ConfigureCOMMRegisters(uint8_t numChips, uint8_t dataToWrite[][3], uint8_t commOutput [][6]);

        void SelectTherm(uint8_t therm);

        void pullThermistors();

        void pullVoltages();

        uint8_t steinhartEst();

        void serializeI2CMsg(uint8_t dataToWrite[][3], uint8_t commOutput[][6]);

        void configureDischarge(uint8_t chip, uint16_t cells);

    public:
        SegmentInterface();

        ~SegmentInterface();

        /**
         * @brief Pulls all cell data from the segments and returns all cell data
         * 
         * @todo make sure that retrieving cell data doesn't block code too much
         * 
         * @return int*
         */
        int* retrieveSegmentData(); //@todo int* is a placeholder for an actual data type for segment data

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

#endif