#ifndef BMS_COMFIG_H
#define BMS_CONFIG_H

#define NUM_SEGMENTS        4
#define NUM_CHIPS           NUM_SEGMENTS*2
#define NUM_CELLS_PER_CHIP  9
#define NUM_THERMS_PER_CHIP 32

// Hardware limits
#define MAX_TEMP            65 //degrees C
#define MIN_TEMP            -5 // deg C

#define MAX_VOLT_MEAS       65535
#define MIN_VOLT_MEAS       0

#endif