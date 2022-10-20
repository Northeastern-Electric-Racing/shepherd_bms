#ifndef COMPUTE_H
#define COMPUTE_H

using namespace std;

class ComputeInterface
{
    private:
        int fanSpeed;

    public: 
        ComputeInterface();

        ~ComputeInterface();

        void enableCharging();

        void setFanSpeed(int newFanSpeed);

        int getPackCurrent();

};
#endif