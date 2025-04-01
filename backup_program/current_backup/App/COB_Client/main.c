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
volatile S_COB_CLIENT *myPs; //my process : COB_Client[?]
char psName[PROCESS_NAME_SIZE];

int main(int argc, char *argv[])
{
    int	rtn;
    struct timeval tv;
    fd_set rfds;

	rtn = Initialize(argc, argv);
	if(rtn < 0) {
		if(rtn > (-10)) {
			printf("COB_Client Initialize fail(%d) %d, %s\n",
				rtn, argc, argv[0]);
		} else {
			userlog(DEBUG_LOG, psName,
				"COB_Client Initialize fail(%d) %d, %s\n",
				rtn, argc, argv[0]);
		}
		return 0;	
	}

    while(myData->AppControl.signal[myPs->misc.psSignal] == P1) {
		myPs->signal[P2_SIG_NET_CONNECTED] = P0;
		sleep(2);
		if(myPs->signal[P2_SIG_NET_CONNECT_DISAPPROVAL] == P1) continue;
		if(InitNetwork() < 0) continue; //kjgw shutdown process

		while(myData->AppControl.signal[myPs->misc.psSignal] == P1) {
	    	tv.tv_sec = 0;
		    tv.tv_usec = 500000;
		    FD_ZERO(&rfds);
		    if(myPs->misc.network_socket > 0)
				FD_SET(myPs->misc.network_socket, &rfds);

			rtn = select(myPs->misc.network_socket+1, &rfds, NULL, NULL, &tv);
			//userlog(DEBUG_LOG, psName, "event %d\n", rtn); //kjgd
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

				rtn = COB_Client_Control();
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

				rtn = COB_Client_Control();
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

int COB_Client_Control(void)
{
	int rtn;

	Check_Message();
	Check_Signal();
	Check_GroupState();
	
	rtn = Check_NetworkState();
	return rtn;
}

void Check_Signal(void)
{
	int rtn, count;
	long diff, time1, time2;
	
	if(myPs->signal[P2_SIG_NET_CONNECTED] != P1) return;

	switch(myPs->signal[P2_SIG_RCV_CH_ATTRIBUTE_SET]) {
		case P0:
			rtn = 0;
			break;
		case P1:
			myPs->signal[P2_SIG_RCV_CH_ATTRIBUTE_SET]++;
			convert_ch_attribute((int)myPs->config.groupNo);
			rtn = send_cmd_ch_attribute_set_reply(1);
			rtn = 1;
			break;
		case P2:
		case P3:
			rtn = 1;
			break;
		case P4:
			myPs->signal[P2_SIG_RCV_CH_ATTRIBUTE_SET]++;
			rtn = 1;
			break;
		case P5:
			myPs->signal[P2_SIG_RCV_CH_ATTRIBUTE_SET]++;
			rtn = 1;
			break;
		case P6:
			//myPs->signal[P2_SIG_SEND_GROUP_STATE] = P1;
			myPs->signal[P2_SIG_RCV_CH_ATTRIBUTE_SET] = P0;
			rtn = 1;
			break;
		default:
			rtn = 0;
			break;
	}
	if(rtn != 0) return;

	if(myPs->signal[P2_SIG_SEND_GROUP_STATE] == P1) {
		myPs->signal[P2_SIG_SEND_GROUP_STATE] = P0;
		if(send_cmd_group_state() < 0) return;
	}

	rtn = send_cmd_dcir_real_time_data();
	if(rtn > 0) count = 5;
	else count = 0;
	while(count) {
		rtn = send_cmd_dcir_real_time_data();
		if(rtn == 0) count = 0;
		else count--;
	}

	//kjgw rtn = send_cmd_measure_data();
	//if(rtn > 0) count = 5;
	//else count = 0;
	//while(count) {
	//	rtn = send_cmd_measure_data();
	//	if(rtn == 0) count = 0;
	//	else count--;
	//}

	rtn = send_cmd_step_data();
	if(rtn > 0) count = 5;
	else count = 0;
	while(count) {
		rtn = send_cmd_step_data();
		if(rtn == 0) count = 0;
		else count--;
	}

	time1 = myData->mData.misc.timer_1sec;
	time2 = myData->mData.misc.timer_1000ms;
	diff = (time1 - myPs->misc.sent_monitor_data_time) * 1000;
	diff += (time2 - myPs->misc.sent_monitor_data_time2);
	if(diff >= myPs->config.send_monitor_data_interval || diff < 0) {
		myPs->misc.sent_monitor_data_time = time1;
		myPs->misc.sent_monitor_data_time2 = time2;
		if(send_cmd_ch_data() < 0) return;
	}

	time1 = myData->mData.misc.timer_1sec;
	time2 = myData->mData.misc.timer_1000ms;
	diff = (time1 - myPs->misc.sent_sensor_data_time) * 1000;
	diff += (time2 - myPs->misc.sent_sensor_data_time2);
	if(myPs->config.send_sensor_data_interval != 0
		&& (diff >= myPs->config.send_sensor_data_interval || diff < 0)) {
		myPs->misc.sent_sensor_data_time = time1;
		myPs->misc.sent_sensor_data_time2 = time2;
		if(send_cmd_sensor_data() < 0) return;
	}

	time1 = myData->mData.misc.timer_1sec;
	time2 = myData->mData.misc.timer_1000ms;
	diff = (time1 - myPs->misc.sent_chamber_data_time) * 1000;
	diff += (time2 - myPs->misc.sent_chamber_data_time2);
	//if(myPs->config.send_chamber_data_interval != 0
	//	&& (diff >= myPs->config.send_chamber_data_interval || diff < 0)) {
	if(diff >= 1000 || diff < 0) { //1sec
		myPs->misc.sent_chamber_data_time = time1;
		myPs->misc.sent_chamber_data_time2 = time2;
		if(send_cmd_chamber_data() < 0) return;
	}
}

void Close_Process(void)
{
	if(myPs->misc.network_socket > 0) {
		close(myPs->misc.network_socket);
	}
	
	myData->AppControl.signal[myPs->misc.psSignal] = P3;

	Close_SystemMemory();
}
