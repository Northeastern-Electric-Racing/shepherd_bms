#include "segment.h"

SegmentInterface segment;

SegmentInterface::SegmentInterface(){}

SegmentInterface::~SegmentInterface(){}

void SegmentInterface::init()
{
    Serial.println("Initializing Segments...");

    LTC6804_initialize();

    //pullChipConfigurations();

    for (int c = 0; c < NUM_CHIPS; c++)
    {
    local_config[c][0] = 0xF8;
    local_config[c][1] = 0x19; // VUV = 0x619 = 1561 -> 2.4992V
    local_config[c][2] = 0x06; // VOV = 0xA60 = 2656 -> 4.2496V
    local_config[c][3] = 0xA6;
    local_config[c][4] = 0x00;
    local_config[c][5] = 0x00;
    }
    pushChipConfigurations();
}

void SegmentInterface::retrieveSegmentData(ChipData_t databuf[NUM_CHIPS])
{
    segment_data = databuf;

    /**
     * Pull voltages and thermistors and indiacte if there was a problem during retrieval
     */
    voltage_error = pullVoltages();
    therm_error = pullThermistors();

    /**
     * Save the contents of the reading so that we can use it to fill in missing data
     */
    memcpy(previous_data, segment_data, sizeof(ChipData_t)*NUM_CHIPS);

    segment_data = nullptr;
}

void SegmentInterface::configureDischarge(uint8_t chip, uint16_t cells)
{
  // chipConfigurations[chip][4] == chipConfigurations[Literally what chip you want][register]
  // 4 and 5 are registers to discharge chips
  local_config[chip][4] = uint8_t(cells & 0x00FF);
  // Register 5 is split in half, so we maintain the upper half and add in the bottom half to discharge cells
  local_config[chip][5] = (local_config[chip][5] & 0xF0) + uint8_t(cells >> 8);
}

void SegmentInterface::enableBalancing(bool balance_enable)
{
    //Discharging all cells in series
    static const uint16_t DICHARGE_ALL_COMMAND = 0xFFFF >> (16-NUM_CELLS_PER_CHIP); // Making the discharge command all 1's for all cells per chip

    if(balance_enable)
    {
		for(int c = 0; c < NUM_CHIPS; c++)
        {
            configureDischarge(c, DICHARGE_ALL_COMMAND);
			discharge_commands[c] = DICHARGE_ALL_COMMAND;
        }
        pushChipConfigurations();
    }
    else
    {
        for (int c = 0; c < NUM_CHIPS; c++)
        {
            configureDischarge(c, 0);
			discharge_commands[c] = 0;
        }
        pushChipConfigurations();
    }
}

/**
 * @todo Revisit after testing
 */
void SegmentInterface::enableBalancing(uint8_t chip_num, uint8_t cell_num, bool balance_enable)
{
    pullChipConfigurations();

	if(balance_enable)
		discharge_commands[chip_num] |= (1 << cell_num);
	else
		discharge_commands[chip_num] &= ~(1 << cell_num);

    configureDischarge(chip_num, discharge_commands[chip_num]);

    pushChipConfigurations();
}

void SegmentInterface::configureBalancing(bool discharge_config[NUM_CHIPS][NUM_CELLS_PER_CHIP])
{
    for(int c = 0; c < NUM_CHIPS; c++)
    {
        for(int cell = 0; cell < NUM_CELLS_PER_CHIP; cell++)
        {
            if(discharge_config[c][cell])
				discharge_commands[c] |= 1 << cell;
			else
				discharge_commands[c] &= ~(1 << cell);
        }

        configureDischarge(c, discharge_commands[c]);
    }
    pushChipConfigurations();
}

bool SegmentInterface::isBalancing(uint8_t chip_num, uint8_t cell_num)
{
    //If the cell is one of the first 8, check the 4th register
    if(cell_num < 8)
    {
        return local_config[chip_num][4] & (1 << cell_num);
    }
    //If the cell number is greater than 8, check the 5th register
    else
    {
        return local_config[chip_num][5] & (1 << (cell_num - 8));
    }

    return false; //default case
}

bool SegmentInterface::isBalancing()
{
    for(int c = 0; c < NUM_CHIPS; c++)
    {
        //Reading from the 4th config register
        for(int cell = 0; cell < 8; cell++)
        {
            if(local_config[c][4] & (1 << cell)) return true;
        }

        //Reading from the 5th config register
        for(int cell = 0; cell < 4; cell++)
        {
            if(local_config[c][5] & (1 << (cell))) return true;
        }
    }

    return false;
}

void SegmentInterface::pullChipConfigurations()
{
    uint8_t remote_config[NUM_CHIPS][8];
    LTC6804_rdcfg(NUM_CHIPS, remote_config);

    for (int chip = 0; chip < NUM_CHIPS; chip++)
    {
        for(int index = 0; index < 6; index++)
        {
            local_config[chip][index] = remote_config[chip][index];
        }
    }
}

void SegmentInterface::pushChipConfigurations()
{
    LTC6804_wrcfg(NUM_CHIPS, local_config);
}

