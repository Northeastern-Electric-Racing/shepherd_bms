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
    {.id = const_cast<char*>("FAULTS"),    .size = 21}                       

};


/**
 * @brief partitions eeprom addresses given table of data and size
 * 
 * 
 */
void eepromInit();

/**
 * @brief returns the starting address of the passed key
 * 
 * @param key
 * @return int 
 */
int eepromGetIndex(char *key);

/**
 * @brief returns the key at the passed index
 *  
 * 
 */
char *eepromGetKey(int index);

/**
 * @brief logs fault code in eeprom
 * 
 * 
 * @param fault_code 
 */
void logFault(uint32_t fault_code);

/**
 * @brief fills passed data pointer with data from eeprom
 * 
 * @note user is responsible for passing data of correct size
 * @param key
 * @param data
 */
void eepromReadData(char *key, void *data);

void eepromReadData(uint8_t index, void *data);

/**
 * @brief loads eeprom with data from passed pointer
 * 
 * @note user is responsible for passing data of correct size
 * @param key 
 * @param data 
 */
void eepromWriteData(char *key, void *data);

void eepromWriteData(uint8_t index, void *data);
/**
 * @brief reads all stored faults from eeprom
 *
 * 
 * @note this updates a static array of fault codes, should be called before accessing the array
 * @note this function is blocking, and will take a few ms to complete. This is why it is kept seperate from logFault(), 
 *      allwing the user more control as to when to use this
 */

void getFaults();


#endif