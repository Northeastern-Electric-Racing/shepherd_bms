#ifndef BMS_CONFIG_H
#define BMS_CONFIG_H

#define NUM_SEGMENTS        4
#define NUM_CHIPS           NUM_SEGMENTS*2
#define NUM_CELLS_PER_CHIP  9
#define NUM_THERMS_PER_CHIP 32

// Hardware limits
#define MAX_TEMP            65 //degrees C
#define MIN_TEMP            -5 // deg C

//cell limits
#define MIN_VOLT            2.9
#define MAX_VOLT            4.2
#define MAX_DELTA_V         0.02
#define BAL_MIN_V           4.00
#define MAX_CELL_TEMP       55
#define CHARGE_TIMEOUT      600000 // 10 minutes, may need adjustment

#define MAX_VOLT_MEAS       65535
#define MIN_VOLT_MEAS       0

#define VOLT_SAG_MARGIN     0.25 // Volts above the minimum cell voltage we would like to aim for

#define MAX_CELL_CURR       700 // Amps per BMS cell

#endif