#include "analyzer.h"

Analyzer analyzer;

Analyzer::Analyzer(){}

Analyzer::~Analyzer(){}

void Analyzer::push(AccumulatorData_t *data)
{
    if(prevbmsdata != nullptr)
        delete prevbmsdata;

    prevbmsdata = bmsdata;
    bmsdata = data;

    disableTherms();
<<<<<<< HEAD
=======

    highCurrThermCheck(); // = prev if curr > 50 
    //diffCurrThermCheck(); // = prev if curr - prevcurr > 10 
    //varianceThermCheck();// = prev if val > 5 deg difference     
    //standardDevThermCheck(); // = prev if std dev > 3
    //averagingThermCheck(); // matt shitty incrementing

>>>>>>> main
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
<<<<<<< HEAD
    for(uint8_t c = 1; c < NUM_CHIPS; c++)
=======
    int total_seg_temp = 0;
    for(uint8_t c = 0; c < NUM_CHIPS; c++)
>>>>>>> main
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
<<<<<<< HEAD
=======
            total_seg_temp += bmsdata->chip_data[c].thermistor_value[therm];
        }
        if (c % 2 == 0) {
            bmsdata->segment_average_temps[c/2] = total_seg_temp / 22;
            total_seg_temp = 0;
>>>>>>> main
        }
    }

    // takes the average of all the cell temperatures
    bmsdata->avg_temp = total_temp / 88;
}

