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
extern volatile S_SIL_CLIENT *myPs;
extern char psName[PROCESS_NAME_SIZE];

void Check_Message(void)
{
	switch(myPs->misc.SilNo) {
		case 0:
			msgParsing(APP_TO_SIL);
			msgParsing(MODULE_TO_SIL);
			msgParsing(COA1_TO_SIL);
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
		case APP_TO_SIL:
			msgParsing_App_to_Sil(fromPs, idx, (S_MSG_VAL *)&RecvMsg);
			break;
		case MODULE_TO_SIL:
			msgParsing_Module_to_Sil(fromPs, idx, (S_MSG_VAL *)&RecvMsg);
			break;
		case COA1_TO_SIL:
			msgParsing_Coa1_to_Sil(fromPs, idx, (S_MSG_VAL *)&RecvMsg);
			break;
		default:
			userlog(DEBUG_LOG, psName,
				"Msg Direction UnKnown : %d %d %d\n", fromPs, idx, RecvMsg.msg);
			break;
	}
}

void msgParsing_App_to_Sil(int fromPs,  int idx, S_MSG_VAL *RecvMsg)
{
	switch(RecvMsg->msg) {
		default:
			userlog(DEBUG_LOG, psName,
				"App to Sil Msg Unknown : %d %d %d\n",
				fromPs, idx, RecvMsg->msg);
			break;
	}
}

void msgParsing_Module_to_Sil(int fromPs,  int idx, S_MSG_VAL *RecvMsg)
{
	switch(RecvMsg->msg) {
		case MSG_MODULE_SIL_DAQ_STOP_REQUEST:
			send_cmd_daq_stop(RecvMsg->val[1]);
			userlog(DEBUG_LOG, psName,"send_cmd_daq_stop : %x\n",RecvMsg->msg);
			break;
		default:
			userlog(DEBUG_LOG, psName,
				"Module to Sil Msg Unknown : %d %d %d\n",
				fromPs, idx, RecvMsg->msg);
			break;
	}
}
void msgParsing_Coa1_to_Sil(int fromPs,  int idx, S_MSG_VAL *RecvMsg)
{
	switch(RecvMsg->msg) {
		case MSG_COA1_SIL_DAQ_START_REQUEST:
			send_cmd_daq_start(RecvMsg->val[1]);
			userlog(DEBUG_LOG, psName,"send_cmd_daq_start : %x\n",RecvMsg->msg);
			break;
		default:
			userlog(DEBUG_LOG, psName,
				"Module to Sil Msg Unknown : %d %d %d\n",
				fromPs, idx, RecvMsg->msg);
			break;
	}
}
