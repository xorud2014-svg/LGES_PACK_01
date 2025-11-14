#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../../INC/datastore.h"
#include "userlog.h"
#include "local_utils.h"
#include "local_message.h"

extern volatile S_SYSTEM_DATA	*myData;
extern volatile S_TIMESCH_CONTROL	*myPs;
extern char psName[PROCESS_NAME_SIZE];

void Check_Message(void)
{
	msgParsing(MODULE_TO_TIMESCH);
}

void msgParsing(int fromPs)
{
	int idx;
	S_MSG_VAL RecvMsg;
	
	idx = rcv_msg(fromPs, (S_MSG_VAL *)&RecvMsg);
	if(idx < 0) return;
	
	userlog(DEBUG_LOG, psName, "kjhd message : %d %d %d\n",
		fromPs, idx, RecvMsg.msg);

	switch(fromPs) {
		case MODULE_TO_TIMESCH:
			msgParsing_Module_to_TimeSch(fromPs, idx, (S_MSG_VAL *)&RecvMsg);
			break;
		default:
			userlog(DEBUG_LOG, psName,
				"Msg Direction UnKnown : %d %d %d\n", fromPs, idx, RecvMsg.msg);
			break;
	}
}

void msgParsing_Module_to_TimeSch(int fromPs, int idx, S_MSG_VAL *RecvMsg)
{
	int ch, step, init;

	switch(RecvMsg->msg) {
		case MSG_MODULE_TIMESCH_INITIALIZE:
			ch = RecvMsg->val[0];
			step = RecvMsg->val[1];
			init = RecvMsg->val[2];

			if(init == 0) { //no init
			} else { //init
				if(myPs->signal[TIMESCH_SIG_TIMESCH_INITIALIZE_1 + ch] == P0) {
					myPs->signal[TIMESCH_SIG_TIMESCH_INITIALIZE_1 + ch] = P1;
				}
			}
			userlog(DEBUG_LOG, psName,
				"timesch_initialize ch: %d, step: %d, init: %d \n",
				ch, step, init);
			break;
		default:
			userlog(DEBUG_LOG, psName, "Module to TimeSch Msg Unknown : %d\n",
				RecvMsg->msg);
			break;
	}
}

