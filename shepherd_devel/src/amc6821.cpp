#include "amc6821.h"


AMC6821::AMC6821()
{
    if(verifyFunctionality())
    {
        setDutyCycle(0);
        enablePWM(1);
        getCharacteristics();
        return;
    }
    Serial.println("~~~~~~~~~~~~~~~~WARNING: Unable to verify functionality of AMC6821~~~~~~~~~~~~~~~~~~~~~~~");
    return;
}


AMC6821::~AMC6821(){}


void AMC6821::AMC6821write(uint8_t *cmd, uint8_t numBytes)
{
    Wire.beginTransmission(AMC6821_I2C_ADDR);
    for(uint8_t i=0; i<numBytes;i++)
    {
        Wire.write(cmd[i]);
    }
    Wire.endTransmission(false);
}


bool AMC6821::AMC6821read(uint8_t *msg,uint8_t numBytes)
{
    Wire.requestFrom(AMC6821_I2C_ADDR, (int)numBytes);

    if (Wire.available())
    {
        uint8_t i2cByte=0;
        while(Wire.available())
        {
            msg[i2cByte] = Wire.read();
            i2cByte++;
        }
        return true;
    }

    return false;
}


bool AMC6821::verifyFunctionality()
{
    uint8_t msg[1];
    uint8_t cmd[1]= {AMC6821_DEVID_REG};
    AMC6821write(cmd,1);
    if(AMC6821read(msg,1))
    {
        if(msg[0] == AMC6821_DEVID)
        {
            return true;
        }
        return false;
    }
    return false;
}


void AMC6821::enablePWM(bool pwm_toggle)
{
    uint8_t *config2 = getConfig2Reg();
    if (*config2 == NULL)
    {
        return;
    }
    uint8_t newconfig2 = pwm_toggle ? config2[0] | 0x01 : config2[0] & 0xFE;    //Setting pwm enable bit of config reg to 1 if pwm toggle, else set to 0
    setConfig2Reg(newconfig2);
}


uint8_t *AMC6821::getConfig2Reg()
{
    uint8_t cmd[1] = {AMC6821_CONFIG2_REG};
    uint8_t msg[1];
    AMC6821write(cmd,1);
    if(AMC6821read(msg,1))
    {
        return msg;
    }
    Serial.println("ERROR: Unable to read from AMC6821 Configuration 2 Register");
    return NULL;
}


void AMC6821::setConfig2Reg(uint8_t config)
{
    uint8_t cmd[2] = {AMC6821_CONFIG2_REG, config};
    AMC6821write(cmd,2);
    return;
}


void AMC6821::enableFanSpinup(bool fanspinup_toggle)
{
    characteristicsmsg.bitfieldmsg.fanspinup_enable = fanspinup_toggle;
    setCharacteristics();
}


void AMC6821::setFanSpinUpTime(fanspinuptime_t fanspinuptime)
{
    characteristicsmsg.bitfieldmsg.fanspinuptime = fanspinuptime;
    setCharacteristics();
}


void AMC6821::setPWMFreq(pwmfreq_t pwmfreq)
{
    characteristicsmsg.bitfieldmsg.pwmfreq = pwmfreq;
    setCharacteristics();
}


void AMC6821::setDutyCycle(uint8_t dutycycle)
{
    uint8_t cmd[2] = {AMC6821_DUTYCYCLE_REG,0xFF};
    AMC6821write(cmd, 2);
}


void AMC6821::setCharacteristics()
{
    AMC6821write(characteristicsmsg.msg,1);
}


void AMC6821::getCharacteristics()
{
    uint8_t cmd[1] = {AMC6821_CHARACTERISTICS_REG};
    AMC6821write(cmd,1);
    AMC6821read(characteristicsmsg.msg,1);
    return;
}

void AMC6821::resetChip()
{
  uint8_t cmd[2] = {AMC6821_CONFIG2_REG, 0x80}; //Writes a 1 in the reset bit in Config 2
  AMC6821write(cmd,2);
}

void AMC6821::writeConfig(uint8_t configNum, uint8_t config)
{
  uint8_t cmd[2];
  cmd[1] = config;

  switch(configNum)
  {
    case 1:
      cmd[0] =  AMC6821_CONFIG1_REG;
      break;
    case 2:
      cmd[0] =  AMC6821_CONFIG2_REG;
      break;
    case 3:
      cmd[0] =  AMC6821_CONFIG3_REG;
      break;
    case 4:
      cmd[0] =  AMC6821_CONFIG4_REG;
      break;
    default:
      Serial.println("Unidentified Config #!");
      break;
  }
  
  AMC6821write(cmd,2);
}