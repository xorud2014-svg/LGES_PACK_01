#include "../../INC/datastore.h"
#include "local_utils.h"
#include "local_message.h"
#include "COM.h"
#include "Analog.h"
#include "ChannelControl.h"
#include "BoardControl.h"
#include "GroupControl.h"

extern S_SYSTEM_DATA *myData;
extern S_MODULE_DATA *myPs;
extern S_GROUP_DATA	*myGroup;

void GroupControl(int group_div)
{
	int group, start_group, end_group;

	if(group_div == MAX_GROUP_4) {
		start_group = 0;
		end_group = myData->AppControl.config.totalGroup;
	} else {
		start_group = group_div;
		end_group = group_div + 1;
	}

	for(group=start_group; group < end_group; group++) {
		if(group >= myData->AppControl.config.totalGroup) continue;

		myGroup = &(myData->gData[group]);
	
		gSignalCheck(group);
	
		switch(myGroup->state) {
			case G_IDLE: 	gIdle(group); 		break;
			case G_STANDBY:	gStandby(group);	break;
			case G_RUN:		gRun(group);		break;
			case G_PAUSE:	gPause(group);		break;
			case G_CALI:	gCali(group);		break;
			default: break;
		}
	}
}

void gIdle(int group)
{
	memset((char *)&myGroup->cmd, 0, sizeof(unsigned char) * MAX_CMD_MODE);
	memset((char *)&myGroup->signal, 0, sizeof(unsigned char) * MAX_SIGNAL);

	myGroup->signal[G_SIG_SAFETY_RESISTOR] = SR_OPEN;

	myGroup->code = G_CD_NONE;
	myGroup->state = G_STANDBY;
	myGroup->phase = P0;
}

void gStandby(int group)
{
	if(myData->AppControl.misc.Load_Process[PROCESS_GROUP_COB][0] == P1) {
	} else {
		gStandby_C(group);
	}
}

void gRun(int group)
{
	if(myData->AppControl.misc.Load_Process[PROCESS_GROUP_COB][0] == P1) {
	} else {
		gRun_C(group);
	}
}

void gPause(int group)
{
	if(myData->AppControl.misc.Load_Process[PROCESS_GROUP_COB][0] == P1) {
	} else {
		gPause_C(group);
	}
}

void gCali(int group)
{
	int ch, cnt1;


	cnt1 = 0;
	for(ch=myGroup->misc.chOffset; ch < (myGroup->misc.chOffset
		+ myPs->config.chInGroup[group]); ch++) {
		if(myData->cData[ch].op.state == C_STANDBY
			|| myData->cData[ch].op.state == C_IDLE) cnt1++;
	}
	if(cnt1 >= myPs->config.chInGroup[group]) {
		myGroup->state = G_STANDBY;
		myGroup->phase = P0;
	}
}

void gStandby_C(int group)
{
	int ch;

	for(ch=myGroup->misc.chOffset;
		ch < (myGroup->misc.chOffset + myPs->config.chInGroup[group]); ch++) {
		if(myData->cData[ch].op.state == C_RUN) {
			myGroup->state = G_RUN;
			myGroup->phase = P0;
		}
	}

	for(ch=myGroup->misc.chOffset;
		ch < (myGroup->misc.chOffset + myPs->config.chInGroup[group]); ch++) {
		if(myData->cData[ch].op.state == C_CALI) {
			myGroup->state = G_CALI;
			myGroup->phase = P0;
		}
	}
}

