#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include "../../INC/datastore.h"
#include "userlog.h"
#include "local_utils.h"
#include "local_message.h"

extern volatile S_SYSTEM_DATA	*myData;
extern volatile S_TIMESCH_CONTROL	*myPs;
extern char psName[PROCESS_NAME_SIZE];

void Init_SystemMemory(void)
{
	memset((char *)&psName[0], 0, PROCESS_NAME_SIZE);
	strcpy(psName, "TimeSchedule");

	memset((char *)&myPs->signal, 0, MAX_SIGNAL);

	myPs->misc.processPointer = (int)&myData;
}

int	Read_TimeSchedule_Config(void)
{
	char temp[32], buf[8], fileName[128];
    int tmp;
    FILE *fp;

	memset(fileName, 0, sizeof(fileName));
	sprintf(fileName, "/root/%s/config/parameter/TimeSchedule_Config",
		myData->AppControl.misc.path1);
	// /root/system_data/config/parameter/TimeSchedule_Config
    if((fp = fopen(fileName, "r")) == NULL) {
		userlog(DEBUG_LOG, psName, "TimeSchedule_Config file read error\n");
		return -1;
	}

    tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->config.useFlag = (unsigned char)atoi(buf);

    fclose(fp);
	return 0;
}

void TimeSch_Time_Control(void)
{
	int ch, installedCh, idx, step;
	unsigned char time_flag, day_flag;
	unsigned long timesch_time, timesch_day;

	S_MSG_VAL SendMsg;

	if(myData->mData.config.installedCh == 0) return;

	time_flag = day_flag = 0;
	timesch_time = timesch_day = 0;
	installedCh = myData->mData.config.installedCh;

	//memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));

	for(ch=0; ch < installedCh; ch++) {
		if(myData->cData[ch].op.state != C_RUN) continue;
		if(myData->ChAttribute[ch].chNo_master == 0) continue;
		if(myPs->signal[TIMESCH_SIG_TIMESCH_INITIALIZE_1 + ch] == P1) {
			myPs->signal[TIMESCH_SIG_TIMESCH_INITIALIZE_1 + ch] = P0;
			myPs->timer[ch].timer_1day = 0;
			myPs->timer[ch].timer_1time = 0;
		} else {
			step = myData->cData[ch].op.idxStepNo; 

			idx = IDX_LOC_OBJ_END_TIMESCH_TIME;
			if(myPs->testCond[ch].local_object[step][idx] <= 0) {
				time_flag = 0;
			} else {
				time_flag = 1;
				timesch_time
					= myPs->testCond[ch].local_object[step][idx];
			}

			idx = IDX_LOC_OBJ_END_TIMESCH_DAY;
			if(myPs->testCond[ch].local_object[step][idx] <= 0) {
				day_flag = 0;
			} else {
				day_flag = 1;
				timesch_day
					= myPs->testCond[ch].local_object[step][idx];
			}

			if((time_flag == 0) && (day_flag == 0)) continue;

			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
			if((timesch_day <= myPs->timer[ch].timer_1day)
				&& (timesch_time <= myPs->timer[ch].timer_1time)) {
				SendMsg.msg = MSG_TIMESCH_COA_CMD_NEXT_STEP;
				SendMsg.val[0] = ch;
				send_msg(TIMESCH_TO_COA1, (char *)&SendMsg);
				userlog(DEBUG_LOG, psName, "TimeSch_COA cmd_next_step: %d %d\n",
					SendMsg.msg, SendMsg.val[0]);
			}
		}
	}
}

