#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>
#include "../../INC/datastore.h"
#include "userlog.h"
#include "local_message.h"
#include "common_message.h"
#include "local_utils.h"
#include "common_utils.h"
#include "network.h"

extern volatile S_SYSTEM_DATA *myData;
extern volatile S_SIL_CLIENT2 *myPs;
extern char psName[PROCESS_NAME_SIZE];

int Initialize(int argc, char *argv[])
{
	int SilNo;

	if(argc != 1){
		printf("SilClient Start Fail %d\n", argc);
		return -1;
	}

	if(Open_SystemMemory(0) < 0) return -2;

	SilNo = 1;

	myPs = &(myData->Sil_Client2);
	
	Init_SystemMemory(SilNo);
	
	if(Read_SilClient_Config() < 0) return -3;
	
	myData->AppControl.signal[myPs->misc.psSignal] = P1;
	
	return 0;
}

void Init_SystemMemory(int SilNo)
{
	memset((char *)&psName[0], 0, PROCESS_NAME_SIZE);
	switch(SilNo){
		case 1:
			strcpy(psName, "Sil_VI");
		    myPs->misc.psSignal = APP_SIG_SIL_CLIENT2_PROCESS;
			break;
		default:
			printf("default %d\n", SilNo);
			break;
	}
		
	memset((char *)&myPs->signal, 0x00, MAX_SIGNAL);
	memset((char *)&myPs->rcvCmd, 0x00, sizeof(S_P1_RCV_COMMAND));
	memset((char *)&myPs->rcvPacket, 0x00, sizeof(S_P1_RCV_PACKET));
	
	myPs->misc.SilNo = SilNo -1;
	myPs->netTimer = myData->mData.misc.timer_1sec;
	myPs->pingTimer = myData->mData.misc.timer_1sec;
	myPs->pingCount = 0;
	myPs->misc.cmd_serial = 0;
	myPs->misc.processPointer = (int)&myData;
}

int	Read_SilClient_Config(void)
{
    int tmp;
	char temp[32], buf[32], fileName[128];
    FILE *fp;

	memset(fileName, 0x00, sizeof(fileName));
	strcpy(fileName, "/root/");
	strcat(fileName, (char *)&myData->AppControl.misc.path1);
	strcat(fileName, "/config/parameter/client/");
	strcat(fileName, psName);
	strcat(fileName, "/SilClient_Config");
    if((fp = fopen(fileName, "r")) == NULL) {
		userlog(DEBUG_LOG, psName, "SilClient_Config file read error\n");
		return -1;
	}

    tmp = fscanf(fp, "%s", temp);	tmp = fscanf(fp, "%s", temp);
	memset(temp, 0x00, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	memcpy((char *)&myPs->config.ipAddr[0], (char *)&temp[0],
		sizeof(char)*16);
		
    tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->config.sendPort = atoi(buf);

    tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->config.receivePort = atoi(buf);

    tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->config.networkPort = atoi(buf);

    tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->config.retryTimeout = atoi(buf);

    tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->config.retryCount = atoi(buf);

    tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->config.heartbeatTimeout = atoi(buf);

    tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->config.CmdSendLog = (unsigned char)atoi(buf);

    tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->config.CmdRcvLog = (unsigned char)atoi(buf);

    tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->config.CmdSendLog_Hexa = (unsigned char)atoi(buf);

    tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->config.CmdRcvLog_Hexa = (unsigned char)atoi(buf);

	tmp = fscanf(fp, "%s", temp);	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	myPs->config.protocol_version = (unsigned int)atoi(buf);
    	
	tmp = fscanf(fp, "%s", temp);	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	myPs->config.crc_type = (unsigned char)atoi(buf);

	tmp = fscanf(fp, "%s", temp);	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	myPs->config.installedTemp = atoi(buf);
    
	tmp = fscanf(fp, "%s", temp);	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	memcpy((char *)&myPs->config.SensorType, (char *)&buf[0], 1);
    	
    fclose(fp);

	return 0;
}
