#include "../../INC/datastore.h"
#include "common_utils.h"
#include "local_message.h"
#include "ModuleControl.h"
#include <rtl_core.h>

extern S_SYSTEM_DATA *myData;
extern S_MODULE_DATA *myPs;
extern S_DIO *myDio;

void ModuleControl(void)
{
	int group, i;

	mSignalCheck();

	switch(myPs->state) {
		case M_IDLE:
			myPs->state = M_STANDBY;
			myPs->phase = P0;
			break;
		case M_STANDBY:
			for(group=0; group < myData->AppControl.config.totalGroup;
				group++) {
				if(myData->gData[group].state == G_RUN) {
					myPs->state = M_RUN;
					myPs->phase = P0;
					myPs->code = M_CD_NONE;
					myPs->signal[M_SIG_FAN_RELAY] = P1;
					break;
				}
			}

			for(group=0; group < myData->AppControl.config.totalGroup;
				group++) {
				if(myData->gData[group].state == G_CALI) {
					myPs->state = M_CALI;
					myPs->phase = P0;
					myPs->signal[M_SIG_FAN_RELAY] = P1;
					break;
				}
			}
			break;
		case M_RUN:
			i = 0;
			for(group=0; group < myData->AppControl.config.totalGroup;
				group++) {
				if(myData->gData[group].state == G_PAUSE
					|| myData->gData[group].state == G_STANDBY) i++;
			}
			if(i >= myData->AppControl.config.totalGroup) {
				myPs->state = M_PAUSE;
				myPs->phase = P0;
				myPs->signal[M_SIG_FAN_RELAY] = P0;
				myPs->misc.fan_delay_time
					= myPs->misc.timer_1sec + myPs->misc.fan_run_time;
				break;
			}

			i = 0;
			for(group=0; group < myData->AppControl.config.totalGroup;
				group++) {
				if(myData->gData[group].state != G_RUN
					&& myData->gData[group].state != G_PAUSE) i++;
			}
			if(i >= myData->AppControl.config.totalGroup) {
				myPs->state = M_STANDBY;
				myPs->phase = P0;
				myPs->signal[M_SIG_FAN_RELAY] = P0;
				myPs->misc.fan_delay_time
					= myPs->misc.timer_1sec + myPs->misc.fan_run_time;
			}
			break;
		case M_PAUSE:
			for(group=0; group < myData->AppControl.config.totalGroup;
				group++) {
				if(myData->gData[group].state == G_RUN) {
					myPs->state = M_RUN;
					myPs->phase = P0;
					myPs->code = M_CD_NONE;
					myPs->signal[M_SIG_FAN_RELAY] = P1;
					break;
				}
			}

			i = 0;
			for(group=0; group < myData->AppControl.config.totalGroup;
				group++) {
				if(myData->gData[group].state != G_RUN
					&& myData->gData[group].state != G_PAUSE) i++;
			}
			if(i >= myData->AppControl.config.totalGroup) {
				myPs->state = M_STANDBY;
				myPs->phase = P0;
				myPs->signal[M_SIG_FAN_RELAY] = P0;
				myPs->misc.fan_delay_time
					= myPs->misc.timer_1sec + myPs->misc.fan_run_time;
			}
			break;
		case M_CALI:
			i = 0;
			for(group=0; group < myData->AppControl.config.totalGroup;
				group++) {
				if(myData->gData[group].state != G_CALI) i++;
			}
			if(i >= myData->AppControl.config.totalGroup) {
				myPs->state = M_STANDBY;
				myPs->phase = P0;
				myPs->signal[M_SIG_FAN_RELAY] = P0;
				myPs->misc.fan_delay_time
					= myPs->misc.timer_1sec + myPs->misc.fan_run_time;
			}
			break;
		case M_FAIL:
			break;
		default: break;
	}
}

void mSignalCheck(void)
{
	int group, count;
	S_MSG_VAL SendMsg;
	
	switch(myPs->signal[M_SIG_EXIT_PHASE]) {
		case P1:
			count = 0;
			for(group=0; group < myData->AppControl.config.totalGroup;
				group++) {
				if(myPs->signal[M_SIG_EXIT_TYPE] == P5) { //pause
					if(group == (int)myPs->signal[M_SIG_EXIT_GROUP]) {
						myData->gData[group].code = myPs->code;
						myData->gData[group].signal[G_SIG_EXIT_PHASE] = P1;
					}
				} else {
					myData->gData[group].code = myPs->code;
					myData->gData[group].signal[G_SIG_EXIT_PHASE] = P1;
				}
				//kjg_w? myData->gData[group].signal[G_SIG_EXIT_VALUE]
				//	= myPs->signal[M_SIG_EXIT_VALUE];
				//kjhw_160810s
				myData->gData[group].signal[G_SIG_EXIT_VALUE]
					= myPs->misc.exit_value;
				//kjhw_160810e
			}
			if(count == 0) { //kjg_101012
				myPs->signal[M_SIG_EXIT_PHASE] = P2;
			}
			break;
		case P2:
			count = 0;
			for(group=0; group < myData->AppControl.config.totalGroup;
				group++) {
				if(myData->gData[group].signal[G_SIG_EXIT_PHASE] == P0) {
					count++;
				}
			}
			if(myPs->signal[M_SIG_EXIT_TYPE] == P3
				|| myPs->signal[M_SIG_EXIT_TYPE] == P4) {
				myPs->signal[M_SIG_EXIT_PHASE] = P3;
			} else {
				if(count == myData->AppControl.config.totalGroup) {
					myPs->signal[M_SIG_EXIT_PHASE] = P3;
				}
			}
			break;
		case P3:
			if(myPs->signal[M_SIG_EXIT_TYPE] != P0
				&& myPs->signal[M_SIG_EXIT_TYPE] != P5) {
				//Quit or Shutdown or force Quit or force Shutdown
				memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
				SendMsg.msg = MSG_MODULE_IO_EXIT;
				SendMsg.val[0] = myPs->code;
				SendMsg.val[1] = (int)myPs->signal[M_SIG_EXIT_TYPE];
				send_msg(MODULE_TO_IO, (char *)&SendMsg);

				myPs->signal[M_SIG_WDT_PHASE] = P2; //kjg_logic_type_140324;
			}

			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
			SendMsg.msg = MSG_MODULE_APP_EXIT;
			SendMsg.val[0] = myPs->code;
			SendMsg.val[1] = (int)myPs->signal[M_SIG_EXIT_TYPE];
			//SendMsg.val[2] = (int)myPs->signal[M_SIG_EXIT_VALUE];
			SendMsg.val[2] = (int)myPs->misc.exit_value;
			send_msg(MODULE_TO_APP, (char *)&SendMsg);

			//kjg_130121_w myPs->signal[M_SIG_EXIT_TYPE] = P0;
			myPs->signal[M_SIG_EXIT_PHASE] = P0;
			myPs->signal[M_SIG_EXIT_GROUP] = P0;
			//myPs->signal[M_SIG_EXIT_VALUE] = P0;
			myPs->misc.exit_value = 0;
			break;
		default: break;
	}

	mSignalCheck_Cable_Check();
	mSignalCheck_Cell_Check();

	mSignalCheck_DC_FAN();	//jhkw_130924
	mSignalCheck_FAN_RELAY();
	mSignalCheck_TOWER_LAMP();
	mSignalCheck_DSP_PARALLEL();	//ktgw_211202

	mSignalCheck_DAQ_ISOLATION_REPLY_DELAY();
	mSignalCheck_DAQ_ISOLATION_REPLY_DELAY_2();
	mSignalCheck_PACK_ISOLATION_PHASE();

	mSignalCheck_BMS_EOL_PHASE();
	mSignalCheck_BMS_EOL_FLASH_PHASE();
	mSignalCheck_BMS_EOL3_PHASE();
	mSignalCheck_EOL_PROCEDURE();

}

void mSignalCheck_Cable_Check(void)
{
	int scan_ch;
	long val1, val2;
	S_MSG_VAL SendMsg;
	long ratioV, ratioI; //kjh_211021

	if(myPs->config.ratioV == MICRO_UNIT && myPs->config.ratioI == MICRO_UNIT) { //uV,uA
		ratioV = 1;
		ratioI = 1;
	} else if(myPs->config.ratioV == MILLI_UNIT
		&& myPs->config.ratioI == MILLI_UNIT) { //mV,mA
		ratioV = 1000;
		ratioI = 1000;
	} else {
		ratioV = 1;
		ratioI = 1;
	} //kjh_211021

	switch(myPs->signal[M_SIG_CABLE_CHECK_PHASE]) {
		case P1:
			scan_ch = (int)myPs->misc.cable_check.scan_ch;
			if(myData->cData[scan_ch].op.state == C_STANDBY) {
				myPs->signal[M_SIG_CABLE_CHECK_PHASE]++;
			} else {
				myPs->signal[M_SIG_CABLE_CHECK_PHASE] = P10;
			}
			break;
		case P2:
			scan_ch = (int)myPs->misc.cable_check.scan_ch;
			myData->cData[scan_ch].signal[C_SIG_CMD_RUN] = P11;
			myPs->signal[M_SIG_CABLE_CHECK_PHASE]++;
			break;
		case P3:
			scan_ch = (int)myPs->misc.cable_check.scan_ch;
			if(myData->cData[scan_ch].op.state == C_STANDBY
				&& myData->cData[scan_ch].signal[C_SIG_CMD_RUN] == P0) {
				myPs->signal[M_SIG_CABLE_CHECK_PHASE] = P10;
			}
			if(myData->cData[scan_ch].op.state == C_RUN) {
				myPs->signal[M_SIG_CABLE_CHECK_PHASE]++;
			}
			break;
		case P4:
			scan_ch = (int)myPs->misc.cable_check.scan_ch;
			if(myData->cData[scan_ch].op.state == C_STANDBY) {
				myPs->signal[M_SIG_CABLE_CHECK_PHASE] = P10;
			}
			break;
		case P10:
			myPs->misc.cable_check.scan_ch++;
			scan_ch = (int)myPs->misc.cable_check.scan_ch;
			if(scan_ch >= myPs->config.installedCh) {
				myPs->signal[M_SIG_CABLE_CHECK_PHASE] = P20;
			} else {
				myPs->signal[M_SIG_CABLE_CHECK_PHASE] = P1;
			}
			break;
		case P20:
			//cycle -> charge -> rest -> discharge -> rest -> loop -> end
			for(scan_ch=0; scan_ch < myPs->config.installedCh; scan_ch++) {
				if(myPs->misc.cable_check.Vsens[scan_ch][1] > (5000000 / ratioV))
					continue; //5V
				if(myPs->misc.cable_check.Isens[scan_ch][1] < (15000000 / ratioI))
					continue; //15A

				val1 = myPs->misc.cable_check.Vsens[scan_ch][1]
					- myPs->misc.cable_check.Vsens[scan_ch][2];
				val2 = myPs->misc.cable_check.Isens[scan_ch][1]
					- myPs->misc.cable_check.Isens[scan_ch][2];
				if(val1 < (5000 / ratioV)) continue; //5mV
				if(val2 < (15000000 / ratioI)) continue; //15A

				if(myPs->misc.cable_check.Vsens[scan_ch][3] > (5000000 / ratioV))
					continue; //5V
				if(myPs->misc.cable_check.Isens[scan_ch][3] > (-15000000 / ratioI))
					continue; //-15A

				val1 = myPs->misc.cable_check.Vsens[scan_ch][4]
					- myPs->misc.cable_check.Vsens[scan_ch][3];
				val2 = myPs->misc.cable_check.Isens[scan_ch][4]
					- myPs->misc.cable_check.Isens[scan_ch][3];
				if(val1 < (5000 / ratioV)) continue; //5mV
				if(val2 < (15000000 / ratioI)) continue; //15A

				myPs->misc.cable_check.code[scan_ch] = 1; //ok
			}

			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
			if(myData->AppControl.misc.Load_Process[PROCESS_GROUP_COA][0]
				== P1) {
				SendMsg.msg = MSG_MODULE_COA_CABLE_CHECK_REPLY;
				send_msg(MODULE_TO_COA1, (char *)&SendMsg);
			}

			if(myData->AppControl.misc.Load_Process[PROCESS_GROUP_COC][0]
				== P1) {
				SendMsg.msg = MSG_MODULE_COC_CABLE_CHECK_REPLY;
				send_msg(MODULE_TO_COC1, (char *)&SendMsg);
			}
			myPs->signal[M_SIG_CABLE_CHECK_PHASE] = P0;
			break;
		default: break;
	}
}

void mSignalCheck_Cell_Check(void)
{
	int scan_ch;
	S_MSG_VAL SendMsg;

	switch(myPs->signal[M_SIG_CELL_CHECK_PHASE]) {
		case P1:
			scan_ch = (int)myPs->misc.cell_check.scan_ch;
			if(myData->cData[scan_ch].op.state == C_STANDBY
				|| myData->cData[scan_ch].op.state == C_PAUSE) {
				myPs->signal[M_SIG_CELL_CHECK_PHASE]++;
			} else {
				myPs->signal[M_SIG_CELL_CHECK_PHASE] = P10;
			}
			break;
		case P2:
			scan_ch = (int)myPs->misc.cell_check.scan_ch;
			myData->cData[scan_ch].signal[C_SIG_CMD_RUN] = P21;
			myPs->signal[M_SIG_CELL_CHECK_PHASE]++;
			break;
		case P3:
			scan_ch = (int)myPs->misc.cell_check.scan_ch;
			if(myData->cData[scan_ch].signal[C_SIG_CMD_RUN] == P22) {
				myData->cData[scan_ch].signal[C_SIG_CMD_RUN] = P0;
				myPs->misc.cell_check.code[scan_ch] = 1; //ok
				myPs->signal[M_SIG_CELL_CHECK_PHASE] = P10;
			} else if(myData->cData[scan_ch].signal[C_SIG_CMD_RUN] == P0) {
				myPs->signal[M_SIG_CELL_CHECK_PHASE] = P10;
			}
			break;
		case P10:
			myPs->misc.cell_check.scan_ch++;
			scan_ch = (int)myPs->misc.cell_check.scan_ch;
			if(scan_ch >= myPs->config.installedCh) {
				myPs->signal[M_SIG_CELL_CHECK_PHASE] = P20;
			} else {
				myPs->signal[M_SIG_CELL_CHECK_PHASE] = P1;
			}
			break;
		case P20:
			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
			if(myData->AppControl.misc.Load_Process[PROCESS_GROUP_COA][0]
				== P1) {
				SendMsg.msg = MSG_MODULE_COA_CELL_CHECK_REPLY;
				send_msg(MODULE_TO_COA1, (char *)&SendMsg);
			}

			if(myData->AppControl.misc.Load_Process[PROCESS_GROUP_COC][0]
				== P1) {
				SendMsg.msg = MSG_MODULE_COC_CELL_CHECK_REPLY;
				send_msg(MODULE_TO_COC1, (char *)&SendMsg);
			}
			myPs->signal[M_SIG_CELL_CHECK_PHASE] = P0;
			break;
		default: break;
	}
}

void mSignalCheck_DC_FAN(void)
{
	int ch;
	//jhkw_180304s
	unsigned char fan_active_flag;
	long stop_count, run_count;
	long scan_period; //kjh_211021
	
	//stop_count = 180000;	//30min
	//run_count = 18000;	//3min
	//kjh_211021 stop_count = 60000;	//10min
	//kjh_211021 run_count = 36000;	//6min
	stop_count = 600000;	//10min	//kjh_211021
	run_count = 360000;	//6min	//kjh_211021
	fan_active_flag = 0;

	scan_period = myPs->config.scan_period;	//kjh_211021
	
	for(ch=0; ch < myPs->config.installedCh; ch++) {
		if((myData->cData[ch].op.state == C_RUN 
			&& myData->cData[ch].op.stepType != STEP_CYCLE
			&& myData->cData[ch].op.stepType != STEP_LOOP
			&& myData->cData[ch].op.stepType != STEP_OCV
			&& myData->cData[ch].op.stepType != STEP_REST
			&& myData->cData[ch].op.stepType != STEP_LONG_TIME_REST)
			|| myData->cData[ch].op.state == C_CALI) {
			fan_active_flag++;
		} else {
		}
	}
	if(fan_active_flag >= 1) {
		myPs->misc.fan_stop_count = 0;
		myPs->misc.fan_run_count = 0;
		myPs->misc.fan_active_flag_run = 0;
		Select_OutPoint(0, 1, O_OUT_DC_FAN, ON);
	} else {
		if(myPs->misc.fan_run_count * scan_period > run_count) {
			myPs->misc.fan_run_count = 0;
			myPs->misc.fan_active_flag_run = 1;
			Select_OutPoint(0, 1, O_OUT_DC_FAN, OFF);
		} else {
			if(myPs->misc.fan_stop_count * scan_period > stop_count) {
				myPs->misc.fan_active_flag_run = 0;
				myPs->misc.fan_stop_count = 0;
				Select_OutPoint(0, 1, O_OUT_DC_FAN, ON);
			} else {
			}
		}
		if(myPs->misc.fan_active_flag_run == 0) {
			myPs->misc.fan_run_count++;
		} else {
			myPs->misc.fan_stop_count++;
		}
	}
}

void mSignalCheck_DSP_PARALLEL(void)	//ktgw_211202
{
	switch(myPs->signal[M_SIG_PARALLER_MODE]) {
		case P0:
			if(myData->ChAttribute[0].opType != OP_INDEPENDENT) {
				Select_OutPoint(0, 1, O_OUT_PARALLEL_MODE, ON);
				if(myData->ChAttribute[0].chNo_slave[1] == 3) {
					Select_OutPoint(0, 2, O_OUT_PARALLEL_MODE, ON);
					if(myData->ChAttribute[0].chNo_slave[2] == 4) {
						Select_OutPoint(0, 4, O_OUT_PARALLEL_MODE, ON);
					} else {
						Select_OutPoint(0, 4, O_OUT_PARALLEL_MODE, OFF);
					}	
				} else {
					Select_OutPoint(0, 2, O_OUT_PARALLEL_MODE, OFF);
				}
			} else {
				Select_OutPoint(0, 1, O_OUT_PARALLEL_MODE, OFF);
				Select_OutPoint(0, 2, O_OUT_PARALLEL_MODE, OFF);
				Select_OutPoint(0, 4, O_OUT_PARALLEL_MODE, OFF);
			}
			if(myData->ChAttribute[2].opType != OP_INDEPENDENT) {
				if(myData->ChAttribute[2].chNo_slave[0] == 4) {
					Select_OutPoint(0, 3, O_OUT_PARALLEL_MODE, ON);
				} else {
					Select_OutPoint(0, 3, O_OUT_PARALLEL_MODE, OFF);
				}
			} else {
				Select_OutPoint(0, 3, O_OUT_PARALLEL_MODE, OFF);
			}
			myPs->signal[M_SIG_PARALLER_MODE] = P1;
			break;
		default: break;
	}
}

void mSignalCheck_FAN_RELAY(void)
{
	int i, group, ch;

	if(myDio->config.dio_Control_Flag == P0) return;
	if(myDio->misc.delayTimer < myDio->config.dioDelay) return;

	group = 0; ch = 0;

	if(myPs->signal[M_SIG_FAN_RELAY] == P0) { //OFF
		if(myPs->misc.timer_1sec < myPs->misc.fan_run_time) {
			if(myPs->signal[M_SIG_FAN_ACTIVE] == P0) {
				myPs->signal[M_SIG_FAN_ACTIVE] = P1;
			}
		} else if((myPs->misc.timer_1sec - myPs->misc.fan_delay_time)
			> myPs->misc.fan_stop_time) {
			myPs->misc.fan_delay_time
				= myPs->misc.timer_1sec + myPs->misc.fan_run_time;
			if(myPs->signal[M_SIG_FAN_ACTIVE] == P0) {
				myPs->signal[M_SIG_FAN_ACTIVE] = P1;
			}
		} else {
			if(myPs->misc.fan_delay_time < myPs->misc.timer_1sec) {
				myPs->signal[M_SIG_FAN_ACTIVE] = P0;
			}
		}
	} else { //ON
		myPs->misc.fan_delay_time = myPs->misc.timer_1sec;
		if(myPs->signal[M_SIG_FAN_ACTIVE] == P0) {
			myPs->signal[M_SIG_FAN_ACTIVE] = P1;
		}
	}

	switch(myPs->signal[M_SIG_FAN_ACTIVE]) {
		case P0:
			myPs->misc.fan_fail_detect_time = myPs->misc.timer_1sec;
			for(i=0; i < 8; i++) Select_OutPoint(group, ch, O_FAN1 + i, OFF);
			break;
		case P1:
			myPs->misc.fan_active_time = myPs->misc.timer_1sec;
			Select_OutPoint(group, ch, O_FAN1, ON);
			Select_OutPoint(group, ch, O_FAN5, ON);
			myPs->signal[M_SIG_FAN_ACTIVE]++;
			break;
		case P2:
			if(myPs->misc.fan_active_time != myPs->misc.timer_1sec) {
				myPs->misc.fan_active_time = myPs->misc.timer_1sec;
				Select_OutPoint(group, ch, O_FAN2, ON);
				Select_OutPoint(group, ch, O_FAN6, ON);
				myPs->signal[M_SIG_FAN_ACTIVE]++;
			}
			break;
		case P3:
			if(myPs->misc.fan_active_time != myPs->misc.timer_1sec) {
				myPs->misc.fan_active_time = myPs->misc.timer_1sec;
				Select_OutPoint(group, ch, O_FAN3, ON);
				Select_OutPoint(group, ch, O_FAN7, ON);
				myPs->signal[M_SIG_FAN_ACTIVE]++;
			}
			break;
		case P4:
			if(myPs->misc.fan_active_time != myPs->misc.timer_1sec) {
				myPs->misc.fan_active_time = myPs->misc.timer_1sec;
				Select_OutPoint(group, ch, O_FAN4, ON);
				Select_OutPoint(group, ch, O_FAN8, ON);
				myPs->signal[M_SIG_FAN_ACTIVE] = P10;
			}
			break;
		default:
			break;
	}
}

void mSignalCheck_TOWER_LAMP(void)
{
	int group, ch;

	if(myDio->config.dio_Control_Flag == P0) return;
	if(myDio->misc.delayTimer < myDio->config.dioDelay) return;

	myPs->signal[M_SIG_TOWER_LAMP_GROUP] = 0;
	myPs->signal[M_SIG_TOWER_LAMP_CH] = 0;
	
	switch(myData->AppControl.config.systemModel) {
		case C_SDI_70V_50A_5A_4KW:
		case C_SDI_70V_50A_5A_4KW_2:
		case C_SDI_70V_50A_5A_7KW:
		case C_SDI_70V_50A_5A_7KW_2:
		case C_SDI_70V_250A_25A_18KW:
			mSignalCheck_TOWER_LAMP4();
			break;
		case C_NEXCON_500V_200A_10A:
		case C_SEBANG_600V_200A_100A_240KW:	//jhk_160223
		case C_SEBANG_1200V_300A_100A_360KW:	//jhk_160223
			mSignalCheck_TOWER_LAMP1();
			break;
		default:
			mSignalCheck_TOWER_LAMP1();
			break;
	}

	group = (int)myPs->signal[M_SIG_TOWER_LAMP_GROUP];
	ch = (int)myPs->signal[M_SIG_TOWER_LAMP_CH];

	if(myPs->signal[M_SIG_TOWER_LAMP_BUZZER] == 0)
		Select_OutPoint(group, ch, O_TOWER_LAMP_BUZZER, OFF);
	else Select_OutPoint(group, ch, O_TOWER_LAMP_BUZZER, ON);

	if(myPs->signal[M_SIG_TOWER_LAMP_RED] == 0)
		Select_OutPoint(group, ch, O_TOWER_LAMP_RED, OFF);
	else Select_OutPoint(group, ch, O_TOWER_LAMP_RED, ON);

	if(myPs->signal[M_SIG_TOWER_LAMP_AMBER] == 0)
		Select_OutPoint(group, ch, O_TOWER_LAMP_AMBER, OFF);
	else Select_OutPoint(group, ch, O_TOWER_LAMP_AMBER, ON);

	if(myPs->signal[M_SIG_TOWER_LAMP_GREEN] == 0)
		Select_OutPoint(group, ch, O_TOWER_LAMP_GREEN, OFF);
	else Select_OutPoint(group, ch, O_TOWER_LAMP_GREEN, ON);

	if(myPs->signal[M_SIG_TOWER_LAMP_BLUE] == 0)
		Select_OutPoint(group, ch, O_TOWER_LAMP_BLUE, OFF);
	else Select_OutPoint(group, ch, O_TOWER_LAMP_BLUE, ON);
}

void mSignalCheck_TOWER_LAMP1(void)
{
	int ch, flag_red, flag_amber, flag_green;

	flag_red = flag_amber = flag_green = 0;

	for(ch=0; ch < myPs->config.chInGroup[0]; ch++) {
		if(myData->cData[ch].op.state == C_RUN) {
			flag_green++;
		} else if(myData->cData[ch].op.state == C_PAUSE
			|| myData->cData[ch].op.state == C_FAULT) {
			//flag_amber++;
			//flag_ref++;
			if(myData->cData[ch].op.code == C_CD_END_STEP) {
			} else if(myData->cData[ch].op.code == C_CD_CHAMBER_CONTROL_WAIT) {	//csk_190719
			} else {
				flag_red++;
			}

			if(myData->cData[ch].op.code >= C_CD_FAULT_HARD_START
				&& myData->cData[ch].op.code <= C_CD_FAULT_EXTERNAL_END)
				flag_red++;
		} else {
			flag_amber++;
		}
	}

	if(flag_red >= 1) {	//jhkw_160501s
		flag_amber = flag_green = 0;
	} else if(flag_green >= 1) {
		flag_amber = 0;
	} else {
	}	//jhkw_160501e

	if(flag_red == 0) {
		myPs->signal[M_SIG_TOWER_LAMP_RED] = OFF;
		myPs->signal[M_SIG_TOWER_LAMP_BUZZER] = OFF;
		if(myPs->signal[M_SIG_BUZZER_SET] == P2) {	//csk_190621
			myPs->signal[M_SIG_BUZZER_SET] = P0;
		}
	} else {
		myPs->signal[M_SIG_TOWER_LAMP_RED] = ON;
		myPs->signal[M_SIG_TOWER_LAMP_BUZZER] = ON;
		if(myPs->signal[M_SIG_BUZZER_SET] == P0) {	//csk_190621
			myPs->signal[M_SIG_BUZZER_SET] = P1;
		}
	}

	if((myPs->signal[M_SIG_BUZZER_SET] == P0)
		|| (myPs->signal[M_SIG_BUZZER_SET] == P2))		//csk_190617
		myPs->signal[M_SIG_TOWER_LAMP_BUZZER] = OFF;	//csk_190617
	if(flag_amber == 0) myPs->signal[M_SIG_TOWER_LAMP_AMBER] = OFF;
	else myPs->signal[M_SIG_TOWER_LAMP_AMBER] = ON;

	if(flag_green == 0) myPs->signal[M_SIG_TOWER_LAMP_GREEN] = OFF;
	else myPs->signal[M_SIG_TOWER_LAMP_GREEN] = ON;
}

void mSignalCheck_TOWER_LAMP2(void)
{
	int ch, flag_buzzer, flag_red, flag_amber, flag_green;

	flag_buzzer = flag_red = flag_amber = flag_green = 0;
	ch = 0;

	if(myData->cData[ch].op.state == C_RUN
		|| myData->cData[ch].op.state == C_CALI) {
		flag_green++;
	} else if(myData->cData[ch].op.state == C_STANDBY
		|| myData->cData[ch].op.state == C_PAUSE) {
		flag_amber++;
	}

	if(myPs->code == M_CD_FAULT_AC_POWER_SHORT
		|| myPs->code == M_CD_FAULT_WARNING_POWER_OFF
		|| myPs->code == M_CD_FAULT_CALI_METER_COMM_ERROR
		|| myPs->code == M_CD_FAULT_CALIBRATOR_COMM_ERROR
		|| myPs->code == M_CD_FAULT_FAN
		|| myPs->code == M_CD_FAULT_LOAD_LINE
		|| myPs->code == M_CD_FAULT_LOAD_LINE2
		|| myPs->code == G_CD_FAULT_UPPER_VOLTAGE
		|| myPs->code == G_CD_FAULT_UPPER_CURRENT
		|| myPs->code == G_CD_FAULT_RUN_TIME_OVER
		|| myPs->code == G_CD_FAULT_DATASAVE_PROCESS_ERROR
		) { //Warning
	} else if(myPs->code == G_CD_FAULT_JIG_ACTIVE_ERROR
		|| myPs->code == G_CD_FAULT_JIG_TRAY_ERROR
		|| myPs->code == G_CD_FAULT_JIG_DOOR_ERROR
		|| myPs->code == G_CD_FAULT_JIG_AIR_PRESS_ERROR
		|| myPs->code == G_CD_FAULT_JIG_STACKER_ERROR
		) { //Pause
		if(myPs->state == M_STANDBY) {
		} else {
			flag_red++;
		}
	} else if(myPs->code == M_CD_NONE
		|| myPs->code == M_CD_FAULT_NORMAL_TERMINAL_QUIT
		|| myPs->code == M_CD_FAULT_NORMAL_POWER_OFF
		) { //Quit
	} else if(myPs->code == M_CD_FAULT_FORCE_TERMINAL_QUIT
		) { //force Quit
		flag_red++;
	} else if(myPs->code == M_CD_FAULT_FORCE_TERMINAL_HALT
		|| myPs->code == M_CD_FAULT_FORCE_POWER_OFF
		) { //force Shutdown
		flag_red++;
	} else { //Shutdown
		flag_red++;
		flag_buzzer++;
	}

	if(flag_buzzer == 0) myPs->signal[M_SIG_TOWER_LAMP_BUZZER] = OFF;
	else myPs->signal[M_SIG_TOWER_LAMP_BUZZER] = ON;

	if(flag_red == 0) myPs->signal[M_SIG_TOWER_LAMP_RED] = OFF;
	else myPs->signal[M_SIG_TOWER_LAMP_RED] = ON;

	if(flag_amber == 0) myPs->signal[M_SIG_TOWER_LAMP_AMBER] = OFF;
	else myPs->signal[M_SIG_TOWER_LAMP_AMBER] = ON;

	if(flag_green == 0) myPs->signal[M_SIG_TOWER_LAMP_GREEN] = OFF;
	else myPs->signal[M_SIG_TOWER_LAMP_GREEN] = ON;
}

void mSignalCheck_TOWER_LAMP3(void)
{
	int flag_buzzer, flag_red, flag_amber, flag_green;

	flag_buzzer = flag_red = flag_amber = flag_green = 0;

	if(myPs->state == M_RUN || myPs->state == M_CALI) {
		flag_green++;
	} else if(myPs->state == M_STANDBY || myPs->state == M_PAUSE) {
		flag_amber++;
	}

	if(myPs->code == M_CD_FAULT_AC_POWER_SHORT
		|| myPs->code == M_CD_FAULT_WARNING_POWER_OFF
		|| myPs->code == M_CD_FAULT_CALI_METER_COMM_ERROR
		|| myPs->code == M_CD_FAULT_CALIBRATOR_COMM_ERROR
		|| myPs->code == M_CD_FAULT_FAN
		|| myPs->code == M_CD_FAULT_LOAD_LINE
		|| myPs->code == M_CD_FAULT_LOAD_LINE2
		|| myPs->code == G_CD_FAULT_UPPER_VOLTAGE
		|| myPs->code == G_CD_FAULT_UPPER_CURRENT
		|| myPs->code == G_CD_FAULT_RUN_TIME_OVER
		|| myPs->code == G_CD_FAULT_DATASAVE_PROCESS_ERROR
		) { //Warning
	} else if(myPs->code == G_CD_FAULT_JIG_ACTIVE_ERROR
		|| myPs->code == G_CD_FAULT_JIG_TRAY_ERROR
		|| myPs->code == G_CD_FAULT_JIG_DOOR_ERROR
		|| myPs->code == G_CD_FAULT_JIG_AIR_PRESS_ERROR
		|| myPs->code == G_CD_FAULT_JIG_STACKER_ERROR
		) { //Pause
		if(myPs->state == M_STANDBY) {
		} else {
			flag_red++;
		}
	} else if(myPs->code == M_CD_NONE
		|| myPs->code == M_CD_FAULT_NORMAL_TERMINAL_QUIT
		|| myPs->code == M_CD_FAULT_NORMAL_POWER_OFF
		) { //Quit
	} else if(myPs->code == M_CD_FAULT_FORCE_TERMINAL_QUIT
		) { //force Quit
		flag_red++;
	} else if(myPs->code == M_CD_FAULT_FORCE_TERMINAL_HALT
		|| myPs->code == M_CD_FAULT_FORCE_POWER_OFF
		) { //force Shutdown
		flag_red++;
	} else { //Shutdown
		flag_red++;
		flag_buzzer++;
	}

	if(flag_buzzer == 0) myPs->signal[M_SIG_TOWER_LAMP_BUZZER] = OFF;
	else myPs->signal[M_SIG_TOWER_LAMP_BUZZER] = ON;

	if(flag_red == 0) myPs->signal[M_SIG_TOWER_LAMP_RED] = OFF;
	else myPs->signal[M_SIG_TOWER_LAMP_RED] = ON;

	if(flag_amber == 0) myPs->signal[M_SIG_TOWER_LAMP_AMBER] = OFF;
	else myPs->signal[M_SIG_TOWER_LAMP_AMBER] = ON;

	if(flag_green == 0) myPs->signal[M_SIG_TOWER_LAMP_GREEN] = OFF;
	else myPs->signal[M_SIG_TOWER_LAMP_GREEN] = ON;
}

void mSignalCheck_TOWER_LAMP4(void)
{
	int ch, flag_amber, flag_green;//, flag_red, flag_buzzer;

	flag_amber = flag_green = 0;// = flag_red = flag_buzzer = 0;

	for(ch=0; ch < myPs->config.chInGroup[0]; ch++) {
		if(myData->cData[ch].op.state == C_RUN
			|| myData->cData[ch].op.state == C_CALI) {
			flag_green++;
		} else if(myData->cData[ch].op.state == C_STANDBY
			|| myData->cData[ch].op.state == C_PAUSE) {
			flag_amber++;
		}
	}

	if(myPs->signal[M_SIG_BUZZER_SET] == P0)
		myPs->signal[M_SIG_TOWER_LAMP_BUZZER] = OFF;
	else myPs->signal[M_SIG_TOWER_LAMP_BUZZER] = ON;

	if(myPs->signal[M_SIG_ALARM_SET] == P0)
		myPs->signal[M_SIG_TOWER_LAMP_RED] = OFF;
	else myPs->signal[M_SIG_TOWER_LAMP_RED] = ON;

	if(flag_amber == 0) myPs->signal[M_SIG_TOWER_LAMP_AMBER] = OFF;
	else myPs->signal[M_SIG_TOWER_LAMP_AMBER] = ON;

	if(flag_green == 0) myPs->signal[M_SIG_TOWER_LAMP_GREEN] = OFF;
	else myPs->signal[M_SIG_TOWER_LAMP_GREEN] = ON;
}

void mSignalCheck_DAQ_ISOLATION_REPLY_DELAY(void)
{

	int group;
	S_MSG_VAL SendMsg;

	group = 0;
	switch(myPs->signal[M_SIG_DAQ_ISOLATION_REPLY_DELAY]) {
		case P0:
			break;
		case P200:
			myPs->signal[M_SIG_DAQ_ISOLATION_REPLY_DELAY] = P0;

			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
			SendMsg.msg = MSG_MODULE_COA_DAQ_ISOLATION_REPLY;
			SendMsg.val[0] = group;
			SendMsg.val[1] = (int)myData->SubSensV
				.signal[SUB_SENS_SIG_DAQ_ISOLATION];
			send_msg(MODULE_TO_COA1+group, (char *)&SendMsg);
			break;
		default:
			myPs->signal[M_SIG_DAQ_ISOLATION_REPLY_DELAY]++;
			break;
	}

}

