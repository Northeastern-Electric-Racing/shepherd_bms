#include "analyzer.h"

Analyzer analyzer;

uint8_t therm_avg_counter = 0;

Analyzer::Analyzer(){}

Analyzer::~Analyzer(){}

void Analyzer::push(AccumulatorData_t *data)
{
    if(prevbmsdata != nullptr)
        delete prevbmsdata;

    prevbmsdata = bmsdata;
    bmsdata = data;

    disableTherms();

    highCurrThermCheck(); // = prev if curr > 50 
    diffCurrThermCheck(); // = prev if curr - prevcurr > 10 
    varianceThermCheck();// = prev if val > 5 deg difference     
    standardDevThermCheck(); // = prev if std dev > 3
    //averagingThermCheck(); // matt shitty incrementing

    calcCellTemps();
	calcPackTemps();
	calcPackVoltageStats();
	calcOpenCellVoltage();
	calcCellResistances();
	calcDCL();
	calcContDCL();
	calcContCCL();
	calcStateOfCharge();

    is_first_reading_ = false;
}

void Analyzer::calcCellTemps()
{
    for(uint8_t c = 0; c < NUM_CHIPS; c++)
    {
        for(uint8_t cell = 0; cell < NUM_CELLS_PER_CHIP; cell++)
        {
            int temp_sum = 0;
            for(uint8_t therm = 0; therm < RelevantThermMap[cell].size(); therm++)
            {
                uint8_t thermNum = RelevantThermMap[cell][therm];
                temp_sum += bmsdata->chip_data[c].thermistor_value[thermNum];
            }

            //Takes the average temperature of all the relevant thermistors
            bmsdata->chip_data[c].cell_temp[cell] = temp_sum / RelevantThermMap[cell].size();

            //Cleansing value
            if(bmsdata->chip_data[c].cell_temp[cell] > MAX_TEMP)
            {
                bmsdata->chip_data[c].cell_temp[cell] = MAX_TEMP;
            }
        }
    }
}

void Analyzer::calcPackTemps()
{
    bmsdata->max_temp = {MIN_TEMP, 0, 0};
    bmsdata->min_temp = {MAX_TEMP, 0, 0};
    int total_temp = 0;
    for(uint8_t c = 1; c < NUM_CHIPS; c++)
    {
        for(uint8_t therm = 17; therm < 28; therm++)
        {
            // finds out the maximum cell temp and location
            if (bmsdata->chip_data[c].thermistor_value[therm] > bmsdata->max_temp.val)
            {
                bmsdata->max_temp = {bmsdata->chip_data[c].thermistor_value[therm], c, therm};
            }

            // finds out the minimum cell temp and location
            if (bmsdata->chip_data[c].thermistor_value[therm] < bmsdata->min_temp.val)
            {
                bmsdata->min_temp = {bmsdata->chip_data[c].thermistor_value[therm], c, therm};
            }

            total_temp += bmsdata->chip_data[c].thermistor_value[therm];
        }
    }

    // takes the average of all the cell temperatures
    bmsdata->avg_temp = total_temp / 88;
}

void Analyzer::calcPackVoltageStats() {
    bmsdata->max_voltage = {MIN_VOLT_MEAS, 0, 0};
    bmsdata->min_voltage = {MAX_VOLT_MEAS, 0, 0};
    uint32_t total_volt = 0;
    for(uint8_t c = 0; c < NUM_CHIPS; c++)
    {
        for(uint8_t cell = 0; cell < 9; cell++)
        {
            // fings out the maximum cell voltage and location
            if (bmsdata->chip_data[c].voltage_reading[cell] > bmsdata->max_voltage.val)
            {
                bmsdata->max_voltage = {bmsdata->chip_data[c].voltage_reading[cell], c, cell};
            }

            //finds out the minimum cell voltage and location
            else if (bmsdata->chip_data[c].voltage_reading[cell] < bmsdata->min_voltage.val)
            {
                bmsdata->min_voltage = {bmsdata->chip_data[c].voltage_reading[cell], c, cell};
            }

            total_volt += bmsdata->chip_data[c].voltage_reading[cell];
        }
    }

    // calculate some voltage stats
    bmsdata->avg_voltage = total_volt / (NUM_CELLS_PER_CHIP * NUM_CHIPS);
    bmsdata->pack_voltage = total_volt / 1000; // convert to voltage * 10
    bmsdata->delt_voltage = bmsdata->max_voltage.val - bmsdata->min_voltage.val;
}

