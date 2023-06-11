#ifndef BMS_STATES_H
#define BMS_STATES_H

#include "datastructs.h"
#include "segment.h"
#include "compute.h"
#include "analyzer.h"


typedef enum
{
    BOOT_STATE,       //State when BMS first starts up, used to initialize everything that needs configuring
    READY_STATE,      //State when car is not on/BMS is not really doing anything
    CHARGING_STATE,   //State when car is on and is charging (Filling battery)
    FAULTED_STATE,    //State when BMS has detected a catastrophic fault and we need to hault operations
    NUM_STATES

} BMSState_t;

typedef enum
{
	BEFORE_TIMER_START,
	DURING_FAULT_EVAL

} FaultEvalState;

typedef enum
{
    GT = 0, // fault if {data} greater than {threshold}
    LT = 1, // fault if {data} less than {threshold}
    GE = 2, // fault if {data} greater than or equal to {threshold}
    LE = 3, // fault if {data} less than or equal to {threshold}
    EQ = 4, // fault if {data} equal to {threshold}
    NOP = 5 // no operation, use for single threshold faults

} FaultEvalType;

//timers and fault states for each fault
struct fault_timer
{
	FaultEvalState faultEvalState = BEFORE_TIMER_START;
	Timer faultTimer;
};

struct fault_eval
{
    char* id; // how tf have we made it this far without the damn string library included
    fault_timer timer;

    int lim_1;
    FaultEvalType optype_1;
    int data_1;

    int lim_2 = 0;            // optional second threshold
    FaultEvalType optype_2 = NOP;
    int data_2 = 0;

    bool is_faulted = false;


};


class StateMachine
{
    private:

        AccumulatorData_t *prevAccData;
        uint32_t bmsFault = FAULTS_CLEAR;

        fault_timer overCurr_tmr;
        fault_timer overChgCurr_tmr;
        fault_timer underVolt_tmr;
        fault_timer overVoltCharge_tmr;
        fault_timer overVolt_tmr;
        fault_timer lowCell_tmr;
        fault_timer highTemp_tmr;

        fault_timer prefaultOverCurr;
        fault_timer prefaultLowCell;

        Timer chargeTimeout;
        fault_timer chargeCutoffTime;

        Timer prefaultCANDelay1; // low cell
        Timer prefaultCANDelay2; // dcl

      

        Timer canMsgTimer;

        bool enteredFaulted = false;


        Timer chargeMessageTimer;
        static const uint16_t CHARGE_MESSAGE_WAIT = 250; //ms

        struct fault_eval fault_table[8] = 
        {
          // ___________FAULT ID____________  ____________TIMER____________   ___________THRESHOLD___________  ___________OPERATOR___________ ___________DATA___________, ___________OPERATOR___________, ___________DATA___________
            {.id = "Discharge Limit",         .timer = overCurr_tmr,       .lim_1 = macro, .optype_1 = GT,         .data_1 = stuff}, // Discharge Limit Enforcement
            {.id = "Over Current",            .timer = overChgCurr_tmr,    .lim_1 = macro, .optype_1 = type,       .data_1 = stuff}, // Over Current (charge)
            {.id = "Cell Voltage Low",        .timer = underVolt_tmr,      .lim_1 = macro, .optype_1 = type,       .data_1 = stuff}, // Low Cell Voltage
            {.id = "Charge Cell Voltage High",.timer = overVoltCharge_tmr, .lim_1 = macro, .optype_1 = type,       .data_1 = stuff}, // High Cell (charging) Voltage
            {.id = "Cell Voltage High",       .timer = overVolt_tmr,       .lim_1 = macro, .optype_1 = type,       .data_1 = stuff}, // High Cell (not charging) Voltage
            {.id = "High Temp",               .timer = highTemp_tmr,       .lim_1 = macro, .optype_1 = type,       .data_1 = stuff}, // High Temperature
            {.id = "Extremely Low Voltage",   .timer = lowCell_tmr,        .lim_1 = macro, .optype_1 = type,       .data_1 = stuff}, // Extremely Low Cell Voltage

            NULL
        };


        const bool validTransitionFromTo[NUM_STATES][NUM_STATES] =
        {
            //BOOT,     READY,      CHARGING,   FAULTED
            {true,      true,       false,      true}, // BOOT
            {false,     true,       true,       true}, // READY
            {false,     true,       true,       true}, // CHARGING
            {true,      false,      false,      true}  // FAULTED
        };

        typedef void (StateMachine::*HandlerFunction_t)(AccumulatorData_t *bmsdata);
        typedef void (StateMachine::*InitFunction_t)();

        void requestTransition(BMSState_t next_state);

        void handleBoot(AccumulatorData_t *bmsdata);

        void handleReady(AccumulatorData_t *bmsdata);

        void handleCharging(AccumulatorData_t *bmsdata);

        void handleFaulted(AccumulatorData_t *bmsdata);

        void initBoot();

        void initReady();

        void initCharging();

        void initFaulted();

        const InitFunction_t initLUT[NUM_STATES] =
        {
            &StateMachine::initBoot,
            &StateMachine::initReady,
            &StateMachine::initCharging,
            &StateMachine::initFaulted
        };

        const HandlerFunction_t handlerLUT[NUM_STATES] =
        {
            &StateMachine::handleBoot,
            &StateMachine::handleReady,
            &StateMachine::handleCharging,
            &StateMachine::handleFaulted
        };


        /**
        * @brief Returns if we want to balance cells during a particular frame
        *
        * @param bmsdata
        * @return true
        * @return false
        */
        bool balancingCheck(AccumulatorData_t *bmsdata);

        /**
        * @brief Returns if we want to charge cells during a particular frame
        *
        * @param bmsdata
        * @return true
        * @return false
        */
        bool chargingCheck(AccumulatorData_t *bmsdata);


        /**
        * @brief Returns any new faults or current faults that have come up
        * @note Should be bitwise OR'ed with the current fault status
        *
        * @param accData
        * @return uint32_t
        */
        uint32_t faultCheck(AccumulatorData_t *accData);


    public:
        StateMachine();

        virtual ~StateMachine();

        void handleState(AccumulatorData_t *bmsdata);

        BMSState_t current_state;

        uint32_t previousFault = 0;
};

        /**
        * @brief Algorithm behind determining which cells we want to balance
        * @note Directly interfaces with the segments
        *
        * @param bms_data
        */
        void balanceCells(AccumulatorData_t *bms_data);

        void broadcastCurrentLimit(AccumulatorData_t *bmsdata);

#endif