#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../../INC/datastore.h"
#include "userlog.h"
#include "common_utils.h"
#include "common_message.h"
#include "network.h"
#include "local_message.h"

extern volatile S_SYSTEM_DATA *myData;
extern volatile S_COB_CLIENT *myPs;
extern char psName[PROCESS_NAME_SIZE];

void Check_Message(void)
{
	switch(myPs->config.groupNo) {
		case 0:
			msgParsing(APP_TO_COB1);
			msgParsing(MODULE_TO_COB1);
			msgParsing(DATASAVE_TO_COB1);
			msgParsing(METER_TO_COB1);
			msgParsing(JIGC_TO_COB1);
			break;
		case 1:
			msgParsing(APP_TO_COB2);
			msgParsing(MODULE_TO_COB2);
			msgParsing(DATASAVE_TO_COB2);
			msgParsing(METER_TO_COB2);
			msgParsing(JIGC_TO_COB2);
			break;
		default: break;
	}
}

void msgParsing(int fromPs)
{
	int idx;
	S_MSG_VAL RecvMsg;
	
	idx = rcv_msg(fromPs, (S_MSG_VAL *)&RecvMsg);
	if(idx < 0) return;
	
	switch(fromPs) {
		case APP_TO_COB1:
		case APP_TO_COB2:
			msgParsing_App_to_COB(fromPs, idx, (S_MSG_VAL *)&RecvMsg);
			break;
		case MODULE_TO_COB1:
		case MODULE_TO_COB2:
			msgParsing_Module_to_COB(fromPs, idx, (S_MSG_VAL *)&RecvMsg);
			break;
		case DATASAVE_TO_COB1:
		case DATASAVE_TO_COB2:
			msgParsing_DataSave_to_COB(fromPs, idx, (S_MSG_VAL *)&RecvMsg);
			break;
		case METER_TO_COB1:
		case METER_TO_COB2:
			msgParsing_Meter_to_COB(fromPs, idx, (S_MSG_VAL *)&RecvMsg);
			break;
		case JIGC_TO_COB1:
		case JIGC_TO_COB2:
			msgParsing_JigC_to_COB(fromPs, idx, (S_MSG_VAL *)&RecvMsg);
			break;
		default:
			userlog(DEBUG_LOG, psName,
				"Msg Direction UnKnown : %d %d %d\n", fromPs, idx, RecvMsg.msg);
			break;
	}
}

void msgParsing_App_to_COB(int fromPs,  int idx, S_MSG_VAL *RecvMsg)
{
	int group;
	S_MSG_VAL msg_val;
	group = myPs->config.groupNo;

	switch(RecvMsg->msg) {
		case MSG_APP_COB_REAL_TIME_REQUEST:
			//kjgw myPs->signal[P2_SIG_SEND_REAL_TIME_REQUEST] = P1;
			break;
		case MSG_APP_COB_TEST_FUNCTION:
			send_cmd_switch_data(RecvMsg->val[0]);
			break;
		case MSG_APP_COB_PLC_DOOR_CLOSE:
			msg_val.msg = MSG_COB_MODULE_PLC_DOOR_CLOSE;
			send_msg(COB1_TO_MODULE+group,(char *)&msg_val);
			break;
		case MSG_APP_COB_PLC_DOOR_OPEN:
			msg_val.msg = MSG_COB_MODULE_PLC_DOOR_OPEN;
			send_msg(COB1_TO_MODULE+group,(char *)&msg_val);
			break;
		case MSG_APP_COB_PLC_FAULT_READ:
			msg_val.msg = MSG_COB_MODULE_PLC_FAULT_READ;
			send_msg(COB1_TO_MODULE+group,(char *)&msg_val);
			break;
		case MSG_APP_COB_JOB_CHANGE:
			msg_val.msg = MSG_COB_JIGC_JOB_CHANGE;
			msg_val.val[0] = RecvMsg->val[0];
			send_msg(COB1_TO_JIGC1+group,(char *)&msg_val);
			break;
		case MSG_APP_COB_PITCH_CHANGE:
			msg_val.msg = MSG_COB_JIGC_PITCH_CHANGE;
			msg_val.val[0] = RecvMsg->val[0];
			send_msg(COB1_TO_JIGC1+group,(char *)&msg_val);
			break;
		case MSG_APP_COB_TRY_CONTACT:
			msg_val.msg = MSG_COB_JIGC_TRY_CONTACT;
			send_msg(COB1_TO_JIGC1+group,(char *)&msg_val);
			break;
		case MSG_APP_COB_TRY_DISCONTACT:
			msg_val.msg = MSG_COB_JIGC_TRY_DISCONTACT;
			send_msg(COB1_TO_JIGC1+group,(char *)&msg_val);
			break;
		default:
			userlog(DEBUG_LOG, psName, "App to COB Msg Unknown : %d %d %d\n",
				fromPs, idx, RecvMsg->msg);
			break;
	}
}