void mSignalCheck_PACK_ISOLATION_PHASE(void)
{

	int group, ch;
	S_MSG_VAL SendMsg;
	long scan_period;	//kjh_211021

	scan_period = myPs->config.scan_period;	//kjh_211021
	
	group = 0;
	switch(myPs->signal[M_SIG_PACK_ISOLATION_PHASE]) {
		case P1:
			myPs->signal[M_SIG_PACK_ISOLATION_COUNT]++;
			if(myPs->signal[M_SIG_PACK_ISOLATION_COUNT] * scan_period > 500) { //500ms
				myPs->signal[M_SIG_PACK_ISOLATION_COUNT] = 0;
				myPs->signal[M_SIG_PACK_ISOLATION_PHASE] = P0;
				break;
			}
			ch = myPs->signal[M_SIG_DAQ_ISOLATION_MAIN_CH];
			if(Read_InPoint(group, ch, I_OUT_PACK_ISOLATION) == OFF) {
				myPs->signal[M_SIG_PACK_ISOLATION_PHASE]++;
			}
			break;
		case P2:
			ch = myPs->signal[M_SIG_DAQ_ISOLATION_MAIN_CH];
			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
			SendMsg.msg = MSG_MODULE_COA_OUT_PACK_ISOLATION;
			SendMsg.val[0] = 1;
			SendMsg.val[1] = ch;
			send_msg(MODULE_TO_COA1+group, (char *)&SendMsg);
			myPs->signal[M_SIG_PACK_ISOLATION_PHASE] = P0;
			break;
		case P11:
			ch = myPs->signal[M_SIG_DAQ_ISOLATION_MAIN_CH];
			myPs->signal[M_SIG_PACK_ISOLATION_COUNT]++;
			if(myPs->signal[M_SIG_PACK_ISOLATION_COUNT] * scan_period > 500) { //500ms
				myPs->signal[M_SIG_PACK_ISOLATION_COUNT] = 0;
				myPs->signal[M_SIG_PACK_ISOLATION_PHASE] = P0;
				break;
			}
			if(Read_InPoint(group, ch, I_OUT_PACK_ISOLATION) == ON) {
				myPs->signal[M_SIG_PACK_ISOLATION_PHASE]++;
			}
			break;
		case P12:
			ch = myPs->signal[M_SIG_DAQ_ISOLATION_MAIN_CH];
			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
			SendMsg.msg = MSG_MODULE_COA_OUT_PACK_ISOLATION;
			SendMsg.val[0] = 0;
			SendMsg.val[1] = ch;
			send_msg(MODULE_TO_COA1+group, (char *)&SendMsg);
			myPs->signal[M_SIG_PACK_ISOLATION_PHASE] = P0;
			break;
		default:	break;
	}
}

//jhkw_150224s
void mSignalCheck_DAQ_ISOLATION_REPLY_DELAY_2(void)
{

	int group;
	int flag1, flag2, flag3, flag4, flag5, iso_state, ch;	//jhkw_160602
	S_MSG_VAL SendMsg;

	iso_state = 0;
	group = 0;
	ch = 0;
	switch(myPs->signal[M_SIG_DAQ_ISOLATION_REPLY_DELAY2]) {
		case P0:
			break;
		case P200:
			ch = myPs->signal[M_SIG_DAQ_ISOLATION_MAIN_CH2];
			myPs->signal[M_SIG_DAQ_ISOLATION_REPLY_DELAY2] = P0;
			if(myPs->signal[M_SIG_DAQ_ISOLATION_DIV_CH] == P10) {
			    iso_state = 10;
			} else {
				flag1 = Read_InPoint(group, ch, I_OUT_PACK_ISOLATION2);
				flag2 = Read_InPoint(group, ch, I_OUT_PACK_ISOLATION3);
				flag3 = Read_InPoint(group, ch, I_OUT_PACK_ISOLATION4);
				flag4 = Read_InPoint(group, ch, I_OUT_PACK_ISOLATION5);
				flag5 = Read_InPoint(group, ch, I_OUT_PACK_ISOLATION6);
				if(flag1 == 1) iso_state += 1;
				if(flag2 == 1) iso_state += 2;
				if(flag3 == 1) iso_state += 3;
				if(flag4 == 1) iso_state += 4;
				if(flag5 == 1) iso_state += 5;
			}

			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
			SendMsg.msg = MSG_MODULE_COA_DAQ_ISOLATION_REPLY2;
			SendMsg.val[0] = group;
			SendMsg.val[1] = (int)myData->SubSensV
				.signal[SUB_SENS_SIG_DAQ_ISOLATION];
			SendMsg.val[2] = iso_state;
			SendMsg.val[3] = ch;
			send_msg(MODULE_TO_COA1+group, (char *)&SendMsg);
			break;
		default:
			myPs->signal[M_SIG_DAQ_ISOLATION_REPLY_DELAY2]++;
			break;
	}
}	//jhkw_150224e

void mSignalCheck_BMS_EOL_PHASE(void)
{
	int i, index, group=0, can_ch1, can_ch2;
	S_MSG_VAL SendMsg;

	switch(myData->AppControl.config.systemModel) { //kjg_130314
		case C_LGC_450V_200A_10A_4:
		case C_LGC_450V_200A_10A_5:
		case C_LGC_450V_200A_10A_6:
		case C_LGC_450V_200A_10A_180KW:
		case C_LGC_450V_200A_10A_180KW_2:
		case C_LGC_450V_200A_10A_180KW_3:	//kjg_130415
		case C_LGC_450V_200A_10A_180KW_4:	//jhkw_130909
		case C_LGC_450V_200A_20A_180KW:
		case C_LGC_450V_200A_20A_180KW_2:
			can_ch1 = 2;
			can_ch2 = 3;
			break;
		default:
			//kjhw_140620s
			if(myData->mData.config.division_CAN == 1) {
				can_ch1 = 0;
				can_ch2 = 0;
			} else {
				can_ch1 = 0;
				can_ch2 = 1;
			}	//kjhw_140620e
			break;
	}

	switch(myPs->signal[M_SIG_BMS_EOL_PHASE]) {
		case P1:
			memset((char *)&myPs->bms_eol_data, 0, sizeof(S_BMS_EOL_DATA));

			myPs->signal[M_SIG_BMS_EOL_DELAY1] = 0;
			myPs->signal[M_SIG_BMS_EOL_DELAY2] = 0;
			myData->CAN.signal[can_ch1][CAN_SIG_TX_PHASE_UDS_DATA1] = 29;
			myPs->signal[M_SIG_BMS_EOL_PHASE]++;
			break;
		case P2:
			if(myPs->signal[M_SIG_BMS_EOL_DELAY2] >= 10) {
				myPs->signal[M_SIG_BMS_EOL_DELAY1]++;
				if(myPs->signal[M_SIG_BMS_EOL_DELAY1] >= 100) {
					myPs->signal[M_SIG_BMS_EOL_DELAY1] = 0;
					myPs->signal[M_SIG_BMS_EOL_DELAY2]++;
				}
			}

			if(myPs->signal[M_SIG_BMS_EOL_DELAY2] == 20) {
				if(myPs->signal[M_SIG_BMS_EOL_DELAY1] == 0) {
					//cvtn_id assing
					myData->CAN.signal[can_ch1][CAN_SIG_TX_PHASE_UDS_DATA1] = 1;
				}
			} else if(myPs->signal[M_SIG_BMS_EOL_DELAY2] == 21) {
				//all off
				if(myPs->signal[M_SIG_BMS_EOL_DELAY1] == 0) {
					//myData->CAN.signal[can_ch1][CAN_SIG_TX_PHASE_MAIN_OFF] = 9;
					myData->CAN.signal[can_ch2][CAN_SIG_TX_PHASE_MAIN_OFF] = 9;
				} else if(myPs->signal[M_SIG_BMS_EOL_DELAY1] == 1) {
					myData->CAN.signal[can_ch1][CAN_SIG_TX_PHASE_UDS_DATA1] = 1;
				}
			} else if(myPs->signal[M_SIG_BMS_EOL_DELAY2] == 22) {
				if(myPs->signal[M_SIG_BMS_EOL_DELAY1] == 0) {
					myData->CAN.signal[can_ch1][CAN_SIG_TX_PHASE_AUX_OFF] = 28;
					//myData->CAN.signal[can_ch2][CAN_SIG_TX_PHASE_AUX_OFF] = 28;
				} else if(myPs->signal[M_SIG_BMS_EOL_DELAY1] == 1) {
					myData->CAN.signal[can_ch1][CAN_SIG_TX_PHASE_UDS_DATA1] = 1;
				}
			} else if(myPs->signal[M_SIG_BMS_EOL_DELAY2] == 30) {
				if(myPs->signal[M_SIG_BMS_EOL_DELAY1] == 0) {
					//cvtn_id check
					myData->CAN.signal[can_ch1][CAN_SIG_TX_PHASE_UDS_DATA1] = 2;
					myPs->signal[M_SIG_BMS_EOL_DELAY1] = 0;
					myPs->signal[M_SIG_BMS_EOL_DELAY2] = 0;
					myPs->signal[M_SIG_BMS_EOL_PHASE]++;
				}
			}
			break;
		case P3:
			break;
		case P4:
			//sw_version
			myData->CAN.signal[can_ch1][CAN_SIG_TX_PHASE_UDS_DATA1] = 3;
			myPs->signal[M_SIG_BMS_EOL_PHASE]++;
			break;
		case P5:
			break;
		case P6:
			//dtc
			myData->CAN.signal[can_ch1][CAN_SIG_TX_PHASE_UDS_DATA1] = 4;
			myPs->signal[M_SIG_BMS_EOL_PHASE]++;
			break;
		case P7:
			break;
		case P8:
			//dtc list
			myData->CAN.signal[can_ch1][CAN_SIG_TX_PHASE_UDS_DATA1] = 19;
			myPs->signal[M_SIG_BMS_EOL_PHASE]++;

			memset((char *)&myPs->misc.dtc_data, 0, 2048);
			myPs->misc.dtc_data_index = 0;
			myData->CAN.signal[can_ch1][CAN_SIG_TX_PHASE_UDS_DATA_INDEX] = 0;
			break;
		case P9:
			break;
		case P10:
			if(myPs->signal[M_SIG_BMS_EOL_CVTN_ID_FLAG] == P1) {
				memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
				SendMsg.msg = MSG_MODULE_COA_BMS_EOL_CVTN_ID_REPLY;
				send_msg(MODULE_TO_COA1+group, (char *)&SendMsg);
				myPs->signal[M_SIG_BMS_EOL_PHASE] = P0;
			} else {
				//module delta temp
				myData->CAN.signal[can_ch1][CAN_SIG_TX_PHASE_UDS_DATA1] = 5;
				myPs->signal[M_SIG_BMS_EOL_PHASE]++;
			}
			break;
		case P11:
			break;
		case P12:
			//thermistor sensor
			myData->CAN.signal[can_ch1][CAN_SIG_TX_PHASE_UDS_DATA1] = 6;
			myPs->signal[M_SIG_BMS_EOL_PHASE]++;
			break;
		case P13:
			break;
		case P14:
			//cell delta v + HvBattHvil_D_Fault
			myData->CAN.signal[can_ch1][CAN_SIG_TX_PHASE_UDS_DATA1] = 7;
			myPs->signal[M_SIG_BMS_EOL_PHASE]++;
			break;
		case P15:
			break;
		case P16: //0:all off
			//aux + main + negative contactor
			myPs->signal[M_SIG_BMS_EOL_DELAY1]++;
			if(myPs->signal[M_SIG_BMS_EOL_DELAY1] >= 100) {
				myPs->signal[M_SIG_BMS_EOL_DELAY1] = 0;
				myPs->signal[M_SIG_BMS_EOL_DELAY2]++;
			}

			if(myPs->signal[M_SIG_BMS_EOL_DELAY2] == 1
				&& myPs->signal[M_SIG_BMS_EOL_DELAY1] == 0) {
				myPs->signal[M_SIG_BMS_EOL_DELAY1] = 0;
				myPs->signal[M_SIG_BMS_EOL_DELAY2] = 0;
				myPs->signal[M_SIG_BMS_EOL_PHASE]++;
			}
			break;
		case P17:
			for(i=0; i < myData->canReceiveDataCount[can_ch1][0]; i++) {
				index = i;

				if(myData->canReceiveSetData.normalData[can_ch1][index].function_div
					== CAN_RX_FUNC_DIV_BMS_EOL_AUX_CONTACT) {
					myPs->bms_eol_data.HvBattAuxCntct_D_Actl[0]
						= (unsigned char)myData->CanData[can_ch1][index].f_val[0];
				} else if(myData->canReceiveSetData.normalData[can_ch1][index]
					.function_div == CAN_RX_FUNC_DIV_BMS_EOL_MAIN_CONTACT) {
					myPs->bms_eol_data.HvBattCntct_D_Actl[0]
						= (unsigned char)myData->CanData[can_ch1][index].f_val[0];
				} else if(myData->canReceiveSetData.normalData[can_ch1][index]
					.function_div == CAN_RX_FUNC_DIV_BMS_EOL_NEGATIVE_CONTACT) {
					myPs->bms_eol_data.HvBattNeg_D_Actl[0]
						= (unsigned char)myData->CanData[can_ch1][index].f_val[0];
				}
			}
			for(i=myData->canReceiveDataCount[can_ch1][0];
				i < (myData->canReceiveDataCount[can_ch1][0]
				+ myData->canReceiveDataCount[can_ch1][1]); i++) {
				index = i;

				if(myData->canReceiveSetData.normalData[can_ch1][index].function_div
					== CAN_RX_FUNC_DIV_BMS_EOL_AUX_CONTACT) {
					myPs->bms_eol_data.HvBattAuxCntct_D_Actl[0]
						= (unsigned char)myData->CanData[can_ch1][index].f_val[0];
				} else if(myData->canReceiveSetData.normalData[can_ch1][index]
					.function_div == CAN_RX_FUNC_DIV_BMS_EOL_MAIN_CONTACT) {
					myPs->bms_eol_data.HvBattCntct_D_Actl[0]
						= (unsigned char)myData->CanData[can_ch1][index].f_val[0];
				} else if(myData->canReceiveSetData.normalData[can_ch1][index]
					.function_div == CAN_RX_FUNC_DIV_BMS_EOL_NEGATIVE_CONTACT) {
					myPs->bms_eol_data.HvBattNeg_D_Actl[0]
						= (unsigned char)myData->CanData[can_ch1][index].f_val[0];
				}
			}
			myPs->signal[M_SIG_BMS_EOL_PHASE]++;
			break;
		case P18:
			//480D_Pack_V + Cycler_Pack_V + HvBatt_U_Actl
			myData->CAN.signal[can_ch1][CAN_SIG_TX_PHASE_UDS_DATA1] = 10;
			myPs->signal[M_SIG_BMS_EOL_PHASE]++;
			break;
		case P19:
			break;
		case P20:
			myPs->signal[M_SIG_BMS_EOL_DELAY1]++;
			if(myPs->signal[M_SIG_BMS_EOL_DELAY1] >= 100) {
				myPs->signal[M_SIG_BMS_EOL_DELAY1] = 0;
				myPs->signal[M_SIG_BMS_EOL_DELAY2]++;
			}

			if(myPs->signal[M_SIG_BMS_EOL_DELAY2] == 1
				&& myPs->signal[M_SIG_BMS_EOL_DELAY1] == 0) {
				myPs->signal[M_SIG_BMS_EOL_DELAY1] = 0;
				myPs->signal[M_SIG_BMS_EOL_DELAY2] = 0;

				if(can_ch1 == 0) {
					myPs->bms_eol_data.Cycler_Pack_V[0]
						= (float)myData->cData[0].op.Vsens;
				} else {
					myPs->bms_eol_data.Cycler_Pack_V[0]
						= (float)myData->cData[1].op.Vsens;
				}

				//Link_V
				myData->CAN.signal[can_ch1][CAN_SIG_TX_PHASE_UDS_DATA1] = 11;
				myPs->signal[M_SIG_BMS_EOL_PHASE]++;
			}
			break;
		case P21:
			break;
		case P22:
			//Charge_V
			myData->CAN.signal[can_ch1][CAN_SIG_TX_PHASE_UDS_DATA1] = 12;
			myPs->signal[M_SIG_BMS_EOL_PHASE]++;
			break;
		case P23:
			break;
		case P24:
			//Pack2_V
			myData->CAN.signal[can_ch1][CAN_SIG_TX_PHASE_UDS_DATA1] = 13;
			myPs->signal[M_SIG_BMS_EOL_PHASE]++;
			break;
		case P25:
			break;
		case P26:
			//Charger_Fuse_V
			myData->CAN.signal[can_ch1][CAN_SIG_TX_PHASE_UDS_DATA1] = 14;
			myPs->signal[M_SIG_BMS_EOL_PHASE]++;
			break;
		case P27:
			break;
		case P28:
			//Elac_Fuse_V
			myData->CAN.signal[can_ch1][CAN_SIG_TX_PHASE_UDS_DATA1] = 15;
			myPs->signal[M_SIG_BMS_EOL_PHASE]++;
			break;
		case P29:
			break;
		case P30: //1:aux on
			myData->CAN.signal[can_ch1][CAN_SIG_TX_PHASE_AUX_ON] = 27;
			//myData->CAN.signal[can_ch2][CAN_SIG_TX_PHASE_AUX_ON] = 27;
			myPs->signal[M_SIG_BMS_EOL_DELAY1] = 0;
			myPs->signal[M_SIG_BMS_EOL_DELAY2] = 0;
			myPs->signal[M_SIG_BMS_EOL_PHASE]++;
			break;
		case P31:
			myPs->signal[M_SIG_BMS_EOL_DELAY1]++;
			if(myPs->signal[M_SIG_BMS_EOL_DELAY1] >= 100) {
				myPs->signal[M_SIG_BMS_EOL_DELAY1] = 0;
				myPs->signal[M_SIG_BMS_EOL_DELAY2]++;
			}

			if(myPs->signal[M_SIG_BMS_EOL_DELAY2] == 1
				&& myPs->signal[M_SIG_BMS_EOL_DELAY1] == 0) {
				myPs->signal[M_SIG_BMS_EOL_DELAY1] = 0;
				myPs->signal[M_SIG_BMS_EOL_DELAY2] = 0;
				myPs->signal[M_SIG_BMS_EOL_PHASE]++;
			}
			break;
		case P32:
			//aux + negative contactor
			myPs->signal[M_SIG_BMS_EOL_PHASE]++;
			break;
		case P33:
			for(i=0; i < myData->canReceiveDataCount[can_ch1][0]; i++) {
				index = i;

				if(myData->canReceiveSetData.normalData[can_ch1][index].function_div
					== CAN_RX_FUNC_DIV_BMS_EOL_AUX_CONTACT) {
					myPs->bms_eol_data.HvBattAuxCntct_D_Actl[1]
						= (unsigned char)myData->CanData[can_ch1][index].f_val[0];
				} else if(myData->canReceiveSetData.normalData[can_ch1][index]
					.function_div == CAN_RX_FUNC_DIV_BMS_EOL_MAIN_CONTACT) {
					myPs->bms_eol_data.HvBattCntct_D_Actl[1]
						= (unsigned char)myData->CanData[can_ch1][index].f_val[0];
				} else if(myData->canReceiveSetData.normalData[can_ch1][index]
					.function_div == CAN_RX_FUNC_DIV_BMS_EOL_NEGATIVE_CONTACT) {
					myPs->bms_eol_data.HvBattNeg_D_Actl[1]
						= (unsigned char)myData->CanData[can_ch1][index].f_val[0];
				}
			}
			for(i=myData->canReceiveDataCount[can_ch1][0];
				i < (myData->canReceiveDataCount[can_ch1][0]
				+ myData->canReceiveDataCount[can_ch1][1]); i++) {
				index = i;

				if(myData->canReceiveSetData.normalData[can_ch1][index].function_div
					== CAN_RX_FUNC_DIV_BMS_EOL_AUX_CONTACT) {
					myPs->bms_eol_data.HvBattAuxCntct_D_Actl[1]
						= (unsigned char)myData->CanData[can_ch1][index].f_val[0];
				} else if(myData->canReceiveSetData.normalData[can_ch1][index]
					.function_div == CAN_RX_FUNC_DIV_BMS_EOL_MAIN_CONTACT) {
					myPs->bms_eol_data.HvBattCntct_D_Actl[1]
						= (unsigned char)myData->CanData[can_ch1][index].f_val[0];
				} else if(myData->canReceiveSetData.normalData[can_ch1][index]
					.function_div == CAN_RX_FUNC_DIV_BMS_EOL_NEGATIVE_CONTACT) {
					myPs->bms_eol_data.HvBattNeg_D_Actl[1]
						= (unsigned char)myData->CanData[can_ch1][index].f_val[0];
				}
			}
			myPs->signal[M_SIG_BMS_EOL_PHASE]++;
			break;
		case P34:
			//480D_Pack_V + Cycler_Pack_V + HvBatt_U_Actl
			myData->CAN.signal[can_ch1][CAN_SIG_TX_PHASE_UDS_DATA1] = 10;
			myPs->signal[M_SIG_BMS_EOL_PHASE]++;
			break;
		case P35:
			break;
		case P36:
			myPs->signal[M_SIG_BMS_EOL_DELAY1]++;
			if(myPs->signal[M_SIG_BMS_EOL_DELAY1] >= 100) {
				myPs->signal[M_SIG_BMS_EOL_DELAY1] = 0;
				myPs->signal[M_SIG_BMS_EOL_DELAY2]++;
			}

			if(myPs->signal[M_SIG_BMS_EOL_DELAY2] == 1
				&& myPs->signal[M_SIG_BMS_EOL_DELAY1] == 0) {
				myPs->signal[M_SIG_BMS_EOL_DELAY1] = 0;
				myPs->signal[M_SIG_BMS_EOL_DELAY2] = 0;

				if(can_ch1 == 0) {
					myPs->bms_eol_data.Cycler_Pack_V[1]
						= (float)myData->cData[0].op.Vsens;
				} else {
					myPs->bms_eol_data.Cycler_Pack_V[1]
						= (float)myData->cData[1].op.Vsens;
				}

				//Link_V
				myData->CAN.signal[can_ch1][CAN_SIG_TX_PHASE_UDS_DATA1] = 11;
				myPs->signal[M_SIG_BMS_EOL_PHASE]++;
			}
			break;
		case P37:
			break;
		case P38:
			//Charge_V
			myData->CAN.signal[can_ch1][CAN_SIG_TX_PHASE_UDS_DATA1] = 12;
			myPs->signal[M_SIG_BMS_EOL_PHASE]++;
			break;
		case P39:
			break;
		case P40:
			//Pack2_V
			myData->CAN.signal[can_ch1][CAN_SIG_TX_PHASE_UDS_DATA1] = 13;
			myPs->signal[M_SIG_BMS_EOL_PHASE]++;
			break;
		case P41:
			break;
		case P42:
			//Charger_Fuse_V
			myData->CAN.signal[can_ch1][CAN_SIG_TX_PHASE_UDS_DATA1] = 14;
			myPs->signal[M_SIG_BMS_EOL_PHASE]++;
			break;
		case P43:
			break;
		case P44:
			//Elac_Fuse_V
			myData->CAN.signal[can_ch1][CAN_SIG_TX_PHASE_UDS_DATA1] = 15;
			myPs->signal[M_SIG_BMS_EOL_PHASE]++;
			break;
		case P45:
			break;
		case P46: //2:aux & main on
			//myData->CAN.signal[can_ch1][CAN_SIG_TX_PHASE_UDS_DATA1] = 8;
			myData->CAN.signal[can_ch2][CAN_SIG_TX_PHASE_UDS_DATA1] = 8;
			myPs->signal[M_SIG_BMS_EOL_DELAY1] = 0;
			myPs->signal[M_SIG_BMS_EOL_DELAY2] = 0;
			myPs->signal[M_SIG_BMS_EOL_PHASE]++;
			break;
		case P47:
			myPs->signal[M_SIG_BMS_EOL_DELAY1]++;
			if(myPs->signal[M_SIG_BMS_EOL_DELAY1] >= 100) {
				myPs->signal[M_SIG_BMS_EOL_DELAY1] = 0;
				myPs->signal[M_SIG_BMS_EOL_DELAY2]++;
			}

			if(myPs->signal[M_SIG_BMS_EOL_DELAY2] == 1
				&& myPs->signal[M_SIG_BMS_EOL_DELAY1] == 0) {
				myPs->signal[M_SIG_BMS_EOL_DELAY1] = 0;
				myPs->signal[M_SIG_BMS_EOL_DELAY2] = 0;
				myPs->signal[M_SIG_BMS_EOL_PHASE]++;
			}
			break;
		case P48:
			//aux + main + negative contactor
			myPs->signal[M_SIG_BMS_EOL_PHASE]++;
			break;
		case P49:
			for(i=0; i < myData->canReceiveDataCount[can_ch1][0]; i++) {
				index = i;

				if(myData->canReceiveSetData.normalData[can_ch1][index].function_div
					== CAN_RX_FUNC_DIV_BMS_EOL_AUX_CONTACT) {
					myPs->bms_eol_data.HvBattAuxCntct_D_Actl[2]
						= (unsigned char)myData->CanData[can_ch1][index].f_val[0];
				} else if(myData->canReceiveSetData.normalData[can_ch1][index]
					.function_div == CAN_RX_FUNC_DIV_BMS_EOL_MAIN_CONTACT) {
					myPs->bms_eol_data.HvBattCntct_D_Actl[2]
						= (unsigned char)myData->CanData[can_ch1][index].f_val[0];
				} else if(myData->canReceiveSetData.normalData[can_ch1][index]
					.function_div == CAN_RX_FUNC_DIV_BMS_EOL_NEGATIVE_CONTACT) {
					myPs->bms_eol_data.HvBattNeg_D_Actl[2]
						= (unsigned char)myData->CanData[can_ch1][index].f_val[0];
				}
			}
			for(i=myData->canReceiveDataCount[can_ch1][0];
				i < (myData->canReceiveDataCount[can_ch1][0]
				+ myData->canReceiveDataCount[can_ch1][1]); i++) {
				index = i;

				if(myData->canReceiveSetData.normalData[can_ch1][index].function_div
					== CAN_RX_FUNC_DIV_BMS_EOL_AUX_CONTACT) {
					myPs->bms_eol_data.HvBattAuxCntct_D_Actl[2]
						= (unsigned char)myData->CanData[can_ch1][index].f_val[0];
				} else if(myData->canReceiveSetData.normalData[can_ch1][index]
					.function_div == CAN_RX_FUNC_DIV_BMS_EOL_MAIN_CONTACT) {
					myPs->bms_eol_data.HvBattCntct_D_Actl[2]
						= (unsigned char)myData->CanData[can_ch1][index].f_val[0];
				} else if(myData->canReceiveSetData.normalData[can_ch1][index]
					.function_div == CAN_RX_FUNC_DIV_BMS_EOL_NEGATIVE_CONTACT) {
					myPs->bms_eol_data.HvBattNeg_D_Actl[2]
						= (unsigned char)myData->CanData[can_ch1][index].f_val[0];
				}
			}
			myPs->signal[M_SIG_BMS_EOL_PHASE]++;
			break;
		case P50:
			//480D_Pack_V + Cycler_Pack_V + HvBatt_U_Actl
			myData->CAN.signal[can_ch1][CAN_SIG_TX_PHASE_UDS_DATA1] = 10;
			myPs->signal[M_SIG_BMS_EOL_PHASE]++;
			break;
		case P51:
			break;
		case P52:
			myPs->signal[M_SIG_BMS_EOL_DELAY1]++;
			if(myPs->signal[M_SIG_BMS_EOL_DELAY1] >= 100) {
				myPs->signal[M_SIG_BMS_EOL_DELAY1] = 0;
				myPs->signal[M_SIG_BMS_EOL_DELAY2]++;
			}

			if(myPs->signal[M_SIG_BMS_EOL_DELAY2] == 1
				&& myPs->signal[M_SIG_BMS_EOL_DELAY1] == 0) {
				myPs->signal[M_SIG_BMS_EOL_DELAY1] = 0;
				myPs->signal[M_SIG_BMS_EOL_DELAY2] = 0;

				if(can_ch1 == 0) {
					myPs->bms_eol_data.Cycler_Pack_V[2]
						= (float)myData->cData[0].op.Vsens;
				} else {
					myPs->bms_eol_data.Cycler_Pack_V[2]
						= (float)myData->cData[1].op.Vsens;
				}

				//Link_V
				myData->CAN.signal[can_ch1][CAN_SIG_TX_PHASE_UDS_DATA1] = 11;
				myPs->signal[M_SIG_BMS_EOL_PHASE]++;
			}
			break;
		case P53:
			break;
		case P54:
			//Charge_V
			myData->CAN.signal[can_ch1][CAN_SIG_TX_PHASE_UDS_DATA1] = 12;
			myPs->signal[M_SIG_BMS_EOL_PHASE]++;
			break;
		case P55:
			break;
		case P56:
			//Pack2_V
			myData->CAN.signal[can_ch1][CAN_SIG_TX_PHASE_UDS_DATA1] = 13;
			myPs->signal[M_SIG_BMS_EOL_PHASE]++;
			break;
		case P57:
			break;
		case P58:
			//Charger_Fuse_V
			myData->CAN.signal[can_ch1][CAN_SIG_TX_PHASE_UDS_DATA1] = 14;
			myPs->signal[M_SIG_BMS_EOL_PHASE]++;
			break;
		case P59:
			break;
		case P60:
			//Elac_Fuse_V
			myData->CAN.signal[can_ch1][CAN_SIG_TX_PHASE_UDS_DATA1] = 15;
			myPs->signal[M_SIG_BMS_EOL_PHASE]++;
			break;
		case P61:
			break;
		case P62: //3:all off (aux off)
			myData->CAN.signal[can_ch1][CAN_SIG_TX_PHASE_AUX_OFF] = 28;
			//myData->CAN.signal[can_ch2][CAN_SIG_TX_PHASE_AUX_OFF] = 28;
			myPs->signal[M_SIG_BMS_EOL_DELAY1] = 0;
			myPs->signal[M_SIG_BMS_EOL_DELAY2] = 0;
			myPs->signal[M_SIG_BMS_EOL_PHASE]++;
			break;
		case P63: //3:all off (main off)
			//myData->CAN.signal[can_ch1][CAN_SIG_TX_PHASE_MAIN_OFF] = 9;
			myData->CAN.signal[can_ch2][CAN_SIG_TX_PHASE_MAIN_OFF] = 9;
			myPs->signal[M_SIG_BMS_EOL_DELAY1] = 0;
			myPs->signal[M_SIG_BMS_EOL_DELAY2] = 0;
			myPs->signal[M_SIG_BMS_EOL_PHASE]++;
			break;
		case P64:
			myPs->signal[M_SIG_BMS_EOL_DELAY1]++;
			if(myPs->signal[M_SIG_BMS_EOL_DELAY1] >= 100) {
				myPs->signal[M_SIG_BMS_EOL_DELAY1] = 0;
				myPs->signal[M_SIG_BMS_EOL_DELAY2]++;
			}

			if(myPs->signal[M_SIG_BMS_EOL_DELAY2] == 1
				&& myPs->signal[M_SIG_BMS_EOL_DELAY1] == 0) {
				myPs->signal[M_SIG_BMS_EOL_DELAY1] = 0;
				myPs->signal[M_SIG_BMS_EOL_DELAY2] = 0;
				myPs->signal[M_SIG_BMS_EOL_PHASE]++;
			}
			break;
		case P65:
			//aux + main + negative contactor
			myPs->signal[M_SIG_BMS_EOL_PHASE]++;
			break;
		case P66:
			for(i=0; i < myData->canReceiveDataCount[can_ch1][0]; i++) {
				index = i;

				if(myData->canReceiveSetData.normalData[can_ch1][index].function_div
					== CAN_RX_FUNC_DIV_BMS_EOL_AUX_CONTACT) {
					myPs->bms_eol_data.HvBattAuxCntct_D_Actl[3]
						= (unsigned char)myData->CanData[can_ch1][index].f_val[0];
				} else if(myData->canReceiveSetData.normalData[can_ch1][index]
					.function_div == CAN_RX_FUNC_DIV_BMS_EOL_MAIN_CONTACT) {
					myPs->bms_eol_data.HvBattCntct_D_Actl[3]
						= (unsigned char)myData->CanData[can_ch1][index].f_val[0];
				} else if(myData->canReceiveSetData.normalData[can_ch1][index]
					.function_div == CAN_RX_FUNC_DIV_BMS_EOL_NEGATIVE_CONTACT) {
					myPs->bms_eol_data.HvBattNeg_D_Actl[3]
						= (unsigned char)myData->CanData[can_ch1][index].f_val[0];
				}
			}
			for(i=myData->canReceiveDataCount[can_ch1][0];
				i < (myData->canReceiveDataCount[can_ch1][0]
				+ myData->canReceiveDataCount[can_ch1][1]); i++) {
				index = i;

				if(myData->canReceiveSetData.normalData[can_ch1][index].function_div
					== CAN_RX_FUNC_DIV_BMS_EOL_AUX_CONTACT) {
					myPs->bms_eol_data.HvBattAuxCntct_D_Actl[3]
						= (unsigned char)myData->CanData[can_ch1][index].f_val[0];
				} else if(myData->canReceiveSetData.normalData[can_ch1][index]
					.function_div == CAN_RX_FUNC_DIV_BMS_EOL_MAIN_CONTACT) {
					myPs->bms_eol_data.HvBattCntct_D_Actl[3]
						= (unsigned char)myData->CanData[can_ch1][index].f_val[0];
				} else if(myData->canReceiveSetData.normalData[can_ch1][index]
					.function_div == CAN_RX_FUNC_DIV_BMS_EOL_NEGATIVE_CONTACT) {
					myPs->bms_eol_data.HvBattNeg_D_Actl[3]
						= (unsigned char)myData->CanData[can_ch1][index].f_val[0];
				}
			}
			myPs->signal[M_SIG_BMS_EOL_PHASE]++;
			break;
		case P67:
			//480D_Pack_V + Cycler_Pack_V + HvBatt_U_Actl
			myData->CAN.signal[can_ch1][CAN_SIG_TX_PHASE_UDS_DATA1] = 10;
			myPs->signal[M_SIG_BMS_EOL_PHASE]++;
			break;
		case P68:
			break;
		case P69:
			myPs->signal[M_SIG_BMS_EOL_DELAY1]++;
			if(myPs->signal[M_SIG_BMS_EOL_DELAY1] >= 100) {
				myPs->signal[M_SIG_BMS_EOL_DELAY1] = 0;
				myPs->signal[M_SIG_BMS_EOL_DELAY2]++;
			}

			if(myPs->signal[M_SIG_BMS_EOL_DELAY2] == 1
				&& myPs->signal[M_SIG_BMS_EOL_DELAY1] == 0) {
				myPs->signal[M_SIG_BMS_EOL_DELAY1] = 0;
				myPs->signal[M_SIG_BMS_EOL_DELAY2] = 0;

				if(can_ch1 == 0) {
					myPs->bms_eol_data.Cycler_Pack_V[3]
						= (float)myData->cData[0].op.Vsens;
				} else {
					myPs->bms_eol_data.Cycler_Pack_V[3]
						= (float)myData->cData[1].op.Vsens;
				}

				//Link_V
				myData->CAN.signal[can_ch1][CAN_SIG_TX_PHASE_UDS_DATA1] = 11;
				myPs->signal[M_SIG_BMS_EOL_PHASE]++;
			}
			break;
		case P70:
			break;
		case P71:
			//Charge_V
			myData->CAN.signal[can_ch1][CAN_SIG_TX_PHASE_UDS_DATA1] = 12;
			myPs->signal[M_SIG_BMS_EOL_PHASE]++;
			break;
		case P72:
			break;
		case P73:
			//Pack2_V
			myData->CAN.signal[can_ch1][CAN_SIG_TX_PHASE_UDS_DATA1] = 13;
			myPs->signal[M_SIG_BMS_EOL_PHASE]++;
			break;
		case P74:
			break;
		case P75:
			//Charger_Fuse_V
			myData->CAN.signal[can_ch1][CAN_SIG_TX_PHASE_UDS_DATA1] = 14;
			myPs->signal[M_SIG_BMS_EOL_PHASE]++;
			break;
		case P76:
			break;
		case P77:
			//Elac_Fuse_V
			myData->CAN.signal[can_ch1][CAN_SIG_TX_PHASE_UDS_DATA1] = 15;
			myPs->signal[M_SIG_BMS_EOL_PHASE]++;
			break;
		case P78:
			break;
		case P79:
			//aux on
			myData->CAN.signal[can_ch1][CAN_SIG_TX_PHASE_AUX_ON] = 27;
			//myData->CAN.signal[can_ch2][CAN_SIG_TX_PHASE_AUX_ON] = 27;
			myPs->signal[M_SIG_BMS_EOL_DELAY1] = 0;
			myPs->signal[M_SIG_BMS_EOL_DELAY2] = 0;
			myPs->signal[M_SIG_BMS_EOL_PHASE]++;
			break;
		case P80:
			//main on
			//myData->CAN.signal[can_ch1][CAN_SIG_TX_PHASE_MAIN_ON] = 8;
			myData->CAN.signal[can_ch2][CAN_SIG_TX_PHASE_MAIN_ON] = 8;
			myPs->signal[M_SIG_BMS_EOL_DELAY1] = 0;
			myPs->signal[M_SIG_BMS_EOL_DELAY2] = 0;
			myPs->signal[M_SIG_BMS_EOL_PHASE]++;
			break;
		case P81:
			myPs->signal[M_SIG_BMS_EOL_DELAY1]++;
			if(myPs->signal[M_SIG_BMS_EOL_DELAY1] >= 100) {
				myPs->signal[M_SIG_BMS_EOL_DELAY1] = 0;
				myPs->signal[M_SIG_BMS_EOL_DELAY2]++;
			}

			if(myPs->signal[M_SIG_BMS_EOL_DELAY2] == 1
				&& myPs->signal[M_SIG_BMS_EOL_DELAY1] == 0) {
				myPs->signal[M_SIG_BMS_EOL_DELAY1] = 0;
				myPs->signal[M_SIG_BMS_EOL_DELAY2] = 0;
				myPs->signal[M_SIG_BMS_EOL_PHASE]++;
			}
			break;
		case P82:
			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
			SendMsg.msg = MSG_MODULE_COA_BMS_EOL_DATA1_REPLY;
			send_msg(MODULE_TO_COA1+group, (char *)&SendMsg);
			myPs->signal[M_SIG_BMS_EOL_PHASE] = P0;
			break;
		case P100: //kjg_120413
			myPs->signal[M_SIG_BMS_EOL_DELAY1]++;
			if(myPs->signal[M_SIG_BMS_EOL_DELAY1] >= 100) {
				myPs->signal[M_SIG_BMS_EOL_DELAY1] = 0;
				myPs->signal[M_SIG_BMS_EOL_DELAY2]++;
			}

			if(myPs->signal[M_SIG_BMS_EOL_DELAY2] == 25
				&& myPs->signal[M_SIG_BMS_EOL_DELAY1] == 0) {
				myPs->signal[M_SIG_BMS_EOL_DELAY1] = 0;
				myPs->signal[M_SIG_BMS_EOL_DELAY2] = 0;
				myPs->signal[M_SIG_BMS_EOL_PHASE]++;
			}
			break;
		case P101:
			//isolation_resistance
			myData->CAN.signal[can_ch1][CAN_SIG_TX_PHASE_UDS_DATA1] = 16;
			myPs->signal[M_SIG_BMS_EOL_DELAY1] = 0;
			myPs->signal[M_SIG_BMS_EOL_DELAY2] = 0;
			myPs->signal[M_SIG_BMS_EOL_PHASE]++;
			break;
		case P102:
			break;
		case P103:
			//isolation_Va
			myData->CAN.signal[can_ch1][CAN_SIG_TX_PHASE_UDS_DATA1] = 17;
			myPs->signal[M_SIG_BMS_EOL_PHASE]++;
			break;
		case P104:
			break;
		case P105:
			//isolation_Vb
			myData->CAN.signal[can_ch1][CAN_SIG_TX_PHASE_UDS_DATA1] = 18;
			myPs->signal[M_SIG_BMS_EOL_PHASE]++;
			break;
		case P106:
			break;
		case P107:
			//all off
			myData->CAN.signal[can_ch1][CAN_SIG_TX_PHASE_AUX_OFF] = 28;
			//myData->CAN.signal[can_ch1][CAN_SIG_TX_PHASE_MAIN_OFF] = 9;
			//myData->CAN.signal[can_ch2][CAN_SIG_TX_PHASE_AUX_OFF] = 28;
			myData->CAN.signal[can_ch2][CAN_SIG_TX_PHASE_MAIN_OFF] = 9;
			myPs->signal[M_SIG_BMS_EOL_DELAY1] = 0;
			myPs->signal[M_SIG_BMS_EOL_DELAY2] = 0;
			myPs->signal[M_SIG_BMS_EOL_PHASE]++;
			break;
		case P108:
			myPs->signal[M_SIG_BMS_EOL_DELAY1]++;
			if(myPs->signal[M_SIG_BMS_EOL_DELAY1] >= 100) {
				myPs->signal[M_SIG_BMS_EOL_DELAY1] = 0;
				myPs->signal[M_SIG_BMS_EOL_DELAY2]++;
			}

			if(myPs->signal[M_SIG_BMS_EOL_DELAY2] == 2
				&& myPs->signal[M_SIG_BMS_EOL_DELAY1] == 0) {
				memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
				SendMsg.msg = MSG_MODULE_COA_BMS_EOL_DATA2_REPLY;
				send_msg(MODULE_TO_COA1+group, (char *)&SendMsg);
				myPs->signal[M_SIG_BMS_EOL_PHASE] = P0;
			}
			break;
		case P150:
			myPs->signal[M_SIG_BMS_EOL_DELAY1] = 0;
			myPs->signal[M_SIG_BMS_EOL_DELAY2] = 0;
			myData->CAN.signal[can_ch1][CAN_SIG_TX_PHASE_UDS_DATA1] = 29;
			myPs->signal[M_SIG_BMS_EOL_PHASE]++;
			break;
		case P152:
			myPs->signal[M_SIG_BMS_EOL_DELAY1]++;
			if(myPs->signal[M_SIG_BMS_EOL_DELAY1] >= 100) {
				myPs->signal[M_SIG_BMS_EOL_DELAY1] = 0;
				myPs->signal[M_SIG_BMS_EOL_DELAY2]++;
			}

			if(myPs->signal[M_SIG_BMS_EOL_DELAY2] == 10
				&& myPs->signal[M_SIG_BMS_EOL_DELAY1] == 0) {
				myPs->signal[M_SIG_BMS_EOL_DELAY1] = 0;
				myPs->signal[M_SIG_BMS_EOL_DELAY2] = 0;
				myData->CAN.signal[can_ch1][CAN_SIG_TX_PHASE_UDS_DATA1] = 30;
				myPs->signal[M_SIG_BMS_EOL_PHASE]++;
			}
			break;
		case P154:
			myData->CAN.signal[can_ch1][CAN_SIG_TX_PHASE_UDS_DATA1] = 31;
			myPs->signal[M_SIG_BMS_EOL_PHASE]++;
			break;
		case P156:
			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
			SendMsg.msg = MSG_MODULE_COA_BMS_EOL_HI_POT_TEST_REPLY;
			SendMsg.val[0] = 1;
			SendMsg.val[1] = 1;
			send_msg(MODULE_TO_COA1+group, (char *)&SendMsg);
			myPs->signal[M_SIG_BMS_EOL_PHASE] = P0;
			break;
		case P160:
			myPs->signal[M_SIG_BMS_EOL_DELAY1] = 0;
			myPs->signal[M_SIG_BMS_EOL_DELAY2] = 0;
			myData->CAN.signal[can_ch1][CAN_SIG_TX_PHASE_UDS_DATA1] = 32;
			myPs->signal[M_SIG_BMS_EOL_PHASE]++;
			break;
		case P162:
			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
			SendMsg.msg = MSG_MODULE_COA_BMS_EOL_HI_POT_TEST_REPLY;
			SendMsg.val[0] = 3;
			SendMsg.val[1] = 1;
			send_msg(MODULE_TO_COA1+group, (char *)&SendMsg);
			myPs->signal[M_SIG_BMS_EOL_PHASE] = P0;
			break;
		case P170:
			myPs->signal[M_SIG_BMS_EOL_DELAY1] = 0;
			myPs->signal[M_SIG_BMS_EOL_DELAY2] = 0;
			myData->CAN.signal[can_ch1][CAN_SIG_TX_PHASE_UDS_DATA1] = 33;
			myPs->signal[M_SIG_BMS_EOL_PHASE]++;
			break;
		case P171:
			myPs->signal[M_SIG_BMS_EOL_DELAY1]++;
			if(myPs->signal[M_SIG_BMS_EOL_DELAY1] == 10) {
				myPs->signal[M_SIG_BMS_EOL_DELAY1] = 0;
				myPs->signal[M_SIG_BMS_EOL_DELAY2] = 0;
				myData->CAN.signal[can_ch1][CAN_SIG_TX_PHASE_UDS_DATA1] = 34;
				myPs->signal[M_SIG_BMS_EOL_PHASE] = P0;
			}
			break;
		case P180:
			myPs->signal[M_SIG_BMS_EOL_DELAY1] = 0;
			myPs->signal[M_SIG_BMS_EOL_DELAY2] = 0;
			myData->CAN.signal[can_ch1][CAN_SIG_TX_PHASE_UDS_DATA1] = 35;
			myPs->signal[M_SIG_BMS_EOL_PHASE] = P0;
			break;
		default:
			break;
	}
}

