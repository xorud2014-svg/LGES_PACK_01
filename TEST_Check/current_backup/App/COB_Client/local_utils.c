#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "../../INC/datastore.h"
#include "userlog.h"
#include "common_utils.h"
#include "common_message.h"
#include "local_message.h"
#include "network.h"
#include "local_utils.h"

extern volatile S_SYSTEM_DATA *myData;
extern volatile S_COB_CLIENT *myPs;
extern char psName[PROCESS_NAME_SIZE];

int Initialize(int argc, char *argv[])
{
	int CO_No, i, j, toPs, rtn;
	S_MSG_VAL SendMsg;

	if(argc != 1) {
		printf("COB_Client start fail %d, %s\n", argc, argv[0]);
		return -1;
	}

	if(Open_SystemMemory(0) < 0) return -2;
	
	CO_No = argv[0][5] - 0x31;
	myPs = &(myData->COB_Client[CO_No]);
	
	Init_SystemMemory(CO_No);
	
	rtn = Read_COB_Client_Config();
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "Read_COB_Client_Config fail %d\n", rtn);
		return -10;
	}
	rtn = Read_WorkMode();
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "Read_WorkMode fail %d\n", rtn);
		return -11;
	}
	rtn = Read_DataSaveNo(CO_No);
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "Read_DataSaveNo fail %d\n", rtn);
		return -12;
	}
	rtn = Read_ChArray();
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "Read_ChArray fail %d\n", rtn);
		return -13;
	}
	rtn = Read_COB_ConfigData();
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "Read_COB_ConfigData fail %d\n", rtn);
		return -14;
	}
	rtn = Read_testConfig();
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "Read_testConfig fail %d\n", rtn);
		return -15;
	}
	rtn = Read_sensor_limit();
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "Read_sensor_limit fail %d\n", rtn);
		return -16;
	}
	rtn = Read_chamber_set();
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "Read_chamber_set fail %d\n", rtn);
		return -17;
	}
	rtn = Read_job_change_set();
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "Read_job_change_set fail %d\n", rtn);
		return -18;
	}

	myPs->misc.chOffset = 0;
	for(i=0; i < MAX_GROUP_8; i++) {
		j = (int)myData->mData.config.chInGroup[i];
		if(myPs->config.groupNo == i) {
			myPs->misc.chInGroup = j;
			break;
		}
		myPs->misc.chOffset += j;
	}

	i = 1;
	if(myData->ChAttribute[0].chNo_slave[0] != 0) i++;
	if(myData->ChAttribute[0].chNo_slave[1] != 0) i++;
	if(myData->ChAttribute[0].chNo_slave[2] != 0) i++;
	myPs->misc.parallel_count = (unsigned char)i;

	memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));

	toPs = COB1_TO_MODULE + myPs->config.groupNo;
	SendMsg.msg = MSG_COB_MODULE_WORK_MODE;
	SendMsg.val[0] = myPs->config.groupNo;
	SendMsg.val[1] = myPs->config.workMode;
	send_msg(toPs, (char *)&SendMsg);

	toPs = COB1_TO_MODULE + myPs->config.groupNo;
	SendMsg.msg = MSG_COB_MODULE_CONTROL_MODE;
	SendMsg.val[0] = myPs->config.groupNo;
	SendMsg.val[1] = myPs->config.groupControlMode;
	send_msg(toPs, (char *)&SendMsg);

	myData->AppControl.signal[myPs->misc.psSignal] = P1;
	return 0;
}

void Init_SystemMemory(int CO_No)
{
	memset((char *)&psName, 0, PROCESS_NAME_SIZE);
	sprintf(psName, "COB%d", CO_No+1);
   	myPs->misc.psSignal = APP_SIG_COB1_CLIENT_PROCESS + CO_No;

	memset((char *)&myPs->signal, 0, MAX_SIGNAL);
	
	memset((char *)&myPs->rcvCmd, 0, sizeof(S_P2_RCV_COMMAND));
	memset((char *)&myPs->rcvPacket, 0, sizeof(S_P2_RCV_PACKET));
	
	memset((char *)&myPs->reply, 0, sizeof(S_P2_REPLY));

	if(myData->AppControl.signal[myPs->misc.psSignal] == P0) {
		myPs->config.groupControlMode = WORK_MODE_CONTROL;
	}
	myPs->misc.cmd_serial = 0;
	myPs->misc.net_time = 0;
	myPs->misc.net_time2 = 0;
	myPs->misc.ping_time = 0;
	myPs->misc.ping_time2 = 0;
	myPs->misc.sent_monitor_data_time = 0;
	myPs->misc.sent_monitor_data_time2 = 0;
	myPs->misc.sent_sensor_data_time = 0;
	myPs->misc.sent_sensor_data_time2 = 0;
	myPs->misc.sent_chamber_data_time = 0;
	myPs->misc.sent_chamber_data_time2 = 0;

	myPs->misc.processPointer = (int)&myData;
	myPs->misc.CO_No = CO_No;
}

int	Read_COB_Client_Config(void)
{
	char temp[32], buf[32], fileName[256];
    int tmp;
    FILE *fp;

	memset(fileName, 0, sizeof fileName);
	strcpy(fileName, "/root/");
	strcat(fileName, (char *)&myData->AppControl.misc.path1);
	strcat(fileName, "/config/parameter/client/");
	strcat(fileName, psName);
	strcat(fileName, "/Client_Config");
	// /root/system_data/config/parameter/client/COB?/Client_Config
    if((fp = fopen(fileName, "r")) == NULL) {
		userlog(DEBUG_LOG, psName, "%s_Client_Config file read error\n",
		psName);
		return -1;
	}

	memset(temp, 0, sizeof temp);
   	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "groupId") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
   	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -3;
	}
	memset(buf, 0, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	myPs->config.groupId = (short int)atoi(buf);

	memset(temp, 0, sizeof temp);
   	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "groupNo") != 0) {
		fclose(fp);
		return -4;
	}
	memset(temp, 0, sizeof temp);
   	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -5;
	}
	memset(buf, 0, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	myPs->config.groupNo = (short int)(atoi(buf) - 1);

	memset(temp, 0, sizeof temp);
   	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "ipAddr") != 0) {
		fclose(fp);
		return -6;
	}
	memset(temp, 0, sizeof temp);
   	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -7;
	}
	memset(buf, 0, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	memcpy((char *)&myPs->config.ipAddr, (char *)&buf, 16);
		
	memset(temp, 0, sizeof temp);
   	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "sendPort") != 0) {
		fclose(fp);
		return -8;
	}
	memset(temp, 0, sizeof temp);
   	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -9;
	}
	memset(buf, 0, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->config.sendPort = atoi(buf);

	memset(temp, 0, sizeof temp);
   	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "receivePort") != 0) {
		fclose(fp);
		return -10;
	}
	memset(temp, 0, sizeof temp);
   	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -11;
	}
	memset(buf, 0, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->config.receivePort = atoi(buf);

	memset(temp, 0, sizeof temp);
   	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "networkPort") != 0) {
		fclose(fp);
		return -12;
	}
	memset(temp, 0, sizeof temp);
   	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -13;
	}
	memset(buf, 0, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->config.networkPort = atoi(buf);

	memset(temp, 0, sizeof temp);
   	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "protocol_version") != 0) {
		fclose(fp);
		return -14;
	}
	memset(temp, 0, sizeof temp);
   	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -15;
	}
	memset(buf, 0, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	myPs->config.protocol_version = atoi(buf);
    	
	memset(temp, 0, sizeof temp);
   	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "retryCount") != 0) {
		fclose(fp);
		return -16;
	}
	memset(temp, 0, sizeof temp);
   	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -17;
	}
	memset(buf, 0, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->config.retryCount = atoi(buf);

	memset(temp, 0, sizeof temp);
   	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "replyTimeout") != 0) {
		fclose(fp);
		return -18;
	}
	memset(temp, 0, sizeof temp);
   	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -19;
	}
	memset(buf, 0, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->config.replyTimeout = atol(buf);

	memset(temp, 0, sizeof temp);
   	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "netTimeout") != 0) {
		fclose(fp);
		return -20;
	}
	memset(temp, 0, sizeof temp);
   	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -21;
	}
	memset(buf, 0, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->config.netTimeout = atol(buf);

	memset(temp, 0, sizeof temp);
   	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "pingTimeout") != 0) {
		fclose(fp);
		return -22;
	}
	memset(temp, 0, sizeof temp);
   	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -23;
	}
	memset(buf, 0, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->config.pingTimeout = atol(buf);

	memset(temp, 0, sizeof temp);
   	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "CmdSendLog") != 0) {
		fclose(fp);
		return -24;
	}
	memset(temp, 0, sizeof temp);
   	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -25;
	}
	memset(buf, 0, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->config.CmdSendLog = (unsigned char)atoi(buf);

	memset(temp, 0, sizeof temp);
   	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "CmdRcvLog") != 0) {
		fclose(fp);
		return -26;
	}
	memset(temp, 0, sizeof temp);
   	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -27;
	}
	memset(buf, 0, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->config.CmdRcvLog = (unsigned char)atoi(buf);

	memset(temp, 0, sizeof temp);
   	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "CmdSendLog_Hex") != 0) {
		fclose(fp);
		return -28;
	}
	memset(temp, 0, sizeof temp);
   	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -29;
	}
	memset(buf, 0, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->config.CmdSendLog_Hex = (unsigned char)atoi(buf);

	memset(temp, 0, sizeof temp);
   	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "CmdRcvLog_Hex") != 0) {
		fclose(fp);
		return -30;
	}
	memset(temp, 0, sizeof temp);
   	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -31;
	}
	memset(buf, 0, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->config.CmdRcvLog_Hex = (unsigned char)atoi(buf);

	memset(temp, 0, sizeof temp);
   	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "CommCheckLog") != 0) {
		fclose(fp);
		return -32;
	}
	memset(temp, 0, sizeof temp);
   	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -33;
	}
	memset(buf, 0, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->config.CommCheckLog = (unsigned char)atoi(buf);

	memset(temp, 0, sizeof temp);
   	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "send_monitor_data_interval") != 0) {
		fclose(fp);
		return -34;
	}
	memset(temp, 0, sizeof temp);
   	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -35;
	}
	memset(buf, 0, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->config.send_monitor_data_interval = atol(buf);

	memset(temp, 0, sizeof temp);
   	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "send_save_data_interval") != 0) {
		fclose(fp);
		return -36;
	}
	memset(temp, 0, sizeof temp);
   	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -37;
	}
	memset(buf, 0, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->config.send_save_data_interval = atol(buf);

	memset(temp, 0, sizeof temp);
   	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "send_sensor_data_interval") != 0) {
		fclose(fp);
		return -38;
	}
	memset(temp, 0, sizeof temp);
   	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -39;
	}
	memset(buf, 0, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->config.send_sensor_data_interval = atol(buf);

	memset(temp, 0, sizeof temp);
   	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "state_change") != 0) {
		fclose(fp);
		return -40;
	}
	memset(temp, 0, sizeof temp);
   	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -41;
	}
	memset(buf, 0, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	//1:pause - network fail, 2:run - network fail
	myPs->config.state_change = (unsigned char)atoi(buf);
    	
    fclose(fp);
	return 0;
}

int	Write_COB_Client_Config(void)
{
	char fileName[256];
    FILE *fp;

	memset(fileName, 0, sizeof(fileName));
	strcpy(fileName, "/root/");
	strcat(fileName, (char *)&myData->AppControl.misc.path1);
	strcat(fileName, "/config/parameter/client/");
	strcat(fileName, psName);
	strcat(fileName, "/Client_Config");
	// /root/system_data/config/parameter/client/COB?/Client_Config
    if((fp = fopen(fileName, "w")) == NULL) {
		userlog(DEBUG_LOG, psName, "%s_Client_Config file write error\n",
		psName);
		return -1;
	}

    fprintf(fp, "groupId             :   %d\n", myPs->config.groupId);
    fprintf(fp, "groupNo             :   %d\n", (int)(myPs->config.groupNo+1));
	fprintf(fp, "ipAddr              :   %s\n", myPs->config.ipAddr);
    fprintf(fp, "sendPort            :   %d\n", myPs->config.sendPort);
	fprintf(fp, "receivePort         :   %d\n", myPs->config.receivePort);
	fprintf(fp, "networkPort         :   %d\n", myPs->config.networkPort);
	fprintf(fp, "protocol_version    :   %d\n", myPs->config.protocol_version);
	fprintf(fp, "retryCount          :   %d\n", myPs->config.retryCount);
	fprintf(fp, "replyTimeout        :   %ld\n", myPs->config.replyTimeout);
	fprintf(fp, "netTimeout          :   %ld\n", myPs->config.netTimeout);
	fprintf(fp, "pingTimeout         :   %ld\n", myPs->config.pingTimeout);
	fprintf(fp, "CmdSendLog          :   %d\n", myPs->config.CmdSendLog);
	fprintf(fp, "CmdRcvLog           :   %d\n", myPs->config.CmdRcvLog);
	fprintf(fp, "CmdSendLog_Hex      :   %d\n", myPs->config.CmdSendLog_Hex);
	fprintf(fp, "CmdRcvLog_Hex       :   %d\n", myPs->config.CmdRcvLog_Hex);
	fprintf(fp, "CommCheckLog        :   %d\n", myPs->config.CommCheckLog);
	fprintf(fp, "send_monitor_data_interval     :   %ld\n",
		myPs->config.send_monitor_data_interval);
	fprintf(fp, "send_save_data_interval        :   %ld\n",
		myPs->config.send_save_data_interval);
	fprintf(fp, "send_sensor_data_interval        :   %ld\n",
		myPs->config.send_sensor_data_interval);
	fprintf(fp, "state_change        :   %d\n", myPs->config.state_change);
    	
    fclose(fp);
	return 0;
}

int Read_WorkMode(void)
{
	char buf[32], fileName[256];
    int tmp;
    FILE *fp;

	memset(fileName, 0, sizeof(fileName));
	strcpy(fileName, "/root/");
	strcat(fileName, (char *)&myData->AppControl.misc.path1);
	strcat(fileName, "/config/parameter/client/");
	strcat(fileName, psName);
	strcat(fileName, "/WorkMode");
	// /root/system_data/config/parameter/client/COB?/WorkMode
    if((fp = fopen(fileName, "r")) == NULL) {
		userlog(DEBUG_LOG, psName, "%s WorkMode file read error\n",
		psName);
		return -1;
	}

	memset(buf, 0, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	//0:online, 1:offline
	myPs->config.workMode = (unsigned char)atoi(buf);

	fclose(fp);
	return 0;
}

int Write_WorkMode(void)
{
	char fileName[256];
    FILE *fp;

	memset(fileName, 0, sizeof(fileName));
	strcpy(fileName, "/root/");
	strcat(fileName, (char *)&myData->AppControl.misc.path1);
	strcat(fileName, "/config/parameter/client/");
	strcat(fileName, psName);
	strcat(fileName, "/WorkMode");
	// /root/system_data/config/parameter/client/COB?/WorkMode
    if((fp = fopen(fileName, "w+")) == NULL) {
		userlog(DEBUG_LOG, psName, "%s WorkMode file write error\n",
		psName);
		return -1;
	}

	//0:online, 1:offline
	fprintf(fp, "%d\n", myPs->config.workMode);

	fclose(fp);
	return 0;
}

int Read_DataSaveNo(int group)
{
	char buf[32], fileName[256];
    int tmp;
    FILE *fp;

	memset(fileName, 0, sizeof(fileName));
	strcpy(fileName, "/root/");
	strcat(fileName, (char *)&myData->AppControl.misc.path1);
	strcat(fileName, "/config/parameter/client/");
	strcat(fileName, psName);
	strcat(fileName, "/DataSaveNo");
	// /root/system_data/config/parameter/client/COB?/DataSaveNo
    if((fp = fopen(fileName, "r")) == NULL) {
		userlog(DEBUG_LOG, psName, "%s DataSaveNo file read error\n",
		psName);
		return -1;
	}

	memset(buf, 0, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	myData->DataSave.resultData_formation[group].test_index = atoi(buf);
	myData->DataSave.monitorData_formation[group].test_index = atoi(buf);

	fclose(fp);
	return 0;
}

int Read_ChArray(void)
{
	if(Read_ChArray1() < 0) return -1;
	if(Read_ChArray2() < 0) return -2;
	if(Read_ChArray3() < 0) return -3;
	
	return 0;
}

int	Read_ChArray1(void)
{
	char temp[32], buf[32], fileName[256];
    int tmp, i, group, max_ch;
    FILE *fp;

	memset(fileName, 0, sizeof(fileName));
	strcpy(fileName, "/root/");
	strcat(fileName, (char *)&myData->AppControl.misc.path1);
	strcat(fileName, "/config/parameter/client/");
	strcat(fileName, psName);
	strcat(fileName, "/ChArray1");
	// /root/system_data/config/parameter/client/COB?/ChArray1
    if((fp = fopen(fileName, "r")) == NULL) {
		userlog(DEBUG_LOG, psName, "%s ChArray1 file read error\n", psName);
		return -1;
	}

	memset(temp, 0, sizeof temp);
    tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "index_ch") != 0) {
		fclose(fp);
		return -2;
	}

	memset(temp, 0, sizeof temp);
    tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "hw_bd") != 0) {
		fclose(fp);
		return -2;
	}

	memset(temp, 0, sizeof temp);
    tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "hw_ch") != 0) {
		fclose(fp);
		return -2;
	}

	group = myPs->config.groupNo;
	max_ch = myData->mData.config.chInGroup[group];

	memset((char *)&myPs->config.ChArray1[0], 0,
		sizeof(S_P2_ARRAY1) * MAX_CH_256);

	for(i=0; i < MAX_CH_256; i++) {
		if(i >= max_ch) {
    		myPs->config.ChArray1[i].bd = (-1);
    		myPs->config.ChArray1[i].ch = (-1);
			continue;
		}

		memset(buf, 0, sizeof buf);
    	tmp = fscanf(fp, "%s", buf);
		tmp = atoi(buf) - 1;
		if(tmp != i) {
			fclose(fp);
			return -3;
		}

		myPs->config.ChArray1[i].index = (short int)i;

		memset(buf, 0, sizeof buf);
    	tmp = fscanf(fp, "%s", buf);
		tmp = atoi(buf) - 1;
    	myPs->config.ChArray1[i].bd = (short int)tmp;

		memset(buf, 0, sizeof buf);
    	tmp = fscanf(fp, "%s", buf);
		tmp = atoi(buf) - 1;
    	myPs->config.ChArray1[i].ch = (short int)tmp;
	}

    fclose(fp);
	return 0;
}

int	Read_ChArray2(void)
{
	char temp[32], buf[32], fileName[256];
    int tmp, i, max_ch, group;
    FILE *fp;

	memset(fileName, 0, sizeof(fileName));
	strcpy(fileName, "/root/");
	strcat(fileName, (char *)&myData->AppControl.misc.path1);
	strcat(fileName, "/config/parameter/client/");
	strcat(fileName, psName);
	strcat(fileName, "/ChArray2");
	// /root/system_data/config/parameter/client/COB?/ChArray2
    if((fp = fopen(fileName, "r")) == NULL) {
		userlog(DEBUG_LOG, psName, "%s ChArray2 file read error\n", psName);
		return -1;
	}

	memset(temp, 0, sizeof temp);
    tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "HARDWARE") != 0) {
		fclose(fp);
		return -2;
	}

	memset(temp, 0, sizeof temp);
    tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "MONITORING") != 0) {
		fclose(fp);
		return -2;
	}

	group = myPs->config.groupNo;
	max_ch = myData->mData.config.chInGroup[group];

	memset((char *)&myPs->config.ChArray2[0], 0,
		sizeof(short int) * MAX_CH_256);

	for(i=0; i < MAX_CH_256; i++) {
		if(i >= max_ch) {
    		myPs->config.ChArray2[i] = (-1);
			continue;
		}

		memset(buf, 0, sizeof buf);
    	tmp = fscanf(fp, "%s", buf); //hardware number
		tmp = atoi(buf) - 1;
		if(tmp != i) {
			fclose(fp);
			return -3;
		}

		memset(buf, 0, sizeof buf);
    	tmp = fscanf(fp, "%s", buf);
		tmp = atoi(buf) - 1;
    	myPs->config.ChArray2[i] = (short int)tmp; //monitor number
	}

    fclose(fp);
	return 0;
}

int	Read_ChArray3(void)
{
    int i, j;

	for(i=0; i < MAX_CH_256; i++) {
		myPs->config.ChArray3[i].index = (-1);
		myPs->config.ChArray3[i].bd = (-1);
		myPs->config.ChArray3[i].ch = (-1);
	}

	for(i=0; i < MAX_CH_256; i++) { //monitor
		for(j=0; j < MAX_CH_256; j++) {
			if(i == myPs->config.ChArray2[j]) { //monitor
				memcpy((char *)&myPs->config.ChArray3[i],
					(char *)&myPs->config.ChArray1[j],
					sizeof(S_P2_ARRAY1)); //hardware number
				break;
			}
		}
	}

	return 0;
}

int	Read_COB_ConfigData(void)
{
	char temp[32], buf[32], fileName[256];
    int tmp;
    FILE *fp;

	memset((char *)&myPs->configData, 0, sizeof(S_P2_CONFIG_DATA));

	memset(fileName, 0, sizeof(fileName));
	strcpy(fileName, "/root/");
	strcat(fileName, (char *)&myData->AppControl.misc.path1);
	strcat(fileName, "/config/parameter/client/");
	strcat(fileName, psName);
	strcat(fileName, "/ConfigData");
	// /root/system_data/config/parameter/client/COB?/ConfigData
    if((fp = fopen(fileName, "r")) == NULL) {
		userlog(DEBUG_LOG, psName, "%s_ConfigData file read error\n", psName);
		return -1;
		/*kjgw	
		//default value setting
		myPs->configData.vAutoCal = 1;			//use
		myPs->configData.iAutoCal = 0;			//unuse
		myPs->configData.huntingCount = 3;
		myPs->configData.vHuntingLevel = 150;	//150mV
		myPs->configData.iHuntingLevel = 180;	//180mV
		myPs->configData.stableTime = 10;		//10sec
		myPs->configData.dVstableTime = 60;		//60sec
		myPs->configData.vRefNGLevel = 300;		//300digit
		myPs->configData.iRefNGLevel = 800;		//800digit
		return 1;*/
	}

	memset(temp, 0, sizeof temp);
    tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "vAutoCal") != 0) {
		fclose(fp);
		return -2;
	}
	memset(buf, 0, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->configData.vAutoCal = (unsigned char)atoi(buf);

	memset(temp, 0, sizeof temp);
    tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "iAutoCal") != 0) {
		fclose(fp);
		return -2;
	}
	memset(buf, 0, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->configData.iAutoCal = (unsigned char)atoi(buf);

	memset(temp, 0, sizeof temp);
    tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "huntingCount") != 0) {
		fclose(fp);
		return -2;
	}
	memset(buf, 0, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->configData.huntingCount = (short int)atoi(buf);

	memset(temp, 0, sizeof temp);
    tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "vHuntingLevel") != 0) {
		fclose(fp);
		return -2;
	}
	memset(buf, 0, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->configData.vHuntingLevel = atol(buf);

	memset(temp, 0, sizeof temp);
    tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "iHuntingLevel") != 0) {
		fclose(fp);
		return -2;
	}
	memset(buf, 0, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->configData.iHuntingLevel = atol(buf);

	memset(temp, 0, sizeof temp);
    tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "stableTime") != 0) {
		fclose(fp);
		return -2;
	}
	memset(buf, 0, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->configData.stableTime = atol(buf);

	memset(temp, 0, sizeof temp);
    tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "dVstableTime") != 0) {
		fclose(fp);
		return -2;
	}
	memset(buf, 0, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->configData.dVstableTime = atol(buf);

	memset(temp, 0, sizeof temp);
    tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "vRefNGLevel") != 0) {
		fclose(fp);
		return -2;
	}
	memset(buf, 0, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->configData.vRefNGLevel = atol(buf);

	memset(temp, 0, sizeof temp);
    tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "iRefNGLevel") != 0) {
		fclose(fp);
		return -2;
	}
	memset(buf, 0, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->configData.iRefNGLevel = atol(buf);

    fclose(fp);
	return 0;
}

