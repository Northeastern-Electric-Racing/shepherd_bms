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
    localConfig[c][0] = 0xF8;
    localConfig[c][1] = 0x19; // VUV = 0x619 = 1561 -> 2.4992V
    localConfig[c][2] = 0x06; // VOV = 0xA60 = 2656 -> 4.2496V
    localConfig[c][3] = 0xA6;
    localConfig[c][4] = 0x00;
    localConfig[c][5] = 0x00;
    }
    pushChipConfigurations();
}

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
    static const uint16_t dischargeAllCommand = 0xFFFF >> (16-NUM_CELLS_PER_CHIP); // Making the discharge command all 1's for all cells per chip

    if(balanceEnable)
    {
		for(int c = 0; c < NUM_CHIPS; c++)
        {
            configureDischarge(c, dischargeAllCommand);
			dischargeCommands[c] = dischargeAllCommand;
        }
        pushChipConfigurations();
    }
    else
    {
        for (int c = 0; c < NUM_CHIPS; c++)
        {
            configureDischarge(c, 0);
			dischargeCommands[c] = 0;
        }
        pushChipConfigurations();
    }
}

/**
 * @todo Revisit after testing 
 */
void SegmentInterface::enableBalancing(uint8_t chipNum, uint8_t cellNum, bool balanceEnable)
{
    pullChipConfigurations();

	if(balanceEnable) 
		dischargeCommands[chipNum] |= (1 << cellNum);
	else
		dischargeCommands[chipNum] &= ~(1 << cellNum);

    configureDischarge(chipNum, dischargeCommands[chipNum]);

    pushChipConfigurations();
}

void SegmentInterface::configureBalancing(bool dischargeConfig[NUM_CHIPS][NUM_CELLS_PER_CHIP])
{
    for(int c = 0; c < NUM_CHIPS; c++)
    {
        for(int cell = 0; cell < NUM_CELLS_PER_CHIP; cell++)
        {
            if(dischargeConfig[c][cell]) 
				dischargeCommands[c] |= 1 << cell;
			else 
				dischargeCommands[c] &= ~(1 << cell);
        }

        configureDischarge(c, dischargeCommands[c]);
    }
    pushChipConfigurations();
}

