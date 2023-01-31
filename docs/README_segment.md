# Shepherd Segment Interface README

This code provided the firmware to interact with the Shepherd Segment board. The segment board manages all data readings from 
the battery cells, and this interface provides functionality to recieve, interpret, and respond to that data.

##  Code Overview

### Cell Data Representation / Data Receiving

Voltages, temperatures, and charging status need to be recorded at any moment. Reading this data, 
and configuring it into a datatype that is easy to interact with through the code proves to be valuable. 

### Individual Segment Control

There are many different segment boards used to control all cells, and there needs to be the ability to isolate
each one to send and recieve individual commands.

### CRC error checking

CRC (Cyclic Redundancy Check) is a method of digital error checking that finds errors often due to noise. 
This needs to be handled to remove or correct this data.

## SPI 

SPI and isoSPI (a variation of SPI) is the communication method used by the LTC6820 (on compute) and LTC6811 (on segment), 
which allows or the sending and receiving of all the segment data. It requires unique functionality seperate from other 
forms of communication, and there are two seperate SPI lines in the vehicle.





