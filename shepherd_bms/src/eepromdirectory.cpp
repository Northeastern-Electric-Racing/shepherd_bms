#include "eepromdirectory.h"

void eepromInit()
{   
    // Initialize EEPROM addresses given data and length

    int i = 1;
    int offset = 0;

    /* initialize root offset to zero */
    eeprom_data[0].offset = EEPROM_ROOT_ADDR;

    /* continue through array, setting offsets */
    while (eeprom_data[i].id != NULL)
    {
        offset += eeprom_data[i-1].size;
        eeprom_data[i].offset = offset;
        i++;
    }
}

int eepromGetIndex(char *key)
{
    /* find the index of the key in the eeprom_data array */
    int i = 0;
    while (eeprom_data[i].id != NULL)
    {
        if (eeprom_data[i].id == key)
        {
            return eeprom_data[i].offset;
        }

        i++;
    }
    return -1;
}

void logFault(uint32_t fault_code)
{
    /* the most recent index is stored in the first byte of the fault partition */
    uint8_t index = EEPROM.read(eepromGetIndex(const_cast<char*>("FAULTS"))); // TODO will need update with new eeprom driver

    /* if the index is at the end of the partition, wrap around (currently store 5 faults, so max = 5 + offset) */
    if (index == NUM_EEPROM_FAULTS + eeprom_data[eepromGetIndex(const_cast<char*>("FAULTS"))].offset)
    {
        index = eeprom_data[eepromGetIndex(const_cast<char*>("FAULTS"))].offset;
    }
    else
    {
        index++;
    }

    /* write the fault code to the new index */
    EEPROM.put(index, fault_code);   // TODO will need update with new eeprom driver
}

void updateFaults()
{
    /* the most recent index is stored in the first byte of the fault partition */
    uint8_t index = EEPROM.read(eepromGetIndex(const_cast<char*>("FAULTS"))); // TODO will need update with new eeprom driver

    /* read and store the faults */

    int currIter = 0;
    while (currIter <= NUM_EEPROM_FAULTS)
    {
        EEPROM.get(index, eeprom_faults[currIter]); // TODO will need update with new eeprom driver
        currIter++;

        /* if the index is at the end of the partition, wrap around (currently store 5 faults, so max = 5 + offset) */
        if (index == NUM_EEPROM_FAULTS + eeprom_data[eepromGetIndex(const_cast<char*>("FAULTS"))].offset)
        {                             
            index = eeprom_data[eepromGetIndex(const_cast<char*>("FAULTS"))].offset;
        }
        
        else
        {
            index++;
        }
    }

}
