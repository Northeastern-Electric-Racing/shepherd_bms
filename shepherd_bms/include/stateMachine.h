#ifndef BMS_STATES_H
#define BMS_STATES_H

#include "datastructs.h"
#include "segment.cpp"
#include "compute.cpp"
#include "analyzer.h"

typedef enum
{
    BOOT_STATE,       //State when BMS first starts up, used to initialize everything that needs configuring
    READY_STATE,      //State when car is not on/BMS is not really doing anything
    CHARGING_STATE,   //State when car is on and is charging (Filling battery)
    FAULTED_STATE,    //State when BMS has detected a catastrophic fault and we need to hault operations
    NUM_STATES
}BMSState_t;

uint32_t bmsFault = FAULTS_CLEAR;

uint16_t overVoltCount = 0;
uint16_t underVoltCount = 0;
uint16_t overCurrCount = 0;
uint16_t chargeOverVolt = 0;
uint16_t overChgCurrCount = 0;
uint16_t lowCellCount = 0;

static Timer chargeMessageTimer;
static const uint16_t CHARGE_MESSAGE_WAIT = 250; //ms


class StateMachine
{
    private:

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

        void requestTransition(BMSState_t nextState);

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
        * @brief Algorithm behind determining which cells we want to balance
        * @note Directly interfaces with the segments
        * 
        * @param bms_data 
        */
        void balanceCells(AccumulatorData_t *bms_data);


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

        void broadcastCurrentLimit(AccumulatorData_t *bmsdata);

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

        BMSState_t currentState;
};

#endif