void msgParsing_Module_to_COB(int fromPs,  int idx, S_MSG_VAL *RecvMsg)
{
	int rtn, code, group;

	switch(RecvMsg->msg) {
		case MSG_MODULE_COB_TROUBLE_CODE:
			if(myPs->signal[P2_SIG_NET_CONNECTED] != P1) break;

			code = convert_group_trouble_code(CONVERT_ORG_TO_P2,
				RecvMsg->val[0]);
			if(code > 0) rtn = send_cmd_trouble_code(code, RecvMsg->val[1]);
			break;
		case MSG_MODULE_COB_START:
		case MSG_MODULE_COB_STOP:
		case MSG_MODULE_COB_CONTINUE:
		case MSG_MODULE_COB_RESET:
		case MSG_MODULE_COB_PAUSE:
			if(myPs->signal[P2_SIG_NET_CONNECTED] != P1) break;

			rtn = send_cmd_user_cmd(RecvMsg->msg);
			break;
		case MSG_MODULE_COB_CONTROL_MODE:
			myPs->config.groupControlMode = RecvMsg->val[2];
			break;
		case MSG_MODULE_COB_RESET_COMPLETE:
			myPs->signal[P2_SIG_SEND_GROUP_STATE] = P1;
			break;
		case MSG_MODULE_COB_SEND_GROUP_STATE:
			myPs->signal[P2_SIG_SEND_GROUP_STATE] = P1;
			break;
		case MSG_MODULE_COB_TRAY_ID:
			if(myPs->signal[P2_SIG_NET_CONNECTED] != P1) break;

			group = RecvMsg->val[0];
			rtn = send_cmd_bcr_info(group);
			if(rtn < 0) {
				userlog(DEBUG_LOG, psName, "Tray_Id Send fail(to COB)\n");
			}
			break;
		case MSG_MODULE_COB_JIG_FUNCTION:
			send_cmd_switch_data(RecvMsg->val[0]);
			break;
		//kjgw_110719
		case MSG_MODULE_COB_CALI_DATA:
			rtn = send_cmd_cali_data(RecvMsg->val[0], RecvMsg->val[1]);
			break;
		case MSG_MODULE_COB_CALI_END:
			rtn = send_cmd_cali_end(RecvMsg->val[0], RecvMsg->val[1]);
			break;
		case MSG_MODULE_COB_CALI_CHECK_DATA:
			rtn = send_cmd_cali_check_data(RecvMsg->val[0], RecvMsg->val[1]);
			break;
		case MSG_MODULE_COB_CALI_CHECK_END:
			rtn = send_cmd_cali_check_end(RecvMsg->val[0], RecvMsg->val[1]);
			break;
		case MSG_MODULE_COB_REAL_MEASURE_DATA:
			rtn = send_cmd_real_measure_data(RecvMsg->val[0], RecvMsg->val[1]);
			break;
		case MSG_MODULE_COB_REAL_MEASURE_END:
			rtn = send_cmd_real_measure_end(RecvMsg->val[0], RecvMsg->val[1]);
			break;
		default:
			userlog(DEBUG_LOG, psName, "Module to COB Msg Unknown : %d %d %d\n",
				fromPs, idx, RecvMsg->msg);
			break;
	}
}

void msgParsing_DataSave_to_COB(int fromPs, int idx, S_MSG_VAL *RecvMsg)
{
	switch(RecvMsg->msg) {
/*		case MSG_DATASAVE_COB_CALI_NORMAL_RESULT_SEND:
			send_cmd_cali_normal_result(RecvMsg->val[0]);
			break;
		case MSG_DATASAVE_COB_CALI_CHECK_RESULT_SEND:
			send_cmd_cali_check_result(RecvMsg->val[0]);
			break;*/
		default:
			userlog(DEBUG_LOG, psName,
				"DataSave to COB Msg Unknown : %d %d %d\n",
				fromPs, idx, RecvMsg->msg);
			break;
	}
}

void msgParsing_Meter_to_COB(int fromPs, int idx, S_MSG_VAL *RecvMsg)
{
	switch(RecvMsg->msg) {
		case MSG_METER_COB_INITIALIZE_REPLY:
			myPs->signal[P2_SIG_METER_CONNECT_REPLY] = P1;
			break;
		default:
			userlog(DEBUG_LOG, psName, "Meter to COB Msg Unknown : %d %d %d\n",
				fromPs, idx, RecvMsg->msg);
			break;
	}
}

void msgParsing_JigC_to_COB(int fromPs, int idx, S_MSG_VAL *RecvMsg)
{
	switch(RecvMsg->msg) {
		case MSG_JIGC_COB_JOB_CHANGE_END:
			myPs->misc.jobModel = (unsigned char)RecvMsg->val[0];
			break;
		default:
			userlog(DEBUG_LOG, psName, "JigC to COB Msg Unknown : %d %d %d\n",
				fromPs, idx, RecvMsg->msg);
			break;
	}
}