void mSignalCheck_BMS_EOL_FLASH_PHASE(void)
{
	int group=0, block_count, block_point, block_size;
	S_MSG_VAL SendMsg;

	switch(myPs->signal[M_SIG_BMS_EOL_FLASH_PHASE2]) { //kjg_120126
		case P1:
			myPs->signal[M_SIG_BMS_EOL_FLASH_DELAY3] = 0;
			myPs->signal[M_SIG_BMS_EOL_FLASH_DELAY4] = 0;
			myPs->signal[M_SIG_BMS_EOL_FLASH_PHASE2]++;
			break;
		case P2:
			myPs->signal[M_SIG_BMS_EOL_FLASH_DELAY3]++;
			if(myPs->signal[M_SIG_BMS_EOL_FLASH_DELAY3] >= 100) {
				myPs->signal[M_SIG_BMS_EOL_FLASH_DELAY3] = 0;
				myPs->signal[M_SIG_BMS_EOL_FLASH_DELAY4]++;
			}

			if(myPs->signal[M_SIG_BMS_EOL_FLASH_DELAY4] == 2) {
				if(myPs->signal[M_SIG_BMS_EOL_FLASH_DELAY3] == 0) {
					//0x7DF 02 3E 80 00 00 00 00 00 send : TesterPresent(3E)
					myData->CAN.signal[0][CAN_SIG_TX_PHASE_UDS_DATA2] = 2;
					myPs->signal[M_SIG_BMS_EOL_FLASH_DELAY3] = 0;
					myPs->signal[M_SIG_BMS_EOL_FLASH_DELAY4] = 0;
				}
			}
			break;
		default:
			break;
	}

	switch(myPs->signal[M_SIG_BMS_EOL_FLASH_PHASE]) { //kjh_120112
		case P1:
			memset((char *)&myPs->bms_eol_data2.pack_id[0][0], 0, 40 * 4);
			memcpy((char *)&myPs->bms_eol_data2.pack_id[0][0],
				(char *)&myData->COA_Client[0].misc.write_pack_id[0], 40);

			//0x7DF 02 10 82 00 00 00 00 00 send
			myData->CAN.signal[0][CAN_SIG_TX_PHASE_UDS_DATA2] = 1;
			myPs->signal[M_SIG_BMS_EOL_FLASH_DELAY1] = 0;
			myPs->signal[M_SIG_BMS_EOL_FLASH_DELAY2] = 0;
			myPs->signal[M_SIG_BMS_EOL_FLASH_PHASE2] = P0;
			myPs->signal[M_SIG_BMS_EOL_FLASH_DELAY3] = 0;
			myPs->signal[M_SIG_BMS_EOL_FLASH_DELAY4] = 0;
			myPs->signal[M_SIG_BMS_EOL_FLASH_PHASE]++;
			break;
		case P2:
			myPs->signal[M_SIG_BMS_EOL_FLASH_DELAY1]++;
			if(myPs->signal[M_SIG_BMS_EOL_FLASH_DELAY1] >= 100) {
				myPs->signal[M_SIG_BMS_EOL_FLASH_DELAY1] = 0;
				myPs->signal[M_SIG_BMS_EOL_FLASH_DELAY2]++;
			}

			if(myPs->signal[M_SIG_BMS_EOL_FLASH_DELAY2] == 2) {
				if(myPs->signal[M_SIG_BMS_EOL_FLASH_DELAY1] == 0) {
					//0x7DF 02 3E 80 00 00 00 00 00 send : TesterPresent(3E)
					myData->CAN.signal[0][CAN_SIG_TX_PHASE_UDS_DATA2] = 2;
				} else if(myPs->signal[M_SIG_BMS_EOL_FLASH_DELAY1] == 10) {
					//0x7DF 02 3E 80 00 00 00 00 00 send : TesterPresent(3E)
					myData->CAN.signal[0][CAN_SIG_TX_PHASE_UDS_DATA2] = 2;
					myPs->signal[M_SIG_BMS_EOL_FLASH_PHASE2] = P1;
				} else if(myPs->signal[M_SIG_BMS_EOL_FLASH_DELAY1] == 20) {
					//Read Software Download Specification Version
					myData->CAN.signal[0][CAN_SIG_TX_PHASE_UDS_DATA3] = 1;
					myPs->signal[M_SIG_BMS_EOL_FLASH_DELAY1] = 0;
					myPs->signal[M_SIG_BMS_EOL_FLASH_DELAY2] = 0;
					myPs->signal[M_SIG_BMS_EOL_FLASH_PHASE]++;
				}
			}
			break;
		case P3:
			break;
		case P4:
			//Diagnostic Session Control : 10 programmingSession
			myData->CAN.signal[0][CAN_SIG_TX_PHASE_UDS_DATA3] = 2;
			myPs->signal[M_SIG_BMS_EOL_FLASH_PHASE]++;
			break;
		case P5:
			break;
		case P6:
			//ReadData By Identifier(KDP_CORE_ASSY_NO)
			myData->CAN.signal[0][CAN_SIG_TX_PHASE_UDS_DATA3] = 3;
			myPs->signal[M_SIG_BMS_EOL_FLASH_PHASE]++;
			myData->CAN.signal[0][CAN_SIG_TX_PHASE_UDS_DATA_INDEX] = 0;
			break;
		case P7:
			break;
		case P8:
			//ReadData By Identifier(ECU_CORE_ASSY_NO)
			myData->CAN.signal[0][CAN_SIG_TX_PHASE_UDS_DATA3] = 4;
			myPs->signal[M_SIG_BMS_EOL_FLASH_PHASE]++;
			myData->CAN.signal[0][CAN_SIG_TX_PHASE_UDS_DATA_INDEX] = 0;
			break;
		case P9:
			break;
		case P10:
			//ReadData By Identifier(ECU_SERIAL_NUM)
			myData->CAN.signal[0][CAN_SIG_TX_PHASE_UDS_DATA3] = 5;
			myPs->signal[M_SIG_BMS_EOL_FLASH_PHASE]++;
			myData->CAN.signal[0][CAN_SIG_TX_PHASE_UDS_DATA_INDEX] = 0;
			break;
		case P11:
			break;
		case P12:
			//ReadData By Identifier(KDP_BOOT_SW_ID)
			myData->CAN.signal[0][CAN_SIG_TX_PHASE_UDS_DATA3] = 6;
			myPs->signal[M_SIG_BMS_EOL_FLASH_PHASE]++;
			myData->CAN.signal[0][CAN_SIG_TX_PHASE_UDS_DATA_INDEX] = 0;
			break;
		case P13:
			break;
		case P14:
			//ReadData By Identifier(PRIMARY_BOOT_NO)
			myData->CAN.signal[0][CAN_SIG_TX_PHASE_UDS_DATA3] = 7;
			myPs->signal[M_SIG_BMS_EOL_FLASH_PHASE]++;
			myData->CAN.signal[0][CAN_SIG_TX_PHASE_UDS_DATA_INDEX] = 0;
			break;
		case P15:
			break;
		case P16:
			//SecurityAccess : seed
			myData->CAN.signal[0][CAN_SIG_TX_PHASE_UDS_DATA3] = 8;
			myPs->signal[M_SIG_BMS_EOL_FLASH_PHASE]++;
			myData->CAN.signal[0][CAN_SIG_TX_PHASE_UDS_DATA_INDEX] = 0;
			break;
		case P17:
			break;
		case P18:
			//SecurityAccess : key
			myData->CAN.signal[0][CAN_SIG_TX_PHASE_UDS_DATA3] = 9;
			myPs->signal[M_SIG_BMS_EOL_FLASH_PHASE]++;
			myData->CAN.signal[0][CAN_SIG_TX_PHASE_UDS_DATA_INDEX] = 0;
			myData->CAN.block_count = 0;
			break;
		case P19:
			break;
		case P20:
			//flashing RequestDownload:34
			myData->CAN.signal[0][CAN_SIG_TX_PHASE_UDS_DATA_FF] = 12;
			myPs->signal[M_SIG_BMS_EOL_FLASH_PHASE]++;
			myData->CAN.signal[0][CAN_SIG_TX_PHASE_UDS_DATA_INDEX] = 0;

			block_count = myData->CAN.block_count;
			block_point = myData->CAN.can_flash_data_block_point[block_count];

			myData->CAN.uds_tx[0].uc_val[0] = 0x10;
			myData->CAN.uds_tx[0].uc_val[1] = 0x0B;
			myData->CAN.uds_tx[0].uc_val[2] = 0x34;
			myData->CAN.uds_tx[0].uc_val[3] = 0x00;
			myData->CAN.uds_tx[0].uc_val[4] = 0x44;
			myData->CAN.uds_tx[0].uc_val[5]
				= myData->CAN.can_flash_file[block_point];
			myData->CAN.uds_tx[0].uc_val[6]
				= myData->CAN.can_flash_file[block_point+1];
			myData->CAN.uds_tx[0].uc_val[7]
				= myData->CAN.can_flash_file[block_point+2];
			break;
		case P21:
			switch(myData->CAN.signal[0][CAN_SIG_TX_PHASE_UDS_DATA_INDEX]) {
				case 1:
					block_count = myData->CAN.block_count;
					block_point = myData->CAN
						.can_flash_data_block_point[block_count];
					block_point += 3;

					myData->CAN.uds_tx[0].uc_val[0] = 0x21;
					myData->CAN.uds_tx[0].uc_val[1]
						= myData->CAN.can_flash_file[block_point];
					myData->CAN.uds_tx[0].uc_val[2]
						= myData->CAN.can_flash_file[block_point+1];
					myData->CAN.uds_tx[0].uc_val[3]
						= myData->CAN.can_flash_file[block_point+2];
					myData->CAN.uds_tx[0].uc_val[4]
						= myData->CAN.can_flash_file[block_point+3];
					myData->CAN.uds_tx[0].uc_val[5]
						= myData->CAN.can_flash_file[block_point+4];
					myData->CAN.uds_tx[0].uc_val[6] = 0;
					myData->CAN.uds_tx[0].uc_val[7] = 0;
					myData->CAN.signal[0][CAN_SIG_TX_PHASE_UDS_DATA_CF] = 13;
					break;
				default:
					break;
			}
			break;
		case P22:
			//flashing TransferData:36
			myData->CAN.signal[0][CAN_SIG_TX_PHASE_UDS_DATA_FF] = 12;
			myPs->signal[M_SIG_BMS_EOL_FLASH_PHASE]++;
			myData->CAN.signal[0][CAN_SIG_TX_PHASE_UDS_DATA_INDEX] = 0;

			block_count = myData->CAN.block_count;
			block_point = myData->CAN.block_point;
			block_size = myData->CAN.can_flash_data_block_point[block_count]
				+ myData->CAN.can_flash_data_block_size[block_count];
			block_size = block_size - block_point + 2;
			if(block_size > 3170) block_size = 3170; //0x0C62 = 3170d
			myData->CAN.uds_tx_block_size = block_size;

			myData->CAN.uds_tx[0].uc_val[0]
				= (unsigned char)(0x10 + block_size / 256);
			myData->CAN.uds_tx[0].uc_val[1]
				= (unsigned char)(block_size % 256);
			myData->CAN.uds_tx[0].uc_val[2] = 0x36;
			myData->CAN.uds_ff_count++;
			myData->CAN.uds_tx[0].uc_val[3]
				= (unsigned char)myData->CAN.uds_ff_count;
			block_point += 8;
			myData->CAN.uds_tx[0].uc_val[4]
				= myData->CAN.can_flash_file[block_point];
			myData->CAN.uds_tx[0].uc_val[5]
				= myData->CAN.can_flash_file[block_point+1];
			myData->CAN.uds_tx[0].uc_val[6]
				= myData->CAN.can_flash_file[block_point+2];
			myData->CAN.uds_tx[0].uc_val[7]
				= myData->CAN.can_flash_file[block_point+3];
			myData->CAN.block_point = block_point+4;
			myData->CAN.uds_tx_size = block_size - 7;
			break;
		case P23:
			switch(myData->CAN.signal[0][CAN_SIG_TX_PHASE_UDS_DATA_INDEX]) {
				case 1:
					block_count = myData->CAN.block_count;
					block_point = myData->CAN.block_point;
					block_size = myData->CAN
						.can_flash_data_block_point[block_count] + 8
						+ myData->CAN.can_flash_data_block_size[block_count];

					myData->CAN.uds_tx[0].uc_val[0] = 0x20 + myData->CAN
						.signal[0][CAN_SIG_TX_PHASE_UDS_DATA_INDEX];
					if(block_point < block_size) {
						myData->CAN.uds_tx[0].uc_val[1]
							= myData->CAN.can_flash_file[block_point];
					} else {
						myData->CAN.uds_tx[0].uc_val[1] = 0x00;
					}
					if((block_point+1) < block_size) {
						myData->CAN.uds_tx[0].uc_val[2]
							= myData->CAN.can_flash_file[block_point+1];
					} else {
						myData->CAN.uds_tx[0].uc_val[2] = 0x00;
					}
					if((block_point+2) < block_size) {
						myData->CAN.uds_tx[0].uc_val[3]
							= myData->CAN.can_flash_file[block_point+2];
					} else {
						myData->CAN.uds_tx[0].uc_val[3] = 0x00;
					}
					if((block_point+3) < block_size) {
						myData->CAN.uds_tx[0].uc_val[4]
							= myData->CAN.can_flash_file[block_point+3];
					} else {
						myData->CAN.uds_tx[0].uc_val[4] = 0x00;
					}
					if((block_point+4) < block_size) {
						myData->CAN.uds_tx[0].uc_val[5]
							= myData->CAN.can_flash_file[block_point+4];
					} else {
						myData->CAN.uds_tx[0].uc_val[5] = 0x00;
					}
					if((block_point+5) < block_size) {
						myData->CAN.uds_tx[0].uc_val[6]
							= myData->CAN.can_flash_file[block_point+5];
					} else {
						myData->CAN.uds_tx[0].uc_val[6] = 0x00;
					}
					if((block_point+6) < block_size) {
						myData->CAN.uds_tx[0].uc_val[7]
							= myData->CAN.can_flash_file[block_point+6];
					} else {
						myData->CAN.uds_tx[0].uc_val[7] = 0x00;
					}
					myData->CAN.block_point = block_point+7;
					myData->CAN.uds_tx_size -= 7;

					//if((block_point+7) < block_size) {
					if(myData->CAN.uds_tx_size >= 0) {
						myData->CAN.signal[0][CAN_SIG_TX_PHASE_UDS_DATA_CF]
							= 13;
						myData->CAN.signal[0]
							[CAN_SIG_TX_PHASE_UDS_DATA_INDEX] = 34;
					} else {
						myData->CAN.signal[0][CAN_SIG_TX_PHASE_UDS_DATA_CF]
							= 13;
						myData->CAN.signal[0]
							[CAN_SIG_TX_PHASE_UDS_DATA_INDEX] = 100;
					}
					break;
				case 32: //0x20
				case 33:
				case 34:
				case 35:
				case 36:
				case 37:
				case 38:
				case 39:
				case 40:
				case 41:
				case 42:
				case 43:
				case 44:
				case 45:
				case 46:
				case 47: //0x2F
					block_count = myData->CAN.block_count;
					block_point = myData->CAN.block_point;
					block_size = myData->CAN
						.can_flash_data_block_point[block_count] + 8
						+ myData->CAN.can_flash_data_block_size[block_count];

					myData->CAN.uds_tx[0].uc_val[0] = myData->CAN
						.signal[0][CAN_SIG_TX_PHASE_UDS_DATA_INDEX];
					if(block_point < block_size) {
						myData->CAN.uds_tx[0].uc_val[1]
							= myData->CAN.can_flash_file[block_point];
					} else {
						myData->CAN.uds_tx[0].uc_val[1] = 0x00;
					}
					if((block_point+1) < block_size) {
						myData->CAN.uds_tx[0].uc_val[2]
							= myData->CAN.can_flash_file[block_point+1];
					} else {
						myData->CAN.uds_tx[0].uc_val[2] = 0x00;
					}
					if((block_point+2) < block_size) {
						myData->CAN.uds_tx[0].uc_val[3]
							= myData->CAN.can_flash_file[block_point+2];
					} else {
						myData->CAN.uds_tx[0].uc_val[3] = 0x00;
					}
					if((block_point+3) < block_size) {
						myData->CAN.uds_tx[0].uc_val[4]
							= myData->CAN.can_flash_file[block_point+3];
					} else {
						myData->CAN.uds_tx[0].uc_val[4] = 0x00;
					}
					if((block_point+4) < block_size) {
						myData->CAN.uds_tx[0].uc_val[5]
							= myData->CAN.can_flash_file[block_point+4];
					} else {
						myData->CAN.uds_tx[0].uc_val[5] = 0x00;
					}
					if((block_point+5) < block_size) {
						myData->CAN.uds_tx[0].uc_val[6]
							= myData->CAN.can_flash_file[block_point+5];
					} else {
						myData->CAN.uds_tx[0].uc_val[6] = 0x00;
					}
					if((block_point+6) < block_size) {
						myData->CAN.uds_tx[0].uc_val[7]
							= myData->CAN.can_flash_file[block_point+6];
					} else {
						myData->CAN.uds_tx[0].uc_val[7] = 0x00;
					}
					myData->CAN.block_point = block_point+7;
					myData->CAN.uds_tx_size -= 7;

					//if((block_point+7) < block_size) {
					if(myData->CAN.uds_tx_size >= 0) {
						if(myData->CAN.signal[0]
							[CAN_SIG_TX_PHASE_UDS_DATA_INDEX] < 47) {
							myData->CAN.signal[0][CAN_SIG_TX_PHASE_UDS_DATA_CF]
								= 13;
							myData->CAN.signal[0]
								[CAN_SIG_TX_PHASE_UDS_DATA_INDEX]++;
						} else {
							myData->CAN.signal[0][CAN_SIG_TX_PHASE_UDS_DATA_CF]
								= 13;
							myData->CAN.signal[0]
								[CAN_SIG_TX_PHASE_UDS_DATA_INDEX] = 32;
						}
					} else {
						myData->CAN.signal[0][CAN_SIG_TX_PHASE_UDS_DATA_CF]
							= 13;
						myData->CAN.signal[0]
							[CAN_SIG_TX_PHASE_UDS_DATA_INDEX] = 100;
					}
					break;
				default:
					break;
			}
			break;
		case P24:
			//flashing RequestTransferExit:37
			myData->CAN.signal[0][CAN_SIG_TX_PHASE_UDS_DATA3] = 10;
			myPs->signal[M_SIG_BMS_EOL_FLASH_PHASE]++;
			myData->CAN.signal[0][CAN_SIG_TX_PHASE_UDS_DATA_INDEX] = 0;
			break;
		case P25:
			break;
		case P26:
			//RoutineControl:31
			myData->CAN.signal[0][CAN_SIG_TX_PHASE_UDS_DATA3] = 11;
			myPs->signal[M_SIG_BMS_EOL_FLASH_PHASE]++;
			myData->CAN.signal[0][CAN_SIG_TX_PHASE_UDS_DATA_INDEX] = 0;
			break;
		case P27:
			break;
		case P28:
			myData->CAN.signal[0][CAN_SIG_TX_PHASE_UDS_DATA3] = 12;
			myPs->signal[M_SIG_BMS_EOL_FLASH_PHASE]++;
			myPs->signal[M_SIG_BMS_EOL_FLASH_DELAY1] = 0;
			myPs->signal[M_SIG_BMS_EOL_FLASH_DELAY2] = 0;
			myData->CAN.signal[0][CAN_SIG_TX_PHASE_UDS_DATA_INDEX] = 0;
			break;
		case P29:
			break;
		case P30:
			//ReadData By Identifier(ECU_SERIAL_NUM)
			myData->CAN.signal[0][CAN_SIG_TX_PHASE_UDS_DATA3] = 5;
			myPs->signal[M_SIG_BMS_EOL_FLASH_PHASE]++;
			myData->CAN.signal[0][CAN_SIG_TX_PHASE_UDS_DATA_INDEX] = 0;
			break;
		case P31:
			break;
		case P32:
			//kjg_121004_d
			//myPs->signal[M_SIG_BMS_EOL_FLASH_PHASE] = P37;
			//myData->CAN.signal[0][CAN_SIG_TX_PHASE_UDS_DATA_INDEX] = 0;

			//WriteData By Identifier(Pack ID)
			myData->CAN.signal[0][CAN_SIG_TX_PHASE_UDS_DATA3] = 13;
			myPs->signal[M_SIG_BMS_EOL_FLASH_PHASE]++;
			myData->CAN.signal[0][CAN_SIG_TX_PHASE_UDS_DATA_INDEX] = 0;
			break;
		case P33:
			break;
		case P34:
			myData->CAN.signal[0][CAN_SIG_TX_PHASE_UDS_DATA3] = 13;
			myPs->signal[M_SIG_BMS_EOL_FLASH_PHASE]++;
			myData->CAN.signal[0][CAN_SIG_TX_PHASE_UDS_DATA_INDEX] = 1;
			break;
		case P35:
			myData->CAN.signal[0][CAN_SIG_TX_PHASE_UDS_DATA3] = 13;
			myPs->signal[M_SIG_BMS_EOL_FLASH_PHASE]++;
			myData->CAN.signal[0][CAN_SIG_TX_PHASE_UDS_DATA_INDEX] = 2;
			break;
		case P36:
			break;
		case P37:
			myPs->signal[M_SIG_BMS_EOL_FLASH_DELAY1]++;
			if(myPs->signal[M_SIG_BMS_EOL_FLASH_DELAY1] >= 100) {
				myPs->signal[M_SIG_BMS_EOL_FLASH_DELAY1] = 0;
				myPs->signal[M_SIG_BMS_EOL_FLASH_DELAY2]++;
			}

			if(myPs->signal[M_SIG_BMS_EOL_FLASH_DELAY2] == 5) {
				if(myPs->signal[M_SIG_BMS_EOL_FLASH_DELAY1] == 0) {
					myPs->signal[M_SIG_BMS_EOL_FLASH_DELAY1] = 0;
					myPs->signal[M_SIG_BMS_EOL_FLASH_DELAY2] = 0;
					myPs->signal[M_SIG_BMS_EOL_FLASH_PHASE]++;
					myPs->signal[M_SIG_BMS_EOL_FLASH_PHASE2] = P0;
				}
			}
			break;
		case P38:
			if(myPs->signal[M_SIG_EOL_PROCEDURE_INDEX] == P35
				&& myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] != P0) {
				myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]++;
			} else { //kjg_120709
				memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
				SendMsg.msg = MSG_MODULE_COA_BMS_EOL_PACK_ID_WRITE_REPLY;
				SendMsg.val[0] = 1; //OK
				send_msg(MODULE_TO_COA1+group, (char *)&SendMsg);
			}
			myPs->signal[M_SIG_BMS_EOL_FLASH_PHASE] = P0;
			break;
		case P39:
			myPs->signal[M_SIG_BMS_EOL_FLASH_DELAY1]++;
			if(myPs->signal[M_SIG_BMS_EOL_FLASH_DELAY1] >= 100) {
				myPs->signal[M_SIG_BMS_EOL_FLASH_DELAY1] = 0;
				myPs->signal[M_SIG_BMS_EOL_FLASH_DELAY2]++;
			}

			if(myPs->signal[M_SIG_BMS_EOL_FLASH_DELAY2] == 5) {
				if(myPs->signal[M_SIG_BMS_EOL_FLASH_DELAY1] == 0) {
					myPs->signal[M_SIG_BMS_EOL_FLASH_DELAY1] = 0;
					myPs->signal[M_SIG_BMS_EOL_FLASH_DELAY2] = 0;
					myPs->signal[M_SIG_BMS_EOL_FLASH_PHASE2] = P0;

					if(myPs->signal[M_SIG_EOL_PROCEDURE_INDEX] == P35
						&& myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] != P0) {
						myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P101;
					} else { //kjg_120709
						memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
						SendMsg.msg
							= MSG_MODULE_COA_BMS_EOL_PACK_ID_WRITE_REPLY;
						SendMsg.val[0] = 0; //NG
						send_msg(MODULE_TO_COA1+group, (char *)&SendMsg);
					}
					myPs->signal[M_SIG_BMS_EOL_FLASH_PHASE] = P0;
				}
			}
			break;
		case P100:
			//kjg_121112
			myPs->signal[M_SIG_BMS_EOL_FLASH_DELAY1] = 0;
			myPs->signal[M_SIG_BMS_EOL_FLASH_DELAY2] = 0;
			myPs->signal[M_SIG_BMS_EOL_FLASH_PHASE2] = P0;
			myPs->signal[M_SIG_BMS_EOL_FLASH_DELAY3] = 0;
			myPs->signal[M_SIG_BMS_EOL_FLASH_DELAY4] = 0;
			myPs->signal[M_SIG_BMS_EOL_FLASH_PHASE] = P105;
			/*kjg_121112
			//0x7DF 02 10 82 00 00 00 00 00 send
			myData->CAN.signal[0][CAN_SIG_TX_PHASE_UDS_DATA2] = 1;
			myPs->signal[M_SIG_BMS_EOL_FLASH_DELAY1] = 0;
			myPs->signal[M_SIG_BMS_EOL_FLASH_DELAY2] = 0;
			myPs->signal[M_SIG_BMS_EOL_FLASH_PHASE2] = P0;
			myPs->signal[M_SIG_BMS_EOL_FLASH_DELAY3] = 0;
			myPs->signal[M_SIG_BMS_EOL_FLASH_DELAY4] = 0;
			myPs->signal[M_SIG_BMS_EOL_FLASH_PHASE]++;
			break;
		case P101:
			myPs->signal[M_SIG_BMS_EOL_FLASH_DELAY1]++;
			if(myPs->signal[M_SIG_BMS_EOL_FLASH_DELAY1] >= 100) {
				myPs->signal[M_SIG_BMS_EOL_FLASH_DELAY1] = 0;
				myPs->signal[M_SIG_BMS_EOL_FLASH_DELAY2]++;
			}

			if(myPs->signal[M_SIG_BMS_EOL_FLASH_DELAY2] == 2) {
				if(myPs->signal[M_SIG_BMS_EOL_FLASH_DELAY1] == 0) {
					//0x7DF 02 3E 80 00 00 00 00 00 send : TesterPresent(3E)
					myData->CAN.signal[0][CAN_SIG_TX_PHASE_UDS_DATA2] = 2;
				} else if(myPs->signal[M_SIG_BMS_EOL_FLASH_DELAY1] == 10) {
					//0x7DF 02 3E 80 00 00 00 00 00 send : TesterPresent(3E)
					myData->CAN.signal[0][CAN_SIG_TX_PHASE_UDS_DATA2] = 2;
					myPs->signal[M_SIG_BMS_EOL_FLASH_PHASE2] = P1;
				} else if(myPs->signal[M_SIG_BMS_EOL_FLASH_DELAY1] == 20) {
					//Read Software Download Specification Version
					myData->CAN.signal[0][CAN_SIG_TX_PHASE_UDS_DATA3] = 1;
					myPs->signal[M_SIG_BMS_EOL_FLASH_DELAY1] = 0;
					myPs->signal[M_SIG_BMS_EOL_FLASH_DELAY2] = 0;
					myPs->signal[M_SIG_BMS_EOL_FLASH_PHASE]++;
				}
			}
			break;
		case P102:
			break;
		case P103:
			//Diagnostic Session Control : 10 programmingSession
			myData->CAN.signal[0][CAN_SIG_TX_PHASE_UDS_DATA3] = 2;
			myPs->signal[M_SIG_BMS_EOL_FLASH_PHASE]++;
			break;
		case P104:
			break;*/
		case P105:
			//ReadData By Identifier(ECU_SERIAL_NUM)
			myData->CAN.signal[0][CAN_SIG_TX_PHASE_UDS_DATA3] = 5;
			myPs->signal[M_SIG_BMS_EOL_FLASH_PHASE]++;
			myData->CAN.signal[0][CAN_SIG_TX_PHASE_UDS_DATA_INDEX] = 0;
			break;
		case P106:
			break;
		case P107:
			myPs->signal[M_SIG_BMS_EOL_FLASH_DELAY1] = 0;
			myPs->signal[M_SIG_BMS_EOL_FLASH_DELAY2] = 0;
			myPs->signal[M_SIG_BMS_EOL_FLASH_PHASE] = P0;
			myPs->signal[M_SIG_BMS_EOL_FLASH_PHASE2] = P0;
			
			if(myPs->signal[M_SIG_EOL_PROCEDURE_INDEX] == P36
				&& myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] != P0) {
				myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]++;
			} else { //kjg_120709
				memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
				SendMsg.msg = MSG_MODULE_COA_BMS_EOL_PACK_ID_CHECK_REPLY;
				SendMsg.val[0] = 1; //OK
				send_msg(MODULE_TO_COA1+group, (char *)&SendMsg);
			}
			break;
		case P108:
			myPs->signal[M_SIG_BMS_EOL_FLASH_DELAY1] = 0;
			myPs->signal[M_SIG_BMS_EOL_FLASH_DELAY2] = 0;
			myPs->signal[M_SIG_BMS_EOL_FLASH_PHASE] = P0;
			myPs->signal[M_SIG_BMS_EOL_FLASH_PHASE2] = P0;
			
			if(myPs->signal[M_SIG_EOL_PROCEDURE_INDEX] == P36
				&& myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] != P0) {
				myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P101;
			} else { //kjg_120709
				memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
				SendMsg.msg = MSG_MODULE_COA_BMS_EOL_PACK_ID_CHECK_REPLY;
				SendMsg.val[0] = 0; //NG
				send_msg(MODULE_TO_COA1+group, (char *)&SendMsg);
			}
			break;
		case P110: //kjg_120817
			//Pack_Part_Number(1464)
			myPs->signal[M_SIG_BMS_EOL_FLASH_DELAY1] = 0;
			myPs->signal[M_SIG_BMS_EOL_FLASH_DELAY2] = 0;
			myPs->signal[M_SIG_BMS_EOL_FLASH_PHASE2] = P0;
			myPs->signal[M_SIG_BMS_EOL_FLASH_DELAY3] = 0;
			myPs->signal[M_SIG_BMS_EOL_FLASH_DELAY4] = 0;

			myData->CAN.signal[0][CAN_SIG_TX_PHASE_UDS_DATA3] = 14;
			myPs->signal[M_SIG_BMS_EOL_FLASH_PHASE]++;
			myData->CAN.signal[0][CAN_SIG_TX_PHASE_UDS_DATA_INDEX] = 0;
			break;
		case P111:
			break;
		case P112:
			myPs->signal[M_SIG_BMS_EOL_FLASH_DELAY1] = 0;
			myPs->signal[M_SIG_BMS_EOL_FLASH_DELAY2] = 0;
			myPs->signal[M_SIG_BMS_EOL_FLASH_PHASE] = P0;
			myPs->signal[M_SIG_BMS_EOL_FLASH_PHASE2] = P0;
			myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]++;
			break;
		case P113:
			myPs->signal[M_SIG_BMS_EOL_FLASH_DELAY1] = 0;
			myPs->signal[M_SIG_BMS_EOL_FLASH_DELAY2] = 0;
			myPs->signal[M_SIG_BMS_EOL_FLASH_PHASE] = P0;
			myPs->signal[M_SIG_BMS_EOL_FLASH_PHASE2] = P0;
			myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P101;
			break;
		default:
			break;
	}
}

