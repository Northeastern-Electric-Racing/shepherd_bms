#include "stateMachine.h"

faultEval overCurr;
faultEval overChgCurr;
faultEval underVolt;
faultEval overVolt;
faultEval lowCell;
faultEval highTemp;

Timer chargeTimeout;
faultEval chargeCutoffTime;

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
	if (compute.chargerConnected())
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
	chargeTimeout.cancelTimer();
    return;
}

void StateMachine::handleCharging(AccumulatorData_t *bmsdata)
{
    if (!compute.chargerConnected())
    {
        requestTransition(READY_STATE);
        return;
    }
	else
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
		else
		{
			digitalWrite(CHARGE_SAFETY_RELAY, LOW);
		}
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
		compute.sendAccStatusMessage(analyzer.bmsdata->pack_voltage, analyzer.bmsdata->pack_current, 0, analyzer.bmsdata->soc, 0);
		compute.sendCurrentsStatus(analyzer.bmsdata->discharge_limit, analyzer.bmsdata->charge_limit, analyzer.bmsdata->pack_current);
		compute.sendBMSStatusMessage(current_state, bmsdata->fault_code, bmsdata->avg_temp, static_cast<int8_t>(0), segment.isBalancing());
		compute.sendCellTemp(analyzer.bmsdata->max_temp, analyzer.bmsdata->min_temp, analyzer.bmsdata->avg_temp);
		compute.sendCellDataMessage(analyzer.bmsdata->max_voltage, analyzer.bmsdata->min_voltage, analyzer.bmsdata->avg_voltage);
		compute.sendSegmentTemps(analyzer.bmsdata->segment_average_temps);
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

	if (overCurr.faultEvalState == BEFORE_TIMER_START && (accData->pack_current) > ((accData->discharge_limit + DCDC_CURRENT_DRAW)*10*1.04)) // *104% to account for current sensor +/-A
    {
		overCurr.faultTimer.startTimer(OVER_CURR_TIME);
		overCurr.faultEvalState = DURING_FAULT_EVAL;
    }
    else if (overCurr.faultEvalState == DURING_FAULT_EVAL)
    {
        if (overCurr.faultTimer.isTimerExpired())
        {
            faultStatus |= DISCHARGE_LIMIT_ENFORCEMENT_FAULT;
        }
        if (!((accData->pack_current) > ((accData->discharge_limit + DCDC_CURRENT_DRAW)*10*1.04)))
        {
            overCurr.faultTimer.cancelTimer();
            overCurr.faultEvalState = BEFORE_TIMER_START;
        }
    }

	// Over current fault for charge
	if (overChgCurr.faultEvalState == BEFORE_TIMER_START && ((accData->pack_current) < 0 && abs((accData->pack_current)) > ((accData->charge_limit)*10)))
    {
        overChgCurr.faultTimer.startTimer(OVER_CHG_CURR_TIME);
        overChgCurr.faultEvalState = DURING_FAULT_EVAL;
    }
    else if (overChgCurr.faultEvalState == DURING_FAULT_EVAL)
    {
        if (overChgCurr.faultTimer.isTimerExpired())
        {
            faultStatus |= CHARGE_LIMIT_ENFORCEMENT_FAULT;
        }
        if (!((accData->pack_current) < 0 && abs((accData->pack_current)) > ((accData->charge_limit)*10)))
        {
            overChgCurr.faultTimer.cancelTimer();
            overChgCurr.faultEvalState = BEFORE_TIMER_START;
        }
    }

	// Low cell voltage fault
	if (underVolt.faultEvalState == BEFORE_TIMER_START && accData->min_voltage.val < MIN_VOLT * 10000)
    {
        underVolt.faultTimer.startTimer(UNDER_VOLT_TIME );
        underVolt.faultEvalState = DURING_FAULT_EVAL;
    }
    else if (underVolt.faultEvalState == DURING_FAULT_EVAL)
    {
        if (underVolt.faultTimer.isTimerExpired())
        {
            faultStatus |= CELL_VOLTAGE_TOO_LOW;
        }
        if (!(accData->min_voltage.val < MIN_VOLT * 10000))
        {
            underVolt.faultTimer.cancelTimer();
            underVolt.faultEvalState = BEFORE_TIMER_START;
        }
    }

	// High cell voltage fault
	if (overVolt.faultEvalState == BEFORE_TIMER_START && (((accData->max_voltage.val > MAX_VOLT * 10000) && digitalRead(CHARGE_DETECT) == HIGH) || (accData->max_voltage.val > MAX_CHARGE_VOLT * 10000)))
    {
        overVolt.faultTimer.startTimer(OVER_VOLT_TIME);
        overVolt.faultEvalState = DURING_FAULT_EVAL;
    }
    else if (overVolt.faultEvalState == DURING_FAULT_EVAL)
    {
        if (overVolt.faultTimer.isTimerExpired())
        {
            faultStatus |= CELL_VOLTAGE_TOO_HIGH;
        }
        if (!((accData->max_voltage.val > MAX_VOLT * 10000) && digitalRead(CHARGE_DETECT) == HIGH) || (accData->max_voltage.val > MAX_CHARGE_VOLT * 10000))
        {
            overVolt.faultTimer.cancelTimer();
            overVolt.faultEvalState = BEFORE_TIMER_START;
        }
    }

	// High Temp Fault
	if (highTemp.faultEvalState == BEFORE_TIMER_START && (accData->max_temp.val > MAX_CELL_TEMP))
    {
        highTemp.faultTimer.startTimer(HIGH_TEMP_TIME);
        highTemp.faultEvalState = DURING_FAULT_EVAL;
    }
    else if (highTemp.faultEvalState == DURING_FAULT_EVAL)
    {
        if (highTemp.faultTimer.isTimerExpired())
        {
            faultStatus |= PACK_TOO_HOT;
        }
        if (!(accData->max_temp.val > MAX_CELL_TEMP))
        {
            highTemp.faultTimer.cancelTimer();
            highTemp.faultEvalState = BEFORE_TIMER_START;
        }
    }

	// Extremely low cell voltage fault
	if (lowCell.faultEvalState == BEFORE_TIMER_START && (accData->min_voltage.val < 900))
    {
        lowCell.faultTimer.startTimer(LOW_CELL_TIME);
        lowCell.faultEvalState = DURING_FAULT_EVAL;
    }
    else if (lowCell.faultEvalState == DURING_FAULT_EVAL)
    {
        if (lowCell.faultTimer.isTimerExpired())
        {
            faultStatus |= LOW_CELL_VOLTAGE;
        }
        if (!(accData->min_voltage.val < 900))
        {
            lowCell.faultTimer.cancelTimer();
            lowCell.faultEvalState = BEFORE_TIMER_START;
        }
    }

	return faultStatus;
}

bool StateMachine::chargingCheck(AccumulatorData_t *bmsdata)
{
	if (!compute.chargerConnected()) return false;
	if(!chargeTimeout.isTimerExpired()) return false;

	if (bmsdata->max_voltage.val >= (MAX_CHARGE_VOLT * 10000) && chargeCutoffTime.faultEvalState == BEFORE_TIMER_START)
    {
        chargeCutoffTime.faultTimer.startTimer(5000);
        chargeCutoffTime.faultEvalState = DURING_FAULT_EVAL;
    }
    else if (chargeCutoffTime.faultEvalState == DURING_FAULT_EVAL)
    {
        if (chargeCutoffTime.faultTimer.isTimerExpired())
        {
			chargeTimeout.startTimer(CHARGE_TIMEOUT);
            return false;
        }
        if (!(bmsdata->max_voltage.val >= (MAX_CHARGE_VOLT * 10000)))
        {
            chargeCutoffTime.faultTimer.cancelTimer();
            chargeCutoffTime.faultEvalState = BEFORE_TIMER_START;
        }
    }

	return true;
}

bool StateMachine::balancingCheck(AccumulatorData_t *bmsdata)
{
	if (!compute.chargerConnected()) return false;
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
