#include "stateMachine.h"




StateMachine::StateMachine()
{
    current_state = BOOT_STATE;
    initBoot();
}

StateMachine::~StateMachine()
{}

void StateMachine::initBoot()
{
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

	//bmsdata->fault_code = FAULTS_CLEAR;

	requestTransition(READY_STATE);

    return;
}

void StateMachine::initReady()
{
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
	segment.enableBalancing(false);
    compute.enableCharging(false);
	enteredFaulted = true;
    return;
}

void StateMachine::handleFaulted(AccumulatorData_t *bmsdata)
{
	if (enteredFaulted)
	{
		enteredFaulted = false;
		previousFault = faultCheck(bmsdata);
	}

    if (bmsdata->fault_code == FAULTS_CLEAR)
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
	    Serial.println(bmsdata->fault_code, HEX);
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

	bmsdata->fault_code = faultCheck(bmsdata);

	 if (bmsdata->fault_code != FAULTS_CLEAR)
    {
		bmsdata->discharge_limit = 0;
        requestTransition(FAULTED_STATE);
    }

    (this->*handlerLUT[current_state])(bmsdata);


	compute.setFanSpeed(analyzer.calcFanPWM());
	broadcastCurrentLimit(bmsdata);

	//send relevant CAN msgs
	if (canMsgTimer.isTimerExpired())
	{
		compute.sendMCMsg(bmsdata->charge_limit, bmsdata->boost_setting);
		compute.sendAccStatusMessage(analyzer.bmsdata->pack_voltage, analyzer.bmsdata->pack_current, 0, analyzer.bmsdata->soc, 0);
		compute.sendCurrentsStatus(analyzer.bmsdata->discharge_limit, analyzer.bmsdata->charge_limit, analyzer.bmsdata->pack_current);
		compute.sendBMSStatusMessage(current_state, bmsdata->fault_code, bmsdata->avg_temp, 0);
		compute.sendCellTemp(analyzer.bmsdata->max_temp, analyzer.bmsdata->min_temp, analyzer.bmsdata->avg_temp);
		compute.sendCellDataMessage(analyzer.bmsdata->max_voltage, analyzer.bmsdata->min_voltage, analyzer.bmsdata->avg_voltage);
		canMsgTimer.startTimer(CAN_MESSAGE_WAIT);
	}

}

void StateMachine::requestTransition(BMSState_t next_state)
{
    if(current_state == next_state) return;
    if(!validTransitionFromTo[current_state][next_state]) return;

    (this->*initLUT[next_state])();
	current_state = next_state;


}


uint32_t StateMachine::faultCheck(AccumulatorData_t *accData)
{
	// FAULT CHECK (Check for fuckies)
	uint32_t faultStatus = 0;

	// Over current fault for discharge
	if ((accData->pack_current) > ((accData->discharge_limit)*10) + CURR_ERR_MARG)
	{
		overCurrCount++;
		if (overCurrCount > 500)
		{ // 1.0 seconds @ 100Hz rate
			faultStatus |= DISCHARGE_LIMIT_ENFORCEMENT_FAULT;
		}
	} else
	{
		overCurrCount = 0;
	}

	// Over current fault for charge
	if ((accData->pack_current) < 0 && abs((accData->pack_current)) > ((accData->charge_limit)*10) + CURR_ERR_MARG)
	{
		overChgCurrCount++;
		if (overChgCurrCount > 1000)
		{ // 1 seconds @ 100Hz rate
			faultStatus |= CHARGE_LIMIT_ENFORCEMENT_FAULT;
		}
	}
	else
	{
		overChgCurrCount = 0;
	}

	// Low cell voltage fault
	if (accData->min_voltage.val < MIN_VOLT * 10000)
	{

		underVoltCount++;
		if (underVoltCount > 20000) // 20 seconds @ 1000Hz rate
		{
			faultStatus |= CELL_VOLTAGE_TOO_LOW;
		}
	}
	else
	{
		underVoltCount = 0;
	}

	// High cell voltage fault
	if (((accData->max_voltage.val > MAX_VOLT * 10000) && digitalRead(CHARGE_DETECT) == HIGH) || (accData->max_voltage.val > MAX_CHARGE_VOLT * 10000))
	{ // Needs to be reimplemented with a flag for every cell in case multiple go over
		overVoltCount++;
		if (overVoltCount > 2000) { // 20 seconds @ 100Hz rate
			faultStatus |= CELL_VOLTAGE_TOO_HIGH;
		}
	}
	else
	{
		overVoltCount = 0;
	}

	// High Temp Fault
	if (accData->max_temp.val > MAX_CELL_TEMP) {
		highTempCount++;
		if (highTempCount > 2000) { // 20 seconds @ 100Hz rate
			faultStatus |= PACK_TOO_HOT;
		}
	}
	else
	{
		highTempCount = 0;
	}

	// Extremely low cell voltage fault
	if (accData->min_voltage.val < 900)
	{ // 90mV
		lowCellCount++;
		if (lowCellCount > 2000) { // 20 seconds @ 100Hz rate
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
	// Serial.println("Timer expired");
	// if(!compute.isCharging()) return false;
	// Serial.println("Timer expired");
	if(bmsdata->max_voltage.val >= (MAX_CHARGE_VOLT * 10000))
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
	if (bmsdata->max_temp.val > MAX_CELL_TEMP_BAL) return false;
	if (bmsdata->max_voltage.val <= (BAL_MIN_V * 10000)) return false;
	if(bmsdata->delt_voltage <= (MAX_DELTA_V * 10000)) return false;

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
	if((bmsdata->pack_current) > ((bmsdata->cont_DCL)*10) && BoostState == BOOST_STANDBY)
	{
		BoostState = BOOSTING;
		boostTimer.startTimer(BOOST_TIME);
	}

	//Currently boosting
	if(BoostState == BOOSTING || BoostState == BOOST_STANDBY)
	{
		bmsdata->boost_setting = min(bmsdata->discharge_limit, bmsdata->cont_DCL * CONTDCL_MULTIPLIER);
	}
	//Currently recharging boost
	else
	{
		bmsdata->boost_setting = min(bmsdata->cont_DCL, bmsdata->discharge_limit);
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
			uint16_t delta = bms_data->chip_data[chip].voltage_reading[cell] - (uint16_t)bms_data-> min_voltage.val;
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
