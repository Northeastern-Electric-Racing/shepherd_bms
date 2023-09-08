#ifndef BMS_STATES_H
#define BMS_STATES_H

#include "datastructs.h"
#include "segment.h"
#include "compute.h"
#include "analyzer.h"

class StateMachine
{
    private:

        AccumulatorData_t *prevAccData;
        uint32_t bmsFault = FAULTS_CLEAR;
    

        tristate_timer overCurr_tmr;
        tristate_timer overChgCurr_tmr;
        tristate_timer underVolt_tmr;
        tristate_timer overVoltCharge_tmr;
        tristate_timer overVolt_tmr;
        tristate_timer lowCell_tmr;
        tristate_timer highTemp_tmr;

        tristate_timer prefaultOverCurr_tmr;
        tristate_timer prefaultLowCell_tmr;

        Timer chargeTimeout;
        tristate_timer chargeCutoffTime;

        Timer prefaultCANDelay1; // low cell
        Timer prefaultCANDelay2; // dcl

      

        Timer canMsgTimer;

        bool enteredFaulted = false;


        Timer chargeMessageTimer;
        static const uint16_t CHARGE_MESSAGE_WAIT = 250; //ms

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
        uint32_t faultReturn(AccumulatorData_t *accData);

        /**
         * @brief Used in parellel to faultReturn(), calculates each fault to append the fault status
         * 
         * @param index
         * @return fault_code
         */
        uint32_t faultEval(fault_eval index);

        /**
         * @brief Used to check for faults immedietly before reaching faulted state, allows for easier handling
         * 
         * @param bmsdata 
         */
        void preFaultCheck(AccumulatorData_t *bmsdata);


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