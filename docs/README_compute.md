# Shepherd Compute Interface README

This code provides the firmware to interact with the Shepherd Compute Board. Features implemented into this interface
work to communicate with and control all chips, boards, and components relating to the BMS.

## Overview 


### CAN

CAN is the principle mode of communication in this system. Data has to be properly formatted to be sent across CAN, and 
there are two seperate CAN lines in the car. The ability to choose either line to send/recieve data is necesarry. 

### PWM

PWM, or pulse-width modulation, is a type of digital signal. The AMC6821 chip on the Compute Board is a PWM fan controller,
and configuration is needed in order to control the BMS fans. More info can be found in the [NERduino README](https://github.com/Northeastern-Electric-Racing/NERduino/blob/main/README.md)

### Charger Handling

To control the status of the charger (charging, discharging, etc), messages have to be sent and recieved from the charger 
controller over the CAN network.

### Motor Controller

To control the motor, current/voltage limits need to be calculated and communicated to the motor over CAN.

### TS-GLV reading

THe compute board needs to read in the TS-GLV (transmission system - grounded low voltage) line through an ADC reading, 
which needs configuring. 




