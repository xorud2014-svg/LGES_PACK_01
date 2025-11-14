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
extern volatile S_TEMP_CLIENT *myPs;
extern char psName[PROCESS_NAME_SIZE];

void Check_Message(void)
{
	switch(myPs->misc.TempNo) {
		case 0:
			msgParsing(APP_TO_TEMP1);
			msgParsing(MODULE_TO_TEMP1);
			break;
		case 1:
			msgParsing(APP_TO_TEMP2);
			msgParsing(MODULE_TO_TEMP2);
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
		case APP_TO_TEMP1:
		case APP_TO_TEMP2:
			msgParsing_App_to_Temp(fromPs, idx, (S_MSG_VAL *)&RecvMsg);
			break;
		case MODULE_TO_TEMP1:
		case MODULE_TO_TEMP2:
			msgParsing_Module_to_Temp(fromPs, idx, (S_MSG_VAL *)&RecvMsg);
			break;
		default:
			userlog(DEBUG_LOG, psName,
				"Msg Direction UnKnown : %d %d %d\n", fromPs, idx, RecvMsg.msg);
			break;
	}
}

void msgParsing_App_to_Temp(int fromPs,  int idx, S_MSG_VAL *RecvMsg)
{
	switch(RecvMsg->msg) {
		default:
			userlog(DEBUG_LOG, psName,
				"App to Temp Msg Unknown : %d %d %d\n",
				fromPs, idx, RecvMsg->msg);
			break;
	}
}

void msgParsing_Module_to_Temp(int fromPs,  int idx, S_MSG_VAL *RecvMsg)
{
	switch(RecvMsg->msg) {
		default:
			userlog(DEBUG_LOG, psName,
				"Module to Temp Msg Unknown : %d %d %d\n",
				fromPs, idx, RecvMsg->msg);
			break;
	}
}