void mSignalCheck_BMS_EOL3_PHASE(void)
{
	int i, index, group=0, can_ch1, can_ch2;
	S_MSG_VAL SendMsg;

	switch(myData->AppControl.config.systemModel) { //kjg_130314
		case C_LGC_450V_200A_10A_4:
		case C_LGC_450V_200A_10A_5:
		case C_LGC_450V_200A_10A_6:
		case C_LGC_450V_200A_10A_180KW:
		case C_LGC_450V_200A_10A_180KW_2:
		case C_LGC_450V_200A_10A_180KW_3:	//kjg_130415
		case C_LGC_450V_200A_10A_180KW_4:	//jhkw_130909
		case C_LGC_450V_200A_20A_180KW:
		case C_LGC_450V_200A_20A_180KW_2:
			can_ch1 = 2;
			can_ch2 = 3;
			break;
		default:
			//kjhw_140620s
			if(myData->mData.config.division_CAN == 1) {
				can_ch1 = 0;
				can_ch2 = 0;
			} else {
				can_ch1 = 0;
				can_ch2 = 1;
			}
			//kjhw_140620s
			break;
	}

	switch(myPs->signal[M_SIG_BMS_EOL3_PHASE]) {
		case P1:
			memset((char *)&myPs->bms_eol_data3, 0, sizeof(S_BMS_EOL_DATA3));

			myData->CAN.signal[can_ch1][CAN_SIG_TX_PHASE_EOL_DATA3] = 1;
			myPs->signal[M_SIG_BMS_EOL3_DELAY1] = 0;
			myPs->signal[M_SIG_BMS_EOL3_DELAY2] = 0;
			myPs->signal[M_SIG_BMS_EOL3_PHASE]++;
			break;
		case P2:
			myPs->signal[M_SIG_BMS_EOL3_DELAY1]++;
			if(myPs->signal[M_SIG_BMS_EOL3_DELAY1] == 10) {
				myData->CAN.signal[can_ch1][CAN_SIG_TX_PHASE_EOL_DATA3] = 2;
			} else if(myPs->signal[M_SIG_BMS_EOL3_DELAY1] >= 20) {
				myPs->signal[M_SIG_BMS_EOL3_DELAY1] = 0;
				myPs->signal[M_SIG_BMS_EOL3_DELAY2] = 0;
				myPs->signal[M_SIG_BMS_EOL3_PHASE]++;
			}
			break;
		case P3:
			for(i=0; i < myData->canReceiveDataCount[can_ch1][0]; i++) {
				index = i;

				switch(myData->canReceiveSetData.normalData[can_ch1][index]
					.function_div) {
					case CAN_RX_FUNC_DIV_BMS_EOL3_UBMS_STATE:
						myPs->bms_eol_data3.UBMS_State
							= (unsigned char)myData->CanData[can_ch1][index].f_val[0];
						break;
					case CAN_RX_FUNC_DIV_BMS_EOL3_RESERVED1:
						myPs->bms_eol_data3.reserved1[0]
							= (unsigned char)myData->CanData[can_ch1][index].f_val[0];
						break;
					case CAN_RX_FUNC_DIV_BMS_EOL3_RESERVED2:
						myPs->bms_eol_data3.reserved1[1]
							= (unsigned char)myData->CanData[can_ch1][index].f_val[0];
						break;
					case CAN_RX_FUNC_DIV_BMS_EOL3_RESERVED3:
						myPs->bms_eol_data3.reserved1[2]
							= (unsigned char)myData->CanData[can_ch1][index].f_val[0];
						break;
					case CAN_RX_FUNC_DIV_BMS_EOL3_RESERVED4:
						myPs->bms_eol_data3.reserved1[3]
							= (unsigned char)myData->CanData[can_ch1][index].f_val[0];
						break;
					case CAN_RX_FUNC_DIV_BMS_EOL3_RESERVED5:
						myPs->bms_eol_data3.reserved2[0]
							= (unsigned char)myData->CanData[can_ch1][index].f_val[0];
						break;
					case CAN_RX_FUNC_DIV_BMS_EOL3_SW_VERSION:
						myPs->bms_eol_data3.SW_Version
							= (unsigned char)myData->CanData[can_ch1][index].f_val[0];
						break;
					case CAN_RX_FUNC_DIV_BMS_EOL3_HW_VERSION:
						myPs->bms_eol_data3.HW_Version
							= (unsigned char)myData->CanData[can_ch1][index].f_val[0];
						break;
					case CAN_RX_FUNC_DIV_BMS_EOL3_UBMS_ID:
						myPs->bms_eol_data3.UBMS_ID
							= (unsigned char)myData->CanData[can_ch1][index].f_val[0];
						break;
					default:
						break;
				}
			}
			myPs->signal[M_SIG_BMS_EOL3_PHASE]++;
			break;
		case P4:
			myData->CAN.signal[can_ch1][CAN_SIG_TX_PHASE_EOL_DATA3] = 3;
			myPs->signal[M_SIG_BMS_EOL3_PHASE]++;
			break;
		case P5:
			myPs->signal[M_SIG_BMS_EOL3_DELAY1]++;
			if(myPs->signal[M_SIG_BMS_EOL3_DELAY1] == 10) {
				myData->CAN.signal[can_ch1][CAN_SIG_TX_PHASE_EOL_DATA3] = 4;
			} else if(myPs->signal[M_SIG_BMS_EOL3_DELAY1] >= 20) {
				myPs->signal[M_SIG_BMS_EOL3_DELAY1] = 0;
				myPs->signal[M_SIG_BMS_EOL3_DELAY2] = 0;
				myPs->signal[M_SIG_BMS_EOL3_PHASE]++;
			}
			break;
		case P6:
			for(i=0; i < myData->canReceiveDataCount[can_ch1][0]; i++) {
				index = i;

				switch(myData->canReceiveSetData.normalData[can_ch1][index]
					.function_div) {
					case CAN_RX_FUNC_DIV_BMS_EOL3_C_V1:
						myPs->bms_eol_data3.cali_v[0]
							= (int)myData->CanData[can_ch1][index].f_val[0];
						break;
					case CAN_RX_FUNC_DIV_BMS_EOL3_C_V2:
						myPs->bms_eol_data3.cali_v[1]
							= (int)myData->CanData[can_ch1][index].f_val[0];
						break;
					case CAN_RX_FUNC_DIV_BMS_EOL3_C_V3:
						myPs->bms_eol_data3.cali_v[2]
							= (int)myData->CanData[can_ch1][index].f_val[0];
						break;
					case CAN_RX_FUNC_DIV_BMS_EOL3_C_V4:
						myPs->bms_eol_data3.cali_v[3]
							= (int)myData->CanData[can_ch1][index].f_val[0];
						break;
					case CAN_RX_FUNC_DIV_BMS_EOL3_CALI_VAL1:
						myPs->bms_eol_data3.cali_val[0]
							= (unsigned char)myData->CanData[can_ch1][index].f_val[0];
						break;
					case CAN_RX_FUNC_DIV_BMS_EOL3_CALI_VAL2:
						myPs->bms_eol_data3.cali_val[1]
							= (unsigned char)myData->CanData[can_ch1][index].f_val[0];
						break;
					case CAN_RX_FUNC_DIV_BMS_EOL3_CALI_VAL3:
						myPs->bms_eol_data3.cali_val[2]
							= (unsigned char)myData->CanData[can_ch1][index].f_val[0];
						break;
					case CAN_RX_FUNC_DIV_BMS_EOL3_CALI_VAL4:
						myPs->bms_eol_data3.cali_val[3]
							= (unsigned char)myData->CanData[can_ch1][index].f_val[0];
						break;
					default:
						break;
				}
			}
			myPs->signal[M_SIG_BMS_EOL3_PHASE]++;
			break;
		case P7:
			myData->CAN.signal[can_ch1][CAN_SIG_TX_PHASE_EOL_DATA3] = 5;
			myPs->signal[M_SIG_BMS_EOL3_PHASE]++;
			break;
		case P8:
			myPs->signal[M_SIG_BMS_EOL3_DELAY1]++;
			if(myPs->signal[M_SIG_BMS_EOL3_DELAY1] == 10) {
				myPs->signal[M_SIG_BMS_EOL3_DELAY1] = 0;
				myPs->signal[M_SIG_BMS_EOL3_DELAY2] = 0;
				myPs->signal[M_SIG_BMS_EOL3_PHASE]++;
			}
			break;
		case P9:
			for(i=0; i < myData->canReceiveDataCount[can_ch1][0]; i++) {
				index = i;

				switch(myData->canReceiveSetData.normalData[0][index]
					.function_div) {
					case CAN_RX_FUNC_DIV_BMS_EOL3_C_V1:
						myPs->bms_eol_data3.cell_v[0]
							= (int)myData->CanData[can_ch1][index].f_val[0];
						break;
					case CAN_RX_FUNC_DIV_BMS_EOL3_C_V2:
						myPs->bms_eol_data3.cell_v[1]
							= (int)myData->CanData[can_ch1][index].f_val[0];
						break;
					case CAN_RX_FUNC_DIV_BMS_EOL3_C_V3:
						myPs->bms_eol_data3.cell_v[2]
							= (int)myData->CanData[can_ch1][index].f_val[0];
						break;
					case CAN_RX_FUNC_DIV_BMS_EOL3_C_V4:
						myPs->bms_eol_data3.cell_v[3]
							= (int)myData->CanData[can_ch1][index].f_val[0];
						break;
					case CAN_RX_FUNC_DIV_BMS_EOL3_T1:
						myPs->bms_eol_data3.temp
							= (int)myData->CanData[can_ch1][index].f_val[0];
						break;
					case CAN_RX_FUNC_DIV_BMS_EOL3_M_V1:
						myPs->bms_eol_data3.module_v
							= (int)myData->CanData[can_ch1][index].f_val[0];
						break;
					case CAN_RX_FUNC_DIV_BMS_EOL3_OBD:
						myPs->bms_eol_data3.obd
							= (int)myData->CanData[can_ch1][index].f_val[0];
						break;
					default:
						break;
				}
			}
			myPs->signal[M_SIG_BMS_EOL3_PHASE] = P0;

			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
			SendMsg.msg = MSG_MODULE_COA_BMS_EOL_DATA3_REPLY;
			send_msg(MODULE_TO_COA1+group, (char *)&SendMsg);
			break;
		case P10:
			myData->CAN.signal[can_ch1][CAN_SIG_TX_PHASE_EOL_DATA3] = 3;
			myPs->signal[M_SIG_BMS_EOL3_DELAY1] = 0;
			myPs->signal[M_SIG_BMS_EOL3_DELAY2] = 0;
			myPs->signal[M_SIG_BMS_EOL3_PHASE]++;
			break;
		case P11:
			myPs->signal[M_SIG_BMS_EOL3_DELAY1]++;
			if(myPs->signal[M_SIG_BMS_EOL3_DELAY1] == 10) {
				myData->CAN.signal[can_ch1][CAN_SIG_TX_PHASE_EOL_DATA3] = 5;
			} else if(myPs->signal[M_SIG_BMS_EOL3_DELAY1] >= 100) {
				myPs->signal[M_SIG_BMS_EOL3_DELAY1] = 0;
				myPs->signal[M_SIG_BMS_EOL3_DELAY2] = 0;
				if(can_ch1 == 0) {
					if(myData->cData[0].op.state == C_STANDBY) {
						myPs->signal[M_SIG_BMS_EOL3_PHASE] = P0;
					} else {
						myPs->signal[M_SIG_BMS_EOL3_PHASE] = P10;
					}
				} else {
					if(myData->cData[1].op.state == C_STANDBY) {
						myPs->signal[M_SIG_BMS_EOL3_PHASE] = P0;
					} else {
						myPs->signal[M_SIG_BMS_EOL3_PHASE] = P10;
					}
				}
			}
			break;
		case P20:
			myData->CAN.signal[can_ch1][CAN_SIG_TX_PHASE_EOL_DATA3] = 3;
			myPs->signal[M_SIG_BMS_EOL3_DELAY1] = 0;
			myPs->signal[M_SIG_BMS_EOL3_DELAY2] = 0;
			myPs->signal[M_SIG_BMS_EOL3_PHASE]++;
			break;
		case P21:
			myPs->signal[M_SIG_BMS_EOL3_DELAY1]++;
			if(myPs->signal[M_SIG_BMS_EOL3_DELAY1] == 10) {
				myData->CAN.signal[can_ch1][CAN_SIG_TX_PHASE_EOL_DATA3] = 6;
			} else if(myPs->signal[M_SIG_BMS_EOL3_DELAY1] >= 100) {
				myPs->signal[M_SIG_BMS_EOL3_DELAY1] = 0;
				myPs->signal[M_SIG_BMS_EOL3_DELAY2] = 0;
				myPs->signal[M_SIG_BMS_EOL3_PHASE] = P0;
			}
			break;
		default:
			break;
	}
}

