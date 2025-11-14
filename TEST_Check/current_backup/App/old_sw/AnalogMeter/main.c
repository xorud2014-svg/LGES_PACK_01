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
volatile S_ANALOG_METER  *myPs; //my process : AnalogMeter
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
		scan_period2 = 120000;
	} else {
		scan_period1 = 0;
		scan_period2 = 450000;
	}

	while(myData->AppControl.signal[APP_SIG_ANALOG_METER_PROCESS] == P1) {
		if(InitComPort() < 0) continue; //kjgw shutdown process

		while(myData->AppControl.signal[APP_SIG_ANALOG_METER_PROCESS] == P1) {
			tv.tv_sec = scan_period1;
			tv.tv_usec = scan_period2;
			FD_ZERO(&rfds);
			if(myPs->misc.ttyS_fd > 0) FD_SET(myPs->misc.ttyS_fd, &rfds);
		
			rtn = select(myPs->misc.ttyS_fd+1, &rfds, NULL, NULL, &tv);
			//userlog(DEBUG_LOG, psName, "event1 %d\n", rtn); //kjgd
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
				rtn = Parsing_SerialEvent();
				if(rtn < 0) break;

				rtn = AnalogMeter_Control();
				if(rtn < 0) break;
			} else {}
		}
	}

	Close_Process();
	return 0;
}

int AnalogMeter_Control(void)
{
	int rtn;

	Check_Message();
	Check_Signal();

	rtn = Check_ComPortState();
	return rtn;
}

