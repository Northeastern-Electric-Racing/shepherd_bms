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

char* eepromGetKey(int index)
{
    /* find the key at the index in the eeprom_data array */
    int i = 0;
    while (eeprom_data[i].id != NULL)
    {
        if (eeprom_data[i].offset == index)
        {
            return eeprom_data[i].id;
        }

        i++;
    }
    return NULL;
}



void eepromReadData(char *key, void *data)
{
    /* read data from eeprom given key */
    int index = eepromGetIndex(key);
    EEPROM.get(index, data); // TODO will need update with new eeprom driver

}

void eepromReadData(uint8_t index, void *data)
{
    /* read data from eeprom given index */
    EEPROM.get(index, data); // TODO will need update with new eeprom driver
}

void eepromWriteData(char *key, void *data)
{
    /* write data to eeprom given key and size of data*/
    int index = eepromGetIndex(key);
    EEPROM.put(index, data); // TODO will need update with new eeprom driver
}

void eepromWriteData(uint8_t index, void *data)
{
    /* write data to eeprom given index and size of data */
    EEPROM.put(index, data); // TODO will need update with new eeprom driver
}

void logFault(uint32_t *fault_code)
{
    /* the most recent index is stored in the first byte of the fault partition */
    uint8_t* index;
    eepromReadData("FAULTS", index);

    uint8_t startIndex =  eeprom_data[eepromGetIndex(const_cast<char*>("FAULTS"))].offset;
    uint8_t size = eeprom_data[eepromGetIndex(const_cast<char*>("FAULTS"))].size;

    /* if the index is at the end of the partition, wrap around (currently store 5 faults, so max = 5 + offset) */
    if (*index == size + startIndex - 3)
    {
        /* first byte of partition is the index of the most recent fault, faults begin at second byte */
        *index = startIndex + 1;
    }
    else
    {
        *index += 4;
    }

    /* write the fault code to the new index */
   eepromWriteData(*index, fault_code);
}

void getFaults()
{
    /* the most recent index is stored in the first byte of the fault partition */
    uint8_t* index;
    eepromReadData("FAULTS", index);

    uint8_t startIndex =  eeprom_data[eepromGetIndex(const_cast<char*>("FAULTS"))].offset;
    uint8_t size = eeprom_data[eepromGetIndex(const_cast<char*>("FAULTS"))].size;

    /* read and store the faults */

    int currIter = 0;
    while (currIter < NUM_EEPROM_FAULTS)
    {
        eepromReadData(*index, &eeprom_faults[currIter]);
        currIter++;

        /* if the index is at the end of the partition, wrap around (5 faults * 4 bytes per fault + offset - 3  for start of fault) */
        if (*index == size + startIndex - 3)
        {                             
            /* first byte of partition is the index of the most recent fault, faults begin at second byte */
            *index = startIndex + 1;
        }
        
        else
        {
            /* 4 bytes per fault */
            *index += 4;
        }
    }

}
