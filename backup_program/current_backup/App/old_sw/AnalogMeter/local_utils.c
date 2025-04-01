#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "../../INC/datastore.h"
#include "userlog.h"
#include "serial.h"
#include "local_utils.h"

extern volatile S_SYSTEM_DATA *myData;
extern volatile S_ANALOG_METER  *myPs;
extern char	psName[PROCESS_NAME_SIZE];

int	Initialize(void)
{
	if(Open_SystemMemory(0) < 0) return -1;
	
	myPs = &(myData->AnalogMeter);

	Init_SystemMemory();

	memset((char *)&psName, 0, PROCESS_NAME_SIZE);
	strcpy(psName, "Analog");
	
	if(Read_AnalogMeter_Config() < 0) return -2;
	if(Read_AnalogArray_A() < 0) return -3;
	
	myData->AppControl.signal[APP_SIG_ANALOG_METER_PROCESS] = P1;
	return 0;
}

void Init_SystemMemory(void)
{
	memset((char *)&myPs->rcvPacket, 0, sizeof(S_ANALOG_METER_RCV_PACKET));
	memset((char *)&myPs->rcvCmd, 0, sizeof(S_ANALOG_METER_RCV_COMMAND));
	memset((char *)&myPs->signal, 0, MAX_SIGNAL);
	memset((char *)&myPs->tmp_value, 0, sizeof(long) * MAX_AUX_DATA);
	//kjgw memset((char *)&myPs->value, 0, sizeof(long) * MAX_AUX_DATA);
	memset((char *)&myPs->Array1, 0, sizeof(int) * MAX_AUX_DATA * 2);
	memset((char *)&myPs->Array2, 0, sizeof(int) * MAX_AUX_DATA * 2);

	myPs->misc.processPointer = (int)&myData;
}

int	Read_AnalogMeter_Config(void)
{
    int tmp, i;
	char temp[20], buf[8], fileName[128];
    FILE *fp;

	memset(fileName, 0, sizeof(fileName));
	strcpy(fileName, "/root/");
	strcat(fileName, (char *)&myData->AppControl.misc.path1);
	strcat(fileName, "/config/parameter/AnalogMeter_Config");
	// /root/system_data/config/parameter/AnalogMeter_Config
    if((fp = fopen(fileName, "r")) == NULL) {
		userlog(DEBUG_LOG, psName, "AnalogMeter_Config file read error\n");
		return -1;
	}

    tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->config.comPort = atoi(buf);

    tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->config.comBps = atoi(buf);

    tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
	//0:CS8_CREAD, 1:CS7_CSTOPB_PARENB
    myPs->config.commType = (unsigned char)atoi(buf);

    tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->config.CmdSendLog = (unsigned char)atoi(buf);

    tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->config.CmdRcvLog = (unsigned char)atoi(buf);

    tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->config.CmdSendLog_Hex = (unsigned char)atoi(buf);

    tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->config.CmdRcvLog_Hex = (unsigned char)atoi(buf);

    tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->config.CommCheckLog = (unsigned char)atoi(buf);

    tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
	//0:don't autoStart, 1:autoStart
    myPs->config.autoStart = (unsigned char)atoi(buf);

    tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->config.countMeter = (unsigned char)atoi(buf);

    tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
	//0:READ_T, 1:READ_V, 2:READ_I
    myPs->config.readType = (unsigned char)atoi(buf);

	for(i=0; i < MAX_COUNT_METER; i++) {
 	   tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
		memset(buf, 0, sizeof buf);
 	   tmp = fscanf(fp, "%s", buf);
    	myPs->config.measure_offset[i] = atol(buf);
	}

    tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->config.functionType = (unsigned char)atoi(buf);

    tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->config.comBps2 = atoi(buf);

    tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
	//0:CS8_CREAD, 1:CS7_CSTOPB_PARENB, 2:CS8_CREAD_PARENB
    myPs->config.commType2 = (unsigned char)atoi(buf);

    tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
	//0:don't autoStart, 1:autoStart
    myPs->config.autoStart2 = (unsigned char)atoi(buf);

    fclose(fp);
	return 0;
}

int Read_AnalogArray_A(void)
{
	int tmp, i, monitor_no, hw_no, ch_per_meter;
	char temp[20], buf[12], fileName[128];
    FILE *fp;

	memset(fileName, 0, sizeof(fileName));
	strcpy(fileName, "/root/");
	strcat(fileName, (char *)&myData->AppControl.misc.path1);
	strcat(fileName, "/config/parameter/AnalogArray_A");
    if((fp = fopen(fileName, "r")) == NULL) {
		userlog(DEBUG_LOG, psName,
			"AnalogArray_A file read error\n");
		return -1;
	}

   	tmp = fscanf(fp, "%s", temp);	tmp = fscanf(fp, "%s", temp);
   	tmp = fscanf(fp, "%s", temp);

	if(myPs->config.functionType == 0) ch_per_meter = 8;
	else ch_per_meter = 16;

	for(i=0; i < (myPs->config.countMeter * ch_per_meter); i++) {
		memset(buf, 0, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		monitor_no = atoi(buf);

   		tmp = fscanf(fp, "%s", temp);
		memset(buf, 0, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		hw_no = atoi(buf);

		myPs->Array1[i][0] = monitor_no;
		myPs->Array1[i][1] = hw_no;

		myPs->Array2[hw_no-1][0] = monitor_no;
		myPs->Array2[hw_no-1][1] = hw_no;
	}

    fclose(fp);
	return 0;
}

