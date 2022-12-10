#include "accContainer.h"

Accumulator::Accumulator()
{
    head = NULL;
}

void Accumulator::insert(AccumulatorData_t data){
    frame *newNode = new frame;
    newNode->data = data;

    if (head == NULL){
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
}