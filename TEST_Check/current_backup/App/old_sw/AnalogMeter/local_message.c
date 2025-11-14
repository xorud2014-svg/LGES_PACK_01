#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../../INC/datastore.h"
#include "userlog.h"
#include "serial.h"
#include "local_message.h"

extern volatile S_SYSTEM_DATA *myData;
extern volatile S_ANALOG_METER  *myPs;
extern char psName[PROCESS_NAME_SIZE];

void Check_Message(void)
{
	msgParsing(APP_TO_METER2);
}

void msgParsing(int fromPs)
{
	int idx;
	S_MSG_VAL RecvMsg;

	idx = rcv_msg(fromPs, (S_MSG_VAL *)&RecvMsg);
	if(idx < 0) return;
	
	switch(fromPs) {
		case APP_TO_METER2:
			msgParsing_App_to_Meter2(fromPs, idx, (S_MSG_VAL *)&RecvMsg);
			break;
		default:
			userlog(DEBUG_LOG, psName,
				"Msg Direction UnKnown : %d %d %d\n", fromPs, idx, RecvMsg.msg);
			break;
	}
}

void msgParsing_App_to_Meter2(int fromPs, int idx, S_MSG_VAL *RecvMsg)
{
	int tmp, i;

	switch(RecvMsg->msg) {
		case MSG_APP_METER2_INITIALIZE:
			if(RecvMsg->val[0] == 1) {
				if(RecvMsg->val[1] == 1) {
					myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P1;
				} else if(RecvMsg->val[1] == 2) {
					myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P3;
				} else if(RecvMsg->val[1] == 3) {
					myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P5;
				} else if(RecvMsg->val[1] == 4) {
					myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P7;
				}
			} else if(RecvMsg->val[0] == 2) {
				if(RecvMsg->val[1] == 1) {
					myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P11;
				} else if(RecvMsg->val[1] == 2) {
					myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P13;
				} else if(RecvMsg->val[1] == 3) {
					myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P15;
				} else if(RecvMsg->val[1] == 4) {
					myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P17;
				}
			} else if(RecvMsg->val[0] == 3) {
				if(RecvMsg->val[1] == 1) {
					myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P21;
				} else if(RecvMsg->val[1] == 2) {
					myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P23;
				} else if(RecvMsg->val[1] == 3) {
					myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P25;
				} else if(RecvMsg->val[1] == 4) {
					myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P27;
				}
			} else if(RecvMsg->val[0] == 4) {
				if(RecvMsg->val[1] == 1) {
					myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P31;
				} else if(RecvMsg->val[1] == 2) {
					myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P33;
				} else if(RecvMsg->val[1] == 3) {
					myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P35;
				} else if(RecvMsg->val[1] == 4) {
					myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P37;
				}
			}
			break;
		case MSG_APP_METER2_REQUEST:
			send_cmd_request(RecvMsg->val[0], RecvMsg->val[1]);
			break;
		case MSG_APP_METER2_MEASURE:
			tmp = 0;
			if(myPs->config.functionType == 0) {
				 if(myPs->config.autoStart == 0) tmp = -1;
			} else {
				 if(myPs->config.autoStart2 == 0) tmp = -1;
				 else {
					for(i=0; i < myPs->config.countMeter; i++) {
						send_cmd_close(i+1);
						usleep(250000);
					}
				 }
			}
			if(tmp < 0) break;

			if(RecvMsg->val[0] == 0) {
				if(myPs->config.countMeter > 0) {
					if(RecvMsg->val[1] == 1
						&& RecvMsg->val[1] <= myPs->config.countMeter) {
						myPs->signal[ANALOG_METER_SIG_MEASURE] = P1;
					} else if(RecvMsg->val[1] == 2
						&& RecvMsg->val[1] <= myPs->config.countMeter) {
						myPs->signal[ANALOG_METER_SIG_MEASURE] = P2;
					} else if(RecvMsg->val[1] == 3
						&& RecvMsg->val[1] <= myPs->config.countMeter) {
						myPs->signal[ANALOG_METER_SIG_MEASURE] = P3;
					} else if(RecvMsg->val[1] == 4
						&& RecvMsg->val[1] <= myPs->config.countMeter) {
						myPs->signal[ANALOG_METER_SIG_MEASURE] = P4;
					} else {
						myPs->signal[ANALOG_METER_SIG_MEASURE] = P0;
					}
				} else {
					myPs->signal[ANALOG_METER_SIG_MEASURE] = P0;
				}
			} else {
				myPs->signal[ANALOG_METER_SIG_MEASURE] = P0;
			}
			break;
		default:
			userlog(DEBUG_LOG, psName,
				"App to Meter2 Msg Unknown : %d %d %d\n",
				RecvMsg->msg, RecvMsg->val[0], RecvMsg->val[1]);
			break;
	}
}

