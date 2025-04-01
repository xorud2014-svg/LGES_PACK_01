#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include "../../INC/datastore.h"
#include "userlog.h"
#include "local_utils.h"
#include "local_message.h"
#include "main.h"

volatile S_SYSTEM_DATA	*myData;
volatile S_TIMESCH_CONTROL	*myPs; //my process : TimeSchedule
char psName[PROCESS_NAME_SIZE];
	
int main(void)
{
    int rtn;
    struct timeval tv;
    fd_set rfds;
    
	if(Initialize() < 0) return 0;

    while(myData->AppControl.signal[APP_SIG_TIME_SCHEDULE_PROCESS] == P1) {
    	tv.tv_sec = 0;
		tv.tv_usec = 500000;
		FD_ZERO(&rfds);

		rtn = select(0, &rfds, NULL, NULL, &tv);
		if(rtn == 0) {
			TimeSchedule_Control();
		} else {
		}
    }
	Close_Process();
    return 0;
}

int Initialize(void)
{
	if(Open_SystemMemory(0) < 0) return -1;
	
	myPs = &(myData->TimeSch);

	Init_SystemMemory();
	
	if(Read_TimeSchedule_Config() < 0) return -2;

	myData->AppControl.signal[APP_SIG_TIME_SCHEDULE_PROCESS] = P1;
	return 0;
}

void TimeSchedule_Control(void)
{
	Check_Message();

	TimeSch_Time_Control();
}

void Close_Process(void)
{
    myData->AppControl.signal[APP_SIG_TIME_SCHEDULE_PROCESS] = P3;
	
	Close_SystemMemory();
}
