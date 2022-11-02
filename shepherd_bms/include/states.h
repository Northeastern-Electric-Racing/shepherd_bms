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

/**
 * @brief Base state used as parent class for all states defined above
 *
 */
class BaseState
{
    public:
        BaseState(){}

        virtual ~BaseState(){}

        BMSState_t state;

        virtual void handleState(){}

        virtual BaseState transition(BaseState nextState){}
};

class BootState: public BaseState
{
    public:
        BootState();

        ~BootState();

        void handleState();

        BaseState transition(BaseState nextState);
};


class IdleState: public BaseState
{
    public:
        IdleState();

        ~IdleState();

        void handleState();

        BaseState trasition(BaseState nextState);
};

class ActiveState: public BaseState
{
    public:
        ActiveState();

        ~ActiveState();

        void handleState();

        BaseState transition(BaseState nextState);
};

class ChargingState: public BaseState
{
    public:
        ChargingState();

        ~ChargingState();

        void handleState();

        BaseState transition(BaseState nextState);
};

class FaultedState: public BaseState
{
    public:
        FaultedState();

        ~FaultedState();

        void handleState();

        BaseState transition(BaseState nextState);
};

#endif