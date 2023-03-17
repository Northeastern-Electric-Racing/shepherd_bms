#include "stateMachine.h"


StateMachine::StateMachine()
{
    currentState = BOOT_STATE;
    initBoot();
}

StateMachine::~StateMachine()
{}

void StateMachine::initBoot()
{
	Serial.println("boot");
    return;
}

void StateMachine::handleBoot(AccumulatorData_t *bmsdata)
{

	overVoltCount = 0;
	underVoltCount = 0;
    overCurrCount = 0;
    chargeOverVolt = 0;
	overChgCurrCount = 0;
    lowCellCount = 0;

	prevAccData = nullptr;

	segment.enableBalancing(false);
    compute.enableCharging(false);

	//bmsdata->faultCode = FAULTS_CLEAR;
	
	requestTransition(READY_STATE);
	
    return;
}

void StateMachine::initReady()
{
	Serial.println("ready");
	segment.enableBalancing(false);
    compute.enableCharging(false);
    return;
}

void StateMachine::handleReady(AccumulatorData_t *bmsdata)
{
	//check for charger
	if (digitalRead(CHARGE_DETECT) == LOW)
	{
		requestTransition(CHARGING_STATE);
	}

	else
	{	
    	broadcastCurrentLimit(bmsdata);
    	return;
	}
}

void StateMachine::initCharging()
{
    return;
}

void StateMachine::handleCharging(AccumulatorData_t *bmsdata)
{
   

    if (digitalRead(CHARGE_DETECT) == HIGH)
    {
        requestTransition(READY_STATE);
        return;
    }

	else
	{
    	if (digitalRead(CHARGE_DETECT) == LOW) 
			{
				// Check if we should charge
				if (chargingCheck(analyzer.bmsdata)) 
				{
					digitalWrite(CHARGE_SAFETY_RELAY, HIGH);
					compute.enableCharging(true);
					
				} 
				else 
				{
					digitalWrite(CHARGE_SAFETY_RELAY, LOW);
					compute.enableCharging(false);
					
				}

				// Check if we should balance
				if (balancingCheck(analyzer.bmsdata)) 
				{
					segment.enableBalancing(true);
					balanceCells(analyzer.bmsdata);
				} 
				else 
				{
					segment.enableBalancing(false);
				}
			
				// Send CAN message, but not too often
				if (chargeMessageTimer.isTimerExpired()) 
				{
					compute.sendChargingMessage((MAX_CHARGE_VOLT * NUM_CELLS_PER_CHIP * NUM_CHIPS), analyzer.bmsdata);
					chargeMessageTimer.startTimer(CHARGE_MESSAGE_WAIT);
				}
			} 
			else
			{
				digitalWrite(CHARGE_SAFETY_RELAY, LOW);
			}

        	return;
	}
}

void StateMachine::initFaulted()
{
	Serial.println("faulted");
	segment.enableBalancing(false);
    compute.enableCharging(false);
    return;
}

void StateMachine::handleFaulted(AccumulatorData_t *bmsdata)
{
    if (bmsdata->faultCode == FAULTS_CLEAR)
    {
        compute.setFault(NOT_FAULTED);
        requestTransition(BOOT_STATE);
        return;
    }

    else 
    {

        compute.setFault(FAULTED);
	    digitalWrite(CHARGE_SAFETY_RELAY, LOW);
	

	    Serial.print("BMS FAULT: ");
	    Serial.println(bmsdata->faultCode, HEX);
	    Serial.println("Hit Spacebar to clear");
	    delay(1000);
	    if (Serial.available()) 
	    { // Check for key presses
	        char keyPress = Serial.read(); // Read key
		    if (keyPress == ' ') 
		    {
			    bmsFault = FAULTS_CLEAR;
		    }   
	    }
    
    }

    return;
}

void StateMachine::handleState(AccumulatorData_t *bmsdata)
{
	bmsdata->faultCode = faultCheck(bmsdata);
	bmsdata->faultCode = 32;

	 if (bmsdata->faultCode != FAULTS_CLEAR)
    {
		bmsdata->dischargeLimit = 0;
        requestTransition(FAULTED_STATE);
    }

    (this->*handlerLUT[currentState])(bmsdata);


	compute.setFanSpeed(analyzer.calcFanPWM());
	broadcastCurrentLimit(bmsdata);

	//send relevant CAN msgs
	compute.sendAccStatusMessage(analyzer.bmsdata->packVoltage, analyzer.bmsdata->packCurrent, 0, 0, 0);
	compute.sendCurrentsStatus(analyzer.bmsdata->dischargeLimit, analyzer.bmsdata->chargeLimit, analyzer.bmsdata->packCurrent);
	
	//todo send BMS status msg once this gets merged with that PR (and any other msgs we want)
	
}

void StateMachine::requestTransition(BMSState_t nextState)
{
    if(currentState == nextState) return;
    if(!validTransitionFromTo[currentState][nextState]) return;
	
    (this->*initLUT[nextState])();
	currentState = nextState;
	
	
}


