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
		switch(myData->AppControl.config.systemModel) {
			case F_SDI_5V_50A_5A:
			case F_SDI_5V_400A_200A_100A_10A:
			case F_SDI_5V_400A_200A_100A_10A_2:
			case F_SDI_5V_450A_200A_100A_10A:
			case F_PNE_5V_15A_30AP_SW:
				gStandby_F(group);
				break;
			default:
				break;
		}
	} else {
		gStandby_C(group);
	}
}

void gRun(int group)
{
	if(myData->AppControl.misc.Load_Process[PROCESS_GROUP_COB][0] == P1) {
		switch(myData->AppControl.config.systemModel) {
			case F_SDI_5V_50A_5A:
				gRun_F1(group);
				break;
			case F_SDI_5V_400A_200A_100A_10A:
			case F_SDI_5V_400A_200A_100A_10A_2:
				gRun_F2(group);
				break;
			case F_SDI_5V_450A_200A_100A_10A:
				gRun_F3(group);
				break;
			case F_PNE_5V_15A_30AP_SW:
				gRun_F4(group);
				break;
			default:
				break;
		}
	} else {
		gRun_C(group);
	}
}

void gPause(int group)
{
	if(myData->AppControl.misc.Load_Process[PROCESS_GROUP_COB][0] == P1) {
		switch(myData->AppControl.config.systemModel) {
			case F_SDI_5V_50A_5A:
				gPause_F1(group);
				break;
			case F_SDI_5V_400A_200A_100A_10A:
			case F_SDI_5V_400A_200A_100A_10A_2:
				gPause_F2(group);
				break;
			case F_SDI_5V_450A_200A_100A_10A:
				gPause_F3(group);
				break;
			case F_PNE_5V_15A_30AP_SW:
				gPause_F4(group);
				break;
			default:
				break;
		}
	} else {
		gPause_C(group);
	}
}

