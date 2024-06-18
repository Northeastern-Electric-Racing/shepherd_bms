**Public Archive. Code in use to/from: 2023/2023**

# shepherd_bms
Repo for Shepherd BMS development

## For cloning

After you clone via ```git clone git@github.com:Northeastern-Electric-Racing/shepherd_bms.git```

Make sure that you run ```git submodule update --init```

And if you are trying to pull new changes from the submodules (i.e. NERduino library), run
```git submodule update --remote```

## Overview of Repo
```
.
|
|───docs                //Directory containing some files for documentation
|───segment_testing     //Directory containing all code for testing
|
|───shepherd_bms        // ** Directory containing all shepherd source code
|   |───include     //Directory for header files
|   |───lib         //Directory for submodules/libraries
|   |───src         //Directory for cpp files
|   └───test        //Directory for files to test code
|
└───README.md
```

Link to LTC6804/LTC6820 Communication datasheet
https://www.analog.com/media/en/technical-documentation/data-sheets/680412fc.pdf

![shepherd_architecture](https://github.com/Northeastern-Electric-Racing/shepherd_bms/blob/main/docs/shepherd_architecture.svg)

![ltc_comms](https://github.com/Northeastern-Electric-Racing/shepherd_bms/blob/main/docs/LTC6820_comms_overview.drawio.svg)
