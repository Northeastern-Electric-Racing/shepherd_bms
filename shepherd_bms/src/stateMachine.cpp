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
		previousFault = faultReturn(bmsdata);
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

    preFaultCheck(bmsdata);
	bmsdata->is_charger_connected = compute.chargerConnected();
	bmsdata->fault_code = faultReturn(bmsdata);
	
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


uint32_t StateMachine::faultReturn(AccumulatorData_t *accData)
{
	// FAULT CHECK (Check for fuckies)

	static struct fault_eval fault_table[8] = 
        {
          // ___________FAULT ID____________   __________TIMER___________   _____________DATA________________    __OPERATOR__   __________________________THRESHOLD____________________________  _______TIMER LENGTH_________  _____________FAULT CODE_________________    _______________DATA_____________________  ___OPERATOR___  ________THRESHOLD________
            {.id = "Discharge Current Limit", .timer =       overCurr_tmr, .data_1 =    accData->pack_current, .optype_1 = GT, .lim_1 = (accData->discharge_limit + DCDC_CURRENT_DRAW)*10*1.04, .timeout =      OVER_CURR_TIME,	.code = DISCHARGE_LIMIT_ENFORCEMENT_FAULT   /* -----------------------------------UNUSED---------------------------------*/ }, 
            {.id = "Charge Current Limit",    .timer =    overChgCurr_tmr, .data_1 =    accData->pack_current, .optype_1 = GT, .lim_1 =                             (accData->charge_limit)*10, .timeout =  OVER_CHG_CURR_TIME, .code =    CHARGE_LIMIT_ENFORCEMENT_FAULT,  .data_2 =         accData->pack_current, .optype_2 = LT, .lim_2 =             0 },
            {.id = "Low Cell Voltage",        .timer =      underVolt_tmr, .data_1 = accData->min_voltage.val, .optype_1 = LT, .lim_1 =                                       MIN_VOLT * 10000, .timeout =     UNDER_VOLT_TIME,	.code =             CELL_VOLTAGE_TOO_LOW   /* -----------------------------------UNUSED---------------------------------*/  },
            {.id = "High Cell Voltage",       .timer = overVoltCharge_tmr, .data_1 = accData->max_voltage.val, .optype_1 = GT, .lim_1 =                                MAX_CHARGE_VOLT * 10000, .timeout =      OVER_VOLT_TIME, .code =            CELL_VOLTAGE_TOO_HIGH   /* -----------------------------------UNUSED---------------------------------*/  }, 
            {.id = "High Cell Voltage",       .timer =       overVolt_tmr, .data_1 = accData->max_voltage.val, .optype_1 = GT, .lim_1 =                                       MAX_VOLT * 10000, .timeout =      OVER_VOLT_TIME,	.code =            CELL_VOLTAGE_TOO_HIGH,   .data_2 = accData->is_charger_connected, .optype_2 = EQ, .lim_2 =         false }, 
            {.id = "High Temp",               .timer =       highTemp_tmr, .data_1 =    accData->max_temp.val, .optype_1 = GT, .lim_1 =                                          MAX_CELL_TEMP,	.timeout =       LOW_CELL_TIME,	.code =                     PACK_TOO_HOT   /* -----------------------------------UNUSED---------------------------------*/  }, 
            {.id = "Extremely Low Voltage",   .timer =        lowCell_tmr, .data_1 = accData->min_voltage.val, .optype_1 = LT, .lim_1 =                                                    900, .timeout =      HIGH_TEMP_TIME,	.code =                 LOW_CELL_VOLTAGE   /* -----------------------------------UNUSED---------------------------------*/  }, 

            NULL
        };


	uint32_t fault_status = 0;
    int incr = 0;

    while (&fault_table[incr] != NULL)
    {
        fault_status |= faultEval(fault_table[incr]);
        incr++;
    }

    return fault_status;
    
}