void gCali(int group)
{
	int ch, cnt1;

	switch(myData->AppControl.config.systemModel) {
		case F_PNE_5V_15A_30AP_SW:
			if(myGroup->phase < P30) {
				gCali_bd();
			} else {
				if(myPs->signal[M_SIG_CALI_VI_SELECT] == P0) {
					gCali_MainDAC_Default();
				} else {
					gCali_MainDAC_1();
				}
			}
			break;
		default:
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
			break;
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

	//jhkw_190830s
	if(myData->COA_Client[group].signal[P1_SIG_10MS_TO_SBC_RESPONSE_CHECK] == 2) {
		for(ch=myGroup->misc.chOffset; ch <
			(myGroup->misc.chOffset + myPs->config.chInGroup[group]);
			ch++) {
#if defined __COA_VER_100B__ || defined __COA_VER_100B2__ || defined __COA_VER_100C__ || defined __COA_VER_100D__ || defined __COA_VER_100F__ //kjhw_170906
			if((myData->cData[ch].op.state == C_RUN)
				&& (myData->cData[ch].signal[C_SIG_MODULE_FAULT] == P0)) {
				myData->cData[ch].signal[C_SIG_MODULE_FAULT] = P1;
			}
#else //COA_VER_100F01~
			if((myData->cData[ch].op.state == C_RUN)
				&& (myData->cData[ch].signal[C_SIG_MODULE_FAULT] == P0)) {
				myData->cData[ch].signal[C_SIG_MODULE_FAULT] = P1;
			}
#endif
		}
		myData->mData.signal[M_SIG_DATA_SAVE_ERROR] = P1;
		myData->COA_Client[group].signal[P1_SIG_10MS_TO_SBC_RESPONSE_CHECK] = 1;
	}
	//jhkw_190830e
	time1 = myPs->misc.timer_1sec;
	time2 = myPs->misc.timer_1000ms;
	diff = (time1 - myData->COA_Client[group].misc.pause_time) * 1000;
	diff += (time2 - myData->COA_Client[group].misc.pause_time2);
	//diff = (time1 - myData->COA_Client[group].misc.net_time) * 1000;
	//diff += (time2 - myData->COA_Client[group].misc.net_time2);
	//if((diff >= (myData->COA_Client[group].config.netTimeout * 6) || diff < 0)
	if((diff >= (myData->COA_Client[group].config.netTimeout * 6))
		&& myGroup->signal[G_SIG_NET_CHECK] == P0) {
		if(myData->COA_Client[group].config.state_change == 1) {
			for(ch=myGroup->misc.chOffset; ch <
				(myGroup->misc.chOffset + myPs->config.chInGroup[group]);
				ch++) {
				//if(myData->cData[ch].op.state == C_RUN) {
				//	myData->cData[ch].signal[C_SIG_MODULE_FAULT] = P1;
				//}
#if defined __COA_VER_100B__ || defined __COA_VER_100B2__ || defined __COA_VER_100C__ || defined __COA_VER_100D__ || defined __COA_VER_100F__ //kjhw_170906
				if((myData->cData[ch].op.state == C_RUN)
					&& (myData->cData[ch].signal[C_SIG_MODULE_FAULT] == P0)) {
					myData->cData[ch].signal[C_SIG_MODULE_FAULT] = P1;
				}
#else //COA_VER_100F01~
				if((myData->cData[ch].op.state == C_RUN)
					&& (myData->cData[ch].signal[C_SIG_MODULE_FAULT] == P0)
					&& (myData->ChAttribute[ch].chiller_control == 1)) {
					myData->cData[ch].signal[C_SIG_MODULE_FAULT] = P1;
				}
#endif
			}
		} else if(myData->COA_Client[group].config.state_change == 2) {
			if(myData->COA_Client[group].signal[P1_SIG_NET_CONNECTED] == P1) {
				myData->COA_Client[group].signal[P1_SIG_NET_CONNECTED] = P0;
				myData->save_msg[0].send_flag = 1; //send save_msg stop
#ifdef __COC__
				myData->save_msg_1[0].send_flag = 1; //send save_msg stop
#endif

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
#ifdef __COC__
				myData->save_msg_1[0].send_flag = 0; //send save_msg continue 
#endif
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
	int ch, stepNo, idx;
	S_MSG_VAL SendMsg;

	switch(myGroup->phase) {
		case P0:
			memset((char *)&myData->dcr_check[group][0], 0,
				sizeof(long) * MAX_CH_256);

			myGroup->misc.total_ng_ch = 0;
			myGroup->misc.scan_ch[0] = 0;
			myGroup->misc.scan_step[0] = 0;
			myGroup->misc.scan_idxStepNo_offset[0] = 0;
			myGroup->misc.tmp_scan_step[0] = 0;
			myGroup->misc.scan_ch[1] = 0;
			myGroup->misc.scan_step[1] = 0;
			myGroup->misc.scan_idxStepNo_offset[1] = 0;
			myGroup->misc.tmp_scan_step[1] = 0;
			myGroup->misc.scan_end_step_flag = P0;
			memset((char *)&myGroup->misc.long_rest_area1[0], 0,
				sizeof(unsigned char) * MAX_CH_PER_MODULE);
			memset((char *)&myGroup->misc.long_rest_area2[0], 0,
				sizeof(unsigned char) * MAX_CH_PER_MODULE);
			memset((char *)&myGroup->misc.long_rest_time1[0], 0,
				sizeof(long) * MAX_CH_PER_MODULE);
			memset((char *)&myGroup->misc.long_rest_time2[0], 0,
				sizeof(long) * MAX_CH_PER_MODULE);

			myGroup->signal[G_SIG_SAFETY_RESISTOR] = SR_OPEN;
			myGroup->phase = P1;
			break;
		case P1:
			if(myGroup->cmd[CMD_MODE_USER] == CMD_TYPE_RUN) {
				myGroup->cmd[CMD_MODE_USER] = CMD_TYPE_NONE;

				memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
				if(myGroup->workMode == WORK_TYPE_ONLINE) { //kjg_w
					SendMsg.msg = MSG_MODULE_DATASAVE_SAVED_FILE_DELETE_TRY_LG;
				} else {
					SendMsg.msg = MSG_MODULE_DATASAVE_SAVED_FILE_DELETE_TRY;
				}
				SendMsg.val[0] = group;
				send_msg(MODULE_TO_DATASAVE, (char *)&SendMsg);

				stepNo = 0;
				myGroup->stepNo = 0;
				myGroup->advStepNo = 0;
				myGroup->cycleNo = 0;
				myGroup->advCycleNo = 0;
				myGroup->start_stepNo = 0;
				myGroup->saveDt = 0;
				myGroup->runTime = 0;
				myGroup->code = G_CD_NONE;

				ch = myGroup->misc.chOffset;
				idx = IDX_LOC_OBJ_CLIENT_STEP_NO;
				myGroup->client_stepNo = (int)myData->testCond[ch]
					.local_object[stepNo][idx];
				idx = IDX_LOC_OBJ_TYPE;
				myGroup->stepType = (unsigned char)myData->testCond[ch]
					.local_object[stepNo][idx];
				idx = IDX_LOC_OBJ_MODE;
				myGroup->stepMode = (unsigned char)myData->testCond[ch]
					.local_object[stepNo][idx];
				idx = IDX_LOC_OBJ_ATTRIBUTE;
				myGroup->attribute = (unsigned char)myData->testCond[ch]
					.local_object[stepNo][idx];

				myGroup->signal[G_SIG_SAFETY_RESISTOR] = SR_OPEN;

				myGroup->state = G_RUN;
				myGroup->phase = P0;
			} else if(myGroup->cmd[CMD_MODE_USER] == CMD_TYPE_RESET) {
				//kjg_w_f bd_cmd_reset
				myGroup->cmd[CMD_MODE_USER] = CMD_TYPE_NONE;
			} else {
				myGroup->cmd[CMD_MODE_USER] = CMD_TYPE_NONE;
			}

			if(myGroup->signal[G_SIG_JIG_FAIL] == P1) {
				myGroup->signal[G_SIG_JIG_FAIL] = P2;
			} else if(myGroup->signal[G_SIG_DISCONTACT_COMPLETE] == P1) {
				myGroup->signal[G_SIG_DISCONTACT_COMPLETE] = P0;
			}
			break;
		default: break;
	}
#endif
}

void gRun_F1(int group)
{
#ifdef __COB__
	int ch, i, j, stepNo, idx, chInGroup, ch_code_check;
	int ch_standby, ch_run_hold, ch_fault, ch_pause, ch_running, ch_pause2;
	S_MSG_VAL SendMsg;

	chInGroup = myData->mData.config.chInGroup[group];
	ch = myGroup->misc.chOffset;

	switch(myGroup->phase) {
		case P0:
			if(myGroup->signal[G_SIG_DATA_SAVE_FAIL] == P1) {
				myGroup->signal[G_SIG_DATA_SAVE_FAIL] = P2;
				myGroup->cmd[CMD_MODE_USER] = CMD_TYPE_NONE;
				myGroup->state = G_STANDBY;
				myGroup->phase = P0;
				break;
			}

			if(myGroup->signal[G_SIG_SAVED_FILE_DELETE_COMPLETE] == P1) {
				myGroup->signal[G_SIG_SAVED_FILE_DELETE_COMPLETE] = P0;

				memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
				SendMsg.msg = MSG_MODULE_JIGM_TRY_CONTACT;
				SendMsg.val[0] = group;
				send_msg(MODULE_TO_JIGM, (char *)&SendMsg);
				myGroup->phase = P1;

				if(myGroup->cmd[CMD_MODE_USER] == CMD_TYPE_STOP) {
					myGroup->cmd[CMD_MODE_USER] = CMD_TYPE_NONE;
					myGroup->state = G_STANDBY;
					myGroup->phase = P0;
				} else {
					myGroup->cmd[CMD_MODE_USER] = CMD_TYPE_NONE;
				}
				break;
			}

			if(myGroup->code != G_CD_NONE) {
				memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
				SendMsg.msg = MSG_MODULE_JIGM_TRY_DISCONTACT;
				SendMsg.val[0] = group;
				send_msg(MODULE_TO_JIGM, (char *)&SendMsg);

				myGroup->state = G_STANDBY;
				myGroup->phase = P0;
			}
			break;
		case P1:
			if(myGroup->signal[G_SIG_JIG_FAIL] == P1) {
				myGroup->signal[G_SIG_JIG_FAIL] = P2;
				myGroup->cmd[CMD_MODE_USER] = CMD_TYPE_NONE;
				myGroup->state = G_STANDBY;
				myGroup->phase = P0;
			} else if(myGroup->signal[G_SIG_CONTACT_COMPLETE] == P1) {
				myGroup->signal[G_SIG_CONTACT_COMPLETE] = P0;
				if(myGroup->cmd[CMD_MODE_USER] == CMD_TYPE_STOP) {
					memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
					SendMsg.msg = MSG_MODULE_JIGM_TRY_DISCONTACT;
					SendMsg.val[0] = group;
					send_msg(MODULE_TO_JIGM, (char *)&SendMsg);

					myGroup->cmd[CMD_MODE_USER] = CMD_TYPE_NONE;
					myGroup->state = G_STANDBY;
					myGroup->phase = P0;
				} else {
					j = myData->mData.config.chPerBd * group;
					for(i=j; i < j + chInGroup; i++) {
						myData->cData[i].signal[C_SIG_CMD_RUN] = P1;
					}
					myGroup->cmd[CMD_MODE_USER] = CMD_TYPE_NONE;

					stepNo = myGroup->stepNo;
					myGroup->advCycleNo = 0;

					idx = IDX_LOC_OBJ_CLIENT_STEP_NO;
					myGroup->client_stepNo = (int)myData->testCond[ch]
						.local_object[stepNo][idx];
					idx = IDX_LOC_OBJ_TYPE;
					myGroup->stepType = (unsigned char)myData->testCond[ch]
						.local_object[stepNo][idx];
					idx = IDX_LOC_OBJ_MODE;
					myGroup->stepMode = (unsigned char)myData->testCond[ch]
						.local_object[stepNo][idx];
					idx = IDX_LOC_OBJ_ATTRIBUTE;
					myGroup->attribute = (unsigned char)myData->testCond[ch]
						.local_object[stepNo][idx];

					myGroup->phase = P10;
					myGroup->runTime = 0;
					myGroup->saveDt = 0;
				}
			} else {
				if(myGroup->code != G_CD_NONE) {
					memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
					SendMsg.msg = MSG_MODULE_JIGM_TRY_DISCONTACT;
					SendMsg.val[0] = group;
					send_msg(MODULE_TO_JIGM, (char *)&SendMsg);

					myGroup->state = G_STANDBY;
					myGroup->phase = P0;
				}
			}
			break;
		case P2:
			if(myGroup->signal[G_SIG_JIG_FAIL] == P1) {
				myGroup->signal[G_SIG_JIG_FAIL] = P2;
				myGroup->cmd[CMD_MODE_USER] = CMD_TYPE_NONE;
				myGroup->state = G_STANDBY;
				myGroup->phase = P0;
			} else if(myGroup->signal[G_SIG_CONTACT_COMPLETE] == P1) {
				myGroup->signal[G_SIG_CONTACT_COMPLETE] = P0;
				if(myGroup->cmd[CMD_MODE_USER] == CMD_TYPE_STOP) {
					myGroup->cmd[CMD_MODE_USER] = CMD_TYPE_NONE;

					memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
					SendMsg.msg = MSG_MODULE_JIGM_TRY_DISCONTACT;
					SendMsg.val[0] = group;
					send_msg(MODULE_TO_JIGM, (char *)&SendMsg);

					myGroup->state = G_STANDBY;
					myGroup->phase = P0;
					break;
				} else {
					myGroup->cmd[CMD_MODE_USER] = CMD_TYPE_NONE;
				}

				if(myGroup->attribute == ATTR_CHECK1_END
					|| myGroup->attribute == ATTR_CHECK2_END
					|| myGroup->attribute == ATTR_CHECK3_END) {
					myGroup->stepNo++;
					stepNo = myGroup->stepNo;
					myGroup->advStepNo++;
					myGroup->cycleNo++;
					myGroup->advCycleNo = 0;

					idx = IDX_LOC_OBJ_CLIENT_STEP_NO;
					myGroup->client_stepNo = (int)myData->testCond[ch]
						.local_object[stepNo][idx];
					idx = IDX_LOC_OBJ_TYPE;
					myGroup->stepType = (unsigned char)myData->testCond[ch]
						.local_object[stepNo][idx];
					idx = IDX_LOC_OBJ_MODE;
					myGroup->stepMode = (unsigned char)myData->testCond[ch]
						.local_object[stepNo][idx];
					idx = IDX_LOC_OBJ_ATTRIBUTE;
					myGroup->attribute = (unsigned char)myData->testCond[ch]
						.local_object[stepNo][idx];
				}

				j = myData->mData.config.chPerBd * group;
				for(i=j; i < j + chInGroup; i++) {
					if(myData->cData[i].op.state == C_PAUSE) {
						if(myData->cData[i].op.phase == P1
							|| myData->cData[i].op.phase == P102)  {
							myData->cData[i].signal[C_SIG_CMD_CONTINUE] = P1;
						}
					}
				}
				myGroup->phase = P10;
			} else {
				if(myGroup->cmd[CMD_MODE_USER] == CMD_TYPE_STOP) {
					myGroup->cmd[CMD_MODE_USER] = CMD_TYPE_NONE;

					memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
					SendMsg.msg = MSG_MODULE_JIGM_TRY_DISCONTACT;
					SendMsg.val[0] = group;
					send_msg(MODULE_TO_JIGM, (char *)&SendMsg);

					myGroup->state = G_STANDBY;
					myGroup->phase = P0;
					break;
				} else {
					myGroup->cmd[CMD_MODE_USER] = CMD_TYPE_NONE;
				}
			}
			break;
		case P3:
			if(myGroup->attribute == ATTR_CHECK1_END
				|| myGroup->attribute == ATTR_CHECK2_END
				|| myGroup->attribute == ATTR_CHECK3_END) {
				myGroup->stepNo++;
				stepNo = myGroup->stepNo;
				myGroup->advStepNo++;
				myGroup->cycleNo++;
				myGroup->advCycleNo = 0;

				idx = IDX_LOC_OBJ_CLIENT_STEP_NO;
				myGroup->client_stepNo = (int)myData->testCond[ch]
					.local_object[stepNo][idx];
				idx = IDX_LOC_OBJ_TYPE;
				myGroup->stepType = (unsigned char)myData->testCond[ch]
					.local_object[stepNo][idx];
				idx = IDX_LOC_OBJ_MODE;
				myGroup->stepMode = (unsigned char)myData->testCond[ch]
					.local_object[stepNo][idx];
				idx = IDX_LOC_OBJ_ATTRIBUTE;
				myGroup->attribute = (unsigned char)myData->testCond[ch]
					.local_object[stepNo][idx];
			}
			myGroup->phase = P10;
			break;
		case P10: //Running
			myGroup->runTime += myPs->config.scan_period;

			if(myGroup->signal[G_SIG_JIG_FAIL] == P1) {
				myGroup->signal[G_SIG_JIG_FAIL] = P2;
				myGroup->cmd[CMD_MODE_USER] = CMD_TYPE_PAUSE;
			}

			if(myGroup->cmd[CMD_MODE_USER] == CMD_TYPE_STOP) {
				myGroup->cmd[CMD_MODE_USER] = CMD_TYPE_NONE;
				myGroup->signal[G_SIG_CMD_STOP] = P1;
				j = myData->mData.config.chPerBd * group;
				for(i=j; i < j + chInGroup; i++) {
					if(myData->cData[i].op.state == C_RUN
						|| myData->cData[i].op.state == C_PAUSE) {
						myData->cData[i].signal[C_SIG_CMD_STOP] = P1;
					}
				}
				break;
			} else if(myGroup->cmd[CMD_MODE_USER] == CMD_TYPE_PAUSE) {
				myGroup->cmd[CMD_MODE_USER] = CMD_TYPE_NONE;
				j = myData->mData.config.chPerBd * group;
				for(i=j; i < j + chInGroup; i++) {
					if(myData->cData[i].op.state == C_RUN) {
						myData->cData[i].signal[C_SIG_CMD_PAUSE] = P1;
					}
				}
				break;
			} else if(myGroup->cmd[CMD_MODE_USER] == CMD_TYPE_NEXT_STEP) {
				myGroup->cmd[CMD_MODE_USER] = CMD_TYPE_NONE;
				myGroup->signal[G_SIG_CMD_NEXT_STEP] = P1;
				j = myData->mData.config.chPerBd * group;
				for(i=j; i < j + chInGroup; i++) {
					myData->cData[i].signal[C_SIG_CMD_NEXT_STEP] = P1;
				}
				break;
			} else {
				myGroup->cmd[CMD_MODE_USER] = CMD_TYPE_NONE;
			}

			send_save_realData_msg(group);

			ch_standby = ch_run_hold = ch_fault = ch_pause = ch_running = 0;
			ch_pause2 = 0; ch_code_check = 0;
			j = myData->mData.config.chPerBd * group;
			for(i=j; i < j + chInGroup; i++) {
				if(myData->cData[i].op.state == C_STANDBY) {
					ch_standby++;
				} else if(myData->cData[i].op.state == C_RUN
					&& myData->cData[i].op.phase == P50) {
					ch_running++;
				} else if(myData->cData[i].op.state == C_RUN
					&& myData->cData[i].op.phase == P102) {
					ch_run_hold++;
				} else if(myData->cData[i].op.state == C_PAUSE) {
					if(myData->cData[i].op.attribute == ATTR_CHECK1_END
						|| myData->cData[i].op.attribute == ATTR_CHECK2_END
						|| myData->cData[i].op.attribute == ATTR_CHECK3_END) {
						ch_pause2++;
					} else {
						if(myData->cData[i].op.code == C_CD_FAULT_PAUSE_CMD)
							ch_pause++;
						else ch_fault++;
					}
				}
				if(myData->cData[i].op.code >= C_CD_FAULT_CHECK_START
					&& myData->cData[i].op.code <= C_CD_FAULT_CHECK_END)
					ch_code_check++;
			}

			if(chInGroup == (ch_standby + ch_fault)) {
				gCompare_Ch_Average_1(group);
				send_save_msg_formation(group, 0);
				myGroup->phase = P20;
			} else if(chInGroup == (ch_pause2 + ch_fault)) {
				gCompare_Ch_Average_1(group);
				send_save_msg_formation(group, 0);
				myGroup->phase = P11;
			} else if(chInGroup == (ch_run_hold + ch_pause2 + ch_fault)) {
				gCompare_Ch_Average_1(group);

				if(myGroup->stepType == STEP_LOOP
					&& (myGroup->attribute == ATTR_CHECK1_END
						|| myGroup->attribute == ATTR_CHECK2_END
						|| myGroup->attribute == ATTR_CHECK3_END)) {
					gCellCheck_Compare(group);
					if(myGroup->misc.cell_check_result == 0) { //check_ok
						if(chInGroup != ch_code_check)
							send_save_msg_formation(group, 0);
						myGroup->phase = P11;
					} else {
						myGroup->misc.cell_check_result = 0;
						if(myGroup->misc.cell_check_count >= 3) {
							if(chInGroup != ch_code_check)
								send_save_msg_formation(group, 0);
							myGroup->phase = P11;
						} else {
							myGroup->phase = P50;
						}
					}
				} else {
					if(chInGroup != ch_code_check)
						send_save_msg_formation(group, 0);
					//send_save_dcir_msg(group); //kjg_w_f
					myGroup->phase = P11;
				}
			} else if(chInGroup == (ch_run_hold + ch_standby)) {
				gCompare_Ch_Average_1(group);
				send_save_msg_formation(group, 0);
				myGroup->phase = P20;
			} else if(chInGroup == (ch_pause + ch_fault)) {
				myGroup->phase = P23;
			}
			break;
		case P11:
			if(myGroup->cmd[CMD_MODE_USER] == CMD_TYPE_STOP) {
				myGroup->cmd[CMD_MODE_USER] = CMD_TYPE_NONE;
				myGroup->signal[G_SIG_CMD_STOP] = P1;

				j = myData->mData.config.chPerBd * group;
				for(i=j; i < j + chInGroup; i++) {
					myData->cData[i].signal[C_SIG_CMD_STOP] = P1;
				}
				break;
			}

			if(myGroup->signal[G_SIG_CMD_NEXT_STEP] == P1) {
				myGroup->signal[G_SIG_CMD_NEXT_STEP] = P0;
				myGroup->phase = P12;
				break;
			}

			ch_standby = ch_run_hold = ch_fault = ch_pause = 0;
			ch_pause2 = 0;
			j = myData->mData.config.chPerBd * group;
			for(i=j; i < j + chInGroup; i++) {
				if(myData->cData[i].op.state == C_STANDBY) {
					ch_standby++;
				} else if(myData->cData[i].op.state == C_RUN
					&& myData->cData[i].op.phase == P102) {
					ch_run_hold++;
				} else if(myData->cData[i].op.state == C_PAUSE) {
					if(myData->cData[i].op.attribute == ATTR_CHECK1_END
						|| myData->cData[i].op.attribute == ATTR_CHECK2_END
						|| myData->cData[i].op.attribute == ATTR_CHECK3_END) {
						ch_pause2++;
					} else {
						if(myData->cData[i].op.code == C_CD_FAULT_PAUSE_CMD)
							ch_pause++;
						else ch_fault++;
					}
				}
			}

			if(chInGroup == ch_fault) { //all ch fault
				if(myGroup->signal[G_SIG_CMD_STOP] == P0) {
					myGroup->signal[G_SIG_ALL_CHANNEL_ERROR] = P1;
					myGroup->phase = P30;//khkw
				} else {
					myGroup->signal[G_SIG_CMD_STOP] = P0;
					myGroup->phase = P20;
				}
			} else if(chInGroup == (ch_standby)) { //normal
				myGroup->phase = P20;
			} else if(chInGroup == (ch_pause2 + ch_fault)) { //pause
				myGroup->phase = P23;
			} else if(chInGroup == (ch_run_hold + ch_pause2 + ch_fault)) { //normal
				myGroup->phase = P12;
			} else if(chInGroup == (ch_pause + ch_fault)) { //pause
				myGroup->phase = P23;
			}
			break;
		case P12:
			switch(myGroup->stepType) {
				case STEP_CYCLE:
					myGroup->start_stepNo = myGroup->stepNo;
					myGroup->stepNo++;
					stepNo = myGroup->stepNo;
					myGroup->advStepNo++;

					idx = IDX_LOC_OBJ_CLIENT_STEP_NO;
					myGroup->client_stepNo = (int)myData->testCond[ch]
						.local_object[stepNo][idx];
					idx = IDX_LOC_OBJ_TYPE;
					myGroup->stepType = (unsigned char)myData->testCond[ch]
						.local_object[stepNo][idx];
					idx = IDX_LOC_OBJ_MODE;
					myGroup->stepMode = (unsigned char)myData->testCond[ch]
						.local_object[stepNo][idx];
					idx = IDX_LOC_OBJ_ATTRIBUTE;
					myGroup->attribute = (unsigned char)myData->testCond[ch]
						.local_object[stepNo][idx];

					j = myData->mData.config.chPerBd * group;
					for(i=j; i < j + chInGroup; i++) {
						if(myData->cData[i].op.state == C_RUN
							&& myData->cData[i].op.phase == P102) {
							myData->cData[i].op.phase = P101;
						} else if(myData->cData[i].op.state == C_PAUSE
							&& myData->cData[i].op.phase == P102) {
							myData->cData[i].op.phase = P101;
						}
					}
					myGroup->phase = P10;
					break;
				case STEP_LOOP:
					if(myGroup->attribute == ATTR_CHECK1_END
						|| myGroup->attribute == ATTR_CHECK2_END
						|| myGroup->attribute == ATTR_CHECK3_END) {
						if(myGroup->workMode == WORK_TYPE_ONLINE) {
							myGroup->phase = P3;
						} else {
							myGroup->phase = P24;
						}
					} else {
						myGroup->stepNo++;
						stepNo = myGroup->stepNo;
						myGroup->advStepNo++;

						idx = IDX_LOC_OBJ_CLIENT_STEP_NO;
						myGroup->client_stepNo = (int)myData->testCond[ch]
							.local_object[stepNo][idx];
						idx = IDX_LOC_OBJ_TYPE;
						myGroup->stepType = (unsigned char)myData->testCond[ch]
							.local_object[stepNo][idx];
						idx = IDX_LOC_OBJ_MODE;
						myGroup->stepMode = (unsigned char)myData->testCond[ch]
							.local_object[stepNo][idx];
						idx = IDX_LOC_OBJ_ATTRIBUTE;
						myGroup->attribute = (unsigned char)myData->testCond[ch]
							.local_object[stepNo][idx];

						j = myData->mData.config.chPerBd * group;
						for(i=j; i < j + chInGroup; i++) {
							if(myData->cData[i].op.state == C_RUN
								&& myData->cData[i].op.phase == P102) {
								myData->cData[i].op.phase = P101;
							} else if(myData->cData[i].op.state == C_PAUSE
								&& myData->cData[i].op.phase == P102) {
								myData->cData[i].op.phase = P101;
							}
						}
						myGroup->phase = P10;
					}
					break;
				case STEP_END:
					stepNo = myGroup->stepNo;
					idx = IDX_LOC_OBJ_CLIENT_STEP_NO;
					myGroup->client_stepNo = (int)myData->testCond[ch]
						.local_object[stepNo][idx];
					idx = IDX_LOC_OBJ_ATTRIBUTE;
					myGroup->attribute = (unsigned char)myData->testCond[ch]
						.local_object[stepNo][idx];

					j = myData->mData.config.chPerBd * group;
					for(i=j; i < j + chInGroup; i++) {
						if(myData->cData[i].op.state == C_RUN
							&& myData->cData[i].op.phase == P102) {
							myData->cData[i].op.phase = P101;
						} else if(myData->cData[i].op.state == C_PAUSE
							&& myData->cData[i].op.phase == P102) {
							myData->cData[i].op.phase = P101;
						} else if(myData->cData[i].op.state == C_PAUSE) {
							myData->cData[i].op.state = C_STANDBY;
							myData->cData[i].op.phase = P0;
						}
					}
					myGroup->phase = P20;
					break;
				case STEP_OCV:
				case STEP_REST:
				case STEP_LONG_TIME_REST:
				case STEP_CHARGE:
				case STEP_DISCHARGE:
					myGroup->stepNo++;
					stepNo = myGroup->stepNo;
					myGroup->advStepNo++;

					idx = IDX_LOC_OBJ_CLIENT_STEP_NO;
					myGroup->client_stepNo = (int)myData->testCond[ch]
						.local_object[stepNo][idx];
					idx = IDX_LOC_OBJ_TYPE;
					myGroup->stepType = (unsigned char)myData->testCond[ch]
						.local_object[stepNo][idx];
					idx = IDX_LOC_OBJ_MODE;
					myGroup->stepMode = (unsigned char)myData->testCond[ch]
						.local_object[stepNo][idx];
					idx = IDX_LOC_OBJ_ATTRIBUTE;
					myGroup->attribute = (unsigned char)myData->testCond[ch]
						.local_object[stepNo][idx];

					j = myData->mData.config.chPerBd * group;
					for(i=j; i < j + chInGroup; i++) {
						if(myData->cData[i].op.state == C_RUN
							&& myData->cData[i].op.phase == P102) {
							myData->cData[i].op.phase = P101;
						} else if(myData->cData[i].op.state == C_PAUSE
							&& myData->cData[i].op.phase == P102) {
							myData->cData[i].op.phase = P101;
						}
					}
					myGroup->phase = P10;
					break;
				default:	break;
			}
			myGroup->runTime = 0;
			myGroup->saveDt = 0;
			break;
		case P13:
			myGroup->state = G_PAUSE;
			myGroup->phase = P10;
			break;
		case P20:
			if(myGroup->cmd[CMD_MODE_USER] == CMD_TYPE_STOP) {
				myGroup->cmd[CMD_MODE_USER] = CMD_TYPE_NONE;
			}
			if(myGroup->attribute == ATTR_TOTAL_CHECK) {
				send_save_msg_formation(group, 0);
				myGroup->phase = P30;
			} else { //normal end
				if(myGroup->workMode == WORK_TYPE_OFFLINE)  {
					myGroup->phase = P30;
				} else { //WORK_ONLINE	
					myGroup->phase = P40;
				}
			}
			break;
		case P23:
			myGroup->phase = P32;
			break;
		case P24:
			myGroup->state = G_PAUSE;
			myGroup->phase = P0;
			break;
		case P30:
			j = myData->mData.config.chPerBd * group;
			for(i=j; i < j + chInGroup; i++) {
				if(myData->cData[i].op.state != C_STANDBY) {
					myData->cData[i].op.state = C_STANDBY;
					myData->cData[i].op.phase = P0;
				}
			}

			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
			SendMsg.msg = MSG_MODULE_JIGM_TRY_DISCONTACT;
			SendMsg.val[0] = group;
			send_msg(MODULE_TO_JIGM, (char *)&SendMsg);

			myGroup->phase = P31;
			break;
		case P31:
			if(myGroup->signal[G_SIG_JIG_FAIL] == P1) {
				myGroup->signal[G_SIG_JIG_FAIL] = P2;
				myGroup->cmd[CMD_MODE_USER] = CMD_TYPE_NONE;
				myGroup->state = G_STANDBY;
				myGroup->phase = P0;
			} else if(myGroup->signal[G_SIG_DISCONTACT_COMPLETE] == P1) {
				myGroup->signal[G_SIG_DISCONTACT_COMPLETE] = P0;
				myGroup->cmd[CMD_MODE_USER] = CMD_TYPE_NONE;
				myGroup->state = G_STANDBY;
				myGroup->phase = P0;
			}
			break;
		case P32:
			myGroup->state = G_PAUSE;
			myGroup->phase = P0;
			break;
		case P40:
			myGroup->phase = P41;
			break;
		case P41:
			myGroup->signal[G_SIG_NEXT_STEP_EXIST] = P0;
			myGroup->cmd[CMD_MODE_USER] = CMD_TYPE_NONE;
			myGroup->state = G_STANDBY;
			myGroup->phase = P0;
			break;
		case P50:
			j = myData->mData.config.chPerBd * group;
			for(i=j; i < j + chInGroup; i++) {
				if(myData->cData[i].op.state != C_STANDBY) {
					myData->cData[i].op.state = C_STANDBY;
					myData->cData[i].op.phase = P0;
				}
			}

			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
			SendMsg.msg = MSG_MODULE_JIGM_TRY_DISCONTACT;
			SendMsg.val[0] = group;
			send_msg(MODULE_TO_JIGM, (char *)&SendMsg);
			myGroup->phase = P51;
			break;
		case P51:
			if(myGroup->signal[G_SIG_JIG_FAIL] == P1) {
				myGroup->signal[G_SIG_JIG_FAIL] = P2;
				myGroup->cmd[CMD_MODE_USER] = CMD_TYPE_NONE;
				myGroup->state = G_STANDBY;
				myGroup->phase = P0;
			} else if(myGroup->signal[G_SIG_DISCONTACT_COMPLETE] == P1) {
				myGroup->signal[G_SIG_DISCONTACT_COMPLETE] = P0;
				myGroup->cmd[CMD_MODE_USER] = CMD_TYPE_NONE;
				myGroup->phase = P52;
			}
			break;
		case P52:
			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
			SendMsg.msg = MSG_MODULE_JIGM_TRY_CONTACT;
			SendMsg.val[0] = group;
			send_msg(MODULE_TO_JIGM, (char *)&SendMsg);

			stepNo = 0;
			myGroup->stepNo = 0;
			myGroup->advStepNo = 0;
			myGroup->cycleNo = 0;
			myGroup->advCycleNo = 0;
			myGroup->start_stepNo = 0;
			myGroup->saveDt = 0;
			myGroup->runTime = 0;
			myGroup->code = G_CD_NONE;

			ch = myGroup->misc.chOffset;
			idx = IDX_LOC_OBJ_CLIENT_STEP_NO;
			myGroup->client_stepNo = (int)myData->testCond[ch]
				.local_object[stepNo][idx];
			idx = IDX_LOC_OBJ_TYPE;
			myGroup->stepType = (unsigned char)myData->testCond[ch]
				.local_object[stepNo][idx];
			idx = IDX_LOC_OBJ_MODE;
			myGroup->stepMode = (unsigned char)myData->testCond[ch]
				.local_object[stepNo][idx];
			idx = IDX_LOC_OBJ_ATTRIBUTE;
			myGroup->attribute = (unsigned char)myData->testCond[ch]
				.local_object[stepNo][idx];

			myGroup->phase = P1;
			break;
		default:	break;
	}
#endif
}

void gRun_F2(int group)
{ //debug_size_cob
/*	int ch, i, j, stepNo, idx, chInGroup;
	int ch_standby, ch_run_hold, ch_fault, ch_pause, ch_pause2;
	int monitor_ch;
	S_MSG_VAL	SendMsg;

	chInGroup = myData->mData.config.chInGroup[group];
	ch = myGroup->misc.chOffset;

	switch(myGroup->phase) {
		case P0:
			if(myGroup->signal[G_SIG_DATA_SAVE_FAIL] == P1) {
				myGroup->signal[G_SIG_DATA_SAVE_FAIL] = P2;
				myGroup->cmd[CMD_MODE_USER] = CMD_TYPE_NONE;
				myGroup->state = G_STANDBY;
				myGroup->phase = P0;
			} else if(myGroup->signal[G_SIG_SAVED_FILE_DELETE_COMPLETE] == P1) {
				myGroup->signal[G_SIG_SAVED_FILE_DELETE_COMPLETE] = P0;

				memset((char *)&myData->COB_opSave[group][0][0], 0,
					sizeof(S_CH_OP_DATA) * MAX_CH_PER_MODULE * MAX_P2_STEP);
				memset((char *)&myData->COB_save_step[group][0], 0,
					sizeof(int) * MAX_CH_PER_MODULE);

				myGroup->misc.total_ng_ch = 0;
				myGroup->misc.scan_ch[0] = 0;
				myGroup->misc.scan_step[0] = 0;
				myGroup->misc.scan_idxStepNo_offset[0] = 0;
				myGroup->misc.tmp_scan_step[0] = 0;
				myGroup->misc.scan_ch[1] = 0;
				myGroup->misc.scan_step[1] = 0;
				myGroup->misc.scan_idxStepNo_offset[1] = 0;
				myGroup->misc.tmp_scan_step[1] = 0;
				myGroup->misc.scan_end_step_flag = P0;
				memset((char *)&myGroup->misc.long_rest_area1[0],
					0, sizeof(unsigned char) * MAX_CH_PER_MODULE);
				memset((char *)&myGroup->misc.long_rest_area2[0],
					0, sizeof(unsigned char) * MAX_CH_PER_MODULE);
				memset((char *)&myGroup->misc.long_rest_time1[0], 0,
					sizeof(long) * MAX_CH_PER_MODULE);
				memset((char *)&myGroup->misc.long_rest_time2[0], 0,
					sizeof(long) * MAX_CH_PER_MODULE);

				if(myData->AppControl.config.debugType == 0) {
					monitor_ch = (int)myData->COB_Client[group].config
						.ChArray2[myGroup->misc.scan_ch[0]];
					if(myData->AppControl.config.systemModel
						== F_SDI_5V_400A_200A_100A_10A_2) {
						monitor_ch += group;
					}
					Select_OutPoint(2, monitor_ch+1, O_RUN_RELAY, ON);
				}

				memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
				SendMsg.msg = MSG_MODULE_JIGM_TRY_CONTACT;
				SendMsg.val[0] = group;
				send_msg(MODULE_TO_JIGM, (char *)&SendMsg);
				myGroup->phase = P1;

				if(myGroup->cmd[CMD_MODE_USER] == CMD_TYPE_STOP) {
					myGroup->cmd[CMD_MODE_USER] = CMD_TYPE_NONE;
					myGroup->state = G_STANDBY;
					myGroup->phase = P0;
				} else {
					myGroup->cmd[CMD_MODE_USER] = CMD_TYPE_NONE;
				}
			} else {
				if(myGroup->code != G_CD_NONE) {
					myGroup->state = G_STANDBY;
					myGroup->phase = P0;

					memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
					SendMsg.msg = MSG_MODULE_JIGM_TRY_DISCONTACT;
					SendMsg.val[0] = group;
					send_msg(MODULE_TO_JIGM, (char *)&SendMsg);
				}
			}
			break;
		case P1:
			if(myGroup->signal[G_SIG_JIG_FAIL] == P1) {
				myGroup->signal[G_SIG_JIG_FAIL] = P2;
				myGroup->cmd[CMD_MODE_USER] = CMD_TYPE_NONE;
				myGroup->state = G_STANDBY;
				myGroup->phase = P0;
			} else if(myGroup->signal[G_SIG_CONTACT_COMPLETE] == P1) {
				myGroup->signal[G_SIG_CONTACT_COMPLETE] = P0;
				if(myGroup->cmd[CMD_MODE_USER] == CMD_TYPE_STOP) {
					memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
					SendMsg.msg = MSG_MODULE_JIGM_TRY_DISCONTACT;
					SendMsg.val[0] = group;
					send_msg(MODULE_TO_JIGM, (char *)&SendMsg);

					myGroup->cmd[CMD_MODE_USER] = CMD_TYPE_NONE;
					myGroup->state = G_STANDBY;
					myGroup->phase = P0;
				} else {
					myData->mData.signal[M_SIG_RUNNING_GROUP]
						= (unsigned char)group;
					myData->cData[0].signal[C_SIG_CMD_RUN] = P1;
					myGroup->cmd[CMD_MODE_USER] = CMD_TYPE_NONE;

					stepNo = myGroup->stepNo;
					myGroup->advCycleNo = 0;

					idx = IDX_LOC_OBJ_CLIENT_STEP_NO;
					myGroup->client_stepNo = (int)myData->testCond[ch]
						.local_object[stepNo][idx];
					idx = IDX_LOC_OBJ_TYPE;
					myGroup->stepType = (unsigned char)myData->testCond[ch]
						.local_object[stepNo][idx];
					idx = IDX_LOC_OBJ_MODE;
					myGroup->stepMode = (unsigned char)myData->testCond[ch]
						.local_object[stepNo][idx];
					idx = IDX_LOC_OBJ_ATTRIBUTE;
					myGroup->attribute = (unsigned char)myData->testCond[ch]
						.local_object[stepNo][idx];

					myGroup->phase = P10;
					myGroup->runTime = 0;
					myGroup->saveDt = 0;
				}
			} else {
				if(myGroup->code != G_CD_NONE) {
					memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
					SendMsg.msg = MSG_MODULE_JIGM_TRY_DISCONTACT;
					SendMsg.val[0] = group;
					send_msg(MODULE_TO_JIGM, (char *)&SendMsg);

					myGroup->state = G_STANDBY;
					myGroup->phase = P0;
				}
			}
			break;
		case P2:
			if(myGroup->signal[G_SIG_JIG_FAIL] == P1) {
				myGroup->signal[G_SIG_JIG_FAIL] = P2;
				myGroup->cmd[CMD_MODE_USER] = CMD_TYPE_NONE;
				myGroup->state = G_STANDBY;
				myGroup->phase = P0;
			} else if(myGroup->signal[G_SIG_CONTACT_COMPLETE] == P1) {
				myGroup->signal[G_SIG_CONTACT_COMPLETE] = P0;
				if(myGroup->cmd[CMD_MODE_USER] == CMD_TYPE_STOP) {
					myGroup->cmd[CMD_MODE_USER] = CMD_TYPE_NONE;

					memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
					SendMsg.msg = MSG_MODULE_JIGM_TRY_DISCONTACT;
					SendMsg.val[0] = group;
					send_msg(MODULE_TO_JIGM, (char *)&SendMsg);

					myGroup->state = G_STANDBY;
					myGroup->phase = P0;
					break;
				} else {
					myGroup->cmd[CMD_MODE_USER] = CMD_TYPE_NONE;
				}

				if(myGroup->attribute == ATTR_CHECK1_END
					|| myGroup->attribute == ATTR_CHECK2_END
					|| myGroup->attribute == ATTR_CHECK3_END) {
					myGroup->stepNo++;
					stepNo = myGroup->stepNo;
					myGroup->advStepNo++;
					myGroup->cycleNo++;
					myGroup->advCycleNo = 0;

					idx = IDX_LOC_OBJ_CLIENT_STEP_NO;
					myGroup->client_stepNo = (int)myData->testCond[ch]
						.local_object[stepNo][idx];
					idx = IDX_LOC_OBJ_TYPE;
					myGroup->stepType = (unsigned char)myData->testCond[ch]
						.local_object[stepNo][idx];
					idx = IDX_LOC_OBJ_MODE;
					myGroup->stepMode = (unsigned char)myData->testCond[ch]
						.local_object[stepNo][idx];
					idx = IDX_LOC_OBJ_ATTRIBUTE;
					myGroup->attribute = (unsigned char)myData->testCond[ch]
						.local_object[stepNo][idx];
				}

				if(myData->cData[0].op.code == C_CD_FAULT_PAUSE_CMD) {
					myData->cData[0].signal[C_SIG_CMD_CONTINUE] = P1;
				} else {
					myData->cData[0].signal[C_SIG_CMD_RUN] = P1;
				}
				myGroup->phase = P10;
			} else {
				if(myGroup->cmd[CMD_MODE_USER] == CMD_TYPE_STOP) {
					myGroup->cmd[CMD_MODE_USER] = CMD_TYPE_NONE;

					memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
					SendMsg.msg = MSG_MODULE_JIGM_TRY_DISCONTACT;
					SendMsg.val[0] = group;
					send_msg(MODULE_TO_JIGM, (char *)&SendMsg);

					myGroup->state = G_STANDBY;
					myGroup->phase = P0;
					break;
				} else {
					myGroup->cmd[CMD_MODE_USER] = CMD_TYPE_NONE;
				}
			}
			break;
		case P3:
			if(myGroup->attribute == ATTR_CHECK1_END
				|| myGroup->attribute == ATTR_CHECK2_END
				|| myGroup->attribute == ATTR_CHECK3_END) {
				myGroup->stepNo++;
				stepNo = myGroup->stepNo;
				myGroup->advStepNo++;
				myGroup->cycleNo++;
				myGroup->advCycleNo = 0;

				idx = IDX_LOC_OBJ_CLIENT_STEP_NO;
				myGroup->client_stepNo = (int)myData->testCond[ch]
					.local_object[stepNo][idx];
				idx = IDX_LOC_OBJ_TYPE;
				myGroup->stepType = (unsigned char)myData->testCond[ch]
					.local_object[stepNo][idx];
				idx = IDX_LOC_OBJ_MODE;
				myGroup->stepMode = (unsigned char)myData->testCond[ch]
					.local_object[stepNo][idx];
				idx = IDX_LOC_OBJ_ATTRIBUTE;
				myGroup->attribute = (unsigned char)myData->testCond[ch]
					.local_object[stepNo][idx];
			}
			myGroup->phase = P10;
			break;
		case P10: //Running
			myGroup->runTime += myPs->config.scan_period;

			if(myGroup->signal[G_SIG_JIG_FAIL] == P1) {
				myGroup->signal[G_SIG_JIG_FAIL] = P2;
				myGroup->cmd[CMD_MODE_USER] = CMD_TYPE_PAUSE;
			}

			if(myGroup->cmd[CMD_MODE_USER] == CMD_TYPE_STOP) {
				myGroup->cmd[CMD_MODE_USER] = CMD_TYPE_NONE;
				myGroup->signal[G_SIG_CMD_STOP] = P1;
				j = 0;
				for(i=j; i < j + chInGroup; i++) {
					if(myData->cData[i].op.state == C_RUN
						|| myData->cData[i].op.state == C_PAUSE) {
						myData->cData[i].signal[C_SIG_CMD_STOP] = P1;
					}
				}
				myGroup->phase = P30;
				break;
			} else if(myGroup->cmd[CMD_MODE_USER] == CMD_TYPE_PAUSE) {
				myGroup->cmd[CMD_MODE_USER] = CMD_TYPE_NONE;
				j = 0;
				for(i=j; i < j + chInGroup; i++) {
					if(myData->cData[i].op.state == C_RUN) {
						myData->cData[i].signal[C_SIG_CMD_PAUSE] = P1;
					}
				}
				myGroup->phase = P32;
				break;
			} else if(myGroup->cmd[CMD_MODE_USER] == CMD_TYPE_NEXT_STEP) {
				myGroup->cmd[CMD_MODE_USER] = CMD_TYPE_NONE;
				myGroup->signal[G_SIG_CMD_NEXT_STEP] = P1;
				j = 0;
				for(i=j; i < j + chInGroup; i++) {
					myData->cData[i].signal[C_SIG_CMD_NEXT_STEP] = P1;
				}
				break;
			} else {
				myGroup->cmd[CMD_MODE_USER] = CMD_TYPE_NONE;
			}

			send_save_realData_msg(group);

			if(myData->AppControl.config.systemModel
				== F_SDI_5V_400A_200A_100A_10A) {
				gRun_F_SDI_5V_400A_200A_100A_10A(group);
			} else if(myData->AppControl.config.systemModel
				== F_SDI_5V_400A_200A_100A_10A_2) {
				gRun_F_SDI_5V_400A_200A_100A_10A_2(group);
			}
			break;
		case P11:
			if(myGroup->cmd[CMD_MODE_USER] == CMD_TYPE_STOP) {
				myGroup->cmd[CMD_MODE_USER] = CMD_TYPE_NONE;
				myGroup->signal[G_SIG_CMD_STOP] = P1;

				j = myData->mData.config.chPerBd * group;
				for(i=j; i < j + chInGroup; i++) {
					myData->cData[i].signal[C_SIG_CMD_STOP] = P1;
				}
				break;
			}

			if(myGroup->signal[G_SIG_CMD_NEXT_STEP] == P1) {
				myGroup->signal[G_SIG_CMD_NEXT_STEP] = P0;
				myGroup->phase = P12;
				break;
			}

			ch_standby = ch_run_hold = ch_fault = ch_pause = 0;
			ch_pause2 = 0;
			j = myData->mData.config.chPerBd * group;
			for(i=j; i < j + chInGroup; i++) {
				if(myData->cData[i].op.state == C_STANDBY) {
					ch_standby++;
				} else if(myData->cData[i].op.state == C_RUN
					&& myData->cData[i].op.phase == P102) {
					ch_run_hold++;
				} else if(myData->cData[i].op.state == C_PAUSE) {
					if(myData->cData[i].op.attribute == ATTR_CHECK1_END
						|| myData->cData[i].op.attribute == ATTR_CHECK2_END
						|| myData->cData[i].op.attribute == ATTR_CHECK3_END) {
						ch_pause2++;
					} else {
						if(myData->cData[i].op.code == C_CD_FAULT_PAUSE_CMD)
							ch_pause++;
						else ch_fault++;
					}
				}
			}

			if(chInGroup == ch_fault) { //all ch fault
				if(myGroup->signal[G_SIG_CMD_STOP] == P0) {
					myGroup->signal[G_SIG_ALL_CHANNEL_ERROR] = P1;
					myGroup->phase = P30;//khkw
				} else {
					myGroup->signal[G_SIG_CMD_STOP] = P0;
					myGroup->phase = P20;
				}
			} else if(chInGroup == (ch_standby)) { //normal
				myGroup->phase = P20;
			} else if(chInGroup == (ch_pause2 + ch_fault)) { //pause
				myGroup->phase = P23;
			} else if(chInGroup == (ch_run_hold + ch_pause2 + ch_fault)) { //normal
				myGroup->phase = P12;
			} else if(chInGroup == (ch_pause + ch_fault)) { //pause
				myGroup->phase = P23;
			}
			break;
		case P12:
			switch(myGroup->stepType) {
				case STEP_CYCLE:
					myGroup->start_stepNo = myGroup->stepNo;
					myGroup->stepNo++;
					stepNo = myGroup->stepNo;
					myGroup->advStepNo++;

					idx = IDX_LOC_OBJ_CLIENT_STEP_NO;
					myGroup->client_stepNo = (int)myData->testCond[ch]
						.local_object[stepNo][idx];
					idx = IDX_LOC_OBJ_TYPE;
					myGroup->stepType = (unsigned char)myData->testCond[ch]
						.local_object[stepNo][idx];
					idx = IDX_LOC_OBJ_MODE;
					myGroup->stepMode = (unsigned char)myData->testCond[ch]
						.local_object[stepNo][idx];
					idx = IDX_LOC_OBJ_ATTRIBUTE;
					myGroup->attribute = (unsigned char)myData->testCond[ch]
						.local_object[stepNo][idx];

					j = myData->mData.config.chPerBd * group;
					for(i=j; i < j + chInGroup; i++) {
						if(myData->cData[i].op.state == C_RUN
							&& myData->cData[i].op.phase == P102) {
							myData->cData[i].op.phase = P101;
						} else if(myData->cData[i].op.state == C_PAUSE
							&& myData->cData[i].op.phase == P102) {
							myData->cData[i].op.phase = P101;
						}
					}
					myGroup->phase = P10;
					break;
				case STEP_LOOP:
					if(myGroup->attribute == ATTR_CHECK1_END
						|| myGroup->attribute == ATTR_CHECK2_END
						|| myGroup->attribute == ATTR_CHECK3_END) {
						if(myGroup->workMode == WORK_TYPE_ONLINE) {
							myGroup->phase = P3;
						} else {
							myGroup->phase = P24;
						}
					} else {
						myGroup->stepNo++;
						stepNo = myGroup->stepNo;
						myGroup->advStepNo++;

						idx = IDX_LOC_OBJ_CLIENT_STEP_NO;
						myGroup->client_stepNo = (int)myData->testCond[ch]
							.local_object[stepNo][idx];
						idx = IDX_LOC_OBJ_TYPE;
						myGroup->stepType = (unsigned char)myData->testCond[ch]
							.local_object[stepNo][idx];
						idx = IDX_LOC_OBJ_MODE;
						myGroup->stepMode = (unsigned char)myData->testCond[ch]
							.local_object[stepNo][idx];
						idx = IDX_LOC_OBJ_ATTRIBUTE;
						myGroup->attribute = (unsigned char)myData->testCond[ch]
							.local_object[stepNo][idx];

						j = myData->mData.config.chPerBd * group;
						for(i=j; i < j + chInGroup; i++) {
							if(myData->cData[i].op.state == C_RUN
								&& myData->cData[i].op.phase == P102) {
								myData->cData[i].op.phase = P101;
							} else if(myData->cData[i].op.state == C_PAUSE
								&& myData->cData[i].op.phase == P102) {
								myData->cData[i].op.phase = P101;
							}
						}
						myGroup->phase = P10;
					}
					break;
				case STEP_END:
					stepNo = myGroup->stepNo;
					idx = IDX_LOC_OBJ_CLIENT_STEP_NO;
					myGroup->client_stepNo = (int)myData->testCond[ch]
						.local_object[stepNo][idx];
					idx = IDX_LOC_OBJ_ATTRIBUTE;
					myGroup->attribute = (unsigned char)myData->testCond[ch]
						.local_object[stepNo][idx];

					j = myData->mData.config.chPerBd * group;
					for(i=j; i < j + chInGroup; i++) {
						if(myData->cData[i].op.state == C_RUN
							&& myData->cData[i].op.phase == P102) {
							myData->cData[i].op.phase = P101;
						} else if(myData->cData[i].op.state == C_PAUSE
							&& myData->cData[i].op.phase == P102) {
							myData->cData[i].op.phase = P101;
						} else if(myData->cData[i].op.state == C_PAUSE) {
							myData->cData[i].op.state = C_STANDBY;
							myData->cData[i].op.phase = P0;
						}
					}
					myGroup->phase = P20;
					break;
				case STEP_OCV:
				case STEP_REST:
				case STEP_LONG_TIME_REST:
				case STEP_CHARGE:
				case STEP_DISCHARGE:
					myGroup->stepNo++;
					stepNo = myGroup->stepNo;
					myGroup->advStepNo++;

					idx = IDX_LOC_OBJ_CLIENT_STEP_NO;
					myGroup->client_stepNo = (int)myData->testCond[ch]
						.local_object[stepNo][idx];
					idx = IDX_LOC_OBJ_TYPE;
					myGroup->stepType = (unsigned char)myData->testCond[ch]
						.local_object[stepNo][idx];
					idx = IDX_LOC_OBJ_MODE;
					myGroup->stepMode = (unsigned char)myData->testCond[ch]
						.local_object[stepNo][idx];
					idx = IDX_LOC_OBJ_ATTRIBUTE;
					myGroup->attribute = (unsigned char)myData->testCond[ch]
						.local_object[stepNo][idx];

					j = myData->mData.config.chPerBd * group;
					for(i=j; i < j + chInGroup; i++) {
						if(myData->cData[i].op.state == C_RUN
							&& myData->cData[i].op.phase == P102) {
							myData->cData[i].op.phase = P101;
						} else if(myData->cData[i].op.state == C_PAUSE
							&& myData->cData[i].op.phase == P102) {
							myData->cData[i].op.phase = P101;
						}
					}
					myGroup->phase = P10;
					break;
				default:	break;
			}
			myGroup->runTime = 0;
			myGroup->saveDt = 0;
			break;
		case P13:
			for(i=0; i < chInGroup; i++) {
				Select_OutPoint(2, i+1, O_RUN_RELAY, OFF);
			}

			myGroup->state = G_PAUSE;
			myGroup->phase = P10;
			break;
		case P20:
			if(myGroup->cmd[CMD_MODE_USER] == CMD_TYPE_STOP) {
				myGroup->cmd[CMD_MODE_USER] = CMD_TYPE_NONE;
			}
			if(myGroup->attribute == ATTR_TOTAL_CHECK) {
				send_save_msg_formation(group, 0);
				myGroup->phase = P30;
			} else { //normal end
				if(myGroup->workMode == WORK_TYPE_OFFLINE)  {
					myGroup->phase = P30;
				} else { //WORK_ONLINE	
					myGroup->phase = P40;
				}
			}
			break;
		case P23:
			myGroup->phase = P32;
			break;
		case P24:
			for(i=0; i < chInGroup; i++) {
				Select_OutPoint(2, i+1, O_RUN_RELAY, OFF);
			}

			myGroup->state = G_PAUSE;
			myGroup->phase = P0;
			break;
		case P30:
			for(i=0; i < chInGroup; i++) {
				Select_OutPoint(2, i+1, O_RUN_RELAY, OFF);
			}

			if(myData->cData[0].op.state != C_STANDBY) {
				myData->cData[0].op.state = C_STANDBY;
				myData->cData[0].op.phase = P0;
			}

			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
			SendMsg.msg = MSG_MODULE_JIGM_TRY_DISCONTACT;
			SendMsg.val[0] = group;
			send_msg(MODULE_TO_JIGM, (char *)&SendMsg);

			myGroup->phase = P31;
			break;
		case P31:
			if(myGroup->signal[G_SIG_JIG_FAIL] == P1) {
				myGroup->signal[G_SIG_JIG_FAIL] = P2;
				myGroup->cmd[CMD_MODE_USER] = CMD_TYPE_NONE;
				myGroup->state = G_STANDBY;
				myGroup->phase = P0;
			} else if(myGroup->signal[G_SIG_DISCONTACT_COMPLETE] == P1) {
				myGroup->signal[G_SIG_DISCONTACT_COMPLETE] = P0;
				myGroup->cmd[CMD_MODE_USER] = CMD_TYPE_NONE;
				myGroup->state = G_STANDBY;
				myGroup->phase = P0;
			}
			break;
		case P32:
			for(i=0; i < chInGroup; i++) {
				Select_OutPoint(2, i+1, O_RUN_RELAY, OFF);
			}

			myGroup->state = G_PAUSE;
			myGroup->phase = P0;
			break;
		case P40:
			for(i=0; i < chInGroup; i++) {
				Select_OutPoint(2, i+1, O_RUN_RELAY, OFF);
			}
			myGroup->phase = P41;
			break;
		case P41:
			myGroup->signal[G_SIG_NEXT_STEP_EXIST] = P0;
			myGroup->cmd[CMD_MODE_USER] = CMD_TYPE_NONE;
			myGroup->state = G_STANDBY;
			myGroup->phase = P0;
			break;
		case P50:
			j = myData->mData.config.chPerBd * group;
			for(i=j; i < j + chInGroup; i++) {
				if(myData->cData[i].op.state != C_STANDBY) {
					myData->cData[i].op.state = C_STANDBY;
					myData->cData[i].op.phase = P0;
				}
			}

			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
			SendMsg.msg = MSG_MODULE_JIGM_TRY_DISCONTACT;
			SendMsg.val[0] = group;
			send_msg(MODULE_TO_JIGM, (char *)&SendMsg);
			myGroup->phase = P51;
			break;
		case P51:
			if(myGroup->signal[G_SIG_JIG_FAIL] == P1) {
				myGroup->signal[G_SIG_JIG_FAIL] = P2;
				myGroup->cmd[CMD_MODE_USER] = CMD_TYPE_NONE;
				myGroup->state = G_STANDBY;
				myGroup->phase = P0;
			} else if(myGroup->signal[G_SIG_DISCONTACT_COMPLETE] == P1) {
				myGroup->signal[G_SIG_DISCONTACT_COMPLETE] = P0;
				myGroup->cmd[CMD_MODE_USER] = CMD_TYPE_NONE;
				myGroup->phase = P52;
			}
			break;
		case P52:
			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
			SendMsg.msg = MSG_MODULE_JIGM_TRY_CONTACT;
			SendMsg.val[0] = group;
			send_msg(MODULE_TO_JIGM, (char *)&SendMsg);

			stepNo = 0;
			myGroup->stepNo = 0;
			myGroup->advStepNo = 0;
			myGroup->cycleNo = 0;
			myGroup->advCycleNo = 0;
			myGroup->start_stepNo = 0;
			myGroup->saveDt = 0;
			myGroup->runTime = 0;
			myGroup->code = G_CD_NONE;

			ch = myGroup->misc.chOffset;
			idx = IDX_LOC_OBJ_CLIENT_STEP_NO;
			myGroup->client_stepNo = (int)myData->testCond[ch]
				.local_object[stepNo][idx];
			idx = IDX_LOC_OBJ_TYPE;
			myGroup->stepType = (unsigned char)myData->testCond[ch]
				.local_object[stepNo][idx];
			idx = IDX_LOC_OBJ_MODE;
			myGroup->stepMode = (unsigned char)myData->testCond[ch]
				.local_object[stepNo][idx];
			idx = IDX_LOC_OBJ_ATTRIBUTE;
			myGroup->attribute = (unsigned char)myData->testCond[ch]
				.local_object[stepNo][idx];

			myGroup->phase = P1;
			break;
		default:	break;
	}*/
}

void gRun_F3(int group)
{ //debug_size_cob
/*	int ch, i, j, stepNo, idx, chInGroup;
	int ch_standby, ch_run_hold, ch_fault, ch_pause, ch_pause2;
	int monitor_ch;
	S_MSG_VAL	SendMsg;

	chInGroup = myData->mData.config.chInGroup[group];
	ch = myGroup->misc.chOffset;

	switch(myGroup->phase) {
		case P0:
			if(myGroup->signal[G_SIG_DATA_SAVE_FAIL] == P1) {
				myGroup->signal[G_SIG_DATA_SAVE_FAIL] = P2;
				myGroup->cmd[CMD_MODE_USER] = CMD_TYPE_NONE;
				myGroup->state = G_STANDBY;
				myGroup->phase = P0;
				break;
			}

			if(myGroup->signal[G_SIG_SAVED_FILE_DELETE_COMPLETE] == P1) {
				myGroup->signal[G_SIG_SAVED_FILE_DELETE_COMPLETE] = P0;

				memset((char *)&myData->COB_opSave[group][0][0], 0,
					sizeof(S_CH_OP_DATA) * MAX_CH_PER_MODULE * MAX_P2_STEP);
				memset((char *)&myData->COB_save_step[group][0], 0,
					sizeof(int) * MAX_CH_PER_MODULE);

				myGroup->misc.total_ng_ch = 0;
				myGroup->misc.scan_ch[0] = 0;
				myGroup->misc.scan_step[0] = 0;
				myGroup->misc.scan_idxStepNo_offset[0] = 0;
				myGroup->misc.tmp_scan_step[0] = 0;
				myGroup->misc.scan_ch[1] = 0;
				myGroup->misc.scan_step[1] = 0;
				myGroup->misc.scan_idxStepNo_offset[1] = 0;
				myGroup->misc.tmp_scan_step[1] = 0;
				myGroup->misc.scan_end_step_flag = P0;
				memset((char *)&myGroup->misc.long_rest_area1[0], 0,
					sizeof(unsigned char) * MAX_CH_PER_MODULE);
				memset((char *)&myGroup->misc.long_rest_area2[0], 0,
					sizeof(unsigned char) * MAX_CH_PER_MODULE);
				memset((char *)&myGroup->misc.long_rest_time1[0], 0,
					sizeof(long) * MAX_CH_PER_MODULE);
				memset((char *)&myGroup->misc.long_rest_time2[0], 0,
					sizeof(long) * MAX_CH_PER_MODULE);

				//ocv1 save(daq Vsens)
				memset((char *)&myData->lineCheck[group][0], 0,
					sizeof(S_LINE_CHECK) * MAX_CH_PER_MODULE);
				for(i=0; i < 12; i++) {
					myData->lineCheck[group][i].avg_val[0]
						= myData->SubSensV.ch[i].sensV;
				}
				for(i=12; i < 24; i++) {
					myData->lineCheck[group][i].avg_val[0]
						= myData->SubSensV.ch[i+20].sensV;
				}
				myGroup->signal[G_SIG_LOAD_LINE_CHECK_DELAY] = 0;
				myGroup->signal[G_SIG_SAFETY_RESISTOR] = SR_CLOSE;

				memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
				SendMsg.msg = MSG_MODULE_JIGM_TRY_CONTACT;
				SendMsg.val[0] = group;
				send_msg(MODULE_TO_JIGM, (char *)&SendMsg);
				myGroup->phase = P1;

				if(myGroup->cmd[CMD_MODE_USER] == CMD_TYPE_STOP) {
					myGroup->cmd[CMD_MODE_USER] = CMD_TYPE_NONE;
					myGroup->state = G_STANDBY;
					myGroup->phase = P0;
				} else {
					myGroup->cmd[CMD_MODE_USER] = CMD_TYPE_NONE;
				}
			} else {
				if(myGroup->code != G_CD_NONE) {
					myGroup->state = G_STANDBY;
					myGroup->phase = P0;

					memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
					SendMsg.msg = MSG_MODULE_JIGM_TRY_DISCONTACT;
					SendMsg.val[0] = group;
					send_msg(MODULE_TO_JIGM, (char *)&SendMsg);
				}
			}
			break;
		case P1:
			if(myGroup->signal[G_SIG_JIG_FAIL] == P1) {
				myGroup->signal[G_SIG_JIG_FAIL] = P2;
				myGroup->cmd[CMD_MODE_USER] = CMD_TYPE_NONE;
				myGroup->state = G_STANDBY;
				myGroup->phase = P0;
				break;
			}

			if(myGroup->signal[G_SIG_LOAD_LINE_CHECK_DELAY] == 30) {
				//ocv2 save(daq Vsens)
				for(i=0; i < 12; i++) {
					myData->lineCheck[group][i].avg_val[1]
						= myData->SubSensV.ch[i].sensV;
				}
				for(i=12; i < 24; i++) {
					myData->lineCheck[group][i].avg_val[1]
						= myData->SubSensV.ch[i+20].sensV;
				}
				myGroup->signal[G_SIG_LOAD_LINE_CHECK_DELAY]++;
			} else if(myGroup->signal[G_SIG_LOAD_LINE_CHECK_DELAY] == 31) {
				myGroup->signal[G_SIG_SAFETY_RESISTOR] = SR_OPEN;
				myGroup->signal[G_SIG_LOAD_LINE_CHECK_DELAY]++;
			} else if(myGroup->signal[G_SIG_LOAD_LINE_CHECK_DELAY] == 32) {
			} else {
				myGroup->signal[G_SIG_LOAD_LINE_CHECK_DELAY]++;
			}

			if(myGroup->signal[G_SIG_CONTACT_COMPLETE] == P1) {
				myGroup->signal[G_SIG_CONTACT_COMPLETE] = P0;
				if(myGroup->cmd[CMD_MODE_USER] == CMD_TYPE_STOP) {
					memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
					SendMsg.msg = MSG_MODULE_JIGM_TRY_DISCONTACT;
					SendMsg.val[0] = group;
					send_msg(MODULE_TO_JIGM, (char *)&SendMsg);

					myGroup->cmd[CMD_MODE_USER] = CMD_TYPE_NONE;
					myGroup->state = G_STANDBY;
					myGroup->phase = P0;
				} else {
					myGroup->signal[G_SIG_LOAD_LINE_CHECK_DELAY] = 0;
					myGroup->signal[G_SIG_LOAD_LINE_CHECK_PHASE] = P0;
					myGroup->phase = P5;
				}
			} else {
				if(myGroup->code != G_CD_NONE) {
					memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
					SendMsg.msg = MSG_MODULE_JIGM_TRY_DISCONTACT;
					SendMsg.val[0] = group;
					send_msg(MODULE_TO_JIGM, (char *)&SendMsg);

					myGroup->state = G_STANDBY;
					myGroup->phase = P0;
				}
			}
			break;
		case P2:
			if(myGroup->signal[G_SIG_JIG_FAIL] == P1) {
				myGroup->signal[G_SIG_JIG_FAIL] = P2;
				myGroup->cmd[CMD_MODE_USER] = CMD_TYPE_NONE;
				myGroup->state = G_STANDBY;
				myGroup->phase = P0;
				break;
			}

			if(myGroup->signal[G_SIG_CONTACT_COMPLETE] == P1) {
				myGroup->signal[G_SIG_CONTACT_COMPLETE] = P0;
				if(myGroup->cmd[CMD_MODE_USER] == CMD_TYPE_STOP) {
					myGroup->cmd[CMD_MODE_USER] = CMD_TYPE_NONE;

					memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
					SendMsg.msg = MSG_MODULE_JIGM_TRY_DISCONTACT;
					SendMsg.val[0] = group;
					send_msg(MODULE_TO_JIGM, (char *)&SendMsg);

					myGroup->state = G_STANDBY;
					myGroup->phase = P0;
					break;
				} else {
					myGroup->cmd[CMD_MODE_USER] = CMD_TYPE_NONE;
				}

				if(myGroup->attribute == ATTR_CHECK1_END
					|| myGroup->attribute == ATTR_CHECK2_END
					|| myGroup->attribute == ATTR_CHECK3_END) {
					myGroup->stepNo++;
					stepNo = myGroup->stepNo;
					myGroup->advStepNo++;
					myGroup->cycleNo++;
					myGroup->advCycleNo = 0;

					idx = IDX_LOC_OBJ_CLIENT_STEP_NO;
					myGroup->client_stepNo = (int)myData->testCond[ch]
						.local_object[stepNo][idx];
					idx = IDX_LOC_OBJ_TYPE;
					myGroup->stepType = (unsigned char)myData->testCond[ch]
						.local_object[stepNo][idx];
					idx = IDX_LOC_OBJ_MODE;
					myGroup->stepMode = (unsigned char)myData->testCond[ch]
						.local_object[stepNo][idx];
					idx = IDX_LOC_OBJ_ATTRIBUTE;
					myGroup->attribute = (unsigned char)myData->testCond[ch]
						.local_object[stepNo][idx];
				}

				if(myData->cData[0].op.code == C_CD_FAULT_PAUSE_CMD) {
					myData->cData[0].signal[C_SIG_CMD_CONTINUE] = P1;
				} else {
					myData->cData[0].signal[C_SIG_CMD_RUN] = P1;
				}
				if(myData->cData[1].op.code == C_CD_FAULT_PAUSE_CMD) {
					myData->cData[1].signal[C_SIG_CMD_CONTINUE] = P1;
				} else {
					myData->cData[1].signal[C_SIG_CMD_RUN] = P1;
				}
				myGroup->signal[G_SIG_SAFETY_RESISTOR] = SR_CLOSE;
				myGroup->phase = P10;
			} else {
				if(myGroup->cmd[CMD_MODE_USER] == CMD_TYPE_STOP) {
					myGroup->cmd[CMD_MODE_USER] = CMD_TYPE_NONE;

					memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
					SendMsg.msg = MSG_MODULE_JIGM_TRY_DISCONTACT;
					SendMsg.val[0] = group;
					send_msg(MODULE_TO_JIGM, (char *)&SendMsg);

					myGroup->state = G_STANDBY;
					myGroup->phase = P0;
					break;
				} else {
					myGroup->cmd[CMD_MODE_USER] = CMD_TYPE_NONE;
				}
			}
			break;
		case P3:
			if(myGroup->attribute == ATTR_CHECK1_END
				|| myGroup->attribute == ATTR_CHECK2_END
				|| myGroup->attribute == ATTR_CHECK3_END) {
				myGroup->stepNo++;
				stepNo = myGroup->stepNo;
				myGroup->advStepNo++;
				myGroup->cycleNo++;
				myGroup->advCycleNo = 0;

				idx = IDX_LOC_OBJ_CLIENT_STEP_NO;
				myGroup->client_stepNo = (int)myData->testCond[ch]
					.local_object[stepNo][idx];
				idx = IDX_LOC_OBJ_TYPE;
				myGroup->stepType = (unsigned char)myData->testCond[ch]
					.local_object[stepNo][idx];
				idx = IDX_LOC_OBJ_MODE;
				myGroup->stepMode = (unsigned char)myData->testCond[ch]
					.local_object[stepNo][idx];
				idx = IDX_LOC_OBJ_ATTRIBUTE;
				myGroup->attribute = (unsigned char)myData->testCond[ch]
					.local_object[stepNo][idx];
			}
			myGroup->phase = P10;
			break;
		case P5: //Load Line Check
			if(myGroup->cmd[CMD_MODE_USER] == CMD_TYPE_STOP) {
				myGroup->cmd[CMD_MODE_USER] = CMD_TYPE_NONE;

				memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
				SendMsg.msg = MSG_MODULE_JIGM_TRY_DISCONTACT;
				SendMsg.val[0] = group;
				send_msg(MODULE_TO_JIGM, (char *)&SendMsg);

				i = gLoadLineCheck(group, 1);

				myGroup->state = G_STANDBY;
				myGroup->phase = P0;
				break;
			}

			if(gLoadLineCheck(group, 0) > 0) {
				if(myData->AppControl.config.debugType == 0) {
					monitor_ch = (int)myData->COB_Client[group].config
						.ChArray2[myGroup->misc.scan_ch[0]];
					Select_OutPoint(2, monitor_ch+1, O_RUN_RELAY, ON);
					monitor_ch = (int)myData->COB_Client[group].config
						.ChArray2[myGroup->misc.scan_ch[1]];
					Select_OutPoint(3, monitor_ch+1, O_RUN_RELAY, ON);
				}
				myGroup->signal[G_SIG_LOAD_LINE_CHECK_PHASE] = P0;
				myGroup->phase = P6;
			}
			break;
		case P6:
			myGroup->signal[G_SIG_LOAD_LINE_CHECK_PHASE]++;
			if(myGroup->signal[G_SIG_LOAD_LINE_CHECK_PHASE] >= P30) {
				//10ms * 30 = 0.3sec
				myGroup->phase = P7;
			}
			break;
		case P7:
			myData->mData.signal[M_SIG_RUNNING_GROUP] = (unsigned char)group;
			myData->cData[0].signal[C_SIG_CMD_RUN] = P1;
			myData->cData[1].signal[C_SIG_CMD_RUN] = P1;
			myGroup->cmd[CMD_MODE_USER] = CMD_TYPE_NONE;

			stepNo = myGroup->stepNo;
			myGroup->advCycleNo = 0;

			idx = IDX_LOC_OBJ_CLIENT_STEP_NO;
			myGroup->client_stepNo = (int)myData->testCond[ch]
				.local_object[stepNo][idx];
			idx = IDX_LOC_OBJ_TYPE;
			myGroup->stepType = (unsigned char)myData->testCond[ch]
				.local_object[stepNo][idx];
			idx = IDX_LOC_OBJ_MODE;
			myGroup->stepMode = (unsigned char)myData->testCond[ch]
				.local_object[stepNo][idx];
			idx = IDX_LOC_OBJ_ATTRIBUTE;
			myGroup->attribute = (unsigned char)myData->testCond[ch]
				.local_object[stepNo][idx];

			myGroup->phase = P10;
			myGroup->runTime = 0;
			myGroup->saveDt = 0;
			break;
		case P10: //Running
			myGroup->runTime += myPs->config.scan_period;

			if(myGroup->signal[G_SIG_JIG_FAIL] == P1) {
				myGroup->signal[G_SIG_JIG_FAIL] = P2;
				myGroup->cmd[CMD_MODE_USER] = CMD_TYPE_PAUSE;
			}

			if(myGroup->cmd[CMD_MODE_USER] == CMD_TYPE_STOP) {
				myGroup->cmd[CMD_MODE_USER] = CMD_TYPE_NONE;
				myGroup->signal[G_SIG_CMD_STOP] = P1;
				j = 0;
				for(i=j; i < j + chInGroup; i++) {
					if(myData->cData[i].op.state == C_RUN
						|| myData->cData[i].op.state == C_PAUSE) {
						myData->cData[i].signal[C_SIG_CMD_STOP] = P1;
					}
				}
				myGroup->phase = P30;
				break;
			} else if(myGroup->cmd[CMD_MODE_USER] == CMD_TYPE_PAUSE) {
				myGroup->cmd[CMD_MODE_USER] = CMD_TYPE_NONE;
				j = 0;
				for(i=j; i < j + chInGroup; i++) {
					if(myData->cData[i].op.state == C_RUN) {
						myData->cData[i].signal[C_SIG_CMD_PAUSE] = P1;
					}
				}
				myGroup->phase = P32;
				break;
			} else if(myGroup->cmd[CMD_MODE_USER] == CMD_TYPE_NEXT_STEP) {
				myGroup->cmd[CMD_MODE_USER] = CMD_TYPE_NONE;
				myGroup->signal[G_SIG_CMD_NEXT_STEP] = P1;
				j = 0;
				for(i=j; i < j + chInGroup; i++) {
					myData->cData[i].signal[C_SIG_CMD_NEXT_STEP] = P1;
				}
				break;
			} else {
				myGroup->cmd[CMD_MODE_USER] = CMD_TYPE_NONE;
			}

			send_save_realData_msg(group);

			gRun_F_SDI_5V_450A_200A_100A_10A(group);
			break;
		case P11:
			if(myGroup->cmd[CMD_MODE_USER] == CMD_TYPE_STOP) {
				myGroup->cmd[CMD_MODE_USER] = CMD_TYPE_NONE;
				myGroup->signal[G_SIG_CMD_STOP] = P1;

				j = myData->mData.config.chPerBd * group;
				for(i=j; i < j + chInGroup; i++) {
					myData->cData[i].signal[C_SIG_CMD_STOP] = P1;
				}
				break;
			}

			if(myGroup->signal[G_SIG_CMD_NEXT_STEP] == P1) {
				myGroup->signal[G_SIG_CMD_NEXT_STEP] = P0;
				myGroup->phase = P12;
				break;
			}

			ch_standby = ch_run_hold = ch_fault = ch_pause = 0;
			ch_pause2 = 0;
			j = myData->mData.config.chPerBd * group;
			for(i=j; i < j + chInGroup; i++) {
				if(myData->cData[i].op.state == C_STANDBY) {
					ch_standby++;
				} else if(myData->cData[i].op.state == C_RUN
					&& myData->cData[i].op.phase == P102) {
					ch_run_hold++;
				} else if(myData->cData[i].op.state == C_PAUSE) {
					if(myData->cData[i].op.attribute == ATTR_CHECK1_END
						|| myData->cData[i].op.attribute == ATTR_CHECK2_END
						|| myData->cData[i].op.attribute == ATTR_CHECK3_END) {
						ch_pause2++;
					} else {
						if(myData->cData[i].op.code == C_CD_FAULT_PAUSE_CMD)
							ch_pause++;
						else ch_fault++;
					}
				}
			}

			if(chInGroup == ch_fault) { //all ch fault
				if(myGroup->signal[G_SIG_CMD_STOP] == P0) {
					myGroup->signal[G_SIG_ALL_CHANNEL_ERROR] = P1;
					myGroup->phase = P30;//khkw
				} else {
					myGroup->signal[G_SIG_CMD_STOP] = P0;
					myGroup->phase = P20;
				}
			} else if(chInGroup == (ch_standby)) { //normal
				myGroup->phase = P20;
			} else if(chInGroup == (ch_pause2 + ch_fault)) { //pause
				myGroup->phase = P23;
			} else if(chInGroup == (ch_run_hold + ch_pause2 + ch_fault)) { //normal
				myGroup->phase = P12;
			} else if(chInGroup == (ch_pause + ch_fault)) { //pause
				myGroup->phase = P23;
			}
			break;
		case P12:
			switch(myGroup->stepType) {
				case STEP_CYCLE:
					myGroup->start_stepNo = myGroup->stepNo;
					myGroup->stepNo++;
					stepNo = myGroup->stepNo;
					myGroup->advStepNo++;

					idx = IDX_LOC_OBJ_CLIENT_STEP_NO;
					myGroup->client_stepNo = (int)myData->testCond[ch]
						.local_object[stepNo][idx];
					idx = IDX_LOC_OBJ_TYPE;
					myGroup->stepType = (unsigned char)myData->testCond[ch]
						.local_object[stepNo][idx];
					idx = IDX_LOC_OBJ_MODE;
					myGroup->stepMode = (unsigned char)myData->testCond[ch]
						.local_object[stepNo][idx];
					idx = IDX_LOC_OBJ_ATTRIBUTE;
					myGroup->attribute = (unsigned char)myData->testCond[ch]
						.local_object[stepNo][idx];

					j = myData->mData.config.chPerBd * group;
					for(i=j; i < j + chInGroup; i++) {
						if(myData->cData[i].op.state == C_RUN
							&& myData->cData[i].op.phase == P102) {
							myData->cData[i].op.phase = P101;
						} else if(myData->cData[i].op.state == C_PAUSE
							&& myData->cData[i].op.phase == P102) {
							myData->cData[i].op.phase = P101;
						}
					}
					myGroup->phase = P10;
					break;
				case STEP_LOOP:
					if(myGroup->attribute == ATTR_CHECK1_END
						|| myGroup->attribute == ATTR_CHECK2_END
						|| myGroup->attribute == ATTR_CHECK3_END) {
						if(myGroup->workMode == WORK_TYPE_ONLINE) {
							myGroup->phase = P3;
						} else {
							myGroup->phase = P24;
						}
					} else {
						myGroup->stepNo++;
						stepNo = myGroup->stepNo;
						myGroup->advStepNo++;

						idx = IDX_LOC_OBJ_CLIENT_STEP_NO;
						myGroup->client_stepNo = (int)myData->testCond[ch]
							.local_object[stepNo][idx];
						idx = IDX_LOC_OBJ_TYPE;
						myGroup->stepType = (unsigned char)myData->testCond[ch]
							.local_object[stepNo][idx];
						idx = IDX_LOC_OBJ_MODE;
						myGroup->stepMode = (unsigned char)myData->testCond[ch]
							.local_object[stepNo][idx];
						idx = IDX_LOC_OBJ_ATTRIBUTE;
						myGroup->attribute = (unsigned char)myData->testCond[ch]
							.local_object[stepNo][idx];

						j = myData->mData.config.chPerBd * group;
						for(i=j; i < j + chInGroup; i++) {
							if(myData->cData[i].op.state == C_RUN
								&& myData->cData[i].op.phase == P102) {
								myData->cData[i].op.phase = P101;
							} else if(myData->cData[i].op.state == C_PAUSE
								&& myData->cData[i].op.phase == P102) {
								myData->cData[i].op.phase = P101;
							}
						}
						myGroup->phase = P10;
					}
					break;
				case STEP_END:
					stepNo = myGroup->stepNo;
					idx = IDX_LOC_OBJ_CLIENT_STEP_NO;
					myGroup->client_stepNo = (int)myData->testCond[ch]
						.local_object[stepNo][idx];
					idx = IDX_LOC_OBJ_ATTRIBUTE;
					myGroup->attribute = (unsigned char)myData->testCond[ch]
						.local_object[stepNo][idx];

					j = myData->mData.config.chPerBd * group;
					for(i=j; i < j + chInGroup; i++) {
						if(myData->cData[i].op.state == C_RUN
							&& myData->cData[i].op.phase == P102) {
							myData->cData[i].op.phase = P101;
						} else if(myData->cData[i].op.state == C_PAUSE
							&& myData->cData[i].op.phase == P102) {
							myData->cData[i].op.phase = P101;
						} else if(myData->cData[i].op.state == C_PAUSE) {
							myData->cData[i].op.state = C_STANDBY;
							myData->cData[i].op.phase = P0;
						}
					}
					myGroup->phase = P20;
					break;
				case STEP_OCV:
				case STEP_REST:
				case STEP_LONG_TIME_REST:
				case STEP_CHARGE:
				case STEP_DISCHARGE:
					myGroup->stepNo++;
					stepNo = myGroup->stepNo;
					myGroup->advStepNo++;

					idx = IDX_LOC_OBJ_CLIENT_STEP_NO;
					myGroup->client_stepNo = (int)myData->testCond[ch]
						.local_object[stepNo][idx];
					idx = IDX_LOC_OBJ_TYPE;
					myGroup->stepType = (unsigned char)myData->testCond[ch]
						.local_object[stepNo][idx];
					idx = IDX_LOC_OBJ_MODE;
					myGroup->stepMode = (unsigned char)myData->testCond[ch]
						.local_object[stepNo][idx];
					idx = IDX_LOC_OBJ_ATTRIBUTE;
					myGroup->attribute = (unsigned char)myData->testCond[ch]
						.local_object[stepNo][idx];

					j = myData->mData.config.chPerBd * group;
					for(i=j; i < j + chInGroup; i++) {
						if(myData->cData[i].op.state == C_RUN
							&& myData->cData[i].op.phase == P102) {
							myData->cData[i].op.phase = P101;
						} else if(myData->cData[i].op.state == C_PAUSE
							&& myData->cData[i].op.phase == P102) {
							myData->cData[i].op.phase = P101;
						}
					}
					myGroup->phase = P10;
					break;
				default:	break;
			}
			myGroup->runTime = 0;
			myGroup->saveDt = 0;
			break;
		case P13:
			for(i=0; i < chInGroup; i++) {
				Select_OutPoint(2, i+1, O_RUN_RELAY, OFF);
				Select_OutPoint(3, i+1, O_RUN_RELAY, OFF);
			}

			myGroup->state = G_PAUSE;
			myGroup->phase = P10;
			break;
		case P20:
			if(myGroup->cmd[CMD_MODE_USER] == CMD_TYPE_STOP) {
				myGroup->cmd[CMD_MODE_USER] = CMD_TYPE_NONE;
			}
			if(myGroup->attribute == ATTR_TOTAL_CHECK) {
				send_save_msg_formation(group, 0);
				myGroup->phase = P30;
			} else { //normal end
				if(myGroup->workMode == WORK_TYPE_OFFLINE)  {
					myGroup->phase = P30;
				} else { //WORK_ONLINE	
					myGroup->phase = P40;
				}
			}
			break;
		case P23:
			myGroup->phase = P32;
			break;
		case P24:
			for(i=0; i < chInGroup; i++) {
				Select_OutPoint(2, i+1, O_RUN_RELAY, OFF);
				Select_OutPoint(3, i+1, O_RUN_RELAY, OFF);
			}

			myGroup->state = G_PAUSE;
			myGroup->phase = P0;
			break;
		case P30:
			for(i=0; i < chInGroup; i++) {
				Select_OutPoint(2, i+1, O_RUN_RELAY, OFF);
				Select_OutPoint(3, i+1, O_RUN_RELAY, OFF);
			}

			if(myData->cData[0].op.state != C_STANDBY) {
				myData->cData[0].op.state = C_STANDBY;
				myData->cData[0].op.phase = P0;
			}
			if(myData->cData[1].op.state != C_STANDBY) {
				myData->cData[1].op.state = C_STANDBY;
				myData->cData[1].op.phase = P0;
			}

			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
			SendMsg.msg = MSG_MODULE_JIGM_TRY_DISCONTACT;
			SendMsg.val[0] = group;
			send_msg(MODULE_TO_JIGM, (char *)&SendMsg);

			myGroup->phase = P31;
			break;
		case P31:
			if(myGroup->signal[G_SIG_JIG_FAIL] == P1) {
				myGroup->signal[G_SIG_JIG_FAIL] = P2;
				myGroup->cmd[CMD_MODE_USER] = CMD_TYPE_NONE;
				myGroup->state = G_STANDBY;
				myGroup->phase = P0;
			} else if(myGroup->signal[G_SIG_DISCONTACT_COMPLETE] == P1) {
				myGroup->signal[G_SIG_DISCONTACT_COMPLETE] = P0;
				myGroup->cmd[CMD_MODE_USER] = CMD_TYPE_NONE;
				myGroup->state = G_STANDBY;
				myGroup->phase = P0;
			}
			break;
		case P32:
			for(i=0; i < chInGroup; i++) {
				Select_OutPoint(2, i+1, O_RUN_RELAY, OFF);
				Select_OutPoint(3, i+1, O_RUN_RELAY, OFF);
			}

			myGroup->state = G_PAUSE;
			myGroup->phase = P0;
			break;
		case P40:
			for(i=0; i < chInGroup; i++) {
				Select_OutPoint(2, i+1, O_RUN_RELAY, OFF);
				Select_OutPoint(3, i+1, O_RUN_RELAY, OFF);
			}
			myGroup->phase = P41;
			break;
		case P41:
			myGroup->signal[G_SIG_NEXT_STEP_EXIST] = P0;
			myGroup->cmd[CMD_MODE_USER] = CMD_TYPE_NONE;
			myGroup->state = G_STANDBY;
			myGroup->phase = P0;
			break;
		case P50:
			j = myData->mData.config.chPerBd * group;
			for(i=j; i < j + chInGroup; i++) {
				if(myData->cData[i].op.state != C_STANDBY) {
					myData->cData[i].op.state = C_STANDBY;
					myData->cData[i].op.phase = P0;
				}
			}

			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
			SendMsg.msg = MSG_MODULE_JIGM_TRY_DISCONTACT;
			SendMsg.val[0] = group;
			send_msg(MODULE_TO_JIGM, (char *)&SendMsg);
			myGroup->phase = P51;
			break;
		case P51:
			if(myGroup->signal[G_SIG_JIG_FAIL] == P1) {
				myGroup->signal[G_SIG_JIG_FAIL] = P2;
				myGroup->cmd[CMD_MODE_USER] = CMD_TYPE_NONE;
				myGroup->state = G_STANDBY;
				myGroup->phase = P0;
			} else if(myGroup->signal[G_SIG_DISCONTACT_COMPLETE] == P1) {
				myGroup->signal[G_SIG_DISCONTACT_COMPLETE] = P0;
				myGroup->cmd[CMD_MODE_USER] = CMD_TYPE_NONE;
				myGroup->phase = P52;
			}
			break;
		case P52:
			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
			SendMsg.msg = MSG_MODULE_JIGM_TRY_CONTACT;
			SendMsg.val[0] = group;
			send_msg(MODULE_TO_JIGM, (char *)&SendMsg);

			stepNo = 0;
			myGroup->stepNo = 0;
			myGroup->advStepNo = 0;
			myGroup->cycleNo = 0;
			myGroup->advCycleNo = 0;
			myGroup->start_stepNo = 0;
			myGroup->saveDt = 0;
			myGroup->runTime = 0;
			myGroup->code = G_CD_NONE;

			ch = myGroup->misc.chOffset;
			idx = IDX_LOC_OBJ_CLIENT_STEP_NO;
			myGroup->client_stepNo = (int)myData->testCond[ch]
				.local_object[stepNo][idx];
			idx = IDX_LOC_OBJ_TYPE;
			myGroup->stepType = (unsigned char)myData->testCond[ch]
				.local_object[stepNo][idx];
			idx = IDX_LOC_OBJ_MODE;
			myGroup->stepMode = (unsigned char)myData->testCond[ch]
				.local_object[stepNo][idx];
			idx = IDX_LOC_OBJ_ATTRIBUTE;
			myGroup->attribute = (unsigned char)myData->testCond[ch]
				.local_object[stepNo][idx];

			myGroup->phase = P1;
			break;
		default:	break;
	}*/
}

void gRun_F4(int group)
{ //debug_size_cob
/*	int type, point, bd, ch_2;
   	int ch, i, j, stepNo, idx, chInGroup, cmp_count, ch_code_check;
	int ch_standby, ch_run_hold, ch_fault, ch_pause, ch_running, ch_pause2;
	int monitor_ch, range;
	long val;
	double sum;
	S_MSG_VAL	SendMsg;

	chInGroup = myData->mData.config.chInGroup[group];
	ch = myGroup->misc.chOffset;

	switch(myGroup->phase) {
		case P0:
			if(myGroup->signal[G_SIG_DATA_SAVE_FAIL] == P1) {
				myGroup->signal[G_SIG_DATA_SAVE_FAIL] = P2;
				myGroup->cmd[CMD_MODE_USER] = CMD_TYPE_NONE;
				myGroup->state = G_STANDBY;
				myGroup->phase = P0;
				break;
			}

			if(myGroup->signal[G_SIG_SAVED_FILE_DELETE_COMPLETE] == P1) {
				myGroup->signal[G_SIG_SAVED_FILE_DELETE_COMPLETE] = P0;

				memset((char *)&myData->COB_opSave[group][0][0], 0,
					sizeof(S_CH_OP_DATA) * MAX_CH_256 * MAX_P2_STEP);
				memset((char *)&myData->COB_save_step[group][0], 0,
					sizeof(int) * MAX_CH_256);

				myGroup->misc.total_ng_ch = 0;
				myGroup->misc.scan_ch[0] = 0;
				myGroup->misc.scan_step[0] = 0;
				myGroup->misc.scan_idxStepNo_offset[0] = 0;
				myGroup->misc.tmp_scan_step[0] = 0;
				myGroup->misc.scan_ch[1] = 0;
				myGroup->misc.scan_step[1] = 0;
				myGroup->misc.scan_idxStepNo_offset[1] = 0;
				myGroup->misc.tmp_scan_step[1] = 0;
				myGroup->misc.scan_end_step_flag = P0;
				memset((char *)&myGroup->misc.long_rest_area1[0], 0,
					sizeof(unsigned char) * MAX_CH_PER_MODULE);
				memset((char *)&myGroup->misc.long_rest_area2[0], 0,
					sizeof(unsigned char) * MAX_CH_PER_MODULE);
				memset((char *)&myGroup->misc.long_rest_time1[0], 0,
					sizeof(long) * MAX_CH_PER_MODULE);
				memset((char *)&myGroup->misc.long_rest_time2[0], 0,
					sizeof(long) * MAX_CH_PER_MODULE);

				//ocv1 save(daq Vsens)
				memset((char *)&myData->lineCheck[group][0], 0,
					sizeof(S_LINE_CHECK) * MAX_CH_256);
				for(i=0; i < 12; i++) {
					myData->lineCheck[group][i].avg_val[0]
						= myData->SubSensV.ch[i].sensV;
				}
				for(i=12; i < 24; i++) {
					myData->lineCheck[group][i].avg_val[0]
						= myData->SubSensV.ch[i+20].sensV;
				}
				myGroup->signal[G_SIG_LOAD_LINE_CHECK_DELAY] = 0;
				myGroup->signal[G_SIG_SAFETY_RESISTOR] = SR_CLOSE;

				memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
				SendMsg.msg = MSG_MODULE_JIGM_TRY_CONTACT;
				SendMsg.val[0] = group;
				send_msg(MODULE_TO_JIGM, (char *)&SendMsg);
				myGroup->phase = P1;

				if(myGroup->cmd[CMD_MODE_USER] == CMD_TYPE_STOP) {
					myGroup->cmd[CMD_MODE_USER] = CMD_TYPE_NONE;
					myGroup->state = G_STANDBY;
					myGroup->phase = P0;
				} else {
					myGroup->cmd[CMD_MODE_USER] = CMD_TYPE_NONE;
				}
				break;
			}

			if(myGroup->code != G_CD_NONE) {
				memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
				SendMsg.msg = MSG_MODULE_JIGM_TRY_DISCONTACT;
				SendMsg.val[0] = group;
				send_msg(MODULE_TO_JIGM, (char *)&SendMsg);

				myGroup->state = G_STANDBY;
				myGroup->phase = P0;
				break;
			}
			break;
		case P1:
			if(myGroup->signal[G_SIG_JIG_FAIL] == P1) {
				myGroup->signal[G_SIG_JIG_FAIL] = P2;
				myGroup->cmd[CMD_MODE_USER] = CMD_TYPE_NONE;
				myGroup->state = G_STANDBY;
				myGroup->phase = P0;
				break;
			}

			if(myGroup->signal[G_SIG_LOAD_LINE_CHECK_DELAY] == 30) {
				//ocv2 save(daq Vsens)
				for(i=0; i < 12; i++) {
					myData->lineCheck[group][i].avg_val[1]
						= myData->SubSensV.ch[i].sensV;
				}
				for(i=12; i < 24; i++) {
					myData->lineCheck[group][i].avg_val[1]
						= myData->SubSensV.ch[i+20].sensV;
				}
				myGroup->signal[G_SIG_LOAD_LINE_CHECK_DELAY]++;
			} else if(myGroup->signal[G_SIG_LOAD_LINE_CHECK_DELAY] == 31) {
				myGroup->signal[G_SIG_SAFETY_RESISTOR] = SR_OPEN;
				myGroup->signal[G_SIG_LOAD_LINE_CHECK_DELAY]++;
			} else if(myGroup->signal[G_SIG_LOAD_LINE_CHECK_DELAY] == 32) {
			} else {
				myGroup->signal[G_SIG_LOAD_LINE_CHECK_DELAY]++;
			}

			if(myGroup->signal[G_SIG_CONTACT_COMPLETE] == P1) {
				myGroup->signal[G_SIG_CONTACT_COMPLETE] = P0;
				if(myGroup->cmd[CMD_MODE_USER] == CMD_TYPE_STOP) {
					memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
					SendMsg.msg = MSG_MODULE_JIGM_TRY_DISCONTACT;
					SendMsg.val[0] = group;
					send_msg(MODULE_TO_JIGM, (char *)&SendMsg);

					myGroup->cmd[CMD_MODE_USER] = CMD_TYPE_NONE;
					myGroup->state = G_STANDBY;
					myGroup->phase = P0;
				} else {
					for(i=ch; i < chInGroup; i++) {
						if(myData->ChAttribute[0].opType == 0) {
						} else {
							if((i % 2) != 0) continue;
						}

						myData->cData[i].signal[C_SIG_CMD_RUN] = P1;
					}
					myGroup->cmd[CMD_MODE_USER] = CMD_TYPE_NONE;

					stepNo = myGroup->stepNo;
					myGroup->advCycleNo = 0;

					idx = IDX_LOC_OBJ_CLIENT_STEP_NO;
					myGroup->client_stepNo = (int)myData->testCond[ch]
						.local_object[stepNo][idx];
					idx = IDX_LOC_OBJ_TYPE;
					myGroup->stepType = (unsigned char)myData->testCond[ch]
						.local_object[stepNo][idx];
					idx = IDX_LOC_OBJ_MODE;
					myGroup->stepMode = (unsigned char)myData->testCond[ch]
						.local_object[stepNo][idx];
					idx = IDX_LOC_OBJ_ATTRIBUTE;
					myGroup->attribute = (unsigned char)myData->testCond[ch]
						.local_object[stepNo][idx];

					myGroup->phase = P10;
					myGroup->runTime = 0;
					myGroup->saveDt = 0;
				}
			}

			if(myGroup->code != G_CD_NONE) {
				memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
				SendMsg.msg = MSG_MODULE_JIGM_TRY_DISCONTACT;
				SendMsg.val[0] = group;
				send_msg(MODULE_TO_JIGM, (char *)&SendMsg);

				myGroup->state = G_STANDBY;
				myGroup->phase = P0;
			}
			break;
		case P2:
			if(myGroup->signal[G_SIG_JIG_FAIL] == P1) {
				myGroup->signal[G_SIG_JIG_FAIL] = P2;
				myGroup->cmd[CMD_MODE_USER] = CMD_TYPE_NONE;
				myGroup->state = G_STANDBY;
				myGroup->phase = P0;
				break;
			}

			if(myGroup->signal[G_SIG_CONTACT_COMPLETE] == P1) {
				myGroup->signal[G_SIG_CONTACT_COMPLETE] = P0;
				if(myGroup->cmd[CMD_MODE_USER] == CMD_TYPE_STOP) {
					myGroup->cmd[CMD_MODE_USER] = CMD_TYPE_NONE;

					memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
					SendMsg.msg = MSG_MODULE_JIGM_TRY_DISCONTACT;
					SendMsg.val[0] = group;
					send_msg(MODULE_TO_JIGM, (char *)&SendMsg);

					myGroup->state = G_STANDBY;
					myGroup->phase = P0;
					break;
				} else {
					myGroup->cmd[CMD_MODE_USER] = CMD_TYPE_NONE;
				}

				if(myGroup->attribute == ATTR_CHECK1_END
					|| myGroup->attribute == ATTR_CHECK2_END
					|| myGroup->attribute == ATTR_CHECK3_END) {
					myGroup->stepNo++;
					stepNo = myGroup->stepNo;
					myGroup->advStepNo++;
					myGroup->cycleNo++;
					myGroup->advCycleNo = 0;

					idx = IDX_LOC_OBJ_CLIENT_STEP_NO;
					myGroup->client_stepNo = (int)myData->testCond[ch]
						.local_object[stepNo][idx];
					idx = IDX_LOC_OBJ_TYPE;
					myGroup->stepType = (unsigned char)myData->testCond[ch]
						.local_object[stepNo][idx];
					idx = IDX_LOC_OBJ_MODE;
					myGroup->stepMode = (unsigned char)myData->testCond[ch]
						.local_object[stepNo][idx];
					idx = IDX_LOC_OBJ_ATTRIBUTE;
					myGroup->attribute = (unsigned char)myData->testCond[ch]
						.local_object[stepNo][idx];
				}

				for(i=ch; i < chInGroup; i++) {
					if(myData->ChAttribute[0].opType == 0) {
					} else {
						if((i % 2) != 0) continue;
					}

					if(myData->cData[i].op.state == C_PAUSE) {
						if(myData->cData[i].op.phase == P1
							|| myData->cData[i].op.phase == P102) {
							myData->cData[i].signal[C_SIG_CMD_CONTINUE] = P1;
						}
					}
				}

				myGroup->signal[G_SIG_SAFETY_RESISTOR] = SR_CLOSE;
				myGroup->phase = P10;
				break;
			}

			if(myGroup->cmd[CMD_MODE_USER] == CMD_TYPE_STOP) {
				myGroup->cmd[CMD_MODE_USER] = CMD_TYPE_NONE;

				memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
				SendMsg.msg = MSG_MODULE_JIGM_TRY_DISCONTACT;
				SendMsg.val[0] = group;
				send_msg(MODULE_TO_JIGM, (char *)&SendMsg);

				myGroup->state = G_STANDBY;
				myGroup->phase = P0;
				break;
			} else {
				myGroup->cmd[CMD_MODE_USER] = CMD_TYPE_NONE;
			}
			break;
		case P3:
			if(myGroup->attribute == ATTR_CHECK1_END
				|| myGroup->attribute == ATTR_CHECK2_END
				|| myGroup->attribute == ATTR_CHECK3_END) {
				myGroup->stepNo++;
				stepNo = myGroup->stepNo;
				myGroup->advStepNo++;
				myGroup->cycleNo++;
				myGroup->advCycleNo = 0;

				idx = IDX_LOC_OBJ_CLIENT_STEP_NO;
				myGroup->client_stepNo = (int)myData->testCond[ch]
					.local_object[stepNo][idx];
				idx = IDX_LOC_OBJ_TYPE;
				myGroup->stepType = (unsigned char)myData->testCond[ch]
					.local_object[stepNo][idx];
				idx = IDX_LOC_OBJ_MODE;
				myGroup->stepMode = (unsigned char)myData->testCond[ch]
					.local_object[stepNo][idx];
				idx = IDX_LOC_OBJ_ATTRIBUTE;
				myGroup->attribute = (unsigned char)myData->testCond[ch]
					.local_object[stepNo][idx];
			}
			myGroup->phase = P10;
			break;
		case P5: //Load Line Check
			if(myGroup->cmd[CMD_MODE_USER] == CMD_TYPE_STOP) {
				myGroup->cmd[CMD_MODE_USER] = CMD_TYPE_NONE;

				memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
				SendMsg.msg = MSG_MODULE_JIGM_TRY_DISCONTACT;
				SendMsg.val[0] = group;
				send_msg(MODULE_TO_JIGM, (char *)&SendMsg);

				i = gLoadLineCheck(group, 1);

				myGroup->state = G_STANDBY;
				myGroup->phase = P0;
				break;
			}

			if(gLoadLineCheck(group, 0) > 0) {
				if(myData->AppControl.config.debugType == 0) {
					monitor_ch = (int)myData->COB_Client[group].config
						.ChArray2[myGroup->misc.scan_ch[0]];
					Select_OutPoint(2, monitor_ch+1, O_RUN_RELAY, ON);
					monitor_ch = (int)myData->COB_Client[group].config
						.ChArray2[myGroup->misc.scan_ch[1]];
					Select_OutPoint(3, monitor_ch+1, O_RUN_RELAY, ON);
				}
				myGroup->signal[G_SIG_LOAD_LINE_CHECK_PHASE] = P0;
				myGroup->phase = P6;
			}
			break;
		case P6:
			myGroup->signal[G_SIG_LOAD_LINE_CHECK_PHASE]++;
			if(myGroup->signal[G_SIG_LOAD_LINE_CHECK_PHASE] >= P30) {
				//10ms * 30 = 0.3sec
				myGroup->phase = P7;
			}
			break;
		case P7:
			myData->mData.signal[M_SIG_RUNNING_GROUP] = (unsigned char)group;
			myData->cData[0].signal[C_SIG_CMD_RUN] = P1;
			myData->cData[1].signal[C_SIG_CMD_RUN] = P1;
			myGroup->cmd[CMD_MODE_USER] = CMD_TYPE_NONE;

			stepNo = myGroup->stepNo;
			myGroup->advCycleNo = 0;

			idx = IDX_LOC_OBJ_CLIENT_STEP_NO;
			myGroup->client_stepNo = (int)myData->testCond[ch]
				.local_object[stepNo][idx];
			idx = IDX_LOC_OBJ_TYPE;
			myGroup->stepType = (unsigned char)myData->testCond[ch]
				.local_object[stepNo][idx];
			idx = IDX_LOC_OBJ_MODE;
			myGroup->stepMode = (unsigned char)myData->testCond[ch]
				.local_object[stepNo][idx];
			idx = IDX_LOC_OBJ_ATTRIBUTE;
			myGroup->attribute = (unsigned char)myData->testCond[ch]
				.local_object[stepNo][idx];

			myGroup->phase = P10;
			myGroup->runTime = 0;
			myGroup->saveDt = 0;
			break;
		case P10: //Running
			myGroup->runTime += myPs->config.scan_period;

			if(myGroup->signal[G_SIG_JIG_FAIL] == P1) {
				myGroup->signal[G_SIG_JIG_FAIL] = P2;
				myGroup->cmd[CMD_MODE_USER] = CMD_TYPE_PAUSE;
			}

			if(myGroup->cmd[CMD_MODE_USER] == CMD_TYPE_STOP) {
				myGroup->cmd[CMD_MODE_USER] = CMD_TYPE_NONE;
				myGroup->signal[G_SIG_CMD_STOP] = P1;
				for(i=ch; i < chInGroup; i++) {
					if(myData->ChAttribute[0].opType == 0) {
					} else {
						if((i % 2) != 0) continue;
					}

					if(myData->cData[i].op.state == C_RUN
						|| myData->cData[i].op.state == C_PAUSE) {
						myData->cData[i].signal[C_SIG_CMD_STOP] = P1;
					}
				}
				myGroup->phase = P30;
				break;
			} else if(myGroup->cmd[CMD_MODE_USER] == CMD_TYPE_PAUSE) {
				myGroup->cmd[CMD_MODE_USER] = CMD_TYPE_NONE;
				for(i=ch; i < chInGroup; i++) {
					if(myData->ChAttribute[0].opType == 0) {
					} else {
						if((i % 2) != 0) continue;
					}

					if(myData->cData[i].op.state == C_RUN) {
						myData->cData[i].signal[C_SIG_CMD_PAUSE] = P1;
					}
				}
				break;
			} else if(myGroup->cmd[CMD_MODE_USER] == CMD_TYPE_NEXT_STEP) {
				myGroup->cmd[CMD_MODE_USER] = CMD_TYPE_NONE;
				myGroup->signal[G_SIG_CMD_NEXT_STEP] = P1;
				for(i=ch; i < chInGroup; i++) {
					if(myData->ChAttribute[0].opType == 0) {
					} else {
						if((i % 2) != 0) continue;
					}

					myData->cData[i].signal[C_SIG_CMD_NEXT_STEP] = P1;
				}
				break;
			} else {
				myGroup->cmd[CMD_MODE_USER] = CMD_TYPE_NONE;
			}

			send_save_realData_msg(group);

			ch_standby = ch_run_hold = ch_fault = ch_pause = ch_running = 0;
			ch_pause2 = 0; ch_code_check = 0;
			for(i=ch; i < chInGroup; i++) {
				if(myData->ChAttribute[0].opType == 0) {
				} else {
					if((i % 2) != 0) continue;
				}

				if(myData->cData[i].op.state == C_STANDBY) {
					ch_standby++;
				} else if(myData->cData[i].op.state == C_RUN
					&& myData->cData[i].op.phase == P50) {
					ch_running++;
				} else if(myData->cData[i].op.state == C_RUN
					&& myData->cData[i].op.phase == P102) {
					ch_run_hold++;
				} else if(myData->cData[i].op.state == C_PAUSE) {
					if(myData->cData[i].op.attribute == ATTR_CHECK1_END
						|| myData->cData[i].op.attribute == ATTR_CHECK2_END
						|| myData->cData[i].op.attribute == ATTR_CHECK3_END) {
						ch_pause2++;
					} else {
						if(myData->cData[i].op.code == C_CD_FAULT_PAUSE_CMD)
							ch_pause++;
						else ch_fault++;
					}
				}
				if(myData->cData[i].op.code >= C_CD_FAULT_CHECK_START
					&& myData->cData[i].op.code <= C_CD_FAULT_CHECK_END)
					ch_code_check++;
			}

			if(myData->ChAttribute[0].opType == 0) {
				cmp_count = chInGroup;
			} else {
				cmp_count = chInGroup / 2;
			}

			if(cmp_count == (ch_standby + ch_fault)) {
				//kjg_110819_w gCompare_Ch_Average_3(group);
				send_save_msg_formation(group, 0);
				myGroup->phase = P20;
			} else if(cmp_count == (ch_pause2 + ch_fault)) {
				//kjg_110819_w gCompare_Ch_Average_3(group);
				send_save_msg_formation(group, 0);
				myGroup->phase = P11;
			} else if(cmp_count == (ch_run_hold + ch_pause2 + ch_fault)) {
				//kjg_110819_w gCompare_Ch_Average_3(group);

				if(myGroup->stepType == STEP_LOOP
					&& (myGroup->attribute == ATTR_CHECK1_END
						|| myGroup->attribute == ATTR_CHECK2_END
						|| myGroup->attribute == ATTR_CHECK3_END)) {
					gCellCheck_Compare(group);
					if(myGroup->misc.cell_check_result == 0) { //check_ok
						if(cmp_count != ch_code_check) {
							send_save_msg_formation(group, 0);
						}
						myGroup->phase = P11;
					} else {
						myGroup->misc.cell_check_result = 0;
						if(myGroup->misc.cell_check_count >= 3) {
							if(cmp_count != ch_code_check) {
								send_save_msg_formation(group, 0);
							}
							myGroup->phase = P11;
						} else {
							myGroup->phase = P50;
						}
					}
				} else {
					if(cmp_count != ch_code_check) {
						send_save_msg_formation(group, 0);
					}
					myGroup->phase = P11;
				}
			} else if(cmp_count == (ch_run_hold + ch_standby)) {
				//kjg_110819_w gCompare_Ch_Average_3(group);
				send_save_msg_formation(group, 0);
				myGroup->phase = P20;
			} else if(cmp_count == (ch_pause + ch_fault)) {
				myGroup->phase = P23;
			}
			break;
		case P11:
			if(myGroup->cmd[CMD_MODE_USER] == CMD_TYPE_STOP) {
				myGroup->cmd[CMD_MODE_USER] = CMD_TYPE_NONE;
				myGroup->signal[G_SIG_CMD_STOP] = P1;

				for(i=ch; i < chInGroup; i++) {
					if(myData->ChAttribute[0].opType == 0) {
					} else {
						if((i % 2) != 0) continue;
					}

					myData->cData[i].signal[C_SIG_CMD_STOP] = P1;
				}
				break;
			}

			if(myGroup->signal[G_SIG_CMD_NEXT_STEP] == P1) {
				myGroup->signal[G_SIG_CMD_NEXT_STEP] = P0;
				myGroup->phase = P12;
				break;
			}

			ch_standby = ch_run_hold = ch_fault = ch_pause = 0;
			ch_pause2 = 0;
			for(i=ch; i < chInGroup; i++) {
				if(myData->ChAttribute[0].opType == 0) {
				} else {
					if((i % 2) != 0) continue;
				}

				if(myData->cData[i].op.state == C_STANDBY) {
					ch_standby++;
				} else if(myData->cData[i].op.state == C_RUN
					&& myData->cData[i].op.phase == P102) {
					ch_run_hold++;
				} else if(myData->cData[i].op.state == C_PAUSE) {
					if(myData->cData[i].op.attribute == ATTR_CHECK1_END
						|| myData->cData[i].op.attribute == ATTR_CHECK2_END
						|| myData->cData[i].op.attribute == ATTR_CHECK3_END) {
						ch_pause2++;
					} else {
						if(myData->cData[i].op.code == C_CD_FAULT_PAUSE_CMD)
							ch_pause++;
						else ch_fault++;
					}
				}
			}

			if(myData->ChAttribute[0].opType == 0) {
				cmp_count = chInGroup;
			} else {
				cmp_count = chInGroup / 2;
			}

			if(cmp_count == ch_fault) { //all ch fault
				if(myGroup->signal[G_SIG_CMD_STOP] == P0) {
					myGroup->signal[G_SIG_ALL_CHANNEL_ERROR] = P1;
					myGroup->phase = P30;
				} else {
					myGroup->signal[G_SIG_CMD_STOP] = P0;
					myGroup->phase = P20;
				}
			} else if(cmp_count == (ch_standby)) { //normal
				myGroup->phase = P20;
			} else if(cmp_count == (ch_pause2 + ch_fault)) { //pause
				myGroup->phase = P23;
			} else if(cmp_count == (ch_run_hold + ch_pause2 + ch_fault)) {
				//normal
				myGroup->phase = P12;
			} else if(cmp_count == (ch_pause + ch_fault)) { //pause
				myGroup->phase = P23;
			}
			break;
		case P12:
			switch(myGroup->stepType) {
				case STEP_CYCLE:
					myGroup->start_stepNo = myGroup->stepNo;
					myGroup->stepNo++;
					stepNo = myGroup->stepNo;
					myGroup->advStepNo++;

					idx = IDX_LOC_OBJ_CLIENT_STEP_NO;
					myGroup->client_stepNo = (int)myData->testCond[ch]
						.local_object[stepNo][idx];
					idx = IDX_LOC_OBJ_TYPE;
					myGroup->stepType = (unsigned char)myData->testCond[ch]
						.local_object[stepNo][idx];
					idx = IDX_LOC_OBJ_MODE;
					myGroup->stepMode = (unsigned char)myData->testCond[ch]
						.local_object[stepNo][idx];
					idx = IDX_LOC_OBJ_ATTRIBUTE;
					myGroup->attribute = (unsigned char)myData->testCond[ch]
						.local_object[stepNo][idx];

					switch(myData->AppControl.config.systemModel) {
						case F_PNE_5V_15A_30AP_SW:
							if(myGroup->stepType == STEP_DISCHARGE) {
								idx = IDX_LOC_OBJ_REF_V2;
							} else {
								idx = IDX_LOC_OBJ_REF_V;
							}
							val = myData->testCond[group]
								.local_object[stepNo][idx];
							idx = IDX_LOC_OBJ_RANGE_V;
							range = myData->testCond[group]
								.local_object[stepNo][idx];
							for(i=0; i < myPs->config.installedBd; i++) {
								bVICmd_Setting(i, val, range, 5, 0);
								myData->bData[i].misc.ch_sens_count = -1;
								myData->bData[i].misc.ch_sens_count_flag = P0;
							}

							idx = IDX_LOC_OBJ_REF_I;
							val = myData->testCond[group]
								.local_object[stepNo][idx];
							idx = IDX_LOC_OBJ_RANGE_I;
							range = myData->testCond[group]
								.local_object[stepNo][idx];
							for(i=0; i < myPs->config.installedBd; i++) {
								bVICmd_Setting(i, val, range, 5, 1);
							}
							break;
						default: break;
					}

					for(i=ch; i < chInGroup; i++) {
						if(myData->ChAttribute[0].opType == 0) {
						} else {
							if((i % 2) != 0) continue;
						}

						bd = i / myPs->config.chPerBd;
						ch_2 = i % myPs->config.chPerBd;

						if(myData->cData[i].op.state == C_RUN
							&& myData->cData[i].op.phase == P102) {
							myData->cData[i].op.phase = P101;

							type = 0;
							if(myGroup->stepType == STEP_DISCHARGE) {
								idx = IDX_LOC_OBJ_REF_V2;
							} else {
								idx = IDX_LOC_OBJ_REF_V;
							}
							val = myData->testCond[group]
								.local_object[stepNo][idx];
							idx = IDX_LOC_OBJ_RANGE_V;
							range = myData->testCond[group]
								.local_object[stepNo][idx];
							point = aFindDACaliPoint(bd, ch_2, val, type,
								range);
							point = 0; //kjg_110821_d
							sum = val * myData->f_cali.cData[bd][ch_2]
								.AUX_DA_A_P[type][range][point] / 1000000.0
								+ myData->f_cali.cData[bd][ch_2]
								.AUX_DA_B_P[type][range][point];

							myData->cData[i].misc.auxDaV
								= (unsigned short)sum;
							//myData->cData[i].signal[C_SIG_AUX_DAV] = P21;
							myData->cData[i].signal[C_SIG_AUX_DAV] = P1;
							//myData->cData[i].signal[C_SIG_AUX_DAV] = P20;

							type = 1;
							idx = IDX_LOC_OBJ_REF_I;
							val = myData->testCond[group]
								.local_object[stepNo][idx];
							idx = IDX_LOC_OBJ_RANGE_I;
							range = myData->testCond[group]
								.local_object[stepNo][idx];
							point = aFindDACaliPoint(bd, ch_2, val, type,
								range);
							point = 0; //kjg_110821_d
							if(val < 0) {
								point = 1; //kjg_110821_d
								val *= (-1);
							}
							sum = val * (double)myData->f_cali
								.cData[bd][ch_2]
								.AUX_DA_A_P[type][range][point] / 1000000.0
								+ myData->f_cali.cData[bd][ch_2]
								.AUX_DA_B_P[type][range][point];

							myData->cData[i].misc.auxDaI = (short int)sum;
							//myData->cData[i].signal[C_SIG_AUX_DAI] = P21;
							myData->cData[i].signal[C_SIG_AUX_DAI] = P1;
							//myData->cData[i].signal[C_SIG_AUX_DAI] = P20;
						} else if(myData->cData[i].op.state == C_PAUSE
							&& myData->cData[i].op.phase == P102) {
							myData->cData[i].op.phase = P101;

							type = 0;
							if(myGroup->stepType == STEP_DISCHARGE) {
								idx = IDX_LOC_OBJ_REF_V2;
							} else {
								idx = IDX_LOC_OBJ_REF_V;
							}
							val = myData->testCond[group]
								.local_object[stepNo][idx];
							idx = IDX_LOC_OBJ_RANGE_V;
							range = myData->testCond[group]
								.local_object[stepNo][idx];
							point = aFindDACaliPoint(bd, ch_2, val, type,
								range);
							point = 0; //kjg_110821_d
							sum = val * myData->f_cali.cData[bd][ch_2]
								.AUX_DA_A_P[type][range][point] / 1000000.0
								+ myData->f_cali.cData[bd][ch_2]
								.AUX_DA_B_P[type][range][point];

							myData->cData[i].misc.auxDaV
								= (unsigned short)sum;
							//myData->cData[i].signal[C_SIG_AUX_DAV] = P21;
							myData->cData[i].signal[C_SIG_AUX_DAV] = P1;
							//myData->cData[i].signal[C_SIG_AUX_DAV] = P20;

							type = 1;
							idx = IDX_LOC_OBJ_REF_I;
							val = myData->testCond[group]
								.local_object[stepNo][idx];
							idx = IDX_LOC_OBJ_RANGE_I;
							range = myData->testCond[group]
								.local_object[stepNo][idx];
							point = aFindDACaliPoint(bd, ch_2, val, type,
								range);
							point = 0; //kjg_110821_d
							if(val < 0) {
								point = 1; //kjg_110821_d
								val *= (-1);
							}
							sum = val * (double)myData->f_cali
								.cData[bd][ch_2]
								.AUX_DA_A_P[type][range][point] / 1000000.0
								+ myData->f_cali.cData[bd][ch_2]
								.AUX_DA_B_P[type][range][point];

							myData->cData[i].misc.auxDaI = (short int)sum;
							//myData->cData[i].signal[C_SIG_AUX_DAI] = P21;
							myData->cData[i].signal[C_SIG_AUX_DAI] = P1;
							//myData->cData[i].signal[C_SIG_AUX_DAI] = P20;
						}
					}
					myGroup->phase = P10;
					break;
				case STEP_LOOP:
					if(myGroup->attribute == ATTR_CHECK1_END
						|| myGroup->attribute == ATTR_CHECK2_END
						|| myGroup->attribute == ATTR_CHECK3_END) {
						if(myGroup->workMode == WORK_TYPE_ONLINE) {
							myGroup->phase = P3;
						} else {
							myGroup->phase = P24;
						}
					} else {
						myGroup->stepNo++;
						stepNo = myGroup->stepNo;
						myGroup->advStepNo++;

						idx = IDX_LOC_OBJ_CLIENT_STEP_NO;
						myGroup->client_stepNo = (int)myData->testCond[ch]
							.local_object[stepNo][idx];
						idx = IDX_LOC_OBJ_TYPE;
						myGroup->stepType = (unsigned char)myData->testCond[ch]
							.local_object[stepNo][idx];
						idx = IDX_LOC_OBJ_MODE;
						myGroup->stepMode = (unsigned char)myData->testCond[ch]
							.local_object[stepNo][idx];
						idx = IDX_LOC_OBJ_ATTRIBUTE;
						myGroup->attribute = (unsigned char)myData->testCond[ch]
							.local_object[stepNo][idx];

						for(i=ch; i < chInGroup; i++) {
							if(myData->ChAttribute[0].opType == 0) {
							} else {
								if((i % 2) != 0) continue;
							}

							if(myData->cData[i].op.state == C_RUN
								&& myData->cData[i].op.phase == P102) {
								myData->cData[i].op.phase = P101;
							} else if(myData->cData[i].op.state == C_PAUSE
								&& myData->cData[i].op.phase == P102) {
								myData->cData[i].op.phase = P101;
							}
						}
						myGroup->phase = P10;
					}
					break;
				case STEP_END:
					stepNo = myGroup->stepNo;
					idx = IDX_LOC_OBJ_CLIENT_STEP_NO;
					myGroup->client_stepNo = (int)myData->testCond[ch]
						.local_object[stepNo][idx];
					idx = IDX_LOC_OBJ_ATTRIBUTE;
					myGroup->attribute = (unsigned char)myData->testCond[ch]
						.local_object[stepNo][idx];

					for(i=ch; i < chInGroup; i++) {
						if(myData->ChAttribute[0].opType == 0) {
						} else {
							if((i % 2) != 0) continue;
						}

						if(myData->cData[i].op.state == C_RUN
							&& myData->cData[i].op.phase == P102) {
							myData->cData[i].op.phase = P101;
						} else if(myData->cData[i].op.state == C_PAUSE
							&& myData->cData[i].op.phase == P102) {
							myData->cData[i].op.phase = P101;
						} else if(myData->cData[i].op.state == C_PAUSE) {
							myData->cData[i].op.state = C_STANDBY;
							myData->cData[i].op.phase = P0;
						}
					}
					myGroup->phase = P20;
					break;
				case STEP_OCV:
				case STEP_REST:
				case STEP_LONG_TIME_REST:
				case STEP_CHARGE:
				case STEP_DISCHARGE:
					myGroup->stepNo++;
					stepNo = myGroup->stepNo;
					myGroup->advStepNo++;

					idx = IDX_LOC_OBJ_CLIENT_STEP_NO;
					myGroup->client_stepNo = (int)myData->testCond[ch]
						.local_object[stepNo][idx];
					idx = IDX_LOC_OBJ_TYPE;
					myGroup->stepType = (unsigned char)myData->testCond[ch]
						.local_object[stepNo][idx];
					idx = IDX_LOC_OBJ_MODE;
					myGroup->stepMode = (unsigned char)myData->testCond[ch]
						.local_object[stepNo][idx];
					idx = IDX_LOC_OBJ_ATTRIBUTE;
					myGroup->attribute = (unsigned char)myData->testCond[ch]
						.local_object[stepNo][idx];

					switch(myData->AppControl.config.systemModel) {
						case F_PNE_5V_15A_30AP_SW:
							if(myGroup->stepType == STEP_DISCHARGE) {
								idx = IDX_LOC_OBJ_REF_V2;
							} else {
								idx = IDX_LOC_OBJ_REF_V;
							}
							val = myData->testCond[group]
								.local_object[stepNo][idx];
							idx = IDX_LOC_OBJ_RANGE_V;
							range = myData->testCond[group]
								.local_object[stepNo][idx];
							for(i=0; i < myPs->config.installedBd; i++) {
								bVICmd_Setting(i, val, range, 5, 0);
							}

							idx = IDX_LOC_OBJ_REF_I;
							val = myData->testCond[group]
								.local_object[stepNo][idx];
							idx = IDX_LOC_OBJ_RANGE_I;
							range = myData->testCond[group]
								.local_object[stepNo][idx];
							for(i=0; i < myPs->config.installedBd; i++) {
								bVICmd_Setting(i, val, range, 5, 1);
								myData->bData[i].misc.ch_sens_count = -1;
								myData->bData[i].misc.ch_sens_count_flag = P0;
							}
							break;
						default: break;
					}

					j = myData->mData.config.chPerBd * group;
					for(i=j; i < j + chInGroup; i++) {
						if(myData->ChAttribute[0].opType == 0) {
						} else {
							if((i % 2) != 0) continue;
						}

						bd = i / myPs->config.chPerBd;
						ch_2 = i % myPs->config.chPerBd;

						if(myData->cData[i].op.state == C_RUN
							&& myData->cData[i].op.phase == P102) {
							myData->cData[i].op.phase = P101;

							type = 0;
							if(myGroup->stepType == STEP_DISCHARGE) {
								idx = IDX_LOC_OBJ_REF_V2;
							} else {
								idx = IDX_LOC_OBJ_REF_V;
							}
							val = myData->testCond[group]
								.local_object[stepNo][idx];
							idx = IDX_LOC_OBJ_RANGE_V;
							range = myData->testCond[group]
								.local_object[stepNo][idx];
							point = aFindDACaliPoint(bd, ch_2, val, type,
								range);
							point = 0; //kjg_110821_d
							sum = val * myData->f_cali.cData[bd][ch_2]
								.AUX_DA_A_P[type][range][point] / 1000000.0
								+ myData->f_cali.cData[bd][ch_2]
								.AUX_DA_B_P[type][range][point];

							myData->cData[i].misc.auxDaV
								= (unsigned short)sum;
							//myData->cData[i].signal[C_SIG_AUX_DAV] = P21;
							myData->cData[i].signal[C_SIG_AUX_DAV] = P1;
							//myData->cData[i].signal[C_SIG_AUX_DAV] = P20;

							type = 1;
							idx = IDX_LOC_OBJ_REF_I;
							val = myData->testCond[group]
								.local_object[stepNo][idx];
							idx = IDX_LOC_OBJ_RANGE_I;
							range = myData->testCond[group]
								.local_object[stepNo][idx];
							point = aFindDACaliPoint(bd, ch_2, val, type,
								range);
							point = 0; //kjg_110821_d
							if(val < 0) {
								point = 1; //kjg_110821_d
								val *= (-1);
							}
							sum = val * (double)myData->f_cali
								.cData[bd][ch_2]
								.AUX_DA_A_P[type][range][point] / 1000000.0
								+ myData->f_cali.cData[bd][ch_2]
								.AUX_DA_B_P[type][range][point];

							myData->cData[i].misc.auxDaI = (short int)sum;
							//myData->cData[i].signal[C_SIG_AUX_DAI] = P21;
							myData->cData[i].signal[C_SIG_AUX_DAI] = P1;
							//myData->cData[i].signal[C_SIG_AUX_DAI] = P20;
						} else if(myData->cData[i].op.state == C_PAUSE
							&& myData->cData[i].op.phase == P102) {
							myData->cData[i].op.phase = P101;

							type = 0;
							if(myGroup->stepType == STEP_DISCHARGE) {
								idx = IDX_LOC_OBJ_REF_V2;
							} else {
								idx = IDX_LOC_OBJ_REF_V;
							}
							val = myData->testCond[group]
								.local_object[stepNo][idx];
							idx = IDX_LOC_OBJ_RANGE_V;
							range = myData->testCond[group]
								.local_object[stepNo][idx];
							point = aFindDACaliPoint(bd, ch_2, val, type,
								range);
							point = 0; //kjg_110821_d
							sum = val * myData->f_cali.cData[bd][ch_2]
								.AUX_DA_A_P[type][range][point] / 1000000.0
								+ myData->f_cali.cData[bd][ch_2]
								.AUX_DA_B_P[type][range][point];

							myData->cData[i].misc.auxDaV
								= (unsigned short)sum;
							//myData->cData[i].signal[C_SIG_AUX_DAV] = P21;
							myData->cData[i].signal[C_SIG_AUX_DAV] = P1;
							//myData->cData[i].signal[C_SIG_AUX_DAV] = P20;

							type = 1;
							idx = IDX_LOC_OBJ_REF_I;
							val = myData->testCond[group]
								.local_object[stepNo][idx];
							idx = IDX_LOC_OBJ_RANGE_I;
							range = myData->testCond[group]
								.local_object[stepNo][idx];
							point = aFindDACaliPoint(bd, ch_2, val, type,
								range);
							point = 0; //kjg_110821_d
							if(val < 0) {
								point = 1; //kjg_110821_d
								val *= (-1);
							}
							sum = val * (double)myData->f_cali
								.cData[bd][ch_2]
								.AUX_DA_A_P[type][range][point] / 1000000.0
								+ myData->f_cali.cData[bd][ch_2]
								.AUX_DA_B_P[type][range][point];

							myData->cData[i].misc.auxDaI = (short int)sum;
							//myData->cData[i].signal[C_SIG_AUX_DAI] = P21;
							myData->cData[i].signal[C_SIG_AUX_DAI] = P1;
							//myData->cData[i].signal[C_SIG_AUX_DAI] = P20;
						}
					}
					myGroup->phase = P10;
					break;
				default: break;
			}
			myGroup->runTime = 0;
			myGroup->saveDt = 0;
			break;
		case P13:
			for(i=0; i < chInGroup; i++) {
				Select_OutPoint(2, i+1, O_RUN_RELAY, OFF);
				Select_OutPoint(3, i+1, O_RUN_RELAY, OFF);
			}

			myGroup->state = G_PAUSE;
			myGroup->phase = P10;
			break;
		case P20:
			if(myGroup->cmd[CMD_MODE_USER] == CMD_TYPE_STOP) {
				myGroup->cmd[CMD_MODE_USER] = CMD_TYPE_NONE;
			}
			if(myGroup->attribute == ATTR_TOTAL_CHECK) {
				send_save_msg_formation(group, 0);
				myGroup->phase = P30;
			} else { //normal end
				if(myGroup->workMode == WORK_TYPE_OFFLINE)  {
					myGroup->phase = P30;
				} else { //WORK_ONLINE	
					myGroup->phase = P40;
				}
			}
			break;
		case P23:
			myGroup->phase = P32;
			break;
		case P24:
			for(i=0; i < chInGroup; i++) {
				Select_OutPoint(2, i+1, O_RUN_RELAY, OFF);
				Select_OutPoint(3, i+1, O_RUN_RELAY, OFF);
			}

			myGroup->state = G_PAUSE;
			myGroup->phase = P0;
			break;
		case P30:
			for(i=0; i < chInGroup; i++) {
				Select_OutPoint(2, i+1, O_RUN_RELAY, OFF);
				Select_OutPoint(3, i+1, O_RUN_RELAY, OFF);
			}

			for(i=ch; i < chInGroup; i++) {
				if(myData->cData[i].op.state != C_STANDBY) {
					myData->cData[i].op.state = C_STANDBY;
					myData->cData[i].op.phase = P0;
				}
			}

			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
			SendMsg.msg = MSG_MODULE_JIGM_TRY_DISCONTACT;
			SendMsg.val[0] = group;
			send_msg(MODULE_TO_JIGM, (char *)&SendMsg);

			myGroup->phase = P31;
			break;
		case P31:
			if(myGroup->signal[G_SIG_JIG_FAIL] == P1) {
				myGroup->signal[G_SIG_JIG_FAIL] = P2;
				myGroup->cmd[CMD_MODE_USER] = CMD_TYPE_NONE;
				myGroup->state = G_STANDBY;
				myGroup->phase = P0;
			} else if(myGroup->signal[G_SIG_DISCONTACT_COMPLETE] == P1) {
				myGroup->signal[G_SIG_DISCONTACT_COMPLETE] = P0;
				myGroup->cmd[CMD_MODE_USER] = CMD_TYPE_NONE;
				myGroup->state = G_STANDBY;
				myGroup->phase = P0;
			}
			break;
		case P32:
			for(i=0; i < chInGroup; i++) {
				Select_OutPoint(2, i+1, O_RUN_RELAY, OFF);
				Select_OutPoint(3, i+1, O_RUN_RELAY, OFF);
			}

			myGroup->state = G_PAUSE;
			myGroup->phase = P0;
			break;
		case P40:
			for(i=0; i < chInGroup; i++) {
				Select_OutPoint(2, i+1, O_RUN_RELAY, OFF);
				Select_OutPoint(3, i+1, O_RUN_RELAY, OFF);
			}
			myGroup->phase = P41;
			break;
		case P41:
			myGroup->signal[G_SIG_NEXT_STEP_EXIST] = P0;
			myGroup->cmd[CMD_MODE_USER] = CMD_TYPE_NONE;
			myGroup->state = G_STANDBY;
			myGroup->phase = P0;
			break;
		case P50:
			for(i=ch; i < chInGroup; i++) {
				if(myData->cData[i].op.state != C_STANDBY) {
					myData->cData[i].op.state = C_STANDBY;
					myData->cData[i].op.phase = P0;
				}
			}

			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
			SendMsg.msg = MSG_MODULE_JIGM_TRY_DISCONTACT;
			SendMsg.val[0] = group;
			send_msg(MODULE_TO_JIGM, (char *)&SendMsg);
			myGroup->phase = P51;
			break;
		case P51:
			if(myGroup->signal[G_SIG_JIG_FAIL] == P1) {
				myGroup->signal[G_SIG_JIG_FAIL] = P2;
				myGroup->cmd[CMD_MODE_USER] = CMD_TYPE_NONE;
				myGroup->state = G_STANDBY;
				myGroup->phase = P0;
			} else if(myGroup->signal[G_SIG_DISCONTACT_COMPLETE] == P1) {
				myGroup->signal[G_SIG_DISCONTACT_COMPLETE] = P0;
				myGroup->cmd[CMD_MODE_USER] = CMD_TYPE_NONE;
				myGroup->phase = P52;
			}
			break;
		case P52:
			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
			SendMsg.msg = MSG_MODULE_JIGM_TRY_CONTACT;
			SendMsg.val[0] = group;
			send_msg(MODULE_TO_JIGM, (char *)&SendMsg);

			stepNo = 0;
			myGroup->stepNo = 0;
			myGroup->advStepNo = 0;
			myGroup->cycleNo = 0;
			myGroup->advCycleNo = 0;
			myGroup->start_stepNo = 0;
			myGroup->saveDt = 0;
			myGroup->runTime = 0;
			myGroup->code = G_CD_NONE;

			ch = myGroup->misc.chOffset;
			idx = IDX_LOC_OBJ_CLIENT_STEP_NO;
			myGroup->client_stepNo = (int)myData->testCond[ch]
				.local_object[stepNo][idx];
			idx = IDX_LOC_OBJ_TYPE;
			myGroup->stepType = (unsigned char)myData->testCond[ch]
				.local_object[stepNo][idx];
			idx = IDX_LOC_OBJ_MODE;
			myGroup->stepMode = (unsigned char)myData->testCond[ch]
				.local_object[stepNo][idx];
			idx = IDX_LOC_OBJ_ATTRIBUTE;
			myGroup->attribute = (unsigned char)myData->testCond[ch]
				.local_object[stepNo][idx];

			myGroup->phase = P1;
			break;
		default: break;
	}*/
}

void gRun_F_SDI_5V_400A_200A_100A_10A(int group)
{ //debug_size_cob
/*	int chInGroup, avg_ng_flag, monitor_ch;

	chInGroup = myData->mData.config.chInGroup[group];

	if(myData->cData[0].op.state == C_STANDBY
		&& myData->cData[0].op.phase == P2) {
		monitor_ch = (int)myData->COB_Client[group].config
			.ChArray2[myGroup->misc.scan_ch[0]-1];
		Select_OutPoint(2, monitor_ch+1, O_RUN_RELAY, OFF);
		monitor_ch = (int)myData->COB_Client[group].config
			.ChArray2[myGroup->misc.scan_ch[0]];
		Select_OutPoint(2, monitor_ch+1, O_RUN_RELAY, ON);
		myGroup->misc.scan_step[0] = 0;
		myData->cData[0].signal[C_SIG_CMD_RUN] = P1;
	} else if(myData->cData[0].op.state == C_PAUSE
		&& myData->cData[0].op.stepType == STEP_LOOP
		&& myData->cData[0].op.phase == P1) {

		if(myData->AppControl.config.debugType == 0) { //normal
			//for 12ch test
			if(myGroup->misc.scan_ch[0] == 11) {
				send_save_msg_formation(group, 0);

				monitor_ch = (int)myData->COB_Client[group].config
					.ChArray2[myGroup->misc.scan_ch[0]];
				Select_OutPoint(2, monitor_ch+1, O_RUN_RELAY, OFF);
				myGroup->misc.scan_ch[0] = 0;
				myGroup->misc.scan_ch[1] = 0;
				myGroup->misc.scan_step[0] = 0;
				if(myData->COB_Client[0].config.protocol_version
					== P2_PROTOCOL_VERSION) {
					myGroup->misc.scan_idxStepNo_offset[0] = 3;
				} else {
					myGroup->misc.scan_idxStepNo_offset[0] = 4;
				}

				if(myGroup->misc.total_ng_ch == (short int)chInGroup) {
					myGroup->phase = P30;
				} else {
					myGroup->phase = P13; //pause
				}
			} else {
				myGroup->misc.scan_ch[0]++;
				myGroup->misc.scan_ch[1]++;
			}
		} else if(myData->AppControl.config.debugType >= 1
			&& myData->AppControl.config.debugType <= 12) {
			//for 1ch test
			myGroup->phase = P13; //pause
		}

		myData->cData[0].op.state = C_STANDBY;
		myData->cData[0].op.phase = P0;
	} else if(myData->cData[0].op.stepType == STEP_LONG_TIME_REST
		&& ((myData->cData[0].op.state == C_RUN
			&& (myData->cData[0].op.phase == P2
			|| myData->cData[0].op.phase == P12))
		|| (myData->cData[0].op.state == C_PAUSE
			&& myData->cData[0].op.phase == P112))) {
		if(myData->AppControl.config.debugType == 0) { //normal
			//for 12ch test
			if(myGroup->misc.scan_ch[0] == 11) {
				avg_ng_flag = gCompare_Ch_Average_2(group);
				if(avg_ng_flag == P0) {
					myGroup->misc.long_rest_time1[myGroup->misc.scan_ch[0]]
						= myGroup->runTime;
					monitor_ch = (int)myData->COB_Client[group].config
						.ChArray2[myGroup->misc.scan_ch[0]];
					Select_OutPoint(2, monitor_ch+1, O_RUN_RELAY, OFF);
					Select_OutPoint(2, 6, O_RUN_RELAY, ON);
					myGroup->misc.scan_ch[0] = 0;
					myGroup->misc.scan_ch[1] = 0;
					myGroup->misc.scan_idxStepNo_offset[0]
						= myData->cData[0].op.idxStepNo;
					myGroup->misc.tmp_scan_step[0] = myGroup->misc.scan_step[0];
					myGroup->misc.scan_end_step_flag = P1;
					myData->cData[0].misc.checkDelayTime = 0;
					myData->cData[0].op.phase++;
				} else {
					send_save_msg_formation(group, 0);
					monitor_ch = (int)myData->COB_Client[group].config
						.ChArray2[myGroup->misc.scan_ch[0]];
					Select_OutPoint(2, monitor_ch+1, O_RUN_RELAY, OFF);
					myGroup->phase = P30;
				}
			} else {
				myGroup->misc.long_rest_time1[myGroup->misc.scan_ch[0]]
					= myGroup->runTime;
				myGroup->misc.scan_ch[0]++;
				myGroup->misc.scan_ch[1]++;
				myGroup->misc.scan_step[0] = myGroup->misc.tmp_scan_step[0];
				monitor_ch = (int)myData->COB_Client[group].config
					.ChArray2[myGroup->misc.scan_ch[0]-1];
				Select_OutPoint(2, monitor_ch+1, O_RUN_RELAY, OFF);
				monitor_ch = (int)myData->COB_Client[group].config
					.ChArray2[myGroup->misc.scan_ch[0]];
				Select_OutPoint(2, monitor_ch+1, O_RUN_RELAY, ON);
				myData->cData[0].misc.checkDelayTime = 0;
				myData->cData[0].op.phase++;
			}
		} else if(myData->AppControl.config.debugType >= 1
			&& myData->AppControl.config.debugType <= 12) {
			//for 1ch test
			myGroup->phase = P30;
		}
	} else if((myData->cData[0].op.state == C_RUN
		|| myData->cData[0].op.state == C_PAUSE)
		&& myData->cData[0].op.stepType == STEP_END
		&& myData->cData[0].op.phase == P102) {

		if(myData->AppControl.config.debugType == 0) { //normal
			//for 12ch test
			if(myGroup->misc.scan_ch[0] == 11) {
				avg_ng_flag = gCompare_Ch_Average_2(group);
				if(avg_ng_flag == P0) {
					myData->cData[0].op.phase = P101;
				}
				send_save_msg_formation(group, 0);
				monitor_ch = (int)myData->COB_Client[group].config
					.ChArray2[myGroup->misc.scan_ch[0]];
				Select_OutPoint(2, monitor_ch+1, O_RUN_RELAY, OFF);
				myGroup->phase = P30;
			} else {
				if(myGroup->misc.scan_end_step_flag == P0) {
					myData->cData[0].misc.checkDelayTime = 0;
					myData->cData[0].op.phase++;
					myGroup->misc.scan_ch[0]++;
					myGroup->misc.scan_ch[1]++;
				} else {
					myGroup->misc.scan_ch[0]++;
					myGroup->misc.scan_ch[1]++;
					myGroup->misc.scan_step[0] = myGroup->misc.tmp_scan_step[0];
					monitor_ch = (int)myData->COB_Client[group].config
						.ChArray2[myGroup->misc.scan_ch[0]-1];
					Select_OutPoint(2, monitor_ch+1, O_RUN_RELAY, OFF);
					monitor_ch = (int)myData->COB_Client[group].config
						.ChArray2[myGroup->misc.scan_ch[0]];
					Select_OutPoint(2, monitor_ch+1, O_RUN_RELAY, ON);
					myData->cData[0].misc.checkDelayTime = 0;
					myData->cData[0].op.phase++;
				}
			}
		} else if(myData->AppControl.config.debugType >= 1
			&& myData->AppControl.config.debugType <= 12) {
			//for 1ch test
			myGroup->phase = P30;
		}
	}*/
}

void gRun_F_SDI_5V_400A_200A_100A_10A_2(int group)
{ //debug_size_cob
/*	int chInGroup, avg_ng_flag, monitor_ch;

	chInGroup = myData->mData.config.chInGroup[group];

	if(myData->cData[0].op.state == C_STANDBY
		&& myData->cData[0].op.phase == P2) {
		if(group == 0) {
			Select_OutPoint(2, 2, O_RUN_RELAY, OFF);
			Select_OutPoint(2, 1, O_RUN_RELAY, ON);
		} else {
			Select_OutPoint(2, 1, O_RUN_RELAY, OFF);
			Select_OutPoint(2, 2, O_RUN_RELAY, ON);
		}
		myGroup->misc.scan_step[0] = 0;
		myData->cData[0].signal[C_SIG_CMD_RUN] = P1;
	} else if(myData->cData[0].op.state == C_PAUSE
		&& myData->cData[0].op.stepType == STEP_LOOP
		&& myData->cData[0].op.phase == P1) {

		if(myData->AppControl.config.debugType == 0) { //normal
			send_save_msg_formation(group, 0);

			monitor_ch = (int)myData->COB_Client[group].config
				.ChArray2[myGroup->misc.scan_ch[0]];
			if(group == 0) {
				Select_OutPoint(2, 1, O_RUN_RELAY, OFF);
			} else {
				Select_OutPoint(2, 2, O_RUN_RELAY, OFF);
			}
			myGroup->misc.scan_ch[0] = 0;
			myGroup->misc.scan_ch[1] = 0;
			myGroup->misc.scan_step[0] = 0;
			if(myData->COB_Client[0].config.protocol_version
				== P2_PROTOCOL_VERSION) {
				myGroup->misc.scan_idxStepNo_offset[0] = 3;
			} else {
				myGroup->misc.scan_idxStepNo_offset[0] = 4;
			}

			if(myGroup->misc.total_ng_ch == (short int)chInGroup) {
				myGroup->phase = P30;
			} else {
				myGroup->phase = P13; //pause
			}
		} else if(myData->AppControl.config.debugType >= 1
			&& myData->AppControl.config.debugType <= 12) {
			//for 1ch test
			myGroup->phase = P13; //pause
		}

		myData->cData[0].op.state = C_STANDBY;
		myData->cData[0].op.phase = P0;
	} else if(myData->cData[0].op.stepType == STEP_LONG_TIME_REST
		&& ((myData->cData[0].op.state == C_RUN
			&& (myData->cData[0].op.phase == P2
			|| myData->cData[0].op.phase == P12))
		|| (myData->cData[0].op.state == C_PAUSE
			&& myData->cData[0].op.phase == P112))) {
		if(myData->AppControl.config.debugType == 0) { //normal
			avg_ng_flag = gCompare_Ch_Average_2(group);
			if(avg_ng_flag == P0) {
				myGroup->misc.long_rest_time1[myGroup->misc.scan_ch[0]]
					= myGroup->runTime;
				monitor_ch = (int)myData->COB_Client[group].config
					.ChArray2[myGroup->misc.scan_ch[0]];
				if(group == 0) {
					Select_OutPoint(2, 2, O_RUN_RELAY, OFF);
					Select_OutPoint(2, 1, O_RUN_RELAY, ON);
				} else {
					Select_OutPoint(2, 1, O_RUN_RELAY, OFF);
					Select_OutPoint(2, 2, O_RUN_RELAY, ON);
				}
				myGroup->misc.scan_ch[0] = 0;
				myGroup->misc.scan_ch[1] = 0;
				myGroup->misc.scan_idxStepNo_offset[0]
					= myData->cData[0].op.idxStepNo;
				myGroup->misc.tmp_scan_step[0] = myGroup->misc.scan_step[0];
				myGroup->misc.scan_end_step_flag = P1;
				myData->cData[0].misc.checkDelayTime = 0;
				myData->cData[0].op.phase++;
			} else {
				send_save_msg_formation(group, 0);
				monitor_ch = (int)myData->COB_Client[group].config
					.ChArray2[myGroup->misc.scan_ch[0]];
				if(group == 0) {
					Select_OutPoint(2, 1, O_RUN_RELAY, OFF);
				} else {
					Select_OutPoint(2, 2, O_RUN_RELAY, OFF);
				}
				myGroup->phase = P30;
			}
		} else if(myData->AppControl.config.debugType >= 1
			&& myData->AppControl.config.debugType <= 12) {
			//for 1ch test
			myGroup->phase = P30;
		}
	} else if((myData->cData[0].op.state == C_RUN
		|| myData->cData[0].op.state == C_PAUSE)
		&& myData->cData[0].op.stepType == STEP_END
		&& myData->cData[0].op.phase == P102) {

		if(myData->AppControl.config.debugType == 0) { //normal
			avg_ng_flag = gCompare_Ch_Average_2(group);
			if(avg_ng_flag == P0) {
				myData->cData[0].op.phase = P101;
			}
			send_save_msg_formation(group, 0);
			monitor_ch = (int)myData->COB_Client[group].config
				.ChArray2[myGroup->misc.scan_ch[0]];
			if(group == 0) {
				Select_OutPoint(2, 1, O_RUN_RELAY, OFF);
			} else {
				Select_OutPoint(2, 2, O_RUN_RELAY, OFF);
			}
			myGroup->phase = P30;
		} else if(myData->AppControl.config.debugType >= 1
			&& myData->AppControl.config.debugType <= 12) {
			//for 1ch test
			myGroup->phase = P30;
		}
	}*/
}

void gRun_F_SDI_5V_450A_200A_100A_10A(int group)
{ //debug_size_cob
/*	unsigned char ch1_flag, ch2_flag;
	int chInGroup, avg_ng_flag, monitor_ch, scan_ch1, scan_ch2;
	S_CH_DATA *myCh1, *myCh2;

	chInGroup = myData->mData.config.chInGroup[group];

	myCh1 = &(myData->cData[0]);
	scan_ch1 = myGroup->misc.scan_ch[0];

	myCh2 = &(myData->cData[1]);
	scan_ch2 = myGroup->misc.scan_ch[1];

	//C_STANDBY
	if(myCh1->op.state == C_STANDBY && myCh1->op.phase == P2) ch1_flag = 1;
	else ch1_flag = 0;

	if(ch1_flag == 1) {
		monitor_ch = (int)myData->COB_Client[group].config
			.ChArray2[scan_ch1-1];
		Select_OutPoint(2, monitor_ch+1, O_RUN_RELAY, OFF);
		monitor_ch = (int)myData->COB_Client[group].config
			.ChArray2[scan_ch1];
		Select_OutPoint(2, monitor_ch+1, O_RUN_RELAY, ON);

		myCh1->signal[C_SIG_CMD_RUN] = P1;
		myGroup->misc.scan_step[0] = 0;
	}

	if(myCh2->op.state == C_STANDBY && myCh2->op.phase == P2) ch2_flag = 1;
	else ch2_flag = 0;

	if(ch2_flag == 1) {
		monitor_ch = (int)myData->COB_Client[group].config
			.ChArray2[scan_ch2-1];
		Select_OutPoint(3, monitor_ch+1, O_RUN_RELAY, OFF);
		monitor_ch = (int)myData->COB_Client[group].config
			.ChArray2[scan_ch2];
		Select_OutPoint(3, monitor_ch+1, O_RUN_RELAY, ON);

		myCh2->signal[C_SIG_CMD_RUN] = P1;
		myGroup->misc.scan_step[1] = 0;
	}

	//STEP_LOOP, C_PAUSE
	if(myCh1->op.stepType == STEP_LOOP && myCh1->op.state == C_PAUSE
		&& myCh1->op.phase == P1) ch1_flag = 1;
	else ch1_flag = 0;

	if(myCh2->op.stepType == STEP_LOOP && myCh2->op.state == C_PAUSE
		&& myCh2->op.phase == P1) ch2_flag = 1;
	else ch2_flag = 0;

	if(ch1_flag == 1 && ch2_flag == 1) {
		if(scan_ch1 == 11 && scan_ch2 == 11) {
			send_save_msg_formation(group, 0);

			monitor_ch = (int)myData->COB_Client[group].config
				.ChArray2[scan_ch1];
			Select_OutPoint(2, monitor_ch+1, O_RUN_RELAY, OFF);
			monitor_ch = (int)myData->COB_Client[group].config
				.ChArray2[scan_ch2];
			Select_OutPoint(3, monitor_ch+1, O_RUN_RELAY, OFF);

			myGroup->misc.scan_ch[0] = 0;
			myGroup->misc.scan_step[0] = 0;
			myGroup->misc.scan_ch[1] = 0;
			myGroup->misc.scan_step[1] = 0;

			if(myData->COB_Client[0].config.protocol_version
				== P2_PROTOCOL_VERSION) {
				myGroup->misc.scan_idxStepNo_offset[0] = 3;
				myGroup->misc.scan_idxStepNo_offset[1] = 3;
			} else {
				myGroup->misc.scan_idxStepNo_offset[0] = 4;
				myGroup->misc.scan_idxStepNo_offset[1] = 4;
			}

			if(myGroup->misc.total_ng_ch == (short int)chInGroup) {
				myGroup->phase = P30;
			} else {
				myGroup->phase = P13; //pause
			}

			myCh1->op.state = C_STANDBY;
			myCh1->op.phase = P0;
			myCh2->op.state = C_STANDBY;
			myCh2->op.phase = P0;
			return;
		}
	}

	if(ch1_flag == 1) {
		if(myData->AppControl.config.debugType == 0) { //normal
			//for 12ch test
			if(scan_ch1 == 11) {
			} else {
				myGroup->misc.scan_ch[0]++;
				myCh1->op.state = C_STANDBY;
				myCh1->op.phase = P0;
			}
		} else if(myData->AppControl.config.debugType >= 1
			&& myData->AppControl.config.debugType <= 12) {
			//for 1ch test
			myCh1->op.state = C_STANDBY;
			myCh1->op.phase = P0;
			myGroup->phase = P13; //pause
		}
	}

	if(ch2_flag == 1) {
		if(myData->AppControl.config.debugType == 0) { //normal
			//for 12ch test
			if(scan_ch2 == 11) {
			} else {
				myGroup->misc.scan_ch[1]++;
				myCh2->op.state = C_STANDBY;
				myCh2->op.phase = P0;
			}
		} else if(myData->AppControl.config.debugType >= 1
			&& myData->AppControl.config.debugType <= 12) {
			//for 1ch test
			myCh2->op.state = C_STANDBY;
			myCh2->op.phase = P0;
			myGroup->phase = P13; //pause
		}
	}

	//STEP_LONG_TIME_REST, C_RUN, C_PAUSE
	ch1_flag = 0;
	if(myCh1->op.stepType == STEP_LONG_TIME_REST) ch1_flag++;
	if(myCh1->op.state == C_RUN
		&& (myCh1->op.phase == P2 || myCh1->op.phase == P12)) ch1_flag++;
	if(myCh1->op.state == C_PAUSE && myCh1->op.phase == P112) ch1_flag++;
	if(ch1_flag >= 2) ch1_flag = 1;
	else ch1_flag = 0;

	ch2_flag = 0;
	if(myCh2->op.stepType == STEP_LONG_TIME_REST) ch2_flag++;
	if(myCh2->op.state == C_RUN
		&& (myCh2->op.phase == P2 || myCh2->op.phase == P12)) ch2_flag++;
	if(myCh2->op.state == C_PAUSE && myCh2->op.phase == P112) ch2_flag++;
	if(ch2_flag >= 2) ch2_flag = 1;
	else ch2_flag = 0;

	if(ch1_flag == 1 && ch2_flag == 1) {
		if(scan_ch1 == 11 && scan_ch2 == 11) {
			avg_ng_flag = gCompare_Ch_Average_2(group);
			if(avg_ng_flag == P0) {
				myGroup->misc.long_rest_time1[scan_ch1] = myGroup->runTime;
				monitor_ch = (int)myData->COB_Client[group].config
					.ChArray2[scan_ch1];
				Select_OutPoint(2, monitor_ch+1, O_RUN_RELAY, OFF);
				Select_OutPoint(2, 1, O_RUN_RELAY, ON);
				myGroup->misc.long_rest_time1[scan_ch2+12] = myGroup->runTime;
				monitor_ch = (int)myData->COB_Client[group].config
					.ChArray2[scan_ch2];
				Select_OutPoint(3, monitor_ch+1, O_RUN_RELAY, OFF);
				Select_OutPoint(3, 1, O_RUN_RELAY, ON);
				myGroup->misc.scan_ch[0] = 0;
				myGroup->misc.scan_ch[1] = 0;
				myGroup->misc.scan_idxStepNo_offset[0]
					= myCh1->op.idxStepNo;
				myGroup->misc.scan_idxStepNo_offset[1]
					= myCh2->op.idxStepNo;
				myGroup->misc.tmp_scan_step[0] = myGroup->misc.scan_step[0];
				myGroup->misc.tmp_scan_step[1] = myGroup->misc.scan_step[1];
				myGroup->misc.scan_end_step_flag = P1;
				myCh1->misc.checkDelayTime = 0;
				myCh1->op.phase++;
				myCh2->misc.checkDelayTime = 0;
				myCh2->op.phase++;
			} else {
				send_save_msg_formation(group, 0);
				monitor_ch = (int)myData->COB_Client[group].config
					.ChArray2[scan_ch1];
				Select_OutPoint(2, monitor_ch+1, O_RUN_RELAY, OFF);
				monitor_ch = (int)myData->COB_Client[group].config
					.ChArray2[scan_ch2];
				Select_OutPoint(3, monitor_ch+1, O_RUN_RELAY, OFF);
				myGroup->phase = P30;
			}
			return;
		}
	}

	if(ch1_flag == 1) {
		if(myData->AppControl.config.debugType == 0) { //normal
			//for 12ch test
			if(scan_ch1 == 11) {
			} else {
				myGroup->misc.long_rest_time1[scan_ch1] = myGroup->runTime;
				myGroup->misc.scan_ch[0]++;
				scan_ch1 = myGroup->misc.scan_ch[0];
				myGroup->misc.scan_step[0] = myGroup->misc.tmp_scan_step[0];
				monitor_ch = (int)myData->COB_Client[group].config
					.ChArray2[scan_ch1 - 1];
				Select_OutPoint(2, monitor_ch+1, O_RUN_RELAY, OFF);
				monitor_ch = (int)myData->COB_Client[group].config
					.ChArray2[scan_ch1];
				Select_OutPoint(2, monitor_ch+1, O_RUN_RELAY, ON);
				myCh1->misc.checkDelayTime = 0;
				myCh1->op.phase++;
			}
		} else if(myData->AppControl.config.debugType >= 1
			&& myData->AppControl.config.debugType <= 12) {
			//for 1ch test
			myGroup->phase = P30;
		}
	}

	if(ch2_flag == 1) {
		if(myData->AppControl.config.debugType == 0) { //normal
			//for 12ch test
			if(scan_ch2 == 11) {
			} else {
				myGroup->misc.long_rest_time1[scan_ch2+12] = myGroup->runTime;
				myGroup->misc.scan_ch[1]++;
				scan_ch2 = myGroup->misc.scan_ch[1];
				myGroup->misc.scan_step[1] = myGroup->misc.tmp_scan_step[1];
				monitor_ch = (int)myData->COB_Client[group].config
					.ChArray2[scan_ch2 - 1];
				Select_OutPoint(3, monitor_ch+1, O_RUN_RELAY, OFF);
				monitor_ch = (int)myData->COB_Client[group].config
					.ChArray2[scan_ch2];
				Select_OutPoint(3, monitor_ch+1, O_RUN_RELAY, ON);
				myCh2->misc.checkDelayTime = 0;
				myCh2->op.phase++;
			}
		} else if(myData->AppControl.config.debugType >= 1
			&& myData->AppControl.config.debugType <= 12) {
			//for 1ch test
			myGroup->phase = P30;
		}
	}

	//STEP_END, C_RUN, C_PAUSE
	if(myCh1->op.stepType == STEP_END && (myCh1->op.state == C_RUN
		|| myCh1->op.state == C_PAUSE) && myCh1->op.phase == P102) ch1_flag = 1;
	else ch1_flag = 0;

	if(myCh2->op.stepType == STEP_END && (myCh2->op.state == C_RUN
		|| myCh2->op.state == C_PAUSE) && myCh2->op.phase == P102) ch2_flag = 1;
	else ch2_flag = 0;

	if(ch1_flag == 1 && ch2_flag == 1) {
		if(scan_ch1 == 11 && scan_ch2 == 11) {
			avg_ng_flag = gCompare_Ch_Average_2(group);
			if(avg_ng_flag == P0) {
				myCh1->op.phase = P101;
				myCh2->op.phase = P101;
			}
			send_save_msg_formation(group, 0);
			monitor_ch = (int)myData->COB_Client[group].config
				.ChArray2[scan_ch1];
			Select_OutPoint(2, monitor_ch+1, O_RUN_RELAY, OFF);
			monitor_ch = (int)myData->COB_Client[group].config
				.ChArray2[scan_ch2];
			Select_OutPoint(3, monitor_ch+1, O_RUN_RELAY, OFF);
			myGroup->phase = P30;
			return;
		}
	}

	if(ch1_flag == 1) {
		if(myData->AppControl.config.debugType == 0) { //normal
			//for 12ch test
			if(scan_ch1 == 11) {
			} else {
				if(myGroup->misc.scan_end_step_flag == P0) {
					myGroup->misc.scan_ch[0]++;
					myCh1->misc.checkDelayTime = 0;
					myCh1->op.phase++;
				} else {
					myGroup->misc.scan_ch[0]++;
					myCh1->misc.checkDelayTime = 0;
					myCh1->op.phase++;
					scan_ch1 = myGroup->misc.scan_ch[0];
					myGroup->misc.scan_step[0]
						= myGroup->misc.tmp_scan_step[0];
					monitor_ch = (int)myData->COB_Client[group].config
						.ChArray2[scan_ch1 - 1];
					Select_OutPoint(2, monitor_ch+1, O_RUN_RELAY, OFF);
					monitor_ch = (int)myData->COB_Client[group].config
						.ChArray2[scan_ch1];
					Select_OutPoint(2, monitor_ch+1, O_RUN_RELAY, ON);
				}
			}
		} else if(myData->AppControl.config.debugType >= 1
			&& myData->AppControl.config.debugType <= 12) {
			//for 1ch test
			myGroup->phase = P30;
		}
	}

	if(ch2_flag == 1) {
		if(myData->AppControl.config.debugType == 0) { //normal
			//for 12ch test
			if(scan_ch2 == 11) {
			} else {
				if(myGroup->misc.scan_end_step_flag == P0) {
					myGroup->misc.scan_ch[1]++;
					myCh2->misc.checkDelayTime = 0;
					myCh2->op.phase++;
				} else {
					myGroup->misc.scan_ch[1]++;
					myCh2->misc.checkDelayTime = 0;
					myCh2->op.phase++;
					scan_ch2 = myGroup->misc.scan_ch[1];
					myGroup->misc.scan_step[1]
						= myGroup->misc.tmp_scan_step[1];
					monitor_ch = (int)myData->COB_Client[group].config
						.ChArray2[scan_ch2 - 1];
					Select_OutPoint(3, monitor_ch+1, O_RUN_RELAY, OFF);
					monitor_ch = (int)myData->COB_Client[group].config
						.ChArray2[scan_ch2];
					Select_OutPoint(3, monitor_ch+1, O_RUN_RELAY, ON);
				}
			}
		} else if(myData->AppControl.config.debugType >= 1
			&& myData->AppControl.config.debugType <= 12) {
			//for 1ch test
			myGroup->phase = P30;
		}
	}*/
}

void gPause_F1(int group)
{ //debug_size_cob
/*	int i, j, chInGroup, monitor_ch;
	S_MSG_VAL SendMsg;

	chInGroup = myData->mData.config.chInGroup[group];

	switch(myGroup->phase) {
		case P0:
			myGroup->cmd[CMD_MODE_USER] = CMD_TYPE_NONE;
			myGroup->phase = P1;
			break;
		case P1:
			if(myGroup->code == M_CD_FAULT_AC_POWER_SHORT
				|| myGroup->attribute == ATTR_CHECK1_END
				|| myGroup->attribute == ATTR_CHECK2_END
				|| myGroup->attribute == ATTR_CHECK3_END) {
				if(myGroup->cmd[CMD_MODE_USER] == CMD_TYPE_CONTINUE) {
					myGroup->cmd[CMD_MODE_USER] = CMD_TYPE_NONE;

					memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
					SendMsg.msg = MSG_MODULE_JIGM_TRY_CONTACT;
					SendMsg.val[0] = group;
					send_msg(MODULE_TO_JIGM, (char *)&SendMsg);

					myGroup->code = G_CD_NONE;
					myGroup->state = G_RUN;
					myGroup->phase = P2;
				} else if(myGroup->cmd[CMD_MODE_STEP] == CMD_TYPE_CONTINUE) {
					myGroup->cmd[CMD_MODE_STEP] = CMD_TYPE_NONE;

					memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
					SendMsg.msg = MSG_MODULE_JIGM_TRY_CONTACT;
					SendMsg.val[0] = group;
					send_msg(MODULE_TO_JIGM, (char *)&SendMsg);

					myGroup->code = G_CD_NONE;
					myGroup->state = G_RUN;
					myGroup->phase = P2;
				} else if(myGroup->cmd[CMD_MODE_USER] == CMD_TYPE_STOP) {
					myGroup->cmd[CMD_MODE_USER] = CMD_TYPE_NONE;
					
					memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
					SendMsg.msg = MSG_MODULE_JIGM_TRY_DISCONTACT;
					SendMsg.val[0] = group;
					send_msg(MODULE_TO_JIGM, (char *)&SendMsg);
					myGroup->phase = P4;
				} else {
					myGroup->cmd[CMD_MODE_USER] = CMD_TYPE_NONE;
					myGroup->cmd[CMD_MODE_STEP] = CMD_TYPE_NONE;
				}
			} else {
				myGroup->phase = P2;
			}
			break;
		case P2:
			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
			SendMsg.msg = MSG_MODULE_JIGM_TRY_DISCONTACT;
			SendMsg.val[0] = group;
			send_msg(MODULE_TO_JIGM, (char *)&SendMsg);
			myGroup->phase = P3;
			break;
		case P3:
			if(myGroup->cmd[CMD_MODE_USER] == CMD_TYPE_CONTINUE) {
				myGroup->cmd[CMD_MODE_USER] = CMD_TYPE_NONE;

				memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
				SendMsg.msg = MSG_MODULE_JIGM_TRY_CONTACT;
				SendMsg.val[0] = group;
				send_msg(MODULE_TO_JIGM, (char *)&SendMsg);

				monitor_ch = (int)myData->COB_Client[group].config
					.ChArray2[myGroup->misc.scan_ch[0]];
				Select_OutPoint(2, monitor_ch+1, O_RUN_RELAY, ON);

				myGroup->code = G_CD_NONE;
				myGroup->state = G_RUN;
				myGroup->phase = P2;
			} else if(myGroup->cmd[CMD_MODE_USER] == CMD_TYPE_STOP) {
				myGroup->cmd[CMD_MODE_USER] = CMD_TYPE_NONE;

				myGroup->phase = P4;
			} else {
				myGroup->cmd[CMD_MODE_USER] = CMD_TYPE_NONE;
				myGroup->cmd[CMD_MODE_STEP] = CMD_TYPE_NONE;
			}

			if(myGroup->signal[G_SIG_JIG_FAIL] == P1) {
				myGroup->signal[G_SIG_JIG_FAIL] = P2;
			}
			break;
		case P4:
			j = myData->mData.config.chPerBd * group;
			for(i=j; i < j + chInGroup; i++) {
				if(myData->cData[i].op.state == C_PAUSE) {
					myData->cData[i].signal[C_SIG_CMD_STOP] = P1;
				}
			}

			myGroup->cmd[CMD_MODE_USER] = CMD_TYPE_NONE;
			myGroup->state = G_STANDBY;
			myGroup->phase = P0;
			break;
		case P10:
			if(myGroup->cmd[CMD_MODE_USER] == CMD_TYPE_CONTINUE) {
				myGroup->cmd[CMD_MODE_USER] = CMD_TYPE_NONE;

				memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
				SendMsg.msg = MSG_MODULE_JIGM_TRY_CONTACT;
				SendMsg.val[0] = group;
				send_msg(MODULE_TO_JIGM, (char *)&SendMsg);

				myGroup->code = G_CD_NONE;
				myGroup->phase = P11;
			} else if(myGroup->cmd[CMD_MODE_STEP] == CMD_TYPE_CONTINUE) {
				myGroup->cmd[CMD_MODE_STEP] = CMD_TYPE_NONE;

				memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
				SendMsg.msg = MSG_MODULE_JIGM_TRY_CONTACT;
				SendMsg.val[0] = group;
				send_msg(MODULE_TO_JIGM, (char *)&SendMsg);

				myGroup->code = G_CD_NONE;
				myGroup->state = G_RUN;
				myGroup->phase = P2;
			} else if(myGroup->cmd[CMD_MODE_USER] == CMD_TYPE_STOP) {
				myGroup->cmd[CMD_MODE_USER] = CMD_TYPE_NONE;
				
				memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
				SendMsg.msg = MSG_MODULE_JIGM_TRY_DISCONTACT;
				SendMsg.val[0] = group;
				send_msg(MODULE_TO_JIGM, (char *)&SendMsg);
				myGroup->phase = P4;
			} else {
				myGroup->cmd[CMD_MODE_USER] = CMD_TYPE_NONE;
				myGroup->cmd[CMD_MODE_STEP] = CMD_TYPE_NONE;
			}
			break;
		case P11:
			myGroup->misc.scan_ch[0] = 0;
			myGroup->misc.scan_ch[1] = 0;
			myGroup->misc.scan_step[0] = 0;
			if(myData->AppControl.config.debugType == 0) {
				monitor_ch = (int)myData->COB_Client[group].config
					.ChArray2[myGroup->misc.scan_ch[0]];
				Select_OutPoint(2, monitor_ch+1, O_RUN_RELAY, ON);
			}

			myGroup->state = G_RUN;
			myGroup->phase = P2;
			break;
		default: break;
	}*/
}

void gPause_F2(int group)
{ //debug_size_cob
/*	int i, j, chInGroup, monitor_ch;
	S_MSG_VAL SendMsg;

	chInGroup = myData->mData.config.chInGroup[group];

	switch(myGroup->phase) {
		case P0:
			myGroup->cmd[CMD_MODE_USER] = CMD_TYPE_NONE;
			myGroup->phase = P1;
			break;
		case P1:
			if(myGroup->code == M_CD_FAULT_AC_POWER_SHORT
				|| myGroup->attribute == ATTR_CHECK1_END
				|| myGroup->attribute == ATTR_CHECK2_END
				|| myGroup->attribute == ATTR_CHECK3_END) {
				if(myGroup->cmd[CMD_MODE_USER] == CMD_TYPE_CONTINUE) {
					myGroup->cmd[CMD_MODE_USER] = CMD_TYPE_NONE;

					memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
					SendMsg.msg = MSG_MODULE_JIGM_TRY_CONTACT;
					SendMsg.val[0] = group;
					send_msg(MODULE_TO_JIGM, (char *)&SendMsg);

					myGroup->code = G_CD_NONE;
					myGroup->state = G_RUN;
					myGroup->phase = P2;
				} else if(myGroup->cmd[CMD_MODE_STEP] == CMD_TYPE_CONTINUE) {
					myGroup->cmd[CMD_MODE_STEP] = CMD_TYPE_NONE;

					memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
					SendMsg.msg = MSG_MODULE_JIGM_TRY_CONTACT;
					SendMsg.val[0] = group;
					send_msg(MODULE_TO_JIGM, (char *)&SendMsg);

					myGroup->code = G_CD_NONE;
					myGroup->state = G_RUN;
					myGroup->phase = P2;
				} else if(myGroup->cmd[CMD_MODE_USER] == CMD_TYPE_STOP) {
					myGroup->cmd[CMD_MODE_USER] = CMD_TYPE_NONE;
					
					memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
					SendMsg.msg = MSG_MODULE_JIGM_TRY_DISCONTACT;
					SendMsg.val[0] = group;
					send_msg(MODULE_TO_JIGM, (char *)&SendMsg);
					myGroup->phase = P4;
				} else {
					myGroup->cmd[CMD_MODE_USER] = CMD_TYPE_NONE;
					myGroup->cmd[CMD_MODE_STEP] = CMD_TYPE_NONE;
				}
			} else {
				myGroup->phase = P2;
			}
			break;
		case P2:
			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
			SendMsg.msg = MSG_MODULE_JIGM_TRY_DISCONTACT;
			SendMsg.val[0] = group;
			send_msg(MODULE_TO_JIGM, (char *)&SendMsg);
			myGroup->phase = P3;
			break;
		case P3:
			if(myGroup->cmd[CMD_MODE_USER] == CMD_TYPE_CONTINUE) {
				myGroup->cmd[CMD_MODE_USER] = CMD_TYPE_NONE;

				memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
				SendMsg.msg = MSG_MODULE_JIGM_TRY_CONTACT;
				SendMsg.val[0] = group;
				send_msg(MODULE_TO_JIGM, (char *)&SendMsg);

				monitor_ch = (int)myData->COB_Client[group].config
					.ChArray2[myGroup->misc.scan_ch[0]];
				if(myData->AppControl.config.systemModel
					== F_SDI_5V_400A_200A_100A_10A_2) {
					monitor_ch += group;
				}
				Select_OutPoint(2, monitor_ch+1, O_RUN_RELAY, ON);

				myGroup->code = G_CD_NONE;
				myGroup->state = G_RUN;
				myGroup->phase = P2;
			} else if(myGroup->cmd[CMD_MODE_USER] == CMD_TYPE_STOP) {
				myGroup->cmd[CMD_MODE_USER] = CMD_TYPE_NONE;

				myGroup->phase = P4;
			} else {
				myGroup->cmd[CMD_MODE_USER] = CMD_TYPE_NONE;
				myGroup->cmd[CMD_MODE_STEP] = CMD_TYPE_NONE;
			}

			if(myGroup->signal[G_SIG_JIG_FAIL] == P1) {
				myGroup->signal[G_SIG_JIG_FAIL] = P2;
			}
			break;
		case P4:
			if(myData->AppControl.config.systemModel
				== F_SDI_5V_400A_200A_100A_10A_2) {
				j = 0;
			} else {
				j = myData->mData.config.chPerBd * group;
			}
			for(i=j; i < j + chInGroup; i++) {
				if(myData->cData[i].op.state == C_PAUSE) {
					myData->cData[i].signal[C_SIG_CMD_STOP] = P1;
				}
			}

			myGroup->cmd[CMD_MODE_USER] = CMD_TYPE_NONE;
			myGroup->state = G_STANDBY;
			myGroup->phase = P0;
			break;
		case P10:
			if(myGroup->cmd[CMD_MODE_USER] == CMD_TYPE_CONTINUE) {
				myGroup->cmd[CMD_MODE_USER] = CMD_TYPE_NONE;

				memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
				SendMsg.msg = MSG_MODULE_JIGM_TRY_CONTACT;
				SendMsg.val[0] = group;
				send_msg(MODULE_TO_JIGM, (char *)&SendMsg);

				myGroup->code = G_CD_NONE;
				myGroup->phase = P11;
			} else if(myGroup->cmd[CMD_MODE_STEP] == CMD_TYPE_CONTINUE) {
				myGroup->cmd[CMD_MODE_STEP] = CMD_TYPE_NONE;

				memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
				SendMsg.msg = MSG_MODULE_JIGM_TRY_CONTACT;
				SendMsg.val[0] = group;
				send_msg(MODULE_TO_JIGM, (char *)&SendMsg);

				myGroup->code = G_CD_NONE;
				myGroup->state = G_RUN;
				myGroup->phase = P2;
			} else if(myGroup->cmd[CMD_MODE_USER] == CMD_TYPE_STOP) {
				myGroup->cmd[CMD_MODE_USER] = CMD_TYPE_NONE;
				
				memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
				SendMsg.msg = MSG_MODULE_JIGM_TRY_DISCONTACT;
				SendMsg.val[0] = group;
				send_msg(MODULE_TO_JIGM, (char *)&SendMsg);
				myGroup->phase = P4;
			} else {
				myGroup->cmd[CMD_MODE_USER] = CMD_TYPE_NONE;
				myGroup->cmd[CMD_MODE_STEP] = CMD_TYPE_NONE;
			}
			break;
		case P11:
			myGroup->misc.scan_ch[0] = 0;
			myGroup->misc.scan_ch[1] = 0;
			myGroup->misc.scan_step[0] = 0;
			if(myData->AppControl.config.debugType == 0) {
				monitor_ch = (int)myData->COB_Client[group].config
					.ChArray2[myGroup->misc.scan_ch[0]];
				if(myData->AppControl.config.systemModel
					== F_SDI_5V_400A_200A_100A_10A_2) {
					if(group == 0) {
						Select_OutPoint(2, 1, O_RUN_RELAY, ON);
					} else {
						Select_OutPoint(2, 2, O_RUN_RELAY, ON);
					}
				} else {
					Select_OutPoint(2, monitor_ch+1, O_RUN_RELAY, ON);
				}
			}

			myGroup->state = G_RUN;
			myGroup->phase = P2;
			break;
		default: break;
	}*/
}

void gPause_F3(int group)
{ //debug_size_cob
/*	int i, j, chInGroup, monitor_ch;
	S_MSG_VAL SendMsg;

	chInGroup = myData->mData.config.chInGroup[group];

	switch(myGroup->phase) {
		case P0:
			myGroup->signal[G_SIG_SAFETY_RESISTOR] = SR_OPEN;
			myGroup->cmd[CMD_MODE_USER] = CMD_TYPE_NONE;
			myGroup->phase = P1;
			break;
		case P1:
			if(myGroup->attribute == ATTR_CHECK1_END
				|| myGroup->attribute == ATTR_CHECK2_END
				|| myGroup->attribute == ATTR_CHECK3_END) {
				if(myGroup->cmd[CMD_MODE_USER] == CMD_TYPE_CONTINUE) {
					myGroup->cmd[CMD_MODE_USER] = CMD_TYPE_NONE;

					memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
					SendMsg.msg = MSG_MODULE_JIGM_TRY_CONTACT;
					SendMsg.val[0] = group;
					send_msg(MODULE_TO_JIGM, (char *)&SendMsg);

					myGroup->signal[G_SIG_SAFETY_RESISTOR] = SR_CLOSE;
					myGroup->code = G_CD_NONE;
					myGroup->state = G_RUN;
					myGroup->phase = P2;
				} else if(myGroup->cmd[CMD_MODE_STEP] == CMD_TYPE_CONTINUE) {
					myGroup->cmd[CMD_MODE_STEP] = CMD_TYPE_NONE;

					memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
					SendMsg.msg = MSG_MODULE_JIGM_TRY_CONTACT;
					SendMsg.val[0] = group;
					send_msg(MODULE_TO_JIGM, (char *)&SendMsg);

					myGroup->signal[G_SIG_SAFETY_RESISTOR] = SR_CLOSE;
					myGroup->code = G_CD_NONE;
					myGroup->state = G_RUN;
					myGroup->phase = P2;
				} else if(myGroup->cmd[CMD_MODE_USER] == CMD_TYPE_STOP) {
					myGroup->cmd[CMD_MODE_USER] = CMD_TYPE_NONE;
					
					memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
					SendMsg.msg = MSG_MODULE_JIGM_TRY_DISCONTACT;
					SendMsg.val[0] = group;
					send_msg(MODULE_TO_JIGM, (char *)&SendMsg);
					myGroup->phase = P4;
				} else {
					myGroup->cmd[CMD_MODE_USER] = CMD_TYPE_NONE;
					myGroup->cmd[CMD_MODE_STEP] = CMD_TYPE_NONE;
				}
			} else {
				myGroup->phase = P2;
			}
			break;
		case P2:
			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
			SendMsg.msg = MSG_MODULE_JIGM_TRY_DISCONTACT;
			SendMsg.val[0] = group;
			send_msg(MODULE_TO_JIGM, (char *)&SendMsg);
			myGroup->phase = P3;
			break;
		case P3:
			if(myGroup->cmd[CMD_MODE_USER] == CMD_TYPE_CONTINUE) {
				myGroup->cmd[CMD_MODE_USER] = CMD_TYPE_NONE;

				memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
				SendMsg.msg = MSG_MODULE_JIGM_TRY_CONTACT;
				SendMsg.val[0] = group;
				send_msg(MODULE_TO_JIGM, (char *)&SendMsg);

				monitor_ch = (int)myData->COB_Client[group].config
					.ChArray2[myGroup->misc.scan_ch[0]];
				Select_OutPoint(2, monitor_ch+1, O_RUN_RELAY, ON);
				monitor_ch = (int)myData->COB_Client[group].config
					.ChArray2[myGroup->misc.scan_ch[1]];
				Select_OutPoint(3, monitor_ch+1, O_RUN_RELAY, ON);

				myGroup->code = G_CD_NONE;
				myGroup->state = G_RUN;
				myGroup->phase = P2;
			} else if(myGroup->cmd[CMD_MODE_USER] == CMD_TYPE_STOP) {
				myGroup->cmd[CMD_MODE_USER] = CMD_TYPE_NONE;

				myGroup->phase = P4;
			} else {
				myGroup->cmd[CMD_MODE_USER] = CMD_TYPE_NONE;
				myGroup->cmd[CMD_MODE_STEP] = CMD_TYPE_NONE;
			}

			if(myGroup->signal[G_SIG_JIG_FAIL] == P1) {
				myGroup->signal[G_SIG_JIG_FAIL] = P2;
			}
			break;
		case P4:
			j = myData->mData.config.chPerBd * group;
			for(i=j; i < j + chInGroup; i++) {
				if(myData->cData[i].op.state == C_PAUSE) {
					myData->cData[i].signal[C_SIG_CMD_STOP] = P1;
				}
			}

			myGroup->cmd[CMD_MODE_USER] = CMD_TYPE_NONE;
			myGroup->state = G_STANDBY;
			myGroup->phase = P0;
			break;
		case P10:
			if(myGroup->cmd[CMD_MODE_USER] == CMD_TYPE_CONTINUE) {
				myGroup->cmd[CMD_MODE_USER] = CMD_TYPE_NONE;

				memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
				SendMsg.msg = MSG_MODULE_JIGM_TRY_CONTACT;
				SendMsg.val[0] = group;
				send_msg(MODULE_TO_JIGM, (char *)&SendMsg);

				myGroup->signal[G_SIG_SAFETY_RESISTOR] = SR_CLOSE;
				myGroup->code = G_CD_NONE;
				myGroup->phase = P11;
			} else if(myGroup->cmd[CMD_MODE_STEP] == CMD_TYPE_CONTINUE) {
				myGroup->cmd[CMD_MODE_STEP] = CMD_TYPE_NONE;

				memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
				SendMsg.msg = MSG_MODULE_JIGM_TRY_CONTACT;
				SendMsg.val[0] = group;
				send_msg(MODULE_TO_JIGM, (char *)&SendMsg);

				myGroup->signal[G_SIG_SAFETY_RESISTOR] = SR_CLOSE;
				myGroup->code = G_CD_NONE;
				myGroup->state = G_RUN;
				myGroup->phase = P2;
			} else if(myGroup->cmd[CMD_MODE_USER] == CMD_TYPE_STOP) {
				myGroup->cmd[CMD_MODE_USER] = CMD_TYPE_NONE;
				
				memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
				SendMsg.msg = MSG_MODULE_JIGM_TRY_DISCONTACT;
				SendMsg.val[0] = group;
				send_msg(MODULE_TO_JIGM, (char *)&SendMsg);
				myGroup->phase = P4;
			} else {
				myGroup->cmd[CMD_MODE_USER] = CMD_TYPE_NONE;
				myGroup->cmd[CMD_MODE_STEP] = CMD_TYPE_NONE;
			}
			break;
		case P11:
			myGroup->misc.scan_ch[0] = 0;
			myGroup->misc.scan_ch[1] = 0;
			myGroup->misc.scan_step[0] = 0;
			myGroup->misc.scan_step[1] = 0;
			if(myData->AppControl.config.debugType == 0) {
				monitor_ch = (int)myData->COB_Client[group].config
					.ChArray2[myGroup->misc.scan_ch[0]];
				Select_OutPoint(2, monitor_ch+1, O_RUN_RELAY, ON);
				monitor_ch = (int)myData->COB_Client[group].config
					.ChArray2[myGroup->misc.scan_ch[1]];
				Select_OutPoint(3, monitor_ch+1, O_RUN_RELAY, ON);
			}

			myGroup->state = G_RUN;
			myGroup->phase = P2;
			break;
		default: break;
	}*/
}

void gPause_F4(int group)
{ //debug_size_cob
/*	int ch, i, chInGroup, monitor_ch;
	S_MSG_VAL SendMsg;

	chInGroup = myData->mData.config.chInGroup[group];
	ch = myGroup->misc.chOffset;

	switch(myGroup->phase) {
		case P0:
			myGroup->signal[G_SIG_SAFETY_RESISTOR] = SR_OPEN;
			myGroup->cmd[CMD_MODE_USER] = CMD_TYPE_NONE;
			myGroup->phase = P1;
			break;
		case P1:
			if(myGroup->attribute == ATTR_CHECK1_END
				|| myGroup->attribute == ATTR_CHECK2_END
				|| myGroup->attribute == ATTR_CHECK3_END) {
				if(myGroup->cmd[CMD_MODE_USER] == CMD_TYPE_CONTINUE) {
					myGroup->cmd[CMD_MODE_USER] = CMD_TYPE_NONE;

					memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
					SendMsg.msg = MSG_MODULE_JIGM_TRY_CONTACT;
					SendMsg.val[0] = group;
					send_msg(MODULE_TO_JIGM, (char *)&SendMsg);

					myGroup->signal[G_SIG_SAFETY_RESISTOR] = SR_CLOSE;
					myGroup->code = G_CD_NONE;
					myGroup->state = G_RUN;
					myGroup->phase = P2;
				} else if(myGroup->cmd[CMD_MODE_STEP] == CMD_TYPE_CONTINUE) {
					myGroup->cmd[CMD_MODE_STEP] = CMD_TYPE_NONE;

					memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
					SendMsg.msg = MSG_MODULE_JIGM_TRY_CONTACT;
					SendMsg.val[0] = group;
					send_msg(MODULE_TO_JIGM, (char *)&SendMsg);

					myGroup->signal[G_SIG_SAFETY_RESISTOR] = SR_CLOSE;
					myGroup->code = G_CD_NONE;
					myGroup->state = G_RUN;
					myGroup->phase = P2;
				} else if(myGroup->cmd[CMD_MODE_USER] == CMD_TYPE_STOP) {
					myGroup->cmd[CMD_MODE_USER] = CMD_TYPE_NONE;
					
					memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
					SendMsg.msg = MSG_MODULE_JIGM_TRY_DISCONTACT;
					SendMsg.val[0] = group;
					send_msg(MODULE_TO_JIGM, (char *)&SendMsg);
					myGroup->phase = P4;
				} else {
					myGroup->cmd[CMD_MODE_USER] = CMD_TYPE_NONE;
					myGroup->cmd[CMD_MODE_STEP] = CMD_TYPE_NONE;
				}
			} else {
				myGroup->phase = P2;
			}
			break;
		case P2:
			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
			SendMsg.msg = MSG_MODULE_JIGM_TRY_DISCONTACT;
			SendMsg.val[0] = group;
			send_msg(MODULE_TO_JIGM, (char *)&SendMsg);
			myGroup->phase = P3;
			break;
		case P3:
			if(myGroup->cmd[CMD_MODE_USER] == CMD_TYPE_CONTINUE) {
				myGroup->cmd[CMD_MODE_USER] = CMD_TYPE_NONE;

				memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
				SendMsg.msg = MSG_MODULE_JIGM_TRY_CONTACT;
				SendMsg.val[0] = group;
				send_msg(MODULE_TO_JIGM, (char *)&SendMsg);

				monitor_ch = (int)myData->COB_Client[group].config
					.ChArray2[myGroup->misc.scan_ch[0]];
				Select_OutPoint(2, monitor_ch+1, O_RUN_RELAY, ON);
				monitor_ch = (int)myData->COB_Client[group].config
					.ChArray2[myGroup->misc.scan_ch[1]];
				Select_OutPoint(3, monitor_ch+1, O_RUN_RELAY, ON);

				myGroup->code = G_CD_NONE;
				myGroup->state = G_RUN;
				myGroup->phase = P2;
			} else if(myGroup->cmd[CMD_MODE_USER] == CMD_TYPE_STOP) {
				myGroup->cmd[CMD_MODE_USER] = CMD_TYPE_NONE;

				myGroup->phase = P4;
			} else {
				myGroup->cmd[CMD_MODE_USER] = CMD_TYPE_NONE;
				myGroup->cmd[CMD_MODE_STEP] = CMD_TYPE_NONE;
			}

			if(myGroup->signal[G_SIG_JIG_FAIL] == P1) {
				myGroup->signal[G_SIG_JIG_FAIL] = P2;
			}
			break;
		case P4:
			for(i=ch; i < chInGroup; i++) {
				if(myData->ChAttribute[0].opType == 0) {
				} else {
					if((i % 2) != 0) continue;
				}

				if(myData->cData[i].op.state == C_PAUSE) {
					myData->cData[i].signal[C_SIG_CMD_STOP] = P1;
				}
			}

			myGroup->cmd[CMD_MODE_USER] = CMD_TYPE_NONE;
			myGroup->state = G_STANDBY;
			myGroup->phase = P0;
			break;
		case P10:
			if(myGroup->cmd[CMD_MODE_USER] == CMD_TYPE_CONTINUE) {
				myGroup->cmd[CMD_MODE_USER] = CMD_TYPE_NONE;

				memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
				SendMsg.msg = MSG_MODULE_JIGM_TRY_CONTACT;
				SendMsg.val[0] = group;
				send_msg(MODULE_TO_JIGM, (char *)&SendMsg);

				myGroup->signal[G_SIG_SAFETY_RESISTOR] = SR_CLOSE;
				myGroup->code = G_CD_NONE;
				myGroup->phase = P11;
			} else if(myGroup->cmd[CMD_MODE_STEP] == CMD_TYPE_CONTINUE) {
				myGroup->cmd[CMD_MODE_STEP] = CMD_TYPE_NONE;

				memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
				SendMsg.msg = MSG_MODULE_JIGM_TRY_CONTACT;
				SendMsg.val[0] = group;
				send_msg(MODULE_TO_JIGM, (char *)&SendMsg);

				myGroup->signal[G_SIG_SAFETY_RESISTOR] = SR_CLOSE;
				myGroup->code = G_CD_NONE;
				myGroup->state = G_RUN;
				myGroup->phase = P2;
			} else if(myGroup->cmd[CMD_MODE_USER] == CMD_TYPE_STOP) {
				myGroup->cmd[CMD_MODE_USER] = CMD_TYPE_NONE;
				
				memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
				SendMsg.msg = MSG_MODULE_JIGM_TRY_DISCONTACT;
				SendMsg.val[0] = group;
				send_msg(MODULE_TO_JIGM, (char *)&SendMsg);
				myGroup->phase = P4;
			} else {
				myGroup->cmd[CMD_MODE_USER] = CMD_TYPE_NONE;
				myGroup->cmd[CMD_MODE_STEP] = CMD_TYPE_NONE;
			}
			break;
		case P11:
			myGroup->misc.scan_ch[0] = 0;
			myGroup->misc.scan_ch[1] = 0;
			myGroup->misc.scan_step[0] = 0;
			myGroup->misc.scan_step[1] = 0;
			if(myData->AppControl.config.debugType == 0) {
				monitor_ch = (int)myData->COB_Client[group].config
					.ChArray2[myGroup->misc.scan_ch[0]];
				Select_OutPoint(2, monitor_ch+1, O_RUN_RELAY, ON);
				monitor_ch = (int)myData->COB_Client[group].config
					.ChArray2[myGroup->misc.scan_ch[1]];
				Select_OutPoint(3, monitor_ch+1, O_RUN_RELAY, ON);
			}

			myGroup->state = G_RUN;
			myGroup->phase = P2;
			break;
		default: break;
	}*/
}

void gSignalCheck(int group)
{
	if(myData->AppControl.misc.Load_Process[PROCESS_GROUP_COB][0] == P1) {
#ifdef __COB__
		switch(myData->AppControl.config.systemModel) {
			case F_SDI_5V_50A_5A:
			case F_SDI_5V_400A_200A_100A_10A:
			case F_SDI_5V_400A_200A_100A_10A_2:
			case F_SDI_5V_450A_200A_100A_10A:
			case F_PNE_5V_15A_30AP_SW:
				gSignalCheck_F(group);
				break;
			default:
				if(myGroup->signal[G_SIG_EXIT_PHASE] == P1) {
					myGroup->signal[G_SIG_EXIT_PHASE] = P0;
				}
				break;
		}
#endif
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
	int toPs, ch, cnt, check_flag;
	S_MSG_VAL SendMsg;

	toPs = MODULE_TO_COB1 + group;
	
	if(myGroup->signal[G_SIG_EXIT_PHASE] == P1) {
		switch(myGroup->code) {
			case M_CD_NONE:
			case M_CD_FAULT_FAN:
			case M_CD_FAULT_LOAD_LINE:
			case M_CD_FAULT_CALI_METER_COMM_ERROR:
			case M_CD_FAULT_CALIBRATOR_COMM_ERROR:
				check_flag = 0;
				myGroup->signal[G_SIG_EXIT_PHASE] = P0;
				break;
			default:
				check_flag = 1;
				myGroup->signal[G_SIG_EXIT_PHASE] = P2;
				break;
		}

		if(check_flag == 1) {
			if(myData->AppControl.config.systemModel
				== F_SDI_5V_400A_200A_100A_10A_2) {
				for(ch=0; ch < myPs->config.chInGroup[group]; ch++) {
					if(myData->cData[ch].op.state == C_RUN) {
						myData->cData[ch].signal[C_SIG_MODULE_FAULT] = P2;
					}
				}
			} else {
				for(ch=myGroup->misc.chOffset;
					ch < (myGroup->misc.chOffset
					+ myPs->config.chInGroup[group]); ch++) {
					if(myData->cData[ch].op.state == C_RUN) {
						myData->cData[ch].signal[C_SIG_MODULE_FAULT] = P2;
					}
				}
			}
		}

		memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
		SendMsg.msg = MSG_MODULE_COB_TROUBLE_CODE;
		SendMsg.val[0] = (int)myGroup->code;
		SendMsg.val[2] = (int)myGroup->signal[G_SIG_EXIT_VALUE];
		send_msg(toPs, (char *)&SendMsg);
	} else if(myGroup->signal[G_SIG_EXIT_PHASE] == P2) {
		cnt = 0;
		if(myData->AppControl.config.systemModel
			== F_SDI_5V_400A_200A_100A_10A_2) {
			for(ch=0; ch < myPs->config.chInGroup[group]; ch++) {
				if(myData->cData[ch].op.state != C_RUN) cnt++;
			}
		} else {
			for(ch=myGroup->misc.chOffset; ch < (myGroup->misc.chOffset
				+ myPs->config.chInGroup[group]); ch++) {
				if(myData->cData[ch].op.state != C_RUN) cnt++;
			}
		}
		if(cnt >= myPs->config.chInGroup[group]) {
			myGroup->signal[G_SIG_EXIT_PHASE] = P0;
		}
	}

	if(myGroup->signal[G_SIG_SAFETY_RESISTOR] == SR_OPEN) {
		Select_OutPoint(1, 0, O_CABLE_CHECK_SR, ON);
		Select_OutPoint(2, 0, O_CABLE_CHECK_SR, ON);
	} else { //SR_CLOSE
		Select_OutPoint(1, 0, O_CABLE_CHECK_SR, OFF);
		Select_OutPoint(2, 0, O_CABLE_CHECK_SR, OFF);
	}
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
	unsigned char stepType;
	int ch, ch_start, normal_count, code, stepNo, ch_offset, idx, new_code;
	long min_value, max_value;
	double total_value, avg_value;

	stepType = myData->gData[group].stepType;
	if((stepType == STEP_CHARGE || stepType == STEP_DISCHARGE
		|| stepType == STEP_OCV)
		&& myData->gData[group].attribute == ATTR_IDLE) {
		normal_count = 0;
		total_value = 0.0;
	} else return;

	stepNo = myData->gData[group].stepNo;
	ch_offset = myGroup->misc.chOffset;
	if(stepType == STEP_OCV) {
		idx = IDX_LOC_OBJ_FAULT_VOLTAGE_AVG_MIN;
		min_value = myData->testCond[ch_offset].local_object[stepNo][idx];
		idx = IDX_LOC_OBJ_FAULT_VOLTAGE_AVG_MAX;
		max_value = myData->testCond[ch_offset].local_object[stepNo][idx];
	} else {
		idx = IDX_LOC_OBJ_FAULT_AMPARE_HOUR_AVG_MIN;
		min_value = myData->testCond[ch_offset].local_object[stepNo][idx];
		idx = IDX_LOC_OBJ_FAULT_AMPARE_HOUR_AVG_MAX;
		max_value = myData->testCond[ch_offset].local_object[stepNo][idx];
	}
	if(min_value == 0 && max_value == 0) return;

	ch_start = myPs->config.chPerBd * group;
	for(ch=ch_start; ch < ch_start + myPs->config.chInGroup[group]; ch++) {
		code = myData->cData[ch].opSave.code;
		if(code == C_CD_NONE
			|| (code >= C_CD_END_START && code <= C_CD_END_END)) {
			normal_count++;
			if(stepType == STEP_OCV) {
				total_value += (double)myData->cData[ch].opSave.Vsens;
			} else {
				total_value
					+= (double)(myData->cData[ch].opSave.charge_AmpareHour
						+ myData->cData[ch].opSave.discharge_AmpareHour);
			}
		}
	}
	if(normal_count == 0) return;

	avg_value = total_value / (double)normal_count;
	new_code = C_CD_NONE;
	if(min_value != 0 && avg_value <= min_value) {
		if(stepType == STEP_OCV) {
			new_code = C_CD_FAULT_VOLTAGE_AVG_MIN;
		} else {
			new_code = C_CD_FAULT_AMPARE_HOUR_AVG_MIN;
		}
	}
	if(max_value != 0 && avg_value >= max_value) {
		if(stepType == STEP_OCV) {
			new_code = C_CD_FAULT_VOLTAGE_AVG_MAX;
		} else {
			new_code = C_CD_FAULT_AMPARE_HOUR_AVG_MAX;
		}
	}
	if(new_code == C_CD_NONE) return;

	for(ch=ch_start; ch < ch_start + myPs->config.chInGroup[group]; ch++) {
		code = myData->cData[ch].opSave.code;
		if(code == C_CD_NONE
			|| (code >= C_CD_END_START && code <= C_CD_END_END)) {
			myData->cData[ch].opSave.code = new_code;
			myData->cData[ch].op.code = new_code;
			myData->cData[ch].op.state = C_PAUSE;
			myData->cData[ch].op.phase = P102;
		}
	}
#endif
}

int gCompare_Ch_Average_2(int group)
{ //debug_size_cob
/*	int ch, ch_start, normal_count, code, stepNo, ch_offset, idx, new_code;
	long min_value, max_value;
	double total_value, avg_value;

	int scan_step_start, scan_step_end, scan_step;
	int ocv_step_count, ocv_scan_step;

	ch_offset = myGroup->misc.chOffset;

	scan_step_start = (int)myGroup->misc.tmp_scan_step[0];
	scan_step_end = (int)myGroup->misc.scan_step[0];
	ocv_step_count = 0;
	ocv_scan_step = 0;
	min_value = max_value = 0;
	for(scan_step=scan_step_start; scan_step < (scan_step_end+1); scan_step++) {
		if(myData->COB_opSave[group][0][scan_step].stepType == STEP_OCV) {
			ocv_step_count++;
			ocv_scan_step = scan_step;
			stepNo = myData->COB_opSave[group][0][scan_step].idxStepNo;
			idx = IDX_LOC_OBJ_FAULT_VOLTAGE_AVG_MIN;
			min_value = myData->testCond[ch_offset].local_object[stepNo][idx];
			idx = IDX_LOC_OBJ_FAULT_VOLTAGE_AVG_MAX;
			max_value = myData->testCond[ch_offset].local_object[stepNo][idx];
		}
	}
	if(ocv_step_count == 0) return 0;
	if(min_value == 0 && max_value == 0) return 0;

	normal_count = 0;
	total_value = 0.0;
	ch_start = 0; //myPs->config.chPerBd * group;
	if(myData->cData[ch_start].op.stepType == STEP_LONG_TIME_REST) {
		scan_step_end--;
	}

	for(ch=ch_start; ch < ch_start + myPs->config.chInGroup[group]; ch++) {
		code = myData->COB_opSave[group][ch][scan_step_end].code;
		if(code == C_CD_NONE
			|| (code >= C_CD_END_START && code <= C_CD_END_END)) {
			normal_count++;
			total_value
				+= (double)myData->COB_opSave[group][ch][ocv_scan_step].Vsens;
		}
	}
	if(normal_count == 0) return 0;

	avg_value = total_value / (double)normal_count;
	new_code = C_CD_NONE;
	if(min_value != 0 && avg_value <= min_value) {
		new_code = C_CD_FAULT_VOLTAGE_AVG_MIN;
	}
	if(max_value != 0 && avg_value >= max_value) {
		new_code = C_CD_FAULT_VOLTAGE_AVG_MAX;
	}
	if(new_code == C_CD_NONE) return 0;

	for(ch=ch_start; ch < ch_start + myPs->config.chInGroup[group]; ch++) {
		code = myData->COB_opSave[group][ch][scan_step_end].code;
		if(code == C_CD_NONE
			|| (code >= C_CD_END_START && code <= C_CD_END_END)) {
			myData->COB_opSave[group][ch][scan_step_end].code = new_code;
		}
	}

	if(myData->cData[ch_start].op.stepType == STEP_LONG_TIME_REST) {
		myGroup->misc.scan_step[0]--;
	}

	myData->cData[ch_start].op.state = C_STANDBY;
	myData->cData[ch_start].op.phase = P0;
*/
	return 1;
}

void gCellCheck_Compare(int group)
{ //debug_size_cob
/*	unsigned char result;
	int chInGroup, i, j, monitor_ch, code;

	chInGroup = myData->mData.config.chInGroup[group];

	myGroup->misc.cell_check_count++;

	result = 0;
	for(i=0; i < chInGroup; i++) {
		if(myData->ChAttribute[0].opType == 0) {
		} else {
			if((i % 2) != 0) continue;
		}

		j = group * chInGroup + i;
		monitor_ch = j / 2; //myData->COB_Client[group].config.ChArray2[i];

		code = myData->cData[j].op.code;

		if(myData->COB_Client[group].misc.nonCell[monitor_ch] == 0
			&& (code >= C_CD_END_START && code <= C_CD_END_END)) { //step run
		} else if(myData->COB_Client[group].misc.nonCell[monitor_ch] == 1
			&& (code < C_CD_END_START || code > C_CD_END_END)) { //fault
		} else if(myData->COB_Client[group].misc.nonCell[monitor_ch] == 0
			&& (code < C_CD_END_START || code > C_CD_END_END)) { //fault ERROR_NO
			result++;
		} else {
			//fault ERROR_YES
			result++;
		}
	}

	myGroup->misc.cell_check_result = result; //0:OK, x:NG
*/
}

int gLoadLineCheck(int group, int div)
{
	int rtn=0;
#ifdef __COB__
	unsigned char tmp;
	int i, j, k;
	double sum;

	//0. sr_open
	//x. ocv1 save(daq Vsens)
	//x. sr_close
	//x. delay(100ms)
	//x. ocv2 save(daq Vsens)
	//x. sr_open
	//1. jig contact
	//3. ocv3 save(daq Vsens)
	//4. sr close
	//5. delay(100ms)
	//6. ocv4 save(daq Vsens)
	//7. line path check
	//8. line_check calculation : abs(ocv3-ocv4) <= 5mV, path_val == 1
	//9. cell check(normal Vsens) : only line_check ok ch
	//10. line path + cell check result report
	//11. sr close(default)
	//12. process start
	
	if(div == 1) {
		for(i=0; i < 2; i++) {
			for(j=0; j < 12; j++) {
				Select_OutPoint(i+1, j+1, O_CABLE_CHECK_CH, OFF);
			}
			Select_OutPoint(i+1, 0, O_CABLE_CHECK_SR, ON);
			Select_OutPoint(i+1, 0, O_CABLE_CHECK_UPPER, OFF);
			Select_OutPoint(i+1, 0, O_CABLE_CHECK_LOWER, OFF);
		}
		return rtn;
	}

	switch(myGroup->signal[G_SIG_LOAD_LINE_CHECK_PHASE]) {
		case P0:
			myGroup->signal[G_SIG_LOAD_LINE_CHECK_DELAY]++;
			//10ms * 30 = 300ms
			j = (int)myGroup->signal[G_SIG_LOAD_LINE_CHECK_DELAY];
			if(j >= 10 && j < 20) {
				for(i=0; i < 12; i++) {
					myData->lineCheck[group][i].val[2][j-10]
						= myData->SubSensV.ch[i].sensV;
				}
				for(i=12; i < 24; i++) {
					myData->lineCheck[group][i].val[2][j-10]
						= myData->SubSensV.ch[i+20].sensV;
				}
			} else if(j == 20) {
				for(i=0; i < 12; i++) {
					sum = 0.0;
					for(k=0; k < 10; k++) {
						sum += (double)myData->lineCheck[group][i].val[2][k];
					}
					sum /= 10.0;
					myData->lineCheck[group][i].avg_val[2] = (long)sum;
				}
				for(i=12; i < 24; i++) {
					sum = 0.0;
					for(k=0; k < 10; k++) {
						sum += (double)myData->lineCheck[group][i].val[2][k];
					}
					sum /= 10.0;
					myData->lineCheck[group][i].avg_val[2] = (long)sum;
				}
			}
			if(myGroup->signal[G_SIG_LOAD_LINE_CHECK_DELAY] < 30) break;

			//ocv3 save(daq Vsens)
/*			for(i=0; i < 12; i++) {
				myData->lineCheck[group][i].val[2]
					= myData->SubSensV.ch[i].sensV;
			}
			for(i=12; i < 24; i++) {
				myData->lineCheck[group][i].val[2]
					= myData->SubSensV.ch[i+20].sensV;
			}*/
			myGroup->signal[G_SIG_SAFETY_RESISTOR] = SR_CLOSE;
			myGroup->signal[G_SIG_LOAD_LINE_CHECK_DELAY] = 0;
			myGroup->signal[G_SIG_LOAD_LINE_CHECK_PHASE]++;
			break;
		case P1:
			myGroup->signal[G_SIG_LOAD_LINE_CHECK_DELAY]++;
			//10ms * 30 = 300ms
			j = (int)myGroup->signal[G_SIG_LOAD_LINE_CHECK_DELAY];
			if(j >= 10 && j < 20) {
				for(i=0; i < 12; i++) {
					myData->lineCheck[group][i].val[3][j-10]
						= myData->SubSensV.ch[i].sensV;
				}
				for(i=12; i < 24; i++) {
					myData->lineCheck[group][i].val[3][j-10]
						= myData->SubSensV.ch[i+20].sensV;
				}
			} else if(j == 20) {
				for(i=0; i < 12; i++) {
					sum = 0.0;
					for(k=0; k < 10; k++) {
						sum += (double)myData->lineCheck[group][i].val[3][k];
					}
					sum /= 10.0;
					myData->lineCheck[group][i].avg_val[3] = (long)sum;
				}
				for(i=12; i < 24; i++) {
					sum = 0.0;
					for(k=0; k < 10; k++) {
						sum += (double)myData->lineCheck[group][i].val[3][k];
					}
					sum /= 10.0;
					myData->lineCheck[group][i].avg_val[3] = (long)sum;
				}
			}
			if(myGroup->signal[G_SIG_LOAD_LINE_CHECK_DELAY] < 30) break;

			//ocv4 save(daq Vsens)
/*			for(i=0; i < 12; i++) {
				myData->lineCheck[group][i].val[3]
					= myData->SubSensV.ch[i].sensV;
			}
			for(i=12; i < 24; i++) {
				myData->lineCheck[group][i].val[3]
					= myData->SubSensV.ch[i+20].sensV;
			}*/
			myGroup->signal[G_SIG_LOAD_LINE_CHECK_CH] = P0;
			myGroup->signal[G_SIG_LOAD_LINE_CHECK_DELAY] = 0;
			myGroup->signal[G_SIG_LOAD_LINE_CHECK_PHASE]++;
			break;
		case P2:
			i = (int)myGroup->signal[G_SIG_LOAD_LINE_CHECK_CH];
			if(i < 12) {
				Select_OutPoint(1, i+1, O_CABLE_CHECK_CH, ON);
				Select_OutPoint(1, 0, O_CABLE_CHECK_UPPER, ON);
				Select_OutPoint(1, 0, O_CABLE_CHECK_LOWER, OFF);
			} else {
				i -= 12;
				Select_OutPoint(2, i+1, O_CABLE_CHECK_CH, ON);
				Select_OutPoint(2, 0, O_CABLE_CHECK_UPPER, ON);
				Select_OutPoint(2, 0, O_CABLE_CHECK_LOWER, OFF);
			}
			myGroup->signal[G_SIG_LOAD_LINE_CHECK_DELAY] = 0;
			myGroup->signal[G_SIG_LOAD_LINE_CHECK_PHASE]++;
			break;
		case P3:
			myGroup->signal[G_SIG_LOAD_LINE_CHECK_DELAY]++;
			//10ms * 5 = 50ms
			if(myGroup->signal[G_SIG_LOAD_LINE_CHECK_DELAY] < 5) break;

			i = (int)myGroup->signal[G_SIG_LOAD_LINE_CHECK_CH];
			if(i < 12) {
				tmp = Read_InPoint(1, 0, I_CABLE_FAIL); //0:ng, 1:ok
				myData->lineCheck[group][i].path_val_p = tmp;
				Select_OutPoint(1, i+1, O_CABLE_CHECK_CH, ON);
				Select_OutPoint(1, 0, O_CABLE_CHECK_UPPER, OFF);
				Select_OutPoint(1, 0, O_CABLE_CHECK_LOWER, OFF);
			} else {
				tmp = Read_InPoint(2, 0, I_CABLE_FAIL); //0:ng, 1:ok
				myData->lineCheck[group][i].path_val_p = tmp;
				i -= 12;
				Select_OutPoint(2, i+1, O_CABLE_CHECK_CH, ON);
				Select_OutPoint(2, 0, O_CABLE_CHECK_UPPER, OFF);
				Select_OutPoint(2, 0, O_CABLE_CHECK_LOWER, OFF);
			}
			myGroup->signal[G_SIG_LOAD_LINE_CHECK_DELAY] = 0;
			myGroup->signal[G_SIG_LOAD_LINE_CHECK_PHASE]++;
			break;
		case P4:
			myGroup->signal[G_SIG_LOAD_LINE_CHECK_DELAY]++;
			//10ms * 2 = 20ms
			if(myGroup->signal[G_SIG_LOAD_LINE_CHECK_DELAY] < 2) break;

			i = (int)myGroup->signal[G_SIG_LOAD_LINE_CHECK_CH];
			if(i < 12) {
				Select_OutPoint(1, i+1, O_CABLE_CHECK_CH, ON);
				Select_OutPoint(1, 0, O_CABLE_CHECK_UPPER, OFF);
				Select_OutPoint(1, 0, O_CABLE_CHECK_LOWER, ON);
			} else {
				i -= 12;
				Select_OutPoint(2, i+1, O_CABLE_CHECK_CH, ON);
				Select_OutPoint(2, 0, O_CABLE_CHECK_UPPER, OFF);
				Select_OutPoint(2, 0, O_CABLE_CHECK_LOWER, ON);
			}
			myGroup->signal[G_SIG_LOAD_LINE_CHECK_DELAY] = 0;
			myGroup->signal[G_SIG_LOAD_LINE_CHECK_PHASE]++;
			break;
		case P5:
			myGroup->signal[G_SIG_LOAD_LINE_CHECK_DELAY]++;
			//10ms * 5 = 50ms
			if(myGroup->signal[G_SIG_LOAD_LINE_CHECK_DELAY] < 5) break;

			i = (int)myGroup->signal[G_SIG_LOAD_LINE_CHECK_CH];
			if(i < 12) {
				tmp = Read_InPoint(1, 0, I_CABLE_FAIL); //0:ng, 1:ok
				myData->lineCheck[group][i].path_val_n = tmp;
				Select_OutPoint(1, i+1, O_CABLE_CHECK_CH, OFF);
				Select_OutPoint(1, 0, O_CABLE_CHECK_UPPER, OFF);
				Select_OutPoint(1, 0, O_CABLE_CHECK_LOWER, OFF);
			} else {
				tmp = Read_InPoint(2, 0, I_CABLE_FAIL); //0:ng, 1:ok
				myData->lineCheck[group][i].path_val_n = tmp;
				i -= 12;
				Select_OutPoint(2, i+1, O_CABLE_CHECK_CH, OFF);
				Select_OutPoint(2, 0, O_CABLE_CHECK_UPPER, OFF);
				Select_OutPoint(2, 0, O_CABLE_CHECK_LOWER, OFF);
			}
			myGroup->signal[G_SIG_LOAD_LINE_CHECK_DELAY] = 0;
			myGroup->signal[G_SIG_LOAD_LINE_CHECK_PHASE]++;
			break;
		case P6:
			myGroup->signal[G_SIG_LOAD_LINE_CHECK_DELAY]++;
			//10ms * 2 = 20ms
			if(myGroup->signal[G_SIG_LOAD_LINE_CHECK_DELAY] < 2) break;

			if(myGroup->signal[G_SIG_LOAD_LINE_CHECK_CH] == P23) {
				myGroup->signal[G_SIG_LOAD_LINE_CHECK_DELAY] = 0;
				myGroup->signal[G_SIG_LOAD_LINE_CHECK_PHASE]++;
			} else {
				myGroup->signal[G_SIG_LOAD_LINE_CHECK_CH]++;
				myGroup->signal[G_SIG_LOAD_LINE_CHECK_DELAY] = 0;
				myGroup->signal[G_SIG_LOAD_LINE_CHECK_PHASE] = P2;
			}
			break;
		case P7:
			myGroup->signal[G_SIG_LOAD_LINE_CHECK_PHASE]++;
			rtn = 1;
			break;
		case P8:
			rtn = 1;
			break;
		default:
			break;
	}
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
	int group, type, range, point, bd, ch, diff;
	long val1, val2;
	double d_val1=0, d_val2=0;
	S_MSG_VAL SendMsg;

	group = (int)myPs->signal[M_SIG_CALI_GROUP];
	bd = (int)myPs->signal[M_SIG_CALI_BD_START];
	type = (int)myPs->signal[M_SIG_CALI_VI_SELECT];

	if(type == 0) {
		range = 0;
	} else {
		range = (int)myPs->signal[M_SIG_CALI_RANGE_I] - 1;
	}

	switch(myGroup->phase) {
		case P30: //MainDAC_Calibration
			if(type == P0) {
				myGroup->cali_main_dac.tmp_cali_pointNum[type][range] = 2;
				myGroup->cali_main_dac.tmp_cali_cmd[type][range][0] = 2700000;
				myGroup->cali_main_dac.tmp_cali_cmd[type][range][1] = 4200000;
			} else {
				myGroup->cali_main_dac.tmp_cali_pointNum[type][range] = 4;
				val1 = myPs->config.maxI[range];
				val2 = myPs->config.minI[range];
				if(myPs->signal[M_SIG_CALI_PARALLEL] == P1) {
					val1 *= 2;
					val2 *= 2;
				}
				myGroup->cali_main_dac.tmp_cali_cmd[type][range][0] = val2;
				myGroup->cali_main_dac.tmp_cali_cmd[type][range][1] = val2 / 4;
				myGroup->cali_main_dac.tmp_cali_cmd[type][range][2] = val1 / 4;
				myGroup->cali_main_dac.tmp_cali_cmd[type][range][3] = val1;
				myGroup->cali_main_dac.tmp_DA_A[type][range][0] = 1.0;
				myGroup->cali_main_dac.tmp_DA_B[type][range][0] = 0.0;
				myGroup->cali_main_dac.tmp_DA_A[type][range][1] = 1.0;
				myGroup->cali_main_dac.tmp_DA_B[type][range][1] = 0.0;
			}

			myPs->misc.cali_delay = myPs->misc.timer_1sec;
			myGroup->phase++;
			break;
		case P31:
			point = 0;
			d_val1 = (double)myGroup->cali_main_dac
				.tmp_cali_cmd[type][range][point];
			myData->bData[bd].misc.refI = d_val1;
			if(d_val1 >= 0) {
				myData->bData[bd].signal[B_SIG_CD_SELECT] = P1;
			} else {
				myData->bData[bd].signal[B_SIG_CD_SELECT] = P2;
			}
			ch = myPs->config.chPerBd * bd;
			myData->cData[ch].signal[C_SIG_RUN_RELAY_ON] = P1;
			myData->cData[ch].signal[C_SIG_RANGE_RELAY_ON] = P1;

			myPs->misc.cali_delay = myPs->misc.timer_1sec;
			myGroup->phase++;
			break;
		case P32:
			diff = myPs->misc.timer_1sec - myPs->misc.cali_delay;
			if(diff < 2) break;
			myPs->misc.cali_delay = myPs->misc.timer_1sec;

			point = 0;
			ch = myPs->config.chPerBd * bd;
			myData->cData[ch].signal[C_SIG_AUX_DAI] = P20;
			d_val1 = (double)myGroup->cali_main_dac
				.tmp_cali_cmd[type][range][point];

			if(type == P0) {
				gCalCmdV(bd, d_val1, 15, range);
				d_val2 = 5000000;
				gCalCmdI(bd, d_val2, 15, range);
			} else {
				if(d_val1 < 0) {
					d_val2 = 0;
				} else {
					d_val2 = 5000000;
				}
				gCalCmdV(bd, d_val2, 15, range);
				gCalCmdI(bd, d_val1, 15, range);
			}
			myGroup->phase++;
			break;
		case P33:
			diff = myPs->misc.timer_1sec - myPs->misc.cali_delay;
			if(diff < 2) break;
			myPs->misc.cali_delay = myPs->misc.timer_1sec;

			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
			SendMsg.msg = MSG_MODULE_METER_REQUEST;
			SendMsg.val[0] = group;
			SendMsg.val[1] = type;
			send_msg(MODULE_TO_METER, (char *)&SendMsg);
			myGroup->phase++;
			break;
		case P34: //wait meter1 response
			diff = myPs->misc.timer_1sec - myPs->misc.cali_delay;
			if(diff < 2) break;
			myPs->misc.cali_delay = myPs->misc.timer_1sec;
			point = 0;
			myGroup->cali_main_dac.tmp_org_meter[type][range][point]
				= myPs->misc.meter_value * 100;
			myGroup->phase++;
			break;
		case P35:
			point = 1;
			d_val1 = (double)myGroup->cali_main_dac
				.tmp_cali_cmd[type][range][point];
			myData->bData[bd].misc.refI = d_val1;
			if(d_val1 >= 0) {
				myData->bData[bd].signal[B_SIG_CD_SELECT] = P1;
			} else {
				myData->bData[bd].signal[B_SIG_CD_SELECT] = P2;
			}
			ch = myPs->config.chPerBd * bd;
			myData->cData[ch].signal[C_SIG_RUN_RELAY_ON] = P1;
			myData->cData[ch].signal[C_SIG_RANGE_RELAY_ON] = P1;
			myGroup->phase++;
			break;
		case P36:
			point = 1;
			ch = myPs->config.chPerBd * bd;
			myData->cData[ch].signal[C_SIG_AUX_DAI] = P20;
			d_val1 = (double)myGroup->cali_main_dac
				.tmp_cali_cmd[type][range][point];
			if(type == P0) {
				gCalCmdV(bd, d_val1, 15, range);
				d_val2 = 5000000;
				gCalCmdI(bd, d_val2, 15, range);
			} else {
				if(d_val1 < 0) {
					d_val2 = 0;
				} else {
					d_val2 = 5000000;
				}
				gCalCmdV(bd, d_val2, 15, range);
				gCalCmdI(bd, d_val1, 15, range);
			}
			myPs->misc.cali_delay = myPs->misc.timer_1sec;
			myGroup->phase++;
			break;
		case P37:
			diff = myPs->misc.timer_1sec - myPs->misc.cali_delay;
			if(diff < 2) break;
			myPs->misc.cali_delay = myPs->misc.timer_1sec;

			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
			SendMsg.msg = MSG_MODULE_METER_REQUEST;
			SendMsg.val[0] = group;
			SendMsg.val[1] = type;
			send_msg(MODULE_TO_METER, (char *)&SendMsg);
			myGroup->phase++;
			break;
		case P38: //wait meter1 response
			diff = myPs->misc.timer_1sec - myPs->misc.cali_delay;
			if(diff < 2) break;
			myPs->misc.cali_delay = myPs->misc.timer_1sec;

			point = 1;
			myGroup->cali_main_dac.tmp_org_meter[type][range][point]
				= myPs->misc.meter_value * 100;
			myGroup->phase++;
			break;
		case P39:
			point = 2;
			d_val1 = (double)myGroup->cali_main_dac
				.tmp_cali_cmd[type][range][point];
			myData->bData[bd].misc.refI = d_val1;
			if(d_val1 >= 0) {
				myData->bData[bd].signal[B_SIG_CD_SELECT] = P1;
			} else {
				myData->bData[bd].signal[B_SIG_CD_SELECT] = P2;
			}
			ch = myPs->config.chPerBd * bd;
			myData->cData[ch].signal[C_SIG_RUN_RELAY_ON] = P1;
			myData->cData[ch].signal[C_SIG_RANGE_RELAY_ON] = P1;
			myGroup->phase++;
			break;
		case P40:
			point = 2;
			ch = myPs->config.chPerBd * bd;
			myData->cData[ch].signal[C_SIG_AUX_DAI] = P20;
			d_val1 = (double)myGroup->cali_main_dac
				.tmp_cali_cmd[type][range][point];
			if(type == P0) {
				gCalCmdV(bd, d_val1, 15, range);
				d_val2 = 5000000;
				gCalCmdI(bd, d_val2, 15, range);
			} else {
				if(d_val1 < 0) {
					d_val2 = 0;
				} else {
					d_val2 = 5000000;
				}
				gCalCmdV(bd, d_val2, 15, range);
				gCalCmdI(bd, d_val1, 15, range);
			}
			myPs->misc.cali_delay = myPs->misc.timer_1sec;
			myGroup->phase++;
			break;
		case P41:
			diff = myPs->misc.timer_1sec - myPs->misc.cali_delay;
			if(diff < 2) break;
			myPs->misc.cali_delay = myPs->misc.timer_1sec;

			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
			SendMsg.msg = MSG_MODULE_METER_REQUEST;
			SendMsg.val[0] = group;
			SendMsg.val[1] = type;
			send_msg(MODULE_TO_METER, (char *)&SendMsg);
			myGroup->phase++;
			break;
		case P42: //wait meter1 response
			diff = myPs->misc.timer_1sec - myPs->misc.cali_delay;
			if(diff < 2) break;
			myPs->misc.cali_delay = myPs->misc.timer_1sec;

			point = 2;
			myGroup->cali_main_dac.tmp_org_meter[type][range][point]
				= myPs->misc.meter_value * 100;
			myGroup->phase++;
		case P43:
			point = 3;
			d_val1 = (double)myGroup->cali_main_dac
				.tmp_cali_cmd[type][range][point];
			if(d_val1 >= 0) {
				myData->bData[bd].signal[B_SIG_CD_SELECT] = P1;
			} else {
				myData->bData[bd].signal[B_SIG_CD_SELECT] = P2;
			}
			ch = myPs->config.chPerBd * bd;
			myData->cData[ch].signal[C_SIG_RUN_RELAY_ON] = P1;
			myData->cData[ch].signal[C_SIG_RANGE_RELAY_ON] = P1;
			myGroup->phase++;
			break;
		case P44:
			point = 3;
			ch = myPs->config.chPerBd * bd;
			myData->cData[ch].signal[C_SIG_AUX_DAI] = P20;
			d_val1 = (double)myGroup->cali_main_dac
				.tmp_cali_cmd[type][range][point];
			myData->bData[bd].misc.refI = d_val1;
			if(type == P0) {
				gCalCmdV(bd, d_val1, 15, range);
				d_val2 = 5000000;
				gCalCmdI(bd, d_val2, 15, range);
			} else {
				if(d_val1 < 0) {
					d_val2 = 0;
				} else {
					d_val2 = 5000000;
				}
				gCalCmdV(bd, d_val2, 15, range);
				gCalCmdI(bd, d_val1, 15, range);
			}
			myPs->misc.cali_delay = myPs->misc.timer_1sec;
			myGroup->phase++;
			break;
		case P45:
			diff = myPs->misc.timer_1sec - myPs->misc.cali_delay;
			if(diff < 2) break;
			myPs->misc.cali_delay = myPs->misc.timer_1sec;

			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
			SendMsg.msg = MSG_MODULE_METER_REQUEST;
			SendMsg.val[0] = group;
			SendMsg.val[1] = type;
			send_msg(MODULE_TO_METER, (char *)&SendMsg);
			myGroup->phase++;
			break;
		case P46: //wait meter1 response
			diff = myPs->misc.timer_1sec - myPs->misc.cali_delay;
			if(diff < 2) break;
			myPs->misc.cali_delay = myPs->misc.timer_1sec;

			point = 3;
			myGroup->cali_main_dac.tmp_org_meter[type][range][point]
				= myPs->misc.meter_value * 100;
			myGroup->phase++;
			break;
		case P47:
			d_val1 = myGroup->cali_main_dac.tmp_cali_cmd[type][range][1]
				- myGroup->cali_main_dac.tmp_cali_cmd[type][range][0];
			d_val2 = myGroup->cali_main_dac.tmp_org_meter[type][range][1]
				- myGroup->cali_main_dac.tmp_org_meter[type][range][0];
			myGroup->cali_main_dac.tmp_DA_A[type][range][0] = d_val1 / d_val2;
			myGroup->cali_main_dac.tmp_DA_B[type][range][0]
				= (double)myGroup->cali_main_dac.tmp_cali_cmd[type][range][0]
				- (double)myGroup->cali_main_dac.tmp_org_meter[type][range][0]
				* myGroup->cali_main_dac.tmp_DA_A[type][range][0];
			
			d_val1 = myGroup->cali_main_dac.tmp_cali_cmd[type][range][3]
				- myGroup->cali_main_dac.tmp_cali_cmd[type][range][2];
			d_val2 = myGroup->cali_main_dac.tmp_org_meter[type][range][3]
				- myGroup->cali_main_dac.tmp_org_meter[type][range][2];
			myGroup->cali_main_dac.tmp_DA_A[type][range][1] = d_val1 / d_val2;
			myGroup->cali_main_dac.tmp_DA_B[type][range][1]
				= (double)myGroup->cali_main_dac.tmp_cali_cmd[type][range][2]
				- (double)myGroup->cali_main_dac.tmp_org_meter[type][range][2]
				* myGroup->cali_main_dac.tmp_DA_A[type][range][1];
			myGroup->phase++;
			break;
		case P48:
			point = 0;
			d_val1 = (double)myGroup->cali_main_dac
				.tmp_cali_cmd[type][range][point];
			myData->bData[bd].misc.refI = d_val1;
			if(d_val1 >= 0) {
				myData->bData[bd].signal[B_SIG_CD_SELECT] = P1;
			} else {
				myData->bData[bd].signal[B_SIG_CD_SELECT] = P2;
			}
			ch = myPs->config.chPerBd * bd;
			myData->cData[ch].signal[C_SIG_RUN_RELAY_ON] = P1;
			myData->cData[ch].signal[C_SIG_RANGE_RELAY_ON] = P1;
			myGroup->phase++;
			break;
		case P49:
			point = 0;
			ch = myPs->config.chPerBd * bd;
			myData->cData[ch].signal[C_SIG_AUX_DAI] = P20;
			d_val1 = (double)myGroup->cali_main_dac
				.tmp_cali_cmd[type][range][point];
			if(type == P0) {
				gCalCmdV(bd, d_val1, 15, range);
				d_val2 = 5000000;
				gCalCmdI(bd, d_val2, 15, range);
			} else {
				if(d_val1 < 0) {
					d_val2 = 0;
				} else {
					d_val2 = 5000000;
				}
				gCalCmdV(bd, d_val2, 15, range);
				gCalCmdI(bd, d_val1, 15, range);
			}
			myPs->misc.cali_delay = myPs->misc.timer_1sec;
			myGroup->phase++;
			break;
		case P50:
			diff = myPs->misc.timer_1sec - myPs->misc.cali_delay;
			if(diff < 2) break;
			myPs->misc.cali_delay = myPs->misc.timer_1sec;

			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
			SendMsg.msg = MSG_MODULE_METER_REQUEST;
			SendMsg.val[0] = group;
			SendMsg.val[1] = type;
			send_msg(MODULE_TO_METER, (char *)&SendMsg);
			myGroup->phase++;
			break;
		case P51: //wait meter1 response
			diff = myPs->misc.timer_1sec - myPs->misc.cali_delay;
			if(diff < 2) break;
			myPs->misc.cali_delay = myPs->misc.timer_1sec;

			point = 0;
			myGroup->cali_main_dac.tmp_cal_meter[type][range][point]
				= myPs->misc.meter_value * 100;
			myGroup->phase++;
			break;
		case P52:
			point = 3;
			d_val1 = (double)myGroup->cali_main_dac
				.tmp_cali_cmd[type][range][point];
			myData->bData[bd].misc.refI = d_val1;
			if(d_val1 >= 0) {
				myData->bData[bd].signal[B_SIG_CD_SELECT] = P1;
			} else {
				myData->bData[bd].signal[B_SIG_CD_SELECT] = P2;
			}
			ch = myPs->config.chPerBd * bd;
			myData->cData[ch].signal[C_SIG_RUN_RELAY_ON] = P1;
			myData->cData[ch].signal[C_SIG_RANGE_RELAY_ON] = P1;
			myGroup->phase++;
			break;
		case P53:
			point = 3;
			ch = myPs->config.chPerBd * bd;
			myData->cData[ch].signal[C_SIG_AUX_DAI] = P20;
			d_val1 = (double)myGroup->cali_main_dac
				.tmp_cali_cmd[type][range][point];
			if(type == P0) {
				gCalCmdV(bd, d_val1, 5, range);
				d_val2 = 5000000;
				gCalCmdI(bd, d_val2, 15, range);
			} else {
				if(d_val1 < 0) {
					d_val2 = 0;
				} else {
					d_val2 = 5000000;
				}
				gCalCmdV(bd, d_val2, 15, range);
				gCalCmdI(bd, d_val1, 15, range);
			}
			myPs->misc.cali_delay = myPs->misc.timer_1sec;
			myGroup->phase++;
			break;
		case P54:
			diff = myPs->misc.timer_1sec - myPs->misc.cali_delay;
			if(diff < 2) break;
			myPs->misc.cali_delay = myPs->misc.timer_1sec;

			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
			SendMsg.msg = MSG_MODULE_METER_REQUEST;
			SendMsg.val[0] = group;
			SendMsg.val[1] = type;
			send_msg(MODULE_TO_METER, (char *)&SendMsg);
			myGroup->phase++;
			break;
		case P55: //wait meter1 response
			diff = myPs->misc.timer_1sec - myPs->misc.cali_delay;
			if(diff < 2) break;
			myPs->misc.cali_delay = myPs->misc.timer_1sec;
			point = 1;

			myGroup->cali_main_dac.tmp_cal_meter[type][range][point]
				= myPs->misc.meter_value * 100;

			ch = myPs->config.chPerBd * bd;
			myData->cData[ch].signal[C_SIG_RUN_RELAY_OFF] = P1;
			myData->cData[ch].signal[C_SIG_RANGE_RELAY_OFF] = P1;
			myGroup->phase++;
			break;
		case P56:
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
#endif
}

int	bFindDACaliPoint(int bd, long value, int type, int range)
{
	int point=0;
#ifdef __COB__
	int	point1, pointNum;
	long da;

	if(myPs->state == M_CALI) {
		pointNum = myGroup->cali_main_dac.tmp_cali_pointNum[type][range];
		for(point=0; point < pointNum; point++) {
			da = myGroup->cali_main_dac.tmp_cali_cmd[type][range][point];
			if(da >= value) {
				if(point <= 0) point = 0;
				else point -= 1;
			   	break;
			}
		}
		point1 = pointNum - 1;
		da = myGroup->cali_main_dac.tmp_cali_cmd[type][range][point1];
		if(da < value) {
			if(point1 <= 0)	point = 0;
			else point = point1-1;
		}
	} else {
		pointNum = myData->bData[bd].cali_main_dac.cali_pointNum[type][range];
		for(point=0; point < pointNum; point++) {
			da = myData->bData[bd].cali_main_dac.cali_cmd[type][range][point];
			if(da >= value) {
				if(point <= 0) point = 0;
				else point -= 1;
			   	break;
			}
		}
		point1 = pointNum - 1;
		da = myData->bData[bd].cali_main_dac.cali_cmd[type][range][point];
		if(da < value) {
			if(point1 <= 0)	point = 0;
			else point = point1-1;
		}
	}
#endif
	return point;
}

void gCalCmdV(int bd, long value, int div, int range)
{
#ifdef __COB__
    int point=0;
    double tmp=0;
	const int type=0;

	if(div > 0 && div <= 10) {
		point = bFindDACaliPoint(bd, value, type, range);
		tmp = (double)value
			* myGroup->cali_main_dac.tmp_DA_A[type][range][point]
			+ myGroup->cali_main_dac.tmp_DA_B[type][range][point];
	} else if(div > 10) {
		tmp = (double)value;
	} else {
		tmp = 0;
	}

	tmp *= 0.0032767;

	aSetMainDA_V(bd, tmp);
#endif
}

void gCalCmdI(int bd, long value, int div, int range)
{
#ifdef __COB__
    int point=0;
	double tmp=0, ratio=0;// shunt=0, digit;
	const int type=1;

	if(value >= 0) {
		point = 1;
	} else {
		point = 0;
	}

	tmp = value * myGroup->cali_main_dac.tmp_DA_A[type][range][point]
		+ myGroup->cali_main_dac.tmp_DA_B[type][range][point];

	if(value >= 0) {
		ratio = myPs->config.daRatioI_P[range];
		tmp = tmp * ratio + myPs->config.daOffsetI_P[range];
	} else {
		ratio = myPs->config.daRatioI_N[range];
		tmp = tmp * (-1) * ratio + myPs->config.daOffsetI_N[range];
	}

	tmp *= 0.0032767;

	aSetMainDA_I(bd, tmp);
#endif
}

void gCali_bd(void)
{ //debug_size_cob
/*	int bd_start, bd_end, bd, ch, channel, div; //, comPort;
   	int group=0, jig=0, range=0, type, diff=0;
	S_MSG_VAL SendMsg;

	bd_start = (int)myPs->signal[M_SIG_CALI_BD_START];
	bd_end = bd_start + (int)myPs->signal[M_SIG_CALI_BD_COUNT];
	if(bd_end > myPs->config.installedBd)  bd_end = myPs->config.installedBd;

	type = (int)myPs->signal[M_SIG_CALI_VI_SELECT];

	if(type == 0) {
		range = (int)myPs->signal[M_SIG_CALI_RANGE_V] - 1;
	} else {
		range = (int)myPs->signal[M_SIG_CALI_RANGE_I] - 1;
	}

	bd = bd_start;
	switch(myGroup->phase) {
		case P0:
			if(myData->jData[jig].config.jigUseFlag == P1) {
			} else {
				myData->gData[group].signal[G_SIG_CONTACT_COMPLETE] = P1;
			}

			if(type == 0) {
				//send_msg(MODULE_TO_JIGM, MSG_MODULE_JIGM_CCCV, 1, CALI_CV);
			} else if(type == 1) {
				//send_msg(MODULE_TO_JIGM, MSG_MODULE_JIGM_CCCV, 1, CALI_CC);
			}

			gCali_bd_initialize();
			myPs->misc.cali_delay = myPs->misc.timer_1sec;
			myGroup->phase++;
			break;
		case P1:
			if(myGroup->signal[G_SIG_CONTACT_COMPLETE] == P0) break;

			diff = myPs->misc.timer_1sec - myPs->misc.cali_delay;
			if(diff < 1) break;
			myPs->misc.cali_delay = myPs->misc.timer_1sec;

			gCali_bd_select();
			myGroup->phase++;
			break;
		case P2:
			diff = myPs->misc.timer_1sec - myPs->misc.cali_delay;
			if(diff < 1) break;
			myPs->misc.cali_delay = myPs->misc.timer_1sec;

			gCali_bd_cmd_output();
			myGroup->phase++;
			break;
		case P3:
			diff = myPs->misc.timer_1sec - myPs->misc.cali_delay;
			if(diff < 1) break;

			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
			SendMsg.msg = MSG_MODULE_METER_REQUEST;
			SendMsg.val[0] = group;
			SendMsg.val[1] = type;
			send_msg(MODULE_TO_METER, (char *)&SendMsg);

			myPs->misc.cali_delay = myPs->misc.timer_1sec;
			myPs->signal[M_SIG_CALI_METER_DIV] = P0;
			myGroup->phase++;
			break;
		case P4:
			diff = myPs->misc.timer_1sec - myPs->misc.cali_delay;
			if(diff < 1) break;
			myPs->misc.cali_delay = myPs->misc.timer_1sec;

			gCali_bd_next_step_check();
			break;
		case P5:
			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
			SendMsg.msg = MSG_MODULE_METER_REQUEST;
			SendMsg.val[0] = group;
			SendMsg.val[1] = type;
			send_msg(MODULE_TO_METER, (char *)&SendMsg);

			myGroup->phase = P4;
			break;
		case P10:
			diff = myPs->misc.timer_1sec - myPs->misc.cali_delay;
			if(diff < 1) break;
			myPs->misc.cali_delay = myPs->misc.timer_1sec;

			gCali_bd_schedule();
			break;
		case P20: //stop routine
			for(bd=bd_start; bd < bd_end; bd++) {
				for(channel=0; channel < myPs->config.chPerBd; channel++) {
					myData->f_cali.tmp_bData[bd].caliFlag[range] = P0;
					myData->f_cali.tmp_cData[bd][channel].caliFlag[type][range]
						= P0;
				}
			}
			myGroup->phase++;
			break;
		case P21:
			for(bd=bd_start; bd < bd_end; bd++) {
				for(channel=0; channel < myPs->config.chPerBd; channel++) {
					ch = myPs->config.chPerBd * bd + channel;
					myData->cData[ch].signal[C_SIG_RUN_RELAY_OFF]= P1;
					myData->cData[ch].signal[C_SIG_RUN_RELAY_ON]= P0;
					myData->cData[ch].signal[C_SIG_RANGE_RELAY_OFF] = P1;
					myData->cData[ch].signal[C_SIG_SSR_ON] = P0;
				}

				div = 15;
				bVICmd_Setting(bd, 0, 0, div, 0);
				bVICmd_Setting(bd, 0, range, div, 1);
			}

			//send_cmd_cali_switch_on_off(comPort, 0, 0, 0);

			myGroup->state = G_STANDBY;
			myGroup->phase = P0;

			myGroup->signal[G_SIG_CONTACT_COMPLETE] = P0;
			break;
		default: break;
	}*/
}

void gCali_bd_initialize(void)
{ //debug_size_cob
/*	int comPort, bd_start, bd_end, bd, ch, channel, rangeV, rangeI, type, i;
	int discharge_pointNum, charge_pointNum;

	if(myPs->signal[M_SIG_CALI_SEQUENCE] != P2
		&& myPs->signal[M_SIG_CALI_SEQUENCE] != P3) return;

	bd_start = (int)myPs->signal[M_SIG_CALI_BD_START];
	bd_end = bd_start + (int)myPs->signal[M_SIG_CALI_BD_COUNT];
	bd = bd_start;

	if(bd_end > myPs->config.installedBd)  bd_end = myPs->config.installedBd;

	myPs->signal[M_SIG_CALI_STEP] = 0;

	rangeV = (int)myPs->signal[M_SIG_CALI_RANGE_V] - 1;
	rangeI = (int)myPs->signal[M_SIG_CALI_RANGE_I] - 1;

	comPort = Find_comPort(COM_FUNC_TYPE_METER1);
	if(comPort >= 0) {
		if(myPs->signal[M_SIG_CALI_VI_SELECT] == P0) {
			myData->COM.com_port[comPort]
				.signal[COM_SIG_CALI_METER_INITIALIZE] = P1;
		} else {
			myData->COM.com_port[comPort]
				.signal[COM_SIG_CALI_METER_INITIALIZE] = P11;
		}
	}

	//voltage
	type = 0;
	myData->f_cali.tmp_bData[bd].DA_Cali_P_PointNum[type][rangeV]
		= (int)myData->cali_set_data.ch_v.set_point_num[rangeV];
	for(i=0; i < MAX_F_CALI_POINT; i++) {
		if(i < myData->cali_set_data.ch_v.set_point_num[rangeV]) {
			myData->f_cali.tmp_bData[bd].DA_Cali_P_Cmd[type][rangeV][i]
				= myData->cali_set_data.ch_v.set_point[rangeV][i];
		} else {
			myData->f_cali.tmp_bData[bd].DA_Cali_P_Cmd[type][rangeV][i] = 0;
		}
	}
	myData->f_cali.tmp_bData[bd].DA_Cali_N_PointNum[type][rangeV]
		= (int)myData->cali_set_data.ch_v.check_point_num[rangeV];
	for(i=0; i < MAX_F_CALI_CHECK_POINT; i++) {
		if(i < myData->cali_set_data.ch_v.check_point_num[rangeV]) {
			myData->f_cali.tmp_bData[bd].DA_Cali_N_Cmd[type][rangeV][i]
				= myData->cali_set_data.ch_v.check_point[rangeV][i];
		} else {
			myData->f_cali.tmp_bData[bd].DA_Cali_N_Cmd[type][rangeV][i] = 0;
		}
	}

	//current
	type = 1;
	myData->f_cali.tmp_bData[bd].DA_Cali_P_PointNum[type][rangeI]
		= (int)myData->cali_set_data.ch_i.set_point_num[rangeI];
	for(i=0; i < MAX_F_CALI_POINT; i++) {
		if(i < myData->cali_set_data.ch_i.set_point_num[rangeI]) {
			myData->f_cali.tmp_bData[bd].DA_Cali_P_Cmd[type][rangeI][i]
				= myData->cali_set_data.ch_i.set_point[rangeI][i];
		} else {
			myData->f_cali.tmp_bData[bd].DA_Cali_P_Cmd[type][rangeI][i] = 0;
		}
	}

	discharge_pointNum = 0;
	charge_pointNum = 0;
	for(i=0; i < myData->f_cali.tmp_bData[bd].DA_Cali_P_PointNum[type][rangeI];
		i++) {
		if(myData->f_cali.tmp_bData[bd].DA_Cali_P_Cmd[type][rangeI][i] < 0) {
			discharge_pointNum++;
		} else {
			charge_pointNum++;
		}
	}
	myData->f_cali.tmp_bData[bd].DA_Cali_D_PointNum[type][rangeI]
		= discharge_pointNum;
	myData->f_cali.tmp_bData[bd].DA_Cali_C_PointNum[type][rangeI]
		= charge_pointNum;

	myData->f_cali.tmp_bData[bd].DA_Cali_N_PointNum[type][rangeI]
		= (int)myData->cali_set_data.ch_i.check_point_num[rangeI];
	for(i=0; i < MAX_F_CALI_CHECK_POINT; i++) {
		if(i < myData->cali_set_data.ch_i.check_point_num[rangeI]) {
			myData->f_cali.tmp_bData[bd].DA_Cali_N_Cmd[type][rangeI][i]
				= myData->cali_set_data.ch_i.check_point[rangeI][i];
		} else {
			myData->f_cali.tmp_bData[bd].DA_Cali_N_Cmd[type][rangeI][i] = 0;
		}
	}

	if(myPs->signal[M_SIG_CALI_VI_SELECT] == P0) {
		myPs->signal[M_SIG_CALI_CD_SELECT] = P0; //charge mode
	} else {
		if(myPs->signal[M_SIG_CALI_SEQUENCE] == P2) {
			if(myData->f_cali.tmp_bData[bd].DA_Cali_P_Cmd[type][rangeI][0]
				>= 0) {
				myPs->signal[M_SIG_CALI_CD_SELECT] = P0;
			} else {
				myPs->signal[M_SIG_CALI_CD_SELECT] = P1;
			}
		} else {
			if(myData->f_cali.tmp_bData[bd].DA_Cali_N_Cmd[type][rangeI][0]
				>= 0) {
				myPs->signal[M_SIG_CALI_CD_SELECT] = P0;
			} else {
				myPs->signal[M_SIG_CALI_CD_SELECT] = P1;
			}
		}
	}

	if(myPs->signal[M_SIG_CALI_SEQUENCE] == P3) {
		gCalculate_bd_CaliData_1(bd);
		gCalculate_bd_CaliData_2(bd);

		for(channel=0; channel < (int)myPs->config.chInBd[bd]; channel++) {
			ch = myPs->config.chPerBd * bd + channel;
			myData->cData[ch].signal[C_SIG_AUX_DAV] = P21;
			myData->cData[ch].signal[C_SIG_AUX_DAI] = P21;
			myData->cData[ch].signal[C_SIG_RUN_RELAY_OFF] = P1;
			myData->cData[ch].signal[C_SIG_RANGE_RELAY_OFF] = P1;
		}
	} else {
		for(channel=0; channel < (int)myPs->config.chInBd[bd]; channel++) {
			ch = myPs->config.chPerBd * bd + channel;
			myData->cData[ch].signal[C_SIG_AUX_DAV] = P20;
			myData->cData[ch].signal[C_SIG_AUX_DAI] = P20;
			myData->cData[ch].signal[C_SIG_RUN_RELAY_OFF] = P1;
			myData->cData[ch].signal[C_SIG_RANGE_RELAY_OFF] = P1;
			myData->cData[ch].signal[C_SIG_SSR_ON] = P0;
		}
	}*/
}

void gCalculate_bd_CaliData_1(int bd)
{ //debug_size_cob
/*	int ch, type, range, point;
   	int charge_pointNum=0, discharge_pointNum=0;
	double a, b, val1, val2, tmp_val1, tmp_val2;

	type = (int)myPs->signal[M_SIG_CALI_VI_SELECT];

	if(type == 0) {
		range = (int)myPs->signal[M_SIG_CALI_RANGE_V] - 1;
	} else {
		range = (int)myPs->signal[M_SIG_CALI_RANGE_I] - 1;
	}

	for(ch=0; ch < myPs->config.chInBd[bd]; ch++) {
		if(myPs->signal[M_SIG_CALI_PARALLEL] == P1 && (ch % 2) != 0) continue;

		if(type == 0) {
			for(point=0; point < myData->f_cali.tmp_bData[bd]
				.DA_Cali_P_PointNum[type][range]-1; point++) {
				val1 = (double)myData->f_cali.tmp_cData[bd][ch]
					.meter_value_P[type][range][point];
				val2 = (double)myData->f_cali.tmp_cData[bd][ch]
					.meter_value_P[type][range][point+1];
				tmp_val1 = (double)myData->f_cali.tmp_cData[bd][ch]
					.ad_value_P[type][range][point];
				tmp_val2 = (double)myData->f_cali.tmp_cData[bd][ch]
					.ad_value_P[type][range][point+1];

				a = (val2 - val1) / (tmp_val2 - tmp_val1);
				b = val1 - tmp_val1 * a;
				myData->f_cali.tmp_cData[bd][ch].AD_A_P[type][range][point] = a;
				myData->f_cali.tmp_cData[bd][ch].AD_B_P[type][range][point] = b;

				val1 = myData->f_cali.tmp_bData[bd]
					.DA_Cali_P_Cmd[type][range][point];
				val2 = myData->f_cali.tmp_bData[bd]
					.DA_Cali_P_Cmd[type][range][point];
				tmp_val1 = (double)myData->f_cali.tmp_cData[bd][ch]
					.meter_value_P[type][range][point];
				tmp_val2 = (double)myData->f_cali.tmp_cData[bd][ch]
					.meter_value_P[type][range][point+1];

				a = (val2 - val1) / (tmp_val2 - tmp_val1);
				b = val1 - tmp_val1 * a;
				myData->f_cali.tmp_cData[bd][ch].DA_A_P[type][range][point] = a;
				myData->f_cali.tmp_cData[bd][ch].DA_B_P[type][range][point] = b;
			}
		} else {
	 		discharge_pointNum = myData->f_cali.tmp_bData[bd]
				.DA_Cali_D_PointNum[type][range];
			charge_pointNum = myData->f_cali.tmp_bData[bd]
				.DA_Cali_C_PointNum[type][range];
			for(point=0; point < discharge_pointNum-1; point++) {
				val1 = (double)myData->f_cali.tmp_cData[bd][ch]
					.meter_value_P[type][range][point];
				val2 = (double)myData->f_cali.tmp_cData[bd][ch]
					.meter_value_P[type][range][point+1];
				tmp_val1 = (double)myData->f_cali.tmp_cData[bd][ch]
					.ad_value_P[type][range][point];
				tmp_val2 = (double)myData->f_cali.tmp_cData[bd][ch]
					.ad_value_P[type][range][point+1];

				a = (val2 - val1) / (tmp_val2 - tmp_val1);
				b = val1 - tmp_val1 * a;
				myData->f_cali.tmp_cData[bd][ch].AD_A_P[type][range][point] = a;
				myData->f_cali.tmp_cData[bd][ch].AD_B_P[type][range][point] = b;
	
				val1 = myData->f_cali.tmp_bData[bd]
					.DA_Cali_P_Cmd[type][range][point];
				val2 = myData->f_cali.tmp_bData[bd]
					.DA_Cali_P_Cmd[type][range][point];
				tmp_val1 = (double)myData->f_cali.tmp_cData[bd][ch]
					.meter_value_P[type][range][point];
				tmp_val2 = (double)myData->f_cali.tmp_cData[bd][ch]
					.meter_value_P[type][range][point+1];

				a = (val2 - val1) / (tmp_val2 - tmp_val1);
				b = val1 - tmp_val1 * a;
				myData->f_cali.tmp_cData[bd][ch].DA_A_P[type][range][point] = a;
				myData->f_cali.tmp_cData[bd][ch].DA_B_P[type][range][point] = b;
			}

			for(point=discharge_pointNum; point < myData->f_cali.tmp_bData[bd]
				.DA_Cali_P_PointNum[type][range]-1; point++) {
				val1 = (double)myData->f_cali.tmp_cData[bd][ch]
					.meter_value_P[type][range][point];
				val2 = (double)myData->f_cali.tmp_cData[bd][ch]
					.meter_value_P[type][range][point+1];
				tmp_val1 = (double)myData->f_cali.tmp_cData[bd][ch]
					.ad_value_P[type][range][point];
				tmp_val2 = (double)myData->f_cali.tmp_cData[bd][ch]
					.ad_value_P[type][range][point+1];

				a = (val2 - val1) / (tmp_val2 - tmp_val1);
				b = val1 - tmp_val1 * a;
				myData->f_cali.tmp_cData[bd][ch].AD_A_P[type][range][point-1]
					= a;
				myData->f_cali.tmp_cData[bd][ch].AD_B_P[type][range][point-1]
					= b;

				val1 = myData->f_cali.tmp_bData[bd]
					.DA_Cali_P_Cmd[type][range][point];
				val2 = myData->f_cali.tmp_bData[bd]
					.DA_Cali_P_Cmd[type][range][point];
				tmp_val1 = (double)myData->f_cali.tmp_cData[bd][ch]
					.meter_value_P[type][range][point];
				tmp_val2 = (double)myData->f_cali.tmp_cData[bd][ch]
					.meter_value_P[type][range][point+1];

				a = (val2 - val1) / (tmp_val2 - tmp_val1);
				b = val1 - tmp_val1 * a;
				myData->f_cali.tmp_cData[bd][ch].DA_A_P[type][range][point-1]
					= a;
				myData->f_cali.tmp_cData[bd][ch].DA_B_P[type][range][point-1]
					= b;
			}
		}
	}*/
}

void gCalculate_bd_CaliData_2(int bd)
{ //debug_size_cob
/*	int ch, ch_2, type, range, point=0, cali_step;
   	int discharge_pointNum, charge_pointNum;
	double val1, val2, tmp_val1, tmp_val2, sum1, sum2;
	double Aux_DA_a, Aux_DA_b, aux_offset;

	type = (int)myPs->signal[M_SIG_CALI_VI_SELECT];

	if(type == 0) {
		range = (int)myPs->signal[M_SIG_CALI_RANGE_V] - 1;
	} else {
		range = (int)myPs->signal[M_SIG_CALI_RANGE_I] - 1;
	}

	if(type == 0) { //voltage
		if(myPs->signal[M_SIG_CALI_STEP] == P0) {
			//ratio = 2500000uV * 100k / (2M + 100k) / 512
			//		= 232.5148809
			sum1 = 232.5148809;
			aux_offset = 511;

			for(ch=0; ch < myPs->config.chInBd[bd]; ch++) {
				if(myPs->signal[M_SIG_CALI_PARALLEL] == P1 && (ch % 2) != 0) {
					continue;
				}
				ch_2 = myPs->config.chPerBd * bd + ch;

				for(point=0; point < myData->f_cali.tmp_bData[bd]
					.DA_Cali_P_PointNum[type][range]-1; point++) {
					val1 = (double)myData->f_cali.tmp_bData[bd]
						.DA_Cali_P_Cmd[type][range][point];
					val2 = (double)myData->f_cali.tmp_cData[bd][ch]
						.meter_value_P[type][range][point];
					sum2 = (val1 - val2);
					val2 = (sum2 / sum1) + aux_offset;

					tmp_val1 = (double)myData->f_cali.tmp_bData[bd]
						.DA_Cali_P_Cmd[type][range][point+1];
					tmp_val2 = (double)myData->f_cali.tmp_cData[bd][ch]
						.meter_value_P[type][range][point+1];
					sum2 = (tmp_val1 - tmp_val2);
					tmp_val2 = (sum2 / sum1) + aux_offset;

					Aux_DA_a = (val2 - tmp_val2) / (val1 - tmp_val1)
						* 1000000.0;
					Aux_DA_b = val2 - (Aux_DA_a * val1) / 1000000.0;

					myData->f_cali.tmp_cData[bd][ch]
						.AUX_DA_A_P[type][range][point] = Aux_DA_a;
					myData->f_cali.tmp_cData[bd][ch]
						.AUX_DA_B_P[type][range][point] = Aux_DA_b;
				}

				cali_step = (int)myPs->signal[M_SIG_CALI_STEP];
				val1 = (double)myData->f_cali.tmp_bData[bd]
					.DA_Cali_N_Cmd[type][range][cali_step];

				point = aFindDACaliPoint(bd, ch, val1, type, range);
				sum2 = val1 * myData->f_cali.tmp_cData[bd][ch]
					.AUX_DA_A_P[type][range][point] / 1000000.0
					+ myData->f_cali.tmp_cData[bd][ch]
					.AUX_DA_B_P[type][range][point];

				myData->cData[ch_2].misc.auxDaV = (unsigned short)sum2;
				myData->cData[ch_2].signal[C_SIG_AUX_DAV] = P21;
			}
		} else {
			for(ch=0; ch < myPs->config.chInBd[bd]; ch++) {
				if(myPs->signal[M_SIG_CALI_PARALLEL] == P1 && (ch % 2) != 0) {
					continue;
				}
				ch_2 = myPs->config.chPerBd * bd + ch;

				cali_step = (int)myPs->signal[M_SIG_CALI_STEP];
				val1 = (double)myData->f_cali.tmp_bData[bd]
					.DA_Cali_N_Cmd[type][range][cali_step];

				point = aFindDACaliPoint(bd, ch, val1, type, range);
				sum2 = val1 * myData->f_cali.tmp_cData[bd][ch]
					.AUX_DA_A_P[type][range][point] / 1000000.0
					+ myData->f_cali.tmp_cData[bd][ch]
					.AUX_DA_B_P[type][range][point];

				myData->cData[ch_2].misc.auxDaV = (unsigned short)sum2;
				myData->cData[ch_2].signal[C_SIG_AUX_DAV] = P21;
			}
		}
	} else { //current
		if(myPs->signal[M_SIG_CALI_STEP] == P0) {
			//2500mV * 100k / (100k + 1M) = 227.2727273mV
			if(myPs->signal[M_SIG_CALI_CD_SELECT] == P0) { //charge
				switch(range) {
					case 0:
						//1020202.02uA / 512 = 1992.58207
						sum1 = 1992.58207;
						break;
					case 1:
						//102038.788uA / 512 = 199.2945078
						sum1 = 199.2945078;
						break;
					default:
						//1020202.02uA / 512 = 1992.58207
						sum1 = 1992.58207;
						break;
				}
			} else { //discharge
				switch(range) {
					case 0:
						//1033057.851uA / 512 = 2017.691115
						sum1 = 2017.691115;
						break;
					case 1:
						//103324.606uA / 512 = 201.8058711
						sum1 = 201.8058711;
						break;
					default:
						//1033057.851uA / 512 = 2017.691115
						sum1 = 2017.691115;
						break;
				}
			}
			aux_offset = 511.0;

	 		discharge_pointNum = myData->f_cali.tmp_bData[bd]
				.DA_Cali_D_PointNum[type][range];
			charge_pointNum = myData->f_cali.tmp_bData[bd]
				.DA_Cali_C_PointNum[type][range];
			for(ch=0; ch < myPs->config.chInBd[bd]; ch++) {
				if(myPs->signal[M_SIG_CALI_PARALLEL] == P1 && (ch % 2) != 0) {
					continue;
				}
				ch_2 = myPs->config.chPerBd * bd + ch;

				for(point=0; point < discharge_pointNum-1; point++) {
					val1 = (double)myData->f_cali.tmp_bData[bd]
						.DA_Cali_P_Cmd[type][range][point];
					val2 = (double)myData->f_cali.tmp_cData[bd][ch]
						.meter_value_P[type][range][point];
					if(val1 < 0) val1 *= (-1);
					if(val2 < 0) val2 *= (-1);

					sum2 = (val1 - val2); //OK
					val2 = (sum2 / sum1) + aux_offset;

					tmp_val1 = (double)myData->f_cali.tmp_bData[bd]
						.DA_Cali_P_Cmd[type][range][point+1];
					tmp_val2 = (double)myData->f_cali.tmp_cData[bd][ch]
						.meter_value_P[type][range][point+1];
					if(tmp_val1 < 0) tmp_val1 *= (-1);
					if(tmp_val2 < 0) tmp_val2 *= (-1);

					sum2 = (tmp_val1 - tmp_val2); //OK
					tmp_val2 = (sum2 / sum1) + aux_offset;

					Aux_DA_a = (val2 - tmp_val2) / (val1 - tmp_val1)
						* 1000000.0;
					Aux_DA_b = val2 - (val1 * Aux_DA_a) / 1000000.0;

					myData->f_cali.tmp_cData[bd][ch]
						.AUX_DA_A_P[type][range][point] = Aux_DA_a;
					myData->f_cali.tmp_cData[bd][ch]
						.AUX_DA_B_P[type][range][point] = Aux_DA_b;
				}

				for(point=discharge_pointNum; point < myData->f_cali
					.tmp_bData[bd].DA_Cali_P_PointNum[type][range]-1; point++) {
					val1 = (double)myData->f_cali.tmp_bData[bd]
						.DA_Cali_P_Cmd[type][range][point];
					val2 = (double)myData->f_cali.tmp_cData[bd][ch]
						.meter_value_P[type][range][point];
					if(val1 < 0) val1 *= (-1);
					if(val2 < 0) val2 *= (-1);

					sum2 = (val1 - val2); //OK
					val2 = (sum2 / sum1) + aux_offset;

					tmp_val1 = (double)myData->f_cali.tmp_bData[bd]
						.DA_Cali_P_Cmd[type][range][point+1];
					tmp_val2 = (double)myData->f_cali.tmp_cData[bd][ch]
						.meter_value_P[type][range][point+1];
					if(tmp_val1 < 0) tmp_val1 *= (-1);
					if(tmp_val2 < 0) tmp_val2 *= (-1);

					sum2 = (tmp_val1 - tmp_val2); //OK
					tmp_val2 = (sum2 / sum1) + aux_offset;

					Aux_DA_a = (val2 - tmp_val2) / (val1 - tmp_val1)
						* 1000000.0;
					Aux_DA_b = val2 - (val1 * Aux_DA_a) / 1000000.0;

					myData->f_cali.tmp_cData[bd][ch]
						.AUX_DA_A_P[type][range][point-1] = Aux_DA_a;
					myData->f_cali.tmp_cData[bd][ch]
						.AUX_DA_B_P[type][range][point-1] = Aux_DA_b;
				}

				cali_step = (int)myPs->signal[M_SIG_CALI_STEP];
				val1 = (double)myData->f_cali.tmp_bData[bd]
					.DA_Cali_N_Cmd[type][range][cali_step];

				point = aFindDACaliPoint(bd, ch, val1, type, range);
				if(val1 < 0) val1 *= (-1);
				sum2 = val1 * (double)myData->f_cali.tmp_cData[bd][ch]
					.AUX_DA_A_P[type][range][point] / 1000000.0
					+ myData->f_cali.tmp_cData[bd][ch]
					.AUX_DA_B_P[type][range][point];

				myData->cData[ch_2].misc.auxDaI = (short int)sum2;
				myData->cData[ch_2].signal[C_SIG_AUX_DAI] = P21;
			}
		} else {
			for(ch=0; ch < myPs->config.chInBd[bd]; ch++) {
				if(myPs->signal[M_SIG_CALI_PARALLEL] == P1 && (ch % 2) != 0) {
					continue;
				}
				ch_2 = myPs->config.chPerBd * bd + ch;

				cali_step = (int)myPs->signal[M_SIG_CALI_STEP];
				val1 = (double)myData->f_cali.tmp_bData[bd]
					.DA_Cali_N_Cmd[type][range][cali_step];

				point = aFindDACaliPoint(bd, ch, val1, type, range);
				if(val1 < 0) val1 *= (-1);

				sum2 = val1 * (double)myData->f_cali.tmp_cData[bd][ch]
					.AUX_DA_A_P[type][range][point] / 1000000.0
					+ myData->f_cali.tmp_cData[bd][ch]
					.AUX_DA_B_P[type][range][point];

				myData->cData[ch_2].misc.auxDaI = (short int)sum2;
				myData->cData[ch_2].signal[C_SIG_AUX_DAI] = P21;
			}
		}
	}*/
}

void gCali_bd_select(void)
{
#ifdef __COB__
	int bd, ch, channel, rangeV, rangeI, div, i, j; //, comPort, meter, group=0;
	long l_val1, l_val2;

	if(myPs->signal[M_SIG_CALI_SEQUENCE] != P2
		&& myPs->signal[M_SIG_CALI_SEQUENCE] != P3) return;

	rangeV = (int)myPs->signal[M_SIG_CALI_RANGE_V] - 1;
	rangeI = (int)myPs->signal[M_SIG_CALI_RANGE_I] - 1;

	bd = (int)myPs->signal[M_SIG_CALI_BD_START];
	if(bd >= myPs->config.installedBd) bd = myPs->config.installedBd - 1;

	gCali_bd_cd_select();

	j = (int)myPs->signal[M_SIG_CALI_CH_FLAG_IDX];
	for(i=j; i < myPs->config.installedCh; i++) {
		if(myData->cali_ch_flag.flag[i] != 0) {
			myPs->signal[M_SIG_CALI_CH_FLAG_IDX] = (unsigned char)i;
			myPs->signal[M_SIG_CALI_CH] = (unsigned char)i;
			break;
		}
	}

	div = 5;
	l_val1 = 0; l_val2 = 0;	
	bVICmd_Setting(bd, l_val1, rangeV, div, 0);
    if(myPs->signal[M_SIG_CALI_CD_SELECT] == P1) { //discharge
		l_val2 = -1;
	}
	bVICmd_Setting(bd, l_val2, rangeI, div, 1);

	for(channel=0; channel < myPs->config.chPerBd; channel++) {
		ch = myPs->config.chPerBd * bd + channel;
		myData->cData[ch].signal[C_SIG_RANGE_RELAY_ON] = P1;

		if(channel == (int)myPs->signal[M_SIG_CALI_CH]) {
			myData->cData[ch].signal[C_SIG_RUN_RELAY_ON] = P1;
			myData->cData[ch].signal[C_SIG_SSR_ON] = P1;
		}
	}
/*
	if(myPs->signal[M_SIG_CALI_VI_SELECT] == P0) { //voltage
		meter = 0x00;
	} else { //current
		meter = 0x01;
	}

	if(myPs->signal[M_SIG_CALI_CD_SELECT] == P0) { //charge
		meter += 0x00;
	} else { //discharge
		meter += 0x02;
	}

	for(comPort=0; comPort < myData->COM.config.installedCOM; comPort++) {
		switch(myData->COM.config.functionType[comPort]) {
			case COM_FUNC_CALI_SWITCH1:
			case COM_FUNC_CALI_SWITCH2:
			case COM_FUNC_CALI_SWITCH3:
				if(myData->gData[group].selectMap == MAP_CALI){
					ch = Convert_Cali_Ch(group, bd, ch);
				}else{
					ch = Convert_Meas_Ch(group, bd, ch);
				}
				send_cmd_cali_switch_on_off(comPort, meter, 0, ch+1);
				break;
			default:	break;
		}
	}*/
#endif
}

void gCali_bd_cd_select(void)
{ //debug_size_cob
/*	int point, type, bd_start, bd, rangeI;
	long l_val2=0;

	bd_start = (int)myPs->signal[M_SIG_CALI_BD_START];
	bd = bd_start;

	rangeI = (int)myPs->signal[M_SIG_CALI_RANGE_I] - 1;
	point = (int)myPs->signal[M_SIG_CALI_STEP];

	if(myPs->signal[M_SIG_CALI_VI_SELECT] == P0) { //voltage
		myPs->signal[M_SIG_CALI_CD_SELECT] = P0;
	} else { //current
		type = 1;
		if(myPs->signal[M_SIG_CALI_SEQUENCE] == P2) { //cali
			l_val2 = myData->f_cali.tmp_bData[bd]
				.DA_Cali_P_Cmd[type][rangeI][point];
		} else { //cali_check
			l_val2 = myData->f_cali.tmp_bData[bd]
				.DA_Cali_N_Cmd[type][rangeI][point];
		}

		if(l_val2 >= 0) { //charge
			myPs->signal[M_SIG_CALI_CD_SELECT] = P0;
		} else { //discharge
			myPs->signal[M_SIG_CALI_CD_SELECT] = P1;
		}
	}*/
}

void gCali_bd_cmd_output(void)
{ //debug_size_cob
/*	int point, type, bd_start, bd, ch, ch_2, rangeV, rangeI, div;
	long l_val1, l_val2;

	myPs->signal[M_SIG_CALI_METER_DIV] = P0;

	if(myPs->signal[M_SIG_CALI_SEQUENCE] != P2
		&& myPs->signal[M_SIG_CALI_SEQUENCE] != P3) return;

	bd_start = (int)myPs->signal[M_SIG_CALI_BD_START];
	bd = bd_start;

	div = 5;
	rangeV = (int)myPs->signal[M_SIG_CALI_RANGE_V] - 1;
	rangeI = (int)myPs->signal[M_SIG_CALI_RANGE_I] - 1;
	ch = (int)myPs->signal[M_SIG_CALI_CH];
	ch_2 = myPs->config.chPerBd * bd + ch;
	point = (int)myPs->signal[M_SIG_CALI_STEP];

	if(myPs->signal[M_SIG_CALI_VI_SELECT] == P0) { //voltage
		type = 0;
	} else {
		type = 1;
	}

	if(myPs->signal[M_SIG_CALI_SEQUENCE] == P2) { //cali
		myData->f_cali.tmp_cData[bd][ch].DA_A_P[type][rangeI][point] = 1.0;
		myData->f_cali.tmp_cData[bd][ch].DA_B_P[type][rangeI][point] = 0.0;

		myData->cData[ch_2].signal[C_SIG_AUX_DAV] = P20;
		myData->cData[ch_2].signal[C_SIG_AUX_DAI] = P20;
	} else if(myPs->signal[M_SIG_CALI_SEQUENCE] == P3) { //check
		gCalculate_bd_CaliData_2(bd);

		myData->cData[ch_2].signal[C_SIG_AUX_DAV] = P21;
		myData->cData[ch_2].signal[C_SIG_AUX_DAI] = P21;
	}

	if(myPs->signal[M_SIG_CALI_VI_SELECT] == P0) { //voltage
		if(myPs->signal[M_SIG_CALI_SEQUENCE] == P2) { //cali
			l_val1 = myData->f_cali.tmp_bData[bd]
				.DA_Cali_P_Cmd[type][rangeV][point];
			myData->f_cali.tmp_cData[bd][ch].DA_Cali_P_Cmd[type][rangeV][point]
				= l_val1;
		} else { //cali_check
			l_val1 = myData->f_cali.tmp_bData[bd]
				.DA_Cali_N_Cmd[type][rangeV][point];
			myData->f_cali.tmp_cData[bd][ch].DA_Cali_N_Cmd[type][rangeV][point]
				= l_val1;
		}
		l_val2 = myPs->config.maxI[rangeI];
		myPs->signal[M_SIG_CALI_CD_SELECT] = P0;
	} else { //current
		if(myPs->signal[M_SIG_CALI_SEQUENCE] == P2) { //cali
			l_val2 = myData->f_cali.tmp_bData[bd]
				.DA_Cali_P_Cmd[type][rangeI][point];
			myData->f_cali.tmp_cData[bd][ch].DA_Cali_P_Cmd[type][rangeI][point]
				= l_val2;
		} else { //cali_check
			l_val2 = myData->f_cali.tmp_bData[bd]
				.DA_Cali_N_Cmd[type][rangeI][point];
			myData->f_cali.tmp_cData[bd][ch].DA_Cali_N_Cmd[type][rangeI][point]
				= l_val2;
		}
		if(l_val2 >= 0) { //charge
		   	l_val1 = myPs->config.maxV[0];
			myPs->signal[M_SIG_CALI_CD_SELECT] = P0;
		} else { //discharge
			l_val1 = myPs->config.minV[0];
			myPs->signal[M_SIG_CALI_CD_SELECT] = P1;
		}
	}

	bVICmd_Setting(bd, l_val1, rangeV, div, 0);
	bVICmd_Setting(bd, l_val2, rangeI, div, 1);*/
}

void gCali_bd_next_step_check(void)
{ //debug_size_cob
/*	unsigned char flag=0x00;
	int bd_start, bd, ch, ch_2, group=0, type, range, i, j, num; //, comPort, meter;
	int sharedCh=0;
	long tmp=0;

	if(myPs->signal[M_SIG_CALI_SEQUENCE] != P2
		&& myPs->signal[M_SIG_CALI_SEQUENCE] != P3) return;

	bd_start = (int)myPs->signal[M_SIG_CALI_BD_START];
	bd = bd_start + myPs->signal[M_SIG_CALI_METER_DIV] * 2;
	ch = (int)myPs->signal[M_SIG_CALI_CH];
	ch_2 = myPs->config.chPerBd * bd + ch;
	type = (int)myPs->signal[M_SIG_CALI_VI_SELECT];

	if(type == 0) {
		range = (int)myPs->signal[M_SIG_CALI_RANGE_V] - 1;
		if(myPs->signal[M_SIG_CALI_CD_SELECT] == P0) { //charge
			flag = 0x01;
		} else { //discharge
			flag = 0x02;
		}
	} else {
		range = (int)myPs->signal[M_SIG_CALI_RANGE_I] - 1;
		if(myPs->signal[M_SIG_CALI_CD_SELECT] == P0) { //charge
			flag = 0x04;
		} else { //discharge
			flag = 0x08;
		}
	}

	myData->cData[ch_2].signal[C_SIG_RUN_RELAY_OFF] = P1;
	myData->cData[ch_2].signal[C_SIG_RANGE_RELAY_OFF] = P1;
	myData->cData[ch_2].signal[C_SIG_SSR_ON] = P0;

	sharedCh = ch;

	if(myPs->signal[M_SIG_CALI_SEQUENCE] == P2) { //cali
		if(myPs->signal[M_SIG_CALI_STEP] > 0) {
			myData->f_cali.tmp_cData[bd][ch].caliFlag[type][range] = P1;
			myData->f_cali.tmp_bData[bd].caliFlag[range] |= flag;
		}
	}

	i = (int)myPs->signal[M_SIG_CALI_STEP];

	if(type == 0) { //voltage
		tmp = (long)myPs->misc.meter_value;

		if(myPs->signal[M_SIG_CALI_SEQUENCE] == P2) { //cali
			myData->f_cali.tmp_cData[bd][ch].meter_value_P[type][range][i]
				= tmp;
			myData->f_cali.tmp_cData[bd][ch].ad_value_P[type][range][i]
				= myData->cData[ch_2].op.Vsens;
			myData->f_cali.tmp_cData[bd][ch].DA_Cali_P_Cmd[type][range][i]
				= myData->f_cali.tmp_bData[bd].DA_Cali_P_Cmd[type][range][i];
			myData->f_cali.tmp_cData[bd][sharedCh]
				.DA_Cali_P_PointNum[type][range]
				= myData->f_cali.tmp_cData[bd][ch]
				.DA_Cali_P_PointNum[type][range];
		} else { //cali_check
			myData->f_cali.tmp_cData[bd][ch].meter_value_N[type][range][i]
				= tmp;
			myData->f_cali.tmp_cData[bd][ch].ad_value_N[type][range][i]
				= myData->cData[ch_2].op.Vsens;
		}
	} else { //current
		//shuntR = 10mOhm -> 100000uV -> 10000000uA
		tmp= (long)myPs->misc.meter_value * 100;

		if(myPs->signal[M_SIG_CALI_SEQUENCE] == P2) { //cali
			myData->f_cali.tmp_cData[bd][ch].meter_value_P[type][range][i]
				= tmp;
			myData->f_cali.tmp_cData[bd][ch].ad_value_P[type][range][i]
				= myData->cData[ch_2].op.Isens;

			myData->f_cali.tmp_cData[bd][ch].DA_Cali_P_Cmd[type][range][i]
				= myData->f_cali.tmp_bData[bd].DA_Cali_P_Cmd[type][range][i];
			myData->f_cali.tmp_cData[bd][ch].DA_Cali_P_PointNum[type][range]
				= myData->f_cali.tmp_cData[bd][ch]
				.DA_Cali_P_PointNum[type][range];
		} else { //cali_check
			myData->f_cali.tmp_cData[bd][ch].meter_value_N[type][range][i]
				= tmp;
			myData->f_cali.tmp_cData[bd][ch].ad_value_N[type][range][i]
				= myData->cData[ch_2].op.Isens;
		}
	}

	gCali_send_cali_data(bd, ch);

	//if(myData->COM.config.multi_cali_type[0] == 1) {
	//	num = 3; //khkw
	//} else {
		num = 0;
	//}
	if(num > 0) {
		if(myPs->signal[M_SIG_CALI_METER_DIV] >= num) {
			myPs->signal[M_SIG_CALI_METER_DIV] = 0;
			j = (int)myPs->signal[M_SIG_CALI_CH_FLAG_IDX] + 1;
			myPs->signal[M_SIG_CALI_CH_FLAG_IDX] = 0;
			for(i=j; i < myPs->config.installedCh; i++) { //kjg_110816_w
				if(myData->cali_ch_flag.flag[i] != 0) {
					myPs->signal[M_SIG_CALI_CH_FLAG_IDX] = (unsigned char)i;
					myPs->signal[M_SIG_CALI_CH] = (unsigned char)i;
					break;
				}
			}

			if(myPs->signal[M_SIG_CALI_CH_FLAG_IDX] != 0 
				&& i < (int)myPs->config.chInBd[bd]) {
				myData->gData[group].phase = P1; // next ch 
			} else {
				myData->gData[group].phase = P10; //next cali step
			}
		} else { //next board meter, ad read
			myPs->signal[M_SIG_CALI_METER_DIV]++;
			myData->gData[group].phase++;
		}
	} else {
		myPs->signal[M_SIG_CALI_METER_DIV] = 0;
		j = (int)myPs->signal[M_SIG_CALI_CH_FLAG_IDX] + 1;
		myPs->signal[M_SIG_CALI_CH_FLAG_IDX] = 0;
		for(i=j; i < myPs->config.installedCh; i++) { //kjg_110816_w
			if(myData->cali_ch_flag.flag[i] != 0) {
				myPs->signal[M_SIG_CALI_CH_FLAG_IDX] = (unsigned char)i;
				myPs->signal[M_SIG_CALI_CH] = (unsigned char)i;
				break;
			} else {
				myPs->signal[M_SIG_CALI_CH_FLAG_IDX] = 0;
				break;
			}
		}

		if(myPs->signal[M_SIG_CALI_CH_FLAG_IDX] != 0 // next ch
			&& i < (int)myPs->config.chInBd[bd]) {
			myData->gData[group].phase = P1;
		} else { //next cali step
			myData->gData[group].phase = P10;
		}
	}

	myPs->signal[M_SIG_CALIBRATION] = P0;*/
}

void gCali_send_cali_data(int bd, int ch)
{
#ifdef __COB__
	int toPs1, group;
	S_MSG_VAL SendMsg;

	group = 0;
	toPs1 = MODULE_TO_COB1 + group;

	memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
	SendMsg.val[0] = bd;
	SendMsg.val[1] = ch;
	if(myPs->signal[M_SIG_CALI_SEQUENCE] == P2) {
		SendMsg.msg = MSG_MODULE_COB_CALI_DATA;
	} else {
		SendMsg.msg = MSG_MODULE_COB_CALI_CHECK_DATA;
	}
	send_msg(toPs1, (char *)&SendMsg);

	myPs->signal[M_SIG_CALI_STEP_TMP] = myPs->signal[M_SIG_CALI_STEP];
	myPs->signal[M_SIG_CALI_VI_SELECT_TMP] = myPs->signal[M_SIG_CALI_VI_SELECT];
#endif
}

void gCali_bd_schedule(void)
{ //debug_size_cob
/*	int cali_step, bd_start, bd_end, bd, range=0, div, type, group=0;
	long l_val1=0;

	if(myPs->signal[M_SIG_CALI_SEQUENCE] != P2
		&& myPs->signal[M_SIG_CALI_SEQUENCE] != P3) return;

	bd_start = (int)myPs->signal[M_SIG_CALI_BD_START];
	bd_end = bd_start + (int)myPs->signal[M_SIG_CALI_BD_COUNT];

	if(bd_end > myPs->config.installedBd) bd_end = myPs->config.installedBd;

	type = (int)myPs->signal[M_SIG_CALI_VI_SELECT];
	div = 5;
	if(type == 0) {
		range = (int)myPs->signal[M_SIG_CALI_RANGE_V] - 1;
	} else {
		range = (int)myPs->signal[M_SIG_CALI_RANGE_I] - 1;
	}

	if(myPs->signal[M_SIG_CALI_STEP] == P0) {
		if(myPs->signal[M_SIG_CALI_SEQUENCE] == P2) { //cali
			myPs->signal[M_SIG_CALI_STEP]++;
			myPs->signal[M_SIG_CALI_CH] = 0;
			myPs->signal[M_SIG_CALI_CH_FLAG_IDX] = 0;
		} else { //cali_check
			myPs->signal[M_SIG_CALI_STEP]++;
			myPs->signal[M_SIG_CALI_CH] = 0;
			myPs->signal[M_SIG_CALI_CH_FLAG_IDX] = 0;
		}
		myData->gData[group].phase = P1;
	} else if(myPs->signal[M_SIG_CALI_STEP] >= P1) {
		if(myPs->signal[M_SIG_CALI_SEQUENCE] == P2) { //cali
			bd = bd_start;
			myPs->signal[M_SIG_CALI_STEP]++;
			cali_step = myPs->signal[M_SIG_CALI_STEP];
			if(myPs->signal[M_SIG_CALI_VI_SELECT] == P0) { //voltage
				l_val1 = myData->f_cali.tmp_bData[bd]
					.DA_Cali_P_Cmd[0][range][cali_step];
			} else { //current
				l_val1 = myData->f_cali.tmp_bData[bd]
					.DA_Cali_P_Cmd[1][range][cali_step];
			}
			if(l_val1 != 0) {
				if(l_val1 >= 0) {
					myPs->signal[M_SIG_CALI_CD_SELECT] = P0; //charge
				} else {
					myPs->signal[M_SIG_CALI_CD_SELECT] = P1; //discharge
				}
				myPs->signal[M_SIG_CALI_CH] = 0;
				myPs->signal[M_SIG_CALI_CH_FLAG_IDX] = 0;
				myData->gData[group].phase = P1;
			} else {
				myPs->signal[M_SIG_CALI_STEP] = P0;
				gCali_bd_next_div_check();
			}
		} else { //cali_check
			myPs->signal[M_SIG_CALI_STEP]++;
			cali_step = myPs->signal[M_SIG_CALI_STEP];
			bd = bd_start;
			if(myPs->signal[M_SIG_CALI_VI_SELECT] == P0) { //voltage
				l_val1 = myData->f_cali.tmp_bData[bd]
					.DA_Cali_N_Cmd[0][range][cali_step];
			} else { //current
				l_val1 = myData->f_cali.tmp_bData[bd]
					.DA_Cali_N_Cmd[1][range][cali_step];
			}
			if(l_val1 != 0) {
				myPs->signal[M_SIG_CALI_CH] = 0;
				myPs->signal[M_SIG_CALI_CH_FLAG_IDX] = 0;
				myData->gData[group].phase = P1;
			} else {
				gCali_bd_next_div_check();
			}
		}
	}*/
}

void gCali_bd_next_div_check(void)
{ //debug_size_cob
/*	unsigned char flag;
	int type, bd_start, bd, range=0, group, toPs1; //, comPort;
	S_MSG_VAL SendMsg;

	bd_start = (int)myPs->signal[M_SIG_CALI_BD_START];
	bd = bd_start;

	type = (int)myPs->signal[M_SIG_CALI_VI_SELECT];

	if(type == 0) {
		range = (int)myPs->signal[M_SIG_CALI_RANGE_V] - 1;
	} else {
		range = (int)myPs->signal[M_SIG_CALI_RANGE_I] - 1;
	}

	if(type == 0) { //voltage
		if(myPs->signal[M_SIG_CALI_CD_SELECT] == P0) { //charge
			flag = 0x01;
		} else { //discharge
			flag = 0x02;
		}

		myData->f_cali.tmp_bData[bd].caliFlag[range] |= flag;
	} else { //current
		if(myPs->signal[M_SIG_CALI_CD_SELECT] == P0) { //charge
			flag = 0x04;
		} else { //discharge
			flag = 0x08;
		}
		myData->f_cali.tmp_bData[bd].caliFlag[range] |= flag;
	}

	memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
	SendMsg.msg = MSG_MODULE_APP_CALI_END;
	SendMsg.val[0] = bd;
	SendMsg.val[1] = (int)range;
	send_msg(MODULE_TO_APP, (char *)&SendMsg);

	group = 0;
	toPs1 = MODULE_TO_COB1 + group;
	memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
	SendMsg.val[0] = bd;
	SendMsg.val[1] = (int)myPs->signal[M_SIG_CALI_VI_SELECT];
	if(myPs->signal[M_SIG_CALI_SEQUENCE] == P2) {
		gCalculate_bd_CaliData_1(bd);
		gCalculate_bd_CaliData_2(bd);

		SendMsg.msg = MSG_MODULE_COB_CALI_END;
	} else {
		SendMsg.msg = MSG_MODULE_COB_CALI_CHECK_END;
	}
	send_msg(toPs1, (char *)&SendMsg);

	myData->gData[group].phase = P21;*/
}