void gRun_C(int group)
{
	int ch, cnt1, cnt2;
	long diff, time1, time2;
	S_MSG_VAL SendMsg;

	cnt1 = cnt2 = 0;
	for(ch=myGroup->misc.chOffset;
		ch < (myGroup->misc.chOffset + myPs->config.chInGroup[group]); ch++) {
		if(myData->cData[ch].op.state == C_STANDBY
			|| myData->cData[ch].op.state == C_IDLE) cnt1++;
		if(myData->cData[ch].op.state == C_PAUSE) cnt2++;
	}
	if(cnt1 >= myPs->config.chInGroup[group]) {
		myGroup->state = G_STANDBY;
		myGroup->phase = P0;
	} else if((cnt1+cnt2) >= myPs->config.chInGroup[group]) {
		myGroup->state = G_PAUSE;
		myGroup->phase = P0;
	}

	time1 = myPs->misc.timer_1sec;
	time2 = myPs->misc.timer_1000ms;
	diff = (time1 - myData->COA_Client[group].misc.net_time) * 1000;
	diff += (time2 - myData->COA_Client[group].misc.net_time2);
	//if((diff >= myData->COA_Client[group].config.netTimeout || diff < 0)
	if((diff >= myData->COA_Client[group].config.netTimeout)	//csk_200506d
		&& myGroup->signal[G_SIG_NET_CHECK] == P0) {
		if(myData->COA_Client[group].config.state_change == 1) {
			for(ch=myGroup->misc.chOffset; ch <
				(myGroup->misc.chOffset + myPs->config.chInGroup[group]);
				ch++) {
				//if(myData->cData[ch].op.state == C_RUN) {
				//	myData->cData[ch].signal[C_SIG_MODULE_FAULT] = P1;
				//}
				if((myData->cData[ch].op.state == C_RUN)
					&& (myData->cData[ch].signal[C_SIG_MODULE_FAULT] == P0)
					&& (myData->ChAttribute[ch].chiller_control == 1)) {
					myData->cData[ch].signal[C_SIG_MODULE_FAULT] = P1;
				}
			}
		} else if(myData->COA_Client[group].config.state_change == 2) {
			if(myData->COA_Client[group].signal[P1_SIG_NET_CONNECTED] == P1) {
				myData->COA_Client[group].signal[P1_SIG_NET_CONNECTED] = P0;
				myData->save_msg[0].send_flag = 1; //send save_msg stop

				memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
				SendMsg.msg = MSG_MODULE_APP_PROCESS_KILL;
				SendMsg.val[0] = 0;
				SendMsg.val[1] = 0;
				send_msg(MODULE_TO_APP, (char *)&SendMsg);
			}
		}
	} else {
		if(myData->COA_Client[group].signal[P1_SIG_NET_CONNECTED] == P1) {
			if(myData->COA_Client[group].config.state_change == 2) {
				myData->save_msg[0].send_flag = 0; //send save_msg continue 
			}
		}
	}
}

void gPause_C(int group)
{
	int ch, cnt1, cnt2;

	cnt1 = cnt2 = 0;
	for(ch=myGroup->misc.chOffset;
		ch < (myGroup->misc.chOffset + myPs->config.chInGroup[group]); ch++) {
		if(myData->cData[ch].op.state == C_STANDBY) cnt1++;
		if(myData->cData[ch].op.state == C_RUN) cnt2++;
	}

	if(cnt1 >= myPs->config.chInGroup[group]) {
		myGroup->state = G_STANDBY;
		myGroup->phase = P0;
	} else if(cnt2 != 0) {
		myGroup->state = G_RUN;
		myGroup->phase = P0;
	}
}

void gStandby_F(int group)
{
#ifdef __COB__
#endif
}

void gRun_F1(int group)
{
#ifdef __COB__

#endif
}

void gRun_F2(int group)
{ //debug_size_cob

}

void gRun_F3(int group)
{ //debug_size_cob

}

void gRun_F4(int group)
{ //debug_size_cob

}

void gRun_F_SDI_5V_400A_200A_100A_10A(int group)
{ //debug_size_cob

}

void gRun_F_SDI_5V_400A_200A_100A_10A_2(int group)
{ //debug_size_cob

}

void gRun_F_SDI_5V_450A_200A_100A_10A(int group)
{ //debug_size_cob

}

void gPause_F1(int group)
{ //debug_size_cob

}

void gPause_F2(int group)
{ //debug_size_cob

}

void gPause_F3(int group)
{ //debug_size_cob

}

void gPause_F4(int group)
{ //debug_size_cob

}

void gSignalCheck(int group)
{
	if(myData->AppControl.misc.Load_Process[PROCESS_GROUP_COB][0] == P1) {
	} else {
		gSignalCheck_C(group);
	}
}

