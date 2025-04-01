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
volatile S_DATA_SAVE	*myPs; //my process : DataSave
char psName[16];
	
int main(void)
{
	int rtn;
	struct timeval tv;
	fd_set rfds;

	if(Initialize() < 0) return 0;

	while(myData->AppControl.signal[APP_SIG_DATA_SAVE_PROCESS] == P1) {
		tv.tv_sec = 0;
		tv.tv_usec = 200000;
		FD_ZERO(&rfds);

		rtn = select(0, &rfds, NULL, NULL, &tv);
		if(rtn == 0) {
			DataSave_Control();
		} else {
		}
	}
	Close_Process();
	return 0;
}

int Initialize(void)
{
	if(Open_SystemMemory(0) < 0) return -1;
	
	myPs = &(myData->DataSave);
	
	Init_SystemMemory();
	
	if(Read_DataSave_Config() < 0) return -2;

	myData->AppControl.signal[APP_SIG_DATA_SAVE_PROCESS] = P1;
	return 0;
}

void DataSave_Control(void)
{
	Check_Message();

	Check_Signal_1();
	Save_ResultData_1(0);

#ifdef __COB__
	Check_Signal_2();
#endif

#ifdef __COC__
	Save_ResultData_General_1(0);
#endif
}

void Check_Signal_1(void)
{
}

void Check_Signal_2(void)
{
	int group, rtn;

	for(group=0; group < myData->AppControl.config.totalGroup; group++) {
		do {
			rtn = Save_ResultData_Formation(group);
		} while(rtn);
	}

	for(group=0; group < myData->AppControl.config.totalGroup; group++) {
		do {
			rtn = Save_MonitoringData_Formation(group);
		} while(rtn);
	}
}

void Close_Process(void)
{
	Save_ResultData_1(1);

#ifdef __COC__
	Save_ResultData_General_1(1);
#endif
	myData->AppControl.signal[APP_SIG_DATA_SAVE_PROCESS] = P3;
	
	Close_SystemMemory();
}
