#include "analyzer.h"

Analyzer::Analyzer(){}

Analyzer::Analyzer(uint16_t newSize)
{
    resize(newSize);
}

Analyzer::~Analyzer()
{
    while(tail != nullptr)
    {
        //remove the last item
        node *tmp = tail;
        tail = tail->next;
        delete tmp;
        tmp = nullptr;
    }
}

void Analyzer::push(AccumulatorData_t data)
{
    node *newNode = new node;
    newNode->data = data;

    //if list is empty
    if (head == nullptr)
    {
        newNode->next = nullptr;
        head = newNode;
        tail = newNode;
        numNodes++;
        return;
    }

    //if the list is not full yet
    if(numNodes < size)
    {
        //add in item
        head->next = newNode;
        head = newNode;
        numNodes++;
    }
    //else if the list is full
    else
    {
        //add in item
        head->next = newNode;
        head = newNode;

        //remove the last item
        node *tmp = tail;
        tail = tail->next;
        delete tmp;
        tmp = nullptr;
    }
}

void Analyzer::resize(uint16_t newSize)
{
    size = newSize;

    if(head == nullptr || numNodes < size) return;

    //if the queue is longer than the new size, we need to delete nodes
    while(numNodes > size)
    {
        //remove the last item
        node *tmp = tail;
        tail = tail->next;
        delete tmp;
        tmp = nullptr;
    }
}