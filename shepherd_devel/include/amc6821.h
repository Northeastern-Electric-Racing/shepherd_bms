
#ifndef AMC6821_H
#define AMC6821_H

#include <Arduino.h>

//AMC6821 PWM Generator
/**
 * https://www.ti.com/lit/ds/symlink/amc6821.pdf?ts=1644706226375&ref_url=https%253A%252F%252Fwww.ti.com%252Fproduct%252FAMC6821%253Futm_source%253Dgoogle%2526utm_medium%253Dcpc%2526utm_campaign%253Dasc-sens-null-prodfolderdynamic-cpc-pf-google-wwe%2526utm_content%253Dprodfolddynamic%2526ds_k%253DDYNAMIC%2BSEARCH%2BADS%2526DCM%253Dyes%2526gclid%253DCj0KCQiA0p2QBhDvARIsAACSOOPKQVP7tfyxbaC8997ZjeHcQWZiSwAi1yblV-rFrJZ4BQS3xCwo1iYaAjmLEALw_wcB%2526gclsrc%253Daw.ds 
 * ^Datasheet
 */
#include <Wire.h> //I2C library 


#define AMC6821_I2C_ADDR                    0x18
#define AMC6821_DEVID_REG                   0x3D
#define AMC6821_DEVID                       0x21
#define AMC6821_CONFIG1_REG                 0x00
#define AMC6821_CONFIG2_REG                 0x01
#define AMC6821_CONFIG3_REG                 0x3F
#define AMC6821_CONFIG4_REG                 0x04


/**
 * @note Duty Cycle is variable corresponding to 1/255 increments 
 *       (0x00 corresponds to 0% duty cycle and 0xFF(255) corresponds to 100% duty cycle)
 * @ref Refer to page 40 of datasheet
 */
#define AMC6821_DUTYCYCLE_REG               0x22


/**
 * @brief PWM I2C Characteristics Command Contents  (8 bits)
 * @ref Refer to page 41 of datasheet
 *                _______________________________________________________________________________________________________________
 * bit numbers:   |            7            |      6      |           5   4   3               |            2   1   0            |
 *                |-------------------------|-------------|-----------------------------------|---------------------------------|
 * contents:      |   Fan spin up disable   |   Reserved  |   PWM Frequency(see AMC macros)   |   Spin up time(see AMC macros)  |
 *                |_________________________|_____________|___________________________________|_________________________________|
 * 
 */
#define AMC6821_CHARACTERISTICS_REG         0x20

//PWM Frequencies type
typedef enum
{
    AMC6821_CHARACTERISTICS_1KHZ,
    AMC6821_CHARACTERISTICS_10KHZ,
    AMC6821_CHARACTERISTICS_20KHZ,
    AMC6821_CHARACTERISTICS_25KHZ,          //(default) frequency for our fans
    AMC6821_CHARACTERISTICS_30KHZ,
    AMC6821_CHARACTERISTICS_40KHZ           //0x06 and 0x07 also result in 40kHz duty cycle
}pwmfreq_t;

//Fan spin up times type
typedef enum
{
    AMC6821_CHARACTERISTICS_SPINUP_02,      //02 means 0.2 seconds
    AMC6821_CHARACTERISTICS_SPINUP_04,      //04 means 0.4 seconds
    AMC6821_CHARACTERISTICS_SPINUP_06,      //...
    AMC6821_CHARACTERISTICS_SPINUP_08,
    AMC6821_CHARACTERISTICS_SPINUP_1,
    AMC6821_CHARACTERISTICS_SPINUP_2,       //(default)
    AMC6821_CHARACTERISTICS_SPINUP_4,
    AMC6821_CHARACTERISTICS_SPINUP_8 
}fanspinuptime_t;

/*********************************************************************************************/

class AMC6821
{
    private:
        /**
         * @brief characteristics register bitfield
         * 
         */
        union
        {
            uint8_t *msg;

            struct
            {
                uint8_t fanspinup_enable        :1;
                uint8_t reserved                :1;
                pwmfreq_t pwmfreq               :3;
                fanspinuptime_t fanspinuptime   :3;
            } bitfieldmsg;
        }characteristicsmsg;

        /**
         * @brief Writing to the local I2C bus with the address of the AMC6821
         * 
         * @param cmd 
         * @param numBytes 
         */
        void AMC6821write(uint8_t *cmd, uint8_t numBytes);

        /**
         * @brief Requesting data to read in from the AMC6821
         * 
         * @param msg 
         * @param numBytes 
         * @return true 
         * @return false 
         */
        bool AMC6821read(uint8_t *msg,uint8_t numBytes);

        /**
         * @brief Sets the characteristics of the characteristics register for PWM operation
         * 
         */
        void setCharacteristics();


    public:
        /**
         * @brief Construct a new AMC6821::AMC6821 object, verify functionality, set PWM duty cycle to 0, and retrieve current pwm characteristics
         * 
         */
        AMC6821();

        ~AMC6821();

        /**
         * @brief Verify the functionality of the onboard AMC6821
         * 
         * @return true 
         * @return false 
         */
        bool verifyFunctionality();

        /**
         * @brief Toggles the PWM output
         * 
         * @param pwm_toggle 
         */
        void enablePWM(bool pwm_toggle);

        /**
         * @brief Toggles the Fan Spin Up mode for PWM output
         * 
         * @param fanspinup_toggle 
         */
        void enableFanSpinup(bool fanspinup_toggle);

        /**
         * @brief Set the Fan Spin Up Time
         * 
         * @param fanspinuptime 
         */
        void setFanSpinUpTime(fanspinuptime_t fanspinuptime);

        /**
         * @brief Sets the PWM Frequency
         * 
         * @param pwmfreq 
         */
        void setPWMFreq(pwmfreq_t pwmfreq);

        /**
         * @brief Sets the duty cycle of the PWM output
         * 
         * @param dutycycle
         */
        void setDutyCycle(uint8_t dutycycle);

        /**
         * @brief Retrieves the current characteristics for PWM operation
         * 
         */
        void getCharacteristics();

        /**
         * @brief Reading the Configuration 2 Register
         * 
         * @return uint8_t* 
         */
        uint8_t *getConfig2Reg();

        /**
         * @brief Set the Configuration 2 Register
         * 
         * @param config 
         */
        void setConfig2Reg(uint8_t config);

        /**
         * @brief Resets the registers of the AMC6821
         * @note Essentially behaves like a soft power cycle
         * 
         */
        void resetChip();

        /**
         * @brief Writes to the specified config register the desired config
         * 
         * @param configNum 
         * @param config 
         */
        void writeConfig(uint8_t configNum, uint8_t config);
};

#endif