bool SegmentInterface::isBalancing(uint8_t chipNum, uint8_t cellNum)
{
    //If the cell is one of the first 8, check the 4th register
    if(cellNum < 8)
    {
        return localConfig[chipNum][4] & (1 << cellNum);
    }
    //If the cell number is greater than 8, check the 5th register
    else
    {
        return localConfig[chipNum][5] & (1 << (cellNum - 8));
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
            if(localConfig[c][4] & (1 << cell)) return true;
        }

        //Reading from the 5th config register
        for(int cell = 0; cell < 4; cell++)
        {
            if(localConfig[c][5] & (1 << (cell))) return true;
        }
    }

    return false;
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
        for(uint8_t c=0; c<NUM_CHIPS; c++)
        {
            memcpy(segmentData[c].voltageReading, previousData[c].voltageReading, sizeof(segmentData[c].voltageReading));
        }
        return voltageError;
    }

    uint16_t segmentVoltages[NUM_CHIPS][12];   

    pushChipConfigurations();
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
    // If polled too soon, just copy existing values from memory
	if (!thermTimer.isTimerExpired())
	{
		for(uint8_t c=0; c<NUM_CHIPS; c++)   
        {
            memcpy(segmentData[c].thermistorReading, previousData[c].thermistorReading, sizeof(segmentData[c].thermistorReading));
        }
        return voltageError;
	}

    uint16_t rawTempVoltages[NUM_CHIPS][6];

    // Rotate through all thermistor pairs (we can poll two at once)
    for (int therm = 1; therm <= 16; therm++) // therm range = 11 = ~12, two at a time = 6 iterations
	{

        // Sets multiplexors to select thermistors
        if (therm >= START_THERM && therm < END_THERM)
        {
            SelectTherm(therm);
        }
        
        if ((therm +16) >= START_THERM && (therm+16) < END_THERM)
        {
            SelectTherm(therm + 16);
        }

		
		pushChipConfigurations();
        LTC6804_adax(); // Run ADC for AUX (GPIOs and refs)
        LTC6804_rdaux(0, NUM_CHIPS, rawTempVoltages); // Fetch ADC results from AUX registers

        if (therm >= START_THERM && therm < END_THERM)
        {
            for (int c = 0; c < NUM_CHIPS; c++) 
		    {
                // Get current temperature LUT. Voltage is adjusted to account for 5V reg fluctuations (index 2 is a reading of the ADC 5V ref)
                segmentData[c].thermistorReading[therm - START_THERM] = steinhartEst(rawTempVoltages[c][0] * (float(rawTempVoltages[c][2]) / 50000) + VOLT_TEMP_CALIB_OFFSET);

                // Directly update for a set time from start up due to therm voltages needing to settle
                if (thermSettleTime < THERM_AVG * 10) 
                {
                    segmentData[c].thermistorValue[therm - START_THERM] = segmentData[c].thermistorReading[therm - 1];
                    thermSettleTime++;
                }   else 
                {
                    // We need to investigate this. Very sloppy
                    // Discard if reading is 33C
                    if (segmentData[c].thermistorReading[therm - START_THERM] != 33) 
                    {
                        // If measured value is larger than current "averaged" value, increment value
                        if (segmentData[c].thermistorReading[therm - START_THERM] > segmentData[c].thermistorValue[therm - 1]) 
                        {
                        segmentData[c].thermistorValue[therm - START_THERM]++;
                    // If measured value is smaller than current "averaged" value, decrement value
                        } else if (segmentData[c].thermistorReading[therm - START_THERM] < segmentData[c].thermistorValue[therm - 1]) 
                        {
                            segmentData[c].thermistorValue[therm - START_THERM]--;
                        }
                    }
                }
            }
        }

        // same as a bove but for upper therms
        if ((therm +16) >= START_THERM && (therm+16) < END_THERM)
        {
            for (int c = 0; c < NUM_CHIPS; c++) 
		    {
                // Get current temperature LUT. Voltage is adjusted to account for 5V reg fluctuations (index 2 is a reading of the ADC 5V ref)
                segmentData[c].thermistorReading[(therm+16) - START_THERM] = steinhartEst(rawTempVoltages[c][0] * (float(rawTempVoltages[c][2]) / 50000) + VOLT_TEMP_CALIB_OFFSET);

                // Directly update for a set time from start up due to therm voltages needing to settle
                if (thermSettleTime < THERM_AVG * 10) 
                {
                    segmentData[c].thermistorValue[(therm+16) - START_THERM] = segmentData[c].thermistorReading[therm - 1];
                    thermSettleTime++;
                }   else 
                {
                    // We need to investigate this. Very sloppy
                    // Discard if reading is 33C
                    if (segmentData[c].thermistorReading[(therm+16) - START_THERM] != 33) 
                    {
                        // If measured value is larger than current "averaged" value, increment value
                        if (segmentData[c].thermistorReading[(therm+16) - START_THERM] > segmentData[c].thermistorValue[therm - 1]) 
                        {
                        segmentData[c].thermistorValue[(therm+16) - START_THERM]++;
                    // If measured value is smaller than current "averaged" value, decrement value
                        } else if (segmentData[c].thermistorReading[(therm+16) - START_THERM] < segmentData[c].thermistorValue[therm - 1]) 
                        {
                            segmentData[c].thermistorValue[(therm+16) - START_THERM]--;
                        }
                    }
                
                    
                }
            }
        }
        
    }
	thermTimer.startTimer(THERM_WAIT_TIME); // Set timeout for reading therms
	return NOT_FAULTED; // Read successfully
}