uint32_t StateMachine::faultCheck(AccumulatorData_t *accData)
{
	// FAULT CHECK (Check for fuckies)
	uint32_t faultStatus = 0;

	// Over current fault for discharge
	if ((accData->packCurrent) > ((accData->dischargeLimit)*10)) 
	{
		overCurrCount++;
		if (overCurrCount > 10) 
		{ // 0.10 seconds @ 100Hz rate
			faultStatus |= DISCHARGE_LIMIT_ENFORCEMENT_FAULT;
		}
	} else 
	{
		overCurrCount = 0;
	}

	// Over current fault for charge
	if ((accData->packCurrent) < 0 && abs((accData->packCurrent)) > ((accData->chargeLimit)*10))
	{
		overChgCurrCount++;
		if (overChgCurrCount > 100) 
		{ // 1 seconds @ 100Hz rate
			faultStatus |= CHARGE_LIMIT_ENFORCEMENT_FAULT;
		}
	} 
	else 
	{
		overChgCurrCount = 0;
	} 

	// Low cell voltage fault
	if (accData->minVoltage.val < MIN_VOLT * 10000) 
	{

		underVoltCount++;
		if (underVoltCount > 900)
		{ // 9 seconds @ 100Hz rate
			faultStatus |= CELL_VOLTAGE_TOO_LOW;
		}
	} 
	else 
	{
		underVoltCount = 0;
	}

	// High cell voltage fault
	if (((accData->maxVoltage.val > MAX_VOLT * 10000) && digitalRead(CHARGE_DETECT) == HIGH) || (accData->maxVoltage.val > MAX_CHARGE_VOLT * 10000)) 
	{ // Needs to be reimplemented with a flag for every cell in case multiple go over
		overVoltCount++;
		if (overVoltCount > 900) { // 9 seconds @ 100Hz rate
			faultStatus |= CELL_VOLTAGE_TOO_HIGH;
		}
	} 
	else 
	{
		overVoltCount = 0;
	}

	// High Temp Fault
	if (accData->maxTemp.val > MAX_CELL_TEMP) {
		faultStatus |= PACK_TOO_HOT;
	}

	// Extremely low cell voltage fault
	if (accData->minVoltage.val < 900) 
	{ // 90mV
		lowCellCount++;
		if (lowCellCount > 100) { // 1 seconds @ 100Hz rate
			faultStatus |= LOW_CELL_VOLTAGE;
		}
	} 
	else 
	{
		lowCellCount = 0;
	}

	return faultStatus;
}

bool StateMachine::chargingCheck(AccumulatorData_t *bmsdata)
{
	static Timer chargeTimeout;

	if(!chargeTimeout.isTimerExpired()) return false;
	if(!compute.isCharging()) return false;
	if(bmsdata->maxVoltage.val >= (MAX_CHARGE_VOLT * 10000))
	{
		chargeOverVolt++;
		if (chargeOverVolt > 100) 
		{
			chargeTimeout.startTimer(CHARGE_TIMEOUT);
			return false;
		}
	} 
	else 
	{
		chargeOverVolt = 0;
	}

	return true;
}

bool StateMachine::balancingCheck(AccumulatorData_t *bmsdata)
{
	if (!compute.isCharging()) return false;
	if (bmsdata->maxTemp.val > MAX_CELL_TEMP_BAL) return false;
	if (bmsdata->maxVoltage.val <= (BAL_MIN_V * 10000)) return false;
	if(bmsdata->deltVoltage <= (MAX_DELTA_V * 10000)) return false;

	return true;
}

void broadcastCurrentLimit(AccumulatorData_t *bmsdata)
{
	// States for Boosting State Machine
	static enum
	{
		BOOST_STANDBY,
		BOOSTING,
		BOOST_RECHARGE
	}BoostState;

	static Timer boostTimer;
	static Timer boostRechargeTimer;

	//Transitioning out of boost
	if(boostTimer.isTimerExpired() && BoostState == BOOSTING)
	{
		BoostState = BOOST_RECHARGE;
		boostRechargeTimer.startTimer(BOOST_RECHARGE_TIME);
	}
	//Transition out of boost recharge
	if(boostRechargeTimer.isTimerExpired() && BoostState == BOOST_RECHARGE)
	{
		BoostState = BOOST_STANDBY;
	}
	//Transition to boosting
	if((bmsdata->packCurrent) > ((bmsdata->contDCL)*10) && BoostState == BOOST_STANDBY)
	{
		BoostState = BOOSTING;
		boostTimer.startTimer(BOOST_TIME);
	}

	//Currently boosting
	if(BoostState == BOOSTING || BoostState == BOOST_STANDBY)
	{
		compute.sendMCMsg(bmsdata->chargeLimit, min(bmsdata->dischargeLimit, bmsdata->contDCL * CONTDCL_MULTIPLIER));
	}
	//Currently recharging boost
	else
	{
		compute.sendMCMsg(bmsdata->chargeLimit, min(bmsdata->contDCL, bmsdata->dischargeLimit));
	}
}

void balanceCells(AccumulatorData_t *bms_data)
{
	bool balanceConfig[NUM_CHIPS][NUM_CELLS_PER_CHIP];

	// For all cells of all the chips, figure out if we need to balance by comparing the difference in voltages
    for(uint8_t chip = 0; chip < NUM_CHIPS; chip++)
    {
		for(uint8_t cell = 0; cell < NUM_CELLS_PER_CHIP; cell++)
		{
			uint16_t delta = bms_data->chipData[chip].voltageReading[cell] - (uint16_t)bms_data-> minVoltage.val;
			if(delta > MAX_DELTA_V * 10000)
				balanceConfig[chip][cell] = true;
			else
				balanceConfig[chip][cell] = false;
        }
    }
	#ifdef DEBUG_CHARGING
	Serial.println("Cell Balancing:");
	for(uint8_t c = 0; c < NUM_CHIPS; c++)
    {
        for(uint8_t cell = 0; cell < NUM_CELLS_PER_CHIP; cell++)
        {
			Serial.print(balanceConfig[c][cell]);
			Serial.print("\t");
		}
		Serial.println();
	}
	#endif

	segment.configureBalancing(balanceConfig);
}
