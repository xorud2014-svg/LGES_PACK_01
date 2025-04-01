#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../../INC/datastore.h"
#include "userlog.h"
#include "serial.h"
#include "local_message.h"

extern volatile S_SYSTEM_DATA *myData;
extern volatile S_CALI_METER  *myPs;
extern char psName[PROCESS_NAME_SIZE];

void Check_Message(void)
{
	msgParsing(APP_TO_METER);
	msgParsing(COA1_TO_METER);
	msgParsing(COA2_TO_METER);
	msgParsing(MODULE_TO_METER);
}

void msgParsing(int fromPs)
{
	int idx;
	S_MSG_VAL RecvMsg;
	
	idx = rcv_msg(fromPs, (S_MSG_VAL *)&RecvMsg);
	if(idx < 0) return;

	switch(fromPs) {
		case APP_TO_METER:
			msgParsing_App_to_Meter(fromPs, idx, (S_MSG_VAL *)&RecvMsg);
			break;
		case COA1_TO_METER:
		case COA2_TO_METER:
			msgParsing_COA_to_Meter(fromPs, idx, (S_MSG_VAL *)&RecvMsg);
			break;
		case MODULE_TO_METER:
			msgParsing_Module_to_Meter(fromPs, idx, (S_MSG_VAL *)&RecvMsg);
			break;
		default:
			userlog(DEBUG_LOG, psName,
				"Msg Direction UnKnown : %d %d %d\n", fromPs, idx, RecvMsg.msg);
			break;
	}
}

void msgParsing_App_to_Meter(int fromPs, int idx, S_MSG_VAL *RecvMsg)
{
	int tmp;

	switch(RecvMsg->msg) {
		case MSG_APP_METER_INITIALIZE:
			if(RecvMsg->val[0] == CALI_TYPE_VOLTAGE) {
				myPs->signal[CALI_METER_SIG_INITIALIZE] = P1;
			} else { //CALI_TYPE_CURRENT
				myPs->signal[CALI_METER_SIG_INITIALIZE] = P11;
			}
			break;
		case MSG_APP_METER_REQUEST:
			send_cmd_request();
			break;
		case MSG_APP_METER_TEST:
			send_cmd_display_value(0, RecvMsg->val[0], 1);
			break;
		case MSG_APP_METER_DISPLAY_START:
			tmp = 0;
			if(myPs->config.functionType == 0) {
				tmp = -1;
			} else {
				if(myPs->config.autoStart2 == 0) tmp = -1;
			}
			if(tmp < 0) break;

			myPs->signal[CALI_METER_SIG_DISPLAY] = P1;
			break;
		default:
			userlog(DEBUG_LOG, psName,
				"App to Meter Msg Unknown : %d %d %d\n",
				RecvMsg->msg, RecvMsg->val[0], RecvMsg->val[1]);
			break;
	}
}

void msgParsing_COA_to_Meter(int fromPs, int idx, S_MSG_VAL *RecvMsg)
{
	switch(RecvMsg->msg) {
		case MSG_COA_METER_INITIALIZE:
			if(RecvMsg->val[0] == CALI_TYPE_VOLTAGE) {
				myPs->signal[CALI_METER_SIG_INITIALIZE] = P21;
			} else { //CALI_TYPE_CURRENT
				myPs->signal[CALI_METER_SIG_INITIALIZE] = P31;
			}
			break;
		default:
			userlog(DEBUG_LOG, psName,
				"COA to Meter Msg Unknown : %d %d %d\n",
				RecvMsg->msg, RecvMsg->val[0], RecvMsg->val[1]);
			break;
	}
}

void msgParsing_Module_to_Meter(int fromPs, int idx, S_MSG_VAL *RecvMsg)
{
	switch(RecvMsg->msg) {
		case MSG_MODULE_METER_REQUEST:
			myPs->receivedCh = RecvMsg->val[0];
			myPs->signal[CALI_METER_SIG_REQUEST_PHASE] = P1;
			break;
		default:
			userlog(DEBUG_LOG, psName,
				"Module to Meter Msg Unknown : %d %d %d\n",
				RecvMsg->msg, RecvMsg->val[0], RecvMsg->val[1]);
			break;
	}
}

