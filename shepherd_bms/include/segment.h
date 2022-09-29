#ifndef SEGMENT_H
#define SEGMENT_H

#include <nerduino.h>
#include "datastructs.h"

#define NUM_SEGMENTS    4
#define NUM_CHIPS       NUM_SEGMENTS*2
#define THERM_REF       4700            // Reference resistor for thermistor measurments / voltage divider
#define MIN_VOLT        2.9
#define MAX_VOLT        4.2
#define MAX_DELTA_V     0.02
#define BAL_MIN_V       4.00

const uint32_t VOLT_TEMP_CONV[] = 
{
    45140, 44890, 44640, 44370, 44060, 43800, 43510, 43210, 42900, 42560,
    42240, 41900, 41550, 41170, 40820, 40450, 40040, 39650, 39220, 38810, 
    38370, 37950, 37500, 37090, 36650, 36180, 35670, 35220, 34740, 34230,
    33770, 33270, 32770, 32280, 31770, 31260, 30750, 30240, 29720, 29220, 
    28710, 28200, 27680, 27160, 26660, 26140, 25650, 25130, 24650, 24150, 
    23660, 23170, 22670, 22190, 21720, 21240, 0};

const uint32_t TEMP_CONV[] = 
{
    33200, 31500, 29900, 28400, 26900, 25600, 24300, 23100, 21900, 20900, 
    19900, 18900, 18000, 17100, 16300, 15500, 14800, 14100, 13500, 12900, 
    12300, 11700, 11200, 10700, 10200, 9780, 9350, 8940, 8560, 8190, 7840, 
    7510, 7190, 6890, 6610, 6340, 6080, 5830, 5590, 5370, 5150, 4950, 4750, 
    4570, 4390, 4220, 4060, 3900, 3750, 3610, 3470, 0};


/**
 * @brief This class serves as the interface for all of the segment boards
 */
class SegmentInterface
{
    private:

    public:
        
};

#endif