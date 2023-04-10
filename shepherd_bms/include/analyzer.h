#ifndef ANALYZER_H
#define ANALYZER_H

#include <nerduino.h>
#include "datastructs.h"
#include "segment.h"

//We want to make sure we aren't doing useless analysis on the same set of data since we are backfilling segment data
#define ANALYSIS_INTERVAL VOLTAGE_WAIT_TIME

//#define MAX_SIZE_OF_HIST_QUEUE  300000U //bytes

/**
 * @brief Resizable linked list queue for performing historical analysis
 *
 */
class Analyzer
{
    private:

        Timer analysisTimer;

        bool is_first_reading_ = true;

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
         * @brief Lookup table for State of Charge
         *
         * @note each index covers 0.1V increase (voltage range is 2.9V - 4.2V, deltaV = 1.3V, currently 13 data points)
         * @note values are unitless percentages that represent % charge
         *
         */
        const uint8_t STATE_OF_CHARGE_CURVE[13] =
        {
            0, 8, 16, 24, 32, 40, 48, 56, 64, 72, 80, 88, 100
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
        const uint8_t THERM_DISABLE[8][11] =
        {
            {0,0,0,0,0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0,0,0,0,0}
        };

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

        void calcStateOfCharge();

        void highCurrThermCheck();

        void diffCurrThermCheck();

        void varianceThermCheck();

        uint8_t calcThermStandardDev();

        void standardDevThermCheck();

    public:
        Analyzer();

        ~Analyzer();

        /**
         * @brief Pushes in a new data point if we have waited long enough
         *
         * @param data
         */
        void push(AccumulatorData_t *data);

        /**
         * @brief Calculates the PWM required to drive the fans at the current moment in time
         *
         * @param bmsdata
         * @return uint8_t
         */
        uint8_t calcFanPWM();

        /**
         * @brief Pointer to the address of the most recent data point
         *
         */
        AccumulatorData_t *bmsdata;

        AccumulatorData_t *prevbmsdata;
};

extern Analyzer analyzer;

#endif