void Analyzer::calcCellResistances()
{
    for(uint8_t c = 0; c < NUM_CHIPS; c++)
    {
        for(uint8_t cell = 0; cell < NUM_CELLS_PER_CHIP; cell++)
        {
            uint8_t cell_temp = bmsdata->chip_data[c].cell_temp[cell];
            uint8_t resIndex = (cell_temp - MIN_TEMP) / 5;  //resistance LUT increments by 5C for each index

            bmsdata->chip_data[c].cell_resistance[cell] = TEMP_TO_CELL_RES[resIndex];

            //Linear interpolation to more accurately represent cell resistances in between increments of 5C
            if(cell_temp != MAX_TEMP)
            {
                float interpolation = (TEMP_TO_CELL_RES[resIndex+1] - TEMP_TO_CELL_RES[resIndex]) / 5;
                bmsdata->chip_data[c].cell_resistance[cell] += (interpolation * (cell_temp % 5));
            }
        }
    }
}

void Analyzer::calcDCL()
{
    int16_t current_limit = 0x7FFF;

    for(uint8_t c = 0; c < NUM_CHIPS; c++)
    {
        for(uint8_t cell = 0; cell < NUM_CELLS_PER_CHIP; cell++)
        {
            // Apply equation
            uint16_t tmpDCL = (bmsdata->chip_data[c].open_cell_voltage[cell] - ((MIN_VOLT + VOLT_SAG_MARGIN) * 10000)) / (bmsdata->chip_data[c].cell_resistance[cell] * 10);
            // Multiplying resistance by 10 to convert from mOhm to Ohm and then to Ohm * 10000 to account for the voltage units

            //Taking the minimum DCL of all the cells
            if(tmpDCL < current_limit) current_limit = tmpDCL;
        }
    }

    // ceiling for current limit
    if (current_limit > MAX_CELL_CURR)
    {
        bmsdata->discharge_limit = MAX_CELL_CURR;
    }
    else
    {
        bmsdata->discharge_limit = current_limit;
    }
}

void Analyzer::calcContDCL()
{
    uint8_t min_res_index = (bmsdata->min_temp.val - MIN_TEMP) / 5;  //resistance LUT increments by 5C for each index
    uint8_t max_res_index = (bmsdata->max_temp.val - MIN_TEMP) / 5;

    if (TEMP_TO_DCL[min_res_index] < TEMP_TO_DCL[max_res_index])
    {
        bmsdata->cont_DCL = TEMP_TO_DCL[min_res_index];
    }
    else
    {
        bmsdata->cont_DCL = TEMP_TO_DCL[max_res_index];
    }
}

void Analyzer::calcContCCL()
{
    uint8_t min_res_index = (bmsdata->min_temp.val - MIN_TEMP) / 5;  //resistance LUT increments by 5C for each index
    uint8_t max_res_index = (bmsdata->max_temp.val - MIN_TEMP) / 5;

    if (TEMP_TO_CCL[min_res_index] < TEMP_TO_CCL[max_res_index])
    {
        bmsdata->charge_limit = TEMP_TO_CCL[min_res_index];
    }
    else
    {
        bmsdata->charge_limit = TEMP_TO_CCL[max_res_index];
    }
}

void Analyzer::calcOpenCellVoltage()
{
    // if there is no previous data point, set inital open cell voltage to current reading
    if (is_first_reading_)
    {
        for (uint8_t chip = 0; chip < NUM_CHIPS; chip++)
        {
            for (uint8_t cell = 0; cell < NUM_CELLS_PER_CHIP; cell++)
            {
                bmsdata->chip_data[chip].open_cell_voltage[cell] = bmsdata->chip_data[chip].voltage_reading[cell];
            }
        }
    }
    // If we are within the current threshold for open voltage measurments
     else if (bmsdata->pack_current < (OCV_CURR_THRESH * 10) && bmsdata->pack_current > (-OCV_CURR_THRESH * 10))
    {
        for (uint8_t chip = 0; chip < NUM_CHIPS; chip++)
        {
            for (uint8_t cell = 0; cell < NUM_CELLS_PER_CHIP; cell++)
            {
                // Sets open cell voltage to a moving average of OCV_AVG values
                bmsdata->chip_data[chip].open_cell_voltage[cell] = (uint32_t(bmsdata->chip_data[chip].voltage_reading[cell]) + (uint32_t(prevbmsdata->chip_data[chip].open_cell_voltage[cell])  * (OCV_AVG - 1))) / OCV_AVG;
            }
        }
    }
    else
    {
        for (uint8_t chip = 0; chip < NUM_CHIPS; chip++)
        {
            for (uint8_t cell = 0; cell < NUM_CELLS_PER_CHIP; cell++)
            {
                // Set OCV to the previous/existing OCV
                bmsdata->chip_data[chip].open_cell_voltage[cell] = prevbmsdata->chip_data[chip].open_cell_voltage[cell];
            }
        }
    }
}

