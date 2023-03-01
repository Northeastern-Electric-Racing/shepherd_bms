#ifndef BMS_STATES_H
#define BMS_STATES_H

#include "datastructs.h"
#include "analyzer.h"
#include "compute.h"
#include "segment.h"

typedef enum
{
    BOOT_STATE,       //State when BMS first starts up, used to initialize everything that needs configuring
    READY_STATE,      //State when car is not on/BMS is not really doing anything
    CHARGING_STATE,   //State when car is on and is charging (Filling battery)
    FAULTED_STATE,    //State when BMS has detected a catastrophic fault and we need to hault operations
    NUM_STATES
}BMSState_t;

class StateMachine
{
    private:

        AccumulatorData_t *prevAccData;
        uint32_t bmsFault;
        uint16_t overVoltCount;
        uint16_t underVoltCount;
        uint16_t overCurrCount;
        uint16_t chargeOverVolt;
        uint16_t overChgCurrCount;
        uint16_t lowCellCount;
        static Timer chargeMessageTimer;
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

    public:
        StateMachine();

        virtual ~StateMachine();

        void handleState(AccumulatorData_t *bmsdata);

        BMSState_t currentState;
};

#endif