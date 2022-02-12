#ifndef SEGMENT_H
#define SEGMENT_H

/*************************************************/

#define MUX1_ADDRESS 0x90
#define MUX2_ADDRESS 0x92
#define MUX3_ADDRESS 0x94
#define MUX4_ADDRESS 0x96
#include <Arduino.h>

//Boolean type to designate whether to serialize or deserialize data


class segment
{
    private:
        typedef enum
        {
            MSG_OPERATION_SERIALIZE,
            MSG_OPERATION_DESERIALIZE
        } msg_Ser_Operation_t;

        void msgSerialization(uint8_t numChips, uint8_t data[][3], uint8_t commData [][6], msg_Ser_Operation_t operation);

    public:
        
};


/*************************************************/

#endif