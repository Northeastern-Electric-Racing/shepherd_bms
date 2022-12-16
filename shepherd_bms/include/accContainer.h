#ifndef ACC_CONTAINER_H
#define ACC_CONTAINER_H

#include <nerduino.h>
#include "datastructs.h"

struct frame
{
    AccumulatorData_t data;
    frame* next;
};


/**
 * @brief Resizable linked list queue for performing historical analysis
 * 
 */
class Accumulator
{
    private:
        frame* read = nullptr;
        frame* write = nullptr;
        
        frame* head = nullptr;
        uint16_t size = 0;
        uint16_t numNodes = 0;
    
    public:
        Accumulator();

        Accumulator(uint16_t newSize);

        ~Accumulator();

        void push(AccumulatorData_t data);

        void resize(uint16_t newSize);

        //Pointer to the data in the head node
        AccumulatorData_t *currentData = &(read->data);
};

#endif