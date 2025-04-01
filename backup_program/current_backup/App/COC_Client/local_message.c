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
extern volatile S_COC_CLIENT *myPs;
extern char psName[PROCESS_NAME_SIZE];

void Check_Message(void)
{
	switch(myPs->config.groupNo) {
		case 0:
			msgParsing(APP_TO_COC1);
			msgParsing(MODULE_TO_COC1);
			msgParsing(DATASAVE_TO_COC1);
			msgParsing(METER_TO_COC1);
			break;
		case 1:
			msgParsing(APP_TO_COC2);
			msgParsing(MODULE_TO_COC2);
			msgParsing(DATASAVE_TO_COC2);
			msgParsing(METER_TO_COC2);
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
		case APP_TO_COC1:
		case APP_TO_COC2:
			msgParsing_App_to_COC(fromPs, idx, (S_MSG_VAL *)&RecvMsg);
			break;
		case MODULE_TO_COC1:
		case MODULE_TO_COC2:
			msgParsing_Module_to_COC(fromPs, idx, (S_MSG_VAL *)&RecvMsg);
			break;
		case DATASAVE_TO_COC1:
		case DATASAVE_TO_COC2:
			msgParsing_DataSave_to_COC(fromPs, idx, (S_MSG_VAL *)&RecvMsg);
			break;
		case METER_TO_COC1:
		case METER_TO_COC2:
			msgParsing_Meter_to_COC(fromPs, idx, (S_MSG_VAL *)&RecvMsg);
			break;
		default:
			userlog(DEBUG_LOG, psName,
				"Msg Direction UnKnown : %d %d %d\n", fromPs, idx, RecvMsg.msg);
			break;
	}
}

void msgParsing_App_to_COC(int fromPs,  int idx, S_MSG_VAL *RecvMsg)
{
	switch(RecvMsg->msg) {
		case MSG_APP_COC_REAL_TIME_REQUEST:
			myPs->signal[P3_SIG_SEND_REAL_TIME_REQUEST] = P1;
			break;
		default:
			userlog(DEBUG_LOG, psName,
				"App to COC Msg Unknown : %d %d %d\n",
				fromPs, idx, RecvMsg->msg);
			break;
	}
}

void msgParsing_Module_to_COC(int fromPs,  int idx, S_MSG_VAL *RecvMsg)
{
	int code;

	switch(RecvMsg->msg) {
		case MSG_MODULE_COC_TROUBLE_CODE:
			code = convert_group_trouble_code(CONVERT_ORG_TO_P3,
				RecvMsg->val[0]);
			if(code > 0) send_cmd_trouble_code(code, RecvMsg->val[1]);
			break;
		case MSG_MODULE_COC_BMS_COMM_END:
			send_cmd_bms_comm_end(RecvMsg->val[0], RecvMsg->val[1]);
			break;
		case MSG_MODULE_COC_CABLE_CHECK_REPLY:
			send_cmd_cable_check_reply();
			break;
		case MSG_MODULE_COC_CELL_CHECK_REPLY:
			send_cmd_cell_check_reply();
			break;
		default:
			userlog(DEBUG_LOG, psName,
				"Module to COC Msg Unknown : %d %d %d\n",
				fromPs, idx, RecvMsg->msg);
			break;
	}
}

void msgParsing_DataSave_to_COC(int fromPs, int idx, S_MSG_VAL *RecvMsg)
{
	switch(RecvMsg->msg) {
		case MSG_DATASAVE_COC_CALI_NORMAL_RESULT_SEND:
			send_cmd_cali_normal_result(RecvMsg->val[0]);
			break;
		case MSG_DATASAVE_COC_CALI_CHECK_RESULT_SEND:
			send_cmd_cali_check_result(RecvMsg->val[0]);
			break;
		default:
			userlog(DEBUG_LOG, psName,
				"DataSave to COC Msg Unknown : %d %d %d\n",
				fromPs, idx, RecvMsg->msg);
			break;
	}
}

void msgParsing_Meter_to_COC(int fromPs, int idx, S_MSG_VAL *RecvMsg)
{
	switch(RecvMsg->msg) {
		case MSG_METER_COC_INITIALIZE_REPLY:
			myPs->signal[P3_SIG_METER_CONNECT_REPLY] = P1;
			break;
		default:
			userlog(DEBUG_LOG, psName,
				"Meter to COC Msg Unknown : %d %d %d\n",
				fromPs, idx, RecvMsg->msg);
			break;
	}
}
