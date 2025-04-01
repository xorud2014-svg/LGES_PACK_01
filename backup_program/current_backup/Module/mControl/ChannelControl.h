#ifndef __CHANNELCONTROL_H__
#define __CHANNELCONTROL_H__

void	ChannelControl(int);

void	cState_Operation(int);

void	cIdle(int);

void	cStandby(int);
void	cStandby_sig_run(int);
void	cStandby_sig_run_2(int);
void	cStandby_sig_cable_check_1(int);
void	cStandby_sig_cable_check_2(int);
void	cStandby_sig_cable_check_11(int);
void	cStandby_sig_cable_check_12(int);
void	cStandby_sig_cable_check_13(int);
void	cStandby_sig_cable_check_21(int);
void	cStandby_sig_cable_check_22(int);
void	cStandby_sig_cable_check_23(int);
void	cStandby_sig_cable_check_31(int);
void	cStandby_sig_out_mux_select(int); //kjhw_151021

void	cRun(int);
void	cRun_sig_step_transition(int);

void	cPause(int);
//void	cPause_sig_can(int);
void	cPause_sig_stop(int);
void	cPause_sig_continue(int);
void	cPause_sig_cycle_continue(int);
void	cPause_sig_chamber_continue_1(int);
void	cPause_sig_chamber_continue_2(int);
void	cPause_sig_next_step(int);
void	cPause_sig_goto_step(int);

void	cCali(int);

void	cFault(int);

int		cBMS_Active(int, int);

void	cStepCharge(int);
void	cStepDischarge(int);
void	cStepRest(int);
void	cStepLongTimeRest(int);
void	cStepOcv(int);
void	cStepZ(int);
void	cStepPattern(int);
void	cStepExternalCan(int);
void	cStepUserMap(int); //kjhw_140828
void	cStepExternalCan2(int);
void	cStepDefault(int);
void	cStepCycle(int);
void	cStepCycle_1(int);
//kjh_211021 void	cStepCycle_2(int);
//kjh_211021 void	cStepCycle_3(int);
void	cStepLoop(int);
void	cStepLoop_1(int);
//kjh_211021 void	cStepLoop_2(int);
//kjh_211021 void	cStepLoop_3(int);
void	cStepLoop_4(int);
void	cStepEnd(int);

void	cNextStepCheck(int);
void	cUpdate_Test_Condition(int);
void	cNextStep_transition(int);
void	cNextStep_Formation(int);

int		cCable_dv_check(int);
//kjh_211021 int		cCable_dv_check_1(int);
//kjh_211021 int		cCable_dv_check_2(int);

void	cCalibrator_Select(int, int, int, int, int);
int 	cCalibrationV(int, int);
int		cCalibrationCheckV(int, int);
double	meter_measureI_1(int);
double	meter_measureI_2(int);
double	meter_measureI_3(int);
double	meter_measureI_4(int);
double	meter_measureI_default(int);
int 	cCalibrationI(int, int);
int		cCalibrationCheckI(int, int);
void	cCalculate_CaliData(int, int);
void	cCalculate_CaliData_2(int, int);	//ktg_230125
double	cCalculate_gain(long, long, long, long); //kjhw_140828
long	cCalculate_offset(long, long, double); //kjhw_140828
void	cCalculate_UserMap(int); //kjhw_140828
void	cCalculate_Temperature_Pattern(int);	//jhkw_161026

void	cActiveDivision(int);

void	cFaultCond_Check(int);
void	cFaultCond_Module(int);
void	cFaultCond_User(int);
void	cFaultCond_Hard(int);
void	cFaultCond_Soft1(int);
void	cFaultCond_Soft2(int);
void	cFaultCond_Aux(int);
int		cFaultCond_Aux_common(int, long, int);
//int		cFaultCond_Aux_step(int, int, long, int);
int		cFaultCond_Aux_step(int, int, long, int, int);
void	cFaultCond_Can(int);
int		cFaultCond_Can_common(int, float, int, int);
int		cFaultCond_Can_step(int, float, int);
int		cFaultCond_Can_100ms_step(int, float, int);	//ktg_210706
int		cFaultCond_Can_default(int, float, int);
void	Option_Compare_Pause(int);	//ktg_220331s
void	Aux_Compare_Pause(int);
void	CAN_Compare_Pause(int);
void	cFaultCond_Check_Pause(int);
void	cFaultCond_Aux_Pause(int);
void	cFaultCond_Can_Pause(int);
int		cFaultCond_Aux_common_Pause(int, long, int);
int		cFaultCond_Aux_step_Pause(int, int, long, int, int);
int		cFaultCond_Can_common_Pause(int, float, int, int);
int		cFaultCond_Can_step_Pause(int, float, int);
int		cFaultCond_Can_default_Pause(int, float, int);	//ktg_220331e
void	cFaultCond_Hard_Pause(int);
void	cFaultCond_Soft1_Pause(int);

void	cStopCond(int);

void	cEndCond(int);
void	cEndCond_Aux(int);
int		cEndCond_Aux_step(int, int, long, int);
void	cEndCond_Can(int);
int		cEndCond_Can_step(int, float, int);
int		cEndCond_Can_default(int, float, int);

void	cUserControl(int);
int		cUserControl_can_stop(int, int);
int		cUserControl_can_pause(int, int);
int		cUserControl_can_continue(int, int);
int		cUserControl_can_next(int, int);
int		cUserControl_can_goto(int, int);

void	cSaveCond(int);

//kjg_w int		cFailCodeCheck(int);
void	cSoftFeedback(int);
//kjg_w void	cSoftPID(int, long, long, long);
void	cSoftFeedback_A1(int);
void	cSoftFeedback_A2(int);
void	cSoftFeedback_A3(int);
void	cSoftFeedback_A4(int); //kjhw_140828 usermap
void	cSoftFeedback_A5(int);
void	cSoftFeedback_B1(int);
void	cSoftFeedback_C1(int);
void	cSoftFeedback_C2(int);
void	cSoftFeedback_C3(int);

void	cCalculate_Capacitance(int, int);
void	cCalculate_DCR(int, int);
void	cCalculate_CV_CC_Time(int);

//void	cSensCount(void);

int		cCodeCompare(int, int, int, long, long);
int		cCodeCompare2(int, int, char[], long, long);
int		com_cCodeCheck(int, int, long);
int		com_cCodeCheck2(int, int, long);
int		loc_cCodeCheck(int, int, long, long);
int		loc_cCodeCheck2(int, int, long, long);
int		loc_cCodeCheck3(int, int, long, int);
int		loc_cCodeCheck4(int, int, long, long);
int		loc_cCodeCheck5(int, int, unsigned long, unsigned long, long);
int		loc_cCodeCheck6(int, int, unsigned long, unsigned long, int);
int		loc_cCodeCheck_aux(int, int, long, long, int, unsigned char);	//kjhw_170904
int		loc_cCodeCheck_can(int, int, long, long, int, unsigned char);	//khj_201120
int		aux_cCodeCheck(int, int, long, long);
int		can_cCodeCheck(int, int, float, float);

void	cIO_State_Check(int);
int		bms_state_check(int);
int		bms_state_check_2(int);
int		bms_state_check_11(int);
int		bms_state_check_12(int);

int		out_mux_select(int); //kjhw_151021
void	v_rising_check(void);	//shhw_220916
int  	cc_deltaV_check(int);	//shhw_220919
int		output_watt_limited(int, int); //kjhw_130903
#ifdef __B_TYPE__
int		output_voltage_limited(int, int); //jhkw_131023
#endif


#endif
