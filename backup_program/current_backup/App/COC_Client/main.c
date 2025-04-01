#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include "../../INC/datastore.h"
#include "userlog.h"
#include "common_utils.h"
#include "local_utils.h"
#include "local_message.h"
#include "com_io.h"
#include "com_socket.h"
#include "network.h"
#include "main.h"

volatile S_SYSTEM_DATA *myData;
volatile S_COC_CLIENT *myPs; //my process : COC_Client[?]
char psName[PROCESS_NAME_SIZE];

int main(int argc, char *argv[])
{
    int	rtn;
    struct timeval tv;
    fd_set rfds;

	if(Initialize(argc, argv) < 0) return 0;	

    while(myData->AppControl.signal[myPs->misc.psSignal] == P1) {
		myPs->signal[P3_SIG_NET_CONNECTED] = P0;
		if(myPs->signal[P3_SIG_NO_CONNECTION_RETRY] == P1) continue;
		if(InitNetwork() < 0) continue; //kjg_w shutdown process

		while(myData->AppControl.signal[myPs->misc.psSignal] == P1) {
	    	tv.tv_sec = 0;
		    tv.tv_usec = 500000;
		    FD_ZERO(&rfds);
		    if(myPs->misc.network_socket > 0)
				FD_SET(myPs->misc.network_socket, &rfds);

			rtn = select(myPs->misc.network_socket+1, &rfds, NULL, NULL, &tv);
			//userlog(DEBUG_LOG, psName, "event %d\n", rtn); //kjg_d
			if(rtn > 0) {
				if(FD_ISSET(myPs->misc.network_socket, &rfds) == 1) {
					if(NetworkPacket_Receive() < 0) {
						rtn = -1;
					} else {
						rtn = Parsing_NetworkEvent();
						if(rtn < 0) {
							StateChange_Pause(2);
							break;
						}
					}
				}

				rtn = COC_Client_Control();
				if(rtn < 0) {
					StateChange_Pause(1);
					break;
				}
			} else if(rtn == 0) {
				rtn = Parsing_NetworkEvent();
				if(rtn < 0) {
					StateChange_Pause(2);
					break;
				}

				rtn = COC_Client_Control();
				if(rtn < 0) {
					StateChange_Pause(1);
					break;
				}
			} else {}
	    }
	}

	Close_Process();
    return 0;
}

int COC_Client_Control(void)
{
	int rtn;

	Check_Message();
	Check_Signal();
	//kjg_w Check_GroupState();
	
	rtn = Check_NetworkState();
	return rtn;
}

void Check_Signal(void)
{
	//int i, j, ch;
	
	if(myPs->signal[P3_SIG_NET_CONNECTED] != P1) return;

	//send_cmd_ch_data();
	send_cmd_monitor_data();
/*
	if(myData->mData.real_time[4] != myPs->misc.sent_real_time_request) {
		j = 0;
		for(i=0; i < myPs->misc.chInGroup; i++) {
			ch = myData->CellArray1[myPs->misc.chOffset + i].number2 - 1;
			if(myData->cData[ch].op.state == C_RUN
				|| myData->cData[ch].op.state == C_PAUSE) {
				j++;
			}
		}
		if(j == 0) {
			if(myPs->signal[P3_SIG_SEND_REAL_TIME_REQUEST] == P0) {
				myPs->signal[P3_SIG_SEND_REAL_TIME_REQUEST] = P1;
			}
		}
	}

	if(myPs->signal[P3_SIG_SEND_REAL_TIME_REQUEST] == P1) {
		myPs->signal[P3_SIG_SEND_REAL_TIME_REQUEST] = P0;
		myPs->misc.sent_real_time_request = myData->mData.real_time[4]; //day
		send_cmd_real_time_request();
	}*/
/*kjg_w
	if((myData->mData.misc.timer_1sec - myPs->misc.sent_heartbeat_time)
		>= myPs->config.send_heartbeat_interval) {
		myPs->misc.sent_heartbeat_time = myData->mData.misc.timer_1sec;
		if(myPs->signal[P3_SIG_HEARTBEAT_FLAG] == P0) {
			myPs->signal[P3_SIG_HEARTBEAT_FLAG] = P1;
			rtn = send_cmd_heartbeat();
		}
	}

	if(myPs->signal[P3_SIG_SEND_ERROR_CODE] == P1) {
		myPs->signal[P3_SIG_SEND_ERROR_CODE] = P0;
		rtn = send_cmd_error_code();
	}

	if(myPs->signal[P3_SIG_METER_CONNECT_REPLY] == P1) {
		myPs->signal[P3_SIG_METER_CONNECT_REPLY] = P0;
		//send_cmd_meter_connect_reply();
		send_cmd_cali_start_reply();
	}*/
}

void Close_Process(void)
{
	if(myPs->misc.network_socket > 0) {
		close(myPs->misc.network_socket);
	}
	
	myData->AppControl.signal[myPs->misc.psSignal] = P3;

	Close_SystemMemory();
}
