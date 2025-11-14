#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <stdlib.h>
#include <string.h>
#include "../../INC/datastore.h"
#include "userlog.h"
#include "local_utils.h"
#include "local_message.h"
#include "com_io.h"
#include "comm.h"
#include "serial.h"
#include "main.h"

volatile S_SYSTEM_DATA *myData;
volatile S_CALI_METER  *myPs; //my process : CaliMeter
char	psName[PROCESS_NAME_SIZE];

int main(void)
{
	int rtn;
	long scan_period1, scan_period2;
	struct timeval tv;
	fd_set rfds;
	
	if(Initialize() < 0) return 0;

	if(myPs->config.functionType == 0) {
		scan_period1 = 0;
		scan_period2 = 230000;
	} else {
		scan_period1 = 0;
		scan_period2 = 120000;
	}
   	
	while(myData->AppControl.signal[APP_SIG_CALI_METER_PROCESS] == P1) {
		if(InitComPort() < 0) continue; //kjgw shutdown process

		while(myData->AppControl.signal[APP_SIG_CALI_METER_PROCESS] == P1) {
			tv.tv_sec = scan_period1;
			tv.tv_usec = scan_period2;
			FD_ZERO(&rfds);
			if(myPs->misc.ttyS_fd > 0) FD_SET(myPs->misc.ttyS_fd, &rfds);
		
			rtn = select(myPs->misc.ttyS_fd+1, &rfds, NULL, NULL, &tv);
			//userlog(DEBUG_LOG, psName, "event %d\n", rtn); //kjgd
			if(rtn > 0) {
				if(FD_ISSET(myPs->misc.ttyS_fd, &rfds) == 1) {
					if(SerialPacket_Receive() < 0) {
						rtn = -1;
						if(rtn < 0) break;
					} else {
						rtn = Parsing_SerialEvent();
						if(rtn < 0) break;
					}
				}
			} else if(rtn == 0) {
				rtn = CaliMeter_Control();
				if(rtn < 0) break;

				rtn = Parsing_SerialEvent();
				if(rtn < 0) break;
			} else {}
		}
	}

	Close_Process();
	return 0;
}

int CaliMeter_Control(void)
{
	int rtn;

	Check_Message();
	Check_Signal();

	rtn = Check_ComPortState();
	return rtn;
}

