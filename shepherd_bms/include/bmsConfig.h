#ifndef BMS_CONFIG_H
#define BMS_CONFIG_H

// Hardware definition
#define NUM_SEGMENTS        4
#define NUM_CHIPS           NUM_SEGMENTS*2
#define NUM_CELLS_PER_CHIP  9
#define NUM_THERMS_PER_CHIP 32

// Firmware limits
#define MAX_TEMP            65 //degrees C
#define MIN_TEMP            -15 // deg C
#define MAX_VOLT_MEAS       65535
#define MIN_VOLT_MEAS       0

// Boosting Parameters
#define BOOST_TIME          5   // seconds
#define BOOST_RECHARGE_TIME 30  // seconds
#define CONTDCL_MULTIPLIER  1.01

//cell limits
#define MIN_VOLT            2.9
#define MAX_VOLT            4.2
#define MAX_CHARGE_VOLT     4.23
#define MAX_DELTA_V         0.02
#define BAL_MIN_V           4.00
#define MAX_CELL_TEMP       55
#define MAX_CELL_CURR       700 // Amps per BMS cell
#define MAX_CELL_TEMP_BAL   45

// Code settings
#define CHARGE_TIMEOUT      300000 // 5 minutes, may need adjustment
#define VOLT_SAG_MARGIN     0.25 // Volts above the minimum cell voltage we would like to aim for

#endif