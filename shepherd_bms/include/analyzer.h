#ifndef ANALYZER_H
#define ANALYZER_H

#include <nerduino.h>
#include "datastructs.h"
#include "segment.h"

//We want to make sure we aren't doing useless analysis on the same set of data since we are backfilling segment data
#define ANALYSIS_INTERVAL VOLTAGE_WAIT_TIME

#define MAX_SIZE_OF_HIST_QUEUE  300000U //bytes

using namespace std;

struct node
{
    AccumulatorData_t data;
    node* next = nullptr;
};

/**
 * @brief Resizable linked list queue for performing historical analysis
 * 
 */
class Analyzer
{
    private:
        node* head = nullptr;
        node* tail = nullptr;

        uint16_t size = 0;
        uint16_t numNodes = 0;

        Timer analysisTimer;

        /**
         * @brief Mapping Cell temperature to the cell resistance based on the 
         *      nominal cell resistance curve profile of the Samsung 186500 INR in the 
         *      Orion BMS software utility app
         * 
         * @note Units are in mOhms and indicies are in (degrees C)/5, stops at 65C
         * @note Resistance should be *interpolated* from these values (i.e. if we are
         *      at 27C, we should take the resistance that is halfway between 25C and 30C)
         */
        const float TEMP_TO_CELL_RES[14] = 
        {
            5.52, 4.84, 4.27, 3.68, 3.16, 2.74, 2.4,
            2.12, 1.98, 1.92, 1.90, 1.90, 1.90, 1.90
        };

        /**
         * @brief Mapping Cell temperatue to the discharge current limit based on the 
         *      temperature discharge limit curve profile of the Samsung 186500 INR
         *      in the Orion BMS software utility app
         * 
         * @note Units are in Amps and indicies are in (degrees C)/5, stops at 65C
         * @note Limit should be *interpolated* from these values (i.e. if we are
         *      at 27C, we should take the limit that is halfway between 25C and 30C)
         * 
         */
        const uint8_t TEMP_TO_DCL[14] =
        {
            110, 125, 140, 140, 140, 140, 
            140, 140, 140, 100, 60, 20, 0, 0
        };

        /**
         * @brief Mapping Cell temperatue to the charge current limit based on the 
         *      temperature charge limit curve profile of the Samsung 186500 INR
         *      in the Orion BMS software utility app
         * 
         * @note Units are in Amps and indicies are in (degrees C)/5, stops at 65C
         * @note Limit should be *interpolated* from these values (i.e. if we are
         *      at 27C, we should take the limit that is halfway between 25C and 30C)
         * 
         */
        const uint8_t TEMP_TO_CCL[14] =
        {
            0, 25, 25, 25, 25, 25, 25, 25,
            20, 15, 10, 5, 1, 1
        };

        /**
         * @brief Mapping the Relevant Thermistors for each cell based on cell #
         * 
         */
        const std::vector<int> RelevantThermMap[NUM_CELLS_PER_CHIP] = 
        {
            {17,18},
            {17,18},
            {17,18,19,20},
            {19,20},
            {19,20,21,22,23},
            {21,22,23,24,25},
            {24,25},
            {24,25,26,27},
            {26,27}
        };

        /**
         * @brief Mapping desired fan speed PWM to the cell temperature
         * 
         * @note Units are in PWM out of 255 and indicies are in (degrees C)/5, stops at 65C
         * @note Limit should be *interpolated* from these values (i.e. if we are
         *      at 27C, we should take the limit that is halfway between 25C and 30C)
         * 
         */
        const uint8_t FAN_CURVE[16] =
        {
            0, 0, 0, 0, 0, 0, 0, 0, 32, 64,
            128, 255, 255, 255, 255, 255
        };

        /**
         * @brief Selecting thermistors to ignore
         * 
         * @note True / 1 will disable the thermistor
         * 
         */
        const uint8_t THERM_DISABLE[4][11] =
        {
            {0,0,0,0,1,0,0,0,0,0,0},
            {1,0,0,0,0,0,0,0,0,0,0},
            {1,1,0,0,0,0,0,0,0,0,0},
            {1,1,0,0,0,0,0,0,0,0,0},
        };

        void enQueue(AccumulatorData_t data);

        void calcPackVoltageStats();

        void calcCellResistances();

        void calcCellTemps();

        void calcPackTemps();

        void calcDCL();

        void calcContDCL();

        void calcContCCL();

        void printData();

        void calcOpenCellVoltage();

        void disableTherms();

        void calcPackResistances(node bms_data);
    
    public:
        Analyzer();

        Analyzer(uint16_t newSize);

        ~Analyzer();

        /**
         * @brief Pushes in a new data point if we have waited long enough
         * 
         * @param data 
         */
        void push(AccumulatorData_t data);

        /**
         * @brief Resizes the number of data points
         * 
         * @param newSize 
         */
        void resize(uint16_t newSize);

        /**
         * @brief 
         * 
         * @param bmsdata 
         * @return uint8_t 
         */
        uint8_t calcFanPWM();

        /**
         * @brief Pointer to the address of the most recent data point
         * 
         */
        AccumulatorData_t *bmsdata = &(head->data);
};

extern Analyzer analyzer;

#endif