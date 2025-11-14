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
volatile S_COA_CLIENT *myPs; //my process : COA_Client[?]
volatile S_TEST_CONDITION *myTestCond; //kjg_170810
char psName[PROCESS_NAME_SIZE];

int main(int argc, char *argv[])
{
    int	rtn;
    struct timeval tv;
    fd_set rfds;

	if(Initialize(argc, argv) < 0) return 0;	

    while(myData->AppControl.signal[myPs->misc.psSignal] == P1) {
		myPs->signal[P1_SIG_NET_CONNECTED] = P0;
		if(myPs->signal[P1_SIG_NO_CONNECTION_RETRY] == P1) continue;
		if(InitNetwork() < 0) continue; //kjg_w shutdown process

		while(myData->AppControl.signal[myPs->misc.psSignal] == P1) {
	    	tv.tv_sec = 0;
		    tv.tv_usec = 500000;
		    FD_ZERO(&rfds);
		    if(myPs->misc.network_socket1 > 0)
				FD_SET(myPs->misc.network_socket1, &rfds);

			rtn = select(myPs->misc.network_socket1+1, &rfds, NULL, NULL,
				&tv);
			//userlog(DEBUG_LOG, psName, "event %d\n", rtn); //kjg_d
			if(rtn > 0) {
				if(FD_ISSET(myPs->misc.network_socket1, &rfds) == 1) {
					if(NetworkPacket_Receive() < 0) {
						rtn = -1;
					} else {
						rtn = Parsing_NetworkEvent();
						if(rtn < 0) {
							//StateChange_Pause(2);
							break;
						}
					}
				}

				rtn = COA_Client_Control();
				if(rtn < 0) {
					//StateChange_Pause(1);
					break;
				}
			} else if(rtn == 0) {
				rtn = Parsing_NetworkEvent();
				if(rtn < 0) {
					//StateChange_Pause(2);
					break;
				}

				rtn = COA_Client_Control();
				if(rtn < 0) {
					//StateChange_Pause(1);
					break;
				}
			} else {}
	    }
	}

	Close_Process();
    return 0;
}

int COA_Client_Control(void)
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
	unsigned char msg, error_point; //jhkw_190830
	int i, j, ch;
	
	if(myPs->signal[P1_SIG_NET_CONNECTED] != P1) return;

	send_cmd_ch_data2();
	//jhkw_190830s
	//send_save_data(200);
	error_point = msg = 0;
#ifdef __10MS__
	if(myData->mData.signal[M_SIG_DATA_SAVE_ERROR] == P1) {
		if(myData->mData.signal[M_SIG_DATA_SAVE_ERROR_PHASE] >= 10) {
			myData->mData.signal[M_SIG_DATA_SAVE_ERROR_PHASE] = 0;
			for(i=0; i < myPs->misc.chInGroup; i++) {
				ch = myData->CellArray1[myPs->misc.chOffset + i].number2 - 1;
				msg = myData->signal_10ms[ch].response_msg_count;
				msg++;
				if((msg >= MAX_10MS_SAVE_MSG_RING) 
					|| (myData->signal_10ms[ch].save_start_flag == 1)) {
					msg = 0;
				}
				if((myData->cData[ch].op.select == SAVE_FLAG_MONITORING_DATA)
				//	|| (myData->cData[ch].op.select == SAVE_FLAG_SAVING_END)
				//	|| (myData->cData[ch].op.select == SAVE_FLAG_SAVING_ETC)
					|| (myData->save_msg_10ms[ch][msg].count_100 == 100)) {
					myData->save_msg_10ms[ch][msg].read_idx = 0;
					send_save_data_2(200, ch, msg);
				}
			}
		} else {
			myData->mData.signal[M_SIG_DATA_SAVE_ERROR_PHASE]++;
		}
	} else {
		for(i=0; i < myPs->misc.chInGroup; i++) {
			ch = myData->CellArray1[myPs->misc.chOffset + i].number2 - 1;
			msg = myData->signal_10ms[ch].response_msg_count;
			msg++;
			if((msg >= MAX_10MS_SAVE_MSG_RING) 
				|| (myData->signal_10ms[ch].save_start_flag == 1)) {
				msg = 0;
			}
			if((myData->cData[ch].op.select == SAVE_FLAG_MONITORING_DATA)
			//	|| (myData->cData[ch].op.select == SAVE_FLAG_SAVING_END)
			//	|| (myData->cData[ch].op.select == SAVE_FLAG_SAVING_ETC)
				|| (myData->save_msg_10ms[ch][msg].count_100 == 100)) {
				send_save_data_2(200, ch, msg);
			}
		}
	}
#else
	send_save_data(200);
#endif
	//jhkw_190830e

	if(myData->mData.real_time[4] != myPs->misc.sent_real_time_request) {
		j = 0;
		for(i=0; i < myPs->misc.chInGroup; i++) {
			ch = myData->CellArray1[myPs->misc.chOffset + i].number2 - 1;
			if(myPs->config.Real_Time_request == 0) {		//khj_200909
				if(myData->cData[ch].op.state == C_RUN
					|| myData->cData[ch].op.state == C_PAUSE) {
					j++;
				}
			}
		}
		if(j == 0) {
			if(myPs->signal[P1_SIG_SEND_REAL_TIME_REQUEST] == P0) {
				myPs->signal[P1_SIG_SEND_REAL_TIME_REQUEST] = P1;
			}
		}
	}

	if(myPs->signal[P1_SIG_SEND_REAL_TIME_REQUEST] == P1) {
		myPs->signal[P1_SIG_SEND_REAL_TIME_REQUEST] = P0;
		myPs->misc.sent_real_time_request = myData->mData.real_time[4]; //day
		send_cmd_real_time_request();
	}
/*kjg_w
	if((myData->mData.misc.timer_1sec - myPs->misc.sent_heartbeat_time)
		>= myPs->config.send_heartbeat_interval) {
		myPs->misc.sent_heartbeat_time = myData->mData.misc.timer_1sec;
		if(myPs->signal[P1_SIG_HEARTBEAT_FLAG] == P0) {
			myPs->signal[P1_SIG_HEARTBEAT_FLAG] = P1;
			rtn = send_cmd_heartbeat();
		}
	}

	if(myPs->signal[P1_SIG_SEND_ERROR_CODE] == P1) {
		myPs->signal[P1_SIG_SEND_ERROR_CODE] = P0;
		rtn = send_cmd_error_code();
	}

	if(myPs->signal[P1_SIG_METER_CONNECT_REPLY] == P1) {
		myPs->signal[P1_SIG_METER_CONNECT_REPLY] = P0;
		//send_cmd_meter_connect_reply();
		send_cmd_cali_start_reply();
	}*/
}

void Close_Process(void)
{
	if(myPs->misc.network_socket1 > 0) {
		close(myPs->misc.network_socket1);
	}

	if(myPs->misc.network_socket2 > 0) {
		close(myPs->misc.network_socket2);
	}
	
	myData->AppControl.signal[myPs->misc.psSignal] = P3;

	Close_SystemMemory();
}