void SegmentInterface::SelectTherm(uint8_t therm)
{
	// Exit if out of range values
	if (therm < 0 || therm > 32) {
		return;
	}
	uint8_t i2cWriteData[NUM_CHIPS][3];
	uint8_t commRegData[NUM_CHIPS][6];
	if (therm <= 8) {
		// Turn off competing multiplexor (therms 9-16)
		for(int chip = 0; chip < NUM_CHIPS; chip++) {
		i2cWriteData[chip][0] = 0x92;
		i2cWriteData[chip][1] = 0x00;
		i2cWriteData[chip][2] = 0x00;
		}
		serializeI2CMsg(i2cWriteData, commRegData);
        pushChipConfigurations();
		LTC6804_wrcomm(NUM_CHIPS, commRegData);
		LTC6804_stcomm(24);

		// Turn on desired thermistor
		for(int chip = 0; chip < NUM_CHIPS; chip++) {
		i2cWriteData[chip][0] = 0x90;
		i2cWriteData[chip][1] = 0x08 + (therm - 1);
		i2cWriteData[chip][2] = 0x00;
		}
		serializeI2CMsg(i2cWriteData, commRegData);
        pushChipConfigurations();
		LTC6804_wrcomm(NUM_CHIPS, commRegData);
		LTC6804_stcomm(24);
	} else if (therm <= 16) {
		// Turn off competing multiplexor (therms 1-8)
		for(int chip = 0; chip < NUM_CHIPS; chip++) {
		i2cWriteData[chip][0] = 0x90;
		i2cWriteData[chip][1] = 0x00;
		i2cWriteData[chip][2] = 0x00;
		}
		serializeI2CMsg(i2cWriteData, commRegData);
        pushChipConfigurations();
		LTC6804_wrcomm(NUM_CHIPS, commRegData);
		LTC6804_stcomm(24);

		// Turn on desired thermistor
		for(int chip = 0; chip < NUM_CHIPS; chip++) {
		i2cWriteData[chip][0] = 0x92;
		i2cWriteData[chip][1] = 0x08 + (therm - 9);
		i2cWriteData[chip][2] = 0x00;
		}
		serializeI2CMsg(i2cWriteData, commRegData);
        pushChipConfigurations();
		LTC6804_wrcomm(NUM_CHIPS, commRegData);
		LTC6804_stcomm(24);
	} else if (therm <= 24) {
		// Turn off competing multiplexor (therms 25-32)
		for(int chip = 0; chip < NUM_CHIPS; chip++) {
		i2cWriteData[chip][0] = 0x96;
		i2cWriteData[chip][1] = 0x00;
		i2cWriteData[chip][2] = 0x00;
		}
		serializeI2CMsg(i2cWriteData, commRegData);
        pushChipConfigurations();
		LTC6804_wrcomm(NUM_CHIPS, commRegData);
		LTC6804_stcomm(24);

		// Turn on desired thermistor
		for(int chip = 0; chip < NUM_CHIPS; chip++) {
		i2cWriteData[chip][0] = 0x94;
		i2cWriteData[chip][1] = 0x08 + (therm - 17);
		i2cWriteData[chip][2] = 0x00;
		}
		serializeI2CMsg(i2cWriteData, commRegData);
        pushChipConfigurations();
		LTC6804_wrcomm(NUM_CHIPS, commRegData);
		LTC6804_stcomm(24);
	} else {
		// Turn off competing multiplexor (therms 17-24)
		for(int chip = 0; chip < NUM_CHIPS; chip++) {
		i2cWriteData[chip][0] = 0x94;
		i2cWriteData[chip][1] = 0x00;
		i2cWriteData[chip][2] = 0x00;
		}
		serializeI2CMsg(i2cWriteData, commRegData);
        pushChipConfigurations();
		LTC6804_wrcomm(NUM_CHIPS, commRegData);
		LTC6804_stcomm(24);

		// Turn on desired thermistor
		for(int chip = 0; chip < NUM_CHIPS; chip++) {
		i2cWriteData[chip][0] = 0x96;
		i2cWriteData[chip][1] = 0x08 + (therm - 25);
		i2cWriteData[chip][2] = 0x00;
		}
		serializeI2CMsg(i2cWriteData, commRegData);
        pushChipConfigurations();
		LTC6804_wrcomm(NUM_CHIPS, commRegData);
		LTC6804_stcomm(24);
	}
}

uint8_t SegmentInterface::steinhartEst(uint16_t V)
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
		localConfig[c][0] |= 0x18;
	}
	pushChipConfigurations();

	pullChipConfigurations();
	Serial.print("Chip CFG:\n");
	for (int c = 0; c < NUM_CHIPS; c++)
	{
		for (int byte = 0; byte < 6; byte++)
		{
		Serial.print(localConfig[c][byte], HEX);
		Serial.print("\t");
		}
		Serial.println();
	}
	Serial.println("Done");
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