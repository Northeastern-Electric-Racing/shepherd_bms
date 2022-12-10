#ifndef ACC_CONTAINER_H
#define ACC_CONTAINER_H

#include <nerduino.h>
#include "datastructs.h"

struct frame
{
    AccumulatorData_t data;
    frame* next;
};

class Accumulator{
    public:

        Accumulator();

        ~Accumulator();

        void insert(AccumulatorData_t data);

        frame* head;
};

#endif