int	Write_COB_ConfigData(void)
{
	char fileName[256];
    FILE *fp;

	memset(fileName, 0, sizeof(fileName));
	strcpy(fileName, "/root/");
	strcat(fileName, (char *)&myData->AppControl.misc.path1);
	strcat(fileName, "/config/parameter/client/");
	strcat(fileName, psName);
	strcat(fileName, "/ConfigData");
	// /root/system_data/config/parameter/client/COB?/ConfigData
    if((fp = fopen(fileName, "w")) == NULL) {
		userlog(DEBUG_LOG, psName, "%s_ConfigData file write error\n", psName);
		return -1;
	}

	fprintf(fp, "vAutoCal		%d\n", (int)myPs->configData.vAutoCal);
    fprintf(fp, "iAutoCal		%d\n", (int)myPs->configData.iAutoCal);
    fprintf(fp, "huntingCount	%d\n", (int)myPs->configData.huntingCount);
	fprintf(fp, "vHuntingLevel	%ld\n", myPs->configData.vHuntingLevel);
    fprintf(fp, "iHuntingLevel	%ld\n", myPs->configData.iHuntingLevel);
    fprintf(fp, "stableTime		%ld\n", myPs->configData.stableTime);
    fprintf(fp, "dVstableTime	%ld\n", myPs->configData.dVstableTime);
    fprintf(fp, "vRefNGLevel	%ld\n", myPs->configData.vRefNGLevel);
    fprintf(fp, "iRefNGLevel	%ld\n", myPs->configData.iRefNGLevel);

    fclose(fp);
	return 0;
}

int	Read_testConfig(void)
{
	char temp[32], buf[32], fileName[256];
    int tmp;
    FILE *fp;

	memset((char *)&myPs->testConfig, 0, sizeof(S_P2_TEST_CONFIG));

	memset(fileName, 0, sizeof fileName);
	strcpy(fileName, "/root/");
	strcat(fileName, (char *)&myData->AppControl.misc.path1);
	strcat(fileName, "/config/parameter/client/");
	strcat(fileName, psName);
	strcat(fileName, "/testConfig");
	// /root/system_data/config/parameter/client/COB?/testConfig
    if((fp = fopen(fileName, "r")) == NULL) {
		userlog(DEBUG_LOG, psName, "testConfig file read error\n", psName);
		return -1;
	}

	memset(temp, 0, sizeof temp);
    tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "maxRefV") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
    tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -2;
	}
	memset(buf, 0, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->testConfig.maxRefV = atol(buf);

	memset(temp, 0, sizeof temp);
    tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "minRefV") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
    tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -2;
	}
	memset(buf, 0, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->testConfig.minRefV = atol(buf);

	memset(temp, 0, sizeof temp);
    tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "maxRefI") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
    tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -2;
	}
	memset(buf, 0, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->testConfig.maxRefI = atol(buf);

	memset(temp, 0, sizeof temp);
    tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "minRefI") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
    tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -2;
	}
	memset(buf, 0, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->testConfig.minRefI = atol(buf);

	memset(temp, 0, sizeof temp);
    tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "failDeltaV") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
    tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -2;
	}
	memset(buf, 0, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->testConfig.failDeltaV = atol(buf);

	memset(temp, 0, sizeof temp);
    tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "failDeltaI") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
    tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -2;
	}
	memset(buf, 0, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->testConfig.failDeltaI = atol(buf);

	memset(temp, 0, sizeof temp);
    tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "check_lowerI") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
    tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -2;
	}
	memset(buf, 0, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->testConfig.check_lowerI = atol(buf);

	memset(temp, 0, sizeof temp);
    tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "check_lowerOCV") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
    tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -2;
	}
	memset(buf, 0, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->testConfig.check_lowerOCV = atol(buf);

	memset(temp, 0, sizeof temp);
    tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "charge_lower_dv") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
    tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -2;
	}
	memset(buf, 0, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->testConfig.fail_charge_lower_dv = atol(buf);

    fclose(fp);
	return 0;
}

int Read_sensor_limit(void)
{
	char temp[32], buf[32], fileName[256];
    int tmp, i;
    FILE *fp;

	memset((char *)&myPs->sensor_limit, 0, sizeof(S_P2_SENSOR_LIMIT));

	memset(fileName, 0, sizeof fileName);
	strcpy(fileName, "/root/");
	strcat(fileName, (char *)&myData->AppControl.misc.path1);
	strcat(fileName, "/config/parameter/client/");
	strcat(fileName, psName);
	strcat(fileName, "/sensor_limit");
	// /root/system_data/config/parameter/client/COB?/sensor_limit
    if((fp = fopen(fileName, "r")) == NULL) {
		userlog(DEBUG_LOG, psName, "%s sensor_limit file read error\n",
		psName);
		return -1;
	}

	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "warning1") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -2;
	}
	memset(buf, 0, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	myPs->sensor_limit.warning1 = (unsigned short)atoi(buf);

	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "warning2") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -2;
	}
	memset(buf, 0, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	myPs->sensor_limit.warning2 = (unsigned short)atoi(buf);

	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "maxLimit") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -2;
	}
	memset(buf, 0, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	myPs->sensor_limit.sensorSet1.maxLimit = atol(buf);

	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "limitFlag") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -2;
	}
	memset(buf, 0, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	myPs->sensor_limit.sensorSet1.limitFlag = (unsigned char)atoi(buf);

	for(i=0; i < MAX_P2_SENSOR_CH; i++) {
		memset(temp, 0, sizeof temp);
		tmp = fscanf(fp, "%s", temp);
		memset(buf, 0, sizeof buf);
		sprintf(buf, "useSensorFlag[%d]", i+1);
		if(strcmp(temp, buf) != 0) {
			fclose(fp);
			return -2;
		}
		memset(temp, 0, sizeof temp);
		tmp = fscanf(fp, "%s", temp);
		if(strcmp(temp, ":") != 0) {
			fclose(fp);
			return -2;
		}

		memset(buf, 0, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		myPs->sensor_limit.sensorSet1.useSensorFlag[i]
			= (unsigned char)atoi(buf);
	}

	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "maxLimit") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -2;
	}
	memset(buf, 0, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	myPs->sensor_limit.sensorSet2.maxLimit = atol(buf);

	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "limitFlag") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -2;
	}
	memset(buf, 0, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	myPs->sensor_limit.sensorSet2.limitFlag = (unsigned char)atoi(buf);

	for(i=0; i < MAX_P2_SENSOR_CH; i++) {
		memset(temp, 0, sizeof temp);
		tmp = fscanf(fp, "%s", temp);
		memset(buf, 0, sizeof buf);
		sprintf(buf, "useSensorFlag[%d]", i+1);
		if(strcmp(temp, buf) != 0) {
			fclose(fp);
			return -2;
		}
		memset(temp, 0, sizeof temp);
		tmp = fscanf(fp, "%s", temp);
		if(strcmp(temp, ":") != 0) {
			fclose(fp);
			return -2;
		}

		memset(buf, 0, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		myPs->sensor_limit.sensorSet2.useSensorFlag[i]
			= (unsigned char)atoi(buf);
	}

	fclose(fp);
	return 0;
}

int Write_sensor_limit(void)
{
	char fileName[256];
	int i;
    FILE *fp;

	memset(fileName, 0, sizeof fileName);
	strcpy(fileName, "/root/");
	strcat(fileName, (char *)&myData->AppControl.misc.path1);
	strcat(fileName, "/config/parameter/client/");
	strcat(fileName, psName);
	strcat(fileName, "/sensor_limit");
	// /root/system_data/config/parameter/client/COB?/sensor_limit
    if((fp = fopen(fileName, "w")) == NULL) {
		userlog(DEBUG_LOG, psName, "%s sensor_limit file write error\n",
		psName);
		return -1;
	}

	fprintf(fp, "warning1	:	%d\n", myPs->sensor_limit.warning1);
	fprintf(fp, "warning2	:	%d\n", myPs->sensor_limit.warning2);
	fprintf(fp, "\n");

	fprintf(fp, "maxLimit	:	%ld\n", myPs->sensor_limit.sensorSet1.maxLimit);
	fprintf(fp, "limitFlag	:	%d\n", myPs->sensor_limit.sensorSet1.limitFlag);
	for(i=0; i < MAX_P2_SENSOR_CH; i++) {
		fprintf(fp, "useSensorFlag[%d]	:	%d\n", i+1,
			myPs->sensor_limit.sensorSet1.useSensorFlag[i]);
	}
	fprintf(fp, "\n");

	fprintf(fp, "maxLimit	:	%ld\n", myPs->sensor_limit.sensorSet2.maxLimit);
	fprintf(fp, "limitFlag	:	%d\n", myPs->sensor_limit.sensorSet2.limitFlag);
	for(i=0; i < MAX_P2_SENSOR_CH; i++) {
		fprintf(fp, "useSensorFlag[%d]	:	%d\n", i+1,
			myPs->sensor_limit.sensorSet2.useSensorFlag[i]);
	}
	fprintf(fp, "\n");

	fclose(fp);
	return 0;
}

int Read_chamber_set(void)
{
	char temp[32], buf[32], fileName[256];
    int tmp;
    FILE *fp;

	memset((char *)&myPs->chamber_set, 0, sizeof(S_P2_CHAMBER_SET));

	memset(fileName, 0, sizeof fileName);
	strcpy(fileName, "/root/");
	strcat(fileName, (char *)&myData->AppControl.misc.path1);
	strcat(fileName, "/config/parameter/client/");
	strcat(fileName, psName);
	strcat(fileName, "/chamber_set");
	// /root/system_data/config/parameter/client/COB?/chamber_set
    if((fp = fopen(fileName, "r")) == NULL) {
		userlog(DEBUG_LOG, psName, "%s chamber_set file read error\n",
		psName);
		return -1;
	}

	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "set_temp") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -2;
	}
	memset(buf, 0, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	myPs->chamber_set.set_temp = (short int)atoi(buf);

	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "set_humidity") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -2;
	}
	memset(buf, 0, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	myPs->chamber_set.set_humidity = (short int)atoi(buf);

	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "set_time") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -2;
	}
	memset(buf, 0, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	myPs->chamber_set.set_time = atol(buf);

	fclose(fp);
	return 0;
}

int Write_chamber_set(void)
{
	char fileName[256];
    FILE *fp;

	memset(fileName, 0, sizeof fileName);
	strcpy(fileName, "/root/");
	strcat(fileName, (char *)&myData->AppControl.misc.path1);
	strcat(fileName, "/config/parameter/client/");
	strcat(fileName, psName);
	strcat(fileName, "/chamber_set");
	// /root/system_data/config/parameter/client/COB?/chamber_set
    if((fp = fopen(fileName, "w")) == NULL) {
		userlog(DEBUG_LOG, psName, "%s chamber_set file write error\n",
		psName);
		return -1;
	}

	fprintf(fp, "set_temp		:	%d\n", myPs->chamber_set.set_temp);
	fprintf(fp, "set_humidity	:	%d\n", myPs->chamber_set.set_humidity);
	fprintf(fp, "set_time		:	%ld\n", myPs->chamber_set.set_time);

	fclose(fp);
	return 0;
}

int Read_job_change_set(void)
{
	char temp[32], buf[32], fileName[256];
    int tmp;
    FILE *fp;

	//kjg_delete_110719 memset((char *)&myPs->chamber_set, 0, sizeof(S_P2_CHAMBER_SET));

	memset(fileName, 0, sizeof fileName);
	strcpy(fileName, "/root/");
	strcat(fileName, (char *)&myData->AppControl.misc.path1);
	strcat(fileName, "/config/parameter/client/");
	strcat(fileName, psName);
	strcat(fileName, "/job_change_set");
	// /root/system_data/config/parameter/client/COB?/job_change_set
    if((fp = fopen(fileName, "r")) == NULL) {
		userlog(DEBUG_LOG, psName, "%s job_change_set file read error\n",
		psName);
		return -1;
	}

	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "jobModel") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -2;
	}
	memset(buf, 0, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	myPs->misc.jobModel = (unsigned char)atoi(buf);

	fclose(fp);
	return 0;
}

int Write_job_change_set(int jobModel)
{
	char fileName[256];
    FILE *fp;

	memset(fileName, 0, sizeof fileName);
	strcpy(fileName, "/root/");
	strcat(fileName, (char *)&myData->AppControl.misc.path1);
	strcat(fileName, "/config/parameter/client/");
	strcat(fileName, psName);
	strcat(fileName, "/job_change_set");
	// /root/system_data/config/parameter/client/COB?/job_change_set
    if((fp = fopen(fileName, "w")) == NULL) {
		userlog(DEBUG_LOG, psName, "%s job_change_set file write error\n",
		psName);
		return -1;
	}

	fprintf(fp, "jobModel	:	%d\n", jobModel);

	fclose(fp);
	return 0;
}

void CaliUpdateCh(int ch)
{
	int type, range, size;

	for(type=0; type < MAX_TYPE; type++) {
		for(range=0; range < MAX_RANGE; range++) {
			if(myData->cali[ch].tmpData[type][range].caliFlag == 0) continue;
			myData->cali[ch].tmpData[type][range].caliFlag = 0;

			memcpy((char *)&myData->cali[ch].data[type][range].point,
				(char *)&myData->cali[ch].tmpCond[type][range].point,
				sizeof(S_CALI_POINT));

			size = sizeof(long) * MAX_CALI_POINT * 4
				+ sizeof(double) * (MAX_CALI_POINT - 1) * 4
				+ sizeof(double) * 2;
			memcpy((char *)&myData->cali[ch].data[type][range].set_ad,
				(char *)&myData->cali[ch].tmpData[type][range].set_ad, size);
		}
	}
}

int Write_BdCaliData(int bd)
{
	char temp[4], fileName[256];
    int	i, ch, type, range, point, chOffset;
    FILE *fp;

	memset(fileName, 0, sizeof(fileName));
	strcpy(fileName, "/root/");
	strcat(fileName, (char *)&myData->AppControl.misc.path1);
	strcat(fileName, "/config/caliData/CALI_BD");
	memset(temp, 0, sizeof temp);
	temp[0] = (char)(49+bd);
	strcat(fileName, temp);
	// /root/system_data/config/caliData/CALI_BD#
	
    if((fp = fopen(fileName, "w")) == NULL) {
		userlog(DEBUG_LOG, psName,
			"Can not open %d Board CaliData file(write)\n", bd+1);
		return -1;
	}
	
	fprintf(fp, "MCTS_Calibration_File\n");
	fprintf(fp, "KJG001-00\n");
	fprintf(fp, "%s\n\n", "date"); //kjgw
	
	for(type=0; type < MAX_TYPE; type++) {
		if(type == 0) {
			fprintf(fp, "voltage\n");
		} else {
			fprintf(fp, "current\n");
		}

		for(range=0; range < MAX_RANGE; range++) {
			if((range+1) == RANGE1) {
				fprintf(fp, "range1\n");
			} else if((range+1) == RANGE2) {
				fprintf(fp, "range2\n");
			} else if((range+1) == RANGE3) {
				fprintf(fp, "range3\n");
			} else {
				fprintf(fp, "range4\n");
			}

			chOffset = myPs->misc.chOffset + bd * myData->mData.config.chPerBd;
			for(i=0; i < P2_MAX_CH_PER_BD; i++) {
	    		fprintf(fp, "ch%02d\n", i+1);

				ch = chOffset + i;
				fprintf(fp, "setPointNum   \n");
			   	fprintf(fp, "%d ", myData->cali[ch].data[type][range]
					.point.setPointNum);
				fprintf(fp, "\n");

				fprintf(fp, "setPoint      \n");
				for(point=0; point < MAX_CALI_POINT; point++) {
			    	fprintf(fp, "%ld ", myData->cali[ch].data[type][range]
						.point.setPoint[point]);
				}
				fprintf(fp, "\n");

				fprintf(fp, "checkPointNum \n");
			   	fprintf(fp, "%d ", myData->cali[ch].data[type][range]
					.point.checkPointNum);
				fprintf(fp, "\n");

				fprintf(fp, "checkPoint    \n");
				for(point=0; point < MAX_CALI_POINT; point++) {
			    	fprintf(fp, "%ld ", myData->cali[ch].data[type][range]
						.point.checkPoint[point]);
				}
				fprintf(fp, "\n");

				fprintf(fp, "set_ad        \n");
				for(point=0; point < MAX_CALI_POINT; point++) {
			    	fprintf(fp, "%ld ", myData->cali[ch].data[type][range]
						.set_ad[point]);
				}
				fprintf(fp, "\n");

				fprintf(fp, "set_meter     \n");
				for(point=0; point < MAX_CALI_POINT; point++) {
			    	fprintf(fp, "%ld ", myData->cali[ch].data[type][range]
						.set_meter[point]);
				}
				fprintf(fp, "\n");

				fprintf(fp, "check_ad      \n");
				for(point=0; point < MAX_CALI_POINT; point++) {
			    	fprintf(fp, "%ld ", myData->cali[ch].data[type][range]
						.check_ad[point]);
				}
				fprintf(fp, "\n");

				fprintf(fp, "check_meter   \n");
				for(point=0; point < MAX_CALI_POINT; point++) {
			    	fprintf(fp, "%ld ", myData->cali[ch].data[type][range]
						.check_meter[point]);
				}
				fprintf(fp, "\n");

				fprintf(fp, "DA_A          \n");
				for(point=0; point < MAX_CALI_POINT-1; point++) {
			    	fprintf(fp, "%f ", (float)myData->cali[ch].data[type][range]
						.DA_A[point]);
				}
				fprintf(fp, "\n");

				fprintf(fp, "DA_B          \n");
				for(point=0; point < MAX_CALI_POINT-1; point++) {
			    	fprintf(fp, "%f ", (float)myData->cali[ch].data[type][range]
						.DA_B[point]);
				}
				fprintf(fp, "\n");

				fprintf(fp, "AD_A          \n");
				for(point=0; point < MAX_CALI_POINT-1; point++) {
			    	fprintf(fp, "%f ", (float)myData->cali[ch].data[type][range]
						.AD_A[point]);
				}
				fprintf(fp, "\n");

				fprintf(fp, "AD_B\n");
				for(point=0; point < MAX_CALI_POINT-1; point++) {
			    	fprintf(fp, "%f ", (float)myData->cali[ch].data[type][range]
						.AD_B[point]);
				}
				fprintf(fp, "\n");

				fprintf(fp, "AD_Ratio      \n");
				fprintf(fp, "%f %f",
					(float)myData->cali[ch].data[type][range].AD_Ratio[0],
					(float)myData->cali[ch].data[type][range].AD_Ratio[1]);
				fprintf(fp, "\n\n");
			}
		}
	}
   	fclose(fp);

	return 0;
}

void Check_GroupState(void)
{
	int group, jig, rtn, i;
	unsigned short bit, flag;

	group = (int)myPs->config.groupNo;
	jig = groupNo_to_jigNo(group);
	if(jig > 0) jig--;
	else return;

	rtn = 0;

	switch(myPs->misc.state) {
		case P2_G_IDLE:
			if(myData->gData[group].workMode == WORK_TYPE_OFFLINE) {
				myPs->misc.state = P2_G_IDLE;
			} else { //ON_LINE
				myPs->misc.state = P2_G_STANDBY;
			}
			break;
		case P2_G_STANDBY:
			if(myData->gData[group].state == G_RUN) {
				myPs->misc.state = P2_G_RUN;
			}
			break;
		case P2_G_RUN:
			if(myData->gData[group].state == G_STANDBY) {
				myPs->misc.state = P2_G_STANDBY;
			}
			if(myData->gData[group].state == G_PAUSE) {
				myPs->misc.state = P2_G_PAUSE;
			}
			break;
		case P2_G_PAUSE:
			if(myData->gData[group].state == G_STANDBY) {
				myPs->misc.state = P2_G_STANDBY;
			}
			if(myData->gData[group].state == G_RUN) {
				myPs->misc.state = P2_G_RUN;
			}
			break;
		default:	break;
	}

	if(myPs->misc.tmpState != myPs->misc.state) {
		rtn = 1;
		//userlog(DEBUG_LOG, psName, "kjg_d %d %d %d\n", rtn,
		//	myPs->misc.tmpState, myPs->misc.state);
	}

	if(myPs->misc.tmpCode != myPs->misc.code) {
		rtn = 2;
		//userlog(DEBUG_LOG, psName, "kjg_d %d %d %d\n", rtn,
		//	myPs->misc.tmpCode, myPs->misc.code);
	}

	if(myData->jData[jig].jigState == J_CONTACT) {
		bit = 0x01;
	} else bit = OFF;
	if(myPs->misc.tmpSensorState[J_S_JIGSTATE_CHECK] != bit) {
		rtn = 3;
		//userlog(DEBUG_LOG, psName, "kjg_d %d %d %d\n", rtn,
		//	myPs->misc.tmpSensorState[J_S_JIGSTATE_CHECK], bit);
	}

	bit = 0x0000;
	for(i=0; i < myData->jData[jig].config.totalTrayNo; i++) {
		flag = 0x0001 << i;
		if(myData->jData[jig].trayState[i] == T_LOAD) bit |= flag;
		else bit &= ~flag;
	}
	if(myPs->misc.tmpSensorState[J_S_TRAYSTATE_CHECK] != bit) {
		rtn = 4;
		//userlog(DEBUG_LOG, psName, "kjg_d %d %d %d\n", rtn,
		//	myPs->misc.tmpSensorState[J_S_TRAYSTATE_CHECK], bit);
	}

	switch(myData->AppControl.config.systemModel) {
		case F_SDI_5V_450A_200A_100A_10A:
			bit = (unsigned short)myPs->misc.jobModel;
			break;
		default:
			if(myData->jData[jig].stopperState == J_CONTACT) {
				bit = 0x01;
			} else bit = OFF;
			break;
	}
	if(myPs->misc.tmpSensorState[J_S_STOPPERSTATE_CHECK] != bit) {
		rtn = 5;
		//userlog(DEBUG_LOG, psName, "kjg_d %d %d %d\n", rtn,
		//	myPs->misc.tmpSensorState[J_S_STOPPERSTATE_CHECK], bit);
	}

	bit = 0x0000;
	for(i=0; i < MAX_TRAY_PER_GROUP; i++) {
		flag = 0x0001 << i;
		if(myData->jData[jig].doorState[i] == D_CLOSE) bit |= flag;
		else bit &= ~flag;
	}
	if(myPs->misc.tmpSensorState[J_S_DOORSTATE_CHECK] != bit) {
		rtn = 6;
		//userlog(DEBUG_LOG, psName, "kjg_d %d %d %d\n", rtn,
		//	myPs->misc.tmpSensorState[J_S_DOORSTATE_CHECK], bit);
	}

	flag = 0;
	switch(myData->AppControl.config.systemModel) {
		case F_SDI_5V_50A_5A:
			if(group == 0) {
				i = MAX_DIO_II_BYTES + MAX_DIO_MI_BYTES + MAX_DIO_CI_BYTES;
			} else {
				i = MAX_DIO_II_BYTES + MAX_DIO_MI_BYTES + MAX_DIO_CI_BYTES + 3;
			}
			bit = (unsigned short)myData->dio.in.bytes[i+1];
			bit = bit << 8;
			bit |= (unsigned short)myData->dio.in.bytes[i];
			if(myPs->misc.tmpSensorState[J_S_IN_BITS1] != bit) flag = 1;

			bit = (unsigned short)myData->dio.in.bytes[i+2];
			if(myPs->misc.tmpSensorState[J_S_IN_BITS2] != bit) flag = 2;
			break;
		case F_SDI_5V_400A_200A_100A_10A:
			i = MAX_DIO_II_BYTES;
			bit = (unsigned short)myData->dio.in.bytes[i+1];
			bit = bit << 8;
			bit |= (unsigned short)myData->dio.in.bytes[i];
			if(myPs->misc.tmpSensorState[J_S_IN_BITS1] != bit) flag = 1;

			bit = (unsigned short)myData->dio.in.bytes[i+2];
			if(myPs->misc.tmpSensorState[J_S_IN_BITS2] != bit) flag = 2;
			break;
		case F_SDI_5V_400A_200A_100A_10A_2:
			if(group == 0) {
				i = MAX_DIO_II_BYTES;
				bit = (unsigned short)myData->dio.in.bytes[i+1] & 0x0038;
				bit = bit << 8;
				bit |= ((unsigned short)myData->dio.in.bytes[i] & 0x0023);
				if(myPs->misc.tmpSensorState[J_S_IN_BITS1] != bit) flag = 1;

				bit = (unsigned short)myData->dio.in.bytes[i+2];
				if(myPs->misc.tmpSensorState[J_S_IN_BITS2] != bit) flag = 2;
			} else {
				i = MAX_DIO_II_BYTES;
				bit = (unsigned short)myData->dio.in.bytes[i+1] & 0x0007;
				bit = bit << 8;
				bit |= ((unsigned short)myData->dio.in.bytes[i] & 0x0007);
				if(myPs->misc.tmpSensorState[J_S_IN_BITS1] != bit) flag = 1;

				bit = (unsigned short)myData->dio.in.bytes[i+2];
				if(myPs->misc.tmpSensorState[J_S_IN_BITS2] != bit) flag = 2;
			}
			break;
		case F_SDI_5V_450A_200A_100A_10A: //kjgw
			if(group == 0) {
				i = MAX_DIO_II_BYTES;
				bit = (unsigned short)myData->dio.in.bytes[i+1] & 0x0038;
				bit = bit << 8;
				bit |= ((unsigned short)myData->dio.in.bytes[i] & 0x0023);
				if(myPs->misc.tmpSensorState[J_S_IN_BITS1] != bit) flag = 1;

				bit = (unsigned short)myData->dio.in.bytes[i+2];
				if(myPs->misc.tmpSensorState[J_S_IN_BITS2] != bit) flag = 2;
			} else {
				i = MAX_DIO_II_BYTES;
				bit = (unsigned short)myData->dio.in.bytes[i+1] & 0x0007;
				bit = bit << 8;
				bit |= ((unsigned short)myData->dio.in.bytes[i] & 0x0007);
				if(myPs->misc.tmpSensorState[J_S_IN_BITS1] != bit) flag = 1;

				bit = (unsigned short)myData->dio.in.bytes[i+2];
				if(myPs->misc.tmpSensorState[J_S_IN_BITS2] != bit) flag = 2;
			}
			break;
		default: break;
	}
	if(flag != 0) rtn = 7;

	if(rtn != 0) {
		myPs->signal[P2_SIG_SEND_GROUP_STATE] = P1;
	}
}

