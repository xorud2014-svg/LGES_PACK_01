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

/* Server main--------------------------------------
int main(int argc, char *argv[])
{
    int	rtn, rtn1;
    struct timeval tv;
    fd_set rfds;

	if(Initialize(argc, argv) < 0) return 0;

   	while(myData->AppControl.signal[myPs->misc.psSignal] == P1) {
		myPs->signal[TEMP_SIG_NET_CONNECTED] = P0;
		myPs->signal[TEMP_SIG_CLIENT_CONNECT] = P0;
		if(InitNetwork() < 0) continue;

		while(myData->AppControl.signal[myPs->misc.psSignal] == P1) {
	    	tv.tv_sec = 0;
		    tv.tv_usec = 300000; //300mSec
		    FD_ZERO(&rfds);
			if(myPs->config.fd_socket >= 0) 
				FD_SET(myPs->config.fd_socket, &rfds);

			rtn = select(myPs->config.fd_socket+1, &rfds, NULL, NULL, &tv);
			if(rtn > 0) {
				if(FD_ISSET(myPs->config.fd_socket, &rfds) == 1) {
					if(NetworkPacket_Receive() < 0) {
						close(myPs->config.network_socket);
						close(myPs->config.fd_socket);
						break;
					}else{
						rtn1 = Parsing_NetworkEvent();
						if(rtn1 < 0){
							close(myPs->config.network_socket);
							close(myPs->config.fd_socket);
							break;
						}
					}
				}
			} else if(rtn == 0) {
				rtn1 = Parsing_NetworkEvent();
				if(rtn1 < 0){
					close(myPs->config.network_socket);
					close(myPs->config.fd_socket);
					break;
				}
				rtn1 = TempClient_Control();
				if(rtn1 < 0){
					close(myPs->config.network_socket);
					close(myPs->config.fd_socket);
					break;
				}
			}
		}
	}
	
	Close_Process();
    return 0;
}
*/

//Client Main----------------------------------------------------
int main(int argc, char *argv[])
{
    int	rtn, rtn1;
    struct timeval tv;
    fd_set rfds;

	if(Initialize(argc, argv) < 0) return 0;

   	while(myData->AppControl.signal[myPs->misc.psSignal] == P1) {
		myPs->signal[TEMP_SIG_NET_CONNECTED] = P0;
		myData->COM.com_port[AUX_TEMP].temp_cnt = 0;	
		myData->COM.com_port[AUX_TEMP].temp_cnt_flag = 0;
		if(InitNetwork() < 0) continue;

		while(myData->AppControl.signal[myPs->misc.psSignal] == P1) {
	    	tv.tv_sec = 0;
		    tv.tv_usec = 200000; //200mSec

		    FD_ZERO(&rfds);
		    if(myPs->config.network_socket > 0)
				FD_SET(myPs->config.network_socket, &rfds);

			rtn = select(myPs->config.network_socket+1,
				&rfds, NULL, NULL, &tv);
			if(rtn > 0) {
				if(FD_ISSET(myPs->config.network_socket, &rfds) == 1) {
					if(NetworkPacket_Receive() < 0) {
						close(myPs->config.network_socket);
						break;
					}else{
						rtn1 = Parsing_NetworkEvent();
						if(rtn1 < 0){
							close(myPs->config.network_socket);
							break;
						}
					}
				}
			} else if(rtn == 0) {
				rtn1 = Parsing_NetworkEvent();
				if(rtn1 < 0){
					close(myPs->config.network_socket);
					break;
				}
				rtn1 = TempClient_Control();
				if(rtn1 < 0){
					close(myPs->config.network_socket);
					break;
				}
			}
		}
	}
	
	Close_Process();
    return 0;
}
//-----------------------------------------------------------

int TempClient_Control(void)
{
	int rtn = 0;
	Check_Message();
	rtn = Check_NetworkState();
	if(rtn < 0) return rtn;
	rtn = Send_Cmd();
	return rtn;
}

int Send_Cmd(void)
{
	int rtn=0;
	
	if(myPs->signal[TEMP_SIG_NET_CONNECTED] == P1) {
		rtn = send_cmd_temp_data_req();
	}
	return rtn;
}


void Close_Process(void)
{
	if(myPs->config.network_socket > 0) {
		close(myPs->config.network_socket);
	}
	if(myPs->config.fd_socket >= 0) {
		close(myPs->config.fd_socket);
	}
	
	myData->AppControl.signal[myPs->misc.psSignal] = P3;

	Close_SystemMemory();
}
