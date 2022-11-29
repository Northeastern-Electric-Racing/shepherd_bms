#ifndef BMS_STATES_H
#define BMS_STATES_H

typedef enum
{
    BOOT,       //State when BMS first starts up, used to initialize everything that needs configuring
    IDLE,       //State when car is not on/BMS is not really doing anything
    ACTIVE,     //State when car is actively discharging/car is on (Draining battery)
    CHARGING,   //State when car is on and is charging (Filling battery)
    FAULTED     //State when BMS has detected a catastrophic fault and we need to hault operations
}BMSState_t;

class StateMachine;

/**
 * @brief Base state used as parent class for all states defined below
 *
 */
class BaseState
{
    private:
        StateMachine *bmsDirector;

    public:
        BMSState_t state;

        BaseState(){}

        virtual ~BaseState(){}

        virtual void handleState(){}

        virtual void initialize(){}

        friend bool operator==(const BaseState &lhs, const BaseState &rhs);
};

class StateMachine
{
    private:
        BaseState currentState;

        void switchState(BaseState &nextState);

    public:
        StateMachine();

        virtual ~StateMachine();

        void handleState();

        void requestTransition(BaseState &nextState);
};

class BootState: public BaseState
{
    public:
        const BMSState_t state = BOOT;

        BootState(){}

        BootState(StateMachine &stateMachine);

        ~BootState(){}

        void handleState();

        void initialize();
};


class IdleState: public BaseState
{
    public:
        const BMSState_t state = IDLE;

        IdleState(){}

        IdleState(StateMachine &stateMachine);

        ~IdleState(){}

        void handleState();

        void initialize();
};

class ActiveState: public BaseState
{
    public:
        const BMSState_t state = ACTIVE;

        ActiveState(){}

        ActiveState(StateMachine &stateMachine);

        ~ActiveState(){}

        void handleState();

        void initialize();
};

class ChargingState: public BaseState
{
    public:
        const BMSState_t state = CHARGING;

        ChargingState(){}

        ChargingState(StateMachine &stateMachine);

        ~ChargingState(){}

        void handleState();

        void initialize();
};

class FaultedState: public BaseState
{
    public:
        const BMSState_t state = FAULTED;

        FaultedState(){}

        FaultedState(StateMachine &stateMachine);

        ~FaultedState(){}

        void handleState();

        void initialize();
};

#endif