#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include "../../INC/datastore.h"
#include "userlog.h"
#include "common_utils.h"
#include "local_utils.h"
#include "common_message.h"
#include "local_message.h"
#include "com_io.h"
#include "com_socket.h"
#include "network.h"
#include "main.h"

volatile S_SYSTEM_DATA	*myData;
volatile S_TEMP_CLIENT 	*myPs;
char psName[PROCESS_NAME_SIZE];

int main(int argc, char *argv[])
{
    int	rtn;
    struct timeval tv;
    fd_set rfds;

	if(Initialize(argc, argv) < 0) return 0;

	printf("khk ----------------------1\n");
   	while(myData->AppControl.signal[myPs->misc.psSignal] == P1) {
		myPs->signal[TEMP_SIG_NET_CONNECTED] = P0;
	printf("khk ----------------------2\n");
//		if(InitNetwork() < 0) continue;
	printf("khk ----------------------3\n");

		while(myData->AppControl.signal[myPs->misc.psSignal] == P1) {
	    	tv.tv_sec = 0;
		    tv.tv_usec = 100000; //100mSec

	printf("khk ----------------------4\n");
		    FD_ZERO(&rfds);
		    if(myPs->config.network_socket > 0)
				FD_SET(myPs->config.network_socket, &rfds);

			rtn = select(myPs->config.network_socket+1,
				&rfds, NULL, NULL, &tv);
			if(rtn > 0) {
	printf("khk ----------------------5\n");
				if(FD_ISSET(myPs->config.network_socket, &rfds) == 1) {
					if(NetworkPacket_Receive() < 0) {
						close(myPs->config.network_socket);
						rtn = -1;
						break;
					}else{
						rtn = Parsing_NetworkEvent();
						if(rtn < 0){
							close(myPs->config.network_socket);
							break;
						}
					}
				}
				rtn = TempClient_Control();
				if(rtn < 0){
					break;
				}
			} else if(rtn == 0) {
	printf("khk ----------------------6\n");
				rtn = TempClient_Control();
				if(rtn < 0){
					break;
				}
			}
		}
	}
	printf("khk ----------------------7\n");
	Close_Process();
    return 0;
}

int TempClient_Control(void)
{
	int rtn = 0;
	Check_Message();
	rtn = Check_Signal();
	if(rtn < 0) return rtn;
//	rtn = Check_NetworkState();
	return rtn;
}

int Check_Signal(void)
{
	int rtn=0;
	
	if(myPs->signal[TEMP_SIG_NET_CONNECTED] == P1) {
		myPs->pingTimer = myData->mData.misc.timer_1sec;
		myPs->pingCount = 0;
//		rtn = send_cmd_ch_data();
	}
	return rtn;
}


void Close_Process(void)
{
	if(myPs->config.network_socket > 0) {
		close(myPs->config.network_socket);
	}
	
	myData->AppControl.signal[myPs->misc.psSignal] = P3;

	Close_SystemMemory();
}
