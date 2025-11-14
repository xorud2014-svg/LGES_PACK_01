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
extern volatile S_CAN_CLIENT *myPs;
extern char psName[PROCESS_NAME_SIZE];

void Check_Message(void)
{
	switch(myPs->misc.CanNo) {
		case 0:
			msgParsing(APP_TO_CAN);
			msgParsing(MODULE_TO_CAN);
			msgParsing(COA1_TO_CAN);
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
		case APP_TO_CAN:
			msgParsing_App_to_Can(fromPs, idx, (S_MSG_VAL *)&RecvMsg);
			break;
		case MODULE_TO_CAN:
			msgParsing_Module_to_Can(fromPs, idx, (S_MSG_VAL *)&RecvMsg);
			break;
		case COA1_TO_CAN:
			msgParsing_Coa1_to_Can(fromPs, idx, (S_MSG_VAL *)&RecvMsg);
			break;
		default:
			userlog(DEBUG_LOG, psName,
				"Msg Direction UnKnown : %d %d %d\n", fromPs, idx, RecvMsg.msg);
			break;
	}
}

void msgParsing_App_to_Can(int fromPs,  int idx, S_MSG_VAL *RecvMsg)
{
	switch(RecvMsg->msg) {
		case MSG_APP_CAN_RECEIVE_SET:
			send_cmd_can_receive_set(RecvMsg->val[0]);
			break;
		case MSG_APP_CAN_TRANSMIT_SET:
			send_cmd_can_transmit_set(RecvMsg->val[0]);
			break;
		default:
			userlog(DEBUG_LOG, psName, "App to Can Msg Unknown : %d %d %d\n",
				fromPs, idx, RecvMsg->msg);
			break;
	}
}

void msgParsing_Module_to_Can(int fromPs,  int idx, S_MSG_VAL *RecvMsg)
{
	switch(RecvMsg->msg) {
		default:
			userlog(DEBUG_LOG, psName, "Module to Can Msg Unknown : %d %d %d\n",
				fromPs, idx, RecvMsg->msg);
			break;
	}
}
void msgParsing_Coa1_to_Can(int fromPs,  int idx, S_MSG_VAL *RecvMsg)
{
	switch(RecvMsg->msg) {
		default:
			userlog(DEBUG_LOG, psName, "Module to Can Msg Unknown : %d %d %d\n",
				fromPs, idx, RecvMsg->msg);
			break;
	}
}