FaultStatus_t SegmentInterface::pullVoltages()
{
    /**
     * If we haven't waited long enough between pulling voltage data
     * just copy over the contents of the last good reading and the fault status from the most recent attempt
     */
    if(!voltage_reading_timer.isTimerExpired())
    {
        for(uint8_t i=0; i<NUM_CHIPS; i++)
        {
            memcpy(segment_data[i].voltage_reading, previous_data[i].voltage_reading, sizeof(segment_data[i].voltage_reading));
        }
        return voltage_error;
    }

    uint16_t segment_voltages[NUM_CHIPS][12];

    pushChipConfigurations();
    LTC6804_adcv();

    /**
     * If we received an incorrect PEC indicating a bad read
     * copy over the data from the last good read and indicate an error
     */
    if(LTC6804_rdcv(0, NUM_CHIPS, segment_voltages) == -1)
    {
        for(uint8_t i=0; i<NUM_CHIPS; i++)
        {
            memcpy(segment_data[i].voltage_reading, previous_data[i].voltage_reading, sizeof(segment_data[i].voltage_reading));
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
            segment_data[i].voltage_reading[j] = segment_voltages[i][j];
        }
    }

    /**
     * Start the timer between readings if successful
     */
    voltage_reading_timer.startTimer(VOLTAGE_WAIT_TIME);
    return NOT_FAULTED;
}

FaultStatus_t SegmentInterface::pullThermistors()
{
    // If polled too soon, just copy existing values from memory
	if (!therm_timer.isTimerExpired())
	{
		for(uint8_t i=0; i<NUM_CHIPS; i++)
        {
            memcpy(segment_data[i].thermistor_reading, previous_data[i].thermistor_reading, sizeof(segment_data[i].thermistor_reading));
            memcpy(segment_data[i].thermistor_value, previous_data[i].thermistor_value, sizeof(segment_data[i].thermistor_value));
        }
        return voltage_error;
	}

    uint16_t raw_temp_voltages[NUM_CHIPS][6];

    // Rotate through all thermistor pairs (we can poll two at once)
    for (int therm = 1; therm <= 16; therm++)
	{
        // Sets multiplexors to select thermistors
        SelectTherm(therm);
        SelectTherm(therm + 16);

		pushChipConfigurations();
        LTC6804_adax(); // Run ADC for AUX (GPIOs and refs)
        LTC6804_rdaux(0, NUM_CHIPS, raw_temp_voltages); // Fetch ADC results from AUX registers

        for (uint8_t c = 0; c < NUM_CHIPS; c++)
		{
            // Get current temperature LUT. Voltage is adjusted to account for 5V reg fluctuations (index 2 is a reading of the ADC 5V ref)
            segment_data[c].thermistor_reading[therm - 1] = steinhartEst(raw_temp_voltages[c][0] * (float(raw_temp_voltages[c][2]) / 50000) + VOLT_TEMP_CALIB_OFFSET);
            segment_data[c].thermistor_reading[therm + 15] = steinhartEst(raw_temp_voltages[c][1] * (float(raw_temp_voltages[c][2]) / 50000) + VOLT_TEMP_CALIB_OFFSET);

            // Directly update for a set time from start up due to therm voltages needing to settle
            segment_data[c].thermistor_value[therm - 1] = segment_data[c].thermistor_reading[therm - 1];
            segment_data[c].thermistor_value[therm + 15] = segment_data[c].thermistor_reading[therm + 15];

            if (raw_temp_voltages[c][0] == LTC_BAD_READ || raw_temp_voltages[c][1] == LTC_BAD_READ)
            {
                memcpy(segment_data[c].thermistor_reading, previous_data[c].thermistor_reading, sizeof(segment_data[c].thermistor_reading));
                memcpy(segment_data[c].thermistor_value, previous_data[c].thermistor_value, sizeof(segment_data[c].thermistor_value));
            }
        }
    }
	therm_timer.startTimer(THERM_WAIT_TIME); // Set timeout for reading therms
	return NOT_FAULTED; // Read successfully
}