void convert_test_cond(void)
{
	int ch, idx, i, step, client_step;
	long attr_count, type;
	S_TEST_CONDITION *myTestCond;
	
	ch = myPs->misc.chOffset;
	memset((char *)&myData->testCond[ch], 0, sizeof(S_TEST_CONDITION));
	myTestCond = (S_TEST_CONDITION *)&(myData->testCond[ch]);

	//common_header
	idx = IDX_COM_OBJ_TOTAL_STEP;
	myTestCond->common_object[idx] = (long)myPs->testCond.header.totalStep;

	idx = IDX_COM_OBJ_ATTRIBUTE_COUNT;
	attr_count = 1;
	if(myData->ChAttribute[ch].opType != OP_INDEPENDENT) {
		for(i=0; i < 3; i++) {
			if(myData->ChAttribute[ch].chNo_slave[i] != 0) attr_count++;
		}
	}
	myTestCond->common_object[idx] = attr_count;

	//common_safety
	if(myPs->config.protocol_version == P2_PROTOCOL_VERSION) {
	} else {
		convert_test_cond_common_safety2(myTestCond, ch);
	}
	
	//common_cycle_branch
	//aux_condition
	//can_condition
	
	step = 0;
	if(myPs->config.protocol_version == P2_PROTOCOL_VERSION) {
		if(myPs->testCond.precheck.compFlag == P1) {
			convert_test_cond_check(myTestCond, ch);
			step += 3;
		}
	} else {
		if(myPs->testCond.precheck2.compFlag == P1) {
			convert_test_cond_check2(myTestCond, ch);
			step += 4;
		} else {
			convert_test_cond_check3(myTestCond, ch);
			step += 4;
		}
	}

//	userlog(DEBUG_LOG, psName, "precheck.compFlag %d, %d\n",
//		myPs->testCond.precheck.compFlag,
//		myPs->testCond.precheck2.compFlag); //kjgd

	//step_cycle
	idx = IDX_LOC_OBJ_CLIENT_STEP_NO;
	myTestCond->local_object[step][idx] = 1;

	idx = IDX_LOC_OBJ_STEP_NO;
	myTestCond->local_object[step][idx] = (long)step;

	idx = IDX_LOC_OBJ_TYPE;
	myTestCond->local_object[step][idx] = STEP_CYCLE;

	//step_condition
	for(client_step=0; client_step < myPs->testCond.header.totalStep-1;
		client_step++) {
		step++;

		//header
		idx = IDX_LOC_OBJ_CLIENT_STEP_NO;
		myTestCond->local_object[step][idx] = (long)client_step+1;

		idx = IDX_LOC_OBJ_STEP_NO;
		myTestCond->local_object[step][idx] = (long)step;

		idx = IDX_LOC_OBJ_TYPE;
		type = convert_step_type(CONVERT_P2_TO_ORG,
			(long)myPs->testCond.stepHeader[client_step].type);
		myTestCond->local_object[step][idx] = type;

		switch(type) {
			case STEP_IDLE:
			case STEP_END:
			case STEP_CYCLE:
			case STEP_LOOP:
				break;
			case STEP_OCV:
				convert_test_cond_ocv(myTestCond, ch, step, client_step);
				break;
			case STEP_REST:
				convert_test_cond_rest(myTestCond, ch, step, client_step);
				break;
			case STEP_LONG_TIME_REST:
				convert_test_cond_long_time_rest(myTestCond, ch, step,
					client_step);
				break;
			case STEP_CHARGE:
				convert_test_cond_charge(myTestCond, ch, step, client_step);
				break;
			case STEP_DISCHARGE:
				convert_test_cond_discharge(myTestCond, ch, step, client_step);
				break;
			case STEP_Z:
				convert_test_cond_z(myTestCond, ch, step, client_step);
				break;
			case STEP_PATTERN:
				convert_test_cond_pattern(myTestCond, ch, step, client_step);
				break;
			default:
				//convert_test_cond_default(ch, step, type, attr_count);
				break;
		}
	}

	//step_loop
	step++;

	idx = IDX_LOC_OBJ_CLIENT_STEP_NO;
	myTestCond->local_object[step][idx] = client_step+1;

	idx = IDX_LOC_OBJ_STEP_NO;
	myTestCond->local_object[step][idx] = (long)step;

	idx = IDX_LOC_OBJ_TYPE;
	myTestCond->local_object[step][idx] = STEP_LOOP;

	idx = IDX_LOC_OBJ_END_CYCLE_COUNT;
	myTestCond->local_object[step][idx] = 1;

	idx = IDX_LOC_OBJ_END_CYCLE_COUNT_BRANCH;
	myTestCond->local_object[step][idx] = ACTIVE_DIV_NEXT;

	//step_end
	step++;

	idx = IDX_LOC_OBJ_CLIENT_STEP_NO;
	myTestCond->local_object[step][idx] = (long)client_step+1;

	idx = IDX_LOC_OBJ_STEP_NO;
	myTestCond->local_object[step][idx] = (long)step;

	idx = IDX_LOC_OBJ_TYPE;
	myTestCond->local_object[step][idx] = STEP_END;
}