void mSignalCheck_EOL_PROCEDURE(void)
{
	int group=0, data_index, check_index, can_ch1, can_ch2;
	S_MSG_VAL SendMsg;

	switch(myData->AppControl.config.systemModel) { //kjg_130314
		case C_LGC_450V_200A_10A_4:
		case C_LGC_450V_200A_10A_5:
		case C_LGC_450V_200A_10A_6:
		case C_LGC_450V_200A_10A_180KW:
		case C_LGC_450V_200A_10A_180KW_2:
		case C_LGC_450V_200A_10A_180KW_3:	//kjg_130415
		case C_LGC_450V_200A_10A_180KW_4:	//jhkw_130909
		case C_LGC_450V_200A_20A_180KW:
		case C_LGC_450V_200A_20A_180KW_2:
			can_ch1 = 2;
			can_ch2 = 3;
			break;
		default:
			//kjhw_140620s
			if(myData->mData.config.division_CAN == 1) {
				can_ch1 = 0;
				can_ch2 = 0;
			} else {
				can_ch1 = 0;
				can_ch2 = 1;
			}
			//kjhw_140620s
			break;
	}

	switch(myPs->signal[M_SIG_EOL_PROCEDURE_INDEX]) {
		case P1: //EOL_MODE(1429)
			switch(myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]) {
				case P0:
					break;
				case P1:
					data_index
						= (int)myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					memset((char *)&myPs->eol_data[data_index].index, 0,
						sizeof(S_EOL_DATA));

					memset((char *)&myData->CAN.uds_rx[can_ch1].uc_val[0], 0, 8);

					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] = 0;
					myData->CAN.signal[can_ch1][CAN_SIG_TX_PHASE_UDS_DATA1] = 29;
					myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]++;

					//kjg_121004
					myPs->signal[M_SIG_BMS_EOL_FLASH_DELAY1] = 0;
					myPs->signal[M_SIG_BMS_EOL_FLASH_DELAY2] = 0;
					myPs->signal[M_SIG_BMS_EOL_FLASH_PHASE2] = P0;
					myPs->signal[M_SIG_BMS_EOL_FLASH_DELAY3] = 0;
					myPs->signal[M_SIG_BMS_EOL_FLASH_DELAY4] = 0;
					myPs->signal[M_SIG_BMS_EOL_FLASH_PHASE] = 0;
					myData->CAN.uds_rx_message_count = 0;
					myData->CAN.uds_rx_dtc_count = 0;
					myPs->signal[M_SIG_EOL_PROCEDURE_VBF_FILE_DOWNLOAD] = P0;
					break;
				case P2:
					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1]++;
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] >= 100) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2]++;
					}
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] == 1) {
						myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P101;
					}
					break;
				case P3:
					data_index
						= (int)myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					myPs->eol_data[data_index].index
						= (unsigned char)((int)myPs->signal
						[M_SIG_EOL_PROCEDURE_INDEX]
						+ (int)myPs->signal[M_SIG_EOL_PROCEDURE_DIVISION]
						* 100);
					myPs->eol_data[data_index].sub_index = (unsigned char)myPs->
						signal[M_SIG_EOL_PROCEDURE_SUB_INDEX];
					myPs->eol_data[data_index].response = 0; //ok

					memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
					SendMsg.msg = MSG_MODULE_COA_EOL_PROCEDURE_REPLY;
					SendMsg.val[0] = group;
					SendMsg.val[1]
						= (int)myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					send_msg(MODULE_TO_COA1+group, (char *)&SendMsg);
					myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]++;
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
						>= MAX_EOL_DATA) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX] = 0;
					}
					myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P0;
					break;
				default:
					data_index
						= (int)myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					myPs->eol_data[data_index].index
						= (unsigned char)((int)myPs->signal
						[M_SIG_EOL_PROCEDURE_INDEX]
						+ (int)myPs->signal[M_SIG_EOL_PROCEDURE_DIVISION]
						* 100);
					myPs->eol_data[data_index].sub_index = (unsigned char)myPs->
						signal[M_SIG_EOL_PROCEDURE_SUB_INDEX];
					myPs->eol_data[data_index].response
						= (short int)myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]
						- 100; //fail

					memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
					SendMsg.msg = MSG_MODULE_COA_EOL_PROCEDURE_REPLY;
					SendMsg.val[0] = group;
					SendMsg.val[1]
						= (int)myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					send_msg(MODULE_TO_COA1+group, (char *)&SendMsg);
					myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]++;
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
						>= MAX_EOL_DATA) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX] = 0;
					}
					myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P0;
					break;
			}
			break;
		case P2:
			switch(myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]) {
				case P0:
					break;
				case P1: //HVIL_Return Check(1436)
					data_index
						= (int)myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					memset((char *)&myPs->eol_data[data_index].index, 0,
						sizeof(S_EOL_DATA));

					memset((char *)&myData->CAN.uds_rx[can_ch1].uc_val[0], 0, 8);

					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] = 0;
					myData->CAN.signal[can_ch1][CAN_SIG_TX_PHASE_UDS_DATA1] = 36;
					myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]++;
					break;
				case P2:
					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1]++;
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] >= 100) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2]++;
					}
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] == 1) {
						myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P101;
					}
					break;
				case P3: //AUX_HVIL_Return Check(1560)
					memset((char *)&myData->CAN.uds_rx[can_ch1].uc_val[0], 0, 8);

					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] = 0;
					myData->CAN.signal[can_ch1][CAN_SIG_TX_PHASE_EOL_DATA2] = 10;
					myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]++;
					break;
				case P4:
					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1]++;
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] >= 100) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2]++;
					}
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] == 1) {
						myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P102;
					}
					break;
				case P5:
					data_index
						= (int)myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					myPs->eol_data[data_index].index
						= (unsigned char)((int)myPs->signal
						[M_SIG_EOL_PROCEDURE_INDEX]
						+ (int)myPs->signal[M_SIG_EOL_PROCEDURE_DIVISION]
						* 100);
					myPs->eol_data[data_index].sub_index = (unsigned char)myPs->
						signal[M_SIG_EOL_PROCEDURE_SUB_INDEX];
					myPs->eol_data[data_index].response = 0; //ok

					memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
					SendMsg.msg = MSG_MODULE_COA_EOL_PROCEDURE_REPLY;
					SendMsg.val[0] = group;
					SendMsg.val[1]
						= (int)myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					send_msg(MODULE_TO_COA1+group, (char *)&SendMsg);
					myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]++;
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
						>= MAX_EOL_DATA) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX] = 0;
					}
					myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P0;
					break;
				default:
					data_index
						= (int)myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					myPs->eol_data[data_index].index
						= (unsigned char)((int)myPs->signal
						[M_SIG_EOL_PROCEDURE_INDEX]
						+ (int)myPs->signal[M_SIG_EOL_PROCEDURE_DIVISION]
						* 100);
					myPs->eol_data[data_index].sub_index = (unsigned char)myPs->
						signal[M_SIG_EOL_PROCEDURE_SUB_INDEX];
					myPs->eol_data[data_index].response
						= (short int)myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]
						- 100; //fail

					memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
					SendMsg.msg = MSG_MODULE_COA_EOL_PROCEDURE_REPLY;
					SendMsg.val[0] = group;
					SendMsg.val[1]
						= (int)myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					send_msg(MODULE_TO_COA1+group, (char *)&SendMsg);
					myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]++;
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
						>= MAX_EOL_DATA) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX] = 0;
					}
					myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P0;
					break;
			}
			break;
		case P3: //CVTN_ID Assign(1401)
			switch(myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]) {
				case P0:
					break;
				case P1:
					data_index
						= (int)myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					memset((char *)&myPs->eol_data[data_index].index, 0,
						sizeof(S_EOL_DATA));

					memset((char *)&myData->CAN.uds_rx[can_ch1].uc_val[0], 0, 8);

					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] = 0;
					myData->CAN.signal[can_ch1][CAN_SIG_TX_PHASE_UDS_DATA1] = 1;
					myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]++;
					break;
				case P2:
					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1]++;
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] >= 100) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2]++;
					}
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] == 10) {
						myData->CAN.signal[can_ch1][CAN_SIG_TX_PHASE_UDS_DATA1] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]++;
					}
					break;
				case P3:
					data_index
						= (int)myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					myPs->eol_data[data_index].index
						= (unsigned char)((int)myPs->signal
						[M_SIG_EOL_PROCEDURE_INDEX]
						+ (int)myPs->signal[M_SIG_EOL_PROCEDURE_DIVISION]
						* 100);
					myPs->eol_data[data_index].sub_index = (unsigned char)myPs->
						signal[M_SIG_EOL_PROCEDURE_SUB_INDEX];
					myPs->eol_data[data_index].response = 0; //ok

					memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
					SendMsg.msg = MSG_MODULE_COA_EOL_PROCEDURE_REPLY;
					SendMsg.val[0] = group;
					SendMsg.val[1]
						= (int)myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					send_msg(MODULE_TO_COA1+group, (char *)&SendMsg);
					myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]++;
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
						>= MAX_EOL_DATA) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX] = 0;
					}
					myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P0;
					break;
				default:
					data_index
						= (int)myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					myPs->eol_data[data_index].index
						= (unsigned char)((int)myPs->signal
						[M_SIG_EOL_PROCEDURE_INDEX]
						+ (int)myPs->signal[M_SIG_EOL_PROCEDURE_DIVISION]
						* 100);
					myPs->eol_data[data_index].sub_index = (unsigned char)myPs->
						signal[M_SIG_EOL_PROCEDURE_SUB_INDEX];
					myPs->eol_data[data_index].response
						= (short int)myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]
						- 100; //fail

					memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
					SendMsg.msg = MSG_MODULE_COA_EOL_PROCEDURE_REPLY;
					SendMsg.val[0] = group;
					SendMsg.val[1]
						= (int)myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					send_msg(MODULE_TO_COA1+group, (char *)&SendMsg);
					myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]++;
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
						>= MAX_EOL_DATA) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX] = 0;
					}
					myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P0;
					break;
			}
			break;
		case P4: //CVTN_ID Check(1402)
			switch(myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]) {
				case P0:
					break;
				case P1:
					data_index
						= (int)myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					memset((char *)&myPs->eol_data[data_index].index, 0,
						sizeof(S_EOL_DATA));

					memset((char *)&myData->CAN.uds_rx[can_ch1].uc_val[0], 0, 8);

					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] = 0;
					myData->CAN.signal[can_ch1][CAN_SIG_TX_PHASE_UDS_DATA1] = 2;
					myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]++;
					break;
				case P2:
					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1]++;
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] >= 100) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2]++;
					}
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] == 1) {
						myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P101;
					}
					break;
				case P3:
					data_index
						= (int)myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					myPs->eol_data[data_index].index
						= (unsigned char)((int)myPs->signal
						[M_SIG_EOL_PROCEDURE_INDEX]
						+ (int)myPs->signal[M_SIG_EOL_PROCEDURE_DIVISION]
						* 100);
					myPs->eol_data[data_index].sub_index = (unsigned char)myPs->
						signal[M_SIG_EOL_PROCEDURE_SUB_INDEX];
					myPs->eol_data[data_index].response = 0; //ok

					memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
					SendMsg.msg = MSG_MODULE_COA_EOL_PROCEDURE_REPLY;
					SendMsg.val[0] = group;
					SendMsg.val[1]
						= (int)myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					send_msg(MODULE_TO_COA1+group, (char *)&SendMsg);
					myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]++;
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
						>= MAX_EOL_DATA) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX] = 0;
					}
					myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P0;
					break;
				default:
					data_index
						= (int)myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					myPs->eol_data[data_index].index
						= (unsigned char)((int)myPs->signal
						[M_SIG_EOL_PROCEDURE_INDEX]
						+ (int)myPs->signal[M_SIG_EOL_PROCEDURE_DIVISION]
						* 100);
					myPs->eol_data[data_index].sub_index = (unsigned char)myPs->
						signal[M_SIG_EOL_PROCEDURE_SUB_INDEX];
					myPs->eol_data[data_index].response
						= (short int)myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]
						- 100; //fail

					memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
					SendMsg.msg = MSG_MODULE_COA_EOL_PROCEDURE_REPLY;
					SendMsg.val[0] = group;
					SendMsg.val[1]
						= (int)myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					send_msg(MODULE_TO_COA1+group, (char *)&SendMsg);
					myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]++;
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
						>= MAX_EOL_DATA) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX] = 0;
					}
					myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P0;
					break;
			}
			break;
		case P5: //SW_VERSION Check(1403)
			switch(myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]) {
				case P0:
					break;
				case P1:
					data_index
						= (int)myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					memset((char *)&myPs->eol_data[data_index].index, 0,
						sizeof(S_EOL_DATA));

					memset((char *)&myData->CAN.uds_rx[can_ch1].uc_val[0], 0, 8);

					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] = 0;
					myData->CAN.signal[can_ch1][CAN_SIG_TX_PHASE_UDS_DATA1] = 3;
					myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]++;
					break;
				case P2:
					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1]++;
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] >= 100) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2]++;
					}
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] == 1) {
						myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P101;
					}
					break;
				case P3:
					data_index
						= (int)myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					myPs->eol_data[data_index].index
						= (unsigned char)((int)myPs->signal
						[M_SIG_EOL_PROCEDURE_INDEX]
						+ (int)myPs->signal[M_SIG_EOL_PROCEDURE_DIVISION]
						* 100);
					myPs->eol_data[data_index].sub_index = (unsigned char)myPs->
						signal[M_SIG_EOL_PROCEDURE_SUB_INDEX];
					myPs->eol_data[data_index].response = 0; //ok

					memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
					SendMsg.msg = MSG_MODULE_COA_EOL_PROCEDURE_REPLY;
					SendMsg.val[0] = group;
					SendMsg.val[1]
						= (int)myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					send_msg(MODULE_TO_COA1+group, (char *)&SendMsg);
					myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]++;
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
						>= MAX_EOL_DATA) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX] = 0;
					}
					myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P0;
					break;
				default:
					data_index
						= (int)myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					myPs->eol_data[data_index].index
						= (unsigned char)((int)myPs->signal
						[M_SIG_EOL_PROCEDURE_INDEX]
						+ (int)myPs->signal[M_SIG_EOL_PROCEDURE_DIVISION]
						* 100);
					myPs->eol_data[data_index].sub_index = (unsigned char)myPs->
						signal[M_SIG_EOL_PROCEDURE_SUB_INDEX];
					myPs->eol_data[data_index].response
						= (short int)myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]
						- 100; //fail

					memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
					SendMsg.msg = MSG_MODULE_COA_EOL_PROCEDURE_REPLY;
					SendMsg.val[0] = group;
					SendMsg.val[1]
						= (int)myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					send_msg(MODULE_TO_COA1+group, (char *)&SendMsg);
					myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]++;
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
						>= MAX_EOL_DATA) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX] = 0;
					}
					myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P0;
					break;
			}
			break;
		case P6: //Fine_Current_Sense_Wire Check(1438)
			switch(myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]) {
				case P0:
					break;
				case P1:
					data_index
						= (int)myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					memset((char *)&myPs->eol_data[data_index].index, 0,
						sizeof(S_EOL_DATA));

					memset((char *)&myData->CAN.uds_rx[can_ch1].uc_val[0], 0, 8);

					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] = 0;
					myData->CAN.signal[can_ch1][CAN_SIG_TX_PHASE_UDS_DATA1] = 38;
					myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]++;
					break;
				case P2:
					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1]++;
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] >= 100) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2]++;
					}
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] == 1) {
						myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P101;
					}
					break;
				case P3:
					data_index
						= (int)myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					myPs->eol_data[data_index].index
						= (unsigned char)((int)myPs->signal
						[M_SIG_EOL_PROCEDURE_INDEX]
						+ (int)myPs->signal[M_SIG_EOL_PROCEDURE_DIVISION]
						* 100);
					myPs->eol_data[data_index].sub_index = (unsigned char)myPs->
						signal[M_SIG_EOL_PROCEDURE_SUB_INDEX];
					myPs->eol_data[data_index].response = 0; //ok

					memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
					SendMsg.msg = MSG_MODULE_COA_EOL_PROCEDURE_REPLY;
					SendMsg.val[0] = group;
					SendMsg.val[1]
						= (int)myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					send_msg(MODULE_TO_COA1+group, (char *)&SendMsg);
					myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]++;
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
						>= MAX_EOL_DATA) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX] = 0;
					}
					myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P0;
					break;
				default:
					data_index
						= (int)myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					myPs->eol_data[data_index].index
						= (unsigned char)((int)myPs->signal
						[M_SIG_EOL_PROCEDURE_INDEX]
						+ (int)myPs->signal[M_SIG_EOL_PROCEDURE_DIVISION]
						* 100);
					myPs->eol_data[data_index].sub_index = (unsigned char)myPs->
						signal[M_SIG_EOL_PROCEDURE_SUB_INDEX];
					myPs->eol_data[data_index].response
						= (short int)myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]
						- 100; //fail

					memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
					SendMsg.msg = MSG_MODULE_COA_EOL_PROCEDURE_REPLY;
					SendMsg.val[0] = group;
					SendMsg.val[1]
						= (int)myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					send_msg(MODULE_TO_COA1+group, (char *)&SendMsg);
					myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]++;
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
						>= MAX_EOL_DATA) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX] = 0;
					}
					myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P0;
					break;
			}
			break;
		case P7: //Coarse_Current_Sense_Wire Check(1439)
			switch(myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]) {
				case P0:
					break;
				case P1:
					data_index
						= (int)myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					memset((char *)&myPs->eol_data[data_index].index, 0,
						sizeof(S_EOL_DATA));

					memset((char *)&myData->CAN.uds_rx[can_ch1].uc_val[0], 0, 8);

					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] = 0;
					myData->CAN.signal[can_ch1][CAN_SIG_TX_PHASE_UDS_DATA1] = 39;
					myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]++;
					break;
				case P2:
					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1]++;
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] >= 100) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2]++;
					}
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] == 1) {
						myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P101;
					}
					break;
				case P3:
					data_index
						= (int)myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					myPs->eol_data[data_index].index
						= (unsigned char)((int)myPs->signal
						[M_SIG_EOL_PROCEDURE_INDEX]
						+ (int)myPs->signal[M_SIG_EOL_PROCEDURE_DIVISION]
						* 100);
					myPs->eol_data[data_index].sub_index = (unsigned char)myPs->
						signal[M_SIG_EOL_PROCEDURE_SUB_INDEX];
					myPs->eol_data[data_index].response = 0; //ok

					memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
					SendMsg.msg = MSG_MODULE_COA_EOL_PROCEDURE_REPLY;
					SendMsg.val[0] = group;
					SendMsg.val[1]
						= (int)myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					send_msg(MODULE_TO_COA1+group, (char *)&SendMsg);
					myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]++;
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
						>= MAX_EOL_DATA) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX] = 0;
					}
					myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P0;
					break;
				default:
					data_index
						= (int)myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					myPs->eol_data[data_index].index
						= (unsigned char)((int)myPs->signal
						[M_SIG_EOL_PROCEDURE_INDEX]
						+ (int)myPs->signal[M_SIG_EOL_PROCEDURE_DIVISION]
						* 100);
					myPs->eol_data[data_index].sub_index = (unsigned char)myPs->
						signal[M_SIG_EOL_PROCEDURE_SUB_INDEX];
					myPs->eol_data[data_index].response
						= (short int)myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]
						- 100; //fail

					memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
					SendMsg.msg = MSG_MODULE_COA_EOL_PROCEDURE_REPLY;
					SendMsg.val[0] = group;
					SendMsg.val[1]
						= (int)myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					send_msg(MODULE_TO_COA1+group, (char *)&SendMsg);
					myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]++;
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
						>= MAX_EOL_DATA) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX] = 0;
					}
					myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P0;
					break;
			}
			break;
		case P8: //Disable_ISO(1431)
			switch(myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]) {
				case P0:
					break;
				case P1:
					data_index
						= (int)myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					memset((char *)&myPs->eol_data[data_index].index, 0,
						sizeof(S_EOL_DATA));

					memset((char *)&myData->CAN.uds_rx[can_ch1].uc_val[0], 0, 8);

					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] = 0;
					myData->CAN.signal[can_ch1][CAN_SIG_TX_PHASE_UDS_DATA1] = 31;
					myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]++;
					break;
				case P2:
					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1]++;
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] >= 100) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2]++;
					}
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] == 1) {
						myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P101;
					}
					break;
				case P3:
					data_index
						= (int)myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					myPs->eol_data[data_index].index
						= (unsigned char)((int)myPs->signal
						[M_SIG_EOL_PROCEDURE_INDEX]
						+ (int)myPs->signal[M_SIG_EOL_PROCEDURE_DIVISION]
						* 100);
					myPs->eol_data[data_index].sub_index = (unsigned char)myPs->
						signal[M_SIG_EOL_PROCEDURE_SUB_INDEX];
					myPs->eol_data[data_index].response = 0; //ok

					memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
					SendMsg.msg = MSG_MODULE_COA_EOL_PROCEDURE_REPLY;
					SendMsg.val[0] = group;
					SendMsg.val[1]
						= (int)myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					send_msg(MODULE_TO_COA1+group, (char *)&SendMsg);
					myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]++;
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
						>= MAX_EOL_DATA) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX] = 0;
					}
					myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P0;
					break;
				default:
					data_index
						= (int)myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					myPs->eol_data[data_index].index
						= (unsigned char)((int)myPs->signal
						[M_SIG_EOL_PROCEDURE_INDEX]
						+ (int)myPs->signal[M_SIG_EOL_PROCEDURE_DIVISION]
						* 100);
					myPs->eol_data[data_index].sub_index = (unsigned char)myPs->
						signal[M_SIG_EOL_PROCEDURE_SUB_INDEX];
					myPs->eol_data[data_index].response
						= (short int)myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]
						- 100; //fail

					memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
					SendMsg.msg = MSG_MODULE_COA_EOL_PROCEDURE_REPLY;
					SendMsg.val[0] = group;
					SendMsg.val[1]
						= (int)myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					send_msg(MODULE_TO_COA1+group, (char *)&SendMsg);
					myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]++;
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
						>= MAX_EOL_DATA) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX] = 0;
					}
					myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P0;
					break;
			}
			break;
		case P9:
			switch(myPs->signal[M_SIG_EOL_PROCEDURE_SUB_INDEX]) {
				case P0: //Main+, Charger Precharge Close(1440)
					switch(myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]) {
						case P0:
							break;
						case P1:
							data_index = (int)myPs->signal
								[M_SIG_EOL_PROCEDURE_DATA_INDEX];
							memset((char *)&myPs->eol_data[data_index].index,
								0, sizeof(S_EOL_DATA));

							memset((char *)&myData->CAN.uds_rx[can_ch1].uc_val[0],
								0, 8);

							myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
							myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] = 0;
							myData->CAN.signal[can_ch1][CAN_SIG_TX_PHASE_UDS_DATA1]
								= 40;
							myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]++;
							break;
						case P2:
							myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1]++;
							if(myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1]
								>= 100) {
								myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
								myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2]++;
							}
							if(myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] == 1) {
								myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P101;
							}
							break;
						case P3:
							data_index = (int)myPs->signal
								[M_SIG_EOL_PROCEDURE_DATA_INDEX];
							myPs->eol_data[data_index].index
								= (unsigned char)((int)myPs->signal
								[M_SIG_EOL_PROCEDURE_INDEX]
								+ (int)myPs->signal
								[M_SIG_EOL_PROCEDURE_DIVISION] * 100);
							myPs->eol_data[data_index].sub_index
								= (unsigned char)myPs->
								signal[M_SIG_EOL_PROCEDURE_SUB_INDEX];
							myPs->eol_data[data_index].response = 0; //ok

							memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
							SendMsg.msg = MSG_MODULE_COA_EOL_PROCEDURE_REPLY;
							SendMsg.val[0] = group;
							SendMsg.val[1] = (int)myPs->signal
								[M_SIG_EOL_PROCEDURE_DATA_INDEX];
							send_msg(MODULE_TO_COA1+group, (char *)&SendMsg);
							myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]++;
							if(myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
								>= MAX_EOL_DATA) {
								myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
									= 0;
							}
							myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P0;
							break;
						default:
							data_index = (int)myPs->signal
								[M_SIG_EOL_PROCEDURE_DATA_INDEX];
							myPs->eol_data[data_index].index
								= (unsigned char)((int)myPs->signal
								[M_SIG_EOL_PROCEDURE_INDEX]
								+ (int)myPs->signal
								[M_SIG_EOL_PROCEDURE_DIVISION] * 100);
							myPs->eol_data[data_index].sub_index
								= (unsigned char)myPs->
								signal[M_SIG_EOL_PROCEDURE_SUB_INDEX];
							myPs->eol_data[data_index].response
								= (short int)myPs->signal
								[M_SIG_EOL_PROCEDURE_PHASE] - 100; //fail

							memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
							SendMsg.msg = MSG_MODULE_COA_EOL_PROCEDURE_REPLY;
							SendMsg.val[0] = group;
							SendMsg.val[1] = (int)myPs->signal
								[M_SIG_EOL_PROCEDURE_DATA_INDEX];
							send_msg(MODULE_TO_COA1+group, (char *)&SendMsg);
							myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]++;
							if(myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
								>= MAX_EOL_DATA) {
								myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
									= 0;
							}
							myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P0;
							break;
					}
					break;
				case P1: //Main+, Charger Precharge Open(1441)
					switch(myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]) {
						case P0:
							break;
						case P1:
							data_index = (int)myPs->signal
								[M_SIG_EOL_PROCEDURE_DATA_INDEX];
							memset((char *)&myPs->eol_data[data_index].index,
								0, sizeof(S_EOL_DATA));

							memset((char *)&myData->CAN.uds_rx[can_ch1].uc_val[0],
								0, 8);

							myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
							myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] = 0;
							myData->CAN.signal[can_ch1][CAN_SIG_TX_PHASE_UDS_DATA1]
								= 41;
							myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]++;
							break;
						case P2:
							myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1]++;
							if(myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1]
								>= 100) {
								myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
								myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2]++;
							}
							if(myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] == 1) {
								myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P101;
							}
							break;
						case P3:
							data_index = (int)myPs->signal
								[M_SIG_EOL_PROCEDURE_DATA_INDEX];
							myPs->eol_data[data_index].index
								= (unsigned char)((int)myPs->signal
								[M_SIG_EOL_PROCEDURE_INDEX]
								+ (int)myPs->signal
								[M_SIG_EOL_PROCEDURE_DIVISION] * 100);
							myPs->eol_data[data_index].sub_index
								= (unsigned char)myPs->
								signal[M_SIG_EOL_PROCEDURE_SUB_INDEX];
							myPs->eol_data[data_index].response = 0; //ok

							memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
							SendMsg.msg = MSG_MODULE_COA_EOL_PROCEDURE_REPLY;
							SendMsg.val[0] = group;
							SendMsg.val[1] = (int)myPs->signal
								[M_SIG_EOL_PROCEDURE_DATA_INDEX];
							send_msg(MODULE_TO_COA1+group, (char *)&SendMsg);
							myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]++;
							if(myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
								>= MAX_EOL_DATA) {
								myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
									= 0;
							}
							myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P0;
							break;
						default:
							data_index = (int)myPs->signal
								[M_SIG_EOL_PROCEDURE_DATA_INDEX];
							myPs->eol_data[data_index].index
								= (unsigned char)((int)myPs->signal
								[M_SIG_EOL_PROCEDURE_INDEX]
								+ (int)myPs->signal
								[M_SIG_EOL_PROCEDURE_DIVISION] * 100);
							myPs->eol_data[data_index].sub_index
								= (unsigned char)myPs->
								signal[M_SIG_EOL_PROCEDURE_SUB_INDEX];
							myPs->eol_data[data_index].response
								= (short int)myPs->signal
								[M_SIG_EOL_PROCEDURE_PHASE] - 100; //fail

							memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
							SendMsg.msg = MSG_MODULE_COA_EOL_PROCEDURE_REPLY;
							SendMsg.val[0] = group;
							SendMsg.val[1] = (int)myPs->signal
								[M_SIG_EOL_PROCEDURE_DATA_INDEX];
							send_msg(MODULE_TO_COA1+group, (char *)&SendMsg);
							myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]++;
							if(myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
								>= MAX_EOL_DATA) {
								myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
									= 0;
							}
							myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P0;
							break;
					}
					break;
				default:
					break;
			}
			break;
		case P10:
			switch(myPs->signal[M_SIG_EOL_PROCEDURE_SUB_INDEX]) {
				case P0: //Charge+, Precharge Close(1441)
					switch(myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]) {
						case P0:
							break;
						case P1:
							data_index = (int)myPs->signal
								[M_SIG_EOL_PROCEDURE_DATA_INDEX];
							memset((char *)&myPs->eol_data[data_index].index,
								0, sizeof(S_EOL_DATA));

							memset((char *)&myData->CAN.uds_rx[can_ch1].uc_val[0],
								0, 8);

							myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
							myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] = 0;
							myData->CAN.signal[can_ch1][CAN_SIG_TX_PHASE_UDS_DATA1]
								= 42;
							myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]++;
							break;
						case P2:
							myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1]++;
							if(myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1]
								>= 100) {
								myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
								myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2]++;
							}
							if(myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] == 1) {
								myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P101;
							}
							break;
						case P3:
							data_index = (int)myPs->signal
								[M_SIG_EOL_PROCEDURE_DATA_INDEX];
							myPs->eol_data[data_index].index
								= (unsigned char)((int)myPs->signal
								[M_SIG_EOL_PROCEDURE_INDEX]
								+ (int)myPs->signal
								[M_SIG_EOL_PROCEDURE_DIVISION] * 100);
							myPs->eol_data[data_index].sub_index
								= (unsigned char)myPs->
								signal[M_SIG_EOL_PROCEDURE_SUB_INDEX];
							myPs->eol_data[data_index].response = 0; //ok

							memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
							SendMsg.msg = MSG_MODULE_COA_EOL_PROCEDURE_REPLY;
							SendMsg.val[0] = group;
							SendMsg.val[1] = (int)myPs->signal
								[M_SIG_EOL_PROCEDURE_DATA_INDEX];
							send_msg(MODULE_TO_COA1+group, (char *)&SendMsg);
							myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]++;
							if(myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
								>= MAX_EOL_DATA) {
								myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
									= 0;
							}
							myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P0;
							break;
						default:
							data_index = (int)myPs->signal
								[M_SIG_EOL_PROCEDURE_DATA_INDEX];
							myPs->eol_data[data_index].index
								= (unsigned char)((int)myPs->signal
								[M_SIG_EOL_PROCEDURE_INDEX]
								+ (int)myPs->signal
								[M_SIG_EOL_PROCEDURE_DIVISION] * 100);
							myPs->eol_data[data_index].sub_index
								= (unsigned char)myPs->
								signal[M_SIG_EOL_PROCEDURE_SUB_INDEX];
							myPs->eol_data[data_index].response
								= (short int)myPs->signal
								[M_SIG_EOL_PROCEDURE_PHASE] - 100; //fail

							memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
							SendMsg.msg = MSG_MODULE_COA_EOL_PROCEDURE_REPLY;
							SendMsg.val[0] = group;
							SendMsg.val[1] = (int)myPs->signal
								[M_SIG_EOL_PROCEDURE_DATA_INDEX];
							send_msg(MODULE_TO_COA1+group, (char *)&SendMsg);
							myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]++;
							if(myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
								>= MAX_EOL_DATA) {
								myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
									= 0;
							}
							myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P0;
							break;
					}
					break;
				case P1: //Charge+, Precharge Open(1443)
					switch(myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]) {
						case P0:
							break;
						case P1:
							data_index = (int)myPs->signal
								[M_SIG_EOL_PROCEDURE_DATA_INDEX];
							memset((char *)&myPs->eol_data[data_index].index,
								0, sizeof(S_EOL_DATA));

							memset((char *)&myData->CAN.uds_rx[can_ch1].uc_val[0],
								0, 8);

							myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
							myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] = 0;
							myData->CAN.signal[can_ch1][CAN_SIG_TX_PHASE_UDS_DATA1]
								= 43;
							myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]++;
							break;
						case P2:
							myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1]++;
							if(myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1]
								>= 100) {
								myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
								myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2]++;
							}
							if(myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] == 1) {
								myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P101;
							}
							break;
						case P3:
							data_index = (int)myPs->signal
								[M_SIG_EOL_PROCEDURE_DATA_INDEX];
							myPs->eol_data[data_index].index
								= (unsigned char)((int)myPs->signal
								[M_SIG_EOL_PROCEDURE_INDEX]
								+ (int)myPs->signal
								[M_SIG_EOL_PROCEDURE_DIVISION] * 100);
							myPs->eol_data[data_index].sub_index
								= (unsigned char)myPs->
								signal[M_SIG_EOL_PROCEDURE_SUB_INDEX];
							myPs->eol_data[data_index].response = 0; //ok

							memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
							SendMsg.msg = MSG_MODULE_COA_EOL_PROCEDURE_REPLY;
							SendMsg.val[0] = group;
							SendMsg.val[1] = (int)myPs->signal
								[M_SIG_EOL_PROCEDURE_DATA_INDEX];
							send_msg(MODULE_TO_COA1+group, (char *)&SendMsg);
							myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]++;
							if(myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
								>= MAX_EOL_DATA) {
								myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
									= 0;
							}
							myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P0;
							break;
						default:
							data_index = (int)myPs->signal
								[M_SIG_EOL_PROCEDURE_DATA_INDEX];
							myPs->eol_data[data_index].index
								= (unsigned char)((int)myPs->signal
								[M_SIG_EOL_PROCEDURE_INDEX]
								+ (int)myPs->signal
								[M_SIG_EOL_PROCEDURE_DIVISION] * 100);
							myPs->eol_data[data_index].sub_index
								= (unsigned char)myPs->
								signal[M_SIG_EOL_PROCEDURE_SUB_INDEX];
							myPs->eol_data[data_index].response
								= (short int)myPs->signal
								[M_SIG_EOL_PROCEDURE_PHASE] - 100; //fail

							memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
							SendMsg.msg = MSG_MODULE_COA_EOL_PROCEDURE_REPLY;
							SendMsg.val[0] = group;
							SendMsg.val[1] = (int)myPs->signal
								[M_SIG_EOL_PROCEDURE_DATA_INDEX];
							send_msg(MODULE_TO_COA1+group, (char *)&SendMsg);
							myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]++;
							if(myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
								>= MAX_EOL_DATA) {
								myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
									= 0;
							}
							myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P0;
							break;
					}
					break;
				default:
					break;
			}
			break;
		case P11:
			switch(myPs->signal[M_SIG_EOL_PROCEDURE_SUB_INDEX]) {
				case P0: //Main+ Close(1444)
					switch(myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]) {
						case P0:
							break;
						case P1:
							data_index = (int)myPs->signal
								[M_SIG_EOL_PROCEDURE_DATA_INDEX];
							memset((char *)&myPs->eol_data[data_index].index,
								0, sizeof(S_EOL_DATA));

							memset((char *)&myData->CAN.uds_rx[can_ch1].uc_val[0],
								0, 8);

							myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
							myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] = 0;
							myData->CAN.signal[can_ch1][CAN_SIG_TX_PHASE_UDS_DATA1]
								= 44;
							myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]++;
							break;
						case P2:
							myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1]++;
							if(myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1]
								>= 100) {
								myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
								myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2]++;
							}
							if(myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] == 1) {
								myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P101;
							}
							break;
						case P3:
							data_index = (int)myPs->signal
								[M_SIG_EOL_PROCEDURE_DATA_INDEX];
							myPs->eol_data[data_index].index
								= (unsigned char)((int)myPs->signal
								[M_SIG_EOL_PROCEDURE_INDEX]
								+ (int)myPs->signal
								[M_SIG_EOL_PROCEDURE_DIVISION] * 100);
							myPs->eol_data[data_index].sub_index
								= (unsigned char)myPs->
								signal[M_SIG_EOL_PROCEDURE_SUB_INDEX];
							myPs->eol_data[data_index].response = 0; //ok

							memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
							SendMsg.msg = MSG_MODULE_COA_EOL_PROCEDURE_REPLY;
							SendMsg.val[0] = group;
							SendMsg.val[1] = (int)myPs->signal
								[M_SIG_EOL_PROCEDURE_DATA_INDEX];
							send_msg(MODULE_TO_COA1+group, (char *)&SendMsg);
							myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]++;
							if(myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
								>= MAX_EOL_DATA) {
								myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
									= 0;
							}
							myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P0;
							break;
						default:
							data_index = (int)myPs->signal
								[M_SIG_EOL_PROCEDURE_DATA_INDEX];
							myPs->eol_data[data_index].index
								= (unsigned char)((int)myPs->signal
								[M_SIG_EOL_PROCEDURE_INDEX]
								+ (int)myPs->signal
								[M_SIG_EOL_PROCEDURE_DIVISION] * 100);
							myPs->eol_data[data_index].sub_index
								= (unsigned char)myPs->
								signal[M_SIG_EOL_PROCEDURE_SUB_INDEX];
							myPs->eol_data[data_index].response
								= (short int)myPs->signal
								[M_SIG_EOL_PROCEDURE_PHASE] - 100; //fail

							memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
							SendMsg.msg = MSG_MODULE_COA_EOL_PROCEDURE_REPLY;
							SendMsg.val[0] = group;
							SendMsg.val[1] = (int)myPs->signal
								[M_SIG_EOL_PROCEDURE_DATA_INDEX];
							send_msg(MODULE_TO_COA1+group, (char *)&SendMsg);
							myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]++;
							if(myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
								>= MAX_EOL_DATA) {
								myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
									= 0;
							}
							myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P0;
							break;
					}
					break;
				case P1: //Main+ Open(1445)
					switch(myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]) {
						case P0:
							break;
						case P1:
							data_index = (int)myPs->signal
								[M_SIG_EOL_PROCEDURE_DATA_INDEX];
							memset((char *)&myPs->eol_data[data_index].index,
								0, sizeof(S_EOL_DATA));

							memset((char *)&myData->CAN.uds_rx[can_ch1].uc_val[0],
								0, 8);

							myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
							myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] = 0;
							myData->CAN.signal[can_ch1][CAN_SIG_TX_PHASE_UDS_DATA1]
								= 45;
							myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]++;
							break;
						case P2:
							myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1]++;
							if(myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1]
								>= 100) {
								myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
								myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2]++;
							}
							if(myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] == 1) {
								myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P101;
							}
							break;
						case P3:
							data_index = (int)myPs->signal
								[M_SIG_EOL_PROCEDURE_DATA_INDEX];
							myPs->eol_data[data_index].index
								= (unsigned char)((int)myPs->signal
								[M_SIG_EOL_PROCEDURE_INDEX]
								+ (int)myPs->signal
								[M_SIG_EOL_PROCEDURE_DIVISION] * 100);
							myPs->eol_data[data_index].sub_index
								= (unsigned char)myPs->
								signal[M_SIG_EOL_PROCEDURE_SUB_INDEX];
							myPs->eol_data[data_index].response = 0; //ok

							memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
							SendMsg.msg = MSG_MODULE_COA_EOL_PROCEDURE_REPLY;
							SendMsg.val[0] = group;
							SendMsg.val[1] = (int)myPs->signal
								[M_SIG_EOL_PROCEDURE_DATA_INDEX];
							send_msg(MODULE_TO_COA1+group, (char *)&SendMsg);
							myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]++;
							if(myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
								>= MAX_EOL_DATA) {
								myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
									= 0;
							}
							myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P0;
							break;
						default:
							data_index = (int)myPs->signal
								[M_SIG_EOL_PROCEDURE_DATA_INDEX];
							myPs->eol_data[data_index].index
								= (unsigned char)((int)myPs->signal
								[M_SIG_EOL_PROCEDURE_INDEX]
								+ (int)myPs->signal
								[M_SIG_EOL_PROCEDURE_DIVISION] * 100);
							myPs->eol_data[data_index].sub_index
								= (unsigned char)myPs->
								signal[M_SIG_EOL_PROCEDURE_SUB_INDEX];
							myPs->eol_data[data_index].response
								= (short int)myPs->signal
								[M_SIG_EOL_PROCEDURE_PHASE] - 100; //fail

							memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
							SendMsg.msg = MSG_MODULE_COA_EOL_PROCEDURE_REPLY;
							SendMsg.val[0] = group;
							SendMsg.val[1] = (int)myPs->signal
								[M_SIG_EOL_PROCEDURE_DATA_INDEX];
							send_msg(MODULE_TO_COA1+group, (char *)&SendMsg);
							myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]++;
							if(myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
								>= MAX_EOL_DATA) {
								myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
									= 0;
							}
							myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P0;
							break;
					}
					break;
				default:
					break;
			}
			break;
		case P12:
			switch(myPs->signal[M_SIG_EOL_PROCEDURE_SUB_INDEX]) {
				case P0: //Charge+ Close(1446)
					switch(myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]) {
						case P0:
							break;
						case P1:
							data_index = (int)myPs->signal
								[M_SIG_EOL_PROCEDURE_DATA_INDEX];
							memset((char *)&myPs->eol_data[data_index].index,
								0, sizeof(S_EOL_DATA));

							memset((char *)&myData->CAN.uds_rx[can_ch1].uc_val[0],
								0, 8);

							myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
							myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] = 0;
							myData->CAN.signal[can_ch1][CAN_SIG_TX_PHASE_UDS_DATA1]
								= 46;
							myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]++;
							break;
						case P2:
							myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1]++;
							if(myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1]
								>= 100) {
								myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
								myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2]++;
							}
							if(myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] == 1) {
								myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P101;
							}
							break;
						case P3:
							data_index = (int)myPs->signal
								[M_SIG_EOL_PROCEDURE_DATA_INDEX];
							myPs->eol_data[data_index].index
								= (unsigned char)((int)myPs->signal
								[M_SIG_EOL_PROCEDURE_INDEX]
								+ (int)myPs->signal
								[M_SIG_EOL_PROCEDURE_DIVISION] * 100);
							myPs->eol_data[data_index].sub_index
								= (unsigned char)myPs->
								signal[M_SIG_EOL_PROCEDURE_SUB_INDEX];
							myPs->eol_data[data_index].response = 0; //ok

							memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
							SendMsg.msg = MSG_MODULE_COA_EOL_PROCEDURE_REPLY;
							SendMsg.val[0] = group;
							SendMsg.val[1] = (int)myPs->signal
								[M_SIG_EOL_PROCEDURE_DATA_INDEX];
							send_msg(MODULE_TO_COA1+group, (char *)&SendMsg);
							myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]++;
							if(myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
								>= MAX_EOL_DATA) {
								myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
									= 0;
							}
							myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P0;
							break;
						default:
							data_index = (int)myPs->signal
								[M_SIG_EOL_PROCEDURE_DATA_INDEX];
							myPs->eol_data[data_index].index
								= (unsigned char)((int)myPs->signal
								[M_SIG_EOL_PROCEDURE_INDEX]
								+ (int)myPs->signal
								[M_SIG_EOL_PROCEDURE_DIVISION] * 100);
							myPs->eol_data[data_index].sub_index
								= (unsigned char)myPs->
								signal[M_SIG_EOL_PROCEDURE_SUB_INDEX];
							myPs->eol_data[data_index].response
								= (short int)myPs->signal
								[M_SIG_EOL_PROCEDURE_PHASE] - 100; //fail

							memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
							SendMsg.msg = MSG_MODULE_COA_EOL_PROCEDURE_REPLY;
							SendMsg.val[0] = group;
							SendMsg.val[1] = (int)myPs->signal
								[M_SIG_EOL_PROCEDURE_DATA_INDEX];
							send_msg(MODULE_TO_COA1+group, (char *)&SendMsg);
							myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]++;
							if(myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
								>= MAX_EOL_DATA) {
								myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
									= 0;
							}
							myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P0;
							break;
					}
					break;
				case P1: //Charge+ Open(1447)
					switch(myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]) {
						case P0:
							break;
						case P1:
							data_index = (int)myPs->signal
								[M_SIG_EOL_PROCEDURE_DATA_INDEX];
							memset((char *)&myPs->eol_data[data_index].index,
								0, sizeof(S_EOL_DATA));

							memset((char *)&myData->CAN.uds_rx[can_ch1].uc_val[0],
								0, 8);

							myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
							myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] = 0;
							myData->CAN.signal[can_ch1][CAN_SIG_TX_PHASE_UDS_DATA1]
								= 47;
							myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]++;
							break;
						case P2:
							myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1]++;
							if(myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1]
								>= 100) {
								myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
								myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2]++;
							}
							if(myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] == 1) {
								myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P101;
							}
							break;
						case P3:
							data_index = (int)myPs->signal
								[M_SIG_EOL_PROCEDURE_DATA_INDEX];
							myPs->eol_data[data_index].index
								= (unsigned char)((int)myPs->signal
								[M_SIG_EOL_PROCEDURE_INDEX]
								+ (int)myPs->signal
								[M_SIG_EOL_PROCEDURE_DIVISION] * 100);
							myPs->eol_data[data_index].sub_index
								= (unsigned char)myPs->
								signal[M_SIG_EOL_PROCEDURE_SUB_INDEX];
							myPs->eol_data[data_index].response = 0; //ok

							memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
							SendMsg.msg = MSG_MODULE_COA_EOL_PROCEDURE_REPLY;
							SendMsg.val[0] = group;
							SendMsg.val[1] = (int)myPs->signal
								[M_SIG_EOL_PROCEDURE_DATA_INDEX];
							send_msg(MODULE_TO_COA1+group, (char *)&SendMsg);
							myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]++;
							if(myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
								>= MAX_EOL_DATA) {
								myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
									= 0;
							}
							myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P0;
							break;
						default:
							data_index = (int)myPs->signal
								[M_SIG_EOL_PROCEDURE_DATA_INDEX];
							myPs->eol_data[data_index].index
								= (unsigned char)((int)myPs->signal
								[M_SIG_EOL_PROCEDURE_INDEX]
								+ (int)myPs->signal
								[M_SIG_EOL_PROCEDURE_DIVISION] * 100);
							myPs->eol_data[data_index].sub_index
								= (unsigned char)myPs->
								signal[M_SIG_EOL_PROCEDURE_SUB_INDEX];
							myPs->eol_data[data_index].response
								= (short int)myPs->signal
								[M_SIG_EOL_PROCEDURE_PHASE] - 100; //fail

							memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
							SendMsg.msg = MSG_MODULE_COA_EOL_PROCEDURE_REPLY;
							SendMsg.val[0] = group;
							SendMsg.val[1] = (int)myPs->signal
								[M_SIG_EOL_PROCEDURE_DATA_INDEX];
							send_msg(MODULE_TO_COA1+group, (char *)&SendMsg);
							myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]++;
							if(myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
								>= MAX_EOL_DATA) {
								myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
									= 0;
							}
							myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P0;
							break;
					}
					break;
				default:
					break;
			}
			break;
		case P13:
			switch(myPs->signal[M_SIG_EOL_PROCEDURE_SUB_INDEX]) {
				case P0: //Main- Close(1448)
					switch(myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]) {
						case P0:
							break;
						case P1:
							data_index = (int)myPs->signal
								[M_SIG_EOL_PROCEDURE_DATA_INDEX];
							memset((char *)&myPs->eol_data[data_index].index,
								0, sizeof(S_EOL_DATA));

							memset((char *)&myData->CAN.uds_rx[can_ch1].uc_val[0],
								0, 8);

							myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
							myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] = 0;
							myData->CAN.signal[can_ch1][CAN_SIG_TX_PHASE_UDS_DATA1]
								= 48;
							myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]++;
							break;
						case P2:
							myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1]++;
							if(myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1]
								>= 100) {
								myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
								myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2]++;
							}
							if(myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] == 1) {
								myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P101;
							}
							break;
						case P3:
							data_index = (int)myPs->signal
								[M_SIG_EOL_PROCEDURE_DATA_INDEX];
							myPs->eol_data[data_index].index
								= (unsigned char)((int)myPs->signal
								[M_SIG_EOL_PROCEDURE_INDEX]
								+ (int)myPs->signal
								[M_SIG_EOL_PROCEDURE_DIVISION] * 100);
							myPs->eol_data[data_index].sub_index
								= (unsigned char)myPs->
								signal[M_SIG_EOL_PROCEDURE_SUB_INDEX];
							myPs->eol_data[data_index].response = 0; //ok

							memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
							SendMsg.msg = MSG_MODULE_COA_EOL_PROCEDURE_REPLY;
							SendMsg.val[0] = group;
							SendMsg.val[1] = (int)myPs->signal
								[M_SIG_EOL_PROCEDURE_DATA_INDEX];
							send_msg(MODULE_TO_COA1+group, (char *)&SendMsg);
							myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]++;
							if(myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
								>= MAX_EOL_DATA) {
								myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
									= 0;
							}
							myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P0;
							break;
						default:
							data_index = (int)myPs->signal
								[M_SIG_EOL_PROCEDURE_DATA_INDEX];
							myPs->eol_data[data_index].index
								= (unsigned char)((int)myPs->signal
								[M_SIG_EOL_PROCEDURE_INDEX]
								+ (int)myPs->signal
								[M_SIG_EOL_PROCEDURE_DIVISION] * 100);
							myPs->eol_data[data_index].sub_index
								= (unsigned char)myPs->
								signal[M_SIG_EOL_PROCEDURE_SUB_INDEX];
							myPs->eol_data[data_index].response
								= (short int)myPs->signal
								[M_SIG_EOL_PROCEDURE_PHASE] - 100; //fail

							memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
							SendMsg.msg = MSG_MODULE_COA_EOL_PROCEDURE_REPLY;
							SendMsg.val[0] = group;
							SendMsg.val[1] = (int)myPs->signal
								[M_SIG_EOL_PROCEDURE_DATA_INDEX];
							send_msg(MODULE_TO_COA1+group, (char *)&SendMsg);
							myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]++;
							if(myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
								>= MAX_EOL_DATA) {
								myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
									= 0;
							}
							myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P0;
							break;
					}
					break;
				case P1: //Main- Open(1449)
					switch(myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]) {
						case P0:
							break;
						case P1:
							data_index = (int)myPs->signal
								[M_SIG_EOL_PROCEDURE_DATA_INDEX];
							memset((char *)&myPs->eol_data[data_index].index,
								0, sizeof(S_EOL_DATA));

							memset((char *)&myData->CAN.uds_rx[can_ch1].uc_val[0],
								0, 8);

							myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
							myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] = 0;
							myData->CAN.signal[can_ch1][CAN_SIG_TX_PHASE_UDS_DATA1]
								= 49;
							myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]++;
							break;
						case P2:
							myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1]++;
							if(myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1]
								>= 100) {
								myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
								myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2]++;
							}
							if(myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] == 1) {
								myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P101;
							}
							break;
						case P3:
							data_index = (int)myPs->signal
								[M_SIG_EOL_PROCEDURE_DATA_INDEX];
							myPs->eol_data[data_index].index
								= (unsigned char)((int)myPs->signal
								[M_SIG_EOL_PROCEDURE_INDEX]
								+ (int)myPs->signal
								[M_SIG_EOL_PROCEDURE_DIVISION] * 100);
							myPs->eol_data[data_index].sub_index
								= (unsigned char)myPs->
								signal[M_SIG_EOL_PROCEDURE_SUB_INDEX];
							myPs->eol_data[data_index].response = 0; //ok

							memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
							SendMsg.msg = MSG_MODULE_COA_EOL_PROCEDURE_REPLY;
							SendMsg.val[0] = group;
							SendMsg.val[1] = (int)myPs->signal
								[M_SIG_EOL_PROCEDURE_DATA_INDEX];
							send_msg(MODULE_TO_COA1+group, (char *)&SendMsg);
							myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]++;
							if(myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
								>= MAX_EOL_DATA) {
								myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
									= 0;
							}
							myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P0;
							break;
						default:
							data_index = (int)myPs->signal
								[M_SIG_EOL_PROCEDURE_DATA_INDEX];
							myPs->eol_data[data_index].index
								= (unsigned char)((int)myPs->signal
								[M_SIG_EOL_PROCEDURE_INDEX]
								+ (int)myPs->signal
								[M_SIG_EOL_PROCEDURE_DIVISION] * 100);
							myPs->eol_data[data_index].sub_index
								= (unsigned char)myPs->
								signal[M_SIG_EOL_PROCEDURE_SUB_INDEX];
							myPs->eol_data[data_index].response
								= (short int)myPs->signal
								[M_SIG_EOL_PROCEDURE_PHASE] - 100; //fail

							memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
							SendMsg.msg = MSG_MODULE_COA_EOL_PROCEDURE_REPLY;
							SendMsg.val[0] = group;
							SendMsg.val[1] = (int)myPs->signal
								[M_SIG_EOL_PROCEDURE_DATA_INDEX];
							send_msg(MODULE_TO_COA1+group, (char *)&SendMsg);
							myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]++;
							if(myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
								>= MAX_EOL_DATA) {
								myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
									= 0;
							}
							myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P0;
							break;
					}
					break;
				default:
					break;
			}
			break;
		case P14:
			switch(myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]) {
				case P0:
					break;
				case P1: //Max_Cell_Temp Check(1551)
					data_index
						= (int)myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					memset((char *)&myPs->eol_data[data_index].index, 0,
						sizeof(S_EOL_DATA));

					memset((char *)&myData->CAN.uds_rx[can_ch1].uc_val[0], 0, 8);

					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] = 0;
					myData->CAN.signal[can_ch1][CAN_SIG_TX_PHASE_EOL_DATA2] = 1;
					myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]++;
					break;
				case P2:
					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1]++;
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] >= 100) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2]++;
					}
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] == 1) {
						myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P101;
					}
					break;
				case P3: //Min_Cell_Temp Check(1552)
					memset((char *)&myData->CAN.uds_rx[can_ch1].uc_val[0], 0, 8);

					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] = 0;
					myData->CAN.signal[can_ch1][CAN_SIG_TX_PHASE_EOL_DATA2] = 2;
					myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]++;
					break;
				case P4:
					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1]++;
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] >= 100) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2]++;
					}
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] == 1) {
						myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P102;
					}
					break;
				case P5:
					data_index
						= (int)myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					myPs->eol_data[data_index].index
						= (unsigned char)((int)myPs->signal
						[M_SIG_EOL_PROCEDURE_INDEX]
						+ (int)myPs->signal[M_SIG_EOL_PROCEDURE_DIVISION]
						* 100);
					myPs->eol_data[data_index].sub_index = (unsigned char)myPs->
						signal[M_SIG_EOL_PROCEDURE_SUB_INDEX];
					myPs->eol_data[data_index].response = 0; //ok

					memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
					SendMsg.msg = MSG_MODULE_COA_EOL_PROCEDURE_REPLY;
					SendMsg.val[0] = group;
					SendMsg.val[1]
						= (int)myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					send_msg(MODULE_TO_COA1+group, (char *)&SendMsg);
					myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]++;
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
						>= MAX_EOL_DATA) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX] = 0;
					}
					myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P0;
					break;
				default:
					data_index
						= (int)myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					myPs->eol_data[data_index].index
						= (unsigned char)((int)myPs->signal
						[M_SIG_EOL_PROCEDURE_INDEX]
						+ (int)myPs->signal[M_SIG_EOL_PROCEDURE_DIVISION]
						* 100);
					myPs->eol_data[data_index].sub_index = (unsigned char)myPs->
						signal[M_SIG_EOL_PROCEDURE_SUB_INDEX];
					myPs->eol_data[data_index].response
						= (short int)myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]
						- 100; //fail

					memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
					SendMsg.msg = MSG_MODULE_COA_EOL_PROCEDURE_REPLY;
					SendMsg.val[0] = group;
					SendMsg.val[1]
						= (int)myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					send_msg(MODULE_TO_COA1+group, (char *)&SendMsg);
					myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]++;
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
						>= MAX_EOL_DATA) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX] = 0;
					}
					myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P0;
					break;
			}
			break;
		case P15: //Coolant Level Open(1553)
			switch(myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]) {
				case P0:
					break;
				case P1:
					data_index = (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					memset((char *)&myPs->eol_data[data_index].index,
						0, sizeof(S_EOL_DATA));

					memset((char *)&myData->CAN.uds_rx[can_ch1].uc_val[0], 0, 8);

					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] = 0;
					myData->CAN.signal[can_ch1][CAN_SIG_TX_PHASE_EOL_DATA2] = 3;
					myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]++;
					break;
				case P2:
					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1]++;
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] >= 100) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2]++;
					}
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] == 2) {
						myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P101;
					}
					break;
				case P3:
					data_index = (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					myPs->eol_data[data_index].index
						= (unsigned char)((int)myPs->signal
						[M_SIG_EOL_PROCEDURE_INDEX]
						+ (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DIVISION] * 100);
					myPs->eol_data[data_index].sub_index
						= (unsigned char)myPs->
						signal[M_SIG_EOL_PROCEDURE_SUB_INDEX];
					myPs->eol_data[data_index].response = 0; //ok

					memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
					SendMsg.msg = MSG_MODULE_COA_EOL_PROCEDURE_REPLY;
					SendMsg.val[0] = group;
					SendMsg.val[1] = (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					send_msg(MODULE_TO_COA1+group, (char *)&SendMsg);
					myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]++;
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
						>= MAX_EOL_DATA) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX] = 0;
					}
					myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P0;
					break;
				default:
					data_index = (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					myPs->eol_data[data_index].index
						= (unsigned char)((int)myPs->signal
						[M_SIG_EOL_PROCEDURE_INDEX]
						+ (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DIVISION] * 100);
					myPs->eol_data[data_index].sub_index
						= (unsigned char)myPs->
						signal[M_SIG_EOL_PROCEDURE_SUB_INDEX];
					myPs->eol_data[data_index].response
						= (short int)myPs->signal
						[M_SIG_EOL_PROCEDURE_PHASE] - 100; //fail

					memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
					SendMsg.msg = MSG_MODULE_COA_EOL_PROCEDURE_REPLY;
					SendMsg.val[0] = group;
					SendMsg.val[1] = (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					send_msg(MODULE_TO_COA1+group, (char *)&SendMsg);
					myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]++;
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
						>= MAX_EOL_DATA) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX] = 0;
					}
					myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P0;
					break;
			}
			break;
		case P16: //CIRCULATION PUMP Check(1554)
			switch(myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]) {
				case P0:
					break;
				case P1:
					data_index = (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					memset((char *)&myPs->eol_data[data_index].index,
						0, sizeof(S_EOL_DATA));

					memset((char *)&myData->CAN.uds_rx[can_ch1].uc_val[0], 0, 8);

					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] = 0;
					myData->CAN.signal[can_ch1][CAN_SIG_TX_PHASE_EOL_DATA2] = 4;
					myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]++;
					break;
				case P2:
					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1]++;
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] >= 100) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2]++;
					}
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] == 1) {
						myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P101;
					}
					break;
				case P3:
					data_index = (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					myPs->eol_data[data_index].index
						= (unsigned char)((int)myPs->signal
						[M_SIG_EOL_PROCEDURE_INDEX]
						+ (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DIVISION] * 100);
					myPs->eol_data[data_index].sub_index
						= (unsigned char)myPs->
						signal[M_SIG_EOL_PROCEDURE_SUB_INDEX];
					myPs->eol_data[data_index].response = 0; //ok

					memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
					SendMsg.msg = MSG_MODULE_COA_EOL_PROCEDURE_REPLY;
					SendMsg.val[0] = group;
					SendMsg.val[1] = (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					send_msg(MODULE_TO_COA1+group, (char *)&SendMsg);
					myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]++;
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
						>= MAX_EOL_DATA) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX] = 0;
					}
					myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P0;
					break;
				default:
					data_index = (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					myPs->eol_data[data_index].index
						= (unsigned char)((int)myPs->signal
						[M_SIG_EOL_PROCEDURE_INDEX]
						+ (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DIVISION] * 100);
					myPs->eol_data[data_index].sub_index
						= (unsigned char)myPs->
						signal[M_SIG_EOL_PROCEDURE_SUB_INDEX];
					myPs->eol_data[data_index].response
						= (short int)myPs->signal
						[M_SIG_EOL_PROCEDURE_PHASE] - 100; //fail

					memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
					SendMsg.msg = MSG_MODULE_COA_EOL_PROCEDURE_REPLY;
					SendMsg.val[0] = group;
					SendMsg.val[1] = (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					send_msg(MODULE_TO_COA1+group, (char *)&SendMsg);
					myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]++;
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
						>= MAX_EOL_DATA) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX] = 0;
					}
					myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P0;
					break;
			}
			break;
		case P17: //COOLING PUMP Check(1555)
			switch(myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]) {
				case P0:
					break;
				case P1:
					data_index = (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					memset((char *)&myPs->eol_data[data_index].index,
						0, sizeof(S_EOL_DATA));

					memset((char *)&myData->CAN.uds_rx[can_ch1].uc_val[0], 0, 8);

					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] = 0;
					myData->CAN.signal[can_ch1][CAN_SIG_TX_PHASE_EOL_DATA2] = 5;
					myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]++;
					break;
				case P2:
					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1]++;
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] >= 100) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2]++;
					}
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] == 1) {
						myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P101;
					}
					break;
				case P3:
					data_index = (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					myPs->eol_data[data_index].index
						= (unsigned char)((int)myPs->signal
						[M_SIG_EOL_PROCEDURE_INDEX]
						+ (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DIVISION] * 100);
					myPs->eol_data[data_index].sub_index
						= (unsigned char)myPs->
						signal[M_SIG_EOL_PROCEDURE_SUB_INDEX];
					myPs->eol_data[data_index].response = 0; //ok

					memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
					SendMsg.msg = MSG_MODULE_COA_EOL_PROCEDURE_REPLY;
					SendMsg.val[0] = group;
					SendMsg.val[1] = (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					send_msg(MODULE_TO_COA1+group, (char *)&SendMsg);
					myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]++;
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
						>= MAX_EOL_DATA) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX] = 0;
					}
					myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P0;
					break;
				default:
					data_index = (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					myPs->eol_data[data_index].index
						= (unsigned char)((int)myPs->signal
						[M_SIG_EOL_PROCEDURE_INDEX]
						+ (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DIVISION] * 100);
					myPs->eol_data[data_index].sub_index
						= (unsigned char)myPs->
						signal[M_SIG_EOL_PROCEDURE_SUB_INDEX];
					myPs->eol_data[data_index].response
						= (short int)myPs->signal
						[M_SIG_EOL_PROCEDURE_PHASE] - 100; //fail

					memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
					SendMsg.msg = MSG_MODULE_COA_EOL_PROCEDURE_REPLY;
					SendMsg.val[0] = group;
					SendMsg.val[1] = (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					send_msg(MODULE_TO_COA1+group, (char *)&SendMsg);
					myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]++;
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
						>= MAX_EOL_DATA) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX] = 0;
					}
					myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P0;
					break;
			}
			break;
		case P18: //3Way Valve Check(1556)
			switch(myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]) {
				case P0:
					break;
				case P1:
					data_index = (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					memset((char *)&myPs->eol_data[data_index].index,
						0, sizeof(S_EOL_DATA));

					memset((char *)&myData->CAN.uds_rx[can_ch1].uc_val[0], 0, 8);

					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] = 0;
					myData->CAN.signal[can_ch1][CAN_SIG_TX_PHASE_EOL_DATA2] = 6;
					myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]++;
					break;
				case P2:
					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1]++;
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] >= 100) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2]++;
					}
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] == 1) {
						myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P101;
					}
					break;
				case P3:
					data_index = (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					myPs->eol_data[data_index].index
						= (unsigned char)((int)myPs->signal
						[M_SIG_EOL_PROCEDURE_INDEX]
						+ (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DIVISION] * 100);
					myPs->eol_data[data_index].sub_index
						= (unsigned char)myPs->
						signal[M_SIG_EOL_PROCEDURE_SUB_INDEX];
					myPs->eol_data[data_index].response = 0; //ok

					memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
					SendMsg.msg = MSG_MODULE_COA_EOL_PROCEDURE_REPLY;
					SendMsg.val[0] = group;
					SendMsg.val[1] = (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					send_msg(MODULE_TO_COA1+group, (char *)&SendMsg);
					myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]++;
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
						>= MAX_EOL_DATA) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX] = 0;
					}
					myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P0;
					break;
				default:
					data_index = (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					myPs->eol_data[data_index].index
						= (unsigned char)((int)myPs->signal
						[M_SIG_EOL_PROCEDURE_INDEX]
						+ (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DIVISION] * 100);
					myPs->eol_data[data_index].sub_index
						= (unsigned char)myPs->
						signal[M_SIG_EOL_PROCEDURE_SUB_INDEX];
					myPs->eol_data[data_index].response
						= (short int)myPs->signal
						[M_SIG_EOL_PROCEDURE_PHASE] - 100; //fail

					memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
					SendMsg.msg = MSG_MODULE_COA_EOL_PROCEDURE_REPLY;
					SendMsg.val[0] = group;
					SendMsg.val[1] = (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					send_msg(MODULE_TO_COA1+group, (char *)&SendMsg);
					myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]++;
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
						>= MAX_EOL_DATA) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX] = 0;
					}
					myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P0;
					break;
			}
			break;
		case P19: //DTC Check(1404)
			switch(myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]) {
				case P0:
					break;
				case P1:
					data_index = (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					memset((char *)&myPs->eol_data[data_index].index,
						0, sizeof(S_EOL_DATA));

					memset((char *)&myData->CAN.uds_rx[can_ch1].uc_val[0], 0, 8);

					myData->CAN.uds_rx_message_count = 0;
					myData->CAN.uds_rx_dtc_count = 0;

					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] = 0;
					myData->CAN.signal[can_ch1][CAN_SIG_TX_PHASE_UDS_DATA1] = 4;
					myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]++;
					break;
				case P2:
					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1]++;
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] >= 100) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2]++;
					}
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] == 1) {
						myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P101;
					}
					break;
				case P3:
					data_index = (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					myPs->eol_data[data_index].index
						= (unsigned char)((int)myPs->signal
						[M_SIG_EOL_PROCEDURE_INDEX]
						+ (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DIVISION] * 100);
					myPs->eol_data[data_index].sub_index
						= (unsigned char)myPs->
						signal[M_SIG_EOL_PROCEDURE_SUB_INDEX];
					myPs->eol_data[data_index].response = 0; //ok

					memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
					SendMsg.msg = MSG_MODULE_COA_EOL_PROCEDURE_REPLY;
					SendMsg.val[0] = group;
					SendMsg.val[1] = (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					send_msg(MODULE_TO_COA1+group, (char *)&SendMsg);
					myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]++;
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
						>= MAX_EOL_DATA) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX] = 0;
					}
					myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P0;
					break;
				default:
					data_index = (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					myPs->eol_data[data_index].index
						= (unsigned char)((int)myPs->signal
						[M_SIG_EOL_PROCEDURE_INDEX]
						+ (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DIVISION] * 100);
					myPs->eol_data[data_index].sub_index
						= (unsigned char)myPs->
						signal[M_SIG_EOL_PROCEDURE_SUB_INDEX];
					myPs->eol_data[data_index].response
						= (short int)myPs->signal
						[M_SIG_EOL_PROCEDURE_PHASE] - 100; //fail

					memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
					SendMsg.msg = MSG_MODULE_COA_EOL_PROCEDURE_REPLY;
					SendMsg.val[0] = group;
					SendMsg.val[1] = (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					send_msg(MODULE_TO_COA1+group, (char *)&SendMsg);
					myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]++;
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
						>= MAX_EOL_DATA) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX] = 0;
					}
					myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P0;
					break;
			}
			break;
		case P20: //DTC List(1419)
			switch(myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]) {
				case P0:
					break;
				case P1:
					data_index = (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					memset((char *)&myPs->eol_data[data_index].index,
						0, sizeof(S_EOL_DATA));

					memset((char *)&myData->CAN.uds_rx[can_ch1].uc_val[0], 0, 8);

					myData->CAN.uds_rx_message_count = 0;
					myData->CAN.uds_rx_dtc_count = 0;

					memset((char *)&myPs->misc.dtc_data, 0, 2048);
					myPs->misc.dtc_data_index = 0;
					myData->CAN.signal[can_ch1][CAN_SIG_TX_PHASE_UDS_DATA_INDEX] = 0;

					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] = 0;
					myData->CAN.signal[can_ch1][CAN_SIG_TX_PHASE_UDS_DATA1] = 19;
					myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]++;
					break;
				case P2:
					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1]++;
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] >= 100) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2]++;
					}
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] == 10) {
						myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P101;
					}
					break;
				case P3:
					data_index = (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					myPs->eol_data[data_index].index
						= (unsigned char)((int)myPs->signal
						[M_SIG_EOL_PROCEDURE_INDEX]
						+ (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DIVISION] * 100);
					myPs->eol_data[data_index].sub_index
						= (unsigned char)myPs->
						signal[M_SIG_EOL_PROCEDURE_SUB_INDEX];
					myPs->eol_data[data_index].response = 0; //ok

					memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
					SendMsg.msg = MSG_MODULE_COA_EOL_PROCEDURE_REPLY;
					SendMsg.val[0] = group;
					SendMsg.val[1] = (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					send_msg(MODULE_TO_COA1+group, (char *)&SendMsg);
					myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]++;
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
						>= MAX_EOL_DATA) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX] = 0;
					}
					myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P0;
					break;
				default:
					data_index = (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					myPs->eol_data[data_index].index
						= (unsigned char)((int)myPs->signal
						[M_SIG_EOL_PROCEDURE_INDEX]
						+ (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DIVISION] * 100);
					myPs->eol_data[data_index].sub_index
						= (unsigned char)myPs->
						signal[M_SIG_EOL_PROCEDURE_SUB_INDEX];
					myPs->eol_data[data_index].response
						= (short int)myPs->signal
						[M_SIG_EOL_PROCEDURE_PHASE] - 100; //fail

					memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
					SendMsg.msg = MSG_MODULE_COA_EOL_PROCEDURE_REPLY;
					SendMsg.val[0] = group;
					SendMsg.val[1] = (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					send_msg(MODULE_TO_COA1+group, (char *)&SendMsg);
					myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]++;
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
						>= MAX_EOL_DATA) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX] = 0;
					}
					myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P0;
					break;
			}
			break;
		case P21: //Cell delta voltage Check(1407)
			switch(myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]) {
				case P0:
					break;
				case P1:
					data_index = (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					memset((char *)&myPs->eol_data[data_index].index,
						0, sizeof(S_EOL_DATA));

					memset((char *)&myData->CAN.uds_rx[can_ch1].uc_val[0], 0, 8);

					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] = 0;
					myData->CAN.signal[can_ch1][CAN_SIG_TX_PHASE_UDS_DATA1] = 7;
					myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]++;
					break;
				case P2:
					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1]++;
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] >= 100) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2]++;
					}
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] == 1) {
						myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P101;
					}
					break;
				case P3:
					data_index = (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					myPs->eol_data[data_index].index
						= (unsigned char)((int)myPs->signal
						[M_SIG_EOL_PROCEDURE_INDEX]
						+ (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DIVISION] * 100);
					myPs->eol_data[data_index].sub_index
						= (unsigned char)myPs->
						signal[M_SIG_EOL_PROCEDURE_SUB_INDEX];
					myPs->eol_data[data_index].response = 0; //ok

					memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
					SendMsg.msg = MSG_MODULE_COA_EOL_PROCEDURE_REPLY;
					SendMsg.val[0] = group;
					SendMsg.val[1] = (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					send_msg(MODULE_TO_COA1+group, (char *)&SendMsg);
					myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]++;
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
						>= MAX_EOL_DATA) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX] = 0;
					}
					myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P0;
					break;
				default:
					data_index = (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					myPs->eol_data[data_index].index
						= (unsigned char)((int)myPs->signal
						[M_SIG_EOL_PROCEDURE_INDEX]
						+ (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DIVISION] * 100);
					myPs->eol_data[data_index].sub_index
						= (unsigned char)myPs->
						signal[M_SIG_EOL_PROCEDURE_SUB_INDEX];
					myPs->eol_data[data_index].response
						= (short int)myPs->signal
						[M_SIG_EOL_PROCEDURE_PHASE] - 100; //fail

					memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
					SendMsg.msg = MSG_MODULE_COA_EOL_PROCEDURE_REPLY;
					SendMsg.val[0] = group;
					SendMsg.val[1] = (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					send_msg(MODULE_TO_COA1+group, (char *)&SendMsg);
					myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]++;
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
						>= MAX_EOL_DATA) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX] = 0;
					}
					myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P0;
					break;
			}
			break;
		case P22: //Module delta temp Check(1405)
			switch(myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]) {
				case P0:
					break;
				case P1:
					data_index = (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					memset((char *)&myPs->eol_data[data_index].index,
						0, sizeof(S_EOL_DATA));

					memset((char *)&myData->CAN.uds_rx[can_ch1].uc_val[0], 0, 8);

					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] = 0;
					myData->CAN.signal[can_ch1][CAN_SIG_TX_PHASE_UDS_DATA1] = 5;
					myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]++;
					break;
				case P2:
					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1]++;
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] >= 100) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2]++;
					}
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] == 1) {
						myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P101;
					}
					break;
				case P3:
					data_index = (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					myPs->eol_data[data_index].index
						= (unsigned char)((int)myPs->signal
						[M_SIG_EOL_PROCEDURE_INDEX]
						+ (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DIVISION] * 100);
					myPs->eol_data[data_index].sub_index
						= (unsigned char)myPs->
						signal[M_SIG_EOL_PROCEDURE_SUB_INDEX];
					myPs->eol_data[data_index].response = 0; //ok

					memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
					SendMsg.msg = MSG_MODULE_COA_EOL_PROCEDURE_REPLY;
					SendMsg.val[0] = group;
					SendMsg.val[1] = (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					send_msg(MODULE_TO_COA1+group, (char *)&SendMsg);
					myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]++;
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
						>= MAX_EOL_DATA) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX] = 0;
					}
					myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P0;
					break;
				default:
					data_index = (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					myPs->eol_data[data_index].index
						= (unsigned char)((int)myPs->signal
						[M_SIG_EOL_PROCEDURE_INDEX]
						+ (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DIVISION] * 100);
					myPs->eol_data[data_index].sub_index
						= (unsigned char)myPs->
						signal[M_SIG_EOL_PROCEDURE_SUB_INDEX];
					myPs->eol_data[data_index].response
						= (short int)myPs->signal
						[M_SIG_EOL_PROCEDURE_PHASE] - 100; //fail

					memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
					SendMsg.msg = MSG_MODULE_COA_EOL_PROCEDURE_REPLY;
					SendMsg.val[0] = group;
					SendMsg.val[1] = (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					send_msg(MODULE_TO_COA1+group, (char *)&SendMsg);
					myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]++;
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
						>= MAX_EOL_DATA) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX] = 0;
					}
					myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P0;
					break;
			}
			break;
		case P23: //Inlet temp Check(1406)
			switch(myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]) {
				case P0:
					break;
				case P1:
					data_index = (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					memset((char *)&myPs->eol_data[data_index].index,
						0, sizeof(S_EOL_DATA));

					memset((char *)&myData->CAN.uds_rx[can_ch1].uc_val[0], 0, 8);

					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] = 0;
					myData->CAN.signal[can_ch1][CAN_SIG_TX_PHASE_UDS_DATA1] = 6;
					myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]++;
					break;
				case P2:
					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1]++;
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] >= 100) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2]++;
					}
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] == 1) {
						myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P101;
					}
					break;
				case P3:
					data_index = (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					myPs->eol_data[data_index].index
						= (unsigned char)((int)myPs->signal
						[M_SIG_EOL_PROCEDURE_INDEX]
						+ (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DIVISION] * 100);
					myPs->eol_data[data_index].sub_index
						= (unsigned char)myPs->
						signal[M_SIG_EOL_PROCEDURE_SUB_INDEX];
					myPs->eol_data[data_index].response = 0; //ok

					memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
					SendMsg.msg = MSG_MODULE_COA_EOL_PROCEDURE_REPLY;
					SendMsg.val[0] = group;
					SendMsg.val[1] = (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					send_msg(MODULE_TO_COA1+group, (char *)&SendMsg);
					myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]++;
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
						>= MAX_EOL_DATA) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX] = 0;
					}
					myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P0;
					break;
				default:
					data_index = (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					myPs->eol_data[data_index].index
						= (unsigned char)((int)myPs->signal
						[M_SIG_EOL_PROCEDURE_INDEX]
						+ (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DIVISION] * 100);
					myPs->eol_data[data_index].sub_index
						= (unsigned char)myPs->
						signal[M_SIG_EOL_PROCEDURE_SUB_INDEX];
					myPs->eol_data[data_index].response
						= (short int)myPs->signal
						[M_SIG_EOL_PROCEDURE_PHASE] - 100; //fail

					memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
					SendMsg.msg = MSG_MODULE_COA_EOL_PROCEDURE_REPLY;
					SendMsg.val[0] = group;
					SendMsg.val[1] = (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					send_msg(MODULE_TO_COA1+group, (char *)&SendMsg);
					myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]++;
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
						>= MAX_EOL_DATA) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX] = 0;
					}
					myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P0;
					break;
			}
			break;
		case P24:
			switch(myPs->signal[M_SIG_EOL_PROCEDURE_SUB_INDEX]) {
				case P0: //Main contactor close(1408)
					switch(myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]) {
						case P0:
							break;
						case P1:
							data_index = (int)myPs->signal
								[M_SIG_EOL_PROCEDURE_DATA_INDEX];
							memset((char *)&myPs->eol_data[data_index].index,
								0, sizeof(S_EOL_DATA));

							memset((char *)&myData->CAN.uds_rx[can_ch1].uc_val[0],
								0, 8);

							myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
							myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] = 0;
							myData->CAN.signal[can_ch2][CAN_SIG_TX_PHASE_UDS_DATA1]
								= 8;
							myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]++;
							break;
						case P2:
							myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1]++;
							if(myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1]
								>= 100) {
								myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
								myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2]++;
							}
							if(myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] == 1) {
								//myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]
								//	= P101;
								myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]++;//kjg_w
							}
							break;
						case P3:
							data_index = (int)myPs->signal
								[M_SIG_EOL_PROCEDURE_DATA_INDEX];
							myPs->eol_data[data_index].index
								= (unsigned char)((int)myPs->signal
								[M_SIG_EOL_PROCEDURE_INDEX]
								+ (int)myPs->signal
								[M_SIG_EOL_PROCEDURE_DIVISION] * 100);
							myPs->eol_data[data_index].sub_index
								= (unsigned char)myPs->
								signal[M_SIG_EOL_PROCEDURE_SUB_INDEX];
							myPs->eol_data[data_index].response = 0; //ok

							memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
							SendMsg.msg = MSG_MODULE_COA_EOL_PROCEDURE_REPLY;
							SendMsg.val[0] = group;
							SendMsg.val[1] = (int)myPs->signal
								[M_SIG_EOL_PROCEDURE_DATA_INDEX];
							send_msg(MODULE_TO_COA1+group, (char *)&SendMsg);
							myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]++;
							if(myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
								>= MAX_EOL_DATA) {
								myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
									= 0;
							}
							myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P0;
							break;
						default:
							data_index = (int)myPs->signal
								[M_SIG_EOL_PROCEDURE_DATA_INDEX];
							myPs->eol_data[data_index].index
								= (unsigned char)((int)myPs->signal
								[M_SIG_EOL_PROCEDURE_INDEX]
								+ (int)myPs->signal
								[M_SIG_EOL_PROCEDURE_DIVISION] * 100);
							myPs->eol_data[data_index].sub_index
								= (unsigned char)myPs->
								signal[M_SIG_EOL_PROCEDURE_SUB_INDEX];
							myPs->eol_data[data_index].response
								= (short int)myPs->signal
								[M_SIG_EOL_PROCEDURE_PHASE] - 100; //fail

							memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
							SendMsg.msg = MSG_MODULE_COA_EOL_PROCEDURE_REPLY;
							SendMsg.val[0] = group;
							SendMsg.val[1] = (int)myPs->signal
								[M_SIG_EOL_PROCEDURE_DATA_INDEX];
							send_msg(MODULE_TO_COA1+group, (char *)&SendMsg);
							myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]++;
							if(myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
								>= MAX_EOL_DATA) {
								myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
									= 0;
							}
							myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P0;
							break;
					}
					break;
				case P1: //Main contactor open(1409)
					switch(myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]) {
						case P0:
							break;
						case P1:
							data_index = (int)myPs->signal
								[M_SIG_EOL_PROCEDURE_DATA_INDEX];
							memset((char *)&myPs->eol_data[data_index].index,
								0, sizeof(S_EOL_DATA));

							memset((char *)&myData->CAN.uds_rx[can_ch1].uc_val[0],
								0, 8);

							myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
							myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] = 0;
							myData->CAN.signal[can_ch2][CAN_SIG_TX_PHASE_UDS_DATA1]
								= 9;
							myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]++;
							break;
						case P2:
							myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1]++;
							if(myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1]
								>= 100) {
								myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
								myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2]++;
							}
							if(myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] == 1) {
								//myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]
								//	= P101;
								myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]++;//kjg_w
							}
							break;
						case P3:
							data_index = (int)myPs->signal
								[M_SIG_EOL_PROCEDURE_DATA_INDEX];
							myPs->eol_data[data_index].index
								= (unsigned char)((int)myPs->signal
								[M_SIG_EOL_PROCEDURE_INDEX]
								+ (int)myPs->signal
								[M_SIG_EOL_PROCEDURE_DIVISION] * 100);
							myPs->eol_data[data_index].sub_index
								= (unsigned char)myPs->
								signal[M_SIG_EOL_PROCEDURE_SUB_INDEX];
							myPs->eol_data[data_index].response = 0; //ok

							memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
							SendMsg.msg = MSG_MODULE_COA_EOL_PROCEDURE_REPLY;
							SendMsg.val[0] = group;
							SendMsg.val[1] = (int)myPs->signal
								[M_SIG_EOL_PROCEDURE_DATA_INDEX];
							send_msg(MODULE_TO_COA1+group, (char *)&SendMsg);
							myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]++;
							if(myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
								>= MAX_EOL_DATA) {
								myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
									= 0;
							}
							myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P0;
							break;
						default:
							data_index = (int)myPs->signal
								[M_SIG_EOL_PROCEDURE_DATA_INDEX];
							myPs->eol_data[data_index].index
								= (unsigned char)((int)myPs->signal
								[M_SIG_EOL_PROCEDURE_INDEX]
								+ (int)myPs->signal
								[M_SIG_EOL_PROCEDURE_DIVISION] * 100);
							myPs->eol_data[data_index].sub_index
								= (unsigned char)myPs->
								signal[M_SIG_EOL_PROCEDURE_SUB_INDEX];
							myPs->eol_data[data_index].response
								= (short int)myPs->signal
								[M_SIG_EOL_PROCEDURE_PHASE] - 100; //fail

							memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
							SendMsg.msg = MSG_MODULE_COA_EOL_PROCEDURE_REPLY;
							SendMsg.val[0] = group;
							SendMsg.val[1] = (int)myPs->signal
								[M_SIG_EOL_PROCEDURE_DATA_INDEX];
							send_msg(MODULE_TO_COA1+group, (char *)&SendMsg);
							myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]++;
							if(myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
								>= MAX_EOL_DATA) {
								myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
									= 0;
							}
							myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P0;
							break;
					}
					break;
				default:
					break;
			}
			break;
		case P25: //Pack voltage Check(1410)
			switch(myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]) {
				case P0:
					break;
				case P1:
					data_index = (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					memset((char *)&myPs->eol_data[data_index].index,
						0, sizeof(S_EOL_DATA));

					memset((char *)&myData->CAN.uds_rx[can_ch1].uc_val[0], 0, 8);

					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] = 0;
					myData->CAN.signal[can_ch1][CAN_SIG_TX_PHASE_UDS_DATA1] = 10;
					myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]++;
					break;
				case P2:
					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1]++;
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] >= 100) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2]++;
					}
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] == 1) {
						myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P101;
					}
					break;
				case P3:
					data_index = (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					myPs->eol_data[data_index].index
						= (unsigned char)((int)myPs->signal
						[M_SIG_EOL_PROCEDURE_INDEX]
						+ (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DIVISION] * 100);
					myPs->eol_data[data_index].sub_index
						= (unsigned char)myPs->
						signal[M_SIG_EOL_PROCEDURE_SUB_INDEX];
					myPs->eol_data[data_index].response = 0; //ok

					memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
					SendMsg.msg = MSG_MODULE_COA_EOL_PROCEDURE_REPLY;
					SendMsg.val[0] = group;
					SendMsg.val[1] = (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					send_msg(MODULE_TO_COA1+group, (char *)&SendMsg);
					myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]++;
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
						>= MAX_EOL_DATA) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX] = 0;
					}
					myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P0;
					break;
				default:
					data_index = (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					myPs->eol_data[data_index].index
						= (unsigned char)((int)myPs->signal
						[M_SIG_EOL_PROCEDURE_INDEX]
						+ (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DIVISION] * 100);
					myPs->eol_data[data_index].sub_index
						= (unsigned char)myPs->
						signal[M_SIG_EOL_PROCEDURE_SUB_INDEX];
					myPs->eol_data[data_index].response
						= (short int)myPs->signal
						[M_SIG_EOL_PROCEDURE_PHASE] - 100; //fail

					memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
					SendMsg.msg = MSG_MODULE_COA_EOL_PROCEDURE_REPLY;
					SendMsg.val[0] = group;
					SendMsg.val[1] = (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					send_msg(MODULE_TO_COA1+group, (char *)&SendMsg);
					myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]++;
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
						>= MAX_EOL_DATA) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX] = 0;
					}
					myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P0;
					break;
			}
			break;
		case P26: //Link voltage Check(1411)
			switch(myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]) {
				case P0:
					break;
				case P1:
					data_index = (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					memset((char *)&myPs->eol_data[data_index].index,
						0, sizeof(S_EOL_DATA));

					memset((char *)&myData->CAN.uds_rx[can_ch1].uc_val[0], 0, 8);

					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] = 0;
					myData->CAN.signal[can_ch1][CAN_SIG_TX_PHASE_UDS_DATA1] = 11;
					myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]++;
					break;
				case P2:
					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1]++;
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] >= 100) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2]++;
					}
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] == 1) {
						myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P101;
					}
					break;
				case P3:
					data_index = (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					myPs->eol_data[data_index].index
						= (unsigned char)((int)myPs->signal
						[M_SIG_EOL_PROCEDURE_INDEX]
						+ (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DIVISION] * 100);
					myPs->eol_data[data_index].sub_index
						= (unsigned char)myPs->
						signal[M_SIG_EOL_PROCEDURE_SUB_INDEX];
					myPs->eol_data[data_index].response = 0; //ok

					memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
					SendMsg.msg = MSG_MODULE_COA_EOL_PROCEDURE_REPLY;
					SendMsg.val[0] = group;
					SendMsg.val[1] = (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					send_msg(MODULE_TO_COA1+group, (char *)&SendMsg);
					myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]++;
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
						>= MAX_EOL_DATA) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX] = 0;
					}
					myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P0;
					break;
				default:
					data_index = (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					myPs->eol_data[data_index].index
						= (unsigned char)((int)myPs->signal
						[M_SIG_EOL_PROCEDURE_INDEX]
						+ (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DIVISION] * 100);
					myPs->eol_data[data_index].sub_index
						= (unsigned char)myPs->
						signal[M_SIG_EOL_PROCEDURE_SUB_INDEX];
					myPs->eol_data[data_index].response
						= (short int)myPs->signal
						[M_SIG_EOL_PROCEDURE_PHASE] - 100; //fail

					memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
					SendMsg.msg = MSG_MODULE_COA_EOL_PROCEDURE_REPLY;
					SendMsg.val[0] = group;
					SendMsg.val[1] = (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					send_msg(MODULE_TO_COA1+group, (char *)&SendMsg);
					myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]++;
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
						>= MAX_EOL_DATA) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX] = 0;
					}
					myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P0;
					break;
			}
			break;
		case P27:
			switch(myPs->signal[M_SIG_EOL_PROCEDURE_SUB_INDEX]) {
				case P0: //Aux contactor close(1427)
					switch(myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]) {
						case P0:
							break;
						case P1:
							data_index = (int)myPs->signal
								[M_SIG_EOL_PROCEDURE_DATA_INDEX];
							memset((char *)&myPs->eol_data[data_index].index,
								0, sizeof(S_EOL_DATA));

							memset((char *)&myData->CAN.uds_rx[can_ch1].uc_val[0],
								0, 8);

							myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
							myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] = 0;
							myData->CAN.signal[can_ch1][CAN_SIG_TX_PHASE_UDS_DATA1]
								= 27;
							myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]++;
							break;
						case P2:
							myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1]++;
							if(myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1]
								>= 100) {
								myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
								myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2]++;
							}
							if(myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] == 1) {
								//myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]
								//	= P101;
								myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]++;//kjg_w
							}
							break;
						case P3:
							data_index = (int)myPs->signal
								[M_SIG_EOL_PROCEDURE_DATA_INDEX];
							myPs->eol_data[data_index].index
								= (unsigned char)((int)myPs->signal
								[M_SIG_EOL_PROCEDURE_INDEX]
								+ (int)myPs->signal
								[M_SIG_EOL_PROCEDURE_DIVISION] * 100);
							myPs->eol_data[data_index].sub_index
								= (unsigned char)myPs->
								signal[M_SIG_EOL_PROCEDURE_SUB_INDEX];
							myPs->eol_data[data_index].response = 0; //ok

							memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
							SendMsg.msg = MSG_MODULE_COA_EOL_PROCEDURE_REPLY;
							SendMsg.val[0] = group;
							SendMsg.val[1] = (int)myPs->signal
								[M_SIG_EOL_PROCEDURE_DATA_INDEX];
							send_msg(MODULE_TO_COA1+group, (char *)&SendMsg);
							myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]++;
							if(myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
								>= MAX_EOL_DATA) {
								myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
									= 0;
							}
							myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P0;
							break;
						default:
							data_index = (int)myPs->signal
								[M_SIG_EOL_PROCEDURE_DATA_INDEX];
							myPs->eol_data[data_index].index
								= (unsigned char)((int)myPs->signal
								[M_SIG_EOL_PROCEDURE_INDEX]
								+ (int)myPs->signal
								[M_SIG_EOL_PROCEDURE_DIVISION] * 100);
							myPs->eol_data[data_index].sub_index
								= (unsigned char)myPs->
								signal[M_SIG_EOL_PROCEDURE_SUB_INDEX];
							myPs->eol_data[data_index].response
								= (short int)myPs->signal
								[M_SIG_EOL_PROCEDURE_PHASE] - 100; //fail

							memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
							SendMsg.msg = MSG_MODULE_COA_EOL_PROCEDURE_REPLY;
							SendMsg.val[0] = group;
							SendMsg.val[1] = (int)myPs->signal
								[M_SIG_EOL_PROCEDURE_DATA_INDEX];
							send_msg(MODULE_TO_COA1+group, (char *)&SendMsg);
							myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]++;
							if(myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
								>= MAX_EOL_DATA) {
								myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
									= 0;
							}
							myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P0;
							break;
					}
					break;
				case P1: //Aux contactor open(1428)
					switch(myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]) {
						case P0:
							break;
						case P1:
							data_index = (int)myPs->signal
								[M_SIG_EOL_PROCEDURE_DATA_INDEX];
							memset((char *)&myPs->eol_data[data_index].index,
								0, sizeof(S_EOL_DATA));

							memset((char *)&myData->CAN.uds_rx[can_ch1].uc_val[0],
								0, 8);

							myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
							myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] = 0;
							myData->CAN.signal[can_ch1][CAN_SIG_TX_PHASE_UDS_DATA1]
								= 28;
							myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]++;
							break;
						case P2:
							myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1]++;
							if(myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1]
								>= 100) {
								myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
								myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2]++;
							}
							if(myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] == 1) {
								//myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]
								//	= P101;
								myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]++;//kjg_w
							}
							break;
						case P3:
							data_index = (int)myPs->signal
								[M_SIG_EOL_PROCEDURE_DATA_INDEX];
							myPs->eol_data[data_index].index
								= (unsigned char)((int)myPs->signal
								[M_SIG_EOL_PROCEDURE_INDEX]
								+ (int)myPs->signal
								[M_SIG_EOL_PROCEDURE_DIVISION] * 100);
							myPs->eol_data[data_index].sub_index
								= (unsigned char)myPs->
								signal[M_SIG_EOL_PROCEDURE_SUB_INDEX];
							myPs->eol_data[data_index].response = 0; //ok

							memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
							SendMsg.msg = MSG_MODULE_COA_EOL_PROCEDURE_REPLY;
							SendMsg.val[0] = group;
							SendMsg.val[1] = (int)myPs->signal
								[M_SIG_EOL_PROCEDURE_DATA_INDEX];
							send_msg(MODULE_TO_COA1+group, (char *)&SendMsg);
							myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]++;
							if(myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
								>= MAX_EOL_DATA) {
								myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
									= 0;
							}
							myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P0;
							break;
						default:
							data_index = (int)myPs->signal
								[M_SIG_EOL_PROCEDURE_DATA_INDEX];
							myPs->eol_data[data_index].index
								= (unsigned char)((int)myPs->signal
								[M_SIG_EOL_PROCEDURE_INDEX]
								+ (int)myPs->signal
								[M_SIG_EOL_PROCEDURE_DIVISION] * 100);
							myPs->eol_data[data_index].sub_index
								= (unsigned char)myPs->
								signal[M_SIG_EOL_PROCEDURE_SUB_INDEX];
							myPs->eol_data[data_index].response
								= (short int)myPs->signal
								[M_SIG_EOL_PROCEDURE_PHASE] - 100; //fail

							memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
							SendMsg.msg = MSG_MODULE_COA_EOL_PROCEDURE_REPLY;
							SendMsg.val[0] = group;
							SendMsg.val[1] = (int)myPs->signal
								[M_SIG_EOL_PROCEDURE_DATA_INDEX];
							send_msg(MODULE_TO_COA1+group, (char *)&SendMsg);
							myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]++;
							if(myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
								>= MAX_EOL_DATA) {
								myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
									= 0;
							}
							myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P0;
							break;
					}
					break;
				default:
					break;
			}
			break;
		case P28: //Charger voltage Check(1412)
			switch(myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]) {
				case P0:
					break;
				case P1:
					data_index = (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					memset((char *)&myPs->eol_data[data_index].index,
						0, sizeof(S_EOL_DATA));

					memset((char *)&myData->CAN.uds_rx[can_ch1].uc_val[0], 0, 8);

					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] = 0;
					myData->CAN.signal[can_ch1][CAN_SIG_TX_PHASE_UDS_DATA1] = 12;
					myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]++;
					break;
				case P2:
					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1]++;
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] >= 100) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2]++;
					}
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] == 1) {
						myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P101;
					}
					break;
				case P3:
					data_index = (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					myPs->eol_data[data_index].index
						= (unsigned char)((int)myPs->signal
						[M_SIG_EOL_PROCEDURE_INDEX]
						+ (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DIVISION] * 100);
					myPs->eol_data[data_index].sub_index
						= (unsigned char)myPs->
						signal[M_SIG_EOL_PROCEDURE_SUB_INDEX];
					myPs->eol_data[data_index].response = 0; //ok

					memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
					SendMsg.msg = MSG_MODULE_COA_EOL_PROCEDURE_REPLY;
					SendMsg.val[0] = group;
					SendMsg.val[1] = (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					send_msg(MODULE_TO_COA1+group, (char *)&SendMsg);
					myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]++;
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
						>= MAX_EOL_DATA) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX] = 0;
					}
					myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P0;
					break;
				default:
					data_index = (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					myPs->eol_data[data_index].index
						= (unsigned char)((int)myPs->signal
						[M_SIG_EOL_PROCEDURE_INDEX]
						+ (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DIVISION] * 100);
					myPs->eol_data[data_index].sub_index
						= (unsigned char)myPs->
						signal[M_SIG_EOL_PROCEDURE_SUB_INDEX];
					myPs->eol_data[data_index].response
						= (short int)myPs->signal
						[M_SIG_EOL_PROCEDURE_PHASE] - 100; //fail

					memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
					SendMsg.msg = MSG_MODULE_COA_EOL_PROCEDURE_REPLY;
					SendMsg.val[0] = group;
					SendMsg.val[1] = (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					send_msg(MODULE_TO_COA1+group, (char *)&SendMsg);
					myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]++;
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
						>= MAX_EOL_DATA) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX] = 0;
					}
					myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P0;
					break;
			}
			break;
		case P29: //Elac_Fuse_V Check(1415)
			switch(myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]) {
				case P0:
					break;
				case P1:
					data_index = (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					memset((char *)&myPs->eol_data[data_index].index,
						0, sizeof(S_EOL_DATA));

					memset((char *)&myData->CAN.uds_rx[can_ch1].uc_val[0], 0, 8);

					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] = 0;
					myData->CAN.signal[can_ch1][CAN_SIG_TX_PHASE_UDS_DATA1] = 15;
					myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]++;
					break;
				case P2:
					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1]++;
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] >= 100) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2]++;
					}
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] == 1) {
						myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P101;
					}
					break;
				case P3:
					data_index = (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					myPs->eol_data[data_index].index
						= (unsigned char)((int)myPs->signal
						[M_SIG_EOL_PROCEDURE_INDEX]
						+ (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DIVISION] * 100);
					myPs->eol_data[data_index].sub_index
						= (unsigned char)myPs->
						signal[M_SIG_EOL_PROCEDURE_SUB_INDEX];
					myPs->eol_data[data_index].response = 0; //ok

					memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
					SendMsg.msg = MSG_MODULE_COA_EOL_PROCEDURE_REPLY;
					SendMsg.val[0] = group;
					SendMsg.val[1] = (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					send_msg(MODULE_TO_COA1+group, (char *)&SendMsg);
					myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]++;
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
						>= MAX_EOL_DATA) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX] = 0;
					}
					myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P0;
					break;
				default:
					data_index = (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					myPs->eol_data[data_index].index
						= (unsigned char)((int)myPs->signal
						[M_SIG_EOL_PROCEDURE_INDEX]
						+ (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DIVISION] * 100);
					myPs->eol_data[data_index].sub_index
						= (unsigned char)myPs->
						signal[M_SIG_EOL_PROCEDURE_SUB_INDEX];
					myPs->eol_data[data_index].response
						= (short int)myPs->signal
						[M_SIG_EOL_PROCEDURE_PHASE] - 100; //fail

					memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
					SendMsg.msg = MSG_MODULE_COA_EOL_PROCEDURE_REPLY;
					SendMsg.val[0] = group;
					SendMsg.val[1] = (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					send_msg(MODULE_TO_COA1+group, (char *)&SendMsg);
					myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]++;
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
						>= MAX_EOL_DATA) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX] = 0;
					}
					myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P0;
					break;
			}
			break;
		case P30:
			switch(myPs->signal[M_SIG_EOL_PROCEDURE_SUB_INDEX]) {
				case P0: //MAIN+, CHARGER+ close(1557)
					switch(myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]) {
						case P0:
							break;
						case P1:
							data_index = (int)myPs->signal
								[M_SIG_EOL_PROCEDURE_DATA_INDEX];
							memset((char *)&myPs->eol_data[data_index].index,
								0, sizeof(S_EOL_DATA));

							memset((char *)&myData->CAN.uds_rx[can_ch1].uc_val[0],
								0, 8);

							myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
							myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] = 0;
							myData->CAN.signal[can_ch1][CAN_SIG_TX_PHASE_EOL_DATA2]
								= 7;
							myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]++;
							break;
						case P2:
							myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1]++;
							if(myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1]
								>= 100) {
								myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
								myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2]++;
							}
							if(myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] == 1) {
								//myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]
								//	= P101;
								myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]++;//kjg_w
							}
							break;
						case P3:
							data_index = (int)myPs->signal
								[M_SIG_EOL_PROCEDURE_DATA_INDEX];
							myPs->eol_data[data_index].index
								= (unsigned char)((int)myPs->signal
								[M_SIG_EOL_PROCEDURE_INDEX]
								+ (int)myPs->signal
								[M_SIG_EOL_PROCEDURE_DIVISION] * 100);
							myPs->eol_data[data_index].sub_index
								= (unsigned char)myPs->
								signal[M_SIG_EOL_PROCEDURE_SUB_INDEX];
							myPs->eol_data[data_index].response = 0; //ok

							memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
							SendMsg.msg = MSG_MODULE_COA_EOL_PROCEDURE_REPLY;
							SendMsg.val[0] = group;
							SendMsg.val[1] = (int)myPs->signal
								[M_SIG_EOL_PROCEDURE_DATA_INDEX];
							send_msg(MODULE_TO_COA1+group, (char *)&SendMsg);
							myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]++;
							if(myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
								>= MAX_EOL_DATA) {
								myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
									= 0;
							}
							myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P0;
							break;
						default:
							data_index = (int)myPs->signal
								[M_SIG_EOL_PROCEDURE_DATA_INDEX];
							myPs->eol_data[data_index].index
								= (unsigned char)((int)myPs->signal
								[M_SIG_EOL_PROCEDURE_INDEX]
								+ (int)myPs->signal
								[M_SIG_EOL_PROCEDURE_DIVISION] * 100);
							myPs->eol_data[data_index].sub_index
								= (unsigned char)myPs->
								signal[M_SIG_EOL_PROCEDURE_SUB_INDEX];
							myPs->eol_data[data_index].response
								= (short int)myPs->signal
								[M_SIG_EOL_PROCEDURE_PHASE] - 100; //fail

							memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
							SendMsg.msg = MSG_MODULE_COA_EOL_PROCEDURE_REPLY;
							SendMsg.val[0] = group;
							SendMsg.val[1] = (int)myPs->signal
								[M_SIG_EOL_PROCEDURE_DATA_INDEX];
							send_msg(MODULE_TO_COA1+group, (char *)&SendMsg);
							myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]++;
							if(myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
								>= MAX_EOL_DATA) {
								myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
									= 0;
							}
							myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P0;
							break;
					}
					break;
				case P1: //MAIN+, CHARGER+ open(1558)
					switch(myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]) {
						case P0:
							break;
						case P1:
							data_index = (int)myPs->signal
								[M_SIG_EOL_PROCEDURE_DATA_INDEX];
							memset((char *)&myPs->eol_data[data_index].index,
								0, sizeof(S_EOL_DATA));

							memset((char *)&myData->CAN.uds_rx[can_ch1].uc_val[0],
								0, 8);

							myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
							myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] = 0;
							myData->CAN.signal[can_ch1][CAN_SIG_TX_PHASE_EOL_DATA2]
								= 8;
							myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]++;
							break;
						case P2:
							myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1]++;
							if(myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1]
								>= 100) {
								myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
								myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2]++;
							}
							if(myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] == 1) {
								//myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]
								//	= P101;
								myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]++;//kjg_w
							}
							break;
						case P3:
							data_index = (int)myPs->signal
								[M_SIG_EOL_PROCEDURE_DATA_INDEX];
							myPs->eol_data[data_index].index
								= (unsigned char)((int)myPs->signal
								[M_SIG_EOL_PROCEDURE_INDEX]
								+ (int)myPs->signal
								[M_SIG_EOL_PROCEDURE_DIVISION] * 100);
							myPs->eol_data[data_index].sub_index
								= (unsigned char)myPs->
								signal[M_SIG_EOL_PROCEDURE_SUB_INDEX];
							myPs->eol_data[data_index].response = 0; //ok

							memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
							SendMsg.msg = MSG_MODULE_COA_EOL_PROCEDURE_REPLY;
							SendMsg.val[0] = group;
							SendMsg.val[1] = (int)myPs->signal
								[M_SIG_EOL_PROCEDURE_DATA_INDEX];
							send_msg(MODULE_TO_COA1+group, (char *)&SendMsg);
							myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]++;
							if(myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
								>= MAX_EOL_DATA) {
								myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
									= 0;
							}
							myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P0;
							break;
						default:
							data_index = (int)myPs->signal
								[M_SIG_EOL_PROCEDURE_DATA_INDEX];
							myPs->eol_data[data_index].index
								= (unsigned char)((int)myPs->signal
								[M_SIG_EOL_PROCEDURE_INDEX]
								+ (int)myPs->signal
								[M_SIG_EOL_PROCEDURE_DIVISION] * 100);
							myPs->eol_data[data_index].sub_index
								= (unsigned char)myPs->
								signal[M_SIG_EOL_PROCEDURE_SUB_INDEX];
							myPs->eol_data[data_index].response
								= (short int)myPs->signal
								[M_SIG_EOL_PROCEDURE_PHASE] - 100; //fail

							memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
							SendMsg.msg = MSG_MODULE_COA_EOL_PROCEDURE_REPLY;
							SendMsg.val[0] = group;
							SendMsg.val[1] = (int)myPs->signal
								[M_SIG_EOL_PROCEDURE_DATA_INDEX];
							send_msg(MODULE_TO_COA1+group, (char *)&SendMsg);
							myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]++;
							if(myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
								>= MAX_EOL_DATA) {
								myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
									= 0;
							}
							myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P0;
							break;
					}
					break;
				default:
					break;
			}
			break;
		case P31: //Charger_Fuse_V Check(1414)
			switch(myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]) {
				case P0:
					break;
				case P1:
					data_index = (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					memset((char *)&myPs->eol_data[data_index].index,
						0, sizeof(S_EOL_DATA));

					memset((char *)&myData->CAN.uds_rx[can_ch1].uc_val[0], 0, 8);

					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] = 0;
					myData->CAN.signal[can_ch1][CAN_SIG_TX_PHASE_UDS_DATA1] = 14;
					myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]++;
					break;
				case P2:
					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1]++;
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] >= 100) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2]++;
					}
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] == 1) {
						myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P101;
					}
					break;
				case P3:
					data_index = (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					myPs->eol_data[data_index].index
						= (unsigned char)((int)myPs->signal
						[M_SIG_EOL_PROCEDURE_INDEX]
						+ (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DIVISION] * 100);
					myPs->eol_data[data_index].sub_index
						= (unsigned char)myPs->
						signal[M_SIG_EOL_PROCEDURE_SUB_INDEX];
					myPs->eol_data[data_index].response = 0; //ok

					memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
					SendMsg.msg = MSG_MODULE_COA_EOL_PROCEDURE_REPLY;
					SendMsg.val[0] = group;
					SendMsg.val[1] = (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					send_msg(MODULE_TO_COA1+group, (char *)&SendMsg);
					myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]++;
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
						>= MAX_EOL_DATA) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX] = 0;
					}
					myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P0;
					break;
				default:
					data_index = (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					myPs->eol_data[data_index].index
						= (unsigned char)((int)myPs->signal
						[M_SIG_EOL_PROCEDURE_INDEX]
						+ (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DIVISION] * 100);
					myPs->eol_data[data_index].sub_index
						= (unsigned char)myPs->
						signal[M_SIG_EOL_PROCEDURE_SUB_INDEX];
					myPs->eol_data[data_index].response
						= (short int)myPs->signal
						[M_SIG_EOL_PROCEDURE_PHASE] - 100; //fail

					memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
					SendMsg.msg = MSG_MODULE_COA_EOL_PROCEDURE_REPLY;
					SendMsg.val[0] = group;
					SendMsg.val[1] = (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					send_msg(MODULE_TO_COA1+group, (char *)&SendMsg);
					myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]++;
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
						>= MAX_EOL_DATA) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX] = 0;
					}
					myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P0;
					break;
			}
			break;
		case P32: //Isolation resistance check(1416)
			switch(myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]) {
				case P0:
					break;
				case P1:
					data_index = (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					memset((char *)&myPs->eol_data[data_index].index,
						0, sizeof(S_EOL_DATA));

					memset((char *)&myData->CAN.uds_rx[can_ch1].uc_val[0], 0, 8);

					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] = 0;
					myData->CAN.signal[can_ch1][CAN_SIG_TX_PHASE_UDS_DATA1] = 16;
					myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]++;
					break;
				case P2:
					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1]++;
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] >= 100) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2]++;
					}
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] == 1) {
						myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P101;
					}
					break;
				case P3:
					data_index = (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					myPs->eol_data[data_index].index
						= (unsigned char)((int)myPs->signal
						[M_SIG_EOL_PROCEDURE_INDEX]
						+ (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DIVISION] * 100);
					myPs->eol_data[data_index].sub_index
						= (unsigned char)myPs->
						signal[M_SIG_EOL_PROCEDURE_SUB_INDEX];
					myPs->eol_data[data_index].response = 0; //ok

					memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
					SendMsg.msg = MSG_MODULE_COA_EOL_PROCEDURE_REPLY;
					SendMsg.val[0] = group;
					SendMsg.val[1] = (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					send_msg(MODULE_TO_COA1+group, (char *)&SendMsg);
					myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]++;
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
						>= MAX_EOL_DATA) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX] = 0;
					}
					myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P0;
					break;
				default:
					data_index = (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					myPs->eol_data[data_index].index
						= (unsigned char)((int)myPs->signal
						[M_SIG_EOL_PROCEDURE_INDEX]
						+ (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DIVISION] * 100);
					myPs->eol_data[data_index].sub_index
						= (unsigned char)myPs->
						signal[M_SIG_EOL_PROCEDURE_SUB_INDEX];
					myPs->eol_data[data_index].response
						= (short int)myPs->signal
						[M_SIG_EOL_PROCEDURE_PHASE] - 100; //fail

					memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
					SendMsg.msg = MSG_MODULE_COA_EOL_PROCEDURE_REPLY;
					SendMsg.val[0] = group;
					SendMsg.val[1] = (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					send_msg(MODULE_TO_COA1+group, (char *)&SendMsg);
					myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]++;
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
						>= MAX_EOL_DATA) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX] = 0;
					}
					myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P0;
					break;
			}
			break;
		case P33:
			switch(myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]) {
				case P0:
					break;
				case P1: //HVIL_Return Check(1436)
					data_index
						= (int)myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					memset((char *)&myPs->eol_data[data_index].index, 0,
						sizeof(S_EOL_DATA));

					memset((char *)&myData->CAN.uds_rx[can_ch1].uc_val[0], 0, 8);

					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] = 0;
					myData->CAN.signal[can_ch1][CAN_SIG_TX_PHASE_UDS_DATA1] = 36;
					myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]++;
					break;
				case P2:
					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1]++;
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] >= 100) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2]++;
					}
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] == 1) {
						myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P101;
					}
					break;
				case P3: //HVIL_Source Check(1559)
					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] = 0;
					myData->CAN.signal[0][CAN_SIG_TX_PHASE_EOL_DATA2] = 9;
					myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]++;
					break;
				case P4:
					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1]++;
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] >= 100) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2]++;
					}
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] == 1) {
						myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P102;
					}
					break;
				case P5:
					data_index
						= (int)myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					myPs->eol_data[data_index].index
						= (unsigned char)((int)myPs->signal
						[M_SIG_EOL_PROCEDURE_INDEX]
						+ (int)myPs->signal[M_SIG_EOL_PROCEDURE_DIVISION]
						* 100);
					myPs->eol_data[data_index].sub_index = (unsigned char)myPs->
						signal[M_SIG_EOL_PROCEDURE_SUB_INDEX];
					myPs->eol_data[data_index].response = 0; //ok

					memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
					SendMsg.msg = MSG_MODULE_COA_EOL_PROCEDURE_REPLY;
					SendMsg.val[0] = group;
					SendMsg.val[1]
						= (int)myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					send_msg(MODULE_TO_COA1+group, (char *)&SendMsg);
					myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]++;
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
						>= MAX_EOL_DATA) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX] = 0;
					}
					myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P0;
					break;
				default:
					data_index
						= (int)myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					myPs->eol_data[data_index].index
						= (unsigned char)((int)myPs->signal
						[M_SIG_EOL_PROCEDURE_INDEX]
						+ (int)myPs->signal[M_SIG_EOL_PROCEDURE_DIVISION]
						* 100);
					myPs->eol_data[data_index].sub_index = (unsigned char)myPs->
						signal[M_SIG_EOL_PROCEDURE_SUB_INDEX];
					myPs->eol_data[data_index].response
						= (short int)myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]
						- 100; //fail

					memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
					SendMsg.msg = MSG_MODULE_COA_EOL_PROCEDURE_REPLY;
					SendMsg.val[0] = group;
					SendMsg.val[1]
						= (int)myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					send_msg(MODULE_TO_COA1+group, (char *)&SendMsg);
					myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]++;
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
						>= MAX_EOL_DATA) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX] = 0;
					}
					myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P0;
					break;
			}
			break;
		case P34:
			switch(myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]) {
				case P0:
					break;
				case P1: //AUX_HVIL_Return Check(1560)
					data_index
						= (int)myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					memset((char *)&myPs->eol_data[data_index].index, 0,
						sizeof(S_EOL_DATA));

					memset((char *)&myData->CAN.uds_rx[can_ch1].uc_val[0], 0, 8);

					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] = 0;
					myData->CAN.signal[can_ch1][CAN_SIG_TX_PHASE_EOL_DATA2] = 10;
					myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]++;
					break;
				case P2:
					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1]++;
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] >= 100) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2]++;
					}
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] == 1) {
						myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P101;
					}
					break;
				case P3: //AUX_HVIL_Source Check(1437)
					memset((char *)&myData->CAN.uds_rx[0].uc_val[0], 0, 8);

					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] = 0;
					myData->CAN.signal[0][CAN_SIG_TX_PHASE_UDS_DATA1] = 37;
					myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]++;
					break;
				case P4:
					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1]++;
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] >= 100) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2]++;
					}
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] == 1) {
						myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P102;
					}
					break;
				case P5:
					data_index
						= (int)myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					myPs->eol_data[data_index].index
						= (unsigned char)((int)myPs->signal
						[M_SIG_EOL_PROCEDURE_INDEX]
						+ (int)myPs->signal[M_SIG_EOL_PROCEDURE_DIVISION]
						* 100);
					myPs->eol_data[data_index].sub_index = (unsigned char)myPs->
						signal[M_SIG_EOL_PROCEDURE_SUB_INDEX];
					myPs->eol_data[data_index].response = 0; //ok

					memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
					SendMsg.msg = MSG_MODULE_COA_EOL_PROCEDURE_REPLY;
					SendMsg.val[0] = group;
					SendMsg.val[1]
						= (int)myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					send_msg(MODULE_TO_COA1+group, (char *)&SendMsg);
					myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]++;
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
						>= MAX_EOL_DATA) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX] = 0;
					}
					myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P0;
					break;
				default:
					data_index
						= (int)myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					myPs->eol_data[data_index].index
						= (unsigned char)((int)myPs->signal
						[M_SIG_EOL_PROCEDURE_INDEX]
						+ (int)myPs->signal[M_SIG_EOL_PROCEDURE_DIVISION]
						* 100);
					myPs->eol_data[data_index].sub_index = (unsigned char)myPs->
						signal[M_SIG_EOL_PROCEDURE_SUB_INDEX];
					myPs->eol_data[data_index].response
						= (short int)myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]
						- 100; //fail

					memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
					SendMsg.msg = MSG_MODULE_COA_EOL_PROCEDURE_REPLY;
					SendMsg.val[0] = group;
					SendMsg.val[1]
						= (int)myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					send_msg(MODULE_TO_COA1+group, (char *)&SendMsg);
					myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]++;
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
						>= MAX_EOL_DATA) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX] = 0;
					}
					myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P0;
					break;
			}
			break;
		case P35: //Flash_Pack_ID
			switch(myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]) {
				case P0:
					break;
				case P1:
					data_index
						= (int)myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					memset((char *)&myPs->eol_data[data_index].index, 0,
						sizeof(S_EOL_DATA));

					memset((char *)&myData->CAN.uds_rx[can_ch1].uc_val[0], 0, 8);

					myData->CAN.uds_rx_message_count = 0;
					myData->CAN.uds_rx_dtc_count = 0;

					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] = 0;
					myPs->signal[M_SIG_BMS_EOL_FLASH_PHASE] = P1;
					myPs->signal[M_SIG_EOL_PROCEDURE_VBF_FILE_DOWNLOAD] = P1; //kjg_121004
					myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]++;
					break;
				case P2:
					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1]++;
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] >= 100) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2]++;
					}
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] == 60) {
						myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P102;
					}
					break;
				case P3:
					data_index
						= (int)myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					myPs->eol_data[data_index].index
						= (unsigned char)((int)myPs->signal
						[M_SIG_EOL_PROCEDURE_INDEX]
						+ (int)myPs->signal[M_SIG_EOL_PROCEDURE_DIVISION]
						* 100);
					myPs->eol_data[data_index].sub_index = (unsigned char)myPs->
						signal[M_SIG_EOL_PROCEDURE_SUB_INDEX];
					myPs->eol_data[data_index].response = 0; //ok

					memcpy((char *)&myPs->eol_data[data_index].string_value[0],
						(char *)&myPs->bms_eol_data2.pack_id[3][0], 40);

					memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
					SendMsg.msg = MSG_MODULE_COA_EOL_PROCEDURE_REPLY;
					SendMsg.val[0] = group;
					SendMsg.val[1]
						= (int)myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					send_msg(MODULE_TO_COA1+group, (char *)&SendMsg);
					myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]++;
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
						>= MAX_EOL_DATA) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX] = 0;
					}
					myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P0;
					myPs->signal[M_SIG_EOL_PROCEDURE_VBF_FILE_DOWNLOAD] = P0; //kjg_121004
					break;
				default:
					data_index
						= (int)myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					myPs->eol_data[data_index].index
						= (unsigned char)((int)myPs->signal
						[M_SIG_EOL_PROCEDURE_INDEX]
						+ (int)myPs->signal[M_SIG_EOL_PROCEDURE_DIVISION]
						* 100);
					myPs->eol_data[data_index].sub_index = (unsigned char)myPs->
						signal[M_SIG_EOL_PROCEDURE_SUB_INDEX];
					myPs->eol_data[data_index].response
						= (short int)myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]
						- 100; //fail

					memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
					SendMsg.msg = MSG_MODULE_COA_EOL_PROCEDURE_REPLY;
					SendMsg.val[0] = group;
					SendMsg.val[1]
						= (int)myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					send_msg(MODULE_TO_COA1+group, (char *)&SendMsg);
					myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]++;
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
						>= MAX_EOL_DATA) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX] = 0;
					}
					myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P0;
					myPs->signal[M_SIG_EOL_PROCEDURE_VBF_FILE_DOWNLOAD] = P0; //kjg_121004
					break;
			}
			break;
		case P36: //Read_Pack_ID
			switch(myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]) {
				case P0:
					break;
				case P1:
					data_index
						= (int)myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					memset((char *)&myPs->eol_data[data_index].index, 0,
						sizeof(S_EOL_DATA));

					memset((char *)&myData->CAN.uds_rx[can_ch1].uc_val[0], 0, 8);

					myData->CAN.uds_rx_message_count = 0;
					myData->CAN.uds_rx_dtc_count = 0;

					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] = 0;
					myPs->signal[M_SIG_BMS_EOL_FLASH_PHASE] = P100;
					myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]++;
					break;
				case P2:
					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1]++;
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] >= 100) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2]++;
					}
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] == 5) {
						myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P102;
					}
					break;
				case P3:
					data_index
						= (int)myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					myPs->eol_data[data_index].index
						= (unsigned char)((int)myPs->signal
						[M_SIG_EOL_PROCEDURE_INDEX]
						+ (int)myPs->signal[M_SIG_EOL_PROCEDURE_DIVISION]
						* 100);
					myPs->eol_data[data_index].sub_index = (unsigned char)myPs->
						signal[M_SIG_EOL_PROCEDURE_SUB_INDEX];
					myPs->eol_data[data_index].response = 0; //ok

					memcpy((char *)&myPs->eol_data[data_index].string_value[0],
						(char *)&myPs->bms_eol_data2.pack_id[3][0], 40);

					memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
					SendMsg.msg = MSG_MODULE_COA_EOL_PROCEDURE_REPLY;
					SendMsg.val[0] = group;
					SendMsg.val[1]
						= (int)myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					send_msg(MODULE_TO_COA1+group, (char *)&SendMsg);
					myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]++;
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
						>= MAX_EOL_DATA) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX] = 0;
					}
					myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P0;
					break;
				default:
					data_index
						= (int)myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					myPs->eol_data[data_index].index
						= (unsigned char)((int)myPs->signal
						[M_SIG_EOL_PROCEDURE_INDEX]
						+ (int)myPs->signal[M_SIG_EOL_PROCEDURE_DIVISION]
						* 100);
					myPs->eol_data[data_index].sub_index = (unsigned char)myPs->
						signal[M_SIG_EOL_PROCEDURE_SUB_INDEX];
					myPs->eol_data[data_index].response
						= (short int)myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]
						- 100; //fail

					memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
					SendMsg.msg = MSG_MODULE_COA_EOL_PROCEDURE_REPLY;
					SendMsg.val[0] = group;
					SendMsg.val[1]
						= (int)myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					send_msg(MODULE_TO_COA1+group, (char *)&SendMsg);
					myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]++;
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
						>= MAX_EOL_DATA) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX] = 0;
					}
					myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P0;
					break;
			}
			break;
		case P37: //Clear_DTCs(1433)
			switch(myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]) {
				case P0:
					break;
				case P1:
					data_index
						= (int)myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					memset((char *)&myPs->eol_data[data_index].index, 0,
						sizeof(S_EOL_DATA));

					memset((char *)&myData->CAN.uds_rx[can_ch1].uc_val[0], 0, 8);

					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] = 0;
					myData->CAN.signal[can_ch1][CAN_SIG_TX_PHASE_UDS_DATA1] = 33;
					myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]++;
					break;
				case P2:
					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1]++;
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] >= 100) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2]++;
					}
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] == 1) {
						myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P101;
					}
					break;
				case P3:
					data_index
						= (int)myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					myPs->eol_data[data_index].index
						= (unsigned char)((int)myPs->signal
						[M_SIG_EOL_PROCEDURE_INDEX]
						+ (int)myPs->signal[M_SIG_EOL_PROCEDURE_DIVISION]
						* 100);
					myPs->eol_data[data_index].sub_index = (unsigned char)myPs->
						signal[M_SIG_EOL_PROCEDURE_SUB_INDEX];
					myPs->eol_data[data_index].response = 0; //ok

					memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
					SendMsg.msg = MSG_MODULE_COA_EOL_PROCEDURE_REPLY;
					SendMsg.val[0] = group;
					SendMsg.val[1]
						= (int)myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					send_msg(MODULE_TO_COA1+group, (char *)&SendMsg);
					myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]++;
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
						>= MAX_EOL_DATA) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX] = 0;
					}
					myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P0;
					break;
				default:
					data_index
						= (int)myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					myPs->eol_data[data_index].index
						= (unsigned char)((int)myPs->signal
						[M_SIG_EOL_PROCEDURE_INDEX]
						+ (int)myPs->signal[M_SIG_EOL_PROCEDURE_DIVISION]
						* 100);
					myPs->eol_data[data_index].sub_index = (unsigned char)myPs->
						signal[M_SIG_EOL_PROCEDURE_SUB_INDEX];
					myPs->eol_data[data_index].response
						= (short int)myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]
						- 100; //fail

					memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
					SendMsg.msg = MSG_MODULE_COA_EOL_PROCEDURE_REPLY;
					SendMsg.val[0] = group;
					SendMsg.val[1]
						= (int)myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					send_msg(MODULE_TO_COA1+group, (char *)&SendMsg);
					myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]++;
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
						>= MAX_EOL_DATA) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX] = 0;
					}
					myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P0;
					break;
			}
			break;
		case P38: //SAVE_EEPROM(1434)
			switch(myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]) {
				case P0:
					break;
				case P1:
					data_index
						= (int)myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					memset((char *)&myPs->eol_data[data_index].index, 0,
						sizeof(S_EOL_DATA));

					memset((char *)&myData->CAN.uds_rx[can_ch1].uc_val[0], 0, 8);

					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] = 0;
					myData->CAN.signal[can_ch1][CAN_SIG_TX_PHASE_UDS_DATA1] = 34;
					myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]++;
					break;
				case P2:
					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1]++;
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] >= 100) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2]++;
					}
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] == 1) {
						myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P101;
					}
					break;
				case P3:
					data_index
						= (int)myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					myPs->eol_data[data_index].index
						= (unsigned char)((int)myPs->signal
						[M_SIG_EOL_PROCEDURE_INDEX]
						+ (int)myPs->signal[M_SIG_EOL_PROCEDURE_DIVISION]
						* 100);
					myPs->eol_data[data_index].sub_index = (unsigned char)myPs->
						signal[M_SIG_EOL_PROCEDURE_SUB_INDEX];
					myPs->eol_data[data_index].response = 0; //ok

					memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
					SendMsg.msg = MSG_MODULE_COA_EOL_PROCEDURE_REPLY;
					SendMsg.val[0] = group;
					SendMsg.val[1]
						= (int)myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					send_msg(MODULE_TO_COA1+group, (char *)&SendMsg);
					myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]++;
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
						>= MAX_EOL_DATA) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX] = 0;
					}
					myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P0;
					break;
				default:
					data_index
						= (int)myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					myPs->eol_data[data_index].index
						= (unsigned char)((int)myPs->signal
						[M_SIG_EOL_PROCEDURE_INDEX]
						+ (int)myPs->signal[M_SIG_EOL_PROCEDURE_DIVISION]
						* 100);
					myPs->eol_data[data_index].sub_index = (unsigned char)myPs->
						signal[M_SIG_EOL_PROCEDURE_SUB_INDEX];
					myPs->eol_data[data_index].response
						= (short int)myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]
						- 100; //fail

					memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
					SendMsg.msg = MSG_MODULE_COA_EOL_PROCEDURE_REPLY;
					SendMsg.val[0] = group;
					SendMsg.val[1]
						= (int)myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					send_msg(MODULE_TO_COA1+group, (char *)&SendMsg);
					myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]++;
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
						>= MAX_EOL_DATA) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX] = 0;
					}
					myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P0;
					break;
			}
			break;
		case P39: //ECU_Reset(1435)
			switch(myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]) {
				case P0:
					break;
				case P1:
					data_index = (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					memset((char *)&myPs->eol_data[data_index].index,
						0, sizeof(S_EOL_DATA));

					memset((char *)&myData->CAN.uds_rx[can_ch1].uc_val[0], 0, 8);

					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] = 0;
					myData->CAN.signal[can_ch1][CAN_SIG_TX_PHASE_UDS_DATA1] = 35;
					myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]++;
					break;
				case P2:
					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1]++;
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] >= 100) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2]++;
					}
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] == 1) {
						//myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P101;
						myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]++;
					}
					break;
				case P3:
					data_index = (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					myPs->eol_data[data_index].index
						= (unsigned char)((int)myPs->signal
						[M_SIG_EOL_PROCEDURE_INDEX]
						+ (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DIVISION] * 100);
					myPs->eol_data[data_index].sub_index
						= (unsigned char)myPs->
						signal[M_SIG_EOL_PROCEDURE_SUB_INDEX];
					myPs->eol_data[data_index].response = 0; //ok

					memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
					SendMsg.msg = MSG_MODULE_COA_EOL_PROCEDURE_REPLY;
					SendMsg.val[0] = group;
					SendMsg.val[1] = (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					send_msg(MODULE_TO_COA1+group, (char *)&SendMsg);
					myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]++;
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
						>= MAX_EOL_DATA) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX] = 0;
					}
					myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P0;
					break;
				default:
					data_index = (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					myPs->eol_data[data_index].index
						= (unsigned char)((int)myPs->signal
						[M_SIG_EOL_PROCEDURE_INDEX]
						+ (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DIVISION] * 100);
					myPs->eol_data[data_index].sub_index
						= (unsigned char)myPs->
						signal[M_SIG_EOL_PROCEDURE_SUB_INDEX];
					myPs->eol_data[data_index].response
						= (short int)myPs->signal
						[M_SIG_EOL_PROCEDURE_PHASE] - 100; //fail

					memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
					SendMsg.msg = MSG_MODULE_COA_EOL_PROCEDURE_REPLY;
					SendMsg.val[0] = group;
					SendMsg.val[1] = (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					send_msg(MODULE_TO_COA1+group, (char *)&SendMsg);
					myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]++;
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
						>= MAX_EOL_DATA) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX] = 0;
					}
					myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P0;
					break;
			}
			break;
		case P42: //Max_Cell_V(1561)
			switch(myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]) {
				case P0:
					break;
				case P1:
					data_index = (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					memset((char *)&myPs->eol_data[data_index].index,
						0, sizeof(S_EOL_DATA));

					memset((char *)&myData->CAN.uds_rx[can_ch1].uc_val[0], 0, 8);

					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] = 0;
					myData->CAN.signal[can_ch1][CAN_SIG_TX_PHASE_EOL_DATA2] = 11;
					myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]++;
					break;
				case P2:
					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1]++;
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] >= 100) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2]++;
					}
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] == 1) {
						myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P101;
					}
					break;
				case P3:
					data_index = (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					myPs->eol_data[data_index].index
						= (unsigned char)((int)myPs->signal
						[M_SIG_EOL_PROCEDURE_INDEX]
						+ (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DIVISION] * 100);
					myPs->eol_data[data_index].sub_index
						= (unsigned char)myPs->
						signal[M_SIG_EOL_PROCEDURE_SUB_INDEX];
					myPs->eol_data[data_index].response = 0; //ok

					memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
					SendMsg.msg = MSG_MODULE_COA_EOL_PROCEDURE_REPLY;
					SendMsg.val[0] = group;
					SendMsg.val[1] = (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					send_msg(MODULE_TO_COA1+group, (char *)&SendMsg);
					myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]++;
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
						>= MAX_EOL_DATA) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX] = 0;
					}
					myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P0;
					break;
				default:
					data_index = (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					myPs->eol_data[data_index].index
						= (unsigned char)((int)myPs->signal
						[M_SIG_EOL_PROCEDURE_INDEX]
						+ (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DIVISION] * 100);
					myPs->eol_data[data_index].sub_index
						= (unsigned char)myPs->
						signal[M_SIG_EOL_PROCEDURE_SUB_INDEX];
					myPs->eol_data[data_index].response
						= (short int)myPs->signal
						[M_SIG_EOL_PROCEDURE_PHASE] - 100; //fail

					memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
					SendMsg.msg = MSG_MODULE_COA_EOL_PROCEDURE_REPLY;
					SendMsg.val[0] = group;
					SendMsg.val[1] = (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					send_msg(MODULE_TO_COA1+group, (char *)&SendMsg);
					myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]++;
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
						>= MAX_EOL_DATA) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX] = 0;
					}
					myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P0;
					break;
			}
			break;
		case P43: //Min_Cell_V(1562)
			switch(myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]) {
				case P0:
					break;
				case P1:
					data_index = (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					memset((char *)&myPs->eol_data[data_index].index,
						0, sizeof(S_EOL_DATA));

					memset((char *)&myData->CAN.uds_rx[can_ch1].uc_val[0], 0, 8);

					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] = 0;
					myData->CAN.signal[can_ch1][CAN_SIG_TX_PHASE_EOL_DATA2] = 12;
					myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]++;
					break;
				case P2:
					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1]++;
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] >= 100) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2]++;
					}
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] == 1) {
						myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P101;
					}
					break;
				case P3:
					data_index = (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					myPs->eol_data[data_index].index
						= (unsigned char)((int)myPs->signal
						[M_SIG_EOL_PROCEDURE_INDEX]
						+ (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DIVISION] * 100);
					myPs->eol_data[data_index].sub_index
						= (unsigned char)myPs->
						signal[M_SIG_EOL_PROCEDURE_SUB_INDEX];
					myPs->eol_data[data_index].response = 0; //ok

					memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
					SendMsg.msg = MSG_MODULE_COA_EOL_PROCEDURE_REPLY;
					SendMsg.val[0] = group;
					SendMsg.val[1] = (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					send_msg(MODULE_TO_COA1+group, (char *)&SendMsg);
					myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]++;
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
						>= MAX_EOL_DATA) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX] = 0;
					}
					myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P0;
					break;
				default:
					data_index = (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					myPs->eol_data[data_index].index
						= (unsigned char)((int)myPs->signal
						[M_SIG_EOL_PROCEDURE_INDEX]
						+ (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DIVISION] * 100);
					myPs->eol_data[data_index].sub_index
						= (unsigned char)myPs->
						signal[M_SIG_EOL_PROCEDURE_SUB_INDEX];
					myPs->eol_data[data_index].response
						= (short int)myPs->signal
						[M_SIG_EOL_PROCEDURE_PHASE] - 100; //fail

					memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
					SendMsg.msg = MSG_MODULE_COA_EOL_PROCEDURE_REPLY;
					SendMsg.val[0] = group;
					SendMsg.val[1] = (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					send_msg(MODULE_TO_COA1+group, (char *)&SendMsg);
					myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]++;
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
						>= MAX_EOL_DATA) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX] = 0;
					}
					myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P0;
					break;
			}
			break;
		case P44: //All_Cell_V(1563)
			switch(myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]) {
				case P0:
					break;
				case P1:
					myPs->signal[M_SIG_EOL_PROCEDURE_CHECK_INDEX] = 0;
					myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]++;
					break;
				case P2:
					data_index = (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					memset((char *)&myPs->eol_data[data_index].index,
						0, sizeof(S_EOL_DATA));

					memset((char *)&myData->CAN.uds_rx[can_ch1].uc_val[0], 0, 8);

					check_index = (int)0xDA3E
						+ (int)myPs->signal[M_SIG_EOL_PROCEDURE_CHECK_INDEX];

					myData->CAN.uds_tx[can_ch1].uc_val[0] = 0x03;
					myData->CAN.uds_tx[can_ch1].uc_val[1] = 0x22;
					myData->CAN.uds_tx[can_ch1].uc_val[2]
						= (unsigned char)((check_index >> 8) & 0x000000FF);
					myData->CAN.uds_tx[can_ch1].uc_val[3]
						= (unsigned char)(check_index & 0x000000FF);
					myData->CAN.uds_tx[can_ch1].uc_val[4] = 0x00;
					myData->CAN.uds_tx[can_ch1].uc_val[5] = 0x00;
					myData->CAN.uds_tx[can_ch1].uc_val[6] = 0x00;
					myData->CAN.uds_tx[can_ch1].uc_val[7] = 0x00;

					myPs->signal[M_SIG_EOL_PROCEDURE_CHECK_INDEX]++;

					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] = 0;
					myData->CAN.signal[can_ch1][CAN_SIG_TX_PHASE_EOL_DATA2] = 13;
					myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]++;
					break;
				case P3:
					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1]++;
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] >= 100) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2]++;
					}
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] == 1) {
						myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P101;
					}
					break;
				case P4:
					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1]++;
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] >= 100) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2]++;
					}
					//if(myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] == 1) {
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] <= 10) {
						break;
					}

					data_index = (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					myPs->eol_data[data_index].index
						= (unsigned char)((int)myPs->signal
						[M_SIG_EOL_PROCEDURE_INDEX]
						+ (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DIVISION] * 100);
					myPs->eol_data[data_index].sub_index
						= myPs->signal[M_SIG_EOL_PROCEDURE_CHECK_INDEX];
					myPs->eol_data[data_index].response = 0; //ok

					memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
					SendMsg.msg = MSG_MODULE_COA_EOL_PROCEDURE_REPLY;
					SendMsg.val[0] = group;
					SendMsg.val[1] = (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					send_msg(MODULE_TO_COA1+group, (char *)&SendMsg);
					myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]++;
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
						>= MAX_EOL_DATA) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX] = 0;
					}

					if(myPs->signal[M_SIG_EOL_PROCEDURE_CHECK_INDEX] == 100) {
						myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P0;
					} else {
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P2;
					}
					break;
				default:
					data_index = (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					myPs->eol_data[data_index].index
						= (unsigned char)((int)myPs->signal
						[M_SIG_EOL_PROCEDURE_INDEX]
						+ (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DIVISION] * 100);
					myPs->eol_data[data_index].sub_index
						= (unsigned char)myPs->
						signal[M_SIG_EOL_PROCEDURE_SUB_INDEX];
					myPs->eol_data[data_index].response
						= (short int)myPs->signal
						[M_SIG_EOL_PROCEDURE_PHASE] - 100; //fail

					memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
					SendMsg.msg = MSG_MODULE_COA_EOL_PROCEDURE_REPLY;
					SendMsg.val[0] = group;
					SendMsg.val[1] = (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					send_msg(MODULE_TO_COA1+group, (char *)&SendMsg);
					myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]++;
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
						>= MAX_EOL_DATA) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX] = 0;
					}
					myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P0;
					break;
			}
			break;
		case P45: //All_Module_Temp(1564)
			switch(myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]) {
				case P0:
					break;
				case P1:
					myPs->signal[M_SIG_EOL_PROCEDURE_CHECK_INDEX] = 0;
					myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]++;
					break;
				case P2:
					data_index = (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					memset((char *)&myPs->eol_data[data_index].index,
						0, sizeof(S_EOL_DATA));

					memset((char *)&myData->CAN.uds_rx[can_ch1].uc_val[0], 0, 8);

					check_index = (int)0xDAA2
						+ (int)myPs->signal[M_SIG_EOL_PROCEDURE_CHECK_INDEX];

					myData->CAN.uds_tx[can_ch1].uc_val[0] = 0x03;
					myData->CAN.uds_tx[can_ch1].uc_val[1] = 0x22;
					myData->CAN.uds_tx[can_ch1].uc_val[2]
						= (unsigned char)((check_index >> 8) & 0x000000FF);
					myData->CAN.uds_tx[can_ch1].uc_val[3]
						= (unsigned char)(check_index & 0x000000FF);
					myData->CAN.uds_tx[can_ch1].uc_val[4] = 0x00;
					myData->CAN.uds_tx[can_ch1].uc_val[5] = 0x00;
					myData->CAN.uds_tx[can_ch1].uc_val[6] = 0x00;
					myData->CAN.uds_tx[can_ch1].uc_val[7] = 0x00;

					myPs->signal[M_SIG_EOL_PROCEDURE_CHECK_INDEX]++;

					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] = 0;
					myData->CAN.signal[can_ch1][CAN_SIG_TX_PHASE_EOL_DATA2] = 14;
					myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]++;
					break;
				case P3:
					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1]++;
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] >= 100) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2]++;
					}
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] == 1) {
						myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P101;
					}
					break;
				case P4:
					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1]++;
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] >= 100) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2]++;
					}
					//if(myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] == 1) {
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] <= 10) {
						break;
					}

					data_index = (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					myPs->eol_data[data_index].index
						= (unsigned char)((int)myPs->signal
						[M_SIG_EOL_PROCEDURE_INDEX]
						+ (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DIVISION] * 100);
					myPs->eol_data[data_index].sub_index
						= myPs->signal[M_SIG_EOL_PROCEDURE_CHECK_INDEX];
					myPs->eol_data[data_index].response = 0; //ok

					memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
					SendMsg.msg = MSG_MODULE_COA_EOL_PROCEDURE_REPLY;
					SendMsg.val[0] = group;
					SendMsg.val[1] = (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					send_msg(MODULE_TO_COA1+group, (char *)&SendMsg);
					myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]++;
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
						>= MAX_EOL_DATA) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX] = 0;
					}

					if(myPs->signal[M_SIG_EOL_PROCEDURE_CHECK_INDEX] == 20) {
						myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P0;
					} else {
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P2;
					}
					break;
				default:
					data_index = (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					myPs->eol_data[data_index].index
						= (unsigned char)((int)myPs->signal
						[M_SIG_EOL_PROCEDURE_INDEX]
						+ (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DIVISION] * 100);
					myPs->eol_data[data_index].sub_index
						= (unsigned char)myPs->
						signal[M_SIG_EOL_PROCEDURE_SUB_INDEX];
					myPs->eol_data[data_index].response
						= (short int)myPs->signal
						[M_SIG_EOL_PROCEDURE_PHASE] - 100; //fail

					memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
					SendMsg.msg = MSG_MODULE_COA_EOL_PROCEDURE_REPLY;
					SendMsg.val[0] = group;
					SendMsg.val[1] = (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					send_msg(MODULE_TO_COA1+group, (char *)&SendMsg);
					myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]++;
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
						>= MAX_EOL_DATA) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX] = 0;
					}
					myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P0;
					break;
			}
			break;
		case P46: //Set_Pump1_PWM(1565)
			switch(myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]) {
				case P0:
					break;
				case P1:
					data_index = (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					memset((char *)&myPs->eol_data[data_index].index,
						0, sizeof(S_EOL_DATA));

					memset((char *)&myData->CAN.uds_rx[can_ch1].uc_val[0], 0, 8);

					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] = 0;
					myData->CAN.signal[can_ch1][CAN_SIG_TX_PHASE_EOL_DATA2] = 15;
					myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]++;
					break;
				case P2:
					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1]++;
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] >= 100) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2]++;
					}
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] == 1) {
						myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P101;
					}
					break;
				case P3:
					data_index = (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					myPs->eol_data[data_index].index
						= (unsigned char)((int)myPs->signal
						[M_SIG_EOL_PROCEDURE_INDEX]
						+ (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DIVISION] * 100);
					myPs->eol_data[data_index].sub_index
						= (unsigned char)myPs->
						signal[M_SIG_EOL_PROCEDURE_SUB_INDEX];
					myPs->eol_data[data_index].response = 0; //ok

					memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
					SendMsg.msg = MSG_MODULE_COA_EOL_PROCEDURE_REPLY;
					SendMsg.val[0] = group;
					SendMsg.val[1] = (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					send_msg(MODULE_TO_COA1+group, (char *)&SendMsg);
					myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]++;
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
						>= MAX_EOL_DATA) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX] = 0;
					}
					myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P0;
					break;
				default:
					data_index = (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					myPs->eol_data[data_index].index
						= (unsigned char)((int)myPs->signal
						[M_SIG_EOL_PROCEDURE_INDEX]
						+ (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DIVISION] * 100);
					myPs->eol_data[data_index].sub_index
						= (unsigned char)myPs->
						signal[M_SIG_EOL_PROCEDURE_SUB_INDEX];
					myPs->eol_data[data_index].response
						= (short int)myPs->signal
						[M_SIG_EOL_PROCEDURE_PHASE] - 100; //fail

					memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
					SendMsg.msg = MSG_MODULE_COA_EOL_PROCEDURE_REPLY;
					SendMsg.val[0] = group;
					SendMsg.val[1] = (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					send_msg(MODULE_TO_COA1+group, (char *)&SendMsg);
					myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]++;
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
						>= MAX_EOL_DATA) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX] = 0;
					}
					myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P0;
					break;
			}
			break;
		case P48: //Set_Pump2_PWM(1567)
			switch(myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]) {
				case P0:
					break;
				case P1:
					data_index = (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					memset((char *)&myPs->eol_data[data_index].index,
						0, sizeof(S_EOL_DATA));

					memset((char *)&myData->CAN.uds_rx[can_ch1].uc_val[0], 0, 8);

					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] = 0;
					myData->CAN.signal[can_ch1][CAN_SIG_TX_PHASE_EOL_DATA2] = 17;
					myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]++;
					break;
				case P2:
					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1]++;
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] >= 100) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2]++;
					}
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] == 1) {
						myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P101;
					}
					break;
				case P3:
					data_index = (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					myPs->eol_data[data_index].index
						= (unsigned char)((int)myPs->signal
						[M_SIG_EOL_PROCEDURE_INDEX]
						+ (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DIVISION] * 100);
					myPs->eol_data[data_index].sub_index
						= (unsigned char)myPs->
						signal[M_SIG_EOL_PROCEDURE_SUB_INDEX];
					myPs->eol_data[data_index].response = 0; //ok

					memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
					SendMsg.msg = MSG_MODULE_COA_EOL_PROCEDURE_REPLY;
					SendMsg.val[0] = group;
					SendMsg.val[1] = (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					send_msg(MODULE_TO_COA1+group, (char *)&SendMsg);
					myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]++;
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
						>= MAX_EOL_DATA) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX] = 0;
					}
					myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P0;
					break;
				default:
					data_index = (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					myPs->eol_data[data_index].index
						= (unsigned char)((int)myPs->signal
						[M_SIG_EOL_PROCEDURE_INDEX]
						+ (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DIVISION] * 100);
					myPs->eol_data[data_index].sub_index
						= (unsigned char)myPs->
						signal[M_SIG_EOL_PROCEDURE_SUB_INDEX];
					myPs->eol_data[data_index].response
						= (short int)myPs->signal
						[M_SIG_EOL_PROCEDURE_PHASE] - 100; //fail

					memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
					SendMsg.msg = MSG_MODULE_COA_EOL_PROCEDURE_REPLY;
					SendMsg.val[0] = group;
					SendMsg.val[1] = (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					send_msg(MODULE_TO_COA1+group, (char *)&SendMsg);
					myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]++;
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
						>= MAX_EOL_DATA) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX] = 0;
					}
					myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P0;
					break;
			}
			break;
		case P50: //Set_Valve_On(1569)
			switch(myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]) {
				case P0:
					break;
				case P1:
					data_index = (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					memset((char *)&myPs->eol_data[data_index].index,
						0, sizeof(S_EOL_DATA));

					memset((char *)&myData->CAN.uds_rx[can_ch1].uc_val[0], 0, 8);

					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] = 0;
					myData->CAN.signal[can_ch1][CAN_SIG_TX_PHASE_EOL_DATA2] = 19;
					myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]++;
					break;
				case P2:
					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1]++;
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] >= 100) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2]++;
					}
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] == 1) {
						myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P101;
					}
					break;
				case P3:
					data_index = (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					myPs->eol_data[data_index].index
						= (unsigned char)((int)myPs->signal
						[M_SIG_EOL_PROCEDURE_INDEX]
						+ (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DIVISION] * 100);
					myPs->eol_data[data_index].sub_index
						= (unsigned char)myPs->
						signal[M_SIG_EOL_PROCEDURE_SUB_INDEX];
					myPs->eol_data[data_index].response = 0; //ok

					memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
					SendMsg.msg = MSG_MODULE_COA_EOL_PROCEDURE_REPLY;
					SendMsg.val[0] = group;
					SendMsg.val[1] = (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					send_msg(MODULE_TO_COA1+group, (char *)&SendMsg);
					myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]++;
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
						>= MAX_EOL_DATA) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX] = 0;
					}
					myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P0;
					break;
				default:
					data_index = (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					myPs->eol_data[data_index].index
						= (unsigned char)((int)myPs->signal
						[M_SIG_EOL_PROCEDURE_INDEX]
						+ (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DIVISION] * 100);
					myPs->eol_data[data_index].sub_index
						= (unsigned char)myPs->
						signal[M_SIG_EOL_PROCEDURE_SUB_INDEX];
					myPs->eol_data[data_index].response
						= (short int)myPs->signal
						[M_SIG_EOL_PROCEDURE_PHASE] - 100; //fail

					memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
					SendMsg.msg = MSG_MODULE_COA_EOL_PROCEDURE_REPLY;
					SendMsg.val[0] = group;
					SendMsg.val[1] = (int)myPs->signal
						[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					send_msg(MODULE_TO_COA1+group, (char *)&SendMsg);
					myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]++;
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
						>= MAX_EOL_DATA) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX] = 0;
					}
					myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P0;
					break;
			}
			break;
		case P52: //NORMAL_MODE(1571)
			switch(myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]) {
				case P0:
					break;
				case P1:
					data_index
						= (int)myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					memset((char *)&myPs->eol_data[data_index].index, 0,
						sizeof(S_EOL_DATA));

					memset((char *)&myData->CAN.uds_rx[can_ch1].uc_val[0], 0, 8);

					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] = 0;
					myData->CAN.signal[can_ch1][CAN_SIG_TX_PHASE_EOL_DATA2] = 21;
					myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]++;
					break;
				case P2:
					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1]++;
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] >= 100) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2]++;
					}
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] == 1) {
						myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P101;
					}
					break;
				case P3:
					data_index
						= (int)myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					myPs->eol_data[data_index].index
						= (unsigned char)((int)myPs->signal
						[M_SIG_EOL_PROCEDURE_INDEX]
						+ (int)myPs->signal[M_SIG_EOL_PROCEDURE_DIVISION]
						* 100);
					myPs->eol_data[data_index].sub_index = (unsigned char)myPs->
						signal[M_SIG_EOL_PROCEDURE_SUB_INDEX];
					myPs->eol_data[data_index].response = 0; //ok

					memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
					SendMsg.msg = MSG_MODULE_COA_EOL_PROCEDURE_REPLY;
					SendMsg.val[0] = group;
					SendMsg.val[1]
						= (int)myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					send_msg(MODULE_TO_COA1+group, (char *)&SendMsg);
					myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]++;
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
						>= MAX_EOL_DATA) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX] = 0;
					}
					myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P0;
					break;
				default:
					data_index
						= (int)myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					myPs->eol_data[data_index].index
						= (unsigned char)((int)myPs->signal
						[M_SIG_EOL_PROCEDURE_INDEX]
						+ (int)myPs->signal[M_SIG_EOL_PROCEDURE_DIVISION]
						* 100);
					myPs->eol_data[data_index].sub_index = (unsigned char)myPs->
						signal[M_SIG_EOL_PROCEDURE_SUB_INDEX];
					myPs->eol_data[data_index].response
						= (short int)myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]
						- 100; //fail

					memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
					SendMsg.msg = MSG_MODULE_COA_EOL_PROCEDURE_REPLY;
					SendMsg.val[0] = group;
					SendMsg.val[1]
						= (int)myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					send_msg(MODULE_TO_COA1+group, (char *)&SendMsg);
					myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]++;
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
						>= MAX_EOL_DATA) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX] = 0;
					}
					myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P0;
					break;
			}
			break;
		case P53: //CIE_VOLTAGE(1572)
			switch(myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]) {
				case P0:
					break;
				case P1:
					data_index
						= (int)myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					memset((char *)&myPs->eol_data[data_index].index, 0,
						sizeof(S_EOL_DATA));

					memset((char *)&myData->CAN.uds_rx[can_ch1].uc_val[0], 0, 8);

					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] = 0;
					myData->CAN.signal[can_ch1][CAN_SIG_TX_PHASE_EOL_DATA2] = 22;
					myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]++;
					break;
				case P2:
					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1]++;
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] >= 100) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2]++;
					}
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] == 1) {
						myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P101;
					}
					break;
				case P3:
					data_index
						= (int)myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					myPs->eol_data[data_index].index
						= (unsigned char)((int)myPs->signal
						[M_SIG_EOL_PROCEDURE_INDEX]
						+ (int)myPs->signal[M_SIG_EOL_PROCEDURE_DIVISION]
						* 100);
					myPs->eol_data[data_index].sub_index = (unsigned char)myPs->
						signal[M_SIG_EOL_PROCEDURE_SUB_INDEX];
					myPs->eol_data[data_index].response = 0; //ok

					memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
					SendMsg.msg = MSG_MODULE_COA_EOL_PROCEDURE_REPLY;
					SendMsg.val[0] = group;
					SendMsg.val[1]
						= (int)myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					send_msg(MODULE_TO_COA1+group, (char *)&SendMsg);
					myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]++;
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
						>= MAX_EOL_DATA) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX] = 0;
					}
					myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P0;
					break;
				default:
					data_index
						= (int)myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					myPs->eol_data[data_index].index
						= (unsigned char)((int)myPs->signal
						[M_SIG_EOL_PROCEDURE_INDEX]
						+ (int)myPs->signal[M_SIG_EOL_PROCEDURE_DIVISION]
						* 100);
					myPs->eol_data[data_index].sub_index = (unsigned char)myPs->
						signal[M_SIG_EOL_PROCEDURE_SUB_INDEX];
					myPs->eol_data[data_index].response
						= (short int)myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]
						- 100; //fail

					memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
					SendMsg.msg = MSG_MODULE_COA_EOL_PROCEDURE_REPLY;
					SendMsg.val[0] = group;
					SendMsg.val[1]
						= (int)myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					send_msg(MODULE_TO_COA1+group, (char *)&SendMsg);
					myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]++;
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
						>= MAX_EOL_DATA) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX] = 0;
					}
					myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P0;
					break;
			}
			break;
		case P54: //Pack_Part_Number(1464) kjg_120817
			switch(myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]) {
				case P0:
					break;
				case P1:
					data_index
						= (int)myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					memset((char *)&myPs->eol_data[data_index].index, 0,
						sizeof(S_EOL_DATA));

					memset((char *)&myData->CAN.uds_rx[can_ch1].uc_val[0], 0, 8);

					myData->CAN.uds_rx_message_count = 0;
					myData->CAN.uds_rx_dtc_count = 0;

					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] = 0;
					myPs->signal[M_SIG_BMS_EOL_FLASH_PHASE] = P110;
					myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]++;
					break;
				case P2:
					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1]++;
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] >= 100) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2]++;
					}
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] == 5) {
						myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P102;
					}
					break;
				case P3:
					data_index
						= (int)myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					myPs->eol_data[data_index].index
						= (unsigned char)((int)myPs->signal
						[M_SIG_EOL_PROCEDURE_INDEX]
						+ (int)myPs->signal[M_SIG_EOL_PROCEDURE_DIVISION]
						* 100);
					myPs->eol_data[data_index].sub_index = (unsigned char)myPs->
						signal[M_SIG_EOL_PROCEDURE_SUB_INDEX];
					myPs->eol_data[data_index].response = 0; //ok

					memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
					SendMsg.msg = MSG_MODULE_COA_EOL_PROCEDURE_REPLY;
					SendMsg.val[0] = group;
					SendMsg.val[1]
						= (int)myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					send_msg(MODULE_TO_COA1+group, (char *)&SendMsg);
					myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]++;
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
						>= MAX_EOL_DATA) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX] = 0;
					}
					myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P0;
					break;
				default:
					data_index
						= (int)myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					myPs->eol_data[data_index].index
						= (unsigned char)((int)myPs->signal
						[M_SIG_EOL_PROCEDURE_INDEX]
						+ (int)myPs->signal[M_SIG_EOL_PROCEDURE_DIVISION]
						* 100);
					myPs->eol_data[data_index].sub_index = (unsigned char)myPs->
						signal[M_SIG_EOL_PROCEDURE_SUB_INDEX];
					myPs->eol_data[data_index].response
						= (short int)myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]
						- 100; //fail

					memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
					SendMsg.msg = MSG_MODULE_COA_EOL_PROCEDURE_REPLY;
					SendMsg.val[0] = group;
					SendMsg.val[1]
						= (int)myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					send_msg(MODULE_TO_COA1+group, (char *)&SendMsg);
					myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]++;
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
						>= MAX_EOL_DATA) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX] = 0;
					}
					myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P0;
					break;
			}
			break;
		case P55: //DISABlE_DIAG(1573) //kjg_120921
			switch(myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]) {
				case P0:
					break;
				case P1:
					data_index
						= (int)myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					memset((char *)&myPs->eol_data[data_index].index, 0,
						sizeof(S_EOL_DATA));

					memset((char *)&myData->CAN.uds_rx[can_ch1].uc_val[0], 0, 8);

					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] = 0;
					myData->CAN.signal[can_ch1][CAN_SIG_TX_PHASE_EOL_DATA2] = 23;
					myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]++;
					break;
				case P2:
					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1]++;
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] >= 100) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2]++;
					}
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] == 1) {
						myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P101;
					}
					break;
				case P3:
					data_index
						= (int)myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					myPs->eol_data[data_index].index
						= (unsigned char)((int)myPs->signal
						[M_SIG_EOL_PROCEDURE_INDEX]
						+ (int)myPs->signal[M_SIG_EOL_PROCEDURE_DIVISION]
						* 100);
					myPs->eol_data[data_index].sub_index = (unsigned char)myPs->
						signal[M_SIG_EOL_PROCEDURE_SUB_INDEX];
					myPs->eol_data[data_index].response = 0; //ok

					memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
					SendMsg.msg = MSG_MODULE_COA_EOL_PROCEDURE_REPLY;
					SendMsg.val[0] = group;
					SendMsg.val[1]
						= (int)myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					send_msg(MODULE_TO_COA1+group, (char *)&SendMsg);
					myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]++;
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
						>= MAX_EOL_DATA) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX] = 0;
					}
					myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P0;
					break;
				default:
					data_index
						= (int)myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					myPs->eol_data[data_index].index
						= (unsigned char)((int)myPs->signal
						[M_SIG_EOL_PROCEDURE_INDEX]
						+ (int)myPs->signal[M_SIG_EOL_PROCEDURE_DIVISION]
						* 100);
					myPs->eol_data[data_index].sub_index = (unsigned char)myPs->
						signal[M_SIG_EOL_PROCEDURE_SUB_INDEX];
					myPs->eol_data[data_index].response
						= (short int)myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]
						- 100; //fail

					memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
					SendMsg.msg = MSG_MODULE_COA_EOL_PROCEDURE_REPLY;
					SendMsg.val[0] = group;
					SendMsg.val[1]
						= (int)myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					send_msg(MODULE_TO_COA1+group, (char *)&SendMsg);
					myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]++;
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
						>= MAX_EOL_DATA) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX] = 0;
					}
					myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P0;
					break;
			}
			break;
		case P56: //ISO_RESULT_WRITE(1574) //kjg_121025
			switch(myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]) {
				case P0:
					break;
				case P1:
					data_index
						= (int)myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					memset((char *)&myPs->eol_data[data_index].index, 0,
						sizeof(S_EOL_DATA));

					memset((char *)&myData->CAN.uds_rx[can_ch1].uc_val[0], 0, 8);

					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] = 0;
					myData->CAN.signal[can_ch1][CAN_SIG_TX_PHASE_EOL_DATA2] = 24;
					myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]++;
					break;
				case P2:
					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1]++;
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] >= 100) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2]++;
					}
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] == 1) {
						myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P101;
					}
					break;
				case P3:
					data_index
						= (int)myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					myPs->eol_data[data_index].index
						= (unsigned char)((int)myPs->signal
						[M_SIG_EOL_PROCEDURE_INDEX]
						+ (int)myPs->signal[M_SIG_EOL_PROCEDURE_DIVISION]
						* 100);
					myPs->eol_data[data_index].sub_index = (unsigned char)myPs->
						signal[M_SIG_EOL_PROCEDURE_SUB_INDEX];
					myPs->eol_data[data_index].response = 0; //ok

					memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
					SendMsg.msg = MSG_MODULE_COA_EOL_PROCEDURE_REPLY;
					SendMsg.val[0] = group;
					SendMsg.val[1]
						= (int)myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					send_msg(MODULE_TO_COA1+group, (char *)&SendMsg);
					myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]++;
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
						>= MAX_EOL_DATA) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX] = 0;
					}
					myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P0;
					break;
				default:
					data_index
						= (int)myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					myPs->eol_data[data_index].index
						= (unsigned char)((int)myPs->signal
						[M_SIG_EOL_PROCEDURE_INDEX]
						+ (int)myPs->signal[M_SIG_EOL_PROCEDURE_DIVISION]
						* 100);
					myPs->eol_data[data_index].sub_index = (unsigned char)myPs->
						signal[M_SIG_EOL_PROCEDURE_SUB_INDEX];
					myPs->eol_data[data_index].response
						= (short int)myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]
						- 100; //fail

					memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
					SendMsg.msg = MSG_MODULE_COA_EOL_PROCEDURE_REPLY;
					SendMsg.val[0] = group;
					SendMsg.val[1]
						= (int)myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					send_msg(MODULE_TO_COA1+group, (char *)&SendMsg);
					myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]++;
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
						>= MAX_EOL_DATA) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX] = 0;
					}
					myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P0;
					break;
			}
			break;
		case P57: //ISO_RESULT_CHECK(1575) //kjg_121025
			switch(myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]) {
				case P0:
					break;
				case P1:
					data_index
						= (int)myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					memset((char *)&myPs->eol_data[data_index].index, 0,
						sizeof(S_EOL_DATA));

					memset((char *)&myData->CAN.uds_rx[can_ch1].uc_val[0], 0, 8);

					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] = 0;
					myData->CAN.signal[can_ch1][CAN_SIG_TX_PHASE_EOL_DATA2] = 25;
					myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]++;
					break;
				case P2:
					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1]++;
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] >= 100) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2]++;
					}
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] == 1) {
						myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P101;
					}
					break;
				case P3:
					data_index
						= (int)myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					myPs->eol_data[data_index].index
						= (unsigned char)((int)myPs->signal
						[M_SIG_EOL_PROCEDURE_INDEX]
						+ (int)myPs->signal[M_SIG_EOL_PROCEDURE_DIVISION]
						* 100);
					myPs->eol_data[data_index].sub_index = (unsigned char)myPs->
						signal[M_SIG_EOL_PROCEDURE_SUB_INDEX];
					myPs->eol_data[data_index].response = 0; //ok

					memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
					SendMsg.msg = MSG_MODULE_COA_EOL_PROCEDURE_REPLY;
					SendMsg.val[0] = group;
					SendMsg.val[1]
						= (int)myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					send_msg(MODULE_TO_COA1+group, (char *)&SendMsg);
					myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]++;
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
						>= MAX_EOL_DATA) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX] = 0;
					}
					myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P0;
					break;
				default:
					data_index
						= (int)myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					myPs->eol_data[data_index].index
						= (unsigned char)((int)myPs->signal
						[M_SIG_EOL_PROCEDURE_INDEX]
						+ (int)myPs->signal[M_SIG_EOL_PROCEDURE_DIVISION]
						* 100);
					myPs->eol_data[data_index].sub_index = (unsigned char)myPs->
						signal[M_SIG_EOL_PROCEDURE_SUB_INDEX];
					myPs->eol_data[data_index].response
						= (short int)myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]
						- 100; //fail

					memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
					SendMsg.msg = MSG_MODULE_COA_EOL_PROCEDURE_REPLY;
					SendMsg.val[0] = group;
					SendMsg.val[1]
						= (int)myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					send_msg(MODULE_TO_COA1+group, (char *)&SendMsg);
					myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]++;
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
						>= MAX_EOL_DATA) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX] = 0;
					}
					myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P0;
					break;
			}
			break;
		case P58: //DUMP_NVM_WRITE(1576) //kjg_121025
			switch(myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]) {
				case P0:
					break;
				case P1:
					data_index
						= (int)myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					memset((char *)&myPs->eol_data[data_index].index, 0,
						sizeof(S_EOL_DATA));

					memset((char *)&myData->CAN.uds_rx[can_ch1].uc_val[0], 0, 8);

					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] = 0;
					myData->CAN.signal[can_ch1][CAN_SIG_TX_PHASE_EOL_DATA2] = 26;
					myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]++;
					break;
				case P2:
					myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1]++;
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] >= 100) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2]++;
					}
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] == 1) {
						myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P101;
					}
					break;
				case P3:
					data_index
						= (int)myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					myPs->eol_data[data_index].index
						= (unsigned char)((int)myPs->signal
						[M_SIG_EOL_PROCEDURE_INDEX]
						+ (int)myPs->signal[M_SIG_EOL_PROCEDURE_DIVISION]
						* 100);
					myPs->eol_data[data_index].sub_index = (unsigned char)myPs->
						signal[M_SIG_EOL_PROCEDURE_SUB_INDEX];
					myPs->eol_data[data_index].response = 0; //ok

					memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
					SendMsg.msg = MSG_MODULE_COA_EOL_PROCEDURE_REPLY;
					SendMsg.val[0] = group;
					SendMsg.val[1]
						= (int)myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					send_msg(MODULE_TO_COA1+group, (char *)&SendMsg);
					myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]++;
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
						>= MAX_EOL_DATA) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX] = 0;
					}
					myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P0;
					break;
				default:
					data_index
						= (int)myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					myPs->eol_data[data_index].index
						= (unsigned char)((int)myPs->signal
						[M_SIG_EOL_PROCEDURE_INDEX]
						+ (int)myPs->signal[M_SIG_EOL_PROCEDURE_DIVISION]
						* 100);
					myPs->eol_data[data_index].sub_index = (unsigned char)myPs->
						signal[M_SIG_EOL_PROCEDURE_SUB_INDEX];
					myPs->eol_data[data_index].response
						= (short int)myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]
						- 100; //fail

					memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
					SendMsg.msg = MSG_MODULE_COA_EOL_PROCEDURE_REPLY;
					SendMsg.val[0] = group;
					SendMsg.val[1]
						= (int)myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					send_msg(MODULE_TO_COA1+group, (char *)&SendMsg);
					myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]++;
					if(myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX]
						>= MAX_EOL_DATA) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX] = 0;
					}
					myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P0;
					break;
			}
			break;
		default:
			break;
	}
}