uint8_t Analyzer::calcFanPWM()
{
    // Resistance LUT increments by 5C for each index, plus we account for negative minimum
    uint8_t min_res_index = (bmsdata->max_temp.val - MIN_TEMP) / 5;
    // Ints are roounded down, so this would be the value if rounded up
    uint8_t max_res_index = (bmsdata->max_temp.val - MIN_TEMP) / 5 + 1;
    // Determine how far into the 5C interval the temp is
    uint8_t part_of_index = (bmsdata->max_temp.val - MIN_TEMP) % 5;

    // Uses fan LUT and finds low and upper end. Then takes average, weighted to how far into the interval the exact temp is
    return ((FAN_CURVE[max_res_index] * part_of_index) + (FAN_CURVE[min_res_index] * (5 - part_of_index))) / (2 * 5);
}

void Analyzer::disableTherms()
{
    int8_t temp_rep_1 = 25; // Iniitalize to room temp (necessary to stabilize when the BMS first boots up/has null values)
    //if (!is_first_reading_) temp_rep_1 = prevbmsdata->avg_temp; // Set to actual average temp of the pack

    for(uint8_t c = 0; c < NUM_CHIPS; c++)
    {
        for(uint8_t therm = 17; therm < 28; therm++)
        {
            // If 2D LUT shows therm should be disable
            if (THERM_DISABLE[(c - 1) / 2][therm - 17])
            {
                // Nullify thermistor by setting to pack average
                bmsdata->chip_data[c].thermistor_value[therm] = temp_rep_1;
            }
        }
    }
}

void Analyzer::calcStateOfCharge()
{
    int index = (((float(bmsdata->min_voltage.val)/10000) - MIN_VOLT) / .1);

    // .1 = 1.3V range / 13 datapoints on curve
    if (index >= 13)
        bmsdata->soc = 100;
    else
    {
        float distance_from_higher = (float(bmsdata->min_voltage.val)/10000) - ((index / 10) + 2.9);
        bmsdata->soc = ((distance_from_higher*STATE_OF_CHARGE_CURVE[index+1]) + ((1-distance_from_higher)*STATE_OF_CHARGE_CURVE[index]));
    }
}

void Analyzer::highCurrThermCheck()
{
    if (prevbmsdata == nullptr)
        return;

    if (bmsdata->pack_current > 500) {
    
        for(uint8_t c = 0; c < NUM_CHIPS; c++)
        {
            for(uint8_t cell = 0; cell < NUM_CELLS_PER_CHIP; cell++)
            {
                bmsdata->chip_data[c].thermistor_reading[cell] = prevbmsdata->chip_data[c].thermistor_reading[cell];
                bmsdata->chip_data[c].thermistor_value[cell] = prevbmsdata->chip_data[c].thermistor_value[cell];
            }
        }
    }
}

void Analyzer::diffCurrThermCheck()
{
    if (prevbmsdata == nullptr)
        return;

    if (abs(bmsdata->pack_current - prevbmsdata->pack_current) > 100) {
        for(uint8_t c = 0; c < NUM_CHIPS; c++)
        {
            for(uint8_t cell = 0; cell < NUM_CELLS_PER_CHIP; cell++)
            {
                bmsdata->chip_data[c].thermistor_reading[cell] = prevbmsdata->chip_data[c].thermistor_reading[cell];
                bmsdata->chip_data[c].thermistor_value[cell] = prevbmsdata->chip_data[c].thermistor_value[cell];
            }
        }
    }
}


