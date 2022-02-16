# BMS and NERduino Code
It's finally here, we're just starting the lay the groundwork for the code for the BMS as testing and 
board design finish up. We hope to use this file structure and the ```nerduino.h``` and ```nerduino.cpp```
files in other parts of the car just to standardize the code and make it more readable

##### The #1 priority of this repo is to consolidate the NERduino chip drivers into a few files that can be univerally used on NERduinos throughout the car. 

###### The #2 priority is to write the drivers behind the BMS system and shepherd segment. 

###### The #3 priority is to optimize battery management through BMS algorithms

### Some things to note
 1. This code is in it's really early stages right now, so it's very empty.

 2. The LTC and LT_SPI files are copied from the Shepherd Segment Tests and _will_ need to be updated

 2. Uses a VSCode extension called ```PlatformIO``` to upload to Teensy 4.1 on NERduino

 3. Updates to Come!

## TODO's
### NERDUINO LOCAL (High Priority)
##### AXDL312 Accelerometer
 - ~~Proof of Concept~~ kinda
 - Implement functionality
 - Test implementation
##### SHT Humidity Sensor
 - Proof of Concept
 - Implement functionality
 - Test implementation
##### SN65HVD Can Transceiver (Note this requires more systems knowledge)
- ~~Proof of Concept~~
- Implement Functionality
- Test implementation
##### AMC6821 PWM Generator / Fan controller
- Proof of Concept
- Implement Functionality
- Test implementation
### SHEPHERD COMPUTE/SEGMENT
##### LTC6820 isoSPI Chip
- Proof of Concept
- Implement Functionality
- Test implementation
##### Other Shepherd Compute Functionalities
- Latching Fault/Charging?
- Diode OR?
