#include "accContainer.h"

Accumulator::Accumulator(){}

Accumulator::Accumulator(uint16_t newSize)
{
    resize(newSize);
}

Accumulator::~Accumulator(){}

void Accumulator::push(AccumulatorData_t data)
{
    //if it isn't full, we need to push and not pop

    //if it is full, we need to push AND pop
    
    frame *newNode = new frame;
    newNode->data = data;

    //if list is empty
    if (head == nullptr)
    {
        head = newNode;
        newNode->next = head;
    }
    else{
        frame *temp = head;
        newNode->next = temp;
        while (temp->next != head){
            temp = temp->next;
        }
        temp->next = newNode;
        head = newNode;
    }

    currentData = &(head->data);
}

void Accumulator::resize(uint16_t newSize)
{
    size = newSize;

    //if the queue is larger than the newSize, then delete the end nodes
}