uint32_t StateMachine::faultEval(fault_eval index)
{
    bool condition1;
    bool condition2;

	index.timer.eval_length = index.timeout;

    switch (index.optype_1)
    {
        case GT: condition1 = index.data_1 > index.lim_1; break;
        case LT: condition1 = index.data_1 < index.lim_1; break;
        case GE: condition1 = index.data_1 >= index.lim_1; break;
        case LE: condition1 = index.data_1 <= index.lim_1; break;
        case EQ: condition1 = index.data_1 == index.lim_1; break;
		case NEQ: condition1 = index.data_1 != index.lim_1; break;
        case NOP: condition1 = true; break;
    }

    switch (index.optype_2)
    {
        case GT: condition2 = index.data_2 > index.lim_2; break;
        case LT: condition2 = index.data_2 < index.lim_2; break;
        case GE: condition2 = index.data_2 >= index.lim_2; break;
        case LE: condition2 = index.data_2 <= index.lim_2; break;
        case EQ: condition2 = index.data_2 == index.lim_2; break;
		case NEQ: condition2 = index.data_2 != index.lim_2; break;
        case NOP: condition2 = true; break;
    }


    if (index.timer.eval_state == BEFORE_TIMER_START && condition1 && condition2) 
    {
        index.timer.startTimer(index.timer.eval_length);
        index.timer.eval_state = DURING_EVAL;
    }

    else if (index.timer.eval_state == DURING_EVAL)
    {
        if (index.timer.isTimerExpired())
        {
            return index.code;
        }
        if (!(condition1 && condition2))
        {
            index.timer.cancelTimer();
            index.timer.eval_state = BEFORE_TIMER_START;
        }
    }

    return 0;
}

bool StateMachine::chargingCheck(AccumulatorData_t *bmsdata)
{
	if (!compute.chargerConnected()) return false;
	if(!chargeTimeout.isTimerExpired()) return false;

	if (bmsdata->max_voltage.val >= (MAX_CHARGE_VOLT * 10000) && chargeCutoffTime.eval_state == BEFORE_TIMER_START)
    {
        chargeCutoffTime.startTimer(5000);
        chargeCutoffTime.eval_state = DURING_EVAL;
    }
    else if (chargeCutoffTime.eval_state == DURING_EVAL)
    {
        if (chargeCutoffTime.isTimerExpired())
        {
			chargeTimeout.startTimer(CHARGE_TIMEOUT);
            return false;
        }
        if (!(bmsdata->max_voltage.val >= (MAX_CHARGE_VOLT * 10000)))
        {
            chargeCutoffTime.cancelTimer();
            chargeCutoffTime.eval_state = BEFORE_TIMER_START;
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

void StateMachine::preFaultCheck(AccumulatorData_t *bmsdata)
{
	//prefault for Low Cell Voltage
	if (prefaultLowCell_tmr.eval_state == BEFORE_TIMER_START && bmsdata->min_voltage.val < MIN_VOLT * 10000)
    {
        prefaultLowCell_tmr.startTimer(PRE_UNDER_VOLT_TIME);
        prefaultLowCell_tmr.eval_state = DURING_EVAL;
    }
    else if (prefaultLowCell_tmr.eval_state == DURING_EVAL)
    {
        if (prefaultLowCell_tmr.isTimerExpired())
        {
            if (prefaultCANDelay1.isTimerExpired())
			{
            	compute.sendDclPreFault(true);
				prefaultCANDelay1.startTimer(CAN_MESSAGE_WAIT);
			}
        }
        if (!(bmsdata->min_voltage.val < MIN_VOLT * 10000))
        {
            prefaultLowCell_tmr.cancelTimer();
            prefaultLowCell_tmr.eval_state = BEFORE_TIMER_START;
        }
    }

	//prefault for DCL
	if (prefaultOverCurr_tmr.eval_state == BEFORE_TIMER_START && (bmsdata->pack_current) > ((bmsdata->discharge_limit + DCDC_CURRENT_DRAW)*10*1.04)) // *104% to account for current sensor +/-A
    {
		prefaultOverCurr_tmr.startTimer(PRE_OVER_CURR_TIME);
		prefaultOverCurr_tmr.eval_state = DURING_EVAL;
    }
    else if (prefaultOverCurr_tmr.eval_state == DURING_EVAL)
    {
        if (prefaultOverCurr_tmr.isTimerExpired())
        {
			if (prefaultCANDelay2.isTimerExpired())
			{
            	compute.sendDclPreFault(true);
				prefaultCANDelay2.startTimer(CAN_MESSAGE_WAIT);
			}
        }
        if (!((bmsdata->pack_current) > ((bmsdata->discharge_limit + DCDC_CURRENT_DRAW)*10*1.04)))
        {
            prefaultOverCurr_tmr.cancelTimer();
            prefaultOverCurr_tmr.eval_state = BEFORE_TIMER_START;
        }
    }
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
