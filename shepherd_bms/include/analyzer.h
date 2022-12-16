#ifndef ANALYZER_H
#define ANALYZER_H

#include <nerduino.h>
#include "datastructs.h"
#include "segment.h"

//The maximum rate we can push at is limited by how fast the slowest action takes, which is sampling thermistors
#define SAMPLING_INTERVAL THERM_WAIT_TIME

struct node
{
    AccumulatorData_t data;
    node* next = nullptr;
};

/**
 * @brief Resizable linked list queue for performing historical analysis
 * 
 */
class Analyzer
{
    private:
        node* head = nullptr;
        node* tail = nullptr;

        uint16_t size = 0;
        uint16_t numNodes = 0;
    
    public:
        Analyzer();

        Analyzer(uint16_t newSize);

        ~Analyzer();

        void push(AccumulatorData_t data);

        void resize(uint16_t newSize);

        //Pointer to the data in the head node
        AccumulatorData_t *currentData = &(head->data);
};

#endif