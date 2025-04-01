#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../../INC/datastore.h"
#include "userlog.h"
#include "common_utils.h"
#include "local_utils.h"
#include "local_message.h"

extern volatile S_SYSTEM_DATA		*myData;
extern volatile S_UPDATE_PROCESS	*myPs;
extern char psName[PROCESS_NAME_SIZE];

void Check_Message(void)
{
	msgParsing(IO_TO_UPDATE);
}

void msgParsing(int fromPs)
{
	int idx;
	S_MSG_VAL RecvMsg;
	
	idx = rcv_msg(fromPs, (S_MSG_VAL *)&RecvMsg);
	if(idx < 0) return;
	
	switch(fromPs) {
		default:
			userlog(DEBUG_LOG, psName,
				"Msg Direction UnKnown : %d %d %d\n", fromPs, idx, RecvMsg.msg);
			break;
	}
}

void msgParsing_Module_to_Update_Process(int fromPs, int idx, S_MSG_VAL *RecvMsg)
{

	switch(RecvMsg->msg) {
		default:
			userlog(DEBUG_LOG, psName,
				"Module to Update Process Msg Unknown : %d %d %d\n",
				RecvMsg->msg, RecvMsg->val[0], RecvMsg->val[1]);
			break;
	}
}

