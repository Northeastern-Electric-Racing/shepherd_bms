#ifndef EEPROMDIRECTORY_H
#define EEPROMDIRECTORY_H

#include <nerduino.h>
#include "bmsConfig.h"

/* index 0 = newest, index 4 = oldest */
static uint32_t eeprom_faults[NUM_EEPROM_FAULTS];

struct eeprom_partition 
{
    char *id;           /*  key  */
    uint16_t size;      /* bytes */
    uint8_t offset;     /* start address */

};

struct eeprom_partition eeprom_data[NUM_EEPROM_ITEMS]
{
/*  ____________KEY________________         _BYTES_   */
    {.id = const_cast<char*>("ROOT"),      .size = 1},
    {.id = const_cast<char*>("FAULTS"),    .size = 20}                       

};


/**
 * @brief partitions eeprom addresses given table of data and size
 * 
 * 
 */
void eepromInit();

/**
 * @brief finds the starting address of the passed key
 * 
 * @param key
 * @return int 
 */
int eepromGetIndex(char *key);

/**
 * @brief logs fault code in eeprom
 * 
 * 
 * @param fault_code 
 */
void logFault(uint32_t fault_code);

/**
 * @brief reads all stored faults from eeprom
 *
 * 
 * @note this updates a static array of fault codes
 */

void updateFaults();


#endif