void Check_Signal(void)
{
	S_MSG_VAL SendMsg;

	switch(myPs->signal[CALI_METER_SIG_INITIALIZE]) {
		case P1:
			send_cmd_initialize(1);
			myPs->signal[CALI_METER_SIG_INITIALIZE]++;
			break;
		case P2:
			send_cmd_initialize(2);
			myPs->signal[CALI_METER_SIG_INITIALIZE]++;
			break;
		case P3:
			send_cmd_initialize(3);
			myPs->signal[CALI_METER_SIG_INITIALIZE]++;
			break;
		case P4:
			send_cmd_initialize(4);
			myPs->signal[CALI_METER_SIG_INITIALIZE] = P0;
			break;
		case P11:
			send_cmd_initialize(1);
			myPs->signal[CALI_METER_SIG_INITIALIZE]++;
			break;
		case P12:
			send_cmd_initialize(2);
			myPs->signal[CALI_METER_SIG_INITIALIZE]++;
			break;
		case P13:
			if(myPs->config.readType == READ_V_V) {
				send_cmd_initialize(3);
			} else if(myPs->config.readType == READ_V_I) {
				send_cmd_initialize(5);
			}
			myPs->signal[CALI_METER_SIG_INITIALIZE]++;
			break;
		case P14:
			send_cmd_initialize(4);
			myPs->signal[CALI_METER_SIG_INITIALIZE] = P0;
			break;
		case P21:
			send_cmd_initialize(1);
			myPs->signal[CALI_METER_SIG_INITIALIZE]++;
			break;
		case P22:
			send_cmd_initialize(2);
			myPs->signal[CALI_METER_SIG_INITIALIZE]++;
			break;
		case P23:
			send_cmd_initialize(3);
			myPs->signal[CALI_METER_SIG_INITIALIZE]++;
			break;
		case P24:
			send_cmd_initialize(4);

			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
			SendMsg.msg = MSG_METER_COA_INITIALIZE_REPLY;
			SendMsg.val[0] = 0;
			send_msg(METER_TO_COA1, (char *)&SendMsg); //kjgw

			myPs->signal[CALI_METER_SIG_INITIALIZE] = P0;
			break;
		case P31:
			send_cmd_initialize(1);
			myPs->signal[CALI_METER_SIG_INITIALIZE]++;
			break;
		case P32:
			send_cmd_initialize(2);
			myPs->signal[CALI_METER_SIG_INITIALIZE]++;
			break;
		case P33:
			if(myPs->config.readType == READ_V_V) {
				send_cmd_initialize(3);
			} else if(myPs->config.readType == READ_V_I) {
				send_cmd_initialize(5);
			}
			myPs->signal[CALI_METER_SIG_INITIALIZE]++;
			break;
		case P34:
			send_cmd_initialize(4);

			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
			SendMsg.msg = MSG_METER_COA_INITIALIZE_REPLY;
			SendMsg.val[0] = 0;
			send_msg(METER_TO_COA1, (char *)&SendMsg); //kjgw

			myPs->signal[CALI_METER_SIG_INITIALIZE] = P0;
			break;
		default:	break;
	}

	if(myPs->signal[CALI_METER_SIG_REQUEST_PHASE] == P1) {
		send_cmd_request();
		myPs->signal[CALI_METER_SIG_REQUEST_PHASE] = P2;
	} else if(myPs->signal[CALI_METER_SIG_REQUEST_PHASE] == P3) {
		memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
		SendMsg.msg = MSG_METER_MODULE_REQUEST_REPLY;
		SendMsg.val[0] = myPs->receivedCh;
		send_msg(METER_TO_MODULE, (char *)&SendMsg);
		myPs->signal[CALI_METER_SIG_REQUEST_PHASE] = P0;
	}

	switch(myPs->signal[CALI_METER_SIG_DISPLAY]) {
		case P1:
			myPs->signal[CALI_METER_SIG_DISPLAY_ERROR] = 0;
			myPs->signal[CALI_METER_SIG_DISPLAY] = P11;
			send_cmd_display_value(0, 1, 0);
			send_cmd_display_value(0, 2, 0);
			break;
		case P2:
			myPs->signal[CALI_METER_SIG_DISPLAY_ERROR] = 0;
			myPs->signal[CALI_METER_SIG_DISPLAY] = P12;
			send_cmd_display_value(0, 0, 0);
			send_cmd_display_value(0, 3, 0);
			break;
		case P3:
			myPs->signal[CALI_METER_SIG_DISPLAY_ERROR] = 0;
			myPs->signal[CALI_METER_SIG_DISPLAY] = P13;

			if(myPs->config.functionType == 1) {
				send_cmd_display_value(0, 11, 0);
				send_cmd_display_value(0, 12, 0);
			} else if(myPs->config.functionType == 2) {
				send_cmd_display_value(1, 1, 0);
				send_cmd_display_value(1, 2, 0);
			}
			break;
		case P4:
			myPs->signal[CALI_METER_SIG_DISPLAY_ERROR] = 0;
			myPs->signal[CALI_METER_SIG_DISPLAY] = P14;

			if(myPs->config.functionType == 1) {
				send_cmd_display_value(0, 10, 0);
				send_cmd_display_value(0, 13, 0);
			} else if(myPs->config.functionType == 2) {
				send_cmd_display_value(1, 0, 0);
				send_cmd_display_value(1, 3, 0);
			}
			break;
		case P11:
		case P12:
		case P13:
		case P14:
			myPs->signal[CALI_METER_SIG_DISPLAY_ERROR] += 1;
			if(myPs->signal[CALI_METER_SIG_DISPLAY_ERROR] >= 3) {
				userlog(DEBUG_LOG, psName, "display error : %d, %d\n",
					(int)myPs->signal[CALI_METER_SIG_DISPLAY],
					(int)myPs->signal[CALI_METER_SIG_DISPLAY_ERROR]);
				//memset((char *)&myPs->rcvPacket, 0,
				//	sizeof(S_CALI_METER_RCV_PACKET));
				//memset((char *)&myPs->rcvCmd, 0,
				//	sizeof(S_CALI_METER_RCV_COMMAND));
				myPs->signal[CALI_METER_SIG_DISPLAY_ERROR] = 0;
				if(myPs->signal[CALI_METER_SIG_DISPLAY] == P14) {
					myPs->signal[CALI_METER_SIG_DISPLAY] = P1;
				} else {
					myPs->signal[CALI_METER_SIG_DISPLAY] -= P9;
				}
			}
			break;
		default:	break;
	}
}

void Close_Process(void)
{
	if(myPs->misc.ttyS_fd > 0) {
		if(closetty(myPs->misc.ttyS_fd) < 0) {
			userlog(DEBUG_LOG, psName, "ttyS %d close error\n",
				myPs->misc.ttyS_fd);
		}
	}
	
	myData->AppControl.signal[APP_SIG_CALI_METER_PROCESS] = P3;	

	Close_SystemMemory();
}
