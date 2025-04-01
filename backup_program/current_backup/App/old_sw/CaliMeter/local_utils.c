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
extern volatile S_CALI_METER  *myPs;
extern char	psName[PROCESS_NAME_SIZE];

int	Initialize(void)
{
	if(Open_SystemMemory(0) < 0) return -1;
	
	myPs = &(myData->CaliMeter);

	Init_SystemMemory();

	memset((char *)&psName, 0, PROCESS_NAME_SIZE);
	strcpy(psName, "Cali");
	
	if(Read_CaliMeter_Config() < 0) return -2;
	
	myData->AppControl.signal[APP_SIG_CALI_METER_PROCESS] = P1;
	return 0;
}

void Init_SystemMemory(void)
{
	memset((char *)&myPs->rcvPacket, 0, sizeof(S_CALI_METER_RCV_PACKET));
	memset((char *)&myPs->rcvCmd, 0, sizeof(S_CALI_METER_RCV_COMMAND));
	memset((char *)&myPs->signal, 0, MAX_SIGNAL);

	myPs->misc.processPointer = (int)&myData;
}

int	Read_CaliMeter_Config(void)
{
    int tmp;
	char temp[20], buf[8], fileName[128];
    FILE *fp;

	memset(fileName, 0, sizeof(fileName));
	strcpy(fileName, "/root/");
	strcat(fileName, (char *)&myData->AppControl.misc.path1);
	strcat(fileName, "/config/parameter/CaliMeter_Config");
	// /root/system_data/config/parameter/CaliMeter_Config
    if((fp = fopen(fileName, "r")) == NULL) {
		userlog(DEBUG_LOG, psName, "CaliMeter_Config file read error\n");
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
	//0:(READ_V_V)v-v,i-v, 1:(READ_V_I)v-v,i-i
    myPs->config.readType = (unsigned char)atoi(buf);

    tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
	//1:shunt 0.010ohm, 2:shunt 0.001ohm, 3:DCCT 600A/400mA,
	//4:DCCT 150A/200mA, 5:shunt 10ohm/100ohm, 6:meter DCI, 7:shunt 300A/100mV
	//8:shunt 600A/100mV
    myPs->config.measureI = (unsigned char)atoi(buf);

    tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->config.I_offset = atol(buf);

    tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
	//0:Agilient 34401A, 1:Display Meter1(232), 2:Display Meter2(485)
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