void gSignalCheck_C(int group)
{
	int toPs1, ch, cnt;
	S_MSG_VAL SendMsg;

	if(myGroup->signal[G_SIG_EXIT_PHASE] == P1) {
		if(myPs->signal[M_SIG_EXIT_TYPE] == P0) {
			cnt = 0;
		} else {
			cnt = 1;
		}

		if(cnt == 1) {
			for(ch=myGroup->misc.chOffset;
				ch < (myGroup->misc.chOffset
				+ myPs->config.chInGroup[group]); ch++) {
				if(myData->cData[ch].op.state == C_RUN) {
					myData->cData[ch].signal[C_SIG_MODULE_FAULT] = P2;
				}
			}
			myGroup->signal[G_SIG_EXIT_PHASE] = P2;
		} else {
			myGroup->signal[G_SIG_EXIT_PHASE] = P0;
		}

		if(myData->AppControl.misc.Load_Process[PROCESS_GROUP_COA][0] == P1) {
			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
			SendMsg.msg = MSG_MODULE_COA_TROUBLE_CODE;
			SendMsg.val[0] = (int)myGroup->code;
			SendMsg.val[2] = (int)myGroup->signal[G_SIG_EXIT_VALUE];
			toPs1 = MODULE_TO_COA1 + group;
			send_msg(toPs1, (char *)&SendMsg);
		}

		if(myData->AppControl.misc.Load_Process[PROCESS_GROUP_COC][0] == P1) {
			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
			SendMsg.msg = MSG_MODULE_COC_TROUBLE_CODE;
			SendMsg.val[0] = (int)myGroup->code;
			SendMsg.val[2] = (int)myGroup->signal[G_SIG_EXIT_VALUE];
			toPs1 = MODULE_TO_COC1 + group;
			send_msg(toPs1, (char *)&SendMsg);
		}
	} else if(myGroup->signal[G_SIG_EXIT_PHASE] == P2) {
		cnt = 0;
		for(ch=myGroup->misc.chOffset;
			ch < (myGroup->misc.chOffset + myPs->config.chInGroup[group]);
			ch++) {
			if(myData->cData[ch].op.state != C_RUN) cnt++;
		}
		if(cnt >= myPs->config.chInGroup[group]) {
			myGroup->signal[G_SIG_EXIT_PHASE] = P0;
		}
	}
}

void gSignalCheck_F(int group)
{
#ifdef __COB__

#endif
}

void gSendSignal_to_Bd(int group, int signal, int val)
{
	int bd, tmp_group;

	for(bd=0; bd < MAX_BD_PER_MODULE; bd++) {
		tmp_group = bdNo_to_groupNo(bd);
		if(tmp_group > 0) tmp_group--;
		else continue;
		if(group == tmp_group) {
			myData->bData[bd].signal[signal] = (unsigned char)val;
		}
	}
}

void gCompare_Ch_Average_1(int group)
{
#ifdef __COB__

#endif
}

int gCompare_Ch_Average_2(int group)
{ //debug_size_cob
	return 1;
}

void gCellCheck_Compare(int group)
{ //debug_size_cob

}

int gLoadLineCheck(int group, int div)
{
	int rtn=0;
#ifdef __COB__

#endif
	return rtn;
}