void Analyzer::varianceThermCheck()
{
    if (prevbmsdata == nullptr)
        return;

    for(uint8_t c = 0; c < NUM_CHIPS; c++)
    {
        for(uint8_t cell = 0; cell < NUM_CELLS_PER_CHIP; cell++)
        {
            if (abs(bmsdata->chip_data[c].thermistor_reading[cell] - prevbmsdata->chip_data[c].thermistor_reading[cell]) > 5 
                && (bmsdata->chip_data[c].thermistor_reading[cell] < 10
                || bmsdata->chip_data[c].thermistor_reading[cell] > 45))
            {
                bmsdata->chip_data[c].thermistor_reading[cell] = prevbmsdata->chip_data[c].thermistor_reading[cell];
                bmsdata->chip_data[c].thermistor_value[cell] = prevbmsdata->chip_data[c].thermistor_value[cell];
            }

                
        }
    }

}

uint8_t Analyzer::calcThermStandardDev()
{
    uint16_t sum_diff_sqrd = 0;
    for(uint8_t chip = 0; chip < NUM_CHIPS; chip++)
    {
        for(uint8_t therm = 17; therm < 28; therm++)
        {
            uint16_t sum_diff =  abs(bmsdata->chip_data[chip].thermistor_value[therm] - bmsdata->avg_temp);
            sum_diff_sqrd += sum_diff * sum_diff;
        }
    }

    uint8_t standard_dev = sqrt(sum_diff_sqrd / 88);
    if(standard_dev < 8)
    {
        standard_dev = 8;
    }
    return standard_dev;
}

void Analyzer::standardDevThermCheck()
{
    if (prevbmsdata == nullptr)
        return;

    uint8_t standard_dev = calcThermStandardDev();
    for(uint8_t c = 0; c < NUM_CHIPS; c++)
    {
        for(uint8_t therm = 17; therm < 28; therm++)
        {
            // If difference between thermistor and average is more than MAX_STANDARD_DEV set the therm to pack average
            if (abs(bmsdata->chip_data[c].thermistor_value[therm] - bmsdata->avg_temp) > (MAX_STANDARD_DEV * standard_dev))
            {
                // Nullify thermistor by setting to pack average
                bmsdata->chip_data[c].thermistor_value[therm] = prevbmsdata->chip_data[c].thermistor_value[therm];
            }

        }
    }
}

void Analyzer::averagingThermCheck()
{
    for (int therm = 1; therm <= 16; therm++)
    {
        for (int c = 0; c < NUM_CHIPS; c++)
        {
 // Directly update for a set time from start up due to therm voltages needing to settle
            if (therm_avg_counter < THERM_AVG * 10) 
            {
                bmsdata->chip_data[c].thermistor_value[therm - 1] = bmsdata->chip_data[c].thermistor_reading[therm - 1];
                bmsdata->chip_data[c].thermistor_value[therm + 15] = bmsdata->chip_data[c].thermistor_reading[therm + 15];
                therm_avg_counter++;
            } else 
            {
                // We need to investigate this. Very sloppy
                // Discard if reading is 33C
                if (bmsdata->chip_data[c].thermistor_reading[therm - 1] != 33) 
                {
                    // If measured value is larger than current "averaged" value, increment value
                    if (bmsdata->chip_data[c].thermistor_reading[therm - 1] > bmsdata->chip_data[c].thermistor_value[therm - 1]) 
                    {
                    bmsdata->chip_data[c].thermistor_value[therm - 1]++;
                    // If measured value is smaller than current "averaged" value, decrement value
                    } else if (bmsdata->chip_data[c].thermistor_reading[therm - 1] < bmsdata->chip_data[c].thermistor_value[therm - 1]) 
                    {
                        bmsdata->chip_data[c].thermistor_value[therm - 1]--;
                    }
                }
                
                // See comments above. Identical but for the upper 16 therms
                if (bmsdata->chip_data[c].thermistor_reading[therm + 15] != 33)
                {
                    if (bmsdata->chip_data[c].thermistor_reading[therm + 15] > bmsdata->chip_data[c].thermistor_value[therm + 15])
                    {
                        bmsdata->chip_data[c].thermistor_value[therm + 15]++;
                    } else if (bmsdata->chip_data[c].thermistor_reading[therm + 15] < bmsdata->chip_data[c].thermistor_value[therm + 15])
                    {
                        bmsdata->chip_data[c].thermistor_value[therm + 15]--;
                    }
                }
            }
        }
    }
}