void convert_test_cond_common_safety2(S_TEST_CONDITION *myTestCond, int ch)
{
	int idx;

	idx = IDX_COM_OBJ_FAULT_HW_UPPER_V;
	myTestCond->common_object[idx] = 5000; //5.0%
	make2_com_chCode_cond(ch, C_CD_COM_FAULT_HW_UPPER_V, 1,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_LOC_OBJ_FAULT_HW_LOWER_V;
	myTestCond->common_object[idx] = 5000; //5.0%
	make2_com_chCode_cond(ch, C_CD_COM_FAULT_HW_LOWER_V, 1,
		//COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>= kjg_change 091109

	idx = IDX_COM_OBJ_FAULT_HW_UPPER_I;
	myTestCond->common_object[idx] = 5000; //5.0%
	make2_com_chCode_cond(ch, C_CD_COM_FAULT_HW_UPPER_I, 1,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_LOC_OBJ_FAULT_HW_LOWER_I;
	myTestCond->common_object[idx] = 5000; //5.0%
	make2_com_chCode_cond(ch, C_CD_COM_FAULT_HW_LOWER_I, 1,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_COM_OBJ_FAULT_CHARGE_UPPER_V;
	myTestCond->common_object[idx]
		= myPs->testCond.common_safety.charge_limit_voltage * 100;
		//= 4500000; //kjgd
	make2_com_chCode_cond(ch, C_CD_COM_FAULT_CHARGE_UPPER_V, 1,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_COM_OBJ_FAULT_CHARGE_UPPER_AMPARE_HOUR;
	myTestCond->common_object[idx]
		= myPs->testCond.common_safety.charge_limit_capacity * 10;
		//= 0; //kjgd
	make2_com_chCode_cond(ch, C_CD_COM_FAULT_CHARGE_UPPER_AMPARE_HOUR, 1,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_COM_OBJ_FAULT_CHARGE_END_UPPER_V;
	myTestCond->common_object[idx]
		= myPs->testCond.common_safety.over_charge_voltage * 100;
		//= 4300000; //kjgd
	make2_com_chCode_cond(ch, C_CD_COM_FAULT_CHARGE_END_UPPER_V, 1,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_COM_OBJ_FAULT_CHARGE_LOWER_I;
	myTestCond->common_object[idx]
		= myPs->testCond.common_safety.charge_lower_current_limit * 10;
		//= 500000; //kjgd
	make2_com_chCode_cond(ch, C_CD_COM_FAULT_CHARGE_LOWER_I, 1,
		COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=

	idx = IDX_COM_OBJ_FAULT_DISCHARGE_LOWER_V;
	myTestCond->common_object[idx]
		= myPs->testCond.common_safety.discharge_limit_voltage * 100;
		//= 2500000; //kjgd
	make2_com_chCode_cond(ch, C_CD_COM_FAULT_DISCHARGE_LOWER_V, 1,
		COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=

	idx = IDX_COM_OBJ_FAULT_DISCHARGE_RUN_TIME;
	myTestCond->common_object[idx]
		= myPs->testCond.common_safety.discharge_limit_time * 10;
		//= 30000; //kjgd
	make2_com_chCode_cond(ch, C_CD_COM_FAULT_DISCHARGE_RUN_TIME, 1,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_COM_OBJ_FAULT_DISCHARGE_LOWER_AMPARE_HOUR;
	myTestCond->common_object[idx]
		= myPs->testCond.common_safety.discharge_lower_limit_capacity * 10;
		//= 500000; //kjgd
	make2_com_chCode_cond(ch, C_CD_COM_FAULT_DISCHARGE_LOWER_AMPARE_HOUR, 1,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_COM_OBJ_FAULT_OVER_CURRENT_LIMIT;
	myTestCond->common_object[idx]
		= myPs->testCond.common_safety.over_current_limit * 10;
		//= 55000000; //kjgd
	make2_com_chCode_cond(ch, C_CD_COM_FAULT_OVER_CURRENT_LIMIT, 1,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_COM_OBJ_FAULT_COMP_V_T1;
	myTestCond->common_object[idx]
		= myPs->testCond.common_safety.charge_check_time * 10;
		//= 1000; //kjgd
	idx = IDX_COM_OBJ_FAULT_COMP_LOWER_V1;
	myTestCond->common_object[idx]
		= myPs->testCond.common_safety.charge_check_voltage * 100;
		//= 2700000; //kjgd

	idx = IDX_COM_OBJ_FAULT_COMP_V_T2;
	myTestCond->common_object[idx]
		= myPs->testCond.common_safety.comp_value1[0][0] * 10;
		//= 1200; //kjgd
	idx = IDX_COM_OBJ_FAULT_COMP_UPPER_V2;
	myTestCond->common_object[idx]
		= myPs->testCond.common_safety.comp_value1[1][0] * 100;
		//= 4500000; //kjgd
	idx = IDX_COM_OBJ_FAULT_COMP_LOWER_V2;
	myTestCond->common_object[idx]
		= myPs->testCond.common_safety.comp_value1[2][0] * 100;
		//= 2700000; //kjgd

	idx = IDX_COM_OBJ_FAULT_COMP_V_T3;
	myTestCond->common_object[idx]
		= myPs->testCond.common_safety.comp_value1[0][1] * 10;
		//= 200; //kjgd
	idx = IDX_COM_OBJ_FAULT_COMP_UPPER_V3;
	myTestCond->common_object[idx]
		= myPs->testCond.common_safety.comp_value1[1][1] * 100;
		//= 4500000; //kjgd
	idx = IDX_COM_OBJ_FAULT_COMP_LOWER_V3;
	myTestCond->common_object[idx]
		= myPs->testCond.common_safety.comp_value1[2][1] * 100;
		//= 2700000; //kjgd

	idx = IDX_COM_OBJ_FAULT_COMP_LOWER_CV_TIME;
	myTestCond->common_object[idx]
		= myPs->testCond.common_safety.comp_value1[0][2] * 10;
		//= 1000; //kjgd
	idx = IDX_COM_OBJ_FAULT_COMP_UPPER_CV_TIME;
	myTestCond->common_object[idx]
		= myPs->testCond.common_safety.comp_value1[0][3] * 10;
		//= 2000; //kjgd

	idx = IDX_COM_OBJ_FAULT_COMP_I_T1;
	myTestCond->common_object[idx]
		= myPs->testCond.common_safety.comp_value1[0][4] * 10;
		//= 3000; //kjgd
	idx = IDX_COM_OBJ_FAULT_COMP_UPPER_I1;
	myTestCond->common_object[idx]
		= myPs->testCond.common_safety.comp_value1[1][4] * 10;
		//= 500000; //kjgd
	idx = IDX_COM_OBJ_FAULT_COMP_LOWER_I1;
	myTestCond->common_object[idx]
		= myPs->testCond.common_safety.comp_value1[2][4] * 10;
		//= 100000; //kjgd
}

void convert_test_cond_check(S_TEST_CONDITION *myTestCond, int ch)
{
	int step, idx, attr_count;
	long refV, refI, l_val, l_val2;
	
	idx = IDX_COM_OBJ_ATTRIBUTE_COUNT;
	attr_count = myTestCond->common_object[idx];

	//step_cycle
	step = 0;

	idx = IDX_LOC_OBJ_CLIENT_STEP_NO;
	myTestCond->local_object[step][idx] = 0;

	idx = IDX_LOC_OBJ_STEP_NO;
	myTestCond->local_object[step][idx] = (long)step;

	idx = IDX_LOC_OBJ_TYPE;
	myTestCond->local_object[step][idx] = STEP_CYCLE;

	idx = IDX_LOC_OBJ_ATTRIBUTE;
	myTestCond->local_object[step][idx] = ATTR_CHECK1_START;

	//step_charge
	step++;

	idx = IDX_LOC_OBJ_CLIENT_STEP_NO;
	myTestCond->local_object[step][idx] = 0;

	idx = IDX_LOC_OBJ_STEP_NO;
	myTestCond->local_object[step][idx] = (long)step;

	idx = IDX_LOC_OBJ_TYPE;
	myTestCond->local_object[step][idx] = STEP_CHARGE;

	idx = IDX_LOC_OBJ_ATTRIBUTE;
	myTestCond->local_object[step][idx] = ATTR_CHECK1_1;

	idx = IDX_LOC_OBJ_MODE;
	myTestCond->local_object[step][idx] = MODE_CC_CV;

	idx = IDX_LOC_OBJ_END_TIME;
	myTestCond->local_object[step][idx]
		= (long)myPs->testCond.precheck.endTime * 10;
	make2_loc_chCode_cond(ch, step, C_CD_END_TIME, 1,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_LOC_OBJ_END_TIME_BRANCH;
	myTestCond->local_object[step][idx] = ACTIVE_DIV_NEXT;

	idx = IDX_LOC_OBJ_SAVE_DELTA_T;
	myTestCond->local_object[step][idx] = 100; //1sec
	make2_loc_chCode_cond(ch, step, C_CD_SAVE_DELTA_T, 1,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_LOC_OBJ_REF_V;
	refV = myPs->testCond.precheck.refV * 100;
	if(refV > myData->mData.config.maxV[0]) {
		refV = myData->mData.config.maxV[0];
	} else if(refV < myData->mData.config.minV[0]) {
		refV = myData->mData.config.minV[0];
	}
	myTestCond->local_object[step][idx] = refV;

	idx = IDX_LOC_OBJ_REF_V2;
	myTestCond->local_object[step][idx] = myData->mData.config.minV[0];

	idx = IDX_LOC_OBJ_RANGE_V;
	myTestCond->local_object[step][idx] = RANGE1 - 1;

	idx = IDX_LOC_OBJ_REF_I;
	refI = myPs->testCond.precheck.refI * 10;
	myTestCond->local_object[step][idx] = refI;

	idx = IDX_LOC_OBJ_RANGE_I;
	if(refI > 0) {
		if(refI <= (myData->mData.config.maxI[3] * attr_count)) {
			l_val = RANGE4 - 1;
		} else if(refI <= (myData->mData.config.maxI[2] * attr_count)) {
			l_val = RANGE3 - 1;
		} else if(refI <= (myData->mData.config.maxI[1] * attr_count)) {
			l_val = RANGE2 - 1;
		} else {
			l_val = RANGE1 - 1;
		}
		if((l_val+1) > (long)myData->mData.config.rangeI) {
			l_val = (long)myData->mData.config.rangeI - 1;
		}

		l_val2 = 0;
		if(l_val2 == 0) { //auto range
			myTestCond->local_object[step][idx] = l_val;
		} else { //manual range
			if(l_val2 < l_val) {
				myTestCond->local_object[step][idx] = l_val2;
			} else {
				myTestCond->local_object[step][idx] = l_val;
			}
		}
	} else if(refI < 0) {
		if(refI >= (myData->mData.config.minI[3] * attr_count)) {
			l_val = RANGE4 - 1;
		} else if(refI >= (myData->mData.config.minI[2] * attr_count)) {
			l_val = RANGE3 - 1;
		} else if(refI >= (myData->mData.config.minI[1] * attr_count)) {
			l_val = RANGE2 - 1;
		} else {
			l_val = RANGE1 - 1;
		}
		if((l_val+1) > (long)myData->mData.config.rangeI) {
			l_val = (long)myData->mData.config.rangeI - 1;
		}

		l_val2 = 0;
		if(l_val2 == 0) { //auto range
			myTestCond->local_object[step][idx] = l_val;
		} else { //manual range
			if(l_val2 < l_val) {
				myTestCond->local_object[step][idx] = l_val2;
			} else {
				myTestCond->local_object[step][idx] = l_val;
			}
		}
	} else {
		if(step == 0) {
			l_val = RANGE1 - 1;
		} else {
			l_val = myTestCond->local_object[step-1][idx];
		}
		myTestCond->local_object[step][idx] = l_val;
	}

	idx = IDX_LOC_OBJ_FAULT_CHECK_UPPER_V;
	myTestCond->local_object[step][idx] = refV + myPs->testConfig.failDeltaV;
	make2_loc_chCode_cond(ch, step, C_CD_FAULT_CHECK_UPPER_V, 1,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_LOC_OBJ_FAULT_CHECK_LOWER_V;
	myTestCond->local_object[step][idx] = myPs->testConfig.check_lowerOCV;
	make2_loc_chCode_cond(ch, step, C_CD_FAULT_CHECK_LOWER_V, 1,
		COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=

	idx = IDX_LOC_OBJ_FAULT_CHECK_UPPER_I;
	myTestCond->local_object[step][idx] = labs((float)refI * 1.2);
	make2_loc_chCode_cond(ch, step, C_CD_FAULT_CHECK_UPPER_I, 1,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_LOC_OBJ_FAULT_CHECK_LOWER_I;
	myTestCond->local_object[step][idx] = myPs->testConfig.check_lowerI;
	make2_loc_chCode_cond(ch, step, C_CD_FAULT_CHECK_LOWER_I, 1,
		COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=

	//step_loop
	step++;

	idx = IDX_LOC_OBJ_CLIENT_STEP_NO;
	myTestCond->local_object[step][idx] = 0;

	idx = IDX_LOC_OBJ_STEP_NO;
	myTestCond->local_object[step][idx] = (long)step;

	idx = IDX_LOC_OBJ_TYPE;
	myTestCond->local_object[step][idx] = STEP_LOOP;

	idx = IDX_LOC_OBJ_ATTRIBUTE;
	myTestCond->local_object[step][idx] = ATTR_CHECK1_END;

	idx = IDX_LOC_OBJ_END_CYCLE_COUNT;
	myTestCond->local_object[step][idx] = 1;

	idx = IDX_LOC_OBJ_END_CYCLE_COUNT_BRANCH;
	myTestCond->local_object[step][idx] = ACTIVE_DIV_NEXT;
}

void convert_test_cond_check2(S_TEST_CONDITION *myTestCond, int ch)
{
	int step, idx, attr_count;
	long refV, refI, l_val, l_val2;
	
	idx = IDX_COM_OBJ_ATTRIBUTE_COUNT;
	attr_count = myTestCond->common_object[idx];

	//step_cycle
	step = 0;

	idx = IDX_LOC_OBJ_CLIENT_STEP_NO;
	myTestCond->local_object[step][idx] = 0;

	idx = IDX_LOC_OBJ_STEP_NO;
	myTestCond->local_object[step][idx] = (long)step;

	idx = IDX_LOC_OBJ_TYPE;
	myTestCond->local_object[step][idx] = STEP_CYCLE;

	idx = IDX_LOC_OBJ_ATTRIBUTE;
	myTestCond->local_object[step][idx] = ATTR_CHECK2_START;

	//step_ocv
	step++;

	idx = IDX_LOC_OBJ_CLIENT_STEP_NO;
	myTestCond->local_object[step][idx] = 0;

	idx = IDX_LOC_OBJ_STEP_NO;
	myTestCond->local_object[step][idx] = (long)step;

	idx = IDX_LOC_OBJ_TYPE;
	myTestCond->local_object[step][idx] = STEP_OCV;

	idx = IDX_LOC_OBJ_ATTRIBUTE;
	myTestCond->local_object[step][idx] = ATTR_CHECK2_1;

//	make2_loc_chCode_cond(ch, step, C_CD_END_OCV, 1, COMP_NONE, 0);

	idx = IDX_LOC_OBJ_END_TIME;
	myTestCond->local_object[step][idx] = 100;
	//myTestCond->local_object[step][idx] = 200;
	make2_loc_chCode_cond(ch, step, C_CD_END_TIME, 1,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_LOC_OBJ_END_TIME_BRANCH;
	myTestCond->local_object[step][idx] = ACTIVE_DIV_NEXT;

	idx = IDX_LOC_OBJ_SAVE_DELTA_T;
	myTestCond->local_object[step][idx] = 100; //1sec
	make2_loc_chCode_cond(ch, step, C_CD_SAVE_DELTA_T, 1,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_LOC_OBJ_FAULT_CHECK_CONTACT_BAD1;
	myTestCond->local_object[step][idx] = 1;
	make2_loc_chCode_cond(ch, step, C_CD_FAULT_CHECK_CONTACT_BAD1, 1,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_LOC_OBJ_FAULT_CHECK_CONTACT_BAD2;
	myTestCond->local_object[step][idx] = 1;
	make2_loc_chCode_cond(ch, step, C_CD_FAULT_CHECK_CONTACT_BAD2, 1,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_LOC_OBJ_FAULT_CHECK_CONTACT_BAD3;
	myTestCond->local_object[step][idx] = 10000; //10mV
	make2_loc_chCode_cond(ch, step, C_CD_FAULT_CHECK_CONTACT_BAD3, 1,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_LOC_OBJ_FAULT_CHECK_DETECT_V;
	myTestCond->local_object[step][idx]
		= myPs->testCond.precheck2.detect_voltage * 100;
		//= 2500000; //kjgd
	make2_loc_chCode_cond(ch, step, C_CD_FAULT_CHECK_DETECT_V, 1,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_LOC_OBJ_FAULT_CHECK_REVERSE_V;
	myTestCond->local_object[step][idx]
		= myPs->testCond.precheck2.reverse_voltage * 100;
	if(myTestCond->local_object[step][idx] <= 0) { //kjg_update_100215
		myTestCond->local_object[step][idx] = -100000;
	}
	make2_loc_chCode_cond(ch, step, C_CD_FAULT_CHECK_REVERSE_V, 1,
		COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=

	idx = IDX_LOC_OBJ_FAULT_CHECK_LOWER_OCV;
	myTestCond->local_object[step][idx]
		= myPs->testCond.precheck2.lowerOCV * 100;
	if(myTestCond->local_object[step][idx] == 0) {
		myTestCond->local_object[step][idx] = 1;
	} //kjg_change 091109
	make2_loc_chCode_cond(ch, step, C_CD_FAULT_CHECK_LOWER_OCV, 1,
		COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=

	idx = IDX_LOC_OBJ_FAULT_CHECK_UPPER_OCV;
	myTestCond->local_object[step][idx]
		= myPs->testCond.precheck2.upperOCV * 100;
		//= 2000000; //kjgd
	make2_loc_chCode_cond(ch, step, C_CD_FAULT_CHECK_UPPER_OCV, 1,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	//step_charge
	step++;

	idx = IDX_LOC_OBJ_CLIENT_STEP_NO;
	myTestCond->local_object[step][idx] = 0;

	idx = IDX_LOC_OBJ_STEP_NO;
	myTestCond->local_object[step][idx] = (long)step;

	idx = IDX_LOC_OBJ_TYPE;
	myTestCond->local_object[step][idx] = STEP_CHARGE;

	idx = IDX_LOC_OBJ_ATTRIBUTE;
	myTestCond->local_object[step][idx] = ATTR_CHECK2_2;

	idx = IDX_LOC_OBJ_MODE;
	myTestCond->local_object[step][idx] = MODE_CC_CV;

	idx = IDX_LOC_OBJ_END_TIME;
	myTestCond->local_object[step][idx]
		= (long)myPs->testCond.precheck2.endTime * 10;
	make2_loc_chCode_cond(ch, step, C_CD_END_TIME, 1,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_LOC_OBJ_END_TIME_BRANCH;
	myTestCond->local_object[step][idx] = ACTIVE_DIV_NEXT;

	idx = IDX_LOC_OBJ_SAVE_DELTA_T;
	myTestCond->local_object[step][idx] = 100; //1sec
	make2_loc_chCode_cond(ch, step, C_CD_SAVE_DELTA_T, 1,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_LOC_OBJ_REF_V;
	refV = myPs->testCond.precheck2.refV * 100;
	refV = 4200000; //kjg_090817
	if(refV > myData->mData.config.maxV[0]) {
		refV = myData->mData.config.maxV[0];
	} else if(refV < myData->mData.config.minV[0]) {
		refV = myData->mData.config.minV[0];
	}
	myTestCond->local_object[step][idx] = refV;

	idx = IDX_LOC_OBJ_REF_V2;
	myTestCond->local_object[step][idx] = myData->mData.config.minV[0];

	idx = IDX_LOC_OBJ_RANGE_V;
	myTestCond->local_object[step][idx] = RANGE1 - 1;

	idx = IDX_LOC_OBJ_REF_I;
	refI = myPs->testCond.precheck2.refI * 10;
	myTestCond->local_object[step][idx] = refI;

	idx = IDX_LOC_OBJ_RANGE_I;
	if(refI > 0) {
		if(refI <= (myData->mData.config.maxI[3] * attr_count)) {
			l_val = RANGE4 - 1;
		} else if(refI <= (myData->mData.config.maxI[2] * attr_count)) {
			l_val = RANGE3 - 1;
		} else if(refI <= (myData->mData.config.maxI[1] * attr_count)) {
			l_val = RANGE2 - 1;
		} else {
			l_val = RANGE1 - 1;
		}
		if((l_val+1) > (long)myData->mData.config.rangeI) {
			l_val = (long)myData->mData.config.rangeI - 1;
		}

		l_val2 = 0;
		if(l_val2 == 0) { //auto range
			myTestCond->local_object[step][idx] = l_val;
		} else { //manual range
			if(l_val2 < l_val) {
				myTestCond->local_object[step][idx] = l_val2;
			} else {
				myTestCond->local_object[step][idx] = l_val;
			}
		}
	} else if(refI < 0) {
		if(refI >= (myData->mData.config.minI[3] * attr_count)) {
			l_val = RANGE4 - 1;
		} else if(refI >= (myData->mData.config.minI[2] * attr_count)) {
			l_val = RANGE3 - 1;
		} else if(refI >= (myData->mData.config.minI[1] * attr_count)) {
			l_val = RANGE2 - 1;
		} else {
			l_val = RANGE1 - 1;
		}
		if((l_val+1) > (long)myData->mData.config.rangeI) {
			l_val = (long)myData->mData.config.rangeI - 1;
		}

		l_val2 = 0;
		if(l_val2 == 0) { //auto range
			myTestCond->local_object[step][idx] = l_val;
		} else { //manual range
			if(l_val2 < l_val) {
				myTestCond->local_object[step][idx] = l_val2;
			} else {
				myTestCond->local_object[step][idx] = l_val;
			}
		}
	} else {
		if(step == 0) {
			l_val = RANGE1 - 1;
		} else {
			l_val = myTestCond->local_object[step-1][idx];
		}
		myTestCond->local_object[step][idx] = l_val;
	}

	idx = IDX_LOC_OBJ_FAULT_CHECK_I_JUDGE_RATIO;
	myTestCond->local_object[step][idx]
		= myPs->testCond.precheck2.current_judge_ratio;
	make2_loc_chCode_cond(ch, step, C_CD_FAULT_CHECK_I_JUDGE_RATIO, 1,
		COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=

	idx = IDX_LOC_OBJ_FAULT_CHECK_UPPER_DELTA_V;
	myTestCond->local_object[step][idx]
		= myPs->testCond.precheck2.deltaVmax * 100;
	make2_loc_chCode_cond(ch, step, C_CD_FAULT_CHECK_UPPER_DELTA_V, 1,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_LOC_OBJ_FAULT_CHECK_LOWER_DELTA_V;
	myTestCond->local_object[step][idx]
		= myPs->testCond.precheck2.deltaVmin * 100;
	make2_loc_chCode_cond(ch, step, C_CD_FAULT_CHECK_LOWER_DELTA_V, 1,
		COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=

	idx = IDX_LOC_OBJ_FAULT_CHECK_UPPER_V;
	myTestCond->local_object[step][idx] = refV + myPs->testConfig.failDeltaV;
	make2_loc_chCode_cond(ch, step, C_CD_FAULT_CHECK_UPPER_V, 1,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	//kjg_change 091109(parameter 500mV -> 0mV)
	idx = IDX_LOC_OBJ_FAULT_CHECK_LOWER_V;
	myTestCond->local_object[step][idx] = myPs->testConfig.check_lowerOCV;
	make2_loc_chCode_cond(ch, step, C_CD_FAULT_CHECK_LOWER_V, 1,
		COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=

	idx = IDX_LOC_OBJ_FAULT_CHECK_UPPER_I;
	myTestCond->local_object[step][idx] = labs((float)refI * 1.2);
	make2_loc_chCode_cond(ch, step, C_CD_FAULT_CHECK_UPPER_I, 1,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_LOC_OBJ_FAULT_CHECK_LOWER_I;
	myTestCond->local_object[step][idx] = myPs->testConfig.check_lowerI;
	make2_loc_chCode_cond(ch, step, C_CD_FAULT_CHECK_LOWER_I, 1,
		COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=

	idx = IDX_COM_OBJ_FAULT_UPPER_TEMP;
	myTestCond->common_object[idx]
		= (long)myPs->testCond.precheck2.upper_temp * 1000;
	make2_com_chCode_cond(ch, C_CD_COM_FAULT_UPPER_TEMP, 1,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_COM_OBJ_FAULT_LOWER_TEMP;
	myTestCond->common_object[idx]
		= (long)myPs->testCond.precheck2.lower_temp * 1000;
	make2_com_chCode_cond(ch, C_CD_COM_FAULT_LOWER_TEMP, 1,
		COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=
	
	//step_loop
	step++;

	idx = IDX_LOC_OBJ_CLIENT_STEP_NO;
	myTestCond->local_object[step][idx] = 0;

	idx = IDX_LOC_OBJ_STEP_NO;
	myTestCond->local_object[step][idx] = (long)step;

	idx = IDX_LOC_OBJ_TYPE;
	myTestCond->local_object[step][idx] = STEP_LOOP;

	idx = IDX_LOC_OBJ_ATTRIBUTE;
	myTestCond->local_object[step][idx] = ATTR_CHECK2_END;

	idx = IDX_LOC_OBJ_END_CYCLE_COUNT;
	myTestCond->local_object[step][idx] = 1;

	idx = IDX_LOC_OBJ_END_CYCLE_COUNT_BRANCH;
	myTestCond->local_object[step][idx] = ACTIVE_DIV_NEXT;
}

void convert_test_cond_check3(S_TEST_CONDITION *myTestCond, int ch)
{
	int step, idx, attr_count;
	
	idx = IDX_COM_OBJ_ATTRIBUTE_COUNT;
	attr_count = myTestCond->common_object[idx];

	//step_cycle
	step = 0;

	idx = IDX_LOC_OBJ_CLIENT_STEP_NO;
	myTestCond->local_object[step][idx] = 0;

	idx = IDX_LOC_OBJ_STEP_NO;
	myTestCond->local_object[step][idx] = (long)step;

	idx = IDX_LOC_OBJ_TYPE;
	myTestCond->local_object[step][idx] = STEP_CYCLE;

	idx = IDX_LOC_OBJ_ATTRIBUTE;
	myTestCond->local_object[step][idx] = ATTR_CHECK3_START;

	//step_ocv
	step++;

	idx = IDX_LOC_OBJ_CLIENT_STEP_NO;
	myTestCond->local_object[step][idx] = 0;

	idx = IDX_LOC_OBJ_STEP_NO;
	myTestCond->local_object[step][idx] = (long)step;

	idx = IDX_LOC_OBJ_TYPE;
	myTestCond->local_object[step][idx] = STEP_OCV;

	idx = IDX_LOC_OBJ_ATTRIBUTE;
	myTestCond->local_object[step][idx] = ATTR_CHECK3_1;

//	make2_loc_chCode_cond(ch, step, C_CD_END_OCV, 1, COMP_NONE, 0);

	idx = IDX_LOC_OBJ_END_TIME;
	myTestCond->local_object[step][idx] = 100;
	//myTestCond->local_object[step][idx] = 200;
	make2_loc_chCode_cond(ch, step, C_CD_END_TIME, 1,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_LOC_OBJ_END_TIME_BRANCH;
	myTestCond->local_object[step][idx] = ACTIVE_DIV_NEXT;

	idx = IDX_LOC_OBJ_SAVE_DELTA_T;
	myTestCond->local_object[step][idx] = 100; //1sec
	make2_loc_chCode_cond(ch, step, C_CD_SAVE_DELTA_T, 1,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_LOC_OBJ_FAULT_CHECK_CONTACT_BAD1;
	myTestCond->local_object[step][idx] = 1;
	make2_loc_chCode_cond(ch, step, C_CD_FAULT_CHECK_CONTACT_BAD1, 1,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_LOC_OBJ_FAULT_CHECK_CONTACT_BAD2;
	myTestCond->local_object[step][idx] = 1;
	make2_loc_chCode_cond(ch, step, C_CD_FAULT_CHECK_CONTACT_BAD2, 1,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_LOC_OBJ_FAULT_CHECK_CONTACT_BAD3;
	myTestCond->local_object[step][idx] = 10000; //10mV
	make2_loc_chCode_cond(ch, step, C_CD_FAULT_CHECK_CONTACT_BAD3, 1,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_LOC_OBJ_FAULT_CHECK_DETECT_V;
	myTestCond->local_object[step][idx]
		= myPs->testCond.precheck2.detect_voltage * 100;
		//= 2500000; //kjgd
	make2_loc_chCode_cond(ch, step, C_CD_FAULT_CHECK_DETECT_V, 1,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_LOC_OBJ_FAULT_CHECK_REVERSE_V;
	myTestCond->local_object[step][idx]
		= myPs->testCond.precheck2.reverse_voltage * 100;
	if(myTestCond->local_object[step][idx] <= 0) { //kjg_update_100215
		myTestCond->local_object[step][idx] = -100000;
	}
	make2_loc_chCode_cond(ch, step, C_CD_FAULT_CHECK_REVERSE_V, 1,
		COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=

	idx = IDX_LOC_OBJ_FAULT_CHECK_LOWER_OCV;
	myTestCond->local_object[step][idx]
		= myPs->testCond.precheck2.lowerOCV * 100;
	if(myTestCond->local_object[step][idx] == 0) {
		myTestCond->local_object[step][idx] = 1;
	} //kjg_change 091109
	/*if(myTestCond->local_object[step][idx] < 2500000) {
		myTestCond->local_object[step][idx] = 2500000;
	} //kjg_change 091109*/
	make2_loc_chCode_cond(ch, step, C_CD_FAULT_CHECK_LOWER_OCV, 1,
		COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=

	idx = IDX_LOC_OBJ_FAULT_CHECK_UPPER_OCV;
	myTestCond->local_object[step][idx]
		= myPs->testCond.precheck2.upperOCV * 100;
		//= 2000000; //kjgd
	make2_loc_chCode_cond(ch, step, C_CD_FAULT_CHECK_UPPER_OCV, 1,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	//step_ocv
	step++;

	idx = IDX_LOC_OBJ_CLIENT_STEP_NO;
	myTestCond->local_object[step][idx] = 0;

	idx = IDX_LOC_OBJ_STEP_NO;
	myTestCond->local_object[step][idx] = (long)step;

	idx = IDX_LOC_OBJ_TYPE;
	myTestCond->local_object[step][idx] = STEP_OCV;

	idx = IDX_LOC_OBJ_ATTRIBUTE;
	myTestCond->local_object[step][idx] = ATTR_CHECK3_2;

//	make2_loc_chCode_cond(ch, step, C_CD_END_OCV, 1, COMP_NONE, 0);

	idx = IDX_LOC_OBJ_END_TIME;
	myTestCond->local_object[step][idx] = 100;
	//myTestCond->local_object[step][idx] = 200;
	make2_loc_chCode_cond(ch, step, C_CD_END_TIME, 1,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_LOC_OBJ_END_TIME_BRANCH;
	myTestCond->local_object[step][idx] = ACTIVE_DIV_NEXT;

	idx = IDX_LOC_OBJ_SAVE_DELTA_T;
	myTestCond->local_object[step][idx] = 100; //1sec
	make2_loc_chCode_cond(ch, step, C_CD_SAVE_DELTA_T, 1,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_LOC_OBJ_FAULT_CHECK_DETECT_V;
	myTestCond->local_object[step][idx]
		= myPs->testCond.precheck2.detect_voltage * 100;
		//= 2500000; //kjgd
	make2_loc_chCode_cond(ch, step, C_CD_FAULT_CHECK_DETECT_V, 1,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_LOC_OBJ_FAULT_CHECK_REVERSE_V;
	myTestCond->local_object[step][idx]
		= myPs->testCond.precheck2.reverse_voltage * 100;
	if(myTestCond->local_object[step][idx] <= 0) { //kjg_update_100215
		myTestCond->local_object[step][idx] = -100000;
	}
	make2_loc_chCode_cond(ch, step, C_CD_FAULT_CHECK_REVERSE_V, 1,
		COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=

	idx = IDX_LOC_OBJ_FAULT_CHECK_LOWER_OCV;
	myTestCond->local_object[step][idx]
		= myPs->testCond.precheck2.lowerOCV * 100;
	if(myTestCond->local_object[step][idx] == 0) {
		myTestCond->local_object[step][idx] = 1;
	} //kjg_change 091109
	/*if(myTestCond->local_object[step][idx] < 2500000) {
		myTestCond->local_object[step][idx] = 2500000;
	} //kjg_change 091109*/
	make2_loc_chCode_cond(ch, step, C_CD_FAULT_CHECK_LOWER_OCV, 1,
		COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=

	idx = IDX_LOC_OBJ_FAULT_CHECK_UPPER_OCV;
	myTestCond->local_object[step][idx]
		= myPs->testCond.precheck2.upperOCV * 100;
		//= 2000000; //kjgd
	make2_loc_chCode_cond(ch, step, C_CD_FAULT_CHECK_UPPER_OCV, 1,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_COM_OBJ_FAULT_UPPER_TEMP;
	myTestCond->common_object[idx]
		= (long)myPs->testCond.precheck2.upper_temp * 1000;
	make2_com_chCode_cond(ch, C_CD_COM_FAULT_UPPER_TEMP, 1,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_COM_OBJ_FAULT_LOWER_TEMP;
	myTestCond->common_object[idx]
		= (long)myPs->testCond.precheck2.lower_temp * 1000;
	make2_com_chCode_cond(ch, C_CD_COM_FAULT_LOWER_TEMP, 1,
		COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=
	
	//step_loop
	step++;

	idx = IDX_LOC_OBJ_CLIENT_STEP_NO;
	myTestCond->local_object[step][idx] = 0;

	idx = IDX_LOC_OBJ_STEP_NO;
	myTestCond->local_object[step][idx] = (long)step;

	idx = IDX_LOC_OBJ_TYPE;
	myTestCond->local_object[step][idx] = STEP_LOOP;

	idx = IDX_LOC_OBJ_ATTRIBUTE;
	myTestCond->local_object[step][idx] = ATTR_CHECK3_END;

	idx = IDX_LOC_OBJ_END_CYCLE_COUNT;
	myTestCond->local_object[step][idx] = 1;

	idx = IDX_LOC_OBJ_END_CYCLE_COUNT_BRANCH;
	myTestCond->local_object[step][idx] = ACTIVE_DIV_NEXT;
}

void convert_test_cond_ocv(S_TEST_CONDITION *myTestCond, int ch, int step, int client_step)
{
	int idx, i, j;

//	make2_loc_chCode_cond(ch, step, C_CD_END_OCV, 1, COMP_NONE, 0);

	idx = IDX_LOC_OBJ_END_TIME;
	myTestCond->local_object[step][idx] = 100;
	//myTestCond->local_object[step][idx] = 200;
	make2_loc_chCode_cond(ch, step, C_CD_END_TIME, 1,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_LOC_OBJ_END_TIME_BRANCH;
	myTestCond->local_object[step][idx] = ACTIVE_DIV_NEXT;

	//record
	idx = IDX_LOC_OBJ_SAVE_DELTA_T;
	myTestCond->local_object[step][idx] = 100;
	make2_loc_chCode_cond(ch, step, C_CD_SAVE_DELTA_T, 1,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	//for sbl ng condition
	idx = IDX_LOC_OBJ_FAULT_VOLTAGE_DATA_MIN;
	myTestCond->local_object[step][idx]
		= myPs->testCond.ng_cond[client_step].value1[0] * 100;
	make2_loc_chCode_cond(ch, step, C_CD_FAULT_VOLTAGE_DATA_MIN, 1,
		COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=

	idx = IDX_LOC_OBJ_FAULT_VOLTAGE_DATA_MAX;
	myTestCond->local_object[step][idx]
		= myPs->testCond.ng_cond[client_step].value1[1] * 100;
	make2_loc_chCode_cond(ch, step, C_CD_FAULT_VOLTAGE_DATA_MAX, 1,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_LOC_OBJ_FAULT_VOLTAGE_AVG_MIN;
	myTestCond->local_object[step][idx]
		= myPs->testCond.ng_cond[client_step].value2[0] * 100;
	make2_loc_chCode_cond(ch, step, C_CD_FAULT_VOLTAGE_AVG_MIN, 1,
		COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=

	idx = IDX_LOC_OBJ_FAULT_VOLTAGE_AVG_MAX;
	myTestCond->local_object[step][idx]
		= myPs->testCond.ng_cond[client_step].value2[1] * 100;
		//= 2000000; //kjgd
	make2_loc_chCode_cond(ch, step, C_CD_FAULT_VOLTAGE_AVG_MAX, 1,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=
	/*if(step == 10) { //kjgd
		idx = IDX_LOC_OBJ_FAULT_VOLTAGE_AVG_MAX;
		myTestCond->local_object[step][idx]
			= 2000000;
		make2_loc_chCode_cond(ch, step, C_CD_FAULT_VOLTAGE_AVG_MAX, 1,
			COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=
	}*/

	//fault
	idx = IDX_LOC_OBJ_FAULT_UPPER_V;
	myTestCond->local_object[step][idx]
		= myPs->testCond.ocv[client_step].upperV * 100;
	make2_loc_chCode_cond(ch, step, C_CD_FAULT_UPPER_OCV, 1,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_LOC_OBJ_FAULT_LOWER_V;
	myTestCond->local_object[step][idx]
		= myPs->testCond.ocv[client_step].lowerV * 100;
	make2_loc_chCode_cond(ch, step, C_CD_FAULT_LOWER_OCV, 1,
		COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=

	//step grade
	i = (int)myPs->testCond.gradeHeader[client_step].item;
	if(i != P2_GRADE_ITEM_IDLE) {
		myTestCond->grade[step].item = (unsigned char)i; //kjgw

		j = (int)myPs->testCond.gradeHeader[client_step].totalGrade;
		if(j > MAX_TEST_GRADE) j = MAX_TEST_GRADE; //kjgw
		myTestCond->grade[step].totalGrade = (unsigned char)j;

		for(i=0; i < j; i++) {
			myTestCond->grade[step].code[i]
				= (long)myPs->testCond.grade[client_step][i].code;

			myTestCond->grade[step].value1[i]
				= myPs->testCond.grade[client_step][i].value1;

			myTestCond->grade[step].value2[i]
				= myPs->testCond.grade[client_step][i].value2;
		}
	}
}

void convert_test_cond_rest(S_TEST_CONDITION *myTestCond, int ch, int step, int client_step)
{
	int idx, i, j;

	idx = IDX_LOC_OBJ_ATTRIBUTE;
	myTestCond->local_object[step][idx] = ATTR_IDLE;
	
	idx = IDX_LOC_OBJ_END_TIME;
	myTestCond->local_object[step][idx]
		= myPs->testCond.rest[client_step].endTime * 10;
	make2_loc_chCode_cond(ch, step, C_CD_END_TIME, 1,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_LOC_OBJ_END_TIME_BRANCH;
	myTestCond->local_object[step][idx] = ACTIVE_DIV_NEXT;

	//record
	idx = IDX_LOC_OBJ_SAVE_DELTA_T;
	myTestCond->local_object[step][idx]
		= (long)myPs->testCond.rest[client_step].recordDeltaTime * 10;
	switch(myData->AppControl.config.systemModel) {
		case F_SDI_5V_400A_200A_100A_10A:
		case F_SDI_5V_400A_200A_100A_10A_2:
		case F_SDI_5V_450A_200A_100A_10A:
		case F_PNE_5V_15A_30AP_SW:
			break;
		default:
			if(myTestCond->local_object[step][idx] != 0
				&& myTestCond->local_object[step][idx] < 100) {
				myTestCond->local_object[step][idx] = 100; //1sec
			}
			break;
	}
	make2_loc_chCode_cond(ch, step, C_CD_SAVE_DELTA_T, 1,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_LOC_OBJ_SAVE_DELTA_V;
	myTestCond->local_object[step][idx]
		= myPs->testCond.rest[client_step].recordDeltaV * 100;
	make2_loc_chCode_cond(ch, step, C_CD_SAVE_DELTA_V, 1,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_LOC_OBJ_SAVE_DELTA_I;
	myTestCond->local_object[step][idx]
		= myPs->testCond.rest[client_step].recordDeltaI * 100;
	make2_loc_chCode_cond(ch, step, C_CD_SAVE_DELTA_I, 1,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	//fault
	idx = IDX_LOC_OBJ_FAULT_UPPER_V;
	myTestCond->local_object[step][idx]
		= myPs->testCond.rest[client_step].upperV * 100;
	make2_loc_chCode_cond(ch, step, C_CD_FAULT_UPPER_V, 1,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_LOC_OBJ_FAULT_LOWER_V;
	myTestCond->local_object[step][idx]
		= myPs->testCond.rest[client_step].lowerV * 100;
	make2_loc_chCode_cond(ch, step, C_CD_FAULT_LOWER_V, 1,
		COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=

	//step grade
	i = (int)myPs->testCond.gradeHeader[client_step].item;
	if(i != P2_GRADE_ITEM_IDLE) {
		myTestCond->grade[step].item = (unsigned char)i; //kjgw

		j = (int)myPs->testCond.gradeHeader[client_step].totalGrade;
		if(j > MAX_TEST_GRADE) j = MAX_TEST_GRADE; //kjgw
		myTestCond->grade[step].totalGrade = (unsigned char)j;

		for(i=0; i < j; i++) {
			myTestCond->grade[step].code[i]
				= (long)myPs->testCond.grade[client_step][i].code;

			myTestCond->grade[step].value1[i]
				= myPs->testCond.grade[client_step][i].value1;

			myTestCond->grade[step].value2[i]
				= myPs->testCond.grade[client_step][i].value2;
		}
	}
}

void convert_test_cond_long_time_rest(S_TEST_CONDITION *myTestCond, int ch, int step, int client_step)
{
	int idx, i, j;

	idx = IDX_LOC_OBJ_ATTRIBUTE;
	myTestCond->local_object[step][idx] = ATTR_IDLE;
	
	idx = IDX_LOC_OBJ_END_TIME;
	myTestCond->local_object[step][idx]
		= myPs->testCond.long_time_rest[client_step].endTime * 10;
	make2_loc_chCode_cond(ch, step, C_CD_END_TIME, 1,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_LOC_OBJ_END_TIME_BRANCH;
	myTestCond->local_object[step][idx] = ACTIVE_DIV_NEXT;

	//record
	idx = IDX_LOC_OBJ_SAVE_DELTA_T;
	myTestCond->local_object[step][idx]
		= (long)myPs->testCond.long_time_rest[client_step].recordDeltaTime * 10;
	switch(myData->AppControl.config.systemModel) {
		case F_SDI_5V_400A_200A_100A_10A:
		case F_SDI_5V_400A_200A_100A_10A_2:
		case F_SDI_5V_450A_200A_100A_10A:
		case F_PNE_5V_15A_30AP_SW:
			break;
		default:
			if(myTestCond->local_object[step][idx] != 0
				&& myTestCond->local_object[step][idx] < 100) {
				myTestCond->local_object[step][idx] = 100; //1sec
			}
			break;
	}
	make2_loc_chCode_cond(ch, step, C_CD_SAVE_DELTA_T, 1,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_LOC_OBJ_SAVE_DELTA_V;
	myTestCond->local_object[step][idx]
		= myPs->testCond.long_time_rest[client_step].recordDeltaV * 100;
	make2_loc_chCode_cond(ch, step, C_CD_SAVE_DELTA_V, 1,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_LOC_OBJ_SAVE_DELTA_I;
	myTestCond->local_object[step][idx]
		= myPs->testCond.long_time_rest[client_step].recordDeltaI * 100;
	make2_loc_chCode_cond(ch, step, C_CD_SAVE_DELTA_I, 1,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	//fault
	idx = IDX_LOC_OBJ_FAULT_UPPER_V;
	myTestCond->local_object[step][idx]
		= myPs->testCond.long_time_rest[client_step].upperV * 100;
	make2_loc_chCode_cond(ch, step, C_CD_FAULT_UPPER_V, 1,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_LOC_OBJ_FAULT_LOWER_V;
	myTestCond->local_object[step][idx]
		= myPs->testCond.long_time_rest[client_step].lowerV * 100;
	make2_loc_chCode_cond(ch, step, C_CD_FAULT_LOWER_V, 1,
		COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=

	//step grade
	i = (int)myPs->testCond.gradeHeader[client_step].item;
	if(i != P2_GRADE_ITEM_IDLE) {
		myTestCond->grade[step].item = (unsigned char)i; //kjgw

		j = (int)myPs->testCond.gradeHeader[client_step].totalGrade;
		if(j > MAX_TEST_GRADE) j = MAX_TEST_GRADE; //kjgw
		myTestCond->grade[step].totalGrade = (unsigned char)j;

		for(i=0; i < j; i++) {
			myTestCond->grade[step].code[i]
				= (long)myPs->testCond.grade[client_step][i].code;

			myTestCond->grade[step].value1[i]
				= myPs->testCond.grade[client_step][i].value1;

			myTestCond->grade[step].value2[i]
				= myPs->testCond.grade[client_step][i].value2;
		}
	}
}

void convert_test_cond_charge(S_TEST_CONDITION *myTestCond, int ch, int step, int client_step)
{
	int idx, idx2, i, j, type, attr_count, cp_i_range=0;
	long mode, refV, refI, l_val, l_val2;
	double d_val;

	type = STEP_CHARGE;

	idx = IDX_COM_OBJ_ATTRIBUTE_COUNT;
	attr_count = myTestCond->common_object[idx];

	//header
	idx = IDX_LOC_OBJ_MODE;
	mode = convert_step_mode(CONVERT_P2_TO_ORG,
		(long)myPs->testCond.stepHeader[client_step].mode);
	myTestCond->local_object[step][idx] = mode;

	//kjgw idx = IDX_LOC_OBJ_TOTAL_SUB_STEP;
	//myData->testCond[ch].local_object[step][idx]
	//	= (long)myPs->testCond.step[step].header.subStep;

	//reference
	idx = IDX_LOC_OBJ_REF_V;
	refV = myPs->testCond.charge[client_step].refV * 100;
	if(refV > myData->mData.config.maxV[0]) {
		refV = myData->mData.config.maxV[0];
	} else if(refV < myData->mData.config.minV[0]) {
		refV = myData->mData.config.minV[0];
	}
	myTestCond->local_object[step][idx] = refV;
	
	idx = IDX_LOC_OBJ_REF_V2;
	myTestCond->local_object[step][idx] = myData->mData.config.minV[0];

	idx = IDX_LOC_OBJ_RANGE_V; //kjgw
	myTestCond->local_object[step][idx] = RANGE1 - 1;

	refI = myPs->testCond.charge[client_step].refI * 10;
	if(type == STEP_CHARGE) {
		if(mode == MODE_CP) {
			idx = IDX_LOC_OBJ_REF_P;
			myTestCond->local_object[step][idx] = refI / 1000;
			d_val = (double)(refI / 1000) / 2000000.0; //2.0V
			d_val *= 1000000000.0;
			l_val = (long)d_val;
			if(l_val <= (myData->mData.config.maxI[3] * attr_count)) {
				cp_i_range = RANGE4 - 1;
			} else if(l_val <= (myData->mData.config.maxI[2] * attr_count)) {
				cp_i_range = RANGE3 - 1;
			} else if(l_val <= (myData->mData.config.maxI[1] * attr_count)) {
				cp_i_range = RANGE2 - 1;
			} else {
				cp_i_range = RANGE1 - 1;
			}
			refI = myData->mData.config.maxI[cp_i_range] * attr_count;
		} else if(mode == MODE_CR) {
			idx = IDX_LOC_OBJ_REF_R;
			myTestCond->local_object[step][idx] = refI / 1000;
			d_val = (double)refV / (double)refI * 1000000.0;
			refI = (long)d_val;
		}
	} else if(type == STEP_DISCHARGE || type == STEP_Z) {
		if(mode == MODE_CP) {
			idx = IDX_LOC_OBJ_REF_P;
			myTestCond->local_object[step][idx] = refI / 1000;
			d_val = (double)(refI / 1000) / 2000000.0; //2.0V
			d_val *= 1000000000.0;
			l_val = (long)d_val;
			if(l_val <= (myData->mData.config.maxI[3] * attr_count)) {
				cp_i_range = RANGE4 - 1;
			} else if(l_val <= (myData->mData.config.maxI[2] * attr_count)) {
				cp_i_range = RANGE3 - 1;
			} else if(l_val <= (myData->mData.config.maxI[1] * attr_count)) {
				cp_i_range = RANGE2 - 1;
			} else {
				cp_i_range = RANGE1 - 1;
			}
			refI = myData->mData.config.maxI[cp_i_range] * attr_count;
		} else if(mode == MODE_CR) {
			idx = IDX_LOC_OBJ_REF_R;
			myTestCond->local_object[step][idx] = refI / 1000;
			d_val = (double)myData->mData.config.maxV[0]
				/ (double)refI * 1000000.0;
			refI = (long)d_val;
		}
		refI *= (-1);
	}
	idx = IDX_LOC_OBJ_REF_I;
	if(refI > (myData->mData.config.maxI[0] * attr_count)) {
		if(myData->mData.config.maxI[0] != 0) {
			refI = myData->mData.config.maxI[0] * attr_count;
		}
	} else if(refI < (myData->mData.config.minI[0] * attr_count)) {
		if(myData->mData.config.minI[0] != 0) {
			refI = myData->mData.config.minI[0] * attr_count;
		}
	}
	myTestCond->local_object[step][idx] = refI;
	//myTestCond->local_object[step][idx] = 20000000; //kjgd

	idx = IDX_LOC_OBJ_RANGE_I;
	if(refI > 0) {
		if(refI <= (myData->mData.config.maxI[3] * attr_count)) {
			l_val = RANGE4 - 1;
		} else if(refI <= (myData->mData.config.maxI[2] * attr_count)) {
			l_val = RANGE3 - 1;
		} else if(refI <= (myData->mData.config.maxI[1] * attr_count)) {
			l_val = RANGE2 - 1;
		} else {
			l_val = RANGE1 - 1;
		}
		if((l_val+1) > (long)myData->mData.config.rangeI) {
			l_val = (long)myData->mData.config.rangeI - 1;
		}

		l_val2 = 0;
		if(l_val2 == 0) { //auto range
			myTestCond->local_object[step][idx] = l_val;
		} else { //manual range
			if(l_val2 < l_val) {
				myTestCond->local_object[step][idx] = l_val2;
			} else {
				myTestCond->local_object[step][idx] = l_val;
			}
		}
	} else if(refI < 0) {
		if(refI >= (myData->mData.config.minI[3] * attr_count)) {
			l_val = RANGE4 - 1;
		} else if(refI >= (myData->mData.config.minI[2] * attr_count)) {
			l_val = RANGE3 - 1;
		} else if(refI >= (myData->mData.config.minI[1] * attr_count)) {
			l_val = RANGE2 - 1;
		} else {
			l_val = RANGE1 - 1;
		}
		if((l_val+1) > (long)myData->mData.config.rangeI) {
			l_val = (long)myData->mData.config.rangeI - 1;
		}

		l_val2 = 0;
		if(l_val2 == 0) { //auto range
			myTestCond->local_object[step][idx] = l_val;
		} else { //manual range
			if(l_val2 < l_val) {
				myTestCond->local_object[step][idx] = l_val2;
			} else {
				myTestCond->local_object[step][idx] = l_val;
			}
		}
	} else {
		if(step == 0) {
			l_val = RANGE1 - 1;
		} else {
			l_val = myTestCond->local_object[step-1][idx];
		}
		myTestCond->local_object[step][idx] = l_val;
	} 

	//kjgw for(i=0; i < myPs->testCond.step[step].header.subStep; i++) {
	//	myData->testCond[ch].step[step].refV[i]
	//		= myPs->testCond.step[step].reference[i].refV;
	//	...
	//}
	
	//record
	idx = IDX_LOC_OBJ_SAVE_DELTA_T;
	myTestCond->local_object[step][idx]
		= (long)myPs->testCond.charge[client_step].recordDeltaTime * 10;
	switch(myData->AppControl.config.systemModel) {
		case F_SDI_5V_400A_200A_100A_10A:
		case F_SDI_5V_400A_200A_100A_10A_2:
		case F_SDI_5V_450A_200A_100A_10A:
		case F_PNE_5V_15A_30AP_SW:
			break;
		default:
			if(myTestCond->local_object[step][idx] != 0
				&& myTestCond->local_object[step][idx] < 100) {
				myTestCond->local_object[step][idx] = 100; //1sec
			}
			break;
	}
	make2_loc_chCode_cond(ch, step, C_CD_SAVE_DELTA_T, 1,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_LOC_OBJ_SAVE_DELTA_V;
	myTestCond->local_object[step][idx]
		= myPs->testCond.charge[client_step].recordDeltaV * 100;
	make2_loc_chCode_cond(ch, step, C_CD_SAVE_DELTA_V, 1,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_LOC_OBJ_SAVE_DELTA_I;
	myTestCond->local_object[step][idx]
		= myPs->testCond.charge[client_step].recordDeltaI * 10;
	make2_loc_chCode_cond(ch, step, C_CD_SAVE_DELTA_I, 1,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	//end_condition
	idx = IDX_LOC_OBJ_END_TIME;
	myTestCond->local_object[step][idx]
		= myPs->testCond.charge[client_step].endTime * 10;
	make2_loc_chCode_cond(ch, step, C_CD_END_TIME, 1,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_LOC_OBJ_END_TIME_BRANCH;
	myTestCond->local_object[step][idx] = ACTIVE_DIV_NEXT;

//kjgw_s	idx = IDX_LOC_OBJ_END_CV_TIME;
//	myTestCond->local_object[step][idx]
//		= myPs->testCond.step[step].reference[0].CVTime;
//	make2_loc_chCode_cond(ch, step, C_CD_END_CV_TIME, 1,
//		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=
//
//	idx = IDX_LOC_OBJ_END_CV_TIME_BRANCH;
//kjgw_e	myTestCond->local_object[step][idx] = ACTIVE_DIV_NEXT;

	idx = IDX_LOC_OBJ_END_V_UPPER;
	myTestCond->local_object[step][idx]
		= myPs->testCond.charge[client_step].endV * 100;
	make2_loc_chCode_cond(ch, step, C_CD_END_V_UPPER, 1,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_LOC_OBJ_END_V_UPPER_BRANCH;
	myTestCond->local_object[step][idx] = ACTIVE_DIV_NEXT;

//kjgw_s	idx = IDX_LOC_OBJ_END_V_LOWER;
//	myTestCond->local_object[step][idx]
//		= myPs->testCond.charge[client_step].endV;
//	make2_loc_chCode_cond(ch, step, C_CD_END_V_LOWER, 1,
//		COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=
//
//	idx = IDX_LOC_OBJ_END_V_LOWER_BRANCH;
//kjgw_e	myTestCond->local_object[step][idx] = ACTIVE_DIV_NEXT;

	idx = IDX_LOC_OBJ_END_AMPARE_HOUR;
	myTestCond->local_object[step][idx]
		= myPs->testCond.charge[client_step].endCapacity * 10;
	make2_loc_chCode_cond(ch, step, C_CD_END_AMPARE_HOUR, 1,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_LOC_OBJ_END_AMPARE_HOUR_BRANCH;
	myTestCond->local_object[step][idx] = ACTIVE_DIV_NEXT;

//kjgw_s	idx = IDX_LOC_OBJ_END_WATT_HOUR;
//	myTestCond->local_object[step][idx]
//		= myPs->testCond.step[step].reference[0].WattHour_SumWattHour;
//	make2_loc_chCode_cond(ch, step, C_CD_END_WATT_HOUR, 1,
//		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=
//			
//	idx = IDX_LOC_OBJ_END_WATT_HOUR_BRANCH;
//kjgw_e	myTestCond->local_object[step][idx] = ACTIVE_DIV_NEXT;

//kjgw_s	tmp = (long)myPs->testCond.step[step].reference[0].ValueRate_Item;
//	if(tmp == P2_VALUE_RATE_ITEM_AMPARE_HOUR) {
		idx = IDX_LOC_OBJ_END_AMPARE_HOUR_RATE;
		myTestCond->local_object[step][idx]
			= myPs->testCond.charge[client_step].socRate;
		make2_loc_chCode_cond(ch, step, C_CD_END_VALUE_RATE_AMPARE_HOUR, 1,
			COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

		idx = IDX_LOC_OBJ_END_AMPARE_HOUR_RATE_BRANCH;
		myTestCond->local_object[step][idx] = ACTIVE_DIV_NEXT;

		idx = IDX_LOC_OBJ_END_AMPARE_HOUR_RATE_COMPARE;
		myTestCond->local_object[step][idx]
			= myPs->testCond.charge[client_step].useActualCapacity;
//	} else if(tmp == P2_VALUE_RATE_ITEM_WATT_HOUR) {
//		idx = IDX_LOC_OBJ_END_WATT_HOUR_RATE;
//		myData->testCond[ch].local_object[step][idx]
//			= myPs->testCond.step[step].reference[0].ValueRate;
//		make2_loc_chCode_cond(ch, step, C_CD_END_VALUE_RATE_WATT_HOUR, 1,
//			COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=
//
//		idx = IDX_LOC_OBJ_END_WATT_HOUR_RATE_BRANCH;
//		myData->testCond[ch].local_object[step][idx]
//			= myPs->testCond.step[step].reference[0].ValueRate_Branch;
//		if(myData->testCond[ch].local_object[step][idx] == ACTIVE_DIV_NONE)
//			myData->testCond[ch].local_object[step][idx] = ACTIVE_DIV_NEXT;
//
//		idx = IDX_LOC_OBJ_END_WATT_HOUR_RATE_COMPARE;
//		myData->testCond[ch].local_object[step][idx]
//			= myPs->testCond.step[step].reference[0].ValueRate_Compare;
//kjgw_e	}

	if(type == STEP_CHARGE) {
		idx = IDX_LOC_OBJ_END_I;
		myTestCond->local_object[step][idx]
			= myPs->testCond.charge[client_step].endI * 10;
		make2_loc_chCode_cond(ch, step, C_CD_END_I, 1,
			COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=
	} else if(type == STEP_DISCHARGE || type == STEP_Z) {
		idx = IDX_LOC_OBJ_END_I;
		myTestCond->local_object[step][idx]
			= myPs->testCond.charge[client_step].endI * (-10);
		make2_loc_chCode_cond(ch, step, C_CD_END_I, 1,
			COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=
	}

	idx = IDX_LOC_OBJ_END_DELTA_V;
	myTestCond->local_object[step][idx]
		= myPs->testCond.charge[client_step].endDV * 100;
	make2_loc_chCode_cond(ch, step, C_CD_END_DELTA_V, 1,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

//kjgw_s	idx = IDX_LOC_OBJ_END_P;
//	myData->testCond[ch].local_object[step][idx]
//		= myPs->testCond.step[step].reference[0].Power;
//	if(type == STEP_CHARGE) {
//		make2_loc_chCode_cond(ch, step, C_CD_END_P, 1,
//			COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=
//	} else if(type == STEP_DISCHARGE || type == STEP_Z) {
//		make2_loc_chCode_cond(ch, step, C_CD_END_P, 1,
//			COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=
//kjgw_e	}

	//step comp //kjgw
	//	= myPs->testCond.step[i].compV[comp_point];
	//	= myPs->testCond.step[i].compI[comp_point];

	//step delta //kjgw
	//	= myPs->testCond.step[i].deltaV;
	//	= myPs->testCond.step[i].deltaI;

	//step record
//kjgw	idx = IDX_LOC_OBJ_SAVE_DELTA_TEMP;
//	myData->testCond[ch].local_object[step][idx]
//		= myPs->testCond.step[step].record.deltaT;
//	make2_loc_chCode_cond(ch, step, C_CD_SAVE_DELTA_TEMP, 1,
//		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

//kjgw	idx = IDX_LOC_OBJ_SAVE_DELTA_P;
//	myData->testCond[ch].local_object[step][idx]
//		= myPs->testCond.step[step].record.deltaP;
//	make2_loc_chCode_cond(ch, step, C_CD_SAVE_DELTA_P, 1,
//		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	//step edlc
//kjgw_s	idx = IDX_LOC_OBJ_CAPACITANCE_V1;
//	myData->testCond[ch].local_object[step][idx]
//		= myPs->testCond.step[step].edlc.capacitanceV1;
//
//	idx = IDX_LOC_OBJ_CAPACITANCE_V2;
//	myData->testCond[ch].local_object[step][idx]
//		= myPs->testCond.step[step].edlc.capacitanceV2;
//
	idx = IDX_LOC_OBJ_Z_T1;
	myTestCond->local_object[step][idx] = 0;
		//kjgw = (long)myPs->testCond.step[step].edlc.startT_Z;

	idx = IDX_LOC_OBJ_Z_T2;
	if(type == STEP_Z) {
		myTestCond->local_object[step][idx]
			= (long)myPs->testCond.charge[client_step].endTime * 10;
			//kjgw = myPs->testCond.step[step].edlc.endT_Z;
	} else {
		l_val = (long)myPs->testCond.charge[client_step].endTime * 10 - 2;
		if(l_val < 0) l_val = 0;
		myTestCond->local_object[step][idx] = l_val;
	}

//	idx = IDX_LOC_OBJ_LC_T1;
//	myData->testCond[ch].local_object[step][idx]
//		= (long)myPs->testCond.step[step].edlc.startT_LC;
//
//	idx = IDX_LOC_OBJ_LC_T2;
//	myData->testCond[ch].local_object[step][idx]
//kjgw_e		= (long)myPs->testCond.step[step].edlc.endT_LC;

	//for sbl ng condition
	if(myData->AppControl.config.systemModel == F_SDI_5V_50A_5A) {
		idx = IDX_LOC_OBJ_FAULT_AMPARE_HOUR_DATA_MIN;
		myTestCond->local_object[step][idx]
			= myPs->testCond.ng_cond[client_step].value1[0] * 10;
		make2_loc_chCode_cond(ch, step, C_CD_FAULT_AMPARE_HOUR_DATA_MIN, 1,
			COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=

		idx = IDX_LOC_OBJ_FAULT_AMPARE_HOUR_DATA_MAX;
		myTestCond->local_object[step][idx]
			= myPs->testCond.ng_cond[client_step].value1[1] * 10;
		make2_loc_chCode_cond(ch, step, C_CD_FAULT_AMPARE_HOUR_DATA_MAX, 1,
			COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

		idx = IDX_LOC_OBJ_FAULT_AMPARE_HOUR_AVG_MIN;
		myTestCond->local_object[step][idx]
			= myPs->testCond.ng_cond[client_step].value2[0] * 10;
		make2_loc_chCode_cond(ch, step, C_CD_FAULT_AMPARE_HOUR_AVG_MIN, 1,
			COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=

		idx = IDX_LOC_OBJ_FAULT_AMPARE_HOUR_AVG_MAX;
		myTestCond->local_object[step][idx]
			= myPs->testCond.ng_cond[client_step].value2[1] * 10;
		make2_loc_chCode_cond(ch, step, C_CD_FAULT_AMPARE_HOUR_AVG_MAX, 1,
			COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=
	} else { //F_SDI_5V_400A_200A_100A_10A, F_SDI_5V_400A_200A_100A_10A_2
		//F_SDI_5V_450A_200A_100A_10A
		idx = IDX_LOC_OBJ_FAULT_POWER_DATA_MIN;
		myTestCond->local_object[step][idx]
			= myPs->testCond.ng_cond[client_step].value1[0];
		make2_loc_chCode_cond(ch, step, C_CD_FAULT_POWER_DATA_MIN, 1,
			COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=

		idx = IDX_LOC_OBJ_FAULT_POWER_DATA_MAX;
		myTestCond->local_object[step][idx]
			= myPs->testCond.ng_cond[client_step].value1[1];
		make2_loc_chCode_cond(ch, step, C_CD_FAULT_POWER_DATA_MAX, 1,
			COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

		idx = IDX_LOC_OBJ_FAULT_POWER_AVG_MIN;
		myTestCond->local_object[step][idx]
			= myPs->testCond.ng_cond[client_step].value2[0];
		make2_loc_chCode_cond(ch, step, C_CD_FAULT_POWER_AVG_MIN, 1,
			COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=

		idx = IDX_LOC_OBJ_FAULT_POWER_AVG_MAX;
		myTestCond->local_object[step][idx]
			= myPs->testCond.ng_cond[client_step].value2[1];
		make2_loc_chCode_cond(ch, step, C_CD_FAULT_POWER_AVG_MAX, 1,
			COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=
	}

	//fault_condition
	idx = IDX_LOC_OBJ_FAULT_UPPER_V;
	myTestCond->local_object[step][idx]
		= myPs->testCond.charge[client_step].upperV * 100;
	make2_loc_chCode_cond(ch, step, C_CD_FAULT_UPPER_V, 1,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_LOC_OBJ_FAULT_LOWER_V;
	myTestCond->local_object[step][idx]
		= myPs->testCond.charge[client_step].lowerV * 100;
	make2_loc_chCode_cond(ch, step, C_CD_FAULT_LOWER_V, 1,
		COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=

//kjgw_s	idx = IDX_LOC_OBJ_FAULT_UPPER_TEMP;
//	myData->testCond[ch].local_object[step][idx]
//		= myPs->testCond.step[step].faultUpperTemp;
//	make2_loc_chCode_cond(ch, step, C_CD_FAULT_UPPER_TEMP, 1,
//		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=
//
//	idx = IDX_LOC_OBJ_FAULT_LOWER_TEMP;
//	myData->testCond[ch].local_object[step][idx]
//		= myPs->testCond.step[step].faultLowerTemp;
//	make2_loc_chCode_cond(ch, step, C_CD_FAULT_LOWER_TEMP, 1,
//kjgw_e		COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=

	switch(type) {
		case STEP_CHARGE:
			if(mode == MODE_CC_CV || mode == MODE_CC || mode == MODE_CV) {
				idx = IDX_LOC_OBJ_FAULT_HW_UPPER_V;
				myTestCond->local_object[step][idx] = 5000; //5.0%
				make2_loc_chCode_cond(ch, step, C_CD_FAULT_HW_UPPER_V, 1,
					COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

				idx = IDX_LOC_OBJ_FAULT_LOWER_DELTA_V;
				//myTestCond->local_object[step][idx] = 20000; //20mV
				myTestCond->local_object[step][idx]
					= myPs->testConfig.fail_charge_lower_dv;
				idx2 = IDX_LOC_OBJ_FAULT_LOWER_DELTA_V_T;
				myTestCond->local_object[step][idx2] = 100; //1sec
				make2_loc_chCode_cond2(ch, step, C_CD_FAULT_LOWER_DELTA_V, 1,
					COMP_GREATER_THAN_OR_EQUAL_TO, idx, idx2); //>=
			}
			break;
		case STEP_DISCHARGE:
			if(mode == MODE_CC_CV || mode == MODE_CC || mode == MODE_CV) {
				idx = IDX_LOC_OBJ_FAULT_HW_LOWER_V;
				myTestCond->local_object[step][idx] = 5000; //5.0%
				make2_loc_chCode_cond(ch, step, C_CD_FAULT_HW_LOWER_V, 1,
					COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=
			}
			break;
		case STEP_Z:
			if(mode == MODE_DC) {
				idx = IDX_LOC_OBJ_FAULT_HW_LOWER_V;
				myTestCond->local_object[step][idx] = 5000; //5.0%
				make2_loc_chCode_cond(ch, step, C_CD_FAULT_HW_LOWER_V, 1,
					COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=
			}
			break;
		default: break;
	}

	idx = IDX_LOC_OBJ_FAULT_HW_UPPER_I;
	myTestCond->local_object[step][idx] = 5000; //5.0%
	make2_loc_chCode_cond(ch, step, C_CD_FAULT_HW_UPPER_I, 1,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=
//kjgw_s
//	idx = IDX_LOC_OBJ_FAULT_HW_LOWER_I;
//	myTestCond->local_object[step][idx] = 5000; //5.0%
//	make2_loc_chCode_cond(ch, step, C_CD_FAULT_HW_LOWER_I, 1,
//		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=
//kjgw_e
	idx = IDX_LOC_OBJ_FAULT_UPPER_I;
	if(type == STEP_CHARGE) {
		myTestCond->local_object[step][idx]
			= myPs->testCond.charge[client_step].upperI * 10;
		make2_loc_chCode_cond(ch, step, C_CD_FAULT_UPPER_I, 1,
			COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=
	} else if(type == STEP_DISCHARGE || type == STEP_Z) {
		myTestCond->local_object[step][idx]
			= myPs->testCond.charge[client_step].upperI * (-10);
		make2_loc_chCode_cond(ch, step, C_CD_FAULT_UPPER_I, 1,
			COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=
	}

	idx = IDX_LOC_OBJ_FAULT_LOWER_I;
	if(type == STEP_CHARGE) {
		myTestCond->local_object[step][idx]
			= myPs->testCond.charge[client_step].lowerI * 10;
		make2_loc_chCode_cond(ch, step, C_CD_FAULT_LOWER_I, 1,
			COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=
	} else if(type == STEP_DISCHARGE || type == STEP_Z) {
		myTestCond->local_object[step][idx]
			= myPs->testCond.charge[client_step].lowerI * (-10);
		make2_loc_chCode_cond(ch, step, C_CD_FAULT_LOWER_I, 1,
			COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=
	}

//kjgw_s	if(type == STEP_Z) {
//		idx = IDX_LOC_OBJ_FAULT_UPPER_Z;
//		myTestCond->local_object[step][idx]
//			= myPs->testCond.charge[client_step].faultUpperZ;
//		make2_loc_chCode_cond(ch, step, C_CD_FAULT_UPPER_Z, 1,
//			COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=
//
//		idx = IDX_LOC_OBJ_FAULT_LOWER_Z;
//		myData->testCond[ch].local_object[step][idx]
//			= myPs->testCond.step[step].faultLowerZ;
//		make2_loc_chCode_cond(ch, step, C_CD_FAULT_LOWER_Z, 1,
//			COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=
//kjgw_e	}

	idx = IDX_LOC_OBJ_FAULT_UPPER_AMPARE_HOUR;
	myTestCond->local_object[step][idx]
		= myPs->testCond.charge[client_step].upperCapacity * 10;
	make2_loc_chCode_cond(ch, step, C_CD_FAULT_UPPER_AMPARE_HOUR, 1,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_LOC_OBJ_FAULT_LOWER_AMPARE_HOUR;
	myTestCond->local_object[step][idx]
		= myPs->testCond.charge[client_step].lowerCapacity * 10;
	make2_loc_chCode_cond(ch, step, C_CD_FAULT_LOWER_AMPARE_HOUR, 1,
		COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=

	//kjgw comp_v1, v2, v3, i1, i2, i3, dV, dI

	//step grade
	i = (int)myPs->testCond.gradeHeader[client_step].item;
	if(i != P2_GRADE_ITEM_IDLE) {
		myTestCond->grade[step].item = (unsigned char)i; //kjgw

		j = (int)myPs->testCond.gradeHeader[client_step].totalGrade;
		if(j > MAX_TEST_GRADE) j = MAX_TEST_GRADE; //kjgw
		myTestCond->grade[step].totalGrade = (unsigned char)j;

		for(i=0; i < j; i++) {
			myTestCond->grade[step].code[i]
				= (long)myPs->testCond.grade[client_step][i].code;

			myTestCond->grade[step].value1[i]
				= myPs->testCond.grade[client_step][i].value1;

			myTestCond->grade[step].value2[i]
				= myPs->testCond.grade[client_step][i].value2;
		}
	}
}

void convert_test_cond_discharge(S_TEST_CONDITION *myTestCond, int ch, int step, int client_step)
{
	int idx, i, j, type, attr_count, cp_i_range=0;
	long mode, refV, refI, l_val, l_val2;
	double d_val;

	type = STEP_DISCHARGE;

	idx = IDX_COM_OBJ_ATTRIBUTE_COUNT;
	attr_count = myTestCond->common_object[idx];

	//header
	idx = IDX_LOC_OBJ_MODE;
	mode = convert_step_mode(CONVERT_P2_TO_ORG,
		(long)myPs->testCond.stepHeader[client_step].mode);
	myTestCond->local_object[step][idx] = mode;

	//kjgw idx = IDX_LOC_OBJ_TOTAL_SUB_STEP;
	//myData->testCond[ch].local_object[step][idx]
	//	= (long)myPs->testCond.step[step].header.subStep;

	//reference
	idx = IDX_LOC_OBJ_REF_V;
	myTestCond->local_object[step][idx] = myData->mData.config.maxV[0];

	idx = IDX_LOC_OBJ_REF_V2;
	refV = myPs->testCond.discharge[client_step].refV * 100;
	if(refV > myData->mData.config.maxV[0]) {
		refV = myData->mData.config.maxV[0];
	} else if(refV < myData->mData.config.minV[0]) {
		refV = myData->mData.config.minV[0];
	}
	myTestCond->local_object[step][idx] = refV;

	idx = IDX_LOC_OBJ_RANGE_V; //kjgw
	myTestCond->local_object[step][idx] = RANGE1 - 1;

	refI = myPs->testCond.discharge[client_step].refI * 10;
	if(type == STEP_CHARGE) {
		if(mode == MODE_CP) {
			idx = IDX_LOC_OBJ_REF_P;
			myTestCond->local_object[step][idx] = refI / 1000;
			d_val = (double)(refI / 1000) / 2000000.0; //2.0V
			d_val *= 1000000000.0;
			l_val = (long)d_val;
			if(l_val <= (myData->mData.config.maxI[3] * attr_count)) {
				cp_i_range = RANGE4 - 1;
			} else if(l_val <= (myData->mData.config.maxI[2] * attr_count)) {
				cp_i_range = RANGE3 - 1;
			} else if(l_val <= (myData->mData.config.maxI[1] * attr_count)) {
				cp_i_range = RANGE2 - 1;
			} else {
				cp_i_range = RANGE1 - 1;
			}
			refI = myData->mData.config.maxI[cp_i_range] * attr_count;
		} else if(mode == MODE_CR) {
			idx = IDX_LOC_OBJ_REF_R;
			myTestCond->local_object[step][idx] = refI / 1000;
			d_val = (double)refV / (double)refI * 1000000.0;
			refI = (long)d_val;
		}
	} else if(type == STEP_DISCHARGE || type == STEP_Z) {
		if(mode == MODE_CP) {
			idx = IDX_LOC_OBJ_REF_P;
			myTestCond->local_object[step][idx] = refI / 1000;
			d_val = (double)(refI / 1000) / 2000000.0; //2.0V
			d_val *= 1000000000.0;
			l_val = (long)d_val;
			if(l_val <= (myData->mData.config.maxI[3] * attr_count)) {
				cp_i_range = RANGE4 - 1;
			} else if(l_val <= (myData->mData.config.maxI[2] * attr_count)) {
				cp_i_range = RANGE3 - 1;
			} else if(l_val <= (myData->mData.config.maxI[1] * attr_count)) {
				cp_i_range = RANGE2 - 1;
			} else {
				cp_i_range = RANGE1 - 1;
			}
			refI = myData->mData.config.maxI[cp_i_range] * attr_count;
		} else if(mode == MODE_CR) {
			idx = IDX_LOC_OBJ_REF_R;
			myTestCond->local_object[step][idx] = refI / 1000;
			d_val = (double)myData->mData.config.maxV[0]
				/ (double)refI * 1000000.0;
			refI = (long)d_val;
		}
		refI *= (-1);
	}
	idx = IDX_LOC_OBJ_REF_I;
	if(refI > (myData->mData.config.maxI[0] * attr_count)) {
		if(myData->mData.config.maxI[0] != 0) {
			refI = myData->mData.config.maxI[0] * attr_count;
		}
	} else if(refI < (myData->mData.config.minI[0] * attr_count)) {
		if(myData->mData.config.minI[0] != 0) {
			refI = myData->mData.config.minI[0] * attr_count;
		}
	}
	myTestCond->local_object[step][idx] = refI;

	idx = IDX_LOC_OBJ_RANGE_I;
	if(refI > 0) {
		if(refI <= (myData->mData.config.maxI[3] * attr_count)) {
			l_val = RANGE4 - 1;
		} else if(refI <= (myData->mData.config.maxI[2] * attr_count)) {
			l_val = RANGE3 - 1;
		} else if(refI <= (myData->mData.config.maxI[1] * attr_count)) {
			l_val = RANGE2 - 1;
		} else {
			l_val = RANGE1 - 1;
		}
		if((l_val+1) > (long)myData->mData.config.rangeI) {
			l_val = (long)myData->mData.config.rangeI - 1;
		}

		l_val2 = 0;
		if(l_val2 == 0) { //auto range
			myTestCond->local_object[step][idx] = l_val;
		} else { //manual range
			if(l_val2 < l_val) {
				myTestCond->local_object[step][idx] = l_val2;
			} else {
				myTestCond->local_object[step][idx] = l_val;
			}
		}
	} else if(refI < 0) {
		if(refI >= (myData->mData.config.minI[3] * attr_count)) {
			l_val = RANGE4 - 1;
		} else if(refI >= (myData->mData.config.minI[2] * attr_count)) {
			l_val = RANGE3 - 1;
		} else if(refI >= (myData->mData.config.minI[1] * attr_count)) {
			l_val = RANGE2 - 1;
		} else {
			l_val = RANGE1 - 1;
		}
		if((l_val+1) > (long)myData->mData.config.rangeI) {
			l_val = (long)myData->mData.config.rangeI - 1;
		}

		l_val2 = 0;
		if(l_val2 == 0) { //auto range
			myTestCond->local_object[step][idx] = l_val;
		} else { //manual range
			if(l_val2 < l_val) {
				myTestCond->local_object[step][idx] = l_val2;
			} else {
				myTestCond->local_object[step][idx] = l_val;
			}
		}
	} else {
		if(step == 0) {
			l_val = RANGE1 - 1;
		} else {
			l_val = myTestCond->local_object[step-1][idx];
		}
		myTestCond->local_object[step][idx] = l_val;
	} 

	//kjgw for(i=0; i < myPs->testCond.step[step].header.subStep; i++) {
	//	myData->testCond[ch].step[step].refV[i]
	//		= myPs->testCond.step[step].reference[i].refV;
	//	...
	//}
	
	//record
	idx = IDX_LOC_OBJ_SAVE_DELTA_T;
	myTestCond->local_object[step][idx]
		= (long)myPs->testCond.discharge[client_step].recordDeltaTime * 10;
	switch(myData->AppControl.config.systemModel) {
		case F_SDI_5V_400A_200A_100A_10A:
		case F_SDI_5V_400A_200A_100A_10A_2:
		case F_SDI_5V_450A_200A_100A_10A:
		case F_PNE_5V_15A_30AP_SW:
			break;
		default:
			if(myTestCond->local_object[step][idx] != 0
				&& myTestCond->local_object[step][idx] < 100) {
				myTestCond->local_object[step][idx] = 100; //1sec
			}
			break;
	}
	make2_loc_chCode_cond(ch, step, C_CD_SAVE_DELTA_T, 1,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_LOC_OBJ_SAVE_DELTA_V;
	myTestCond->local_object[step][idx]
		= myPs->testCond.discharge[client_step].recordDeltaV * 100;
	make2_loc_chCode_cond(ch, step, C_CD_SAVE_DELTA_V, 1,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_LOC_OBJ_SAVE_DELTA_I;
	myTestCond->local_object[step][idx]
		= myPs->testCond.discharge[client_step].recordDeltaI * 10;
	make2_loc_chCode_cond(ch, step, C_CD_SAVE_DELTA_I, 1,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	//end_condition
	idx = IDX_LOC_OBJ_END_TIME;
	myTestCond->local_object[step][idx]
		= myPs->testCond.discharge[client_step].endTime * 10;
	make2_loc_chCode_cond(ch, step, C_CD_END_TIME, 1,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_LOC_OBJ_END_TIME_BRANCH;
	myTestCond->local_object[step][idx] = ACTIVE_DIV_NEXT;

//kjgw_s	idx = IDX_LOC_OBJ_END_CV_TIME;
//	myTestCond->local_object[step][idx]
//		= myPs->testCond.step[step].reference[0].CVTime;
//	make2_loc_chCode_cond(ch, step, C_CD_END_CV_TIME, 1,
//		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=
//
//	idx = IDX_LOC_OBJ_END_CV_TIME_BRANCH;
//kjgw_e	myTestCond->local_object[step][idx] = ACTIVE_DIV_NEXT;

//kjgw_s	idx = IDX_LOC_OBJ_END_V_UPPER;
//	myTestCond->local_object[step][idx]
//		= myPs->testCond.discharge[client_step].endV * 100;
//	make2_loc_chCode_cond(ch, step, C_CD_END_V_UPPER, 1,
//		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=
//
//	idx = IDX_LOC_OBJ_END_V_UPPER_BRANCH;
//kjgw_e	myTestCond->local_object[step][idx] = ACTIVE_DIV_NEXT;

	idx = IDX_LOC_OBJ_END_V_LOWER;
	myTestCond->local_object[step][idx]
		= myPs->testCond.discharge[client_step].endV * 100;
	make2_loc_chCode_cond(ch, step, C_CD_END_V_LOWER, 1,
		COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=

	idx = IDX_LOC_OBJ_END_V_LOWER_BRANCH;
	myTestCond->local_object[step][idx] = ACTIVE_DIV_NEXT;

	idx = IDX_LOC_OBJ_END_AMPARE_HOUR;
	myTestCond->local_object[step][idx]
		= myPs->testCond.discharge[client_step].endCapacity * 10;
	make2_loc_chCode_cond(ch, step, C_CD_END_AMPARE_HOUR, 1,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_LOC_OBJ_END_AMPARE_HOUR_BRANCH;
	myTestCond->local_object[step][idx] = ACTIVE_DIV_NEXT;

//kjgw_s	idx = IDX_LOC_OBJ_END_WATT_HOUR;
//	myTestCond->local_object[step][idx]
//		= myPs->testCond.step[step].reference[0].WattHour_SumWattHour;
//	make2_loc_chCode_cond(ch, step, C_CD_END_WATT_HOUR, 1,
//		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=
//			
//	idx = IDX_LOC_OBJ_END_WATT_HOUR_BRANCH;
//kjgw_e	myTestCond->local_object[step][idx] = ACTIVE_DIV_NEXT;

//kjgw_s	tmp = (long)myPs->testCond.step[step].reference[0].ValueRate_Item;
//	if(tmp == P2_VALUE_RATE_ITEM_AMPARE_HOUR) {
		idx = IDX_LOC_OBJ_END_AMPARE_HOUR_RATE;
		myTestCond->local_object[step][idx]
			= myPs->testCond.discharge[client_step].socRate;
		make2_loc_chCode_cond(ch, step, C_CD_END_VALUE_RATE_AMPARE_HOUR, 1,
			COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

		idx = IDX_LOC_OBJ_END_AMPARE_HOUR_RATE_BRANCH;
		myTestCond->local_object[step][idx] = ACTIVE_DIV_NEXT;

		idx = IDX_LOC_OBJ_END_AMPARE_HOUR_RATE_COMPARE;
		myTestCond->local_object[step][idx]
			= myPs->testCond.discharge[client_step].useActualCapacity;
//	} else if(tmp == P2_VALUE_RATE_ITEM_WATT_HOUR) {
//		idx = IDX_LOC_OBJ_END_WATT_HOUR_RATE;
//		myData->testCond[ch].local_object[step][idx]
//			= myPs->testCond.step[step].reference[0].ValueRate;
//		make2_loc_chCode_cond(ch, step, C_CD_END_VALUE_RATE_WATT_HOUR, 1,
//			COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=
//
//		idx = IDX_LOC_OBJ_END_WATT_HOUR_RATE_BRANCH;
//		myData->testCond[ch].local_object[step][idx]
//			= myPs->testCond.step[step].reference[0].ValueRate_Branch;
//		if(myData->testCond[ch].local_object[step][idx] == ACTIVE_DIV_NONE)
//			myData->testCond[ch].local_object[step][idx] = ACTIVE_DIV_NEXT;
//
//		idx = IDX_LOC_OBJ_END_WATT_HOUR_RATE_COMPARE;
//		myData->testCond[ch].local_object[step][idx]
//			= myPs->testCond.step[step].reference[0].ValueRate_Compare;
//kjgw_e	}

	if(type == STEP_CHARGE) {
		idx = IDX_LOC_OBJ_END_I;
		myTestCond->local_object[step][idx]
			= myPs->testCond.discharge[client_step].endI * 10;
		make2_loc_chCode_cond(ch, step, C_CD_END_I, 1,
			COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=
	} else if(type == STEP_DISCHARGE || type == STEP_Z) {
		idx = IDX_LOC_OBJ_END_I;
		myTestCond->local_object[step][idx]
			= myPs->testCond.discharge[client_step].endI * (-10);
		make2_loc_chCode_cond(ch, step, C_CD_END_I, 1,
			COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=
	}

	idx = IDX_LOC_OBJ_END_DELTA_V;
	myTestCond->local_object[step][idx]
		= myPs->testCond.discharge[client_step].endDV * 100;
	make2_loc_chCode_cond(ch, step, C_CD_END_DELTA_V, 1,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

//kjgw_s	idx = IDX_LOC_OBJ_END_P;
//	myData->testCond[ch].local_object[step][idx]
//		= myPs->testCond.step[step].reference[0].Power;
//	if(type == STEP_CHARGE) {
//		make2_loc_chCode_cond(ch, step, C_CD_END_P, 1,
//			COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=
//	} else if(type == STEP_DISCHARGE || type == STEP_Z) {
//		make2_loc_chCode_cond(ch, step, C_CD_END_P, 1,
//			COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=
//kjgw_e	}

	//step comp //kjgw
	//	= myPs->testCond.step[i].compV[comp_point];
	//	= myPs->testCond.step[i].compI[comp_point];

	//step delta //kjgw
	//	= myPs->testCond.step[i].deltaV;
	//	= myPs->testCond.step[i].deltaI;

	//step record
//kjgw	idx = IDX_LOC_OBJ_SAVE_DELTA_TEMP;
//	myData->testCond[ch].local_object[step][idx]
//		= myPs->testCond.step[step].record.deltaT;
//	make2_loc_chCode_cond(ch, step, C_CD_SAVE_DELTA_TEMP, 1,
//		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

//kjgw	idx = IDX_LOC_OBJ_SAVE_DELTA_P;
//	myData->testCond[ch].local_object[step][idx]
//		= myPs->testCond.step[step].record.deltaP;
//	make2_loc_chCode_cond(ch, step, C_CD_SAVE_DELTA_P, 1,
//		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	//step edlc
//kjgw_s	idx = IDX_LOC_OBJ_CAPACITANCE_V1;
//	myData->testCond[ch].local_object[step][idx]
//		= myPs->testCond.step[step].edlc.capacitanceV1;
//
//	idx = IDX_LOC_OBJ_CAPACITANCE_V2;
//	myData->testCond[ch].local_object[step][idx]
//		= myPs->testCond.step[step].edlc.capacitanceV2;
//
	idx = IDX_LOC_OBJ_Z_T1;
	myTestCond->local_object[step][idx] = 0;
		//kjgw = (long)myPs->testCond.step[step].edlc.startT_Z;

	idx = IDX_LOC_OBJ_Z_T2;
	if(type == STEP_Z) {
		myTestCond->local_object[step][idx]
			= (long)myPs->testCond.discharge[client_step].endTime * 10;
			//kjgw = myPs->testCond.step[step].edlc.endT_Z;
	} else {
		l_val = (long)myPs->testCond.discharge[client_step].endTime * 10 - 2;
		if(l_val < 0) l_val = 0;
		myTestCond->local_object[step][idx] = l_val;
	}

//	idx = IDX_LOC_OBJ_LC_T1;
//	myData->testCond[ch].local_object[step][idx]
//		= (long)myPs->testCond.step[step].edlc.startT_LC;
//
//	idx = IDX_LOC_OBJ_LC_T2;
//	myData->testCond[ch].local_object[step][idx]
//kjgw_e		= (long)myPs->testCond.step[step].edlc.endT_LC;

	//for sbl ng condition
	if(myData->AppControl.config.systemModel == F_SDI_5V_50A_5A) {
		idx = IDX_LOC_OBJ_FAULT_AMPARE_HOUR_DATA_MIN;
		myTestCond->local_object[step][idx]
			= myPs->testCond.ng_cond[client_step].value1[0] * 10;
		make2_loc_chCode_cond(ch, step, C_CD_FAULT_AMPARE_HOUR_DATA_MIN, 1,
			COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=

		idx = IDX_LOC_OBJ_FAULT_AMPARE_HOUR_DATA_MAX;
		myTestCond->local_object[step][idx]
			= myPs->testCond.ng_cond[client_step].value1[1] * 10;
		make2_loc_chCode_cond(ch, step, C_CD_FAULT_AMPARE_HOUR_DATA_MAX, 1,
			COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

		idx = IDX_LOC_OBJ_FAULT_AMPARE_HOUR_AVG_MIN;
		myTestCond->local_object[step][idx]
			= myPs->testCond.ng_cond[client_step].value2[0] * 10;
		make2_loc_chCode_cond(ch, step, C_CD_FAULT_AMPARE_HOUR_AVG_MIN, 1,
			COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=

		idx = IDX_LOC_OBJ_FAULT_AMPARE_HOUR_AVG_MAX;
		myTestCond->local_object[step][idx]
			= myPs->testCond.ng_cond[client_step].value2[1] * 10;
		make2_loc_chCode_cond(ch, step, C_CD_FAULT_AMPARE_HOUR_AVG_MAX, 1,
			COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=
	} else { //F_SDI_5V_400A_200A_100A_10A, F_SDI_5V_400A_200A_100A_10A_2
		//F_SDI_5V_450A_200A_100A_10A
		idx = IDX_LOC_OBJ_FAULT_POWER_DATA_MIN;
		myTestCond->local_object[step][idx]
			= myPs->testCond.ng_cond[client_step].value1[0];
		make2_loc_chCode_cond(ch, step, C_CD_FAULT_POWER_DATA_MIN, 1,
			COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=

		idx = IDX_LOC_OBJ_FAULT_POWER_DATA_MAX;
		myTestCond->local_object[step][idx]
			= myPs->testCond.ng_cond[client_step].value1[1];
		make2_loc_chCode_cond(ch, step, C_CD_FAULT_POWER_DATA_MAX, 1,
			COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

		idx = IDX_LOC_OBJ_FAULT_POWER_AVG_MIN;
		myTestCond->local_object[step][idx]
			= myPs->testCond.ng_cond[client_step].value2[0];
		make2_loc_chCode_cond(ch, step, C_CD_FAULT_POWER_AVG_MIN, 1,
			COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=

		idx = IDX_LOC_OBJ_FAULT_POWER_AVG_MAX;
		myTestCond->local_object[step][idx]
			= myPs->testCond.ng_cond[client_step].value2[1];
		make2_loc_chCode_cond(ch, step, C_CD_FAULT_POWER_AVG_MAX, 1,
			COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=
	}

	//fault_condition
	idx = IDX_LOC_OBJ_FAULT_UPPER_V;
	myTestCond->local_object[step][idx]
		= myPs->testCond.discharge[client_step].upperV * 100;
	make2_loc_chCode_cond(ch, step, C_CD_FAULT_UPPER_V, 1,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_LOC_OBJ_FAULT_LOWER_V;
	myTestCond->local_object[step][idx]
		= myPs->testCond.discharge[client_step].lowerV * 100;
	make2_loc_chCode_cond(ch, step, C_CD_FAULT_LOWER_V, 1,
		COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=

//kjgw_s	idx = IDX_LOC_OBJ_FAULT_UPPER_TEMP;
//	myData->testCond[ch].local_object[step][idx]
//		= myPs->testCond.step[step].faultUpperTemp;
//	make2_loc_chCode_cond(ch, step, C_CD_FAULT_UPPER_TEMP, 1,
//		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=
//
//	idx = IDX_LOC_OBJ_FAULT_LOWER_TEMP;
//	myData->testCond[ch].local_object[step][idx]
//		= myPs->testCond.step[step].faultLowerTemp;
//	make2_loc_chCode_cond(ch, step, C_CD_FAULT_LOWER_TEMP, 1,
//kjgw_e		COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=

	switch(type) {
		case STEP_CHARGE:
			if(mode == MODE_CC_CV || mode == MODE_CC || mode == MODE_CV) {
				idx = IDX_LOC_OBJ_FAULT_HW_UPPER_V;
				myTestCond->local_object[step][idx] = 5000; //5.0%
				make2_loc_chCode_cond(ch, step, C_CD_FAULT_HW_UPPER_V, 1,
					COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=
			}
			break;
		case STEP_DISCHARGE:
			if(mode == MODE_CC_CV || mode == MODE_CC || mode == MODE_CV) {
				idx = IDX_LOC_OBJ_FAULT_HW_LOWER_V;
				myTestCond->local_object[step][idx] = 5000; //5.0%
				make2_loc_chCode_cond(ch, step, C_CD_FAULT_HW_LOWER_V, 1,
					COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=
			}
			break;
		case STEP_Z:
			if(mode == MODE_DC) {
				idx = IDX_LOC_OBJ_FAULT_HW_LOWER_V;
				myTestCond->local_object[step][idx] = 5000; //5.0%
				make2_loc_chCode_cond(ch, step, C_CD_FAULT_HW_LOWER_V, 1,
					COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=
			}
			break;
		default: break;
	}

	idx = IDX_LOC_OBJ_FAULT_HW_UPPER_I;
	myTestCond->local_object[step][idx] = 5000; //5.0%
	make2_loc_chCode_cond(ch, step, C_CD_FAULT_HW_UPPER_I, 1,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=
//kjgw_s
//	idx = IDX_LOC_OBJ_FAULT_HW_LOWER_I;
//	myTestCond->local_object[step][idx] = 5000; //5.0%
//	make2_loc_chCode_cond(ch, step, C_CD_FAULT_HW_LOWER_I, 1,
//		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=
//kjgw_e
	idx = IDX_LOC_OBJ_FAULT_UPPER_I;
	if(type == STEP_CHARGE) {
		myTestCond->local_object[step][idx]
			= myPs->testCond.discharge[client_step].upperI * 10;
		make2_loc_chCode_cond(ch, step, C_CD_FAULT_UPPER_I, 1,
			COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=
	} else if(type == STEP_DISCHARGE || type == STEP_Z) {
		myTestCond->local_object[step][idx]
			= myPs->testCond.discharge[client_step].upperI * (-10);
		make2_loc_chCode_cond(ch, step, C_CD_FAULT_UPPER_I, 1,
			COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=
	}

	idx = IDX_LOC_OBJ_FAULT_LOWER_I;
	if(type == STEP_CHARGE) {
		myTestCond->local_object[step][idx]
			= myPs->testCond.discharge[client_step].lowerI * 10;
		make2_loc_chCode_cond(ch, step, C_CD_FAULT_LOWER_I, 1,
			COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=
	} else if(type == STEP_DISCHARGE || type == STEP_Z) {
		myTestCond->local_object[step][idx]
			= myPs->testCond.discharge[client_step].lowerI * (-10);
		make2_loc_chCode_cond(ch, step, C_CD_FAULT_LOWER_I, 1,
			COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=
	}

//kjgw_s	if(type == STEP_Z) {
//		idx = IDX_LOC_OBJ_FAULT_UPPER_Z;
//		myTestCond->local_object[step][idx]
//			= myPs->testCond.discharge[client_step].faultUpperZ;
//		make2_loc_chCode_cond(ch, step, C_CD_FAULT_UPPER_Z, 1,
//			COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=
//
//		idx = IDX_LOC_OBJ_FAULT_LOWER_Z;
//		myData->testCond[ch].local_object[step][idx]
//			= myPs->testCond.step[step].faultLowerZ;
//		make2_loc_chCode_cond(ch, step, C_CD_FAULT_LOWER_Z, 1,
//			COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=
//kjgw_e	}

	idx = IDX_LOC_OBJ_FAULT_UPPER_AMPARE_HOUR;
	myTestCond->local_object[step][idx]
		= myPs->testCond.discharge[client_step].upperCapacity * 10;
	make2_loc_chCode_cond(ch, step, C_CD_FAULT_UPPER_AMPARE_HOUR, 1,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_LOC_OBJ_FAULT_LOWER_AMPARE_HOUR;
	myTestCond->local_object[step][idx]
		= myPs->testCond.discharge[client_step].lowerCapacity * 10;
	make2_loc_chCode_cond(ch, step, C_CD_FAULT_LOWER_AMPARE_HOUR, 1,
		COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=

	//kjgw comp_v1, v2, v3, i1, i2, i3, dV, dI

	//step grade
	i = (int)myPs->testCond.gradeHeader[client_step].item;
	if(i != P2_GRADE_ITEM_IDLE) {
		myTestCond->grade[step].item = (unsigned char)i; //kjgw

		j = (int)myPs->testCond.gradeHeader[client_step].totalGrade;
		if(j > MAX_TEST_GRADE) j = MAX_TEST_GRADE; //kjgw
		myTestCond->grade[step].totalGrade = (unsigned char)j;

		for(i=0; i < j; i++) {
			myTestCond->grade[step].code[i]
				= (long)myPs->testCond.grade[client_step][i].code;

			myTestCond->grade[step].value1[i]
				= myPs->testCond.grade[client_step][i].value1;

			myTestCond->grade[step].value2[i]
				= myPs->testCond.grade[client_step][i].value2;
		}
	}
}

void convert_test_cond_z(S_TEST_CONDITION *myTestCond, int ch, int step, int client_step)
{
	int idx, i, j, type, attr_count;
	long mode, refV, refI, l_val, l_val2;
	double d_val;

	type = STEP_Z;

	idx = IDX_COM_OBJ_ATTRIBUTE_COUNT;
	attr_count = myTestCond->common_object[idx];

	//header
	idx = IDX_LOC_OBJ_MODE;
	mode = convert_step_mode(CONVERT_P2_TO_ORG,
		(long)myPs->testCond.stepHeader[client_step].mode);
	myTestCond->local_object[step][idx] = mode;

	//kjgw idx = IDX_LOC_OBJ_TOTAL_SUB_STEP;
	//myData->testCond[ch].local_object[step][idx]
	//	= (long)myPs->testCond.step[step].header.subStep;

	//reference
	idx = IDX_LOC_OBJ_REF_V;
	myTestCond->local_object[step][idx] = myData->mData.config.maxV[0];

	idx = IDX_LOC_OBJ_REF_V2;
	refV = myPs->testCond.z[client_step].refV * 100;
	if(refV > myData->mData.config.maxV[0]) {
		refV = myData->mData.config.maxV[0];
	} else if(refV < myData->mData.config.minV[0]) {
		refV = myData->mData.config.minV[0];
	}
	myTestCond->local_object[step][idx] = refV;

	idx = IDX_LOC_OBJ_RANGE_V; //kjgw
	myTestCond->local_object[step][idx] = RANGE1 - 1;

	refI = myPs->testCond.z[client_step].refI * 10;
	if(type == STEP_CHARGE) {
		if(mode == MODE_CP) {
			idx = IDX_LOC_OBJ_REF_P;
			myTestCond->local_object[step][idx] = refI / 1000;
			refI = myData->mData.config.maxI[0] * attr_count;
		} else if(mode == MODE_CR) {
			idx = IDX_LOC_OBJ_REF_R;
			myTestCond->local_object[step][idx] = refI / 1000;
			d_val = (double)refV / (double)refI * 1000000.0;
			refI = (long)d_val;
		}
	} else if(type == STEP_DISCHARGE || type == STEP_Z) {
		if(mode == MODE_CP) {
			idx = IDX_LOC_OBJ_REF_P;
			myTestCond->local_object[step][idx] = refI / 1000;
			refI = myData->mData.config.maxI[0] * attr_count;
		} else if(mode == MODE_CR) {
			idx = IDX_LOC_OBJ_REF_R;
			myTestCond->local_object[step][idx] = refI / 1000;
			d_val = (double)myData->mData.config.maxV[0]
				/ (double)refI * 1000000.0;
			refI = (long)d_val;
		}
		refI *= (-1);
	}
	idx = IDX_LOC_OBJ_REF_I;
	if(refI > (myData->mData.config.maxI[0] * attr_count)) {
		if(myData->mData.config.maxI[0] != 0) {
			refI = myData->mData.config.maxI[0] * attr_count;
		}
	} else if(refI < (myData->mData.config.minI[0] * attr_count)) {
		if(myData->mData.config.minI[0] != 0) {
			refI = myData->mData.config.minI[0] * attr_count;
		}
	}
	myTestCond->local_object[step][idx] = refI;

	idx = IDX_LOC_OBJ_RANGE_I;
	if(refI > 0) {
		if(refI <= (myData->mData.config.maxI[3] * attr_count)) {
			l_val = RANGE4 - 1;
		} else if(refI <= (myData->mData.config.maxI[2] * attr_count)) {
			l_val = RANGE3 - 1;
		} else if(refI <= (myData->mData.config.maxI[1] * attr_count)) {
			l_val = RANGE2 - 1;
		} else {
			l_val = RANGE1 - 1;
		}
		if((l_val+1) > (long)myData->mData.config.rangeI) {
			l_val = (long)myData->mData.config.rangeI - 1;
		}

		l_val2 = 0;
		if(l_val2 == 0) { //auto range
			myTestCond->local_object[step][idx] = l_val;
		} else { //manual range
			if(l_val2 < l_val) {
				myTestCond->local_object[step][idx] = l_val2;
			} else {
				myTestCond->local_object[step][idx] = l_val;
			}
		}
	} else if(refI < 0) {
		if(refI >= (myData->mData.config.minI[3] * attr_count)) {
			l_val = RANGE4 - 1;
		} else if(refI >= (myData->mData.config.minI[2] * attr_count)) {
			l_val = RANGE3 - 1;
		} else if(refI >= (myData->mData.config.minI[1] * attr_count)) {
			l_val = RANGE2 - 1;
		} else {
			l_val = RANGE1 - 1;
		}
		if((l_val+1) > (long)myData->mData.config.rangeI) {
			l_val = (long)myData->mData.config.rangeI - 1;
		}

		l_val2 = 0;
		if(l_val2 == 0) { //auto range
			myTestCond->local_object[step][idx] = l_val;
		} else { //manual range
			if(l_val2 < l_val) {
				myTestCond->local_object[step][idx] = l_val2;
			} else {
				myTestCond->local_object[step][idx] = l_val;
			}
		}
	} else {
		if(step == 0) {
			l_val = RANGE1 - 1;
		} else {
			l_val = myTestCond->local_object[step-1][idx];
		}
		myTestCond->local_object[step][idx] = l_val;
	} 

	//kjgw for(i=0; i < myPs->testCond.step[step].header.subStep; i++) {
	//	myData->testCond[ch].step[step].refV[i]
	//		= myPs->testCond.step[step].reference[i].refV;
	//	...
	//}
	
	//record
	idx = IDX_LOC_OBJ_SAVE_DELTA_T;
	myTestCond->local_object[step][idx]
		= (long)myPs->testCond.z[client_step].recordDeltaTime * 10;
	switch(myData->AppControl.config.systemModel) {
		case F_SDI_5V_400A_200A_100A_10A:
		case F_SDI_5V_400A_200A_100A_10A_2:
		case F_SDI_5V_450A_200A_100A_10A:
		case F_PNE_5V_15A_30AP_SW:
			break;
		default:
			if(myTestCond->local_object[step][idx] != 0
				&& myTestCond->local_object[step][idx] < 100) {
				myTestCond->local_object[step][idx] = 100; //1sec
			}
			break;
	}
	make2_loc_chCode_cond(ch, step, C_CD_SAVE_DELTA_T, 1,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_LOC_OBJ_SAVE_DELTA_V;
	myTestCond->local_object[step][idx]
		= myPs->testCond.z[client_step].recordDeltaV * 100;
	make2_loc_chCode_cond(ch, step, C_CD_SAVE_DELTA_V, 1,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_LOC_OBJ_SAVE_DELTA_I;
	myTestCond->local_object[step][idx]
		= myPs->testCond.z[client_step].recordDeltaI * 10;
	make2_loc_chCode_cond(ch, step, C_CD_SAVE_DELTA_I, 1,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	//end_condition
	idx = IDX_LOC_OBJ_END_TIME;
	myTestCond->local_object[step][idx]
		= myPs->testCond.z[client_step].endTime * 10;
	make2_loc_chCode_cond(ch, step, C_CD_END_TIME, 1,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_LOC_OBJ_END_TIME_BRANCH;
	myTestCond->local_object[step][idx] = ACTIVE_DIV_NEXT;

//kjgw_s	idx = IDX_LOC_OBJ_END_CV_TIME;
//	myTestCond->local_object[step][idx]
//		= myPs->testCond.step[step].reference[0].CVTime;
//	make2_loc_chCode_cond(ch, step, C_CD_END_CV_TIME, 1,
//		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=
//
//	idx = IDX_LOC_OBJ_END_CV_TIME_BRANCH;
//kjgw_e	myTestCond->local_object[step][idx] = ACTIVE_DIV_NEXT;

//kjgw_s	idx = IDX_LOC_OBJ_END_V_UPPER;
//	myTestCond->local_object[step][idx]
//		= myPs->testCond.z[client_step].endV * 100;
//	make2_loc_chCode_cond(ch, step, C_CD_END_V_UPPER, 1,
//		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=
//
//	idx = IDX_LOC_OBJ_END_V_UPPER_BRANCH;
//kjgw_e	myTestCond->local_object[step][idx] = ACTIVE_DIV_NEXT;

//kjgw_s	idx = IDX_LOC_OBJ_END_V_LOWER;
//	myTestCond->local_object[step][idx]
//		= myPs->testCond.z[client_step].endV * 100;
//	make2_loc_chCode_cond(ch, step, C_CD_END_V_LOWER, 1,
//		COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=
//
//	idx = IDX_LOC_OBJ_END_V_LOWER_BRANCH;
//kjgw_e	myTestCond->local_object[step][idx] = ACTIVE_DIV_NEXT;

//kjgw_s	idx = IDX_LOC_OBJ_END_AMPARE_HOUR;
//	myTestCond->local_object[step][idx]
//		= myPs->testCond.z[client_step].endCapacity * 10;
//	make2_loc_chCode_cond(ch, step, C_CD_END_AMPARE_HOUR, 1,
//		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=
//
//	idx = IDX_LOC_OBJ_END_AMPARE_HOUR_BRANCH;
//kjgw_e	myTestCond->local_object[step][idx] = ACTIVE_DIV_NEXT;

//kjgw_s	idx = IDX_LOC_OBJ_END_WATT_HOUR;
//	myTestCond->local_object[step][idx]
//		= myPs->testCond.step[step].reference[0].WattHour_SumWattHour;
//	make2_loc_chCode_cond(ch, step, C_CD_END_WATT_HOUR, 1,
//		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=
//			
//	idx = IDX_LOC_OBJ_END_WATT_HOUR_BRANCH;
//kjgw_e	myTestCond->local_object[step][idx] = ACTIVE_DIV_NEXT;

//kjgw_s	tmp = (long)myPs->testCond.step[step].reference[0].ValueRate_Item;
//	if(tmp == P2_VALUE_RATE_ITEM_AMPARE_HOUR) {
//		idx = IDX_LOC_OBJ_END_AMPARE_HOUR_RATE;
//		myTestCond->local_object[step][idx]
//			= myPs->testCond.z[client_step].socRate;
//		make2_loc_chCode_cond(ch, step, C_CD_END_VALUE_RATE_AMPARE_HOUR, 1,
//			COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=
//
//		idx = IDX_LOC_OBJ_END_AMPARE_HOUR_RATE_BRANCH;
//		myTestCond->local_object[step][idx] = ACTIVE_DIV_NEXT;
//
//		idx = IDX_LOC_OBJ_END_AMPARE_HOUR_RATE_COMPARE;
//		myTestCond->local_object[step][idx]
//			= myPs->testCond.z[client_step].useActualCapacity;
//	} else if(tmp == P2_VALUE_RATE_ITEM_WATT_HOUR) {
//		idx = IDX_LOC_OBJ_END_WATT_HOUR_RATE;
//		myData->testCond[ch].local_object[step][idx]
//			= myPs->testCond.step[step].reference[0].ValueRate;
//		make2_loc_chCode_cond(ch, step, C_CD_END_VALUE_RATE_WATT_HOUR, 1,
//			COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=
//
//		idx = IDX_LOC_OBJ_END_WATT_HOUR_RATE_BRANCH;
//		myData->testCond[ch].local_object[step][idx]
//			= myPs->testCond.step[step].reference[0].ValueRate_Branch;
//		if(myData->testCond[ch].local_object[step][idx] == ACTIVE_DIV_NONE)
//			myData->testCond[ch].local_object[step][idx] = ACTIVE_DIV_NEXT;
//
//		idx = IDX_LOC_OBJ_END_WATT_HOUR_RATE_COMPARE;
//		myData->testCond[ch].local_object[step][idx]
//			= myPs->testCond.step[step].reference[0].ValueRate_Compare;
//kjgw_e	}

//kjgw_s	if(type == STEP_CHARGE) {
//		idx = IDX_LOC_OBJ_END_I;
//		myTestCond->local_object[step][idx]
//			= myPs->testCond.z[client_step].endI * 10;
//		make2_loc_chCode_cond(ch, step, C_CD_END_I, 1,
//			COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=
//	} else if(type == STEP_DISCHARGE || type == STEP_Z) {
//		idx = IDX_LOC_OBJ_END_I;
//		myTestCond->local_object[step][idx]
//			= myPs->testCond.z[client_step].endI * (-10);
//		make2_loc_chCode_cond(ch, step, C_CD_END_I, 1,
//			COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=
//kjgw_e	}

//kjgw_s	idx = IDX_LOC_OBJ_END_DELTA_V;
//	myTestCond->local_object[step][idx]
//		= myPs->testCond.z[client_step].endDV * 100;
//	make2_loc_chCode_cond(ch, step, C_CD_END_DELTA_V, 1,
//kjgw_e		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

//kjgw_s	idx = IDX_LOC_OBJ_END_P;
//	myData->testCond[ch].local_object[step][idx]
//		= myPs->testCond.step[step].reference[0].Power;
//	if(type == STEP_CHARGE) {
//		make2_loc_chCode_cond(ch, step, C_CD_END_P, 1,
//			COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=
//	} else if(type == STEP_DISCHARGE || type == STEP_Z) {
//		make2_loc_chCode_cond(ch, step, C_CD_END_P, 1,
//			COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=
//kjgw_e	}

	//step comp //kjgw
	//	= myPs->testCond.step[i].compV[comp_point];
	//	= myPs->testCond.step[i].compI[comp_point];

	//step delta //kjgw
	//	= myPs->testCond.step[i].deltaV;
	//	= myPs->testCond.step[i].deltaI;

	//step record
//kjgw	idx = IDX_LOC_OBJ_SAVE_DELTA_TEMP;
//	myData->testCond[ch].local_object[step][idx]
//		= myPs->testCond.step[step].record.deltaT;
//	make2_loc_chCode_cond(ch, step, C_CD_SAVE_DELTA_TEMP, 1,
//		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

//kjgw	idx = IDX_LOC_OBJ_SAVE_DELTA_P;
//	myData->testCond[ch].local_object[step][idx]
//		= myPs->testCond.step[step].record.deltaP;
//	make2_loc_chCode_cond(ch, step, C_CD_SAVE_DELTA_P, 1,
//		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	//step edlc
//kjgw_s	idx = IDX_LOC_OBJ_CAPACITANCE_V1;
//	myData->testCond[ch].local_object[step][idx]
//		= myPs->testCond.step[step].edlc.capacitanceV1;
//
//	idx = IDX_LOC_OBJ_CAPACITANCE_V2;
//	myData->testCond[ch].local_object[step][idx]
//		= myPs->testCond.step[step].edlc.capacitanceV2;
//
	idx = IDX_LOC_OBJ_Z_T1;
	myTestCond->local_object[step][idx] = 0;
		//kjgw = (long)myPs->testCond.step[step].edlc.startT_Z;

	idx = IDX_LOC_OBJ_Z_T2;
	if(type == STEP_Z) {
		myTestCond->local_object[step][idx]
			= (long)myPs->testCond.z[client_step].endTime * 10;
			//kjgw = myPs->testCond.step[step].edlc.endT_Z;
	} else {
		l_val = (long)myPs->testCond.z[client_step].endTime * 10 - 2;
		if(l_val < 0) l_val = 0;
		myTestCond->local_object[step][idx] = l_val;
	}

//	idx = IDX_LOC_OBJ_LC_T1;
//	myData->testCond[ch].local_object[step][idx]
//		= (long)myPs->testCond.step[step].edlc.startT_LC;
//
//	idx = IDX_LOC_OBJ_LC_T2;
//	myData->testCond[ch].local_object[step][idx]
//kjgw_e		= (long)myPs->testCond.step[step].edlc.endT_LC;

	//fault_condition
	idx = IDX_LOC_OBJ_FAULT_UPPER_V;
	myTestCond->local_object[step][idx]
		= myPs->testCond.z[client_step].upperV * 100;
	make2_loc_chCode_cond(ch, step, C_CD_FAULT_UPPER_V, 1,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_LOC_OBJ_FAULT_LOWER_V;
	myTestCond->local_object[step][idx]
		= myPs->testCond.z[client_step].lowerV * 100;
	make2_loc_chCode_cond(ch, step, C_CD_FAULT_LOWER_V, 1,
		COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=

//kjgw_s	idx = IDX_LOC_OBJ_FAULT_UPPER_TEMP;
//	myData->testCond[ch].local_object[step][idx]
//		= myPs->testCond.step[step].faultUpperTemp;
//	make2_loc_chCode_cond(ch, step, C_CD_FAULT_UPPER_TEMP, 1,
//		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=
//
//	idx = IDX_LOC_OBJ_FAULT_LOWER_TEMP;
//	myData->testCond[ch].local_object[step][idx]
//		= myPs->testCond.step[step].faultLowerTemp;
//	make2_loc_chCode_cond(ch, step, C_CD_FAULT_LOWER_TEMP, 1,
//kjgw_e		COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=

	switch(type) {
		case STEP_CHARGE:
			if(mode == MODE_CC_CV || mode == MODE_CC || mode == MODE_CV) {
				idx = IDX_LOC_OBJ_FAULT_HW_UPPER_V;
				myTestCond->local_object[step][idx] = 5000; //5.0%
				make2_loc_chCode_cond(ch, step, C_CD_FAULT_HW_UPPER_V, 1,
					COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=
			}
			break;
		case STEP_DISCHARGE:
			if(mode == MODE_CC_CV || mode == MODE_CC || mode == MODE_CV) {
				idx = IDX_LOC_OBJ_FAULT_HW_LOWER_V;
				myTestCond->local_object[step][idx] = 5000; //5.0%
				make2_loc_chCode_cond(ch, step, C_CD_FAULT_HW_LOWER_V, 1,
					COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=
			}
			break;
		case STEP_Z:
			if(mode == MODE_DC) {
				idx = IDX_LOC_OBJ_FAULT_HW_LOWER_V;
				myTestCond->local_object[step][idx] = 5000; //5.0%
				make2_loc_chCode_cond(ch, step, C_CD_FAULT_HW_LOWER_V, 1,
					COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=
			}
			break;
		default: break;
	}

	idx = IDX_LOC_OBJ_FAULT_HW_UPPER_I;
	myTestCond->local_object[step][idx] = 5000; //5.0%
	make2_loc_chCode_cond(ch, step, C_CD_FAULT_HW_UPPER_I, 1,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=
//kjgw_s
//	idx = IDX_LOC_OBJ_FAULT_HW_LOWER_I;
//	myTestCond->local_object[step][idx] = 5000; //5.0%
//	make2_loc_chCode_cond(ch, step, C_CD_FAULT_HW_LOWER_I, 1,
//		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=
//kjgw_e
	idx = IDX_LOC_OBJ_FAULT_UPPER_I;
	if(type == STEP_CHARGE) {
		myTestCond->local_object[step][idx]
			= myPs->testCond.z[client_step].upperI * 10;
		make2_loc_chCode_cond(ch, step, C_CD_FAULT_UPPER_I, 1,
			COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=
	} else if(type == STEP_DISCHARGE || type == STEP_Z) {
		myTestCond->local_object[step][idx]
			= myPs->testCond.z[client_step].upperI * (-10);
		make2_loc_chCode_cond(ch, step, C_CD_FAULT_UPPER_I, 1,
			COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=
	}

	idx = IDX_LOC_OBJ_FAULT_LOWER_I;
	if(type == STEP_CHARGE) {
		myTestCond->local_object[step][idx]
			= myPs->testCond.z[client_step].lowerI * 10;
		make2_loc_chCode_cond(ch, step, C_CD_FAULT_LOWER_I, 1,
			COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=
	} else if(type == STEP_DISCHARGE || type == STEP_Z) {
		myTestCond->local_object[step][idx]
			= myPs->testCond.z[client_step].lowerI * (-10);
		make2_loc_chCode_cond(ch, step, C_CD_FAULT_LOWER_I, 1,
			COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=
	}

	if(type == STEP_Z) {
		idx = IDX_LOC_OBJ_FAULT_UPPER_Z;
		myTestCond->local_object[step][idx]
			= myPs->testCond.z[client_step].upperZ;
		make2_loc_chCode_cond(ch, step, C_CD_FAULT_UPPER_Z, 1,
			COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

		idx = IDX_LOC_OBJ_FAULT_LOWER_Z;
		myData->testCond[ch].local_object[step][idx]
			= myPs->testCond.z[client_step].lowerZ;
		make2_loc_chCode_cond(ch, step, C_CD_FAULT_LOWER_Z, 1,
			COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=
	}

//kjgw_s	idx = IDX_LOC_OBJ_FAULT_UPPER_AMPARE_HOUR;
//	myTestCond->local_object[step][idx]
//		= myPs->testCond.z[client_step].upperCapacity * 10;
//	make2_loc_chCode_cond(ch, step, C_CD_FAULT_UPPER_AMPARE_HOUR, 1,
//		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=
//
//	idx = IDX_LOC_OBJ_FAULT_LOWER_AMPARE_HOUR;
//	myTestCond->local_object[step][idx]
//		= myPs->testCond.z[client_step].lowerCapacity * 10;
//	make2_loc_chCode_cond(ch, step, C_CD_FAULT_LOWER_AMPARE_HOUR, 1,
//kjgw_e		COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=

	//kjgw comp_v1, v2, v3, i1, i2, i3, dV, dI

	//step grade
	i = (int)myPs->testCond.gradeHeader[client_step].item;
	if(i != P2_GRADE_ITEM_IDLE) {
		myTestCond->grade[step].item = (unsigned char)i; //kjgw

		j = (int)myPs->testCond.gradeHeader[client_step].totalGrade;
		if(j > MAX_TEST_GRADE) j = MAX_TEST_GRADE; //kjgw
		myTestCond->grade[step].totalGrade = (unsigned char)j;

		for(i=0; i < j; i++) {
			myTestCond->grade[step].code[i]
				= (long)myPs->testCond.grade[client_step][i].code;

			myTestCond->grade[step].value1[i]
				= myPs->testCond.grade[client_step][i].value1;

			myTestCond->grade[step].value2[i]
				= myPs->testCond.grade[client_step][i].value2;
		}
	}
}

void convert_test_cond_pattern(S_TEST_CONDITION *myTestCond, int ch, int step, int client_step)
{
}

void StateChange_Pause(int num)
{
	int i, ch, toPs;
	S_MSG_VAL SendMsg;

	if(myPs->config.state_change == 1) {
		for(i=0; i < myPs->misc.chInGroup; i++) {
			//kjgw
			ch = (int)myData->CellArray1[myPs->misc.chOffset + i].number2 - 1;
			if(myData->cData[ch].op.state == C_RUN)
				myData->cData[ch].signal[C_SIG_CMD_PAUSE] = P1;
		}
	} else if(myPs->config.state_change == 2) {
		memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));

		toPs = COB1_TO_MODULE + myPs->config.groupNo;
		SendMsg.msg = MSG_COB_MODULE_SAVE_MSG_FLAG;
		SendMsg.val[0] = 0;
		SendMsg.val[1] = 1; //stop
		send_msg(toPs, (char *)&SendMsg);
	}

	close(myPs->misc.network_socket);
	userlog(DEBUG_LOG, psName, "network communication error : %d\n", num);
}

void Update_RealTime(char *real_time)
{
	char cmd[32];

	memset(cmd, 0, sizeof cmd);
	strcpy(cmd, "date -s '");
	strncat(cmd, real_time, 19);
	strcat(cmd, "'");
	system(cmd);

	userlog(DEBUG_LOG, psName, "Update real_time %s, %s\n", real_time, cmd);
}

int	Write_Cali_Set_Main(void)
{
	char fileName[256], buf[32];
	int i, j;
    FILE *fp;

	memset(buf, 0, sizeof buf);
	sprintf(buf, "%s", "Cali_Set_Main");

	memset(fileName, 0, sizeof fileName);
	sprintf(fileName, "%s%s", "/root/formation_data/config/caliData/", buf);

    if((fp = fopen(fileName, "w")) == NULL) {
		userlog(DEBUG_LOG, psName, "Cali_Set_Main file write error\n");
		return -1;
	}

    fprintf(fp, "mainDA\n");
	fprintf(fp, "\n");

	fprintf(fp, "DAV_set_point\n");
	for(i=0; i < MAX_RANGE; i++) {
		fprintf(fp, "RANGE%d\n", i+1);
		fprintf(fp, "cali_point %d\n",
			myData->cali_set_data.main_dav.set_point_num[i]);
		for(j=0; j < MAX_F_CALI_POINT; j++) {
			fprintf(fp, "%ld ", myData->cali_set_data.main_dav.set_point[i][j]);
		}
		fprintf(fp, "\n");
	}

	fprintf(fp, "DAV_check_point\n");
	for(i=0; i < MAX_RANGE; i++) {
		fprintf(fp, "RANGE%d\n", i+1);
		fprintf(fp, "cali_point %d\n",
			myData->cali_set_data.main_dav.check_point_num[i]);
		for(j=0; j < MAX_F_CALI_CHECK_POINT; j++) {
			fprintf(fp, "%ld ",
				myData->cali_set_data.main_dav.check_point[i][j]);
		}
		fprintf(fp, "\n");
	}

	fprintf(fp, "DAI_set_point\n");
	for(i=0; i < MAX_RANGE; i++) {
		fprintf(fp, "RANGE%d\n", i+1);
		fprintf(fp, "cali_point %d\n",
			myData->cali_set_data.main_dai.set_point_num[i]);
		for(j=0; j < MAX_F_CALI_POINT; j++) {
			fprintf(fp, "%ld ", myData->cali_set_data.main_dai.set_point[i][j]);
		}
		fprintf(fp, "\n");
	}

	fprintf(fp, "DAI_check_point\n");
	for(i=0; i < MAX_RANGE; i++) {
		fprintf(fp, "RANGE%d\n", i+1);
		fprintf(fp, "cali_point %d\n",
			myData->cali_set_data.main_dai.check_point_num[i]);
		for(j=0; j < MAX_F_CALI_CHECK_POINT; j++) {
			fprintf(fp, "%ld ",
				myData->cali_set_data.main_dai.check_point[i][j]);
		}
		fprintf(fp, "\n");
	}

    fclose(fp);
	return 0;
}

int	Write_Cali_Set_Ch(void)
{
	char fileName[256], buf[32];
	int i, j;
    FILE *fp;

	memset(buf, 0, sizeof buf);
	sprintf(buf, "%s", "Cali_Set_Ch");

	memset(fileName, 0, sizeof fileName);
	sprintf(fileName, "%s%s", "/root/formation_data/config/caliData/", buf);

    if((fp = fopen(fileName, "w")) == NULL) {
		userlog(DEBUG_LOG, psName, "Cali_Set_Ch file write error\n");
		return -1;
	}

    fprintf(fp, "ch_ad_da\n");
	fprintf(fp, "\n");

	fprintf(fp, "V_set_point\n");
	for(i=0; i < MAX_RANGE; i++) {
		fprintf(fp, "RANGE%d\n", i+1);
		fprintf(fp, "cali_point %d\n",
			myData->cali_set_data.ch_v.set_point_num[i]);
		for(j=0; j < MAX_F_CALI_POINT; j++) {
			fprintf(fp, "%ld ", myData->cali_set_data.ch_v.set_point[i][j]);
		}
		fprintf(fp, "\n");
	}
	fprintf(fp, "\n");

	fprintf(fp, "V_check_point\n");
	for(i=0; i < MAX_RANGE; i++) {
		fprintf(fp, "RANGE%d\n", i+1);
		fprintf(fp, "cali_point %d\n",
			myData->cali_set_data.ch_v.check_point_num[i]);
		for(j=0; j < MAX_F_CALI_CHECK_POINT; j++) {
			fprintf(fp, "%ld ", myData->cali_set_data.ch_v.check_point[i][j]);
		}
		fprintf(fp, "\n");
	}
	fprintf(fp, "\n");

	fprintf(fp, "I_set_point\n");
	for(i=0; i < MAX_RANGE; i++) {
		fprintf(fp, "RANGE%d\n", i+1);
		fprintf(fp, "cali_point %d\n",
			myData->cali_set_data.ch_i.set_point_num[i]);
		for(j=0; j < MAX_F_CALI_POINT; j++) {
			fprintf(fp, "%ld ", myData->cali_set_data.ch_i.set_point[i][j]);
		}
		fprintf(fp, "\n");
	}
	fprintf(fp, "\n");

	fprintf(fp, "I_check_point\n");
	for(i=0; i < MAX_RANGE; i++) {
		fprintf(fp, "RANGE%d\n", i+1);
		fprintf(fp, "cali_point %d\n",
			myData->cali_set_data.ch_i.check_point_num[i]);
		for(j=0; j < MAX_F_CALI_CHECK_POINT; j++) {
			fprintf(fp, "%ld ", myData->cali_set_data.ch_i.check_point[i][j]);
		}
		fprintf(fp, "\n");
	}
	fprintf(fp, "\n");

    fclose(fp);
	return 0;
}

int	Write_jig_temp_set_data(void)
{
	char fileName[256];
	int group;
    FILE *fp;

	group = (int)myPs->config.groupNo;

	memset(fileName, 0, sizeof fileName);
	strcpy(fileName, "/root/formation_data/config/parameter/client/");
	strcat(fileName, psName);
	strcat(fileName, "/JigTempSetData");
	// /root/formation_data/config/parameter/client/COB?/JigTempSetData
    if((fp = fopen(fileName, "w+")) == NULL) {
		userlog(DEBUG_LOG, psName, "%s JigTempSetData file write error\n",
		psName);
		return -1;
	}

    fprintf(fp, "useJigTempSetData	:	%d\n",
		myData->gData[group].jigTempSetData.useJigTempSetData);
    fprintf(fp, "curJigAvgTemp[100='C]	:	%d\n",
		myData->gData[group].jigTempSetData.curJigAvgTemp);
    fprintf(fp, "checkTempTime[Min]	:	%d\n",
		myData->gData[group].jigTempSetData.checkTempTime);
    fprintf(fp, "targetTemp[100='C]	:	%d\n",
		myData->gData[group].jigTempSetData.targetTemp);
    fclose(fp);

	return 0;
}

int	Read_jig_temp_set_data(void)
{
	char temp[32], buf[32], fileName[256];
    int tmp, group;
    FILE *fp;

	S_P2_JIG_TEMP_SET_DATA jigTempSetData;

	group = (int)myPs->config.groupNo;

	memset(fileName, 0, sizeof(fileName));
	strcpy(fileName, "/root/formation_data/config/parameter/client/");
	strcat(fileName, psName);
	strcat(fileName, "/JigTempSetData");
	// /root/formation_data/config/parameter/client/COB?/JigTempSetData
    if((fp = fopen(fileName, "r")) == NULL) {
		userlog(DEBUG_LOG, psName, "%s JigTempSetData file read error\n",
			psName);
		return -1;
	}

	memset((char *)&jigTempSetData, 0, sizeof(S_P2_JIG_TEMP_SET_DATA));

    tmp = fscanf(fp, "%s", temp); tmp = fscanf(fp, "%s", temp);
	memset(buf, 0, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    jigTempSetData.useJigTempSetData = (unsigned char)atoi(buf);

    tmp = fscanf(fp, "%s", temp); tmp = fscanf(fp, "%s", temp);
	memset(buf, 0, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    jigTempSetData.curJigAvgTemp = (unsigned short)atoi(buf);

    tmp = fscanf(fp, "%s", temp); tmp = fscanf(fp, "%s", temp);
	memset(buf, 0, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    jigTempSetData.checkTempTime = (unsigned short)atoi(buf);//min

    tmp = fscanf(fp, "%s", temp); tmp = fscanf(fp, "%s", temp);
	memset(buf, 0, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    jigTempSetData.targetTemp = (unsigned short)atoi(buf);

	memcpy((char *)&myData->gData[group].jigTempSetData, 
		(char *)&jigTempSetData, sizeof(S_P2_JIG_TEMP_SET_DATA));
    fclose(fp);

	return 0;
}

int get_chInGroup(int group)
{
	int chInGroup;

	switch(myPs->misc.parallel_count) {
		case 1:
			chInGroup = (int)myData->mData.config.chInGroup[group];
			break;
		case 2:
			chInGroup = (int)myData->mData.config.chInGroup[group] / 2;
			break;
		case 3:
			chInGroup = (int)myData->mData.config.chInGroup[group] / 3;
			break;
		case 4:
			chInGroup = (int)myData->mData.config.chInGroup[group] / 4;
			break;
		default:
			chInGroup = (int)myData->mData.config.chInGroup[group];
			break;
	}

	return chInGroup;
}

int get_chNo(int group, int ch)
{
	int cnt, channel;

	cnt = (int)myPs->misc.parallel_count;

	channel = ch * cnt;

	return channel;
}

void convert_ch_attribute(int group)
{
	int toPs, cnt, i, j, ch;
	S_MSG_VAL SendMsg;

	cnt = (int)myPs->misc.parallel_count;

	ch = 0;
	for(i=0; i < MAX_CH_256; i++) {
		switch(cnt) {
			case 1:
				myData->ChAttribute[i].chNo_master = i + 1;
				myData->ChAttribute[i].chNo_slave[0] = 0;
				myData->ChAttribute[i].chNo_slave[1] = 0;
				myData->ChAttribute[i].chNo_slave[2] = 0;
				myData->ChAttribute[i].opType = 0; //independent
				break;
			case 2:
				if((i % cnt) == 0) {
					j = ch * cnt + 1;
					if(j > MAX_CH_256) j = 0;
					myData->ChAttribute[i].chNo_master = j;
					j++;
					if(j > MAX_CH_256) j = 0;
					myData->ChAttribute[i].chNo_slave[0] = j;
					myData->ChAttribute[i].chNo_slave[1] = 0;
					myData->ChAttribute[i].chNo_slave[2] = 0;
					myData->ChAttribute[i].opType = 1; //parallel
					ch++;
				} else {
					myData->ChAttribute[i].chNo_master = 0;
					myData->ChAttribute[i].chNo_slave[0] = 0;
					myData->ChAttribute[i].chNo_slave[1] = 0;
					myData->ChAttribute[i].chNo_slave[2] = 0;
					myData->ChAttribute[i].opType = 0;
				}
				break;
			case 3:
				if((i % cnt) == 0) {
					j = ch * cnt + 1;
					if(j > MAX_CH_256) j = 0;
					myData->ChAttribute[i].chNo_master = j;
					j++;
					if(j > MAX_CH_256) j = 0;
					myData->ChAttribute[i].chNo_slave[0] = j;
					j++;
					if(j > MAX_CH_256) j = 0;
					myData->ChAttribute[i].chNo_slave[1] = j;
					myData->ChAttribute[i].chNo_slave[2] = 0;
					myData->ChAttribute[i].opType = 1; //parallel
					ch++;
				} else {
					myData->ChAttribute[i].chNo_master = 0;
					myData->ChAttribute[i].chNo_slave[0] = 0;
					myData->ChAttribute[i].chNo_slave[1] = 0;
					myData->ChAttribute[i].chNo_slave[2] = 0;
					myData->ChAttribute[i].opType = 0;
				}
				break;
			case 4:
				if((i % cnt) == 0) {
					j = ch * cnt + 1;
					if(j > MAX_CH_256) j = 0;
					myData->ChAttribute[i].chNo_master = j;
					j++;
					if(j > MAX_CH_256) j = 0;
					myData->ChAttribute[i].chNo_slave[0] = j;
					j++;
					if(j > MAX_CH_256) j = 0;
					myData->ChAttribute[i].chNo_slave[1] = j;
					j++;
					if(j > MAX_CH_256) j = 0;
					myData->ChAttribute[i].chNo_slave[2] = j;
					myData->ChAttribute[i].opType = 1; //parallel
					ch++;
				} else {
					myData->ChAttribute[i].chNo_master = 0;
					myData->ChAttribute[i].chNo_slave[0] = 0;
					myData->ChAttribute[i].chNo_slave[1] = 0;
					myData->ChAttribute[i].chNo_slave[2] = 0;
					myData->ChAttribute[i].opType = 0;
				}
				break;
			default:
				myData->ChAttribute[i].chNo_master = i + 1;
				myData->ChAttribute[i].chNo_slave[0] = 0;
				myData->ChAttribute[i].chNo_slave[1] = 0;
				myData->ChAttribute[i].chNo_slave[2] = 0;
				myData->ChAttribute[i].opType = 0; //independent
				break;
		}
	}

	memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));

	toPs = COB1_TO_APP + group;
	SendMsg.msg = MSG_COB_APP_WRITE_CH_ATTRIBUTE;
	SendMsg.val[0] = group;
	send_msg(toPs, (char *)&SendMsg);
}