void gCali_MainDAC_Default(void)
{
	int group, type, range, point, bd, diff, ch;
	double d_val1=0, d_val2=0;
	S_MSG_VAL SendMsg;

	group = (int)myPs->signal[M_SIG_CALI_GROUP];
	bd = (int)myPs->signal[M_SIG_CALI_BD_START];
	type = (int)myPs->signal[M_SIG_CALI_VI_SELECT];

	range = 0;

	switch(myGroup->phase) {
		case P30: //MainDAC_Calibration
			if(type == P0) {
				myGroup->cali_main_dac.tmp_cali_pointNum[type][range] = 2;
				myGroup->cali_main_dac.tmp_cali_cmd[type][range][0] = 2700000;
				myGroup->cali_main_dac.tmp_cali_cmd[type][range][1] = 4200000;
				myGroup->cali_main_dac.tmp_DA_A[type][range][0] = 1.0;
				myGroup->cali_main_dac.tmp_DA_B[type][range][0] = 0.0;
				myGroup->cali_main_dac.tmp_DA_A[type][range][1] = 1.0;
				myGroup->cali_main_dac.tmp_DA_B[type][range][1] = 0.0;
				myGroup->cali_main_dac.tmp_DA_A[1][range][0] = 1.0;
				myGroup->cali_main_dac.tmp_DA_B[1][range][0] = 0.0;
				myGroup->cali_main_dac.tmp_DA_A[1][range][1] = 1.0;
				myGroup->cali_main_dac.tmp_DA_B[1][range][1] = 0.0;
			} else {
				myGroup->cali_main_dac.tmp_cali_pointNum[type][range] = 2;
				myGroup->cali_main_dac.tmp_cali_cmd[type][range][0] = -8000000;
				myGroup->cali_main_dac.tmp_cali_cmd[type][range][1] = 8000000;
			}

			point = 0;
			d_val1 = (double)myGroup->cali_main_dac
				.tmp_cali_cmd[type][range][point];
			myData->bData[bd].misc.refV = d_val1;
			myData->bData[bd].misc.refI = 5000000;

			myData->bData[bd].signal[B_SIG_CD_SELECT] = P1;
			ch = myPs->config.chPerBd * bd;
			myData->cData[ch].signal[C_SIG_RUN_RELAY_ON] = P1;
			myData->cData[ch].signal[C_SIG_RANGE_RELAY_ON] = P1;

			myPs->misc.cali_delay = myPs->misc.timer_1sec;
			myGroup->phase++;
			break;
		case P31:
			diff = myPs->misc.timer_1sec - myPs->misc.cali_delay;
			if(diff < 2) break;
			myPs->misc.cali_delay = myPs->misc.timer_1sec;

			point = 0;
			ch = myPs->config.chPerBd * bd;
			myData->cData[ch].signal[C_SIG_AUX_DAV] = P20;
			d_val1 = (double)myGroup->cali_main_dac
				.tmp_cali_cmd[type][range][point];
			if(type == P0) {
				gCalCmdV(bd, d_val1, 15, range);
				d_val2 = 5000000;
				gCalCmdI(bd, d_val2, 15, range);
			} else {
				gCalCmdI(bd, d_val1, 15, range);
			}
			myGroup->phase++;
			break;
		case P32:
			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
			SendMsg.msg = MSG_MODULE_METER_REQUEST;
			SendMsg.val[0] = group;
			SendMsg.val[1] = type;
			send_msg(MODULE_TO_METER, (char *)&SendMsg);

			myPs->misc.cali_delay = myPs->misc.timer_1sec;
			myGroup->phase++;
			break;
		case P33: //wait meter1 response
			diff = myPs->misc.timer_1sec - myPs->misc.cali_delay;
			if(diff < 2) break;
			myPs->misc.cali_delay = myPs->misc.timer_1sec;

			point = 0;
			myGroup->cali_main_dac.tmp_org_meter[type][range][point]
				= myPs->misc.meter_value;

			point = 1;
			d_val1 = (double)myGroup->cali_main_dac
				.tmp_cali_cmd[type][range][point];
			myData->bData[bd].misc.refV = d_val1;
			myData->bData[bd].misc.refI = 5000000;

			myGroup->phase++;
			break;
		case P34:
			point = 1;
			ch = myPs->config.chPerBd * bd;
			myData->cData[ch].signal[C_SIG_AUX_DAV] = P20;
			d_val1 = (double)myGroup->cali_main_dac
				.tmp_cali_cmd[type][range][point];
			if(type == P0) {
				gCalCmdV(bd, d_val1, 15, range);
				d_val2 = 5000000;
				gCalCmdI(bd, d_val2, 15, range);
			} else {
				gCalCmdI(bd, d_val1, 15, range);
			}
			myGroup->phase++;
			break;
		case P35:
			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
			SendMsg.msg = MSG_MODULE_METER_REQUEST;
			SendMsg.val[0] = group;
			SendMsg.val[1] = type;
			send_msg(MODULE_TO_METER, (char *)&SendMsg);

			myPs->misc.cali_delay = myPs->misc.timer_1sec;
			myGroup->phase++;
			break;
		case P36: //wait meter1 response
			diff = myPs->misc.timer_1sec - myPs->misc.cali_delay;
			if(diff < 2) break;
			myPs->misc.cali_delay = myPs->misc.timer_1sec;

			type = (int)myPs->signal[M_SIG_CALI_VI_SELECT];
			point = 1; //khkw
			myGroup->cali_main_dac.tmp_org_meter[type][range][point]
				= myPs->misc.meter_value;

			point = 0;
			d_val1 = (double)myGroup->cali_main_dac
				.tmp_cali_cmd[type][range][point];
			myData->bData[bd].misc.refV = d_val1;
			myData->bData[bd].misc.refI = 5000000;

			myGroup->phase++;
			break;
		case P37:
			d_val1 = myGroup->cali_main_dac.tmp_cali_cmd[type][range][1]
				- myGroup->cali_main_dac.tmp_cali_cmd[type][range][0];
			d_val2 = myGroup->cali_main_dac.tmp_org_meter[type][range][1]
				- myGroup->cali_main_dac.tmp_org_meter[type][range][0];
			myGroup->cali_main_dac.tmp_DA_A[type][range][0] = d_val1 / d_val2;
			myGroup->cali_main_dac.tmp_DA_B[type][range][0]
				= (double)myGroup->cali_main_dac.tmp_cali_cmd[type][range][0]
				- (double)myGroup->cali_main_dac.tmp_org_meter[type][range][0]
				* myGroup->cali_main_dac.tmp_DA_A[type][range][0];

			point = 0; //khkw
			ch = myPs->config.chPerBd * bd;
			myData->cData[ch].signal[C_SIG_AUX_DAV] = P20;
			d_val1 = (double)myGroup->cali_main_dac
				.tmp_cali_cmd[type][range][point];
			if(type == P0) { //dav
				gCalCmdV(bd, d_val1, 5, range);
				d_val2 = 5000000;
				gCalCmdI(bd, d_val2, 15, range);
			} else { //dai
				gCalCmdI(bd, d_val1, 5, range);
			}
			myGroup->phase++;
			break;
		case P38:
			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
			SendMsg.msg = MSG_MODULE_METER_REQUEST;
			SendMsg.val[0] = group;
			SendMsg.val[1] = type;
			send_msg(MODULE_TO_METER, (char *)&SendMsg);

			myPs->misc.cali_delay = myPs->misc.timer_1sec;
			myGroup->phase++;
			break;
		case P39: //wait meter1 response
			diff = myPs->misc.timer_1sec - myPs->misc.cali_delay;
			if(diff < 2) break;
			myPs->misc.cali_delay = myPs->misc.timer_1sec;

			point = 0;
			myGroup->cali_main_dac.tmp_cal_meter[type][range][point]
				= myPs->misc.meter_value;

			point = 1;
			d_val1 = (double)myGroup->cali_main_dac
				.tmp_cali_cmd[type][range][point];
			myData->bData[bd].misc.refV = d_val1;
			myData->bData[bd].misc.refI = 5000000;

			myGroup->phase++;
			break;
		case P40:
			point = 1;
			ch = myPs->config.chPerBd * bd;
			myData->cData[ch].signal[C_SIG_AUX_DAV] = P20;
			d_val1 = (double)myGroup->cali_main_dac
				.tmp_cali_cmd[type][range][point];
			if(type == P0) {
				gCalCmdV(bd, d_val1, 5, range);
				d_val2 = 5000000;
				gCalCmdI(bd, d_val2, 15, range);
			} else {
				gCalCmdI(bd, d_val1, 5, range);
			}
			myGroup->phase++;
			break;
		case P41:
			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
			SendMsg.msg = MSG_MODULE_METER_REQUEST;
			SendMsg.val[0] = group;
			SendMsg.val[1] = type;
			send_msg(MODULE_TO_METER, (char *)&SendMsg);
			
			myPs->misc.cali_delay = myPs->misc.timer_1sec;
			myGroup->phase++;
			break;
		case P42: //wait meter1 response
			diff = myPs->misc.timer_1sec - myPs->misc.cali_delay;
			if(diff < 2) break;
			myPs->misc.cali_delay = myPs->misc.timer_1sec;

			point = 1;
			myGroup->cali_main_dac.tmp_cal_meter[type][range][point]
				= myPs->misc.meter_value;

			ch = myPs->config.chPerBd * bd;
			myData->cData[ch].signal[C_SIG_RUN_RELAY_OFF] = P1;
			myData->cData[ch].signal[C_SIG_RANGE_RELAY_OFF] = P1;

			myGroup->phase++;
			break;
		case P43:
			ch = myPs->config.chPerBd * bd;
			myData->cData[ch].signal[C_SIG_AUX_DAV] = P20;
			gCalCmdV(bd, 0, 15, range);
			gCalCmdI(bd, 0, 15, range);

			myGroup->cali_main_dac.tmp_da_caliFlag[type][range] = P1;

			for(point=0; point < myGroup->cali_main_dac
				.tmp_cali_pointNum[type][range]; point++) {
				myData->bData[bd].cali_main_dac.tmp_cali_cmd[type][range][point]
					= myGroup->cali_main_dac.tmp_cali_cmd[type][range][point];
				myData->bData[bd].cali_main_dac
					.tmp_org_meter[type][range][point]
					= myGroup->cali_main_dac.tmp_org_meter[type][range][point];
				myData->bData[bd].cali_main_dac
					.tmp_cal_meter[type][range][point]
					= myGroup->cali_main_dac.tmp_cal_meter[type][range][point];
				myData->bData[bd].cali_main_dac.tmp_cali_pointNum[type][range]
					= myGroup->cali_main_dac.tmp_cali_pointNum[type][range];
			}

			for(point=0; point < myGroup->cali_main_dac
				.tmp_cali_pointNum[type][range]-1; point++) {
				myData->bData[bd].cali_main_dac.tmp_DA_A[type][range][point]
					= myGroup->cali_main_dac.tmp_DA_A[type][range][point];
				myData->bData[bd].cali_main_dac.tmp_DA_B[type][range][point]
					= myGroup->cali_main_dac.tmp_DA_B[type][range][point];
			}

			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
			SendMsg.msg = MSG_MODULE_APP_CALI_DA_END;
			SendMsg.val[0] = group;
			SendMsg.val[1] = type;
			send_msg(MODULE_TO_APP, (char *)&SendMsg);

			myGroup->state = G_STANDBY;
			myGroup->phase = P0;
			break;
		default: break;
	}
}