void Check_Signal(void)
{
	int debug=0;

	switch(myPs->signal[ANALOG_METER_SIG_INITIALIZE]) {
		case P1:
			send_cmd_initialize(1, 1, 1);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE]++;
			break;
		case P2:
			send_cmd_initialize(2, 1, 1);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P0;
			break;
		case P3:
			send_cmd_initialize(1, 1, 2);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE]++;
			break;
		case P4:
			send_cmd_initialize(2, 1, 2);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P0;
			break;
		case P5:
			send_cmd_initialize(1, 1, 3);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE]++;
			break;
		case P6:
			send_cmd_initialize(2, 1, 3);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P0;
			break;
		case P7:
			send_cmd_initialize(1, 1, 4);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE]++;
			break;
		case P8:
			send_cmd_initialize(2, 1, 4);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P0;
			break;
		case P11:
			send_cmd_initialize(1, 2, 1);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE]++;
			break;
		case P12:
			send_cmd_initialize(2, 2, 1);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P0;
			break;
		case P13:
			send_cmd_initialize(1, 2, 2);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE]++;
			break;
		case P14:
			send_cmd_initialize(2, 2, 2);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P0;
			break;
		case P15:
			send_cmd_initialize(1, 2, 3);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE]++;
			break;
		case P16:
			send_cmd_initialize(2, 2, 3);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P0;
			break;
		case P17:
			send_cmd_initialize(1, 2, 4);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE]++;
			break;
		case P18:
			send_cmd_initialize(2, 2, 4);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P0;
			break;
		case P21:
			send_cmd_initialize(1, 3, 1);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE]++;
			break;
		case P22:
			send_cmd_initialize(2, 3, 1);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P0;
			break;
		case P23:
			send_cmd_initialize(1, 3, 2);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE]++;
			break;
		case P24:
			send_cmd_initialize(2, 3, 2);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P0;
			break;
		case P25:
			send_cmd_initialize(1, 3, 3);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE]++;
			break;
		case P26:
			send_cmd_initialize(2, 3, 3);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P0;
			break;
		case P27:
			send_cmd_initialize(1, 3, 4);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE]++;
			break;
		case P28:
			send_cmd_initialize(2, 3, 4);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P0;
			break;
		case P31:
			send_cmd_initialize(1, 4, 1);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE]++;
			break;
		case P32:
			send_cmd_initialize(2, 4, 1);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P0;
			break;
		case P33:
			send_cmd_initialize(1, 4, 2);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE]++;
			break;
		case P34:
			send_cmd_initialize(2, 4, 2);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P0;
			break;
		case P35:
			send_cmd_initialize(1, 4, 3);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE]++;
			break;
		case P36:
			send_cmd_initialize(2, 4, 3);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P0;
			break;
		case P37:
			send_cmd_initialize(1, 4, 4);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE]++;
			break;
		case P38:
			send_cmd_initialize(2, 4, 4);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P0;
			break;
		default:	break;
	}

	if(myPs->config.functionType == 0) {
		switch(myPs->signal[ANALOG_METER_SIG_MEASURE]) {
			case P1:
				myPs->signal[ANALOG_METER_SIG_MEASURE] = P11;
				send_cmd_request(0, 1);
				break;
			case P2:
				myPs->signal[ANALOG_METER_SIG_MEASURE] = P12;
				send_cmd_request(0, 2);
				break;
			case P3:
				myPs->signal[ANALOG_METER_SIG_MEASURE] = P13;
				send_cmd_request(0, 3);
				break;
			case P4:
				myPs->signal[ANALOG_METER_SIG_MEASURE] = P14;
				send_cmd_request(0, 4);
				break;
			case P11:
			case P12:
			case P13:
			case P14:
				myPs->signal[ANALOG_METER_SIG_MEASURE_ERROR] += 1;
				if(myPs->signal[ANALOG_METER_SIG_MEASURE_ERROR] >= 3) {
					userlog(DEBUG_LOG, psName, "measure error : %d, %d\n",
						(int)myPs->signal[ANALOG_METER_SIG_MEASURE],
						(int)myPs->signal[ANALOG_METER_SIG_MEASURE_ERROR]);
					memset((char *)&myPs->rcvPacket, 0,
						sizeof(S_ANALOG_METER_RCV_PACKET));
					memset((char *)&myPs->rcvCmd, 0,
						sizeof(S_ANALOG_METER_RCV_COMMAND));
					myPs->signal[ANALOG_METER_SIG_MEASURE_ERROR] = 0;
					myPs->signal[ANALOG_METER_SIG_MEASURE] -= P10;
				}
				break;
			default:	break;
		}
	} else {
		switch(myPs->signal[ANALOG_METER_SIG_MEASURE]) {
			case P1:
				send_cmd_open(1);
				myPs->signal[ANALOG_METER_SIG_MEASURE] = P11;
				break;
			case P2:
				send_cmd_open(2);
				myPs->signal[ANALOG_METER_SIG_MEASURE] = P12;
				break;
			case P3:
				send_cmd_open(3);
				myPs->signal[ANALOG_METER_SIG_MEASURE] = P13;
				break;
			case P4:
				send_cmd_open(4);
				myPs->signal[ANALOG_METER_SIG_MEASURE] = P14;
				break;
			case P11:
			case P12:
			case P13:
			case P14:
				myPs->signal[ANALOG_METER_SIG_MEASURE_ERROR] += 1;
				if(myPs->signal[ANALOG_METER_SIG_MEASURE_ERROR] >= 3) {
					if(debug == 1) {
						userlog(DEBUG_LOG, psName, "measure error : %d, %d\n",
							(int)myPs->signal[ANALOG_METER_SIG_MEASURE],
							(int)myPs->signal[ANALOG_METER_SIG_MEASURE_ERROR]);
					}
					memset((char *)&myPs->rcvPacket, 0,
						sizeof(S_ANALOG_METER_RCV_PACKET));
					memset((char *)&myPs->rcvCmd, 0,
						sizeof(S_ANALOG_METER_RCV_COMMAND));
					myPs->signal[ANALOG_METER_SIG_MEASURE_ERROR] = 0;
					myPs->signal[ANALOG_METER_SIG_MEASURE] -= P10;
				}
				break;
			case P21:
				send_cmd_request(0, 1);
				myPs->signal[ANALOG_METER_SIG_MEASURE] = P31;
				break;
			case P22:
				send_cmd_request(0, 2);
				myPs->signal[ANALOG_METER_SIG_MEASURE] = P32;
				break;
			case P23:
				send_cmd_request(0, 3);
				myPs->signal[ANALOG_METER_SIG_MEASURE] = P33;
				break;
			case P24:
				send_cmd_request(0, 4);
				myPs->signal[ANALOG_METER_SIG_MEASURE] = P34;
				break;
			case P31:
			case P32:
			case P33:
			case P34:
				myPs->signal[ANALOG_METER_SIG_MEASURE_ERROR] += 1;
				if(myPs->signal[ANALOG_METER_SIG_MEASURE_ERROR] >= 3) {
					if(debug == 1) {
						userlog(DEBUG_LOG, psName, "measure error : %d, %d\n",
							(int)myPs->signal[ANALOG_METER_SIG_MEASURE],
							(int)myPs->signal[ANALOG_METER_SIG_MEASURE_ERROR]);
					}
					memset((char *)&myPs->rcvPacket, 0,
						sizeof(S_ANALOG_METER_RCV_PACKET));
					memset((char *)&myPs->rcvCmd, 0,
						sizeof(S_ANALOG_METER_RCV_COMMAND));
					myPs->signal[ANALOG_METER_SIG_MEASURE_ERROR] = 0;
					myPs->signal[ANALOG_METER_SIG_MEASURE] -= P10;
				}
				break;
			case P41:
				send_cmd_close(1);
				myPs->signal[ANALOG_METER_SIG_MEASURE] = P51;
				break;
			case P42:
				send_cmd_close(2);
				myPs->signal[ANALOG_METER_SIG_MEASURE] = P52;
				break;
			case P43:
				send_cmd_close(3);
				myPs->signal[ANALOG_METER_SIG_MEASURE] = P53;
				break;
			case P44:
				send_cmd_close(4);
				myPs->signal[ANALOG_METER_SIG_MEASURE] = P54;
				break;
			case P51:
			case P52:
			case P53:
			case P54:
				myPs->signal[ANALOG_METER_SIG_MEASURE_ERROR] += 1;
				if(myPs->signal[ANALOG_METER_SIG_MEASURE_ERROR] >= 3) {
					if(debug == 1) {
						userlog(DEBUG_LOG, psName, "measure error : %d, %d\n",
							(int)myPs->signal[ANALOG_METER_SIG_MEASURE],
							(int)myPs->signal[ANALOG_METER_SIG_MEASURE_ERROR]);
					}
					memset((char *)&myPs->rcvPacket, 0,
						sizeof(S_ANALOG_METER_RCV_PACKET));
					memset((char *)&myPs->rcvCmd, 0,
						sizeof(S_ANALOG_METER_RCV_COMMAND));
					myPs->signal[ANALOG_METER_SIG_MEASURE_ERROR] = 0;
					myPs->signal[ANALOG_METER_SIG_MEASURE] -= P10;
				}
				break;
			/*case P61:
				send_cmd_close(1);
				myPs->signal[ANALOG_METER_SIG_MEASURE] = P71;
				break;
			case P62:
				send_cmd_close(2);
				myPs->signal[ANALOG_METER_SIG_MEASURE] = P72;
				break;
			case P63:
				send_cmd_close(3);
				myPs->signal[ANALOG_METER_SIG_MEASURE] = P73;
				break;
			case P64:
				send_cmd_close(4);
				myPs->signal[ANALOG_METER_SIG_MEASURE] = P74;
				break;
			case P71:
			case P72:
			case P73:
			case P74:
				myPs->signal[ANALOG_METER_SIG_MEASURE_ERROR] += 1;
				if(myPs->signal[ANALOG_METER_SIG_MEASURE_ERROR] >= 3) {
					memset((char *)&myPs->rcvPacket, 0,
						sizeof(S_ANALOG_METER_RCV_PACKET));
					memset((char *)&myPs->rcvCmd, 0,
						sizeof(S_ANALOG_METER_RCV_COMMAND));
					myPs->signal[ANALOG_METER_SIG_MEASURE_ERROR] = 0;
					myPs->signal[ANALOG_METER_SIG_MEASURE] -= P10;
				}
				break;*/
			default:	break;
		}
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
	
	myData->AppControl.signal[APP_SIG_ANALOG_METER_PROCESS] = P3;	

	Close_SystemMemory();
}
