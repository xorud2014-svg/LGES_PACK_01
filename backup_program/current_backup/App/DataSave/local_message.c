#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../../INC/datastore.h"
#include "userlog.h"
#include "common_utils.h"
#include "local_utils.h"
#include "local_message.h"

extern volatile S_SYSTEM_DATA	*myData;
extern volatile S_DATA_SAVE		*myPs;
extern char psName[PROCESS_NAME_SIZE];

void Check_Message(void)
{
	msgParsing(MODULE_TO_DATASAVE);
}

void msgParsing(int fromPs)
{
	int idx;
	S_MSG_VAL RecvMsg;
	
	idx = rcv_msg(fromPs, (S_MSG_VAL *)&RecvMsg);
	if(idx < 0) return;
	
	switch(fromPs) {
		case MODULE_TO_DATASAVE:
			msgParsing_Module_to_DataSave(fromPs, idx, (S_MSG_VAL *)&RecvMsg);
			break;
		default:
			userlog(DEBUG_LOG, psName,
				"Msg Direction UnKnown : %d %d %d\n", fromPs, idx, RecvMsg.msg);
			break;
	}
}

void msgParsing_Module_to_DataSave(int fromPs, int idx, S_MSG_VAL *RecvMsg)
{
	unsigned char chFlag, chFlag1;
	int group, ch, i, j, k, toPs, rtn=0, code;
	int idx2, stepNo, idxStepNo;	//20181219 KHK
//#ifdef __B_TYPE__
	int count; //kjhw_120504 Vref x 2
//#endif
	S_MSG_CH_FLAG ch_flag;
	S_MSG_VAL SendMsg;

	group = RecvMsg->val[0];
	chFlag = 0x01;
	k = 0;
	for(i=0; i < MAX_GROUP_PER_MODULE; i++) {
		if(group == i) {
			break;
		}
		k += (int)myData->mData.config.chInGroup[i];
	}

	memcpy((char *)&ch_flag, (char *)&myData->msg[fromPs].msg_ch_flag[idx],
		sizeof(S_MSG_CH_FLAG));

	switch(RecvMsg->msg) {
		//for pack_cycler, triangle_cycler
		case MSG_MODULE_DATASAVE_RCVED_CMD_RUN:
			userlog(DEBUG_LOG, psName, "rcved_cmd_run1\n");
			for(i=0; i < myData->mData.config.chInGroup[group]; i++) {
				j = i / 8;
				chFlag1 = (chFlag << (i % 8)) & ch_flag.flag[j];
				if(chFlag1 == 0) continue;

				Open_ResultData_1(k + i);

#ifdef __COC__
				Open_ResultData_General_1(k + i);
#endif
			}
		
			memcpy((char *)&SendMsg,
				(char *)&myData->msg[fromPs].msg_val[idx], sizeof(S_MSG_VAL));

			toPs = DATASAVE_TO_MODULE;
			send_msg_ch_flag(toPs, (char *)&ch_flag);
			SendMsg.msg = MSG_DATASAVE_MODULE_CMD_RUN;
			SendMsg.val[2] = RecvMsg->val[2]; //debug_on
			send_msg(toPs, (char *)&SendMsg);
			userlog(DEBUG_LOG, psName, "rcved_cmd_run2 %d %d %d %d\n",
				SendMsg.val[0], SendMsg.val[1], SendMsg.val[2], SendMsg.val[3]);
			break;
		case MSG_MODULE_DATASAVE_RCVED_CMD_CONTINUE:
			for(i=0; i < myData->mData.config.chInGroup[group]; i++) {
				j = i / 8;
				chFlag1 = (chFlag << (i % 8)) & ch_flag.flag[j];
				if(chFlag1 == 0) continue;

				Open_ResultData_2(k + i);

#ifdef __COC__
				Open_ResultData_General_2(k + i);
#endif
			}
		
			memcpy((char *)&SendMsg,
				(char *)&myData->msg[fromPs].msg_val[idx], sizeof(S_MSG_VAL));

			toPs = DATASAVE_TO_MODULE;
			send_msg_ch_flag(toPs, (char *)&ch_flag);
			SendMsg.msg = MSG_DATASAVE_MODULE_CMD_CONTINUE;
			send_msg(toPs, (char *)&SendMsg);
			break;
		case MSG_MODULE_DATASAVE_CALI_NORMAL_RESULT_SAVE:
			ch = RecvMsg->val[0];
//#ifdef __B_TYPE__
			count = RecvMsg->val[1]; //kjhw_120504 Vref x 2
//#endif
			//kjg_w CaliCheckDataSave(bd, ch);

			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));

			if(myData->AppControl.misc.Load_Process[PROCESS_GROUP_COA][0]
				== P1) {
				toPs = DATASAVE_TO_COA1; //kjg_w
				SendMsg.msg = MSG_DATASAVE_COA_CALI_NORMAL_RESULT_SEND;
				SendMsg.val[0] = ch;
//#ifdef __B_TYPE__
				SendMsg.val[1] = count; //kjhw_120504 Vref x 2
//#endif
				send_msg(toPs, (char *)&SendMsg);
			}

			if(myData->AppControl.misc.Load_Process[PROCESS_GROUP_COC][0]
				== P1) {
				toPs = DATASAVE_TO_COC1; //kjg_w
				SendMsg.msg = MSG_DATASAVE_COC_CALI_NORMAL_RESULT_SEND;
				SendMsg.val[0] = ch;
				send_msg(toPs, (char *)&SendMsg);
			}

			toPs = DATASAVE_TO_MODULE;
			SendMsg.msg = MSG_DATASAVE_MODULE_CALI_NORMAL_RESULT_SAVED;
			SendMsg.val[0] = ch;
			send_msg(toPs, (char *)&SendMsg);
			break;
		case MSG_MODULE_DATASAVE_CALI_CHECK_RESULT_SAVE:
			ch = RecvMsg->val[0];
			//kjg_w CaliCheckDataSave(bd, ch);

			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));

			if(myData->AppControl.misc.Load_Process[PROCESS_GROUP_COA][0]
				== P1) {
				toPs = DATASAVE_TO_COA1; //kjg_w
				SendMsg.msg = MSG_DATASAVE_COA_CALI_CHECK_RESULT_SEND;
				SendMsg.val[0] = ch;
				send_msg(toPs, (char *)&SendMsg);
			}

			if(myData->AppControl.misc.Load_Process[PROCESS_GROUP_COC][0]
				== P1) {
				toPs = DATASAVE_TO_COC1; //kjg_w
				SendMsg.msg = MSG_DATASAVE_COC_CALI_CHECK_RESULT_SEND;
				SendMsg.val[0] = ch;
				send_msg(toPs, (char *)&SendMsg);
			}

			toPs = DATASAVE_TO_MODULE;
			SendMsg.msg = MSG_DATASAVE_MODULE_CALI_CHECK_RESULT_SAVED;
			SendMsg.val[0] = ch;
			send_msg(toPs, (char *)&SendMsg);
			break;
		case MSG_MODULE_DATASAVE_READ_PATTERN_FILE:
			ch = RecvMsg->val[0];

			i = read_test_cond_pattern_file_coa(psName, ch, RecvMsg->val[1], 1);	//ktg_220512	//shh_220607