void SegmentInterface::SelectTherm(uint8_t therm)
{
	// Exit if out of range values
	if (therm < 0 || therm > 32) {
		return;
	}
	uint8_t i2c_write_data[NUM_CHIPS][3];
	uint8_t comm_reg_data[NUM_CHIPS][6];
	if (therm <= 8) {
		// Turn off competing multiplexor (therms 9-16)
		for(int chip = 0; chip < NUM_CHIPS; chip++) {
		i2c_write_data[chip][0] = 0x92;
		i2c_write_data[chip][1] = 0x00;
		i2c_write_data[chip][2] = 0x00;
		}
		serializeI2CMsg(i2c_write_data, comm_reg_data);
        pushChipConfigurations();
		LTC6804_wrcomm(NUM_CHIPS, comm_reg_data);
		LTC6804_stcomm(24);

		// Turn on desired thermistor
		for(int chip = 0; chip < NUM_CHIPS; chip++) {
		i2c_write_data[chip][0] = 0x90;
		i2c_write_data[chip][1] = 0x08 + (therm - 1);
		i2c_write_data[chip][2] = 0x00;
		}
		serializeI2CMsg(i2c_write_data, comm_reg_data);
        pushChipConfigurations();
		LTC6804_wrcomm(NUM_CHIPS, comm_reg_data);
		LTC6804_stcomm(24);
	} else if (therm <= 16) {
		// Turn off competing multiplexor (therms 1-8)
		for(int chip = 0; chip < NUM_CHIPS; chip++) {
		i2c_write_data[chip][0] = 0x90;
		i2c_write_data[chip][1] = 0x00;
		i2c_write_data[chip][2] = 0x00;
		}
		serializeI2CMsg(i2c_write_data, comm_reg_data);
        pushChipConfigurations();
		LTC6804_wrcomm(NUM_CHIPS, comm_reg_data);
		LTC6804_stcomm(24);

		// Turn on desired thermistor
		for(int chip = 0; chip < NUM_CHIPS; chip++) {
		i2c_write_data[chip][0] = 0x92;
		i2c_write_data[chip][1] = 0x08 + (therm - 9);
		i2c_write_data[chip][2] = 0x00;
		}
		serializeI2CMsg(i2c_write_data, comm_reg_data);
        pushChipConfigurations();
		LTC6804_wrcomm(NUM_CHIPS, comm_reg_data);
		LTC6804_stcomm(24);
	} else if (therm <= 24) {
		// Turn off competing multiplexor (therms 25-32)
		for(int chip = 0; chip < NUM_CHIPS; chip++) {
		i2c_write_data[chip][0] = 0x96;
		i2c_write_data[chip][1] = 0x00;
		i2c_write_data[chip][2] = 0x00;
		}
		serializeI2CMsg(i2c_write_data, comm_reg_data);
        pushChipConfigurations();
		LTC6804_wrcomm(NUM_CHIPS, comm_reg_data);
		LTC6804_stcomm(24);

		// Turn on desired thermistor
		for(int chip = 0; chip < NUM_CHIPS; chip++) {
		i2c_write_data[chip][0] = 0x94;
		i2c_write_data[chip][1] = 0x08 + (therm - 17);
		i2c_write_data[chip][2] = 0x00;
		}
		serializeI2CMsg(i2c_write_data, comm_reg_data);
        pushChipConfigurations();
		LTC6804_wrcomm(NUM_CHIPS, comm_reg_data);
		LTC6804_stcomm(24);
	} else {
		// Turn off competing multiplexor (therms 17-24)
		for(int chip = 0; chip < NUM_CHIPS; chip++) {
		i2c_write_data[chip][0] = 0x94;
		i2c_write_data[chip][1] = 0x00;
		i2c_write_data[chip][2] = 0x00;
		}
		serializeI2CMsg(i2c_write_data, comm_reg_data);
        pushChipConfigurations();
		LTC6804_wrcomm(NUM_CHIPS, comm_reg_data);
		LTC6804_stcomm(24);

		// Turn on desired thermistor
		for(int chip = 0; chip < NUM_CHIPS; chip++) {
		i2c_write_data[chip][0] = 0x96;
		i2c_write_data[chip][1] = 0x08 + (therm - 25);
		i2c_write_data[chip][2] = 0x00;
		}
		serializeI2CMsg(i2c_write_data, comm_reg_data);
        pushChipConfigurations();
		LTC6804_wrcomm(NUM_CHIPS, comm_reg_data);
		LTC6804_stcomm(24);
	}
}

int8_t SegmentInterface::steinhartEst(uint16_t V)
{
  int i = 0;
  while (V < VOLT_TEMP_CONV[i]) i++;
  return i + MIN_TEMP;
}

void SegmentInterface::disableGPIOPulldowns()
{
	delay(1000);
	// Turn OFF GPIO 1 & 2 pull downs
	pullChipConfigurations();
	for (int c = 0; c < NUM_CHIPS; c++)
	{
		local_config[c][0] |= 0x18;
	}
	pushChipConfigurations();

	pullChipConfigurations();
	Serial.print("Chip CFG:\n");
	for (int c = 0; c < NUM_CHIPS; c++)
	{
		for (int byte = 0; byte < 6; byte++)
		{
		Serial.print(local_config[c][byte], HEX);
		Serial.print("\t");
		}
		Serial.println();
	}
	Serial.println("Done");
}

void SegmentInterface::serializeI2CMsg(uint8_t data_to_write[][3], uint8_t comm_output[][6])
{
    for (int chip = 0; chip < NUM_CHIPS; chip++)
    {
        comm_output[chip][0] = 0x60 | (data_to_write[chip][0] >> 4); // START + high side of B0
        comm_output[chip][1] = (data_to_write[chip][0] << 4) | 0x00; // low side of B0 + ACK
        comm_output[chip][2] = 0x00 | (data_to_write[chip][1] >> 4); // BLANK + high side of B1
        comm_output[chip][3] = (data_to_write[chip][1] << 4) | 0x00; // low side of B1 + ACK
        comm_output[chip][4] = 0x00 | (data_to_write[chip][2] >> 4); // BLANK + high side of B2
        comm_output[chip][5] = (data_to_write[chip][2] << 4) | 0x09; // low side of B2 + STOP & NACK
    }
}