void Analyzer::calcPackVoltageStats() {
    bmsdata->max_voltage = {MIN_VOLT_MEAS, 0, 0};
<<<<<<< HEAD
    bmsdata->min_voltage = {MAX_VOLT_MEAS, 0, 0};
    uint32_t total_volt = 0;
    for(uint8_t c = 0; c < NUM_CHIPS; c++)
    {
        for(uint8_t cell = 0; cell < 9; cell++)
=======
    bmsdata->max_ocv = {MIN_VOLT_MEAS, 0, 0};
    bmsdata->min_voltage = {MAX_VOLT_MEAS, 0, 0};
    bmsdata->min_ocv = {MAX_VOLT_MEAS, 0, 0};
    uint32_t total_volt = 0;
    uint32_t total_ocv = 0;
    for(uint8_t c = 0; c < NUM_CHIPS; c++)
    {
        for(uint8_t cell = 0; cell < NUM_CELLS_PER_CHIP; cell++)
>>>>>>> main
        {
            // fings out the maximum cell voltage and location
            if (bmsdata->chip_data[c].voltage_reading[cell] > bmsdata->max_voltage.val)
            {
                bmsdata->max_voltage = {bmsdata->chip_data[c].voltage_reading[cell], c, cell};
<<<<<<< HEAD
            }

            //finds out the minimum cell voltage and location
            else if (bmsdata->chip_data[c].voltage_reading[cell] < bmsdata->min_voltage.val)
=======
            }

            if (bmsdata->chip_data[c].open_cell_voltage[cell] > bmsdata->max_ocv.val)
            {
                bmsdata->max_ocv = {bmsdata->chip_data[c].open_cell_voltage[cell], c, cell};
            }

            //finds out the minimum cell voltage and location
            if (bmsdata->chip_data[c].voltage_reading[cell] < bmsdata->min_voltage.val)
>>>>>>> main
            {
                bmsdata->min_voltage = {bmsdata->chip_data[c].voltage_reading[cell], c, cell};
            }

<<<<<<< HEAD
            total_volt += bmsdata->chip_data[c].voltage_reading[cell];
=======
            if (bmsdata->chip_data[c].open_cell_voltage[cell] < bmsdata->min_ocv.val)
            {
                bmsdata->min_ocv = {bmsdata->chip_data[c].open_cell_voltage[cell], c, cell};
            }

            total_volt += bmsdata->chip_data[c].voltage_reading[cell];
            total_ocv += bmsdata->chip_data[c].open_cell_voltage[cell];
>>>>>>> main
        }
    }

    // calculate some voltage stats
    bmsdata->avg_voltage = total_volt / (NUM_CELLS_PER_CHIP * NUM_CHIPS);
    bmsdata->pack_voltage = total_volt / 1000; // convert to voltage * 10
    bmsdata->delt_voltage = bmsdata->max_voltage.val - bmsdata->min_voltage.val;
<<<<<<< HEAD
=======

    bmsdata->avg_ocv = total_ocv / (NUM_CELLS_PER_CHIP * NUM_CHIPS);
    bmsdata->pack_ocv = total_ocv / 1000; // convert to voltage * 10
    bmsdata->delt_ocv = bmsdata->max_ocv.val - bmsdata->min_ocv.val;
>>>>>>> main
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
<<<<<<< HEAD
=======

    typedef enum
	{
		BEFORE_TIMER_START,
		DURING_DCL_EVAL
	}DCL_state;

    struct DCLeval
    {
	    DCL_state state = BEFORE_TIMER_START;
	    Timer timer;
    };

    DCLeval dclEval;
>>>>>>> main
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
<<<<<<< HEAD
    else
    {
        bmsdata->discharge_limit = current_limit;
=======

    else if (dclEval.state == BEFORE_TIMER_START && current_limit < 5) // 5 is arbitrary @ matt adjust as needed
    {
        if (prevbmsdata == nullptr)
        {
            bmsdata->discharge_limit = current_limit;
            return;
        }

        bmsdata->discharge_limit = prevbmsdata->discharge_limit;
        dclEval.state = DURING_DCL_EVAL;
        dclEval.timer.startTimer(500); // 1 second is arbitrary @ matt adjust as needed
>>>>>>> main
    }

    else if (dclEval.state == DURING_DCL_EVAL)
    {
        if (dclEval.timer.isTimerExpired())
        {
            bmsdata->discharge_limit = current_limit;
        }
        if (current_limit > 5)
        {
            bmsdata->discharge_limit = current_limit;
            dclEval.state = BEFORE_TIMER_START;
            dclEval.timer.cancelTimer();
        }

        else 
        {
            bmsdata->discharge_limit = prevbmsdata->discharge_limit;
        }
    }
    else
    {
        bmsdata->discharge_limit = current_limit;
    }

    if (bmsdata->discharge_limit > DCDC_CURRENT_DRAW)
        bmsdata->discharge_limit -= DCDC_CURRENT_DRAW;
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
        return;
    }
    // If we are within the current threshold for open voltage measurments
<<<<<<< HEAD
     else if (bmsdata->pack_current < (OCV_CURR_THRESH * 10) && bmsdata->pack_current > (-OCV_CURR_THRESH * 10))
    {
        for (uint8_t chip = 0; chip < NUM_CHIPS; chip++)
        {
            for (uint8_t cell = 0; cell < NUM_CELLS_PER_CHIP; cell++)
            {
                // Sets open cell voltage to a moving average of OCV_AVG values
                bmsdata->chip_data[chip].open_cell_voltage[cell] = (uint32_t(bmsdata->chip_data[chip].voltage_reading[cell]) + (uint32_t(prevbmsdata->chip_data[chip].open_cell_voltage[cell])  * (OCV_AVG - 1))) / OCV_AVG;
=======
    else if (bmsdata->pack_current < (OCV_CURR_THRESH * 10) && bmsdata->pack_current > (-OCV_CURR_THRESH * 10))
    {
        if (ocvTimer.isTimerExpired()) {
            for (uint8_t chip = 0; chip < NUM_CHIPS; chip++)
            {
                for (uint8_t cell = 0; cell < NUM_CELLS_PER_CHIP; cell++)
                {
                    // Sets open cell voltage to a moving average of OCV_AVG values
                    bmsdata->chip_data[chip].open_cell_voltage[cell] = (uint32_t(bmsdata->chip_data[chip].voltage_reading[cell]) + (uint32_t(prevbmsdata->chip_data[chip].open_cell_voltage[cell])  * (OCV_AVG - 1))) / OCV_AVG;
                    bmsdata->chip_data[chip].open_cell_voltage[cell] = bmsdata->chip_data[chip].voltage_reading[cell];

                    if (bmsdata->chip_data[chip].open_cell_voltage[cell] > MAX_VOLT * 10000) 
                    {
                        bmsdata->chip_data[chip].open_cell_voltage[cell] = prevbmsdata->chip_data[chip].open_cell_voltage[cell];
                    } 
                    else if (bmsdata->chip_data[chip].open_cell_voltage[cell] < MIN_VOLT * 10000) 
                    {
                        bmsdata->chip_data[chip].open_cell_voltage[cell] = prevbmsdata->chip_data[chip].open_cell_voltage[cell];
                    }
                }
>>>>>>> main
            }
            return;
        }
    }
<<<<<<< HEAD
    else
    {
        for (uint8_t chip = 0; chip < NUM_CHIPS; chip++)
        {
            for (uint8_t cell = 0; cell < NUM_CELLS_PER_CHIP; cell++)
            {
                // Set OCV to the previous/existing OCV
                bmsdata->chip_data[chip].open_cell_voltage[cell] = prevbmsdata->chip_data[chip].open_cell_voltage[cell];
            }
=======
    else 
    {
        ocvTimer.startTimer(1000);
    }
    for (uint8_t chip = 0; chip < NUM_CHIPS; chip++)
    {
        for (uint8_t cell = 0; cell < NUM_CELLS_PER_CHIP; cell++)
        {
            // Set OCV to the previous/existing OCV
            bmsdata->chip_data[chip].open_cell_voltage[cell] = prevbmsdata->chip_data[chip].open_cell_voltage[cell];
>>>>>>> main
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
<<<<<<< HEAD
    int index = (((bmsdata->min_voltage.val) - MIN_VOLT) / .1);

    // .1 = 1.3V range / 13 datapoints on curve
    if (index >= 13)
        bmsdata->soc = 100;
    else
    {
        float distance_from_higher = (bmsdata->min_voltage.val) - ((index / 10) + 2.9);
        bmsdata->soc = ((distance_from_higher*STATE_OF_CHARGE_CURVE[index+1]) + ((1-distance_from_higher)*STATE_OF_CHARGE_CURVE[index]));
    }
}

void Analyzer::calcPackResistances(AccumulatorData_t *bmsdata)
{
    float pack_resistance[NUM_CHIPS][NUM_CELLS_PER_CHIP];
    // Want to calculate the pack resistance when current is high
    if(bmsdata->pack_current >= 100)
    {
        calcCellResistances();
        for(int chip = 0; chip < NUM_CHIPS; chip++)
        {
            for(int cell = 0; cell < NUM_CELLS_PER_CHIP; cell++)
            {
                pack_resistance[chip][cell] = (bmsdata->chip_data[chip].open_cell_voltage[cell] - bmsdata->chip_data[chip].voltage_reading[cell]) / bmsdata->pack_current;
                // If the tested resistance is greater than the thermal limit then it is set to the thermal limit
                pack_resistance[chip][cell] = (pack_resistance[chip][cell] > bmsdata->chip_data[chip].cell_resistance[cell]) ? bmsdata->chip_data[chip].cell_resistance[cell] : pack_resistance[chip][cell];
                Serial.println(pack_resistance[chip][cell]);
                Serial.println("  ");
            }
            Serial.println("\n");
        }
    }
    // Update the bms data when current is low
    if(bmsdata->pack_current < 5)
    {
       for(int chip = 0; chip < NUM_CHIPS; chip++)
        {
            for(int cell = 0; cell < NUM_CELLS_PER_CHIP; cell++)
            {
                bmsdata->chip_data[chip].cell_resistance[cell] = pack_resistance[chip][cell];
            }
        }
    }
}
=======
     /* Spltting the delta voltage into 18 increments */
    const uint16_t increments = ((uint16_t)(MAX_VOLT*10000 - MIN_VOLT*10000) / ((MAX_VOLT - MIN_VOLT) * 10));

    /* Retrieving a index of 0-18 */
    uint8_t index = ((bmsdata->min_ocv.val) - MIN_VOLT*10000) / increments;

    bmsdata->soc = STATE_OF_CHARGE_CURVE[index];

    if (bmsdata->soc != 100)
    {
        float interpolation = (float)(STATE_OF_CHARGE_CURVE[index+1] - STATE_OF_CHARGE_CURVE[index]) / increments;
        bmsdata->soc += (uint8_t)(interpolation * (((bmsdata->min_ocv.val) - (int32_t)(MIN_VOLT * 10000)) % increments));
    }

    if (bmsdata->soc < 0)
    {
        bmsdata->soc = 0;
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




>>>>>>> main