//20181219 KHK--------------------------------------------
		//jhkw_201102s
		idx2 = IDX_LOC_OBJ_SOC_TRACKING_FLAG;
		stepNo = (int)RecvMsg->val[1];
		if(myData->testCond[ch].local_object[stepNo][idx2] == P1){
			i += Read_SOC_Tracking_File(psName, ch, stepNo, 0); //charge
		}

		idx2 = IDX_LOC_OBJ_DISCHARGE_SOC_TRACKING_FLAG;
		stepNo = (int)RecvMsg->val[1];
		if(myData->testCond[ch].local_object[stepNo][idx2] == P1){
			i += Read_SOC_Tracking_File(psName, ch, stepNo, 1); //discharge
		}
		//jhkw_201102e
//----------------------------------------------------------
#ifdef __COC__
			i = Read_Pattern_File_2(psName, ch, RecvMsg->val[1]);
#endif
			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
			toPs = DATASAVE_TO_MODULE;
			SendMsg.msg = MSG_DATASAVE_MODULE_READ_PATTERN_FILE_END;
			SendMsg.val[0] = ch;
			if(i >= 0) SendMsg.val[1] = P1;
			else SendMsg.val[1] = P2;
			send_msg(toPs, (char *)&SendMsg);

			userlog(DEBUG_LOG, psName, "read_pattern_file_end %d %d %d\n",
				SendMsg.val[0], SendMsg.val[1],
				(int)myData->cData[ch].signal[C_SIG_OUT_SWITCH]);
			break;
		//jhkw_221205s
	case MSG_MODULE_DATASAVE_READ_SEQUENCE_CHARGE_FILE:
		ch = RecvMsg->val[0];
		i = Read_Sequence_Charge_File(psName, ch, RecvMsg->val[1]);
		if(i >= 0) {
			idx2 = IDX_LOC_OBJ_REF_I;
			idxStepNo = myData->cData[ch].op.idxStepNo;
			myData->testCond[ch].local_object[idxStepNo][idx2]
				= myData->testCond[ch].SQ_Charge.maxI;
		}
		memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
		toPs = DATASAVE_TO_MODULE;
		SendMsg.msg = MSG_DATASAVE_MODULE_READ_END_SEQUENCE_CHARGE_FILE;
		SendMsg.val[0] = ch;
		if(i >= 0) SendMsg.val[1] = P1;
		else SendMsg.val[1] = P2;
		send_msg(toPs, (char *)&SendMsg);

		userlog(DEBUG_LOG, psName, "read end  Sequence Charge file%d %d %d\n",
			SendMsg.val[0], SendMsg.val[1],
			(int)myData->cData[ch].signal[C_SIG_OUT_SWITCH]);
		break;
	//jhkw_221205e
	case MSG_MODULE_DATASAVE_READ_USERMAP_FILE: //kjwh_140828
			ch = RecvMsg->val[0];
			i = read_test_cond_usermap_file_coa(psName, ch, RecvMsg->val[1]);

			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
			toPs = DATASAVE_TO_MODULE;
			SendMsg.msg = MSG_DATASAVE_MODULE_READ_USERMAP_FILE_END;
			SendMsg.val[0] = ch;
			if(i >= 0) SendMsg.val[1] = P1;
			else SendMsg.val[1] = P2;
			send_msg(toPs, (char *)&SendMsg);

			userlog(DEBUG_LOG, psName, "read_usermap_file_end %d %d %d\n",
				SendMsg.val[0], SendMsg.val[1],
				(int)myData->cData[ch].signal[C_SIG_OUT_SWITCH]);
			break;
		case MSG_MODULE_DATASAVE_CALI_VALUE:
			userlog(DEBUG_LOG, psName, "Cali meter:%ld, ad:%ld\n",
				RecvMsg->val[1], RecvMsg->val[0]);
			break;
		case MSG_MODULE_DATASAVE_CREATE_RUN_FILE:
			ch = RecvMsg->val[0];
			rtn = create_run_file(ch);
			break;
		case MSG_MODULE_DATASAVE_DELETE_RUN_FILE:
			ch = RecvMsg->val[0];
			rtn = delete_run_file(ch);
			break;

		//for formation
		case MSG_MODULE_DATASAVE_SAVED_FILE_DELETE_TRY:
			rtn = Open_MonitoringData_Formation_1(group);
			if(rtn < 0) {
				code = convert_datasave_code(CONVERT_DATASAVE_TO_ORG,
					DATASAVE_CD_SAVED_FILE_DELETE_ERROR);

				memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
				toPs = DATASAVE_TO_MODULE;
				SendMsg.msg = MSG_DATASAVE_MODULE_FAIL;
				SendMsg.val[0] = group;
				SendMsg.val[1] = code;
				send_msg(toPs, (char *)&SendMsg);
				break;
			}

			rtn = Open_ResultData_Formation_1(group);
			if(rtn < 0) {
				code = convert_datasave_code(CONVERT_DATASAVE_TO_ORG,
					DATASAVE_CD_SAVED_FILE_DELETE_ERROR);

				memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
				toPs = DATASAVE_TO_MODULE;
				SendMsg.msg = MSG_DATASAVE_MODULE_FAIL;
				SendMsg.val[0] = group;
				SendMsg.val[1] = code;
				send_msg(toPs, (char *)&SendMsg);
			} else {
				memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
				toPs = DATASAVE_TO_MODULE;
				SendMsg.msg = MSG_DATASAVE_MODULE_SAVED_FILE_DELETE_COMPLETE;
				SendMsg.val[0] = group;
				send_msg(toPs, (char *)&SendMsg);
			}
			break;
		default:
			userlog(DEBUG_LOG, psName,
				"Module to DataSave Msg Unknown : %d %d %d\n",
				RecvMsg->msg, RecvMsg->val[0], RecvMsg->val[1]);
			break;
	}
}