void gCali_MainDAC_1(void)
{ 
#ifdef __COB__

#endif
}

int	bFindDACaliPoint(int bd, long value, int type, int range)
{
	int point=0;
#ifdef __COB__

#endif
	return point;
}

void gCalCmdV(int bd, long value, int div, int range)
{
#ifdef __COB__
 
#endif
}

void gCalCmdI(int bd, long value, int div, int range)
{
#ifdef __COB__
 
#endif
}

void gCali_bd(void)
{ //debug_size_cob

}

void gCali_bd_initialize(void)
{ //debug_size_cob

}

void gCalculate_bd_CaliData_1(int bd)
{ //debug_size_cob

}

void gCalculate_bd_CaliData_2(int bd)
{ //debug_size_cob

}

void gCali_bd_select(void)
{
#ifdef __COB__

#endif
}

void gCali_bd_cd_select(void)
{ //debug_size_cob

}

void gCali_bd_cmd_output(void)
{ //debug_size_cob

}

void gCali_bd_next_step_check(void)
{ //debug_size_cob

}

void gCali_send_cali_data(int bd, int ch)
{
#ifdef __COB__

#endif
}

void gCali_bd_schedule(void)
{ //debug_size_cob

}

void gCali_bd_next_div_check(void)
{ //debug_size_cob

}
