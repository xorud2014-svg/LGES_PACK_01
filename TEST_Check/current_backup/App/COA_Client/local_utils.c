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
extern volatile S_COA_CLIENT *myPs;
extern volatile S_TEST_CONDITION *myTestCond; //kjg_170810
extern char psName[PROCESS_NAME_SIZE];

int Initialize(int argc, char *argv[])
{
	int CO_No, group, rtn;//, i, j;

	if(argc != 1) {
		printf("COA_Client start fail %d\n", argc);
		return -1;
	}

	if(Open_SystemMemory(0) < 0) return -2;
	
	CO_No = argv[0][5] - 0x31;
	myPs = &(myData->COA_Client[CO_No]);
	
	Init_SystemMemory(CO_No);
	
	rtn = Read_COA_Client_Config();
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "Read_COA_Client_Config error %d\n", rtn);
		return -3;
	}

	//kjg_w Read_COA_Client_SetData - ConfigData
	rtn = Read_COA_Client_testConfig();
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName,
			"Read_COA_Client_testConfig error %d\n", rtn);
		return -4;
	}

	group = myPs->config.groupNo;
	myPs->misc.chOffset = 0;
	/*for(i=0; i < MAX_GROUP_PER_MODULE; i++) {
		j = (int)myData->mData.config.chInGroup[i];
		if(group == i) {
			myPs->misc.chInGroup = j;
			break;
		}
		myPs->misc.chOffset += j;
	}kjg_w*/
	myPs->misc.chInGroup = (int)myData->mData.config.installedCh;

	if(myPs->config.protocol_version <= P1_PROTOCOL_VERSION_9) {
		myPs->misc.network_port_type = 1; //network port 1ea
	} else {
		myPs->misc.network_port_type = 2; //network port 2ea
	}

	myData->AppControl.signal[myPs->misc.psSignal] = P1;
	return 0;
}

void Init_SystemMemory(int CO_No)
{
	memset((char *)&psName, 0, PROCESS_NAME_SIZE);
	sprintf(psName, "COA%d", CO_No+1);
   	myPs->misc.psSignal = APP_SIG_COA1_CLIENT_PROCESS + CO_No;

	memset((char *)&myPs->signal, 0, MAX_SIGNAL);

	memset((char *)&myPs->rcvCmd, 0, sizeof(S_P1_RCV_COMMAND));
	memset((char *)&myPs->rcvPacket, 0, sizeof(S_P1_RCV_PACKET));
	
	memset((char *)&myPs->reply, 0, sizeof(S_P1_REPLY));

	myPs->misc.cmd_serial = 0;
	myPs->misc.net_time = 0;
	myPs->misc.net_time2 = 0;
	myPs->misc.ping_time = 0;
	myPs->misc.ping_time2 = 0;
	myPs->misc.sent_monitor_data_time = 0;
	myPs->misc.sent_monitor_data_time2 = 0;
	//jhkw_190503s
	myPs->misc.net_time = myData->mData.misc.timer_1sec;
	myPs->misc.net_time2 = myData->mData.misc.timer_1000ms;
	//jhkw_190503e

	//kjg_120430
	myPs->misc.rcv_test_step_count = 0;
	myPs->misc.rcv_test_pattern_count = 0;

	myPs->misc.processPointer = (int)&myData;
	myPs->misc.CO_No = CO_No;
}

int	Read_COA_Client_Config(void)
{
    int tmp;
	char temp[32], buf[16], fileName[128];
    FILE *fp;

	memset(fileName, 0, sizeof(fileName));
	strcpy(fileName, "/root/");
	strcat(fileName, (char *)&myData->AppControl.misc.path1);
	strcat(fileName, "/config/parameter/client/");
	strcat(fileName, psName);
	strcat(fileName, "/Client_Config");
	// /root/system_data/config/parameter/client/COA?/Client_Config
    if((fp = fopen(fileName, "r")) == NULL) {
		userlog(DEBUG_LOG, psName, "%s_Client_Config file read error\n",
		psName);
		return -1;
	}

	memset(temp, 0, sizeof temp);
    tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "groupId") != 0) return -2;
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) return -2;
	memset(buf, 0, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	myPs->config.groupId = (short int)atoi(buf);

	memset(temp, 0, sizeof temp);
    tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "groupNo") != 0) return -2;
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) return -2;
	memset(buf, 0, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	myPs->config.groupNo = (short int)(atoi(buf) - 1);

	memset(temp, 0, sizeof temp);
    tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "ipAddr") != 0) return -2;
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) return -2;
	memset(buf, 0, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	memcpy((char *)&myPs->config.ipAddr, (char *)&buf, 16);
		
	memset(temp, 0, sizeof temp);
    tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "networkPort1") != 0) return -2;
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) return -2;
	memset(buf, 0, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->config.networkPort1 = atoi(buf);

	memset(temp, 0, sizeof temp);
    tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "networkPort2") != 0) return -2;
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) return -2;
	memset(buf, 0, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->config.networkPort2 = atoi(buf);

	memset(temp, 0, sizeof temp);
    tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "protocol_version") != 0) return -2;
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) return -2;
	memset(buf, 0, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	myPs->config.protocol_version = atoi(buf);
    	
	memset(temp, 0, sizeof temp);
    tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "retryCount") != 0) return -2;
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) return -2;
	memset(buf, 0, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->config.retryCount = atoi(buf);

	memset(temp, 0, sizeof temp);
    tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "replyTimeout") != 0) return -2;
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) return -2;
	memset(buf, 0, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->config.replyTimeout = atol(buf);

	memset(temp, 0, sizeof temp);
    tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "netTimeout") != 0) return -2;
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) return -2;
	memset(buf, 0, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->config.netTimeout = atol(buf);

	memset(temp, 0, sizeof temp);
    tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "pingTimeout") != 0) return -2;
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) return -2;
	memset(buf, 0, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->config.pingTimeout = atol(buf);

	memset(temp, 0, sizeof temp);
    tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "CmdSendLog") != 0) return -2;
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) return -2;
	memset(buf, 0, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->config.CmdSendLog = (unsigned char)atoi(buf);

	memset(temp, 0, sizeof temp);
    tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "CmdRcvLog") != 0) return -2;
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) return -2;
	memset(buf, 0, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->config.CmdRcvLog = (unsigned char)atoi(buf);

	memset(temp, 0, sizeof temp);
    tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "CmdSendLog_Hex") != 0) return -2;
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) return -2;
	memset(buf, 0, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->config.CmdSendLog_Hex = (unsigned char)atoi(buf);

	memset(temp, 0, sizeof temp);
    tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "CmdRcvLog_Hex") != 0) return -2;
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) return -2;
	memset(buf, 0, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->config.CmdRcvLog_Hex = (unsigned char)atoi(buf);

	memset(temp, 0, sizeof temp);
    tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "CommCheckLog") != 0) return -2;
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) return -2;
	memset(buf, 0, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->config.CommCheckLog = (unsigned char)atoi(buf);

	memset(temp, 0, sizeof temp);
    tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "send_monitor_data_interval") != 0) return -2;
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) return -2;
	memset(buf, 0, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->config.send_monitor_data_interval = atol(buf);

	memset(temp, 0, sizeof temp);
    tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "send_save_data_interval") != 0) return -2;
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) return -2;
	memset(buf, 0, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->config.send_save_data_interval = atol(buf);

	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "state_change") != 0) return -2;
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) return -2;
	memset(buf, 0, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	//1:pause - network fail, 2:run - network fail
	myPs->config.state_change = (unsigned char)atoi(buf);
    	
	//jhkw_190830s
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "data_save_10ms") != 0) return -2;
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) return -2;
	memset(buf, 0, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	//0: nomal, 1: 10ms & sbc don't save
	myPs->config.data_save_10ms = (unsigned char)atoi(buf);
	//jhkw_190830e
	
	//khj_200909s
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "Real_Time_request") != 0) return -3;
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) return -3;
	memset(buf, 0, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	//0: nomal, 1: 10ms & sbc don't save
	myPs->config.Real_Time_request = (unsigned char)atoi(buf);
	//khj_200909
    	
    fclose(fp);
	return 0;
}

int	Write_COA_Client_Config(void)
{
	char fileName[128];
    FILE *fp;

	memset(fileName, 0, sizeof(fileName));
	strcpy(fileName, "/root/");
	strcat(fileName, (char *)&myData->AppControl.misc.path1);
	strcat(fileName, "/config/parameter/client/");
	strcat(fileName, psName);
	strcat(fileName, "/Client_Config");
	// /root/system_data/config/parameter/client/COA?/Client_Config
    if((fp = fopen(fileName, "w")) == NULL) {
		userlog(DEBUG_LOG, psName, "%s_Client_Config file write error\n",
		psName);
		return -1;
	}

    fprintf(fp, "groupId             :   %d\n", myPs->config.groupId);
    fprintf(fp, "groupNo             :   %d\n", (int)(myPs->config.groupNo+1));
	fprintf(fp, "ipAddr              :   %s\n", myPs->config.ipAddr);
	fprintf(fp, "networkPort1        :   %d\n", myPs->config.networkPort1);
	fprintf(fp, "networkPort2        :   %d\n", myPs->config.networkPort2);
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
	fprintf(fp, "state_change        :   %d\n", myPs->config.state_change);
	//jhkw_190830
	fprintf(fp, "data_save_10ms      :   %d\n", myPs->config.data_save_10ms);
    	
    fclose(fp);
	return 0;
}

int	Read_COA_Client_testConfig(void)
{
    int tmp;
	char temp[32], buf[12], fileName[128];
    FILE *fp;

	memset((char *)&myPs->testConfig, 0, sizeof(S_P1_TEST_CONFIG));

	memset(fileName, 0, sizeof(fileName));
	strcpy(fileName, "/root/");
	strcat(fileName, (char *)&myData->AppControl.misc.path1);
	strcat(fileName, "/config/parameter/client/");
	strcat(fileName, psName);
	strcat(fileName, "/testConfig");
	// /root/system_data/config/parameter/client/COA?/testConfig
    if((fp = fopen(fileName, "r")) == NULL) {
		userlog(DEBUG_LOG, psName, "testConfig file read error\n", psName);
		return -1;
	}

    tmp = fscanf(fp, "%s", temp); tmp = fscanf(fp, "%s", temp);
	memset(buf, 0, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->testConfig.maxRefV = atol(buf);

    tmp = fscanf(fp, "%s", temp); tmp = fscanf(fp, "%s", temp);
	memset(buf, 0, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->testConfig.minRefV = atol(buf);

    tmp = fscanf(fp, "%s", temp); tmp = fscanf(fp, "%s", temp);
	memset(buf, 0, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->testConfig.maxRefI = atol(buf);

    tmp = fscanf(fp, "%s", temp); tmp = fscanf(fp, "%s", temp);
	memset(buf, 0, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->testConfig.minRefI = atol(buf);

    tmp = fscanf(fp, "%s", temp); tmp = fscanf(fp, "%s", temp);
	memset(buf, 0, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->testConfig.failDeltaV = atol(buf);

    tmp = fscanf(fp, "%s", temp); tmp = fscanf(fp, "%s", temp);
	memset(buf, 0, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->testConfig.failDeltaI = atol(buf);

    tmp = fscanf(fp, "%s", temp); tmp = fscanf(fp, "%s", temp);
	memset(buf, 0, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->testConfig.check_lowerI = atol(buf);

    tmp = fscanf(fp, "%s", temp); tmp = fscanf(fp, "%s", temp);
	memset(buf, 0, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->testConfig.check_lowerOCV = atol(buf);

    tmp = fscanf(fp, "%s", temp); tmp = fscanf(fp, "%s", temp);
	memset(buf, 0, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->testConfig.fail_charge_lower_dv = atol(buf);

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

			size = sizeof(long) * MAX_CALI_POINT * 6
				+ sizeof(double) * (MAX_CALI_POINT - 1) * 4
				+ sizeof(double) * 2;
			memcpy((char *)&myData->cali[ch].data[type][range].set_ad,
				(char *)&myData->cali[ch].tmpData[type][range].set_ad, size);
		}
	}
}

void CaliUpdateCh1(int ch) //kjhw_120506s Vref x 2
{
	int type, range, size;

	for(type=0; type < MAX_TYPE; type++) {
		for(range=0; range < MAX_RANGE; range++) {
			if(myData->cali[ch].tmpData[type][range].caliFlag == 0) continue;
			myData->cali[ch].tmpData[type][range].caliFlag = 0;

			memcpy((char *)&myData->cali[ch+4].data[type][range].point,
				(char *)&myData->cali[ch].tmpCond[type][range].point,
				sizeof(S_CALI_POINT));

			size = sizeof(long) * MAX_CALI_POINT * 6
				+ sizeof(double) * (MAX_CALI_POINT - 1) * 4
				+ sizeof(double) * 2;
			memcpy((char *)&myData->cali[ch+4].data[type][range].set_ad,
				(char *)&myData->cali[ch].tmpData[type][range].set_ad, size);
		}
	}
} //kjhw_120506e

int Write_BdCaliData(int bd)
{
    int	i, ch, type, range, point, chOffset;
	char temp[4], fileName[128];
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
	fprintf(fp, "%s\n\n", "date"); //kjg_w
	
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
			for(i=0; i < MAX_CH_PER_BD; i++) {
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

int read_test_cond_step_file(int ch, int div)
{ //kjh_140113
#if defined __COA_VER_100B__ || defined __COA_VER_100B2__ || defined __COA_VER_100C__
#else //COA_VER_100D~
	char cmd[256];
	int fp, rtn, i, size, checksum;
	S_P1_FILE_TEST_COND test_cond;

	//file check : kjg_w
	
	memset(cmd, 0, sizeof cmd);
	if(div == 0) { //first
		sprintf(cmd, "/root/START_INFO/CH%03d/sbc_schedule_info.sch", ch+1);
		// /root/START_INFO/CH00#/sbc_scedule_info.sch
	} else { //1 update kjg_170810
		sprintf(cmd, "/root/START_INFO/CH%03d/UPDATE/sbc_schedule_info.sch",
			ch+1);
		// /root/START_INFO/CH00#/UPDATE/sbc_scedule_info.sch
	}

	if((fp = open(cmd, O_RDONLY)) < 0) {
		userlog(DEBUG_LOG, psName, "Can not open %s file(load)\n", cmd);
		return -1;
	}

	rtn = read(fp, (char *)&test_cond, sizeof(S_P1_FILE_TEST_COND));
	if(rtn != (int)myPs->testCond.test_cond_file_size) {
		userlog(DEBUG_LOG, psName, "error %s size sbc:%d, gui:%d\n",
			cmd, rtn, (int)myPs->testCond.test_cond_file_size);
		rtn = -2;
	} else rtn = 0;
	close(fp);
	if(rtn < 0) return rtn;

	size = (int)myPs->testCond.test_cond_file_size;
	checksum = (int)myPs->testCond.test_cond_file_checksum;
	rtn = checksum_file(psName, ch, size, checksum, (char *)&test_cond);
	if(rtn != checksum) {
		userlog(DEBUG_LOG, psName, "error %s checksum sbc:%d, gui:%d\n",
			cmd, rtn, checksum);
		return -3;
	}
	
	if(myPs->testCond.header.totalStep != test_cond.header.totalStep) {
		userlog(DEBUG_LOG, psName, "error totalStep %d, %d\n",
			(int)myPs->testCond.header.totalStep,
			(int)test_cond.header.totalStep);
		return -4;
	}

	if(myPs->testCond.header.totalPatternCount
		!= test_cond.header.totalPatternCount) {
		userlog(DEBUG_LOG, psName, "error totalPatternCount %d, %d\n",
			(int)myPs->testCond.header.totalPatternCount,
			(int)test_cond.header.totalPatternCount);
		return -5;
	}

	memcpy((char *)&myPs->testCond.safety, (char *)&test_cond.safety,
		sizeof(S_P1_TEST_COND_SAFETY));

	for(i=0; i < (int)test_cond.header.totalStep; i++) {
		memcpy((char *)&myPs->testCond.step[i], (char *)&test_cond.step[i],
			sizeof(S_P1_TEST_COND_STEP));
	}

	if(div == 0) { //first
		userlog(DEBUG_LOG, psName,
			"read step_file completed ch:%d size:%d checksum:%d\n",
			ch+1, size, checksum);
	} else { //1 update
		userlog(DEBUG_LOG, psName,
			"read step_file completed(update) ch:%d size:%d checksum:%d\n",
			ch+1, size, checksum);
	}

#endif
	return 0;
}

int read_test_cond_time_step_file(int ch, int div)
{ //kjh_160418
#if defined __COA_VER_100B__ || defined __COA_VER_100B2__ || defined __COA_VER_100C__ || defined __COA_VER_100D__
#else //COA_VER_100F~
	char cmd[256];
	int fp, rtn, i, size, checksum;
	S_P1_FILE_TEST_COND_TIME test_cond_time;

	//file check : kjg_w
	
	memset(cmd, 0, sizeof cmd);
	if(div == 0) { //first
		sprintf(cmd,
			"/root/START_INFO/CH%03d/sbc_schedule_info_wait_goto.sch", ch+1);
		// /root/START_INFO/CH00#/sbc_scedule_info_wait_goto.sch
	} else { //1 update kjg_170810
		sprintf(cmd,
			"/root/START_INFO/CH%03d/UPDATE/sbc_schedule_info_wait_goto.sch",
			ch+1);
		// /root/START_INFO/CH00#/UPDATE/sbc_scedule_info_wait_goto.sch
	}

	if((fp = open(cmd, O_RDONLY)) < 0) {
		userlog(DEBUG_LOG, psName, "Can not open %s file(load)\n", cmd);
		return -1;
	}

	rtn = read(fp, (char *)&test_cond_time, sizeof(S_P1_FILE_TEST_COND_TIME));
	if(rtn != (int)myPs->testCond_time.test_cond_time_file_size) {
		userlog(DEBUG_LOG, psName, "error %s size sbc:%d, gui:%d\n", cmd, rtn,
			(int)myPs->testCond_time.test_cond_time_file_size);
		rtn = -2;
	} else rtn = 0;
	close(fp);
	if(rtn < 0) return rtn;

	size = (int)myPs->testCond_time.test_cond_time_file_size;
	checksum = (int)myPs->testCond_time.test_cond_time_file_checksum;
	rtn = checksum_file(psName, ch, size, checksum, (char *)&test_cond_time);
	if(rtn != checksum) {
		userlog(DEBUG_LOG, psName, "error %s checksum sbc:%d, gui:%d\n",
			cmd, rtn, checksum);
		return -3;
	}
	
	if(myPs->testCond.header.totalTimeSchCount
		!= test_cond_time.header.totalTimeSchCount) {
		userlog(DEBUG_LOG, psName, "error totalTimeSchCount %d, %d\n",
			(int)myPs->testCond.header.totalTimeSchCount,
			(int)test_cond_time.header.totalTimeSchCount);
		return -4;
	}

	for(i=0; i < (int)myPs->testCond.header.totalStep; i++) {
		memcpy((char *)&myPs->testCond_time.time_step[i],
			(char *)&test_cond_time.time_step[i],
			sizeof(S_P1_TEST_COND_TIME_STEP));
	}

	if(div == 0) { //first
		userlog(DEBUG_LOG, psName,
			"read time_step_file completed ch:%d size:%d checksum:%d\n",
			ch+1, size, checksum);
	} else { //1 update
		userlog(DEBUG_LOG, psName,
			"read time_step_file completed(update) ch:%d size:%d checksum:%d\n",
			ch+1, size, checksum);
	}
#endif
	return 0;
}

int convert_test_cond(int ch)
{
	int rtn, step, idx, i, fault_count;
	long attr_count, type;
	S_P1_TEST_COND_STEP *P1_stepCond;

	//kjg_170810 memset((char *)&myData->testCond[ch], 0, sizeof(S_TEST_CONDITION));
	memset((char *)myTestCond, 0, sizeof(S_TEST_CONDITION));

	//common_header
	idx = IDX_COM_OBJ_TOTAL_STEP;
	myTestCond->common_object[idx] = (long)myPs->testCond.header.totalStep;

//20181219 KHK --------------------------------------------------------
	idx = IDX_COM_OBJ_SCHEDULE_LINK_FLAG;
	myTestCond->common_object[idx] = (long)myPs->testCond.safety.schedule_link_flag;
	userlog(DEBUG_LOG, psName,"Schedule Link Flag %d\n"
		, myTestCond->common_object[idx]);

	idx = IDX_COM_OBJ_USER_DEFINE_MODE_FLAG;
	myTestCond->common_object[idx] = (long)myPs->testCond.safety.user_define_mode_flag;
	userlog(DEBUG_LOG, psName,"Schedule user define mode %d\n"
		, myTestCond->common_object[idx]);
	if(myTestCond->common_object[idx] == P1){
		rtn = Read_User_Define_Mode_Charging_Count_File(psName, ch);
		if(rtn < 0) {
			userlog(DEBUG_LOG, psName, "ch%d ChargingCount file error %d\n", ch+1, rtn);
			return rtn;
		}
		rtn = Read_User_Define_Mode_Charging_RPT_SOC_File(psName, ch);
		if(rtn < 0) {
			userlog(DEBUG_LOG, psName, "ch%d Charging RPTSOC file error %d\n", ch+1, rtn);
			return rtn;
		}
	}
//---------------------------------------------------------------------
#if defined __COA_VER_100B__ || defined __COA_VER_100B2__ || defined __COA_VER_100C__
#else //COA_VER_100D~
	idx = IDX_COM_OBJ_TOTAL_PATTERN;
	myTestCond->common_object[idx]
		= (long)myPs->testCond.header.totalPatternCount;
#endif

	idx = IDX_COM_OBJ_ATTRIBUTE_COUNT;
	attr_count = 1;
	if(myData->ChAttribute[ch].opType != OP_INDEPENDENT) {
		for(i=0; i < 3; i++) {
			if(myData->ChAttribute[ch].chNo_slave[i] != 0) attr_count++;
		}
	}
	myTestCond->common_object[idx] = attr_count;

	//common_safety
	switch(myData->AppControl.config.systemModel) {
		//for motor inverter application
		case C_DAEWOO_450V_200A_10A_90KW:
		case C_DAEWOO_450V_200A_10A_90KW_2:
		case C_DAEWOO_450V_200A_50A_90KW:
		case C_DAEWOO_450V_200A_50A_90KW_2:	//jhk_150710
			fault_count = 50; //500ms delay
			break;
		default:
			fault_count = 1;
			break;
	}

	//jhkw_190503s
	idx = IDX_COM_OBJ_RPTSOC;
	myTestCond->common_object[idx] = myPs->testCond.safety.rptsoc; //uAh
	//myTestCond->common_object[idx] = 1000000;
	
	idx = IDX_COM_OBJ_SOC;
	myTestCond->common_object[idx] = myPs->testCond.safety.soc; //10 base
	myTestCond->common_object[idx] = myTestCond->common_object[idx] / 100;
	//myTestCond->common_object[idx] = 1000;
	//jhkw_190503s
	
	idx = IDX_COM_OBJ_FAULT_HW_UPPER_V;
	myTestCond->common_object[idx] = 5000; //5.0%
	make_com_chCode_cond((S_TEST_CONDITION *)myTestCond,
		C_CD_COM_FAULT_HW_UPPER_V, fault_count,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_COM_OBJ_FAULT_HW_LOWER_V;
	myTestCond->common_object[idx] = 5000; //5.0%
	make_com_chCode_cond((S_TEST_CONDITION *)myTestCond,
		C_CD_COM_FAULT_HW_LOWER_V, fault_count,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_COM_OBJ_FAULT_HW_UPPER_I;
	myTestCond->common_object[idx] = 5000; //5.0%
	make_com_chCode_cond((S_TEST_CONDITION *)myTestCond,
		C_CD_COM_FAULT_HW_UPPER_I, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_COM_OBJ_FAULT_HW_LOWER_I;
	myData->testCond[ch].common_object[idx] = 5000; //5.0%
	make_com_chCode_cond((S_TEST_CONDITION *)myTestCond,
		C_CD_COM_FAULT_HW_LOWER_I, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_COM_OBJ_FAULT_UPPER_V;
	myTestCond->common_object[idx] = myPs->testCond.safety.faultUpperV;
	make_com_chCode_cond((S_TEST_CONDITION *)myTestCond,
		C_CD_COM_FAULT_UPPER_V, fault_count,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_COM_OBJ_FAULT_LOWER_V;
	myTestCond->common_object[idx] = myPs->testCond.safety.faultLowerV;
	make_com_chCode_cond((S_TEST_CONDITION *)myTestCond,
		C_CD_COM_FAULT_LOWER_V, fault_count,
		COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=

#if defined __COA_VER_100B__ || defined __COA_VER_100B2__ || defined __COA_VER_100C__
	idx = IDX_COM_OBJ_FAULT_UPPER_I;
	myTestCond->common_object[idx] = myPs->testCond.safety.faultUpperI;
	make_com_chCode_cond((S_TEST_CONDITION *)myTestCond,
		C_CD_COM_FAULT_UPPER_I, 3, COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=
		//kjg_140206 1 -> 3
#else //COA_VER_100D~
	idx = IDX_COM_OBJ_FAULT_UPPER_I;
	myTestCond->common_object[idx] = myPs->testCond.safety.faultUpperI;
	make_com_chCode_cond((S_TEST_CONDITION *)myTestCond,
		C_CD_COM_FAULT_UPPER_I, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=
#endif

	idx = IDX_COM_OBJ_FAULT_UPPER_P;
	myTestCond->common_object[idx] = myPs->testCond.safety.faultUpperP;
	make_com_chCode_cond((S_TEST_CONDITION *)myTestCond,
		C_CD_COM_FAULT_UPPER_P, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_COM_OBJ_FAULT_UPPER_WATT_HOUR;
	myTestCond->common_object[idx] = myPs->testCond.safety.faultUpper_WattHour;
	make_com_chCode_cond((S_TEST_CONDITION *)myTestCond,
		C_CD_COM_FAULT_UPPER_WATT_HOUR, 1,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_COM_OBJ_FAULT_UPPER_AMPARE_HOUR;
	myTestCond->common_object[idx] = myPs->testCond.safety
		.faultUpper_AmpareHour;
	make_com_chCode_cond((S_TEST_CONDITION *)myTestCond,
		C_CD_COM_FAULT_UPPER_AMPARE_HOUR, 1,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_COM_OBJ_FAULT_UPPER_TEMP;
	myTestCond->common_object[idx] = myPs->testCond.safety.faultUpperTemp;
	make_com_chCode_cond((S_TEST_CONDITION *)myTestCond,
		C_CD_COM_FAULT_UPPER_TEMP, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_COM_OBJ_FAULT_LOWER_TEMP;
	myTestCond->common_object[idx] = myPs->testCond.safety.faultLowerTemp;
	make_com_chCode_cond((S_TEST_CONDITION *)myTestCond,
		C_CD_COM_FAULT_LOWER_TEMP, 1, COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=

#if defined __COA_VER_100B__ || defined __COA_VER_100B2__ || defined __COA_VER_100C__
#else //COA_VER_100D~
	idx = IDX_COM_OBJ_FAULT_COMP_AUX_V; //kjhw_150730
	myTestCond->common_object[idx] = myPs->testCond.safety.faultCompAuxV;
	make_com_chCode_cond((S_TEST_CONDITION *)myTestCond,
		C_CD_COM_FAULT_COMP_AUX_V, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_COM_OBJ_FAULT_DELTA_V; //jhkw_160828
	myTestCond->common_object[idx] = 10000; //10.0%
	make_com_chCode_cond((S_TEST_CONDITION *)myTestCond,
		C_CD_COM_FAULT_DELTA_V, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=
		//1s delay
#endif
	//jhkw_191108s
	idx = IDX_COM_OBJ_CURRENT_AH;
	myTestCond->common_object[idx] = myPs->testCond.safety.current_AH; //uAh

	idx = IDX_COM_OBJ_INITIAL_AH;
	myTestCond->common_object[idx] = myPs->testCond.safety.initial_AH; //uAh

	idx = IDX_COM_OBJ_CRATE_FACTOR;
	myTestCond->common_object[idx] = myPs->testCond.safety.crate_factor; //uAh
	//jhkw_191108e
	//jhkw_200507s
	idx = IDX_COM_OBJ_FAULT_FIX_UPPER_AUXV;
	myTestCond->common_object[idx] = myPs->testCond.safety.faultUpper_AuxV;
//	make_com_chCode_cond((S_TEST_CONDITION *)myTestCond,
//		C_CD_COM_FAULT_FIX_UPPER_AUXV, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=
	userlog(DEBUG_LOG, psName,"FIX_UPPER_AUXV %ld\n", myTestCond->common_object[idx]);

	idx = IDX_COM_OBJ_FAULT_FIX_LOWER_AUXV;
	myTestCond->common_object[idx] = myPs->testCond.safety.faultLower_AuxV;
//	make_com_chCode_cond((S_TEST_CONDITION *)myTestCond,
//		C_CD_COM_FAULT_FIX_LOWER_AUXV, 1, COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=
	userlog(DEBUG_LOG, psName,"FIX_LOWER_AUXV %ld\n", myTestCond->common_object[idx]);

	idx = IDX_COM_OBJ_FAULT_FIX_UPPER_TEMP;
	myTestCond->common_object[idx] = myPs->testCond.safety.faultUpper_Temp;
	userlog(DEBUG_LOG, psName,"FIX_UPPER_TEMP %ld\n", myTestCond->common_object[idx]);
	//jhkw_200507e
	
	//shhw_230605s
	idx = IDX_COM_OBJ_FAULT_FIX_DELTA_I;
	myTestCond->common_object[idx] = myPs->testCond.safety.faultDelta_I;
	userlog(DEBUG_LOG, psName,"FIX_DELTA_I %ld\n", myTestCond->common_object[idx]);
	
	idx = IDX_COM_OBJ_FAULT_FIX_DELTA_AUXV;
	myTestCond->common_object[idx] = myPs->testCond.safety.faultDelta_AuxV;
	userlog(DEBUG_LOG, psName,"FIX_DELTA_AUXV %ld\n", myTestCond->common_object[idx]);
	//shhw_230605e

	//common_cycle_branch
	
	//common_aux_condition
	convert_test_cond_common_aux((S_P1_TEST_COND_SAFETY *)&(
		myPs->testCond.safety), ch);

	//common_can_condition
	convert_test_cond_common_can((S_P1_TEST_COND_SAFETY *)&(
		myPs->testCond.safety), ch);

	//step_condition
	for(step=0; step < myPs->testCond.header.totalStep; step++) {
		//header
		idx = IDX_LOC_OBJ_CLIENT_STEP_NO;
		myTestCond->local_object[step][idx]
			= (long)myPs->testCond.step[step].header.stepNo;

		idx = IDX_LOC_OBJ_STEP_NO;
		myTestCond->local_object[step][idx]
			= (long)myPs->testCond.step[step].header.stepNo;

		idx = IDX_LOC_OBJ_TYPE;
		type = convert_step_type(CONVERT_P1_TO_ORG,
			(long)myPs->testCond.step[step].header.type);
		myTestCond->local_object[step][idx] = type;

		P1_stepCond = (S_P1_TEST_COND_STEP *)&(myPs->testCond.step[step]);

		switch(type) {
			case STEP_IDLE:
			case STEP_END:
				break;
			case STEP_CYCLE:
				convert_test_cond_cycle(P1_stepCond, ch, step, type,attr_count);
				break;
			case STEP_LOOP:
				convert_test_cond_loop(P1_stepCond, ch, step, type, attr_count);
				break;
			default:
				//jhkw_200517s
				if(myData->COA_Client[0].signal[P1_SIG_TEST_HEADER_UPDATE_RCV] == 1) {
					rtn = convert_test_cond_step_default(P1_stepCond, ch, step,
						type,attr_count, 1);
				} else {
					rtn = convert_test_cond_step_default(P1_stepCond, ch, step,
						type,attr_count, 0);
				}
				//rtn = convert_test_cond_step_default(P1_stepCond, ch, step,
				//	type,attr_count, 0);
				//jhkw_200517s
				if(rtn < 0) return rtn;
				break;
		}
	}

	return 0;
}

int convert_test_cond_time(int ch)
{ //kjh_160418
#if defined __COA_VER_100B__ || defined __COA_VER_100B2__ || defined __COA_VER_100C__ || defined __COA_VER_100D__
#else //COA_VER_100F~
	int step, idx, idx2;
	unsigned long tmp_day, tmp_time, tmp_hour, tmp_min, tmp_sec;

	memset((char *)&myData->TimeSch.testCond[ch], 0,
		sizeof(S_TIMESCH_TEST_CONDITION));

	//step_condition
	for(step=0; step < myPs->testCond.header.totalStep; step++) {
		idx = IDX_LOC_OBJ_TIMESCH_STEP_NO;
		myData->TimeSch.testCond[ch].local_object[step][idx]
			= (long)myPs->testCond_time.time_step[step].stepNo;

		if(myData->TimeSch.testCond[ch].local_object[step][idx] != step+1) {
			return -1;
		}

		idx = IDX_LOC_OBJ_TIMESCH_INIT;
		myData->TimeSch.testCond[ch].local_object[step][idx]
			= (long)myPs->testCond_time.time_step[step].timeInit;

		idx2 = IDX_LOC_OBJ_END_TIMESCH_DAY;
		tmp_day = (unsigned long)myPs->testCond_time.time_step[step].wait_day;
		myData->TimeSch.testCond[ch].local_object[step][idx2] = tmp_day;

		if(myData->TimeSch.testCond[ch].local_object[step][idx2] < 0) {
			return -2;
		}

		tmp_hour = (unsigned long)myPs->testCond_time.time_step[step].wait_hour;
		tmp_min = (unsigned long)myPs->testCond_time.time_step[step].wait_min;
		tmp_sec	= (unsigned long)myPs->testCond_time.time_step[step].wait_sec;

		tmp_time = tmp_hour * 360000;
		tmp_time += (tmp_min * 6000);
		tmp_time += (tmp_sec * 100);

		idx = IDX_LOC_OBJ_END_TIMESCH_TIME;
		myData->TimeSch.testCond[ch].local_object[step][idx] = tmp_time;

		if(myData->TimeSch.testCond[ch].local_object[step][idx] < 0) {
			return -3;
		}
	}
#endif
	return 0;
}

void convert_test_cond_common_aux(S_P1_TEST_COND_SAFETY *P1_safety, int ch)
{
	int idx, i, compare_type;
	long func_div, data_type, data_value;
	float tmp_f = 0.0; //jhkw_190714
	
	make_aux_chCode_cond((S_TEST_CONDITION *)myTestCond,
		C_CD_AUX_END_UPPER_TEMP, 3, COMP_GREATER_THAN_OR_EQUAL_TO); //>=
	make_aux_chCode_cond((S_TEST_CONDITION *)myTestCond,
		C_CD_AUX_END_LOWER_TEMP, 3, COMP_LESS_THAN_OR_EQUAL_TO); //<=
	make_aux_chCode_cond((S_TEST_CONDITION *)myTestCond,
		C_CD_AUX_END_UPPER_V, 3, COMP_GREATER_THAN_OR_EQUAL_TO); //>=
	make_aux_chCode_cond((S_TEST_CONDITION *)myTestCond,
		C_CD_AUX_END_LOWER_V, 3, COMP_LESS_THAN_OR_EQUAL_TO); //<=
	make_aux_chCode_cond((S_TEST_CONDITION *)myTestCond,
		C_CD_AUX_FAULT_UPPER_TEMP, 3, COMP_GREATER_THAN_OR_EQUAL_TO); //>=
	make_aux_chCode_cond((S_TEST_CONDITION *)myTestCond,
		C_CD_AUX_FAULT_LOWER_TEMP, 3, COMP_LESS_THAN_OR_EQUAL_TO); //<=
	make_aux_chCode_cond((S_TEST_CONDITION *)myTestCond,
		C_CD_AUX_FAULT_UPPER_V, 3, COMP_GREATER_THAN_OR_EQUAL_TO); //>=
	make_aux_chCode_cond((S_TEST_CONDITION *)myTestCond,
		C_CD_AUX_FAULT_LOWER_V, 3, COMP_LESS_THAN_OR_EQUAL_TO); //<=

	for(i=0; i < MAX_P1_AUX_FUNCTION; i++) {
		func_div = (long)P1_safety->aux_func_div[i];

		idx = IDX_COM_OBJ_AUX_FUNC_DIV_TMP_1 + i; //kjg_170810_s
		myTestCond->common_object[idx] = func_div;

		idx = IDX_COM_OBJ_AUX_COMP_TYPE_TMP_1 + i;
		myTestCond->common_object[idx] = (long)P1_safety->aux_compare_type[i];

		idx = IDX_COM_OBJ_AUX_VALUE_TMP_1 + i;
		//jhkw_190714s
		//myTestCond->common_object[idx]
		//	= (long)(P1_safety->aux_value[i] * 1000.0);
		tmp_f = P1_safety->aux_value[i] * 1000.0;
		myTestCond->common_object[idx] = (long)tmp_f;
		//jhkw_190714s

		data_type = (long)P1_safety->aux_data_type[i];
		idx = IDX_COM_OBJ_AUX_VALUE_TYPE_1 + i;
		myTestCond->common_object[idx] = data_type; //kjg_170810_e

		//if(func_div < AUX_BRANCH_FUNC_DIV_START
		//	|| func_div > AUX_BRANCH_FUNC_DIV_END) continue;
		if((func_div >= AUX_BRANCH_FUNC_DIV_START
			&& func_div <= AUX_BRANCH_FUNC_DIV_END)
			|| (func_div >= AUX_BRANCH_FUNC_DIV_START1
			&& func_div <= AUX_BRANCH_FUNC_DIV_END1)) {
		} else continue; //kjhw_170820

		/*kjg_170810 data_type = (long)P1_safety->aux_data_type[i];
		idx = IDX_COM_OBJ_AUX_VALUE_TYPE_1 + i;
		myTestCond->common_object[idx] = data_type;*/

		idx = IDX_COM_OBJ_AUX_VALUE_1 + i;
		if(data_type == 2) { //1:signed, 2:float
			//jhkw_190714s
			//data_value = (long)(P1_safety->aux_value[i] * 1000.0);
			tmp_f = P1_safety->aux_value[i] * 1000.0;
			data_value = (long)tmp_f;
			//jhkw_190714s
		} else {
			data_value = (long)P1_safety->aux_value[i];
		}

		if(func_div == AUX_FUNC_DIV_CV_SELECT) {
			switch(P1_safety->aux_compare_type[i]) {
				case P1_COMP_LESS_THAN: //<
					idx = IDX_COM_OBJ_AUX_CV_LOWER_VALUE;
					myTestCond->common_object[idx] = data_value;
					break;
				case P1_COMP_LESS_THAN_OR_EQUAL_TO: //<=
					idx = IDX_COM_OBJ_AUX_CV_LOWER_VALUE;
					myTestCond->common_object[idx] = data_value;
					break;
				case P1_COMP_EQUAL_TO: //=
					break;
				case P1_COMP_GREATER_THAN: //>
					idx = IDX_COM_OBJ_AUX_CV_UPPER_VALUE;
					myTestCond->common_object[idx] = data_value;
					break;
				case P1_COMP_GREATER_THAN_OR_EQUAL_TO: //>=
					idx = IDX_COM_OBJ_AUX_CV_UPPER_VALUE;
					myTestCond->common_object[idx] = data_value;
					break;
				case P1_COMP_NOT_EQUAL_TO: //!=
					break;
				default:
					break;
			}
			continue;
		}

		if(func_div == AUX_FUNC_DIV_TEMP_PAUSE) { //jhkw_160123s
			switch(P1_safety->aux_compare_type[i]) {
				case P1_COMP_LESS_THAN: //<
					idx = IDX_COM_OBJ_AUX_TEMP_P_LOWER_VALUE;
					myTestCond->common_object[idx] = data_value;
					break;
				case P1_COMP_LESS_THAN_OR_EQUAL_TO: //<=
					idx = IDX_COM_OBJ_AUX_TEMP_P_LOWER_VALUE;
					myTestCond->common_object[idx] = data_value;
					break;
				case P1_COMP_EQUAL_TO: //=
					break;
				case P1_COMP_GREATER_THAN: //>
					idx = IDX_COM_OBJ_AUX_TEMP_P_UPPER_VALUE;
					myTestCond->common_object[idx] = data_value;
					break;
				case P1_COMP_GREATER_THAN_OR_EQUAL_TO: //>=
					idx = IDX_COM_OBJ_AUX_TEMP_P_UPPER_VALUE;
					myTestCond->common_object[idx] = data_value;
					break;
				case P1_COMP_NOT_EQUAL_TO: //!=
					break;
				default:
					break;
			}
			continue;
		}

		if(func_div == AUX_FUNC_DIV_TEMP_CONTINUE) {
			switch(P1_safety->aux_compare_type[i]) {
				case P1_COMP_LESS_THAN: //<
					idx = IDX_COM_OBJ_AUX_TEMP_C_LOWER_VALUE;
					myTestCond->common_object[idx] = data_value;
					break;
				case P1_COMP_LESS_THAN_OR_EQUAL_TO: //<=
					idx = IDX_COM_OBJ_AUX_TEMP_C_LOWER_VALUE;
					myTestCond->common_object[idx] = data_value;
					break;
				case P1_COMP_EQUAL_TO: //=
					break;
				case P1_COMP_GREATER_THAN: //>
					idx = IDX_COM_OBJ_AUX_TEMP_C_UPPER_VALUE;
					myTestCond->common_object[idx] = data_value;
					break;
				case P1_COMP_GREATER_THAN_OR_EQUAL_TO: //>=
					idx = IDX_COM_OBJ_AUX_TEMP_C_UPPER_VALUE;
					myTestCond->common_object[idx] = data_value;
					break;
				case P1_COMP_NOT_EQUAL_TO: //!=
					break;
				default:
					break;
			}
			continue;
		} //jhkw_160123e

		idx = IDX_COM_OBJ_AUX_FUNC_DIV_1 + i;
		myTestCond->common_object[idx] = func_div;

		idx = IDX_COM_OBJ_AUX_VALUE_1 + i;
		myTestCond->common_object[idx] = data_value;

		switch(P1_safety->aux_compare_type[i]) {
			case P1_COMP_LESS_THAN:
				compare_type = COMP_LESS_THAN; //<
				break;
			case P1_COMP_LESS_THAN_OR_EQUAL_TO:
				compare_type = COMP_LESS_THAN_OR_EQUAL_TO; //<=
				break;
			case P1_COMP_EQUAL_TO:
				compare_type = COMP_EQUAL_TO; //=
				break;
			case P1_COMP_GREATER_THAN:
				compare_type = COMP_GREATER_THAN; //>
				break;
			case P1_COMP_GREATER_THAN_OR_EQUAL_TO:
				compare_type = COMP_GREATER_THAN_OR_EQUAL_TO; //>=
				break;
			case P1_COMP_NOT_EQUAL_TO:
				compare_type = COMP_NOT_EQUAL_TO; //!=
				break;
			default:
				compare_type = COMP_NONE;
				break;
		}

		if(compare_type != COMP_NONE) { //jhkw_130319s
			if(func_div == AUX_FUNC_DIV_CONCENT1_ON
				|| func_div == AUX_FUNC_DIV_CONCENT2_ON
				|| func_div == AUX_FUNC_DIV_CONCENT3_ON
				|| func_div == AUX_FUNC_DIV_CONCENT4_ON
				|| func_div == AUX_FUNC_DIV_CONCENT5_ON
				|| func_div == AUX_FUNC_DIV_CONCENT6_ON
				|| func_div == AUX_FUNC_DIV_CONCENT7_ON	//jhkw_180206
				|| func_div == AUX_FUNC_DIV_CONCENT8_ON	//jhkw_180206
				|| func_div == AUX_FUNC_DIV_CONCENT1_OFF
				|| func_div == AUX_FUNC_DIV_CONCENT2_OFF
				|| func_div == AUX_FUNC_DIV_CONCENT3_OFF
				|| func_div == AUX_FUNC_DIV_CONCENT4_OFF
				|| func_div == AUX_FUNC_DIV_CONCENT5_OFF
				|| func_div == AUX_FUNC_DIV_CONCENT6_OFF
				|| func_div == AUX_FUNC_DIV_CONCENT7_OFF //jhkw_180206
				|| func_div == AUX_FUNC_DIV_CONCENT8_OFF //jhkw_180206
				|| func_div == AUX_FUNC_DIV_CONCENT1_FORCE_ON
				|| func_div == AUX_FUNC_DIV_CONCENT2_FORCE_ON
				|| func_div == AUX_FUNC_DIV_CONCENT3_FORCE_ON
				|| func_div == AUX_FUNC_DIV_CONCENT4_FORCE_ON
				|| func_div == AUX_FUNC_DIV_CONCENT5_FORCE_ON
				|| func_div == AUX_FUNC_DIV_CONCENT6_FORCE_ON
				|| func_div == AUX_FUNC_DIV_CONCENT7_FORCE_ON	//jhkw_180206
				|| func_div == AUX_FUNC_DIV_CONCENT8_FORCE_ON	//jhkw_180206
				|| func_div == AUX_FUNC_DIV_CONCENT1_FORCE_OFF
				|| func_div == AUX_FUNC_DIV_CONCENT2_FORCE_OFF
				|| func_div == AUX_FUNC_DIV_CONCENT3_FORCE_OFF
				|| func_div == AUX_FUNC_DIV_CONCENT4_FORCE_OFF
				|| func_div == AUX_FUNC_DIV_CONCENT5_FORCE_OFF
				|| func_div == AUX_FUNC_DIV_CONCENT6_FORCE_OFF
				|| func_div == AUX_FUNC_DIV_CONCENT7_FORCE_OFF	//jhkw_180206
				|| func_div == AUX_FUNC_DIV_CONCENT8_FORCE_OFF	//jhkw_180206
				|| func_div == AUX_FUNC_DIV_MUX_A_FORCE_ON //kjhw_151021s
				|| func_div == AUX_FUNC_DIV_MUX_A_FORCE_OFF
				|| func_div == AUX_FUNC_DIV_MUX_B_FORCE_ON
				|| func_div == AUX_FUNC_DIV_MUX_B_FORCE_OFF) { //kjhw_151021e
				make_aux_chCode_cond((S_TEST_CONDITION *)myTestCond,
					C_CD_COM_AUX_FAULT_FUNC_DIV_1 + i, 100, compare_type);
			} else {
				make_aux_chCode_cond((S_TEST_CONDITION *)myTestCond,
					C_CD_COM_AUX_FAULT_FUNC_DIV_1 + i, 1, compare_type);
			}
		} //jhkw_130319e
	}
}

void convert_test_cond_common_can(S_P1_TEST_COND_SAFETY *P1_safety, int ch)
{
	int idx, i, compare_type;
	long func_div, data_type, data_value;
	float tmp_f = 0.0; //jhkw_190714
	
	make_can_chCode_cond((S_TEST_CONDITION *)myTestCond,
		C_CD_CAN_END_UPPER, 1, COMP_GREATER_THAN_OR_EQUAL_TO); //>=
	make_can_chCode_cond((S_TEST_CONDITION *)myTestCond,
		C_CD_CAN_END_LOWER, 1, COMP_LESS_THAN_OR_EQUAL_TO); //<=
	make_can_chCode_cond((S_TEST_CONDITION *)myTestCond,
		C_CD_CAN_FAULT_UPPER, 1, COMP_GREATER_THAN_OR_EQUAL_TO); //>=
	make_can_chCode_cond((S_TEST_CONDITION *)myTestCond,
		C_CD_CAN_FAULT_LOWER, 1, COMP_LESS_THAN_OR_EQUAL_TO); //<=
	
	for(i=0; i < MAX_P1_CAN_FUNCTION; i++) {
		func_div = (long)P1_safety->can_func_div[i];

		idx = IDX_COM_OBJ_CAN_FUNC_DIV_TMP_1 + i; //kjg_170810_s
		myTestCond->common_object[idx] = func_div;

		idx = IDX_COM_OBJ_CAN_COMP_TYPE_TMP_1 + i;
		myTestCond->common_object[idx] = (long)P1_safety->can_compare_type[i];

		idx = IDX_COM_OBJ_CAN_VALUE_TMP_1 + i;
		//jhkw_190714s
		//myTestCond->common_object[idx]
		//	= (long)(P1_safety->can_value[i] * 1000.0);
		tmp_f = P1_safety->can_value[i] * 1000.0;
		myTestCond->common_object[idx] = (long)tmp_f;
		//jhkw_190714e

		data_type = (long)P1_safety->can_data_type[i];
		idx = IDX_COM_OBJ_CAN_VALUE_TYPE_1 + i;
		myTestCond->common_object[idx] = data_type; //kjg_170810_e

		//if(func_div < CAN_RX_BRANCH_FUNC_DIV_START
		//	|| func_div > CAN_RX_BRANCH_FUNC_DIV_END) continue;
		if((func_div >= CAN_RX_BRANCH_FUNC_DIV_START
			&& func_div <= CAN_RX_BRANCH_FUNC_DIV_END)
			|| (func_div >= CAN_RX_FUNC_DIV_START
			&& func_div <= CAN_RX_FUNC_DIV_END)) {
		} else continue; //kjhw_170820

		/*kjg_170810 data_type = (long)P1_safety->can_data_type[i];
		idx = IDX_COM_OBJ_CAN_VALUE_TYPE_1 + i;
		myTestCond->common_object[idx] = (long)data_type;*/

		idx = IDX_COM_OBJ_CAN_VALUE_1 + i;
		if(data_type == 2) { //1:signed, 2:float
			//jhkw_190714s
			//data_value = (long)(P1_safety->can_value[i] * 1000.0);
			tmp_f = P1_safety->can_value[i] * 1000.0;
			data_value = (long)tmp_f;
			//jhkw_190714s
		} else {
			data_value = (long)P1_safety->can_value[i];
		}

		if(func_div == CAN_RX_FUNC_DIV_TEMP_PAUSE) { //jhkw_160123s
			switch(P1_safety->can_compare_type[i]) {
				case P1_COMP_LESS_THAN: //<
					idx = IDX_COM_OBJ_CAN_TEMP_P_LOWER_VALUE;
					myTestCond->common_object[idx] = data_value;
					break;
				case P1_COMP_LESS_THAN_OR_EQUAL_TO: //<=
					idx = IDX_COM_OBJ_CAN_TEMP_P_LOWER_VALUE;
					myTestCond->common_object[idx] = data_value;
					break;
				case P1_COMP_EQUAL_TO: //=
					break;
				case P1_COMP_GREATER_THAN: //>
					idx = IDX_COM_OBJ_CAN_TEMP_P_UPPER_VALUE;
					myTestCond->common_object[idx] = data_value;
					break;
				case P1_COMP_GREATER_THAN_OR_EQUAL_TO: //>=
					idx = IDX_COM_OBJ_CAN_TEMP_P_UPPER_VALUE;
					myTestCond->common_object[idx] = data_value;
					break;
				case P1_COMP_NOT_EQUAL_TO: //!=
					break;
				default:
					break;
			}
			continue;
		}

		if(func_div == CAN_RX_FUNC_DIV_TEMP_CONTINUE) {
			switch(P1_safety->can_compare_type[i]) {
				case P1_COMP_LESS_THAN: //<
					idx = IDX_COM_OBJ_CAN_TEMP_C_LOWER_VALUE;
					myTestCond->common_object[idx] = data_value;
					break;
				case P1_COMP_LESS_THAN_OR_EQUAL_TO: //<=
					idx = IDX_COM_OBJ_CAN_TEMP_C_LOWER_VALUE;
					myTestCond->common_object[idx] = data_value;
					break;
				case P1_COMP_EQUAL_TO: //=
					break;
				case P1_COMP_GREATER_THAN: //>
					idx = IDX_COM_OBJ_CAN_TEMP_C_UPPER_VALUE;
					myTestCond->common_object[idx] = data_value;
					break;
				case P1_COMP_GREATER_THAN_OR_EQUAL_TO: //>=
					idx = IDX_COM_OBJ_CAN_TEMP_C_UPPER_VALUE;
					myTestCond->common_object[idx] = data_value;
					break;
				case P1_COMP_NOT_EQUAL_TO: //!=
					break;
				default:
					break;
			}
			continue;
		} //jhkw_160123e

		if(func_div == CAN_RX_FUNC_DIV_CV_SELECT) {
			switch(P1_safety->can_compare_type[i]) {
				case P1_COMP_LESS_THAN: //<
					idx = IDX_COM_OBJ_CAN_CV_LOWER_VALUE;
					myTestCond->common_object[idx] = data_value;
					break;
				case P1_COMP_LESS_THAN_OR_EQUAL_TO: //<=
					idx = IDX_COM_OBJ_CAN_CV_LOWER_VALUE;
					myTestCond->common_object[idx] = data_value;
					break;
				case P1_COMP_EQUAL_TO: //=
					break;
				case P1_COMP_GREATER_THAN: //>
					idx = IDX_COM_OBJ_CAN_CV_UPPER_VALUE;
					myTestCond->common_object[idx] = data_value;
					break;
				case P1_COMP_GREATER_THAN_OR_EQUAL_TO: //>=
					idx = IDX_COM_OBJ_CAN_CV_UPPER_VALUE;
					myTestCond->common_object[idx] = data_value;
					break;
				case P1_COMP_NOT_EQUAL_TO: //!=
					break;
				default:
					break;
			}
			continue;
		}

		idx = IDX_COM_OBJ_CAN_FUNC_DIV_1 + i;
		myTestCond->common_object[idx] = func_div;

		idx = IDX_COM_OBJ_CAN_VALUE_1 + i;
		myTestCond->common_object[idx] = data_value;

		switch(P1_safety->can_compare_type[i]) {
			case P1_COMP_LESS_THAN:
				compare_type = COMP_LESS_THAN; //<
				break;
			case P1_COMP_LESS_THAN_OR_EQUAL_TO:
				compare_type = COMP_LESS_THAN_OR_EQUAL_TO; //<=
				break;
			case P1_COMP_EQUAL_TO:
				compare_type = COMP_EQUAL_TO; //=
				break;
			case P1_COMP_GREATER_THAN:
				compare_type = COMP_GREATER_THAN; //>
				break;
			case P1_COMP_GREATER_THAN_OR_EQUAL_TO:
				compare_type = COMP_GREATER_THAN_OR_EQUAL_TO; //>=
				break;
			case P1_COMP_NOT_EQUAL_TO:
				compare_type = COMP_NOT_EQUAL_TO; //!=
				break;
			default:
				compare_type = COMP_NONE;
				break;
		}

		if(compare_type != COMP_NONE) { //jhkw_130319s
			if(func_div == CAN_RX_FUNC_DIV_CONCENT1_ON
				|| func_div == CAN_RX_FUNC_DIV_CONCENT2_ON
				|| func_div == CAN_RX_FUNC_DIV_CONCENT3_ON
				|| func_div == CAN_RX_FUNC_DIV_CONCENT4_ON
				|| func_div == CAN_RX_FUNC_DIV_CONCENT5_ON
				|| func_div == CAN_RX_FUNC_DIV_CONCENT6_ON
				|| func_div == CAN_RX_FUNC_DIV_CONCENT7_ON	//jhkw_180206
				|| func_div == CAN_RX_FUNC_DIV_CONCENT8_ON	//jhkw_180206
				|| func_div == CAN_RX_FUNC_DIV_CONCENT1_OFF
				|| func_div == CAN_RX_FUNC_DIV_CONCENT2_OFF
				|| func_div == CAN_RX_FUNC_DIV_CONCENT3_OFF
				|| func_div == CAN_RX_FUNC_DIV_CONCENT4_OFF
				|| func_div == CAN_RX_FUNC_DIV_CONCENT5_OFF
				|| func_div == CAN_RX_FUNC_DIV_CONCENT6_OFF
				|| func_div == CAN_RX_FUNC_DIV_CONCENT7_OFF	//jhkw_180206
				|| func_div == CAN_RX_FUNC_DIV_CONCENT8_OFF	//jhkw_180206
				|| func_div == CAN_RX_FUNC_DIV_CONCENT1_FORCE_ON
				|| func_div == CAN_RX_FUNC_DIV_CONCENT2_FORCE_ON
				|| func_div == CAN_RX_FUNC_DIV_CONCENT3_FORCE_ON
				|| func_div == CAN_RX_FUNC_DIV_CONCENT4_FORCE_ON
				|| func_div == CAN_RX_FUNC_DIV_CONCENT5_FORCE_ON
				|| func_div == CAN_RX_FUNC_DIV_CONCENT6_FORCE_ON
				|| func_div == CAN_RX_FUNC_DIV_CONCENT7_FORCE_ON //jhkw_180206
				|| func_div == CAN_RX_FUNC_DIV_CONCENT8_FORCE_ON //jhkw_180206
				|| func_div == CAN_RX_FUNC_DIV_CONCENT1_FORCE_OFF
				|| func_div == CAN_RX_FUNC_DIV_CONCENT2_FORCE_OFF
				|| func_div == CAN_RX_FUNC_DIV_CONCENT3_FORCE_OFF
				|| func_div == CAN_RX_FUNC_DIV_CONCENT4_FORCE_OFF
				|| func_div == CAN_RX_FUNC_DIV_CONCENT5_FORCE_OFF
				|| func_div == CAN_RX_FUNC_DIV_CONCENT6_FORCE_OFF
				|| func_div == CAN_RX_FUNC_DIV_CONCENT7_FORCE_OFF //jhkw_180206
				|| func_div == CAN_RX_FUNC_DIV_CONCENT8_FORCE_OFF //jhkw_180206
				|| func_div == CAN_RX_FUNC_DIV_MUX_A_FORCE_ON //kjhw_151021s
				|| func_div == CAN_RX_FUNC_DIV_MUX_A_FORCE_OFF
				|| func_div == CAN_RX_FUNC_DIV_MUX_A_FORCE_ON
				|| func_div == CAN_RX_FUNC_DIV_MUX_A_FORCE_OFF //kjhw_151021e
				|| func_div == CAN_RX_FUNC_DIV_A_ON //jhkw_170119s
				|| func_div == CAN_RX_FUNC_DIV_A_OFF
				|| func_div == CAN_RX_FUNC_DIV_B_ON
				|| func_div == CAN_RX_FUNC_DIV_B_OFF //jhkw_170119e
				) {
				make_can_chCode_cond((S_TEST_CONDITION *)myTestCond,
					C_CD_COM_CAN_FAULT_FUNC_DIV_1 + i, 100, compare_type);
			} else {
				make_can_chCode_cond((S_TEST_CONDITION *)myTestCond,
					C_CD_COM_CAN_FAULT_FUNC_DIV_1 + i, 1, compare_type);
			}
		} //jhkw_130319e
	}
}

void convert_test_cond_cycle(S_P1_TEST_COND_STEP *P1_stepCond, int ch, int step, long type, long attr_count)
{
	int idx, idx2;
#ifdef __COA_VER_100B2__
	unsigned long tmp, tmp_day, tmp_time;
#endif
	//jhkw_180726s
	long l_val;

	idx = IDX_COM_OBJ_FAULT_UPPER_V;
	l_val = myTestCond->common_object[idx];

	if(l_val == 0) {
		l_val = RANGE1 - 1;
	} else if(l_val <= myData->mData.config.maxV[3]) {
		l_val = RANGE4 - 1;
	} else if(l_val <= myData->mData.config.maxV[2]) {
		l_val = RANGE3 - 1;
	} else if(l_val <= myData->mData.config.maxV[1]) {
		l_val = RANGE2 - 1;
	} else {
		l_val = RANGE1 - 1;
	}

	if((l_val+1) > (long)myData->mData.config.rangeV) {
		l_val = (long)myData->mData.config.rangeV - 1;
	}

	idx = IDX_LOC_OBJ_RANGE_V;
	myTestCond->local_object[step][idx] = l_val;
	//jhkw_180726e
	
	idx2 = 0;

	idx = IDX_LOC_OBJ_END_MULTI_CYCLE_COUNT_COMPARE;
	myTestCond->local_object[step][idx] = P1_stepCond->reference[0]
		.AmpareHour_Branch_MultiCycleCountId;

	idx = IDX_LOC_OBJ_END_SUM_AMPARE_HOUR;
	myTestCond->local_object[step][idx] = P1_stepCond->reference[0]
		.AmpareHour_SumAmpareHour;
	if(myTestCond->local_object[step][idx] >= 0) {
		make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
			C_CD_END_SUM_AMPARE_HOUR, 1,
			COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=
	} else {
		make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
			C_CD_END_SUM_AMPARE_HOUR, 1, COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=
	}

	idx = IDX_LOC_OBJ_END_SUM_AMPARE_HOUR_TYPE;
	myTestCond->local_object[step][idx] = (long)P1_stepCond->reference[0]
		.rangeI; //0:all, 1:charge, 2:discharge

	idx = IDX_LOC_OBJ_END_SUM_AMPARE_HOUR_BRANCH;
	myTestCond->local_object[step][idx] = ACTIVE_DIV_CURRENT_LOOP;

	idx = IDX_LOC_OBJ_END_SUM_WATT_HOUR;
	myTestCond->local_object[step][idx] = P1_stepCond-> reference[0]
		.WattHour_SumWattHour;
	if(myTestCond->local_object[step][idx] >= 0) {
		make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
			C_CD_END_SUM_WATT_HOUR, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=
	} else {
		make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
			C_CD_END_SUM_WATT_HOUR, 1, COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=
	}

	idx = IDX_LOC_OBJ_END_SUM_WATT_HOUR_TYPE;
	myTestCond->local_object[step][idx] = (long)P1_stepCond->reference[0]
		.ValueRate_Compare; //0:all,1:charge,2:discharge

	idx = IDX_LOC_OBJ_END_SUM_WATT_HOUR_BRANCH;
	myTestCond->local_object[step][idx] = ACTIVE_DIV_CURRENT_LOOP;

#ifdef __COA_VER_100B__
	idx = IDX_LOC_OBJ_END_SUM_TIME;
	myTestCond->local_object[step][idx] = P1_stepCond->reference[0].Time;
	make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
		C_CD_END_TIME, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=
#elif __COA_VER_100B2__
	tmp = P1_stepCond->reference[0].Time;
	tmp_day = tmp / (360000 * 24);
	tmp_time = tmp % (360000 * 24);
	idx2 = IDX_LOC_OBJ_END_SUM_TIME_DAY;
	myTestCond->local_object[step][idx2] = tmp_day;
	idx = IDX_LOC_OBJ_END_SUM_TIME;
	myTestCond->local_object[step][idx] = tmp_time;
	make_loc_chCode_cond2((S_TEST_CONDITION *)myTestCond, step,
		C_CD_END_SUM_TIME, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx, idx2); //>=
#else //COA_VER_100C~
	idx2 = IDX_LOC_OBJ_END_SUM_TIME_DAY;
	myTestCond->local_object[step][idx2] = P1_stepCond->reference[0]
		.endTime_day;
	idx = IDX_LOC_OBJ_END_SUM_TIME;
	myTestCond->local_object[step][idx] = P1_stepCond->reference[0].endTime;
	make_loc_chCode_cond2((S_TEST_CONDITION *)myTestCond, step,
		C_CD_END_SUM_TIME, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx, idx2); //>=
#endif

	idx = IDX_LOC_OBJ_END_SUM_TIME_BRANCH;
	myTestCond->local_object[step][idx] = ACTIVE_DIV_CURRENT_LOOP;
}

void convert_test_cond_loop(S_P1_TEST_COND_STEP *P1_stepCond, int ch, int step, long type, long attr_count)
{
	int idx;
	//jhkw_180726s
	long l_val;

	idx = IDX_COM_OBJ_FAULT_UPPER_V;
	l_val = myTestCond->common_object[idx];

	if(l_val == 0) {
		l_val = RANGE1 - 1;
	} else if(l_val <= myData->mData.config.maxV[3]) {
		l_val = RANGE4 - 1;
	} else if(l_val <= myData->mData.config.maxV[2]) {
		l_val = RANGE3 - 1;
	} else if(l_val <= myData->mData.config.maxV[1]) {
		l_val = RANGE2 - 1;
	} else {
		l_val = RANGE1 - 1;
	}

	if((l_val+1) > (long)myData->mData.config.rangeV) {
		l_val = (long)myData->mData.config.rangeV - 1;
	}

	idx = IDX_LOC_OBJ_RANGE_V;
	myTestCond->local_object[step][idx] = l_val;
	//jhkw_180726e

	idx = IDX_LOC_OBJ_END_CYCLE_COUNT;
	myTestCond->local_object[step][idx] = P1_stepCond->reference[0]
		.V_Lower_Branch_CycleCount;

	idx = IDX_LOC_OBJ_END_CYCLE_COUNT_BRANCH;
	myTestCond->local_object[step][idx] = (long)P1_stepCond->reference[0]
		.V_Upper_Branch_CycleCount_Branch;
	if(myTestCond->local_object[step][idx] == ACTIVE_DIV_NONE)
		myTestCond->local_object[step][idx] = ACTIVE_DIV_NEXT;

	idx = IDX_LOC_OBJ_END_MULTI_CYCLE_COUNT;
	myTestCond->local_object[step][idx] = P1_stepCond->reference[0]
		.V_Lower_MultiCycleCount;

	idx = IDX_LOC_OBJ_END_MULTI_CYCLE_COUNT_BRANCH;
	myTestCond->local_object[step][idx] = P1_stepCond->reference[0]
		.WattHour_Branch_MultiCycleCount_Branch;
	if(myTestCond->local_object[step][idx] == ACTIVE_DIV_NONE)
		myTestCond->local_object[step][idx] = ACTIVE_DIV_NEXT;

	idx = IDX_LOC_OBJ_END_MULTI_CYCLE_COUNT_COMPARE;
	myTestCond->local_object[step][idx] = P1_stepCond->reference[0]
		.AmpareHour_Branch_MultiCycleCountId;

	idx = IDX_LOC_OBJ_END_ACC_CYCLE_COUNT;
	myTestCond->local_object[step][idx] = P1_stepCond->reference[0]
		.Time_Branch_AccCycleCount;

	idx = IDX_LOC_OBJ_END_ACC_CYCLE_COUNT_BRANCH;
	myTestCond->local_object[step][idx] = P1_stepCond->reference[0]
		.AccCycleCount_Branch;
	if(myTestCond->local_object[step][idx] == ACTIVE_DIV_NONE)
		myTestCond->local_object[step][idx] = ACTIVE_DIV_NEXT;

	idx = IDX_LOC_OBJ_END_ACC_CYCLE_COUNT_COMPARE;
	myTestCond->local_object[step][idx] = P1_stepCond->reference[0]
		.CVTime_Branch_AccCycleCountId;

	idx = IDX_LOC_OBJ_CYCLE_PAUSE;

	myTestCond->local_object[step][idx] = (long)P1_stepCond->header.cycle_pause;
	//0:none, 1:pause(element_cycle count)
}

int convert_test_cond_step_default(S_P1_TEST_COND_STEP *P1_stepCond, int ch, int step, long type, long attr_count, int div)
{
	int rtn, idx, idx2, i, j, grade, fault_count, use_flag;
	int slave_idx, slave_ch;		//csk_190712
	long crate;	//jhkw_200524
#ifdef __COA_VER_100B2__
	unsigned long tmp, tmp_day, tmp_time;
#endif

	idx2 = 0;

	//end_condition
	if(type == STEP_OCV) {
		make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
			C_CD_END_OCV, 1, COMP_NONE, 0);
	}

#ifdef __COA_VER_100B__
	idx = IDX_LOC_OBJ_END_TIME;
	myTestCond->local_object[step][idx] = P1_stepCond->reference[0].Time;
	make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
		C_CD_END_TIME, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=
#elif __COA_VER_100B2__
	tmp = P1_stepCond->reference[0].Time;
	tmp_day = tmp / (360000 * 24);
	tmp_time = tmp % (360000 * 24);
	idx2 = IDX_LOC_OBJ_END_TIME_DAY;
	myTestCond->local_object[step][idx2] = tmp_day;
	idx = IDX_LOC_OBJ_END_TIME;
	myTestCond->local_object[step][idx] = tmp_time;
	make_loc_chCode_cond2((S_TEST_CONDITION *)myTestCond, step,
		C_CD_END_TIME, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx, idx2); //>=
#else //COA_VER_100C~
	idx2 = IDX_LOC_OBJ_END_TIME_DAY;
	myTestCond->local_object[step][idx2] = P1_stepCond->reference[0]
		.endTime_day;
	idx = IDX_LOC_OBJ_END_TIME;
	myTestCond->local_object[step][idx] = P1_stepCond->reference[0].endTime;
	make_loc_chCode_cond2((S_TEST_CONDITION *)myTestCond, step,
		C_CD_END_TIME, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx, idx2); //>=
#endif

	idx = IDX_LOC_OBJ_END_TIME_BRANCH;
	myTestCond->local_object[step][idx] = P1_stepCond->reference[0]
		.Time_Branch_AccCycleCount;
	if(myTestCond->local_object[step][idx] == ACTIVE_DIV_NONE)
		myTestCond->local_object[step][idx] = ACTIVE_DIV_NEXT;
	
#ifdef __COA_VER_100B__
	idx = IDX_LOC_OBJ_END_CV_TIME;
	myTestCond->local_object[step][idx] = P1_stepCond->reference[0].CVTime;
	make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
		C_CD_END_CV_TIME, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=
#elif __COA_VER_100B2__
	tmp = P1_stepCond->reference[0].CVTime;
	tmp_day = tmp / (360000 * 24);
	tmp_time = tmp % (360000 * 24);
	idx2 = IDX_LOC_OBJ_END_CV_TIME_DAY;
	myTestCond->local_object[step][idx2] = tmp_day;
	idx = IDX_LOC_OBJ_END_CV_TIME;
	myTestCond->local_object[step][idx] = tmp_time;
	make_loc_chCode_cond2((S_TEST_CONDITION *)myTestCond, step,
		C_CD_END_CV_TIME, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx, idx2); //>=
#else //COA_VER_100C~
	idx2 = IDX_LOC_OBJ_END_CV_TIME_DAY;
	myTestCond->local_object[step][idx2] = P1_stepCond->reference[0].CVTime_day;
	idx = IDX_LOC_OBJ_END_CV_TIME;
	myTestCond->local_object[step][idx] = P1_stepCond->reference[0].CVTime;
	make_loc_chCode_cond2((S_TEST_CONDITION *)myTestCond, step,
		C_CD_END_CV_TIME, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx, idx2); //>=
#endif

	idx = IDX_LOC_OBJ_END_CV_TIME_BRANCH;
	myTestCond->local_object[step][idx] = P1_stepCond->reference[0]
		.CVTime_Branch_AccCycleCountId;
	if(myTestCond->local_object[step][idx] == ACTIVE_DIV_NONE)
		myTestCond->local_object[step][idx] = ACTIVE_DIV_NEXT;

//20181219 KHK------------------------------------------------------------
	idx = IDX_LOC_OBJ_END_RPT_SOC;
	myTestCond->local_object[step][idx] = P1_stepCond->reference[0].rptSOC; //10Base
	if(myTestCond->local_object[step][idx] != 0) {
		userlog(DEBUG_LOG, psName,"rptSOC %d\n", myTestCond->local_object[step][idx]);
	}
//	idx = IDX_LOC_OBJ_LIMIT_CURRENT_LOWER;
//	myTestCond->local_object[step][idx] = P1_stepCond->reference[0].limitCurrent_Lower * 1000;
//	userlog(DEBUG_LOG, psName,"limit Current_discharge %ld\n", myTestCond->local_object[step][idx]);

//	idx = IDX_LOC_OBJ_LIMIT_CURRENT_UPPER;
//	myTestCond->local_object[step][idx] = P1_stepCond->reference[0].limitCurrent_Upper * 1000;
//	userlog(DEBUG_LOG, psName,"limit Current_charge %ld\n", myTestCond->local_object[step][idx]);

	idx = IDX_LOC_OBJ_SOC_TRACKING_FLAG;
	myTestCond->local_object[step][idx] = P1_stepCond->reference[0]
		.SOC_Tracking_flag;
	if(myTestCond->local_object[step][idx] != 0) {
		userlog(DEBUG_LOG, psName,"SOC Tracking Flag %ld\n", myTestCond->local_object[step][idx]);
	}
		
	//jhkw_201102s
	idx = IDX_LOC_OBJ_DISCHARGE_SOC_TRACKING_FLAG;
	myTestCond->local_object[step][idx] = P1_stepCond->reference[0]
		.DisCharge_SOC_Tracking_flag;
	if(myTestCond->local_object[step][idx] != 0) {
		userlog(DEBUG_LOG, psName,"DisCharge SOC Tracking Flag %ld\n"
			, myTestCond->local_object[step][idx]);
	}
	//jhkw_201102e
	//jhkw_221205s
	idx = IDX_LOC_OBJ_SEQUENCE_CHARGE_FLAG;
	myTestCond->local_object[step][idx] = P1_stepCond->reference[0]
		.sequence_charge_flag;
	if(myTestCond->local_object[step][idx] != 0) {
		userlog(DEBUG_LOG, psName,"Sequence Charge Flag %ld\n", myTestCond->local_object[step][idx]);
	}
	//jhkw_221205e
	
	idx = IDX_LOC_OBJ_CELL_BALANCING_FLAG;
	myTestCond->local_object[step][idx] = P1_stepCond->reference[0]
		.Cell_Balancing_flag;
	if(myTestCond->local_object[step][idx] != 0) {
		userlog(DEBUG_LOG, psName,"Cell Balancing Flag %ld\n"
			, myTestCond->local_object[step][idx]);
	}
//-----------------------------------------------------------------------	


	idx = IDX_LOC_OBJ_END_V_UPPER;
	myTestCond->local_object[step][idx] = P1_stepCond->reference[0].V_Upper;
	make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
		C_CD_END_V_UPPER, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_LOC_OBJ_END_V_UPPER_BRANCH;
	myTestCond->local_object[step][idx] = P1_stepCond->reference[0]
		.V_Upper_Branch_CycleCount_Branch;
	if(myTestCond->local_object[step][idx] == ACTIVE_DIV_NONE)
		myTestCond->local_object[step][idx] = ACTIVE_DIV_NEXT;

	idx = IDX_LOC_OBJ_END_V_LOWER;
	myTestCond->local_object[step][idx] = P1_stepCond->reference[0]
		.V_Lower_MultiCycleCount;
	make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
		C_CD_END_V_LOWER, 1, COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=

	idx = IDX_LOC_OBJ_END_V_LOWER_BRANCH;
	myTestCond->local_object[step][idx] = P1_stepCond->reference[0]
		.V_Lower_Branch_CycleCount;
	if(myTestCond->local_object[step][idx] == ACTIVE_DIV_NONE)
		myTestCond->local_object[step][idx] = ACTIVE_DIV_NEXT;

#if defined __COA_VER_100B__ || defined __COA_VER_100B2__ || defined __COA_VER_100C__
#else //COA_VER_100D~
	idx = IDX_LOC_OBJ_FAULT_CAN_CHECK_FLAG; //kjhw_141208
	myTestCond->local_object[step][idx] = P1_stepCond->fault_can_check_flag;

	idx = IDX_LOC_OBJ_FAULT_CAN_TX_STOP; //kjhw_141208
	myTestCond->local_object[step][idx] = P1_stepCond->can_tx_stop;

	idx = IDX_LOC_OBJ_CHAMBER_STEP_CHECK_FLAG; //kjhw_150821
	myTestCond->local_object[step][idx] = P1_stepCond->reference[0]
		.chamber_step_check_flag;
	if(myData->ChAttribute[ch].opType != OP_INDEPENDENT) {	//csk_190712s
		for(slave_idx = 0; slave_idx < 3; slave_idx++) {
			slave_ch = (int)myData->ChAttribute[ch].chNo_slave[slave_idx] - 1;
			if(slave_ch >= 0) {
				myData->testCond[slave_ch].local_object[step][idx]
					= myTestCond->local_object[step][idx];
			}
		}
	}														//csk_190712e

	idx = IDX_LOC_OBJ_CAN_COMM_STEP_CHECK_FLAG; //kjhw_150821
	myTestCond->local_object[step][idx] = P1_stepCond->can_comm_step_check_flag;
#endif

	convert_test_cond_step_aux(P1_stepCond, ch, step);

	convert_test_cond_step_can(P1_stepCond, ch, step);

	//record
	idx = IDX_LOC_OBJ_SAVE_DELTA_T;
	myTestCond->local_object[step][idx] = (long)P1_stepCond->record.time;
		//= 1; //kjg_d 10ms katech
	make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
		C_CD_SAVE_DELTA_T, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_LOC_OBJ_SAVE_DELTA_V;
	myTestCond->local_object[step][idx] = P1_stepCond->record.deltaV;
	make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
		C_CD_SAVE_DELTA_V, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_LOC_OBJ_SAVE_DELTA_I;
	myTestCond->local_object[step][idx] = P1_stepCond->record.deltaI;
	make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
		C_CD_SAVE_DELTA_I, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_LOC_OBJ_SAVE_DELTA_TEMP;
	myTestCond->local_object[step][idx] = P1_stepCond->record.deltaT;
	make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
		C_CD_SAVE_DELTA_TEMP, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_LOC_OBJ_SAVE_DELTA_P;
	myTestCond->local_object[step][idx] = P1_stepCond->record.deltaP;
	make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
		C_CD_SAVE_DELTA_P, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	//edlc -> record kjg_101221
	switch(myData->AppControl.config.systemModel) {
		case C_SDI_70V_50A_5A_4KW:
		case C_SDI_70V_50A_5A_4KW_2:
		case C_SDI_70V_50A_5A_7KW:
		case C_SDI_70V_50A_5A_7KW_2:
		case C_SDI_70V_250A_25A_18KW:
			use_flag = 1;
			break;
		default:
			use_flag = 0;
			break;
	}

	if(use_flag == 1) {
		idx = IDX_LOC_OBJ_SAVE_RECORD_T1;
		myTestCond->local_object[step][idx] = (long)P1_stepCond->edlc.startT_Z;

		idx = IDX_LOC_OBJ_SAVE_RECORD_T2;
		myTestCond->local_object[step][idx] = (long)P1_stepCond->edlc.endT_Z;

		idx = IDX_LOC_OBJ_SAVE_RECORD_T3;
		myTestCond->local_object[step][idx] = (long)P1_stepCond->edlc.startT_LC;
	}

	//fault
	switch(myData->AppControl.config.systemModel) {
		//for motor inverter application
		case C_DAEWOO_450V_200A_10A_90KW:
		case C_DAEWOO_450V_200A_10A_90KW_2:
		case C_DAEWOO_450V_200A_50A_90KW:
		case C_DAEWOO_450V_200A_50A_90KW_2:	//jhk_150710
			fault_count = 50; //500ms delay
			break;
		default:
			fault_count = 1;
			break;
	}

	idx = IDX_LOC_OBJ_FAULT_UPPER_V;
	myTestCond->local_object[step][idx] = P1_stepCond->faultUpperV;
	make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
		C_CD_FAULT_UPPER_V, fault_count,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_LOC_OBJ_FAULT_LOWER_V;
	myTestCond->local_object[step][idx] = P1_stepCond->faultLowerV;
	make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
		C_CD_FAULT_LOWER_V, fault_count, COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=

/*	idx = IDX_LOC_OBJ_FAULT_UPPER_TEMP;
	myTestCond->local_object[step][idx] = P1_stepCond->faultUpperTemp;
	make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
		C_CD_FAULT_UPPER_TEMP, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_LOC_OBJ_FAULT_LOWER_TEMP;
	myTestCond->local_object[step][idx] = P1_stepCond->faultLowerTemp;
	make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
		C_CD_FAULT_LOWER_TEMP, 1, COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=
		*/
	//jhkw_200524s
	idx = IDX_LOC_OBJ_LIMIT_CURRENT_UPPER_FLAG;
	myTestCond->local_object[step][idx] = (long)P1_stepCond->crate_flag;
	//userlog(DEBUG_LOG, psName,"limit Current_charge_flag %d\n", 
	//	myTestCond->local_object[step][idx]);
	if(myTestCond->local_object[step][idx] == 1) {
		crate = (long)Calculate_Crate_Value(1, ch);
		crate = (crate / 1000) * (P1_stepCond->faultUpperTemp / 1000);	//shh_check
		userlog(DEBUG_LOG, psName,"limit Current_crate %ld\n", crate);
	
		if(crate < 0) {
			crate = 2000000; //shh_check
		}
		idx = IDX_LOC_OBJ_LIMIT_CURRENT_UPPER;
		myTestCond->local_object[step][idx] = crate;
		//userlog(DEBUG_LOG, psName,"limit Current_charge %ld\n", 
		//	myTestCond->local_object[step][idx]);
	} else {
		idx = IDX_LOC_OBJ_LIMIT_CURRENT_UPPER;
		myTestCond->local_object[step][idx] = P1_stepCond->faultUpperTemp * 1000;
		//userlog(DEBUG_LOG, psName,"limit Current_charge %ld\n", 
		//	myTestCond->local_object[step][idx]);

	}
	//idx = IDX_LOC_OBJ_LIMIT_CURRENT_UPPER;
	//myTestCond->local_object[step][idx] = P1_stepCond->faultUpperTemp * 1000;
	//userlog(DEBUG_LOG, psName,"limit Current_charge %ld\n", 
	//	myTestCond->local_object[step][idx]);
	//jhkw_200524e
	
	idx = IDX_LOC_OBJ_LIMIT_CURRENT_LOWER;
	myTestCond->local_object[step][idx] = P1_stepCond->faultLowerTemp * 1000;
	//userlog(DEBUG_LOG, psName,"limit Current_discharge %ld\n", myTestCond->local_object[step][idx]);

	//step grade
	for(grade=0; grade < MAX_P1_GRADE_ITEM; grade++) {
		i = (int)P1_stepCond->grade[grade].item;
		if(i != P1_GRADE_ITEM_IDLE) {
			myTestCond->grade[step].item = (unsigned char)i;

			j = (int)P1_stepCond->grade[grade].gradeStepCount;
			if(j > MAX_TEST_GRADE) j = MAX_TEST_GRADE;
			myTestCond->grade[step].totalGrade = (unsigned char)j;

			for(i=0; i < j; i++) {
				myTestCond->grade[step].code[i]
					= (long)P1_stepCond->grade[grade].gradeStep[i].gradeCode;

				myTestCond->grade[step].value1[i] = P1_stepCond->grade[grade]
					.gradeStep[i].lowerValue;

				myTestCond->grade[step].value2[i] = P1_stepCond->grade[grade]
					.gradeStep[i].upperValue;
			}
			break;
		}
	}

	switch(type) {
		case STEP_OCV:
		case STEP_REST:
			//kjhw_121031 Vref x 2
			convert_test_cond_ocv_rest(P1_stepCond, ch, step, type, attr_count);
			break;
		case STEP_CHARGE:
		case STEP_DISCHARGE:
		case STEP_Z:
			rtn = convert_test_cond_charge_discharge_z(P1_stepCond, ch, step, type,
				attr_count);
			if(rtn < 0) return rtn;
			break;
		case STEP_PATTERN:
			rtn = convert_test_cond_pattern(P1_stepCond, ch, step, type,
				attr_count, div);
			if(rtn < 0) return rtn;
			break;
		case STEP_EXTERNAL_CAN:
			convert_test_cond_external_can(P1_stepCond, ch, step, type,
				attr_count);
			break;
#if defined __COA_VER_100B__ || defined __COA_VER_100B2__ || defined __COA_VER_100C__
#else //COA_VER_100D~
		case STEP_USERMAP: //kjhw_140828
			rtn = convert_test_cond_usermap(P1_stepCond, ch, step, type,
				attr_count, div);
			if(rtn < 0) return rtn;
			break;
#endif
		default: break;
	}

	return 0;
}

void convert_test_cond_step_aux(S_P1_TEST_COND_STEP *P1_stepCond, int ch, int step)
{
	int idx, i, compare_type, code, branch;
	int delay_time; //kjhw_171117
	long func_div, data_type, data_value;
	float tmp_f = 0.0; //jhkw_190714
	//long delay_time; //kjhw_170828
	
	for(i=0; i < MAX_P1_AUX_FUNCTION; i++) {
		func_div = (long)P1_stepCond->reference[0].aux_func_div[i];

		idx = IDX_LOC_OBJ_AUX_FUNC_DIV_TMP_1 + i; //kjg_170810_s
		myTestCond->local_object[step][idx] = func_div;

		idx = IDX_LOC_OBJ_AUX_COMP_TYPE_TMP_1 + i;
		myTestCond->local_object[step][idx]
			= (long)P1_stepCond->reference[0].aux_compare_type[i];

		idx = IDX_LOC_OBJ_AUX_VALUE_TMP_1 + i;
		//jhkw_190714s
		//myTestCond->local_object[step][idx]
		//	= (long)(P1_stepCond->reference[0].aux_value[i] * 1000.0);
		tmp_f = P1_stepCond->reference[0].aux_value[i] * 1000.0;
		myTestCond->local_object[step][idx] = (long)tmp_f;
		//jhkw_190714e

		data_type = (long)P1_stepCond->reference[0].aux_data_type[i];
		idx = IDX_LOC_OBJ_AUX_VALUE_TYPE_1 + i;
		myTestCond->local_object[step][idx] = data_type;
	
		idx = IDX_LOC_OBJ_AUX_BRANCH_TMP_1 + i;
		myTestCond->local_object[step][idx]
			= (long)P1_stepCond->reference[0].aux_branch[i]; //kjg_170810_e

		//if(func_div < AUX_BRANCH_FUNC_DIV_START
		//	|| func_div > AUX_BRANCH_FUNC_DIV_END) continue;
		if((func_div >= AUX_BRANCH_FUNC_DIV_START
			&& func_div <= AUX_BRANCH_FUNC_DIV_END)
			|| (func_div >= AUX_BRANCH_FUNC_DIV_START1
			&& func_div <= AUX_BRANCH_FUNC_DIV_END1)) {
		} else continue; //kjhw_170820
		
		/*kjg_170810 data_type = (long)P1_stepCond->reference[0].aux_data_type[i];
		idx = IDX_LOC_OBJ_AUX_VALUE_TYPE_1 + i;
		myTestCond->local_object[step][idx] = data_type;*/
	
		if(data_type == 2) { //1:signed, 2:float
			//jhkw_190714s
			//data_value = (long)(P1_stepCond->reference[0].aux_value[i]
			//	* 1000.0);
			tmp_f = P1_stepCond->reference[0].aux_value[i] * 1000.0;
			data_value = (long)tmp_f;
			//jhkw_190714e
		} else {
			data_value = (long)P1_stepCond->reference[0].aux_value[i];
		}
		//kjhw_170828s
		//delay_time = (long)P1_stepCond->reference[0].aux_delay_time[i]; //kjh_171117
		delay_time = (int)P1_stepCond->reference[0].aux_delay_time[i];
		delay_time *= 100;
		if(delay_time > 1000) delay_time = 1000;
		else if(delay_time <= 0) delay_time = 1;

		idx = IDX_LOC_OBJ_AUX_DELAY_TIME_1 + i;
		//myTestCond->local_object[step][idx] = delay_time;  //kjh_171117
		myTestCond->local_object[step][idx] = (long)delay_time;
		//kjhw_170828s
		
		if(func_div == AUX_FUNC_DIV_CV_SELECT) {
			switch(P1_stepCond->reference[0].aux_compare_type[i]) {
				case P1_COMP_LESS_THAN: //<
					idx = IDX_LOC_OBJ_AUX_CV_LOWER_VALUE;
					myTestCond->local_object[step][idx] = data_value;
					break;
				case P1_COMP_LESS_THAN_OR_EQUAL_TO: //<=
					idx = IDX_LOC_OBJ_AUX_CV_LOWER_VALUE;
					myTestCond->local_object[step][idx] = data_value;
					break;
				case P1_COMP_EQUAL_TO: //=
					break;
				case P1_COMP_GREATER_THAN: //>
					idx = IDX_LOC_OBJ_AUX_CV_UPPER_VALUE;
					myTestCond->local_object[step][idx] = data_value;
					break;
				case P1_COMP_GREATER_THAN_OR_EQUAL_TO: //>=
					idx = IDX_LOC_OBJ_AUX_CV_UPPER_VALUE;
					myTestCond->local_object[step][idx] = data_value;
					break;
				case P1_COMP_NOT_EQUAL_TO: //!=
					break;
				default:
					break;
			}
			continue;
		}

		if(func_div == AUX_FUNC_DIV_TEMP_PAUSE) { //jhkw_160123s
			switch(P1_stepCond->reference[0].aux_compare_type[i]) {
				case P1_COMP_LESS_THAN: //<
					idx = IDX_LOC_OBJ_AUX_TEMP_P_LOWER_VALUE;
					myTestCond->local_object[step][idx] = data_value;
					break;
				case P1_COMP_LESS_THAN_OR_EQUAL_TO: //<=
					idx = IDX_LOC_OBJ_AUX_TEMP_P_LOWER_VALUE;
					myTestCond->local_object[step][idx] = data_value;
					break;
				case P1_COMP_EQUAL_TO: //=
					break;
				case P1_COMP_GREATER_THAN: //>
					idx = IDX_LOC_OBJ_AUX_TEMP_P_UPPER_VALUE;
					myTestCond->local_object[step][idx] = data_value;
					break;
				case P1_COMP_GREATER_THAN_OR_EQUAL_TO: //>=
					idx = IDX_LOC_OBJ_AUX_TEMP_P_UPPER_VALUE;
					myTestCond->local_object[step][idx] = data_value;
					break;
				case P1_COMP_NOT_EQUAL_TO: //!=
					break;
				default:
					break;
			}
			continue;
		}

		if(func_div == AUX_FUNC_DIV_TEMP_CONTINUE) {
			switch(P1_stepCond->reference[0].aux_compare_type[i]) {
				case P1_COMP_LESS_THAN: //<
					idx = IDX_LOC_OBJ_AUX_TEMP_C_LOWER_VALUE;
					myTestCond->local_object[step][idx] = data_value;
					break;
				case P1_COMP_LESS_THAN_OR_EQUAL_TO: //<=
					idx = IDX_LOC_OBJ_AUX_TEMP_C_LOWER_VALUE;
					myTestCond->local_object[step][idx] = data_value;
					break;
				case P1_COMP_EQUAL_TO: //=
					break;
				case P1_COMP_GREATER_THAN: //>
					idx = IDX_LOC_OBJ_AUX_TEMP_C_UPPER_VALUE;
					myTestCond->local_object[step][idx] = data_value;
					break;
				case P1_COMP_GREATER_THAN_OR_EQUAL_TO: //>=
					idx = IDX_LOC_OBJ_AUX_TEMP_C_UPPER_VALUE;
					myTestCond->local_object[step][idx] = data_value;
					break;
				case P1_COMP_NOT_EQUAL_TO: //!=
					break;
				default:
					break;
			}
			continue;
		} //jhkw_160123e

		idx = IDX_LOC_OBJ_AUX_FUNC_DIV_1 + i;
		myTestCond->local_object[step][idx] = func_div;

		idx = IDX_LOC_OBJ_AUX_BRANCH_1 + i;
		myTestCond->local_object[step][idx] = (long)P1_stepCond->reference[0]
			.aux_branch[i];
		branch = (int)P1_stepCond->reference[0].aux_branch[i];
		if(branch == ACTIVE_DIV_PAUSE || branch == ACTIVE_DIV_STOP
			|| branch == ACTIVE_DIV_FAULT) {
			code = C_CD_AUX_FAULT_FUNC_DIV_1 + i;
		} else {
			code = C_CD_AUX_END_FUNC_DIV_1 + i;
		}

		idx = IDX_LOC_OBJ_AUX_VALUE_1 + i;
		myTestCond->local_object[step][idx] = data_value;

		switch(P1_stepCond->reference[0].aux_compare_type[i]) {
			case P1_COMP_LESS_THAN:
				compare_type = COMP_LESS_THAN; //<
				break;
			case P1_COMP_LESS_THAN_OR_EQUAL_TO:
				compare_type = COMP_LESS_THAN_OR_EQUAL_TO; //<=
				break;
			case P1_COMP_EQUAL_TO:
				compare_type = COMP_EQUAL_TO; //=
				break;
			case P1_COMP_GREATER_THAN:
				compare_type = COMP_GREATER_THAN; //>
				break;
			case P1_COMP_GREATER_THAN_OR_EQUAL_TO:
				compare_type = COMP_GREATER_THAN_OR_EQUAL_TO; //>=
				break;
			case P1_COMP_NOT_EQUAL_TO:
				compare_type = COMP_NOT_EQUAL_TO; //!=
				break;
			default:
				compare_type = COMP_NONE;
				break;
		}

		if(compare_type != COMP_NONE) { //jhkw_130319s
			if(func_div == AUX_FUNC_DIV_CONCENT1_ON
				|| func_div == AUX_FUNC_DIV_CONCENT2_ON
				|| func_div == AUX_FUNC_DIV_CONCENT3_ON
				|| func_div == AUX_FUNC_DIV_CONCENT4_ON
				|| func_div == AUX_FUNC_DIV_CONCENT5_ON
				|| func_div == AUX_FUNC_DIV_CONCENT6_ON
				|| func_div == AUX_FUNC_DIV_CONCENT7_ON	//jhkw_180206
				|| func_div == AUX_FUNC_DIV_CONCENT8_ON	//jhkw_180206
				|| func_div == AUX_FUNC_DIV_CONCENT1_OFF
				|| func_div == AUX_FUNC_DIV_CONCENT2_OFF
				|| func_div == AUX_FUNC_DIV_CONCENT3_OFF
				|| func_div == AUX_FUNC_DIV_CONCENT4_OFF
				|| func_div == AUX_FUNC_DIV_CONCENT5_OFF
				|| func_div == AUX_FUNC_DIV_CONCENT6_OFF
				|| func_div == AUX_FUNC_DIV_CONCENT7_OFF	//jhkw_180206
				|| func_div == AUX_FUNC_DIV_CONCENT8_OFF	//jhkw_180206
				|| func_div == AUX_FUNC_DIV_CONCENT1_FORCE_ON
				|| func_div == AUX_FUNC_DIV_CONCENT2_FORCE_ON
				|| func_div == AUX_FUNC_DIV_CONCENT3_FORCE_ON
				|| func_div == AUX_FUNC_DIV_CONCENT4_FORCE_ON
				|| func_div == AUX_FUNC_DIV_CONCENT5_FORCE_ON
				|| func_div == AUX_FUNC_DIV_CONCENT6_FORCE_ON
				|| func_div == AUX_FUNC_DIV_CONCENT7_FORCE_ON	//jhkw_180206
				|| func_div == AUX_FUNC_DIV_CONCENT8_FORCE_ON	//jhkw_180206
				|| func_div == AUX_FUNC_DIV_CONCENT1_FORCE_OFF
				|| func_div == AUX_FUNC_DIV_CONCENT2_FORCE_OFF
				|| func_div == AUX_FUNC_DIV_CONCENT3_FORCE_OFF
				|| func_div == AUX_FUNC_DIV_CONCENT4_FORCE_OFF
				|| func_div == AUX_FUNC_DIV_CONCENT5_FORCE_OFF
				|| func_div == AUX_FUNC_DIV_CONCENT6_FORCE_OFF
				|| func_div == AUX_FUNC_DIV_CONCENT7_FORCE_OFF	//jhkw_180206
				|| func_div == AUX_FUNC_DIV_CONCENT8_FORCE_OFF	//jhkw_180206
				|| func_div == AUX_FUNC_DIV_MUX_A_FORCE_ON //kjhw_151021s
				|| func_div == AUX_FUNC_DIV_MUX_A_FORCE_OFF
				|| func_div == AUX_FUNC_DIV_MUX_B_FORCE_ON
				|| func_div == AUX_FUNC_DIV_MUX_B_FORCE_OFF) { //kjhw_151021e
				make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
					code, 100, compare_type, idx);
			} else {
				//make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
				//	code, 1, compare_type, idx);
				make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
					code, delay_time, compare_type, idx); //kjhw_170828
			}
		} //jhkw_130319e
	}
}

void convert_test_cond_step_can(S_P1_TEST_COND_STEP *P1_stepCond, int ch, int step)
{
	int idx, i, compare_type, code, branch;
	long func_div, data_type, data_value;
	float tmp_f = 0.0; //jhkw_190714

	for(i=0; i < MAX_P1_CAN_FUNCTION; i++) {
		func_div = (long)P1_stepCond->reference[0].can_func_div[i];

		idx = IDX_LOC_OBJ_CAN_FUNC_DIV_TMP_1 + i; //kjg_170810_s
		myTestCond->local_object[step][idx] = func_div;

		idx = IDX_LOC_OBJ_CAN_COMP_TYPE_TMP_1 + i;
		myTestCond->local_object[step][idx]
			= (long)P1_stepCond->reference[0].can_compare_type[i];

		idx = IDX_LOC_OBJ_CAN_VALUE_TMP_1 + i;
		//jhkw_190714s
		//myTestCond->local_object[step][idx]
		//	= (long)(P1_stepCond->reference[0].can_value[i] * 1000.0);
		tmp_f = P1_stepCond->reference[0].can_value[i] * 1000.0;
		myTestCond->local_object[step][idx] = (long)tmp_f; //kjh_190709
		//jhkw_190714e

		data_type = (long)P1_stepCond->reference[0].can_data_type[i];
		idx = IDX_LOC_OBJ_CAN_VALUE_TYPE_1 + i;
		myTestCond->local_object[step][idx] = data_type;
	
		idx = IDX_LOC_OBJ_CAN_BRANCH_TMP_1 + i;
		myTestCond->local_object[step][idx]
			= (long)P1_stepCond->reference[0].can_branch[i]; //kjg_170810_e

		if((func_div >= CAN_RX_BRANCH_FUNC_DIV_START
			&& func_div <= CAN_RX_BRANCH_FUNC_DIV_END)
			|| (func_div >= CAN_RX_FUNC_DIV_START
			&& func_div <= CAN_RX_FUNC_DIV_END)) { //kjhw_170303
		} else continue;
		
		/*kjg_170810 data_type = (long)P1_stepCond->reference[0].can_data_type[i];
		idx = IDX_LOC_OBJ_CAN_VALUE_TYPE_1 + i;
		myTestCond->local_object[step][idx] = data_type;*/
		
		if(data_type == 2) { //1:signed, 2:float
			//jhkw_190714s
			//data_value = (long)(P1_stepCond->reference[0].can_value[i]
			//	* 1000.0);
			tmp_f = P1_stepCond->reference[0].can_value[i] * 1000.0;
			data_value = (long)tmp_f;
			//jhkw_190714e
		} else {
			data_value = (long)P1_stepCond->reference[0].can_value[i];
		}

		if(func_div == CAN_RX_FUNC_DIV_TEMP_PAUSE) { //jhkw_160123s
			switch(P1_stepCond->reference[0].can_compare_type[i]) {
				case P1_COMP_LESS_THAN: //<
					idx = IDX_LOC_OBJ_CAN_TEMP_P_LOWER_VALUE;
					myTestCond->local_object[step][idx] = data_value;
					break;
				case P1_COMP_LESS_THAN_OR_EQUAL_TO: //<=
					idx = IDX_LOC_OBJ_CAN_TEMP_P_LOWER_VALUE;
					myTestCond->local_object[step][idx] = data_value;
					break;
				case P1_COMP_EQUAL_TO: //=
					break;
				case P1_COMP_GREATER_THAN: //>
					idx = IDX_LOC_OBJ_CAN_TEMP_P_UPPER_VALUE;
					myTestCond->local_object[step][idx] = data_value;
					break;
				case P1_COMP_GREATER_THAN_OR_EQUAL_TO: //>=
					idx = IDX_LOC_OBJ_CAN_TEMP_P_UPPER_VALUE;
					myTestCond->local_object[step][idx] = data_value;
					break;
				case P1_COMP_NOT_EQUAL_TO: //!=
					break;
				default:
					break;
			}
			continue;
		}

		if(func_div == CAN_RX_FUNC_DIV_TEMP_CONTINUE) {
			switch(P1_stepCond->reference[0].can_compare_type[i]) {
				case P1_COMP_LESS_THAN: //<
					idx = IDX_LOC_OBJ_CAN_TEMP_C_LOWER_VALUE;
					myTestCond->local_object[step][idx] = data_value;
					break;
				case P1_COMP_LESS_THAN_OR_EQUAL_TO: //<=
					idx = IDX_LOC_OBJ_CAN_TEMP_C_LOWER_VALUE;
					myTestCond->local_object[step][idx] = data_value;
					break;
				case P1_COMP_EQUAL_TO: //=
					break;
				case P1_COMP_GREATER_THAN: //>
					idx = IDX_LOC_OBJ_CAN_TEMP_C_UPPER_VALUE;
					myTestCond->local_object[step][idx] = data_value;
					break;
				case P1_COMP_GREATER_THAN_OR_EQUAL_TO: //>=
					idx = IDX_LOC_OBJ_CAN_TEMP_C_UPPER_VALUE;
					myTestCond->local_object[step][idx] = data_value;
					break;
				case P1_COMP_NOT_EQUAL_TO: //!=
					break;
				default:
					break;
			}
			continue;
		} //jhkw_160123e

		if(func_div == CAN_RX_FUNC_DIV_CV_SELECT) {
			switch(P1_stepCond->reference[0].can_compare_type[i]) {
				case P1_COMP_LESS_THAN: //<
					idx = IDX_LOC_OBJ_CAN_CV_LOWER_VALUE;
					myTestCond->local_object[step][idx] = data_value;
					break;
				case P1_COMP_LESS_THAN_OR_EQUAL_TO: //<=
					idx = IDX_LOC_OBJ_CAN_CV_LOWER_VALUE;
					myTestCond->local_object[step][idx] = data_value;
					break;
				case P1_COMP_EQUAL_TO: //=
					break;
				case P1_COMP_GREATER_THAN: //>
					idx = IDX_LOC_OBJ_CAN_CV_UPPER_VALUE;
					myTestCond->local_object[step][idx] = data_value;
					break;
				case P1_COMP_GREATER_THAN_OR_EQUAL_TO: //>=
					idx = IDX_LOC_OBJ_CAN_CV_UPPER_VALUE;
					myTestCond->local_object[step][idx] = data_value;
					break;
				case P1_COMP_NOT_EQUAL_TO: //!=
					break;
				default:
					break;
			}
			continue;
		}
		
		idx = IDX_LOC_OBJ_CAN_FUNC_DIV_1 + i;
		myTestCond->local_object[step][idx] = func_div;
				
		idx = IDX_LOC_OBJ_CAN_BRANCH_1 + i;
		myTestCond->local_object[step][idx]
			= (long)P1_stepCond->reference[0].can_branch[i];
		branch = (int)P1_stepCond->reference[0].can_branch[i];
		if(branch == ACTIVE_DIV_PAUSE || branch == ACTIVE_DIV_STOP
			|| branch == ACTIVE_DIV_FAULT) {
			code = C_CD_CAN_FAULT_FUNC_DIV_1 + i;
		} else {
			code = C_CD_CAN_END_FUNC_DIV_1 + i;
		}

		idx = IDX_LOC_OBJ_CAN_VALUE_1 + i;
		myTestCond->local_object[step][idx] = data_value;
		
		switch(P1_stepCond->reference[0].can_compare_type[i]) {
			case P1_COMP_LESS_THAN:
				compare_type = COMP_LESS_THAN; //<
				break;
			case P1_COMP_LESS_THAN_OR_EQUAL_TO:
				compare_type = COMP_LESS_THAN_OR_EQUAL_TO; //<=
				break;
			case P1_COMP_EQUAL_TO:
				compare_type = COMP_EQUAL_TO; //=
				break;
			case P1_COMP_GREATER_THAN:
				compare_type = COMP_GREATER_THAN; //>
				break;
			case P1_COMP_GREATER_THAN_OR_EQUAL_TO:
				compare_type = COMP_GREATER_THAN_OR_EQUAL_TO; //>=
				break;
			case P1_COMP_NOT_EQUAL_TO:
				compare_type = COMP_NOT_EQUAL_TO; //!=
				break;
			default:
				compare_type = COMP_NONE;
				break;
		}

		if(compare_type != COMP_NONE) { //jhkw_130319s
			if(func_div == CAN_RX_FUNC_DIV_CHARGING_POWER
				|| func_div == CAN_RX_FUNC_DIV_AVAILABLE_POWER
				|| func_div == CAN_RX_FUNC_DIV_GENERATION_CURRENT //kjhw_150914
				|| func_div == CAN_RX_FUNC_DIV_AVAILABLE_CURRENT //kjhw_150914
				|| func_div == CAN_RX_FUNC_DIV_CONCENT1_ON
				|| func_div == CAN_RX_FUNC_DIV_CONCENT2_ON
				|| func_div == CAN_RX_FUNC_DIV_CONCENT3_ON
				|| func_div == CAN_RX_FUNC_DIV_CONCENT4_ON
				|| func_div == CAN_RX_FUNC_DIV_CONCENT5_ON
				|| func_div == CAN_RX_FUNC_DIV_CONCENT6_ON
				|| func_div == CAN_RX_FUNC_DIV_CONCENT7_ON	//jhkw_180206
				|| func_div == CAN_RX_FUNC_DIV_CONCENT8_ON	//jhkw_180206
				|| func_div == CAN_RX_FUNC_DIV_CONCENT1_OFF
				|| func_div == CAN_RX_FUNC_DIV_CONCENT2_OFF
				|| func_div == CAN_RX_FUNC_DIV_CONCENT3_OFF
				|| func_div == CAN_RX_FUNC_DIV_CONCENT4_OFF
				|| func_div == CAN_RX_FUNC_DIV_CONCENT5_OFF
				|| func_div == CAN_RX_FUNC_DIV_CONCENT6_OFF
				|| func_div == CAN_RX_FUNC_DIV_CONCENT7_OFF	//jhkw_180206
				|| func_div == CAN_RX_FUNC_DIV_CONCENT8_OFF	//jhkw_180206
				|| func_div == CAN_RX_FUNC_DIV_CONCENT1_FORCE_ON
				|| func_div == CAN_RX_FUNC_DIV_CONCENT2_FORCE_ON
				|| func_div == CAN_RX_FUNC_DIV_CONCENT3_FORCE_ON
				|| func_div == CAN_RX_FUNC_DIV_CONCENT4_FORCE_ON
				|| func_div == CAN_RX_FUNC_DIV_CONCENT5_FORCE_ON
				|| func_div == CAN_RX_FUNC_DIV_CONCENT6_FORCE_ON
				|| func_div == CAN_RX_FUNC_DIV_CONCENT7_FORCE_ON //jhkw_180206
				|| func_div == CAN_RX_FUNC_DIV_CONCENT8_FORCE_ON //jhkw_180206
				|| func_div == CAN_RX_FUNC_DIV_CONCENT1_FORCE_OFF
				|| func_div == CAN_RX_FUNC_DIV_CONCENT2_FORCE_OFF
				|| func_div == CAN_RX_FUNC_DIV_CONCENT3_FORCE_OFF
				|| func_div == CAN_RX_FUNC_DIV_CONCENT4_FORCE_OFF
				|| func_div == CAN_RX_FUNC_DIV_CONCENT5_FORCE_OFF
				|| func_div == CAN_RX_FUNC_DIV_CONCENT6_FORCE_OFF
				|| func_div == CAN_RX_FUNC_DIV_CONCENT7_FORCE_OFF //jhkw_180206
				|| func_div == CAN_RX_FUNC_DIV_CONCENT8_FORCE_OFF //jhkw_180206
				|| func_div == CAN_RX_FUNC_DIV_MUX_A_FORCE_ON //kjhw_151021s
				|| func_div == CAN_RX_FUNC_DIV_MUX_A_FORCE_OFF
				|| func_div == CAN_RX_FUNC_DIV_MUX_B_FORCE_ON
				|| func_div == CAN_RX_FUNC_DIV_MUX_B_FORCE_OFF //kjhw_151021e
				|| func_div == CAN_RX_FUNC_DIV_A_ON //jhkw_170119s
				|| func_div == CAN_RX_FUNC_DIV_A_OFF
				|| func_div == CAN_RX_FUNC_DIV_B_ON
				|| func_div == CAN_RX_FUNC_DIV_B_OFF //jhkw_170119e
				) {
				make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
					code, 100, compare_type, idx);
			} else {
				make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
					code, 1, compare_type, idx);
			}
		} //jhkw_130319e
	}
}

void convert_test_cond_ocv_rest(S_P1_TEST_COND_STEP *P1_stepCond, int ch, int step, long type, long attr_count)
{ //kjhw_120517 Vref x 2
	int idx;
	long l_val;

	idx = IDX_COM_OBJ_FAULT_UPPER_V;
	l_val = myTestCond->common_object[idx];

	if(l_val == 0) {
		l_val = RANGE1 - 1;
	} else if(l_val <= myData->mData.config.maxV[3]) {
		l_val = RANGE4 - 1;
	} else if(l_val <= myData->mData.config.maxV[2]) {
		l_val = RANGE3 - 1;
	} else if(l_val <= myData->mData.config.maxV[1]) {
		l_val = RANGE2 - 1;
	} else {
		l_val = RANGE1 - 1;
	}

	if((l_val+1) > (long)myData->mData.config.rangeV) {
		l_val = (long)myData->mData.config.rangeV - 1;
	}

	idx = IDX_LOC_OBJ_RANGE_V;
	myTestCond->local_object[step][idx] = l_val;

//20181219 KHK----------------------------------------------------------
	idx = IDX_LOC_OBJ_END_DELTA_V;
	myTestCond->local_object[step][idx] = P1_stepCond->reference[0].DeltaV;
	make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
		C_CD_END_DELTA_V, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	if(myTestCond->local_object[step][idx] != 0) {
		userlog(DEBUG_LOG, psName,"DeltaV %d\n", myTestCond->local_object[step][idx]);
	}
		
	idx = IDX_LOC_OBJ_CELL_BALANCING_START;
	myTestCond->local_object[step][idx] = P1_stepCond->reference[0].cellBalancingStart;
	//userlog(DEBUG_LOG, psName,"Cell Balancing Start %xd\n"
	//		, myTestCond->local_object[step][idx]);

	idx = IDX_LOC_OBJ_CELL_BALANCING_STOP;
	myTestCond->local_object[step][idx] = P1_stepCond->reference[0].cellBalancingStart;
	//userlog(DEBUG_LOG, psName,"Cell Balancing STOP %xd\n"
	//		, myTestCond->local_object[step][idx]);

	idx = IDX_LOC_OBJ_CELL_BALANCING_END;
	myTestCond->local_object[step][idx] = P1_stepCond->reference[0].cellBalancingEnd;
	//userlog(DEBUG_LOG, psName,"Cell Balancing End %xd\n"
	//		, myTestCond->local_object[step][idx]);

//----------------------------------------------------------------------	
}

int convert_test_cond_charge_discharge_z(S_P1_TEST_COND_STEP *P1_stepCond, int ch, int step, long type, long attr_count)
{

#if defined __COA_VER_100B__ || defined __COA_VER_100B2__ || defined __COA_VER_100C__
#else //COA_VER_100D~
	int i, func_div;
#endif
	int idx, idx2, fault_count, rtn = 0, pattern_index;
	long mode, refV1, refV2, refI, l_val, l_val2, tmp, refP, refR;
	//unsigned long dv_time, di_time;	//jhkw_180823
	double d_val1, d_val2;
	long maxI, minI;        //phb_230302
	long ratioV, ratioI; //jhkw_231127
	double ratioP; //jhkw_231127

	ratioV = myData->mData.ratioV;	//jhkw_231127s
	ratioI = myData->mData.ratioI;	
	ratioP = myData->mData.ratioP;	//jhkw_231127e

	//dv_time = di_time = 0;	//jhkw_180823
	refP = refR = 0;
	maxI = myData->mData.config.maxI[0] * attr_count;       //phb_230302
    if(maxI < 0) maxI = MAX_SUM_ULONG;
    minI = myData->mData.config.minI[0] * attr_count;
    if(minI > 0) minI = MAX_SUM_ULONG * (-1);

	//header
	idx = IDX_LOC_OBJ_MODE;
	mode = convert_step_mode(CONVERT_P1_TO_ORG, (long)P1_stepCond->header.mode);
	myTestCond->local_object[step][idx] = mode;
	userlog(DEBUG_LOG, psName, "ch%d step Mode %d\n", ch+1, mode);

	//reference
	idx = IDX_LOC_OBJ_REF_V; //charge ref_v
	refV1 = P1_stepCond->reference[0].refV_upper;
	if(refV1 > myData->mData.config.maxV[0]) {
		refV1 = myData->mData.config.maxV[0];
	} else if(refV1 < myData->mData.config.minV[0]) {
		refV1 = myData->mData.config.minV[0];
	}
	myTestCond->local_object[step][idx] = refV1;

	idx = IDX_COM_OBJ_FAULT_UPPER_V; //kjhw_120517s
	l_val = myTestCond->common_object[idx];

	if(l_val == 0) {
		l_val = RANGE1 - 1;
	} else if(l_val <= myData->mData.config.maxV[3]) {
		l_val = RANGE4 - 1;
	} else if(l_val <= myData->mData.config.maxV[2]) {
		l_val = RANGE3 - 1;
	} else if(l_val <= myData->mData.config.maxV[1]) {
		l_val = RANGE2 - 1;
	} else {
		l_val = RANGE1 - 1;
	}

	if((l_val+1) > (long)myData->mData.config.rangeV) {
		l_val = (long)myData->mData.config.rangeV - 1;
	}

	idx = IDX_LOC_OBJ_RANGE_V;
	myTestCond->local_object[step][idx] = l_val; //kjhw_120517e

	idx = IDX_LOC_OBJ_REF_V2; //discharge ref_v
	refV2 = P1_stepCond->reference[0].refV_lower;
	if(refV2 > myData->mData.config.maxV[0]) {
		refV2 = myData->mData.config.maxV[0];
	} else if(refV2 < myData->mData.config.minV[0]) {
		refV2 = myData->mData.config.minV[0];
	}
	myTestCond->local_object[step][idx] = refV2;

	idx = IDX_COM_OBJ_FAULT_UPPER_V; //kjhw_120517s
	l_val = myTestCond->common_object[idx];

	if(l_val == 0) {
		l_val = RANGE1 - 1;
	} else if(l_val <= myData->mData.config.maxV[3]) {
		l_val = RANGE4 - 1;
	} else if(l_val <= myData->mData.config.maxV[2]) {
		l_val = RANGE3 - 1;
	} else if(l_val <= myData->mData.config.maxV[1]) {
		l_val = RANGE2 - 1;
	} else {
		l_val = RANGE1 - 1;
	}

	if((l_val+1) > (long)myData->mData.config.rangeV) {
		l_val = (long)myData->mData.config.rangeV - 1;
	}

	idx = IDX_LOC_OBJ_RANGE_V2;
	myTestCond->local_object[step][idx] = l_val; //kjhw_120517e

	idx = IDX_LOC_OBJ_PATTERN_UPDATED; //jhkw_200517s
	if(myData->COA_Client[0].signal[P1_SIG_TEST_HEADER_UPDATE_RCV] == 1) {
		myData->testCond_update.local_object[step][idx] = P1;
		myData->testCond[ch].local_object[step][idx] = P1;			//jhkw_200816
	} else {
		myData->testCond_update.local_object[step][idx] = P0;
		myData->testCond[ch].local_object[step][idx] = P0;			//jhkw_200816
	} //jhkw_200517e
//20181219 KHK------------------------------------------------------
	if(type == STEP_CHARGE) {
		if(myTestCond->local_object[step][IDX_LOC_OBJ_SOC_TRACKING_FLAG] == P1){
			idx = IDX_LOC_OBJ_PATTERN_INDEX;
			pattern_index = (long)step;
			myTestCond->local_object[step][idx] = pattern_index;
			//jhkw_201102s
			//rtn = Read_SOC_Tracking_File(psName, ch, (int)pattern_index);
			rtn = Read_SOC_Tracking_File(psName, ch, (int)pattern_index, 0);
			//jhkw_201102e
		}
		if(rtn < 0) {
			userlog(DEBUG_LOG, psName, "ch%d SOC file error %d\n", ch+1, rtn);
			return rtn;
		}
		if(mode == MODE_USER) {
			myTestCond->local_object[step][IDX_LOC_OBJ_USER_DEFINE_MODE_FLAG] = P1;
		}
	}
//------------------------------------------------------------------

	if(type == STEP_CHARGE) {
		//jhkw_221205s
		if(myTestCond->local_object[step][IDX_LOC_OBJ_SEQUENCE_CHARGE_FLAG] == P1){
			idx = IDX_LOC_OBJ_PATTERN_INDEX;
			pattern_index = (long)step;
			myTestCond->local_object[step][idx] = pattern_index;
			rtn = Read_Sequence_Charge_File(psName, ch, (int)pattern_index);
		}
		if(rtn < 0) {
			userlog(DEBUG_LOG, psName, "ch%d Sequence Charge file error %d\n", ch+1, rtn);
			return rtn;
		}
		//jhkw_221205e
		if(mode == MODE_CP) {
			refP = P1_stepCond->reference[0].refP;
			if(refP < 0) refP = 0;
			//refI = myData->mData.config.maxI[0] * attr_count;
			refI = maxI; //phb_230302
			//d_val1 = (double)(refV1 / 1000) * (double)(refI / 1000) / 1000.0;
			//jhkw_231127
			d_val1 = (double)(refV1 / ratioV) * (double)(refI / ratioI) / ratioP;
			if(refP > (long)d_val1) refP = (long)d_val1;
			idx = IDX_LOC_OBJ_REF_P;
			myTestCond->local_object[step][idx] = refP;
		} else if(mode == MODE_CP_CC) {
			refP = P1_stepCond->reference[0].refP;
			if(refP < 0) refP = 0;
			refI = P1_stepCond->reference[0].refI;
			//if(refI > (myData->mData.config.minI[0] * (-1) * attr_count)) {
			//	refI = myData->mData.config.minI[0] * (-1) * attr_count;
			if(refI > maxI) {   // phb_230302    
                refI = maxI;
			} else if(refI < 0) refI = 0;
			//d_val1 = (double)(refV1 / 1000) * (double)(refI / 1000) / 1000.0;
			//jhkw_231127
			d_val1 = (double)(refV1 / ratioV) * (double)(refI / ratioI) / ratioP;
			if(refP <= (long)d_val1) d_val1 = (long)refP;
			idx = IDX_LOC_OBJ_REF_P;
			myTestCond->local_object[step][idx] = (long)d_val1;
			d_val2 = d_val1 / (double)refV1;
			//d_val2 *= 1000000000.0;
			d_val2 *= myData->mData.cp_to_cc;	//jhkw_231127
			if(d_val2 > (double)refI) d_val2 = (double)refI;
			refI = (long)d_val2; 
		} else if(mode == MODE_CR) {
			refR = P1_stepCond->reference[0].refR;
			if(refR < 1) refR = 1; //0.001ohm
			else if(refR > 1000000000) refR = 1000000000; //1Mohm
			idx = IDX_LOC_OBJ_REF_R;
			myTestCond->local_object[step][idx] = refR;
			d_val1 = (double)refV1 / (double)refR * 1000.0;
			//d_val2 = (double)(myData->mData.config.maxI[0] * attr_count);
			d_val2 = (double)maxI;
			if(d_val1 > d_val2) d_val1 = d_val2;
			refI = (long)d_val1;
		} else {
//20181219 KHK------------------------------------------				
			if(myTestCond->local_object[step][IDX_LOC_OBJ_SOC_TRACKING_FLAG] == P1){
				refI = myTestCond->SOC_tracking[0].maxI; //jhkw_201102
			}else if(myTestCond->local_object[step][IDX_LOC_OBJ_SEQUENCE_CHARGE_FLAG] == P1){
				refI = myTestCond->SQ_Charge.maxI;
			}else{
				refI = P1_stepCond->reference[0].refI;
			}
//------------------------------------------------------			
		}
	} else { //STEP_DISCHARGE, STEP_Z
		if(mode == MODE_CP) {
			refP = P1_stepCond->reference[0].refP;
			//refI = myData->mData.config.minI[0] * (-1) * attr_count;
			refI = minI * (-1);	//phb_230302
			//d_val1 = (double)(myData->mData.config.maxV[0] / 1000)
			//	* (double)(refI / 1000) / 1000.0;
			d_val1 = (double)(myData->mData.config.maxV[0] / ratioV)
				* (double)(refI / ratioI) / ratioP;	//jhkw_231127
			if(refP > (long)d_val1) refP = (long)d_val1;
			idx = IDX_LOC_OBJ_REF_P;
			myTestCond->local_object[step][idx] = refP;
		} else if(mode == MODE_CP_CC) {
			refP = P1_stepCond->reference[0].refP;
			refI = P1_stepCond->reference[0].refI;
			//if(refI > (myData->mData.config.minI[0] * (-1) * attr_count)) {
			//	refI = myData->mData.config.minI[0] * (-1) * attr_count;
			if(refI > (minI * (-1))) refI = (minI * (-1));	//phb_230302
			else if(refI < 0) refI = 0;
			//d_val1 = (double)(myData->mData.config.maxV[0] / 1000)
			//	* (double)(refI / 1000) / 1000.0;
			d_val1 = (double)(myData->mData.config.maxV[0] / ratioV)
				* (double)(refI / ratioI) / ratioP;	//jhkw_231127
			if(refP > (long)d_val1) refP = (long)d_val1;
			idx = IDX_LOC_OBJ_REF_P;
			myTestCond->local_object[step][idx] = refP;
			//jhkw_231127s
			/*if(refV2 <= 1000) { //1mV
				d_val2 = (double)refP / 1000.0; //1mV
			} else {
				d_val2 = (double)refP / (double)refV2;
			}
			d_val2 *= 1000000000.0;*/
			if(myData->mData.config.ratioV == MICRO_UNIT) { //uV //kjh_211021s
				if(refV2 <= 1000) { //1mV
					d_val2 = (double)refP / 1000.0; //1mV
				} else {
					d_val2 = (double)refP / (double)refV2;
				}
			} else if(myData->mData.config.ratioV == MILLI_UNIT) { //mV
				if(refV2 <= 1) { //1mV
					d_val2 = (double)refP / 1.0; //1mV
				} else {
					d_val2 = (double)refP / (double)refV2;
				}
			} else {
				if(refV2 <= 1000) { //1mV
					d_val2 = (double)refP / 1000.0; //1mV
				} else {
					d_val2 = (double)refP / (double)refV2;
				}
			}
			d_val2 *= myData->mData.cp_to_cc;	
			//jhkw_231127e

			if(d_val2 > (double)refI) d_val2 = (double)refI;
			refI = (long)d_val2; 
		} else if(mode == MODE_CR) {
			refR = P1_stepCond->reference[0].refR;
			if(refR < 1) refR = 1; //0.001ohm
			else if(refR > 1000000000) refR = 1000000000; //1Mohm
			idx = IDX_LOC_OBJ_REF_R;
			myTestCond->local_object[step][idx] = refR;
			d_val1 = (double)myData->mData.config.maxV[0]
				/ (double)refR * 1000.0;
			//d_val2 = (double)(myData->mData.config.minI[0] * (-1) * attr_count);
			d_val2 = (double)(minI * (-1));	//phb_230302
			if(d_val1 > d_val2) d_val1 = d_val2;
			refI = (long)d_val1;
		} else {
			refI = P1_stepCond->reference[0].refI;
			//if(refI > (myData->mData.config.minI[0] * (-1) * attr_count)) {
			//	refI = myData->mData.config.minI[0] * (-1) * attr_count;
			if(refI > (minI * (-1))) {  //phb_230302
                refI = minI * (-1);
			} else if(refI < 0) refI = 0;
		}
		refI *= (-1);
	}
	if(refI > maxI) {  
        if(myData->mData.config.maxI[0] != 0) {
            refI = maxI;
        }
    } else if(refI < minI) {
        if(myData->mData.config.minI[0] != 0) {
            refI = minI;
        }
    }	//phb_230302

	/*idx = IDX_LOC_OBJ_REF_I;
	if(refI > (myData->mData.config.maxI[0] * attr_count)) {
		if(myData->mData.config.maxI[0] != 0) {
			refI = myData->mData.config.maxI[0] * attr_count;
		}
	} else if(refI < (myData->mData.config.minI[0] * attr_count)) {
		if(myData->mData.config.minI[0] != 0) {
			refI = myData->mData.config.minI[0] * attr_count;
		}
	}*/
	idx = IDX_LOC_OBJ_REF_I;
	myTestCond->local_object[step][idx] = refI;
//20181219 KHK--------------------------------------	
	if(myTestCond->local_object[step][IDX_LOC_OBJ_SOC_TRACKING_FLAG] == P1){
		if(type == STEP_CHARGE) {
			refI = myData->mData.config.maxI[0] * attr_count;
		}
	}
//---------------------------------------------------	

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

		l_val2 = (long)P1_stepCond->reference[0].rangeI;
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

		l_val2 = (long)P1_stepCond->reference[0].rangeI;
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

#if defined __COA_VER_100B__ || defined __COA_VER_100B2__ || defined __COA_VER_100C__
#else //COA_VER_100D~
	if(mode == MODE_CC) { //kjh_160623
		for(i=0; i < MAX_P1_CAN_FUNCTION; i++) {
			idx = IDX_LOC_OBJ_CAN_FUNC_DIV_1 + i;
			func_div = (int)myTestCond->local_object[step][idx];
			if(func_div == CAN_RX_FUNC_DIV_BMS_LINK_POWER) {
				idx = IDX_LOC_OBJ_RANGE_I;
				myTestCond->local_object[step][idx] = RANGE1 - 1;
			}
		}
	}
#endif

	//end_condition
	idx = IDX_LOC_OBJ_END_AMPARE_HOUR;
	myTestCond->local_object[step][idx] = P1_stepCond->reference[0]
		.AmpareHour_SumAmpareHour;
	if(myTestCond->local_object[step][idx] >= 0) {
		make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
			C_CD_END_AMPARE_HOUR, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=
	} else {
		make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
			C_CD_END_AMPARE_HOUR, 1, COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=
	}

	idx = IDX_LOC_OBJ_END_AMPARE_HOUR_BRANCH;
	myTestCond->local_object[step][idx] = P1_stepCond->reference[0]
		.AmpareHour_Branch_MultiCycleCountId;
	if(myTestCond->local_object[step][idx] == ACTIVE_DIV_NONE)
		myTestCond->local_object[step][idx] = ACTIVE_DIV_NEXT;

	idx = IDX_LOC_OBJ_END_WATT_HOUR;
	myTestCond->local_object[step][idx] = P1_stepCond->reference[0]
		.WattHour_SumWattHour;
	if(myTestCond->local_object[step][idx] >= 0) {
		make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
			C_CD_END_WATT_HOUR, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=
	} else {
		make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
			C_CD_END_WATT_HOUR, 1, COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=
	}
			
	idx = IDX_LOC_OBJ_END_WATT_HOUR_BRANCH;
	myTestCond->local_object[step][idx] = P1_stepCond->reference[0]
		.WattHour_Branch_MultiCycleCount_Branch;
	if(myTestCond->local_object[step][idx] == ACTIVE_DIV_NONE)
		myTestCond->local_object[step][idx] = ACTIVE_DIV_NEXT;

	tmp = (long)P1_stepCond->reference[0].ValueRate_Item;
	if(tmp == P1_VALUE_RATE_ITEM_AMPARE_HOUR) {
		idx = IDX_LOC_OBJ_END_AMPARE_HOUR_RATE;
		//myTestCond[ch].local_object[step][idx] = P1_stepCond->reference[0]
		//kjh_171214
		myTestCond->local_object[step][idx] = P1_stepCond->reference[0]
			.ValueRate;
		make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
			C_CD_END_VALUE_RATE_AMPARE_HOUR, 1,
			COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

		idx = IDX_LOC_OBJ_END_AMPARE_HOUR_RATE_BRANCH;
		myTestCond->local_object[step][idx] = P1_stepCond->reference[0]
			.ValueRate_Branch;
		if(myTestCond->local_object[step][idx] == ACTIVE_DIV_NONE)
			myTestCond->local_object[step][idx] = ACTIVE_DIV_NEXT;

		idx = IDX_LOC_OBJ_END_AMPARE_HOUR_RATE_COMPARE;
		myTestCond->local_object[step][idx] = P1_stepCond->reference[0]
			.ValueRate_Compare;
	} else if(tmp == P1_VALUE_RATE_ITEM_WATT_HOUR) {
		idx = IDX_LOC_OBJ_END_WATT_HOUR_RATE;
		myTestCond->local_object[step][idx] = P1_stepCond->reference[0]
			.ValueRate;
		make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
			C_CD_END_VALUE_RATE_WATT_HOUR, 1,
			COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

		idx = IDX_LOC_OBJ_END_WATT_HOUR_RATE_BRANCH;
		myTestCond->local_object[step][idx] = P1_stepCond->reference[0]
			.ValueRate_Branch;
		if(myTestCond->local_object[step][idx] == ACTIVE_DIV_NONE)
			myTestCond->local_object[step][idx] = ACTIVE_DIV_NEXT;

		idx = IDX_LOC_OBJ_END_WATT_HOUR_RATE_COMPARE;
		myTestCond->local_object[step][idx] = P1_stepCond->reference[0]
			.ValueRate_Compare;
	}

	if(type == STEP_CHARGE) {
		idx = IDX_LOC_OBJ_END_I;
		myTestCond->local_object[step][idx] = P1_stepCond->reference[0].Current;
		make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
			C_CD_END_I, 1, COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=
//20181219 KHK-----------------------------------------------------
		idx = IDX_LOC_OBJ_END_SOC;
		myTestCond->local_object[step][idx] = P1_stepCond->reference[0].endSOC; //10Base
		userlog(DEBUG_LOG, psName,"endSOC %d\n", myTestCond->local_object[step][idx]);
		make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
			C_CD_END_SOC, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

		idx = IDX_LOC_OBJ_END_SOC_BRANCH;
		myTestCond->local_object[step][idx] = P1_stepCond->reference[0]
			.endSOC_branch;
		if(myTestCond->local_object[step][idx] == ACTIVE_DIV_NONE)
			myTestCond->local_object[step][idx] = ACTIVE_DIV_NEXT;
//----------------------------------------------------------------------		
	} else if(type == STEP_DISCHARGE || type == STEP_Z) {
		idx = IDX_LOC_OBJ_END_I;
		myTestCond->local_object[step][idx] = P1_stepCond->reference[0]
			.Current * (-1);
		make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
			C_CD_END_I, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

//20181219 KHK-----------------------------------------------------
		idx = IDX_LOC_OBJ_END_SOC;
		myTestCond->local_object[step][idx] = P1_stepCond->reference[0].endSOC; //10Base
		userlog(DEBUG_LOG, psName,"endSOC %d\n", myTestCond->local_object[step][idx]);
		make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
			C_CD_END_SOC, 1, COMP_LESS_THAN_OR_EQUAL_TO, idx); //>=

		idx = IDX_LOC_OBJ_END_SOC_BRANCH;
		myTestCond->local_object[step][idx] = P1_stepCond->reference[0]
			.endSOC_branch;
		if(myTestCond->local_object[step][idx] == ACTIVE_DIV_NONE)
			myTestCond->local_object[step][idx] = ACTIVE_DIV_NEXT;
//----------------------------------------------------------------------	
	}

	idx = IDX_LOC_OBJ_END_DELTA_V;
	myTestCond->local_object[step][idx] = P1_stepCond->reference[0].DeltaV;
	make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
		C_CD_END_DELTA_V, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_LOC_OBJ_END_P;
	myTestCond->local_object[step][idx] = P1_stepCond->reference[0].Power;
	if(type == STEP_CHARGE) {
		make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
			C_CD_END_P, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=
	} else if(type == STEP_DISCHARGE || type == STEP_Z) {
		make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
			C_CD_END_P, 1, COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=
	}

	//step edlc
	l_val = P1_stepCond->edlc.capacitanceV1;
	l_val2 = P1_stepCond->edlc.capacitanceV2;
	if(refI >= 0) {
		if(l_val > l_val2) {
			l_val2 = P1_stepCond->edlc.capacitanceV1;
			l_val = P1_stepCond->edlc.capacitanceV2;
		}
	} else {
		if(l_val2 > l_val) {
			l_val2 = P1_stepCond->edlc.capacitanceV1;
			l_val = P1_stepCond->edlc.capacitanceV2;
		}
	}

	if(l_val < 0 || l_val2 < 0) {
		l_val = 0;
		l_val2 = 0;
	}

	idx = IDX_LOC_OBJ_CAPACITANCE_V1;
	myTestCond->local_object[step][idx] = l_val;

	idx = IDX_LOC_OBJ_CAPACITANCE_V2;
	myTestCond->local_object[step][idx] = l_val2;

	//fault_condition
	switch(myData->AppControl.config.systemModel) {
		//for motor inverter application
		case C_DAEWOO_450V_200A_10A_90KW:
		case C_DAEWOO_450V_200A_10A_90KW_2:
		case C_DAEWOO_450V_200A_50A_90KW:
		case C_DAEWOO_450V_200A_50A_90KW_2:	//jhk_150710
			fault_count = 50; //500ms delay
			break;
		default:
			fault_count = 1;
			break;
	}

	switch(type) {
		case STEP_CHARGE:
			idx = IDX_LOC_OBJ_FAULT_HW_UPPER_V;
			myTestCond->local_object[step][idx] = 5000; //5.0%
			make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
				C_CD_FAULT_HW_UPPER_V, fault_count,
				COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

			switch(myData->AppControl.config.systemModel) {
				case C_SDI_70V_50A_5A_4KW:
				case C_SDI_70V_50A_5A_4KW_2:
				case C_SDI_70V_50A_5A_7KW:
				case C_SDI_70V_50A_5A_7KW_2:
				case C_SDI_70V_250A_25A_18KW:
					idx = IDX_LOC_OBJ_FAULT_LOWER_DELTA_V;
					myTestCond->local_object[step][idx]
						= myPs->testConfig.fail_charge_lower_dv;
					idx2 = IDX_LOC_OBJ_FAULT_LOWER_DELTA_V_T;
					myTestCond->local_object[step][idx2] = 100; //1sec
					make_loc_chCode_cond2((S_TEST_CONDITION *)myTestCond, step,
						C_CD_FAULT_LOWER_DELTA_V, 1,
						COMP_GREATER_THAN_OR_EQUAL_TO, idx, idx2); //>=
					break;
				default:
					break;
			}
			/*//jhkw_180823s
			dv_time = P1_stepCond->faultDeltaV_T;
			di_time = P1_stepCond->faultDeltaI_T;
			
			idx = IDX_LOC_OBJ_FAULT_COND_LOW_DELTA_V;
			myTestCond->local_object[step][idx] = P1_stepCond->faultDeltaV;
			make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
				C_CD_FAULT_COND_LOW_DELTA_V, dv_time,
				COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=
		
			idx = IDX_LOC_OBJ_FAULT_COND_UP_DELTA_I;
			myTestCond->local_object[step][idx] = P1_stepCond->faultDeltaI;
			make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
				C_CD_FAULT_COND_UP_DELTA_I, di_time,
				COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=
			*///jhkw_180823e
			break;
		case STEP_DISCHARGE:
			idx = IDX_LOC_OBJ_FAULT_HW_LOWER_V;
			myTestCond->local_object[step][idx] = 5000; //5.0%
			make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
				C_CD_FAULT_HW_LOWER_V, fault_count,
				COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=
			/*//jhkw_180823s
			dv_time = P1_stepCond->faultDeltaV_T;
			di_time = P1_stepCond->faultDeltaI_T;
			
			idx = IDX_LOC_OBJ_FAULT_COND_UP_DELTA_V;
			myTestCond->local_object[step][idx] = P1_stepCond->faultDeltaV;
			make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
				C_CD_FAULT_COND_UP_DELTA_V, dv_time,
				COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=
		
			idx = IDX_LOC_OBJ_FAULT_COND_LOW_DELTA_I;
			myTestCond->local_object[step][idx] = P1_stepCond->faultDeltaI;
			make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
				C_CD_FAULT_COND_LOW_DELTA_I, di_time,
				COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=
			*///jhkw_180823e
			break;
		case STEP_Z:
			idx = IDX_LOC_OBJ_FAULT_HW_LOWER_V;
			myTestCond->local_object[step][idx] = 5000; //5.0%
			make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
				C_CD_FAULT_HW_LOWER_V, fault_count,
				COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=
			break;
		default: break;
	}

	idx = IDX_LOC_OBJ_FAULT_HW_UPPER_I;
	myTestCond->local_object[step][idx] = 5000; //5.0%
	make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
		C_CD_FAULT_HW_UPPER_I, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=
//20181219 KHKW-------------------------
	if(myTestCond->local_object[step][IDX_LOC_OBJ_SOC_TRACKING_FLAG] == P1
		|| myTestCond->local_object[step][IDX_LOC_OBJ_SEQUENCE_CHARGE_FLAG] == P1){
	} else {
		idx = IDX_LOC_OBJ_FAULT_HW_LOWER_I; //kjhw_130912
		myTestCond->local_object[step][idx] = 5000; //5.0%
		make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
			C_CD_FAULT_HW_LOWER_I, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=
	}
//----------------------------------------------
	idx = IDX_LOC_OBJ_FAULT_UPPER_I;
	if(type == STEP_CHARGE) {
		myTestCond->local_object[step][idx] = P1_stepCond->faultUpperI;
		make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
			C_CD_FAULT_UPPER_I, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=
	} else if(type == STEP_DISCHARGE || type == STEP_Z) {
		myTestCond->local_object[step][idx] = P1_stepCond->faultUpperI * (-1);
		make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
			C_CD_FAULT_UPPER_I, 1, COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=
	}

	idx = IDX_LOC_OBJ_FAULT_LOWER_I;
	if(type == STEP_CHARGE) {
		myTestCond->local_object[step][idx] = P1_stepCond->faultLowerI;
		make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
			C_CD_FAULT_LOWER_I, 1, COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=
	} else if(type == STEP_DISCHARGE || type == STEP_Z) {
		myTestCond->local_object[step][idx] = P1_stepCond->faultLowerI * (-1);
		make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
			C_CD_FAULT_LOWER_I, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=
	}

	if(type == STEP_Z) {
		idx = IDX_LOC_OBJ_FAULT_UPPER_Z;
		myTestCond->local_object[step][idx] = P1_stepCond->faultUpperZ;
		make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
			C_CD_FAULT_UPPER_Z, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

		idx = IDX_LOC_OBJ_FAULT_LOWER_Z;
		myTestCond->local_object[step][idx] = P1_stepCond->faultLowerZ;
		make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
			C_CD_FAULT_LOWER_Z, 1, COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=
	}

	idx = IDX_LOC_OBJ_FAULT_UPPER_AMPARE_HOUR;
	myTestCond->local_object[step][idx] = P1_stepCond->faultUpper_AmpareHour;
	make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
		C_CD_FAULT_UPPER_AMPARE_HOUR, 1,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_LOC_OBJ_FAULT_LOWER_AMPARE_HOUR;
	myTestCond->local_object[step][idx] = P1_stepCond->faultLower_AmpareHour;
	make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
		C_CD_FAULT_LOWER_AMPARE_HOUR, 1, COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=
	
	return rtn;
}

int convert_test_cond_pattern(S_P1_TEST_COND_STEP *P1_stepCond, int ch, int step, long type, long attr_count, int div)
{
	int idx, i, rtn, fault_count;
#if defined __COA_VER_100B__ || defined __COA_VER_100B2__ || defined __COA_VER_100C__
#else //COA_VER_100D~
	int func_div;
	long refI2;
#endif
	long pattern_index, mode, l_val, refV, refI, tmp;
	
#if defined __COA_VER_100B__ || defined __COA_VER_100B2__ || defined __COA_VER_100C__
	//header
	idx = IDX_LOC_OBJ_PATTERN_INDEX;
	pattern_index = P1_stepCond->header.patternIndex;
	myTestCond->local_object[step][idx] = pattern_index;

	rtn = Read_Pattern_File_1(psName, ch, pattern_index);
#else //COA_VER_100D~
	//header
	idx = IDX_LOC_OBJ_PATTERN_INDEX;
	pattern_index = (long)step;
	myTestCond->local_object[step][idx] = pattern_index;

	idx = IDX_LOC_OBJ_PATTERN_FILE_SIZE;
	myTestCond->local_object[step][idx] = (long)P1_stepCond->pattern_file_size;
	myData->testCond[ch].local_object[step][idx] = (long)P1_stepCond->pattern_file_size;
	idx = IDX_LOC_OBJ_PATTERN_FILE_CHECKSUM;			//jhkw_200816
	myTestCond->local_object[step][idx]
		= (long)P1_stepCond->pattern_file_checksum;
	myData->testCond[ch].local_object[step][idx]
		= (long)P1_stepCond->pattern_file_checksum;		//jhkw_200816

	idx = IDX_LOC_OBJ_PATTERN_UPDATED; //kjg_170810
	//jhkw_200517s
	//myTestCond->local_object[step][idx] = (long)div;
	if(myData->COA_Client[0].signal[P1_SIG_TEST_HEADER_UPDATE_RCV] == 1) {
		myData->testCond_update.local_object[step][idx] = P1;
		myData->testCond[ch].local_object[step][idx] = P1;			//jhkw_200816
	} else {
		myData->testCond_update.local_object[step][idx] = P0;
		myData->testCond[ch].local_object[step][idx] = P0;			//jhkw_200816
	}
	//jhkw_200517e

	rtn = read_test_cond_pattern_file_coa(psName, ch, (int)pattern_index, 0);
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "ch%d pattern file error %d\n", ch+1, rtn);
		return rtn;
	}
	idx = IDX_LOC_OBJ_SOC_TRACKING_FLAG;
	//if(myData->testCond[ch].local_object[step][idx] == P1){
	if(myTestCond->local_object[step][idx] == P1){
		rtn = Read_SOC_Tracking_File(psName, ch, (int)pattern_index, 0);
	}
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "ch%d Charge SOC file error %d\n", ch+1, rtn);
		return rtn;
	}

	//jhkw_201102s
	idx = IDX_LOC_OBJ_DISCHARGE_SOC_TRACKING_FLAG;
	//if(myData->testCond[ch].local_object[step][idx] == P1){
	if(myTestCond->local_object[step][idx] == P1){
		rtn = Read_SOC_Tracking_File(psName, ch, (int)pattern_index, 1);
	}
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "ch%d DisCharge SOC file error %d\n", ch+1, rtn);
		return rtn;
	}
	//jhkw_201102e
#endif

	idx = IDX_LOC_OBJ_MODE;
	mode = convert_step_mode(CONVERT_P1_TO_ORG, (long)P1_stepCond->header.mode);
	myTestCond->local_object[step][idx] = mode;

#if defined __COA_VER_100B__ || defined __COA_VER_100B2__ || defined __COA_VER_100C__
	//reference
	idx = IDX_LOC_OBJ_REF_V; //charge ref_v
	refV = P1_stepCond->reference[0].refV_upper;
	if(refV > myData->mData.config.maxV[0]) {
		refV = myData->mData.config.maxV[0];
	} else if(refV < myData->mData.config.minV[0]) {
		refV = myData->mData.config.minV[0];
	}
	myTestCond->local_object[step][idx] = refV;

	idx = IDX_COM_OBJ_FAULT_UPPER_V; //kjhw_120517s
	l_val = myTestCond->common_object[idx];

	if(l_val == 0) {
		l_val = RANGE1 - 1;
	} else if(l_val <= myData->mData.config.maxV[3]) {
		l_val = RANGE4 - 1;
	} else if(l_val <= myData->mData.config.maxV[2]) {
		l_val = RANGE3 - 1;
	} else if(l_val <= myData->mData.config.maxV[1]) {
		l_val = RANGE2 - 1;
	} else {
		l_val = RANGE1 - 1;
	}

	if((l_val+1) > (long)myData->mData.config.rangeV) {
		l_val = (long)myData->mData.config.rangeV - 1;
	}

	idx = IDX_LOC_OBJ_RANGE_V;
	myTestCond->local_object[step][idx] = l_val; //kjhw_120517e
	
	idx = IDX_LOC_OBJ_REF_V2; //discharge ref_v
	refV = P1_stepCond->reference[0].refV_lower;
	if(refV > myData->mData.config.maxV[0]) {
		refV = myData->mData.config.maxV[0];
	} else if(refV < myData->mData.config.minV[0]) {
		refV = myData->mData.config.minV[0];
	}
	myTestCond->local_object[step][idx] = refV;

	idx = IDX_COM_OBJ_FAULT_UPPER_V; //kjhw_120517s
	l_val = myTestCond->common_object[idx];

	if(l_val == 0) {
		l_val = RANGE1 - 1;
	} else if(l_val <= myData->mData.config.maxV[3]) {
		l_val = RANGE4 - 1;
	} else if(l_val <= myData->mData.config.maxV[2]) {
		l_val = RANGE3 - 1;
	} else if(l_val <= myData->mData.config.maxV[1]) {
		l_val = RANGE2 - 1;
	} else {
		l_val = RANGE1 - 1;
	}

	if((l_val+1) > (long)myData->mData.config.rangeV) {
		l_val = (long)myData->mData.config.rangeV - 1;
	}

	idx = IDX_LOC_OBJ_RANGE_V2;
	myTestCond->local_object[step][idx] = l_val; //kjhw_120517e

	if(mode == MODE_CC) {
		refI = 0;
		for(i=1; i < MAX_PATTERN_DATA; i++) {
			l_val = myTestCond->pattern[i].t_val;
			if(l_val < 0) break;

			l_val = myTestCond->pattern[i].cmd_val[0];
			if(l_val < 0) l_val *= (-1);

			if(l_val > refI) refI = l_val;
		}
	} else { //MODE_CP
		refI = myData->mData.config.maxI[0] * attr_count;
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
	idx = IDX_LOC_OBJ_RANGE_I;
	myTestCond->local_object[step][idx] = l_val;
#else //COA_VER_100D~
	//reference
	switch(mode) { //kjhw_150210s
		case MODE_CV:
			/*refV = 0;
			for(i=1; i < MAX_PATTERN_DATA; i++) {
				l_val = myTestCond->pattern[i].t_val;
				if(l_val < 0) break;

				l_val = myTestCond->pattern[i].cmd_val[0]; //Vref
				if(l_val < 0) {
					userlog(DEBUG_LOG, psName,
						"ch%d pattern file error2 (V)%d\n",
						ch+1, (int)pattern_index);
					return -1;
				}
				if(l_val > refV) refV = l_val;
			}
			idx = IDX_LOC_OBJ_REF_V;
			if(refV > myData->mData.config.maxV[0]) {
				refV = myData->mData.config.maxV[0];
			} else if(refV < myData->mData.config.minV[0]) {
				refV = myData->mData.config.minV[0];
			}*/
			idx = IDX_LOC_OBJ_REF_V;
			myTestCond->local_object[step][idx]
				= myData->mData.config.maxV[0]; //kjhw_150507_tt

			idx = IDX_COM_OBJ_FAULT_UPPER_V;
			l_val = myTestCond->common_object[idx];

			if(l_val == 0) {
				l_val = RANGE1 - 1;
			} else if(l_val <= myData->mData.config.maxV[3]) {
				l_val = RANGE4 - 1;
			} else if(l_val <= myData->mData.config.maxV[2]) {
				l_val = RANGE3 - 1;
			} else if(l_val <= myData->mData.config.maxV[1]) {
				l_val = RANGE2 - 1;
			} else {
				l_val = RANGE1 - 1;
			}

			if((l_val+1) > (long)myData->mData.config.rangeV) {
				l_val = (long)myData->mData.config.rangeV - 1;
			}

			idx = IDX_LOC_OBJ_RANGE_V; //kjhw_150507_tt
			//myTestCond->local_object[step][idx] = RANGE1 - 1;
			myTestCond->local_object[step][idx] = l_val; //kjhw_181128

			/*refV = myData->mData.config.maxV[0];
			for(i=1; i < MAX_PATTERN_DATA; i++) {
				l_val = myTestCond->pattern[i].t_val;
				if(l_val < 0) break;

				l_val = myTestCond->pattern[i].cmd_val[0]; //Vref
				if(l_val < refV) refV = l_val;
			}
			idx = IDX_LOC_OBJ_REF_V2; //discharge ref_v
			if(refV > myData->mData.config.maxV[0]) {
				refV = myData->mData.config.maxV[0];
			} else if(refV < myData->mData.config.minV[0]) {
				refV = myData->mData.config.minV[0];
			}*/
			idx = IDX_LOC_OBJ_REF_V2; //discharge ref_v
			myTestCond->local_object[step][idx]
				= myData->mData.config.minV[0]; //kjhw_150210_tt

			idx = IDX_COM_OBJ_FAULT_UPPER_V;
			l_val = myTestCond->common_object[idx];

			if(l_val == 0) {
				l_val = RANGE1 - 1;
			} else if(l_val <= myData->mData.config.maxV[3]) {
				l_val = RANGE4 - 1;
			} else if(l_val <= myData->mData.config.maxV[2]) {
				l_val = RANGE3 - 1;
			} else if(l_val <= myData->mData.config.maxV[1]) {
				l_val = RANGE2 - 1;
			} else {
				l_val = RANGE1 - 1;
			}

			if((l_val+1) > (long)myData->mData.config.rangeV) {
				l_val = (long)myData->mData.config.rangeV - 1;
			}
			idx = IDX_LOC_OBJ_RANGE_V2; //kjhw_150507_tt
			//myTestCond->local_object[step][idx] = RANGE1 - 1;
			myTestCond->local_object[step][idx] = l_val;

			/*refI = 0;
			for(i=1; i < MAX_PATTERN_DATA; i++) {
				l_val = myTestCond->pattern[i].t_val;
				if(l_val < 0) break;

				l_val = myTestCond->pattern[i].cmd_val[1]; //+Iref
				if(l_val < 0) {
					userlog(DEBUG_LOG, psName,
						"ch%d pattern file error2 (+I)%d\n",
						ch+1, (int)pattern_index);
					return -1;
				}
				if(l_val > refI) refI = l_val;
			}*/
			idx = IDX_LOC_OBJ_REF_I;
			refI = P1_stepCond->reference[0].pattern_max_refI;
			if(refI > (myData->mData.config.maxI[0] * attr_count)) {
				if(myData->mData.config.maxI[0] != 0) {
					refI = myData->mData.config.maxI[0] * attr_count;
				}
			} else if(refI < 0) refI = 0;
			myTestCond->local_object[step][idx] = refI;

			/*refI2 = 0;
			for(i=1; i < MAX_PATTERN_DATA; i++) {
				l_val = myTestCond->pattern[i].t_val;
				if(l_val < 0) break;

				l_val = myTestCond->pattern[i].cmd_val[2]; //-Iref
				if(l_val > 0) {
					userlog(DEBUG_LOG, psName,
						"ch%d pattern file error2 (-I)%d\n",
						ch+1, (int)pattern_index);
					return -1;
				}
				if(l_val < refI2) refI2 = l_val;
			}*/
			idx = IDX_LOC_OBJ_REF_I2;
			refI2 = P1_stepCond->reference[0].pattern_min_refI;
			if(refI2 < (myData->mData.config.minI[0] * attr_count)) {
				if(myData->mData.config.minI[0] != 0) {
					refI2 = myData->mData.config.minI[0] * attr_count;
				}
			} else if(refI2 > 0) refI2 = 0;
			if(refI < refI2 * (-1)) refI = refI2;
			myTestCond->local_object[step][idx] = refI2;

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
			idx = IDX_LOC_OBJ_RANGE_I;
			myTestCond->local_object[step][idx] = l_val;
			break;
		default: //MODE_CC, MODE_CP
			idx = IDX_LOC_OBJ_REF_V; //charge ref_v
			refV = P1_stepCond->reference[0].refV_upper;
			if(refV > myData->mData.config.maxV[0]) {
				refV = myData->mData.config.maxV[0];
			} else if(refV < myData->mData.config.minV[0]) {
				refV = myData->mData.config.minV[0];
			}
			myTestCond->local_object[step][idx] = refV;

			idx = IDX_COM_OBJ_FAULT_UPPER_V; //kjhw_120517s
			l_val = myTestCond->common_object[idx];

			if(l_val == 0) {
				l_val = RANGE1 - 1;
			} else if(l_val <= myData->mData.config.maxV[3]) {
				l_val = RANGE4 - 1;
			} else if(l_val <= myData->mData.config.maxV[2]) {
				l_val = RANGE3 - 1;
			} else if(l_val <= myData->mData.config.maxV[1]) {
				l_val = RANGE2 - 1;
			} else {
				l_val = RANGE1 - 1;
			}

			if((l_val+1) > (long)myData->mData.config.rangeV) {
				l_val = (long)myData->mData.config.rangeV - 1;
			}

			idx = IDX_LOC_OBJ_RANGE_V;
			myTestCond->local_object[step][idx] = l_val; //kjhw_120517e
	
			idx = IDX_LOC_OBJ_REF_V2; //discharge ref_v
			refV = P1_stepCond->reference[0].refV_lower;
			if(refV > myData->mData.config.maxV[0]) {
				refV = myData->mData.config.maxV[0];
			} else if(refV < myData->mData.config.minV[0]) {
				refV = myData->mData.config.minV[0];
			}
			myTestCond->local_object[step][idx] = refV;

			idx = IDX_COM_OBJ_FAULT_UPPER_V; //kjhw_120517s
			l_val = myTestCond->common_object[idx];

			if(l_val == 0) {
				l_val = RANGE1 - 1;
			} else if(l_val <= myData->mData.config.maxV[3]) {
				l_val = RANGE4 - 1;
			} else if(l_val <= myData->mData.config.maxV[2]) {
				l_val = RANGE3 - 1;
			} else if(l_val <= myData->mData.config.maxV[1]) {
				l_val = RANGE2 - 1;
			} else {
				l_val = RANGE1 - 1;
			}

			if((l_val+1) > (long)myData->mData.config.rangeV) {
				l_val = (long)myData->mData.config.rangeV - 1;
			}

			idx = IDX_LOC_OBJ_RANGE_V2;
			myTestCond->local_object[step][idx] = l_val; //kjhw_120517e

			if(mode == MODE_CC) {
				/*refI = 0;
				for(i=1; i < MAX_PATTERN_DATA; i++) {
					//jhkw_161026s
					//if(myTestCond->pattern[0].t_val == 2) {
					if(myData->testCond[ch].pattern[0].t_val == 2) {
					} else {
						//l_val = myTestCond->pattern[i].t_val;
						//l_val = myData->testCond[0].pattern[i].t_val;
						l_val = myData->testCond[ch].pattern[i].t_val;
						if(l_val < 0) break;
					} //jhkw_161026e

					//l_val = myTestCond->pattern[i].cmd_val[0];
					//l_val = myData->testCond[0].pattern[i].cmd_val[0];
					l_val = myData->testCond[ch].pattern[i].cmd_val[0];
					if(l_val < 0) l_val *= (-1);

					if(l_val > refI) refI = l_val;
				}*/
				refI = P1_stepCond->reference[0].pattern_max_refI;
				refI2 = P1_stepCond->reference[0].pattern_min_refI;
				refI2 *= (-1);
				if(refI < refI2) {
					refI = refI2;
				}
			} else { //MODE_CP
				refI = myData->mData.config.maxI[0] * attr_count;
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
			idx = IDX_LOC_OBJ_RANGE_I;
			myTestCond->local_object[step][idx] = l_val;

			for(i=0; i < MAX_P1_CAN_FUNCTION; i++) { //kjh_160623
				idx = IDX_LOC_OBJ_CAN_FUNC_DIV_1 + i;
				func_div = (int)myTestCond->local_object[step][idx];
				if(func_div == CAN_RX_FUNC_DIV_BMS_LINK_POWER) {
					idx = IDX_LOC_OBJ_RANGE_I;
					myTestCond->local_object[step][idx] = RANGE1 - 1;
				}
			}
			break;
	} //kjhw_150210e
#endif

	//end_condition
	idx = IDX_LOC_OBJ_END_AMPARE_HOUR;
	myTestCond->local_object[step][idx] = P1_stepCond->reference[0]
		.AmpareHour_SumAmpareHour;
	if(myTestCond->local_object[step][idx] >= 0) {
		make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
			C_CD_END_AMPARE_HOUR, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=
	} else {
		make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
			C_CD_END_AMPARE_HOUR, 1, COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=
	}

	idx = IDX_LOC_OBJ_END_AMPARE_HOUR_BRANCH;
	myTestCond->local_object[step][idx] = P1_stepCond->reference[0]
		.AmpareHour_Branch_MultiCycleCountId;
	if(myTestCond->local_object[step][idx] == ACTIVE_DIV_NONE)
		myTestCond->local_object[step][idx] = ACTIVE_DIV_NEXT;

	idx = IDX_LOC_OBJ_END_WATT_HOUR;
	myTestCond->local_object[step][idx] = P1_stepCond->reference[0]
		.WattHour_SumWattHour;
	if(myTestCond->local_object[step][idx] >= 0) {
		make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
			C_CD_END_WATT_HOUR, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=
	} else {
		make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
			C_CD_END_WATT_HOUR, 1, COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=
	}
			
	idx = IDX_LOC_OBJ_END_WATT_HOUR_BRANCH;
	myTestCond->local_object[step][idx] = P1_stepCond->reference[0]
		.WattHour_Branch_MultiCycleCount_Branch;
	if(myTestCond->local_object[step][idx] == ACTIVE_DIV_NONE)
		myTestCond->local_object[step][idx] = ACTIVE_DIV_NEXT;

	tmp = (long)P1_stepCond->reference[0].ValueRate_Item;
	if(tmp == P1_VALUE_RATE_ITEM_AMPARE_HOUR) {
		idx = IDX_LOC_OBJ_END_AMPARE_HOUR_RATE;
		myTestCond->local_object[step][idx] = P1_stepCond->reference[0]
			.ValueRate;
		make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
			C_CD_END_VALUE_RATE_AMPARE_HOUR, 1,
			COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

		idx = IDX_LOC_OBJ_END_AMPARE_HOUR_RATE_BRANCH;
		myTestCond->local_object[step][idx] = P1_stepCond->reference[0]
			.ValueRate_Branch;
		if(myTestCond->local_object[step][idx] == ACTIVE_DIV_NONE)
			myTestCond->local_object[step][idx] = ACTIVE_DIV_NEXT;

		idx = IDX_LOC_OBJ_END_AMPARE_HOUR_RATE_COMPARE;
		myTestCond->local_object[step][idx] = P1_stepCond->reference[0]
			.ValueRate_Compare;
	} else if(tmp == P1_VALUE_RATE_ITEM_WATT_HOUR) {
		idx = IDX_LOC_OBJ_END_WATT_HOUR_RATE;
		myTestCond->local_object[step][idx] = P1_stepCond->reference[0]
			.ValueRate;
		make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
			C_CD_END_VALUE_RATE_WATT_HOUR, 1,
			COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

		idx = IDX_LOC_OBJ_END_WATT_HOUR_RATE_BRANCH;
		myTestCond->local_object[step][idx] = P1_stepCond->reference[0]
			.ValueRate_Branch;
		if(myTestCond->local_object[step][idx] == ACTIVE_DIV_NONE)
			myTestCond->local_object[step][idx] = ACTIVE_DIV_NEXT;

		idx = IDX_LOC_OBJ_END_WATT_HOUR_RATE_COMPARE;
		myTestCond->local_object[step][idx] = P1_stepCond->reference[0]
			.ValueRate_Compare;
	}
//jhkw_190314-----------------------------------------------------
	idx = IDX_LOC_OBJ_END_SOC;
	myTestCond->local_object[step][idx] = P1_stepCond->reference[0].endSOC; //10Base
	userlog(DEBUG_LOG, psName,"endSOC %d\n", myTestCond->local_object[step][idx]);
	make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
		C_CD_END_SOC, 1, COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=
		idx = IDX_LOC_OBJ_END_SOC_BRANCH;
	myTestCond->local_object[step][idx] = P1_stepCond->reference[0]
		.endSOC_branch;
	if(myTestCond->local_object[step][idx] == ACTIVE_DIV_NONE)
		myTestCond->local_object[step][idx] = ACTIVE_DIV_NEXT;
//----------------------------------------------------------------------	
	//jhkw_221205s
	if(myTestCond->local_object[step][IDX_LOC_OBJ_SEQUENCE_CHARGE_FLAG] == P1){
		idx = IDX_LOC_OBJ_PATTERN_INDEX;
		pattern_index = (long)step;
		myTestCond->local_object[step][idx] = pattern_index;
		rtn = Read_Sequence_Charge_File(psName, ch, (int)pattern_index);
	}
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "ch%d Sequence Charge file error %d\n", ch+1, rtn);
		return rtn;
	}
	//jhkw_221205e

	//fault_condition
	switch(myData->AppControl.config.systemModel) {
		//for motor inverter application
		case C_DAEWOO_450V_200A_10A_90KW:
		case C_DAEWOO_450V_200A_10A_90KW_2:
		case C_DAEWOO_450V_200A_50A_90KW:
		case C_DAEWOO_450V_200A_50A_90KW_2:	//jhk_150710
			fault_count = 50; //500ms delay
			break;
		default:
			fault_count = 1;
			break;
	}

	idx = IDX_LOC_OBJ_FAULT_HW_UPPER_V;
	myTestCond->local_object[step][idx] = 5000; //5.0%
	make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
		C_CD_FAULT_HW_UPPER_V, fault_count,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_LOC_OBJ_FAULT_HW_LOWER_V;
	myTestCond->local_object[step][idx] = 5000; //5.0%
	make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
		C_CD_FAULT_HW_LOWER_V, fault_count,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_LOC_OBJ_FAULT_HW_UPPER_I;
	myTestCond->local_object[step][idx] = 5000; //5.0%
	make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
		C_CD_FAULT_HW_UPPER_I, 3, COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_LOC_OBJ_FAULT_UPPER_I;
	myTestCond->local_object[step][idx] = P1_stepCond->faultUpperI;

	idx = IDX_LOC_OBJ_FAULT_LOWER_I;
	myTestCond->local_object[step][idx] = P1_stepCond->faultLowerI;

	idx = IDX_LOC_OBJ_FAULT_UPPER_AMPARE_HOUR;
	myTestCond->local_object[step][idx] = P1_stepCond->faultUpper_AmpareHour;
	make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
		C_CD_FAULT_UPPER_AMPARE_HOUR, 1,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_LOC_OBJ_FAULT_LOWER_AMPARE_HOUR;
	myTestCond->local_object[step][idx] = P1_stepCond->faultLower_AmpareHour;
	make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
		C_CD_FAULT_LOWER_AMPARE_HOUR, 1, COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=

	return 0;
}

void convert_test_cond_external_can(S_P1_TEST_COND_STEP *P1_stepCond, int ch, int step, long type, long attr_count)
{
	int idx;
	long mode, refV1, refV2, refI, l_val, l_val2, refP, refR, tmp;
	double d_val1;
	long ratioV, ratioI; //jhkw_231127
	double ratioP; //jhkw_231127

	ratioV = myData->mData.ratioV;	//jhkw_231127s
	ratioI = myData->mData.ratioI;
	ratioP = myData->mData.ratioP;	//jhkw_231127e

	refP = refR = 0;

	//header
	idx = IDX_LOC_OBJ_MODE;
	mode = convert_step_mode(CONVERT_P1_TO_ORG, (long)P1_stepCond->header.mode);
	myTestCond->local_object[step][idx] = mode;

	//reference
	idx = IDX_LOC_OBJ_REF_V; //charge ref_v
	refV1 = P1_stepCond->reference[0].refV_upper;
	if(refV1 > myData->mData.config.maxV[0]) {
		refV1 = myData->mData.config.maxV[0];
	} else if(refV1 < myData->mData.config.minV[0]) {
		refV1 = myData->mData.config.minV[0];
	}
	myTestCond->local_object[step][idx] = refV1;

	idx = IDX_LOC_OBJ_RANGE_V; //kjg_w
	myTestCond->local_object[step][idx] = RANGE1 - 1;
	
	idx = IDX_LOC_OBJ_REF_V2; //discharge ref_v
	refV2 = P1_stepCond->reference[0].refV_lower;
	if(refV2 > myData->mData.config.maxV[0]) {
		refV2 = myData->mData.config.maxV[0];
	} else if(refV2 < myData->mData.config.minV[0]) {
		refV2 = myData->mData.config.minV[0];
	}
	myTestCond->local_object[step][idx] = refV2;

	idx = IDX_LOC_OBJ_RANGE_V2; //kjg_w
	myTestCond->local_object[step][idx] = RANGE1 - 1;

	refP = P1_stepCond->reference[0].refP;
	refI = myData->mData.config.maxI[0] * attr_count;
	//d_val1 = (double)(myData->mData.config.maxV[0] / 1000)
	//	* (double)(refI / 1000) / 1000.0;
	d_val1 = (double)(myData->mData.config.maxV[0] / ratioV)
		* (double)(refI / ratioI) / ratioP;	//jhkw_231127
	if(refP > (long)d_val1) refP = (long)d_val1;
	idx = IDX_LOC_OBJ_REF_P;
	myTestCond->local_object[step][idx] = refP;

	refI = P1_stepCond->reference[0].refI;
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

		l_val2 = (long)P1_stepCond->reference[0].rangeI;
		if(l_val2 == 0) { //auto range
		} else { //manual range
			if(l_val2 < l_val) {
				l_val = l_val2;
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

		l_val2 = (long)P1_stepCond->reference[0].rangeI;
		if(l_val2 == 0) { //auto range
		} else { //manual range
			if(l_val2 < l_val) {
				l_val = l_val2;
			}
		}
	} else {
		if(step == 0) {
			l_val = RANGE1 - 1;
		} else {
			l_val = myTestCond->local_object[step-1][idx];
		}
	} 
	l_val = RANGE1 - 1;
	myTestCond->local_object[step][idx] = l_val;

	//end_condition
	idx = IDX_LOC_OBJ_END_AMPARE_HOUR;
	myTestCond->local_object[step][idx] = P1_stepCond->reference[0]
		.AmpareHour_SumAmpareHour;
	if(myTestCond->local_object[step][idx] >= 0) {
		make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
			C_CD_END_AMPARE_HOUR, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=
	} else {
		make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
			C_CD_END_AMPARE_HOUR, 1, COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=
	}

	idx = IDX_LOC_OBJ_END_AMPARE_HOUR_BRANCH;
	myTestCond->local_object[step][idx] = P1_stepCond->reference[0]
		.AmpareHour_Branch_MultiCycleCountId;
	if(myTestCond->local_object[step][idx] == ACTIVE_DIV_NONE)
		myTestCond->local_object[step][idx] = ACTIVE_DIV_NEXT;

	idx = IDX_LOC_OBJ_END_WATT_HOUR;
	myTestCond->local_object[step][idx] = P1_stepCond->reference[0]
		.WattHour_SumWattHour;
	if(myTestCond->local_object[step][idx] >= 0) {
		make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
		C_CD_END_WATT_HOUR, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=
	} else {
		make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
			C_CD_END_WATT_HOUR, 1, COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=
	}
			
	idx = IDX_LOC_OBJ_END_WATT_HOUR_BRANCH;
	myTestCond->local_object[step][idx] = P1_stepCond->reference[0]
		.WattHour_Branch_MultiCycleCount_Branch;
	if(myTestCond->local_object[step][idx] == ACTIVE_DIV_NONE)
		myTestCond->local_object[step][idx] = ACTIVE_DIV_NEXT;

	tmp = (long)P1_stepCond->reference[0].ValueRate_Item;
	if(tmp == P1_VALUE_RATE_ITEM_AMPARE_HOUR) {
		idx = IDX_LOC_OBJ_END_AMPARE_HOUR_RATE;
		myTestCond->local_object[step][idx] = P1_stepCond->reference[0]
			.ValueRate;
		make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
			C_CD_END_VALUE_RATE_AMPARE_HOUR, 1,
			COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

		idx = IDX_LOC_OBJ_END_AMPARE_HOUR_RATE_BRANCH;
		myTestCond->local_object[step][idx] = P1_stepCond->reference[0]
			.ValueRate_Branch;
		if(myTestCond->local_object[step][idx] == ACTIVE_DIV_NONE)
			myTestCond->local_object[step][idx] = ACTIVE_DIV_NEXT;

		idx = IDX_LOC_OBJ_END_AMPARE_HOUR_RATE_COMPARE;
		myTestCond->local_object[step][idx] = P1_stepCond->reference[0]
			.ValueRate_Compare;
	} else if(tmp == P1_VALUE_RATE_ITEM_WATT_HOUR) {
		idx = IDX_LOC_OBJ_END_WATT_HOUR_RATE;
		myTestCond->local_object[step][idx] = P1_stepCond->reference[0]
			.ValueRate;
		make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
			C_CD_END_VALUE_RATE_WATT_HOUR, 1,
			COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

		idx = IDX_LOC_OBJ_END_WATT_HOUR_RATE_BRANCH;
		myTestCond->local_object[step][idx] = P1_stepCond->reference[0]
			.ValueRate_Branch;
		if(myTestCond->local_object[step][idx] == ACTIVE_DIV_NONE)
			myTestCond->local_object[step][idx] = ACTIVE_DIV_NEXT;

		idx = IDX_LOC_OBJ_END_WATT_HOUR_RATE_COMPARE;
		myTestCond->local_object[step][idx] = P1_stepCond->reference[0]
			.ValueRate_Compare;
	}

	idx = IDX_LOC_OBJ_END_DELTA_V;
	myTestCond->local_object[step][idx] = P1_stepCond->reference[0].DeltaV;
	make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
		C_CD_END_DELTA_V, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	//fault_condition
	idx = IDX_LOC_OBJ_FAULT_UPPER_I;
	myTestCond->local_object[step][idx] = P1_stepCond->faultUpperI;

	idx = IDX_LOC_OBJ_FAULT_LOWER_I;
	myTestCond->local_object[step][idx] = P1_stepCond->faultLowerI;

	idx = IDX_LOC_OBJ_FAULT_UPPER_AMPARE_HOUR;
	myTestCond->local_object[step][idx] = P1_stepCond->faultUpper_AmpareHour;
	make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
		C_CD_FAULT_UPPER_AMPARE_HOUR, 1,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_LOC_OBJ_FAULT_LOWER_AMPARE_HOUR;
	myTestCond->local_object[step][idx] = P1_stepCond->faultLower_AmpareHour;
	make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
		C_CD_FAULT_LOWER_AMPARE_HOUR, 1, COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=
}

int convert_test_cond_usermap(S_P1_TEST_COND_STEP *P1_stepCond, int ch, int step, long type, long attr_count, int div)
{ //kjhw_140828
#if defined __COA_VER_100B__ || defined __COA_VER_100B2__ || defined __COA_VER_100C__
#else //COA_VER_100D~
	int master, slave, idx, rtn, fault_count;
	int i, k, func_div, func_div1, func_div2, func_div3;
	long usermap_index, mode, l_val, refV, refI, tmp;

	k = 0;
	if(myData->mData.config.division_CAN == 1) {
		master = myData->canReceiveDataCount[ch][0];
		slave = 0;
	} else {
		master = myData->canReceiveDataCount[ch][0];
		slave = myData->canReceiveDataCount[ch][1];
	}

	for(i=0; i < master + slave; i++) {
		func_div1 = myData->canReceiveSetData.normalData[ch][i].function_div;
		func_div2 = myData->canReceiveSetData.normalData[ch][i].function_div2;
		func_div3 = myData->canReceiveSetData.normalData[ch][i].function_div3;
		func_div = CAN_RX_FUNC_DIV_USERMAP_TEMP; //define 1100
		if(func_div1 == func_div || func_div2 == func_div
			|| func_div3 == func_div) {
			if(k == 1 || k > 3) break;
			k += 1;
		}
		func_div = CAN_RX_FUNC_DIV_USERMAP_SOC; //define 1099
		if(func_div1 == func_div || func_div2 == func_div
			|| func_div3 == func_div) {
			if(k == 2 || k > 3) break;
			k += 2;
		}
	}
	if(k != 3) {
		userlog(DEBUG_LOG, psName, "ch%d usermap can_func set error %d\n",
			ch+1, k);
		return -1;
	}

	//header
	idx = IDX_LOC_OBJ_PATTERN_INDEX;
	usermap_index = (long)step;
	myTestCond->local_object[step][idx] = usermap_index;

	idx = IDX_LOC_OBJ_PATTERN_FILE_SIZE;
	myTestCond->local_object[step][idx] = (long)P1_stepCond->pattern_file_size;
	idx = IDX_LOC_OBJ_PATTERN_FILE_CHECKSUM;
	myTestCond->local_object[step][idx]
		= (long)P1_stepCond->pattern_file_checksum;

	idx = IDX_LOC_OBJ_USERMAP_UPDATED; //kjg_170810
	myTestCond->local_object[step][idx] = (long)div;

	rtn = read_test_cond_usermap_file_coa(psName, ch, (int)usermap_index);
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "ch%d usermap file error %d\n", ch+1, rtn);
		return rtn;
	}

	idx = IDX_LOC_OBJ_MODE;
	mode = convert_step_mode(CONVERT_P1_TO_ORG, (long)P1_stepCond->header.mode);
	myTestCond->local_object[step][idx] = mode;

	//reference
	idx = IDX_LOC_OBJ_REF_V; //charge ref_v
	refV = P1_stepCond->reference[0].refV_upper;
	if(refV > myData->mData.config.maxV[0]) {
		refV = myData->mData.config.maxV[0];
	} else if(refV < myData->mData.config.minV[0]) {
		refV = myData->mData.config.minV[0];
	}
	myTestCond->local_object[step][idx] = refV;

	idx = IDX_COM_OBJ_FAULT_UPPER_V; //kjhw_120517s
	l_val = myTestCond->common_object[idx];

	if(l_val == 0) {
		l_val = RANGE1 - 1;
	} else if(l_val <= myData->mData.config.maxV[3]) {
		l_val = RANGE4 - 1;
	} else if(l_val <= myData->mData.config.maxV[2]) {
		l_val = RANGE3 - 1;
	} else if(l_val <= myData->mData.config.maxV[1]) {
		l_val = RANGE2 - 1;
	} else {
		l_val = RANGE1 - 1;
	}

	if((l_val+1) > (long)myData->mData.config.rangeV) {
		l_val = (long)myData->mData.config.rangeV - 1;
	}

	idx = IDX_LOC_OBJ_RANGE_V;
	myTestCond->local_object[step][idx] = l_val; //kjhw_120517e
	
	idx = IDX_LOC_OBJ_REF_V2; //discharge ref_v
	refV = P1_stepCond->reference[0].refV_lower;
	if(refV > myData->mData.config.maxV[0]) {
		refV = myData->mData.config.maxV[0];
	} else if(refV < myData->mData.config.minV[0]) {
		refV = myData->mData.config.minV[0];
	}
	myTestCond->local_object[step][idx] = refV;

	idx = IDX_COM_OBJ_FAULT_UPPER_V; //kjhw_120517s
	l_val = myTestCond->common_object[idx];

	if(l_val == 0) {
		l_val = RANGE1 - 1;
	} else if(l_val <= myData->mData.config.maxV[3]) {
		l_val = RANGE4 - 1;
	} else if(l_val <= myData->mData.config.maxV[2]) {
		l_val = RANGE3 - 1;
	} else if(l_val <= myData->mData.config.maxV[1]) {
		l_val = RANGE2 - 1;
	} else {
		l_val = RANGE1 - 1;
	}

	if((l_val+1) > (long)myData->mData.config.rangeV) {
		l_val = (long)myData->mData.config.rangeV - 1;
	}

	idx = IDX_LOC_OBJ_RANGE_V2;
	myTestCond->local_object[step][idx] = l_val; //kjhw_120517e

	refI = myData->mData.config.maxI[0] * attr_count;

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
	idx = IDX_LOC_OBJ_RANGE_I;
	myTestCond->local_object[step][idx] = l_val;

	//end_condition
	idx = IDX_LOC_OBJ_END_P; //jhkw_141122
	myTestCond->local_object[step][idx] = P1_stepCond->reference[0].Power;
	if(myTestCond->local_object[step][idx] >= 0) {
		make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
			C_CD_END_P, 1, COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=
	} else { //<
		make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
			C_CD_END_P, 1, COMP_GREATER_THAN, idx); //>
	}

	idx = IDX_LOC_OBJ_END_AMPARE_HOUR;
	myTestCond->local_object[step][idx] = P1_stepCond->reference[0]
		.AmpareHour_SumAmpareHour;
	if(myTestCond->local_object[step][idx] >= 0) {
		make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
			C_CD_END_AMPARE_HOUR, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=
	} else {
		make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
			C_CD_END_AMPARE_HOUR, 1, COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=
	}

	idx = IDX_LOC_OBJ_END_AMPARE_HOUR_BRANCH;
	myTestCond->local_object[step][idx] = P1_stepCond->reference[0]
		.AmpareHour_Branch_MultiCycleCountId;
	if(myTestCond->local_object[step][idx] == ACTIVE_DIV_NONE)
		myTestCond->local_object[step][idx] = ACTIVE_DIV_NEXT;

	idx = IDX_LOC_OBJ_END_WATT_HOUR;
	myTestCond->local_object[step][idx] = P1_stepCond->reference[0]
		.WattHour_SumWattHour;
	if(myTestCond->local_object[step][idx] >= 0) {
		make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
			C_CD_END_WATT_HOUR, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=
	} else {
		make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
			C_CD_END_WATT_HOUR, 1, COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=
	}
			
	idx = IDX_LOC_OBJ_END_WATT_HOUR_BRANCH;
	myTestCond->local_object[step][idx] = P1_stepCond->reference[0]
		.WattHour_Branch_MultiCycleCount_Branch;
	if(myTestCond->local_object[step][idx] == ACTIVE_DIV_NONE)
		myTestCond->local_object[step][idx] = ACTIVE_DIV_NEXT;

	tmp = (long)P1_stepCond->reference[0].ValueRate_Item;
	if(tmp == P1_VALUE_RATE_ITEM_AMPARE_HOUR) {
		idx = IDX_LOC_OBJ_END_AMPARE_HOUR_RATE;
		myTestCond->local_object[step][idx] = P1_stepCond->reference[0]
			.ValueRate;
		make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
			C_CD_END_VALUE_RATE_AMPARE_HOUR, 1,
			COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

		idx = IDX_LOC_OBJ_END_AMPARE_HOUR_RATE_BRANCH;
		myTestCond->local_object[step][idx] = P1_stepCond->reference[0]
			.ValueRate_Branch;
		if(myTestCond->local_object[step][idx] == ACTIVE_DIV_NONE)
			myTestCond->local_object[step][idx] = ACTIVE_DIV_NEXT;

		idx = IDX_LOC_OBJ_END_AMPARE_HOUR_RATE_COMPARE;
		myTestCond->local_object[step][idx] = P1_stepCond->reference[0]
			.ValueRate_Compare;
	} else if(tmp == P1_VALUE_RATE_ITEM_WATT_HOUR) {
		idx = IDX_LOC_OBJ_END_WATT_HOUR_RATE;
		myTestCond->local_object[step][idx] = P1_stepCond->reference[0]
			.ValueRate;
		make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
			C_CD_END_VALUE_RATE_WATT_HOUR, 1,
			COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

		idx = IDX_LOC_OBJ_END_WATT_HOUR_RATE_BRANCH;
		myTestCond->local_object[step][idx] = P1_stepCond->reference[0]
			.ValueRate_Branch;
		if(myTestCond->local_object[step][idx] == ACTIVE_DIV_NONE)
			myTestCond->local_object[step][idx] = ACTIVE_DIV_NEXT;

		idx = IDX_LOC_OBJ_END_WATT_HOUR_RATE_COMPARE;
		myTestCond->local_object[step][idx] = P1_stepCond->reference[0]
			.ValueRate_Compare;
	}

	//fault_condition
	switch(myData->AppControl.config.systemModel) {
		//for motor inverter application
		case C_DAEWOO_450V_200A_10A_90KW:
		case C_DAEWOO_450V_200A_10A_90KW_2:
		case C_DAEWOO_450V_200A_50A_90KW:
		case C_DAEWOO_450V_200A_50A_90KW_2:	//jhk_150710
			fault_count = 50; //500ms delay
			break;
		default:
			fault_count = 1;
			break;
	}

	idx = IDX_LOC_OBJ_FAULT_HW_UPPER_V;
	myTestCond->local_object[step][idx] = 5000; //5.0%
	make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
		C_CD_FAULT_HW_UPPER_V, fault_count,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_LOC_OBJ_FAULT_HW_LOWER_V;
	myTestCond->local_object[step][idx] = 5000; //5.0%
	make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
		C_CD_FAULT_HW_LOWER_V, fault_count,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_LOC_OBJ_FAULT_HW_UPPER_I;
	myTestCond->local_object[step][idx] = 5000; //5.0%
	make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
		C_CD_FAULT_HW_UPPER_I, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_LOC_OBJ_FAULT_UPPER_I;
	myTestCond->local_object[step][idx] = P1_stepCond->faultUpperI;

	idx = IDX_LOC_OBJ_FAULT_LOWER_I;
	myTestCond->local_object[step][idx] = P1_stepCond->faultLowerI;

	idx = IDX_LOC_OBJ_FAULT_UPPER_AMPARE_HOUR;
	myTestCond->local_object[step][idx] = P1_stepCond->faultUpper_AmpareHour;
	make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
		C_CD_FAULT_UPPER_AMPARE_HOUR, 1,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_LOC_OBJ_FAULT_LOWER_AMPARE_HOUR;
	myTestCond->local_object[step][idx] = P1_stepCond->faultLower_AmpareHour;
	make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
		C_CD_FAULT_LOWER_AMPARE_HOUR, 1, COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=
#endif
	return 0;
}

void convert_test_cond_cable_check(int ch)
{ //kjg_101221_w

#if defined __COA_VER_100B__ || defined __COA_VER_100B2__ || defined __COA_VER_100C__
#else //COA_VER_100D~
	int idx2;
#endif
	int step, idx, i, fault_count;
	long attr_count, refV, refI, l_val, l_val2;



	//kjg_170810 memset((char *)&myData->testCond[ch], 0, sizeof(S_TEST_CONDITION));
	memset((char *)myTestCond, 0, sizeof(S_TEST_CONDITION));

	//cycle -> charge 1sec -> rest 1sec -> discharge 1sec -> rest 1sec -> loop -> end
	//common_header
	idx = IDX_COM_OBJ_TOTAL_STEP;
	myTestCond->common_object[idx] = 7;

	idx = IDX_COM_OBJ_ATTRIBUTE_COUNT;
	attr_count = 1;
	if(myData->ChAttribute[ch].opType != OP_INDEPENDENT) {
		for(i=0; i < 3; i++) {
			if(myData->ChAttribute[ch].chNo_slave[i] != 0) attr_count++;
		}
	}
	myTestCond->common_object[idx] = attr_count;

	//common_safety
	fault_count = 1;

	idx = IDX_COM_OBJ_FAULT_HW_UPPER_V;
	myTestCond->common_object[idx] = 5000; //5.0%
	make_com_chCode_cond((S_TEST_CONDITION *)myTestCond,
		C_CD_COM_FAULT_HW_UPPER_V, fault_count,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_COM_OBJ_FAULT_HW_LOWER_V;
	myTestCond->common_object[idx] = 5000; //5.0%
	make_com_chCode_cond((S_TEST_CONDITION *)myTestCond,
		C_CD_COM_FAULT_HW_LOWER_V, fault_count,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_COM_OBJ_FAULT_HW_UPPER_I;
	myTestCond->common_object[idx] = 5000; //5.0%
	make_com_chCode_cond((S_TEST_CONDITION *)myTestCond,
		C_CD_COM_FAULT_HW_UPPER_I, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_COM_OBJ_FAULT_HW_LOWER_I;
	myTestCond->common_object[idx] = 5000; //5.0%
	make_com_chCode_cond((S_TEST_CONDITION *)myTestCond,
		C_CD_COM_FAULT_HW_LOWER_I, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	//common_cycle_branch

	//step_condition
	//step_cycle
	step = 0;

	idx = IDX_LOC_OBJ_CLIENT_STEP_NO;
	myTestCond->local_object[step][idx] = step+1;

	idx = IDX_LOC_OBJ_STEP_NO;
	myTestCond->local_object[step][idx] = step;

	idx = IDX_LOC_OBJ_TYPE;
	myTestCond->local_object[step][idx] = STEP_CYCLE;

	idx = IDX_LOC_OBJ_ATTRIBUTE;
	myTestCond->local_object[step][idx] = ATTR_CHECK4_START;

	//step_charge
	step++;

	idx = IDX_LOC_OBJ_CLIENT_STEP_NO;
	myTestCond->local_object[step][idx] = step+1;

	idx = IDX_LOC_OBJ_STEP_NO;
	myTestCond->local_object[step][idx] = step;

	idx = IDX_LOC_OBJ_TYPE;
	myTestCond->local_object[step][idx] = STEP_CHARGE;

	idx = IDX_LOC_OBJ_ATTRIBUTE;
	myTestCond->local_object[step][idx] = ATTR_CHECK4_1;

	idx = IDX_LOC_OBJ_MODE;
	myTestCond->local_object[step][idx] = MODE_CC_CV;

#if defined __COA_VER_100B__ || defined __COA_VER_100B2__ || defined __COA_VER_100C__
	idx = IDX_LOC_OBJ_END_TIME;
	myTestCond->local_object[step][idx] = 100; //1sec
	make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
		C_CD_END_TIME, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=
#else //COA_VER_100D~
	idx2 = IDX_LOC_OBJ_END_TIME_DAY;
	myTestCond->local_object[step][idx2] = 0;
	idx = IDX_LOC_OBJ_END_TIME;
	myTestCond->local_object[step][idx] = 100; //1sec
	make_loc_chCode_cond2((S_TEST_CONDITION *)myTestCond, step,
		C_CD_END_TIME, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx, idx2); //>=
#endif

	idx = IDX_LOC_OBJ_END_TIME_BRANCH;
	myTestCond->local_object[step][idx] = ACTIVE_DIV_NEXT;

	idx = IDX_LOC_OBJ_SAVE_DELTA_T;
	myTestCond->local_object[step][idx] = 10; //0.1sec
	make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
		C_CD_SAVE_DELTA_T, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_LOC_OBJ_REF_V;
	//refV = 4200000; //4.2V
	if(myData->mData.config.ratioV == MICRO_UNIT) {	//uV //jhkw_231127s
		refV = 4200000; //4.2V
	} else if(myData->mData.config.ratioV == MILLI_UNIT) {	//mV
		refV = 4200; //4.2V
	} else {
		refV = 0; //0V
	} //jhkw_231127e
	myTestCond->local_object[step][idx] = refV;

	idx = IDX_LOC_OBJ_REF_V2;
	myTestCond->local_object[step][idx] = 0;

	idx = IDX_LOC_OBJ_RANGE_V;
	myTestCond->local_object[step][idx] = RANGE1 - 1;

	idx = IDX_LOC_OBJ_REF_I;
	//refI = 20000000; //20A
	if(myData->mData.config.ratioI == MICRO_UNIT) {	//uA //jhkw_231127s
		refI = 20000000; //20A
	} else if(myData->mData.config.ratioI == MILLI_UNIT) {	//mA
		refI = 20000; //20A
	} else {
		refI = 0; //0A
	} //jhkw_231127e
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

	//step_rest
	step++;

	idx = IDX_LOC_OBJ_CLIENT_STEP_NO;
	myTestCond->local_object[step][idx] = step+1;

	idx = IDX_LOC_OBJ_STEP_NO;
	myTestCond->local_object[step][idx] = step;

	idx = IDX_LOC_OBJ_TYPE;
	myTestCond->local_object[step][idx] = STEP_REST;

	idx = IDX_LOC_OBJ_ATTRIBUTE;
	myTestCond->local_object[step][idx] = ATTR_CHECK4_2;

#if defined __COA_VER_100B__ || defined __COA_VER_100B2__ || defined __COA_VER_100C__
	idx = IDX_LOC_OBJ_END_TIME;
	myTestCond->local_object[step][idx] = 100; //1sec
	make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
		C_CD_END_TIME, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=
#else //COA_VER_100D~
	idx2 = IDX_LOC_OBJ_END_TIME_DAY;
	myTestCond->local_object[step][idx2] = 0;
	idx = IDX_LOC_OBJ_END_TIME;
	myTestCond->local_object[step][idx] = 100; //1sec
	make_loc_chCode_cond2((S_TEST_CONDITION *)myTestCond, step,
		C_CD_END_TIME, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx, idx2); //>=
#endif

	idx = IDX_LOC_OBJ_END_TIME_BRANCH;
	myTestCond->local_object[step][idx] = ACTIVE_DIV_NEXT;

	idx = IDX_LOC_OBJ_SAVE_DELTA_T;
	myTestCond->local_object[step][idx] = 100; //1.0sec
	make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
		C_CD_SAVE_DELTA_T, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	//step_discharge
	step++;

	idx = IDX_LOC_OBJ_CLIENT_STEP_NO;
	myTestCond->local_object[step][idx] = step+1;

	idx = IDX_LOC_OBJ_STEP_NO;
	myTestCond->local_object[step][idx] = step;

	idx = IDX_LOC_OBJ_TYPE;
	myTestCond->local_object[step][idx] = STEP_DISCHARGE;

	idx = IDX_LOC_OBJ_ATTRIBUTE;
	myTestCond->local_object[step][idx] = ATTR_CHECK4_3;

	idx = IDX_LOC_OBJ_MODE;
	myTestCond->local_object[step][idx] = MODE_CC_CV;

#if defined __COA_VER_100B__ || defined __COA_VER_100B2__ || defined __COA_VER_100C__
	idx = IDX_LOC_OBJ_END_TIME;
	myTestCond->local_object[step][idx] = 100; //1sec
	make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
		C_CD_END_TIME, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=
#else //COA_VER_100D~
	idx2 = IDX_LOC_OBJ_END_TIME_DAY;
	myTestCond->local_object[step][idx2] = 0;
	idx = IDX_LOC_OBJ_END_TIME;
	myTestCond->local_object[step][idx] = 100; //1sec
	make_loc_chCode_cond2((S_TEST_CONDITION *)myTestCond, step,
		C_CD_END_TIME, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx, idx2); //>=
#endif

	idx = IDX_LOC_OBJ_END_TIME_BRANCH;
	myTestCond->local_object[step][idx] = ACTIVE_DIV_NEXT;

	idx = IDX_LOC_OBJ_SAVE_DELTA_T;
	myTestCond->local_object[step][idx] = 10; //0.1sec
	make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
		C_CD_SAVE_DELTA_T, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_LOC_OBJ_REF_V;
	//refV = 4200000; //4.2V
	if(myData->mData.config.ratioV == MICRO_UNIT) {	//uV //jhkw_231127s
		refV = 4200000; //4.2V
	} else if(myData->mData.config.ratioV == MILLI_UNIT) {	//mV
		refV = 4200; //4.2V
	} else {
		refV = 0; //0V
	} //jhkw_231127e
	myTestCond->local_object[step][idx] = refV;

	idx = IDX_LOC_OBJ_REF_V2;
	myTestCond->local_object[step][idx] = 0;

	idx = IDX_LOC_OBJ_RANGE_V;
	myTestCond->local_object[step][idx] = RANGE1 - 1;

	idx = IDX_LOC_OBJ_REF_I;
	//refI = -20000000; //-20A
	if(myData->mData.config.ratioI == MICRO_UNIT) {	//uA //jhkw_231127s
		refI = 20000000; //20A
	} else if(myData->mData.config.ratioI == MILLI_UNIT) {	//mA
		refI = 20000; //20A
	} else {
		refI = 0; //0A
	} //jhkw_231127e
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

	//step_rest
	step++;

	idx = IDX_LOC_OBJ_CLIENT_STEP_NO;
	myTestCond->local_object[step][idx] = step+1;

	idx = IDX_LOC_OBJ_STEP_NO;
	myTestCond->local_object[step][idx] = step;

	idx = IDX_LOC_OBJ_TYPE;
	myTestCond->local_object[step][idx] = STEP_REST;

	idx = IDX_LOC_OBJ_ATTRIBUTE;
	myTestCond->local_object[step][idx] = ATTR_CHECK4_4;

#if defined __COA_VER_100B__ || defined __COA_VER_100B2__ || defined __COA_VER_100C__
	idx = IDX_LOC_OBJ_END_TIME;
	myTestCond->local_object[step][idx] = 100; //1sec
	make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
		C_CD_END_TIME, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=
#else //COA_VER_100D~
	idx2 = IDX_LOC_OBJ_END_TIME_DAY;
	myTestCond->local_object[step][idx2] = 0;
	idx = IDX_LOC_OBJ_END_TIME;
	myTestCond->local_object[step][idx] = 100; //1sec
	make_loc_chCode_cond2((S_TEST_CONDITION *)myTestCond, step,
		C_CD_END_TIME, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx, idx2); //>=
#endif

	idx = IDX_LOC_OBJ_END_TIME_BRANCH;
	myTestCond->local_object[step][idx] = ACTIVE_DIV_NEXT;

	idx = IDX_LOC_OBJ_SAVE_DELTA_T;
	myTestCond->local_object[step][idx] = 100; //1.0sec
	make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
		C_CD_SAVE_DELTA_T, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	//step_loop
	step++;

	idx = IDX_LOC_OBJ_CLIENT_STEP_NO;
	myTestCond->local_object[step][idx] = step+1;

	idx = IDX_LOC_OBJ_STEP_NO;
	myTestCond->local_object[step][idx] = step;

	idx = IDX_LOC_OBJ_TYPE;
	myTestCond->local_object[step][idx] = STEP_LOOP;

	idx = IDX_LOC_OBJ_ATTRIBUTE;
	myTestCond->local_object[step][idx] = ATTR_CHECK4_5;

	idx = IDX_LOC_OBJ_END_CYCLE_COUNT;
	myTestCond->local_object[step][idx] = 1;

	idx = IDX_LOC_OBJ_END_CYCLE_COUNT_BRANCH;
	myTestCond->local_object[step][idx] = ACTIVE_DIV_NEXT;

	//step_end
	step++;

	idx = IDX_LOC_OBJ_CLIENT_STEP_NO;
	myTestCond->local_object[step][idx] = step+1;

	idx = IDX_LOC_OBJ_STEP_NO;
	myTestCond->local_object[step][idx] = step;

	idx = IDX_LOC_OBJ_TYPE;
	myTestCond->local_object[step][idx] = STEP_END;

	idx = IDX_LOC_OBJ_ATTRIBUTE;
	myTestCond->local_object[step][idx] = ATTR_CHECK4_END;
}

int convert_test_cond_step_update(int ch)
{ //kjg_170810
	int rtn, idx, i, stepNo;
	long attr_count, type;
	S_P1_TEST_COND_STEP *P1_stepCond;

	stepNo = myPs->step_cond_update.stepNo;

	idx = IDX_COM_OBJ_TOTAL_STEP;
	i = myData->testCond[ch].common_object[idx];
	if(stepNo < 0 || stepNo >= i) {
		userlog(DEBUG_LOG, psName,
			"convert_test_cond_step_update error1 %d, %d, %d\n", ch, stepNo, i);
		return (-1);
	}

	//myTestCond Assign
	//kjg_171219 myTestCond = (S_TEST_CONDITION *)&(myData->testCond_update);
	myTestCond = &(myData->testCond_update);

	memset((char *)myTestCond, 0, sizeof(S_TEST_CONDITION));

	attr_count = 1;
	if(myData->ChAttribute[ch].opType != OP_INDEPENDENT) {
		for(i=0; i < 3; i++) {
			if(myData->ChAttribute[ch].chNo_slave[i] != 0) attr_count++;
		}
	}

	P1_stepCond
		= (S_P1_TEST_COND_STEP *)&(myPs->step_cond_update.testCond_step);

	//step header
	idx = IDX_LOC_OBJ_CLIENT_STEP_NO;
	myTestCond->local_object[stepNo][idx] = (long)P1_stepCond->header.stepNo;

	idx = IDX_LOC_OBJ_STEP_NO;
	myTestCond->local_object[stepNo][idx] = (long)P1_stepCond->header.stepNo;

	idx = IDX_LOC_OBJ_TYPE;
	type = convert_step_type(CONVERT_P1_TO_ORG, (long)P1_stepCond->header.type);
	myTestCond->local_object[stepNo][idx] = type;

	rtn = 0;
	switch(type) {
		case STEP_IDLE:
		case STEP_END:
			userlog(DEBUG_LOG, psName,
				"convert_test_cond_step_update error2 %d, %d, %d\n",
				ch, stepNo, type);
			rtn = (-2);
			break;
		case STEP_CYCLE:
			convert_test_cond_cycle(P1_stepCond, ch, stepNo, type, attr_count);
			break;
		case STEP_LOOP:
			convert_test_cond_loop(P1_stepCond, ch, stepNo, type, attr_count);
			break;
		default:
			rtn = convert_test_cond_step_default(P1_stepCond, ch, stepNo, type,
				attr_count, 1);
			if(rtn < 0) return (rtn - 10);
			break;
	}

	//jhkw_191219s
	idx = IDX_LOC_OBJ_RANGE_V;
	myTestCond->local_object[stepNo][idx] = myData->testCond[ch].local_object[0][idx];
	idx = IDX_LOC_OBJ_RANGE_V2;
	myTestCond->local_object[stepNo][idx] = myData->testCond[ch].local_object[0][idx];
	//jhkw_191219e
	return rtn;
}

int convert_test_cond_common_safety_update(int ch)
{ //kjg_170810
	int rtn=0, idx, i, fault_count;
	long attr_count;

	//myTestCond Assign
	//kjg_171219 myTestCond = (S_TEST_CONDITION *)&(myData->testCond_update);
	myTestCond = &(myData->testCond_update);

	memset((char *)myTestCond, 0, sizeof(S_TEST_CONDITION));

	//common_header
	idx = IDX_COM_OBJ_TOTAL_STEP;
	myTestCond->common_object[idx] = (long)myPs->common_safety_cond_update
		.testCond_header.totalStep;

#if defined __COA_VER_100B__ || defined __COA_VER_100B2__ || defined __COA_VER_100C__
#else //COA_VER_100D~
	idx = IDX_COM_OBJ_TOTAL_PATTERN;
	myTestCond->common_object[idx] = (long)myPs->common_safety_cond_update
		.testCond_header.totalPatternCount;
#endif

	idx = IDX_COM_OBJ_ATTRIBUTE_COUNT;
	attr_count = 1;
	if(myData->ChAttribute[ch].opType != OP_INDEPENDENT) {
		for(i=0; i < 3; i++) {
			if(myData->ChAttribute[ch].chNo_slave[i] != 0) attr_count++;
		}
	}
	myTestCond->common_object[idx] = attr_count;

	//common_safety
	switch(myData->AppControl.config.systemModel) {
		//for motor inverter application
		case C_DAEWOO_450V_200A_10A_90KW:
		case C_DAEWOO_450V_200A_10A_90KW_2:
		case C_DAEWOO_450V_200A_50A_90KW:
		case C_DAEWOO_450V_200A_50A_90KW_2: //jhk_150710
			fault_count = 50; //500ms delay
			break;
		default:
			fault_count = 1;
			break;
	}

	idx = IDX_COM_OBJ_FAULT_HW_UPPER_V;
	myTestCond->common_object[idx] = 5000; //5.0%
	make_com_chCode_cond((S_TEST_CONDITION *)myTestCond,
		C_CD_COM_FAULT_HW_UPPER_V, fault_count,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_COM_OBJ_FAULT_HW_LOWER_V;
	myTestCond->common_object[idx] = 5000; //5.0%
	make_com_chCode_cond((S_TEST_CONDITION *)myTestCond,
		C_CD_COM_FAULT_HW_LOWER_V, fault_count,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_COM_OBJ_FAULT_HW_UPPER_I;
	myTestCond->common_object[idx] = 5000; //5.0%
	make_com_chCode_cond((S_TEST_CONDITION *)myTestCond,
		C_CD_COM_FAULT_HW_UPPER_I, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_COM_OBJ_FAULT_HW_LOWER_I;
	//myData->testCond[ch].common_object[idx] = 5000; //5.0% //kjh_171214
	myTestCond->common_object[idx] = 5000; //5.0%
	make_com_chCode_cond((S_TEST_CONDITION *)myTestCond,
		C_CD_COM_FAULT_HW_LOWER_I, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_COM_OBJ_FAULT_UPPER_V;
	myTestCond->common_object[idx] = myPs->common_safety_cond_update
		.testCond_safety.faultUpperV;
	make_com_chCode_cond((S_TEST_CONDITION *)myTestCond,
		C_CD_COM_FAULT_UPPER_V, fault_count,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_COM_OBJ_FAULT_LOWER_V;
	myTestCond->common_object[idx] = myPs->common_safety_cond_update
		.testCond_safety.faultLowerV;
	make_com_chCode_cond((S_TEST_CONDITION *)myTestCond,
		C_CD_COM_FAULT_LOWER_V, fault_count,
		COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=

#if defined __COA_VER_100B__ || defined __COA_VER_100B2__ || defined __COA_VER_100C__
	idx = IDX_COM_OBJ_FAULT_UPPER_I;
	myTestCond->common_object[idx] = myPs->common_safety_cond_update
		.testCond_safety.faultUpperI;
	make_com_chCode_cond((S_TEST_CONDITION *)myTestCond,
		C_CD_COM_FAULT_UPPER_I, 3, COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=
		//kjg_140206 1 -> 3
#else //COA_VER_100D~
	idx = IDX_COM_OBJ_FAULT_UPPER_I;
	myTestCond->common_object[idx] = myPs->common_safety_cond_update
		.testCond_safety.faultUpperI;
	make_com_chCode_cond((S_TEST_CONDITION *)myTestCond,
		C_CD_COM_FAULT_UPPER_I, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=
#endif

	idx = IDX_COM_OBJ_FAULT_UPPER_P;
	myTestCond->common_object[idx] = myPs->common_safety_cond_update
		.testCond_safety.faultUpperP;
	make_com_chCode_cond((S_TEST_CONDITION *)myTestCond,
		C_CD_COM_FAULT_UPPER_P, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_COM_OBJ_FAULT_UPPER_WATT_HOUR;
	myTestCond->common_object[idx] = myPs->common_safety_cond_update
		.testCond_safety.faultUpper_WattHour;
	make_com_chCode_cond((S_TEST_CONDITION *)myTestCond,
		C_CD_COM_FAULT_UPPER_WATT_HOUR, 1,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_COM_OBJ_FAULT_UPPER_AMPARE_HOUR;
	myTestCond->common_object[idx] = myPs->common_safety_cond_update
		.testCond_safety.faultUpper_AmpareHour;
	make_com_chCode_cond((S_TEST_CONDITION *)myTestCond,
		C_CD_COM_FAULT_UPPER_AMPARE_HOUR, 1,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_COM_OBJ_FAULT_UPPER_TEMP;
	myTestCond->common_object[idx] = myPs->common_safety_cond_update
		.testCond_safety.faultUpperTemp;
	make_com_chCode_cond((S_TEST_CONDITION *)myTestCond,
		C_CD_COM_FAULT_UPPER_TEMP, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_COM_OBJ_FAULT_LOWER_TEMP;
	myTestCond->common_object[idx] = myPs->common_safety_cond_update
		.testCond_safety.faultLowerTemp;
	make_com_chCode_cond((S_TEST_CONDITION *)myTestCond,
		C_CD_COM_FAULT_LOWER_TEMP, 1, COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=

#if defined __COA_VER_100B__ || defined __COA_VER_100B2__ || defined __COA_VER_100C__
#else //COA_VER_100D~
	idx = IDX_COM_OBJ_FAULT_COMP_AUX_V; //kjhw_150730
	myTestCond->common_object[idx] = myPs->common_safety_cond_update
		.testCond_safety.faultCompAuxV;
	make_com_chCode_cond((S_TEST_CONDITION *)myTestCond,
		C_CD_COM_FAULT_COMP_AUX_V, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_COM_OBJ_FAULT_DELTA_V; //jhkw_160828
	myTestCond->common_object[idx] = 10000; //10.0%
	make_com_chCode_cond((S_TEST_CONDITION *)myTestCond,
		C_CD_COM_FAULT_DELTA_V, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=
		//1s delay
#endif
	//jhkw_191108s
	idx = IDX_COM_OBJ_CURRENT_AH;
	myTestCond->common_object[idx] = myPs->common_safety_cond_update
		.testCond_safety.current_AH; //uAh

	idx = IDX_COM_OBJ_INITIAL_AH;
	myTestCond->common_object[idx] = myPs->common_safety_cond_update
		.testCond_safety.initial_AH; //uAh

	idx = IDX_COM_OBJ_CRATE_FACTOR;
	myTestCond->common_object[idx] = myPs->common_safety_cond_update
		.testCond_safety.crate_factor; //uAh
	//jhkw_191108e
	//jhkw_200507s
	idx = IDX_COM_OBJ_FAULT_FIX_UPPER_AUXV;
	myTestCond->common_object[idx] = myPs->common_safety_cond_update
		.testCond_safety.faultUpper_AuxV;
	userlog(DEBUG_LOG, psName,"update_FIX_UPPER_AUXV %ld\n", myTestCond->common_object[idx]);
//	make_com_chCode_cond((S_TEST_CONDITION *)myTestCond,
//		C_CD_COM_FAULT_FIX_UPPER_AUXV, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_COM_OBJ_FAULT_FIX_LOWER_AUXV;
	myTestCond->common_object[idx] = myPs->common_safety_cond_update
		.testCond_safety.faultLower_AuxV;
	userlog(DEBUG_LOG, psName,"update_FIX_LOWER_AUXV %ld\n", myTestCond->common_object[idx]);
//	make_com_chCode_cond((S_TEST_CONDITION *)myTestCond,
//		C_CD_COM_FAULT_FIX_LOWER_AUXV, 1, COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=

	idx = IDX_COM_OBJ_FAULT_FIX_UPPER_TEMP;
	myTestCond->common_object[idx] = myPs->common_safety_cond_update
		.testCond_safety.faultUpper_Temp;
	userlog(DEBUG_LOG, psName,"update_FIX_UPPER_TEMP %ld\n", myTestCond->common_object[idx]);
	//jhkw_200507e
	
	//shhw_230605s
	idx = IDX_COM_OBJ_FAULT_FIX_DELTA_I;
	myTestCond->common_object[idx] = myPs->common_safety_cond_update
		.testCond_safety.faultDelta_I;
	userlog(DEBUG_LOG, psName,"update_FIX_DELTA_I %ld\n", myTestCond->common_object[idx]);
	
	idx = IDX_COM_OBJ_FAULT_FIX_DELTA_AUXV;
	myTestCond->common_object[idx] = myPs->common_safety_cond_update
		.testCond_safety.faultDelta_AuxV;
	userlog(DEBUG_LOG, psName,"update_FIX_DELTA_AUXV %ld\n", myTestCond->common_object[idx]);
	//shhw_230605e

	//common_cycle_branch
	
	//common_aux_condition
	convert_test_cond_common_aux((S_P1_TEST_COND_SAFETY *)&(
		myPs->common_safety_cond_update.testCond_safety), ch);

	//common_can_condition
	convert_test_cond_common_can((S_P1_TEST_COND_SAFETY *)&(
		myPs->common_safety_cond_update.testCond_safety), ch);

	return rtn;
}

void convert_test_cond_module_coa_common_safety(int ch)
{ //kjg_170810
	int idx, i;

	memset((char *)&myPs->common_safety_cond_update.testCond_header, 0,
		sizeof(S_P1_TEST_COND_HEADER));

	idx = IDX_COM_OBJ_TOTAL_STEP;
	myPs->common_safety_cond_update.testCond_header.totalStep
		= (short int)myData->testCond[ch].common_object[idx];

	idx = IDX_COM_OBJ_TOTAL_PATTERN;
	myPs->common_safety_cond_update.testCond_header.totalPatternCount
		= (short int)myData->testCond[ch].common_object[idx];

	myPs->common_safety_cond_update.testCond_header.totalTimeSchCount
		= myPs->misc.rcv_test_timesch_count;

	memset((char *)&myPs->common_safety_cond_update.testCond_safety, 0,
		sizeof(S_P1_TEST_COND_SAFETY));

	idx = IDX_COM_OBJ_FAULT_LOWER_V;
	myPs->common_safety_cond_update.testCond_safety.faultLowerV
		= myData->testCond[ch].common_object[idx];

	idx = IDX_COM_OBJ_FAULT_UPPER_V;
	myPs->common_safety_cond_update.testCond_safety.faultUpperV
		= myData->testCond[ch].common_object[idx];

	idx = IDX_COM_OBJ_FAULT_COMP_AUX_V;
	myPs->common_safety_cond_update.testCond_safety.faultCompAuxV
		= myData->testCond[ch].common_object[idx];

	idx = IDX_COM_OBJ_FAULT_UPPER_I;
	myPs->common_safety_cond_update.testCond_safety.faultUpperI
		= myData->testCond[ch].common_object[idx];

	idx = IDX_COM_OBJ_FAULT_LOWER_TEMP;
	myPs->common_safety_cond_update.testCond_safety.faultLowerTemp
		= myData->testCond[ch].common_object[idx];

	idx = IDX_COM_OBJ_FAULT_UPPER_TEMP;
	myPs->common_safety_cond_update.testCond_safety.faultUpperTemp
		= myData->testCond[ch].common_object[idx];

	idx = IDX_COM_OBJ_FAULT_UPPER_AMPARE_HOUR;
	myPs->common_safety_cond_update.testCond_safety.faultUpper_AmpareHour
		= myData->testCond[ch].common_object[idx];

	idx = IDX_COM_OBJ_FAULT_UPPER_P;
	myPs->common_safety_cond_update.testCond_safety.faultUpperP
		= myData->testCond[ch].common_object[idx];

	idx = IDX_COM_OBJ_FAULT_UPPER_WATT_HOUR;
	myPs->common_safety_cond_update.testCond_safety.faultUpper_WattHour
		= myData->testCond[ch].common_object[idx];
	//jhkw_191108s
	idx = IDX_COM_OBJ_CURRENT_AH;
	myPs->common_safety_cond_update.testCond_safety.current_AH
		= myData->testCond[ch].common_object[idx]; //uAh

	idx = IDX_COM_OBJ_INITIAL_AH;
	myPs->common_safety_cond_update.testCond_safety.initial_AH
		= myData->testCond[ch].common_object[idx]; //uAh

	idx = IDX_COM_OBJ_CRATE_FACTOR;
	myPs->common_safety_cond_update.testCond_safety.crate_factor
		= myData->testCond[ch].common_object[idx]; //uAh
	//jhkw_191108e
	//jhkw_200507s
	idx = IDX_COM_OBJ_FAULT_FIX_UPPER_AUXV;
	myPs->common_safety_cond_update.testCond_safety.faultUpper_AuxV
		= myData->testCond[ch].common_object[idx];
	userlog(DEBUG_LOG, psName,"PS-update_FIX_UPPER_AUXV %ld\n",
		myPs->common_safety_cond_update.testCond_safety.faultUpper_AuxV);
	
	idx = IDX_COM_OBJ_FAULT_FIX_LOWER_AUXV;
	myPs->common_safety_cond_update.testCond_safety.faultLower_AuxV
		= myData->testCond[ch].common_object[idx];
	userlog(DEBUG_LOG, psName,"PS-update_FIX_LOWER_AUXV %ld\n",
		myPs->common_safety_cond_update.testCond_safety.faultLower_AuxV);
	
	idx = IDX_COM_OBJ_FAULT_FIX_UPPER_TEMP;
	myPs->common_safety_cond_update.testCond_safety.faultUpper_Temp
		= myData->testCond[ch].common_object[idx];
	userlog(DEBUG_LOG, psName,"PS-update_FIX_UPPER_TEMP %ld\n",
		myPs->common_safety_cond_update.testCond_safety.faultUpper_Temp);
	//jhkw_200507e
	
	//shhw_230605s
	idx = IDX_COM_OBJ_FAULT_FIX_DELTA_I;
	myPs->common_safety_cond_update.testCond_safety.faultDelta_I
		= myData->testCond[ch].common_object[idx];
	userlog(DEBUG_LOG, psName,"PS-update_FIX_DELTA_I %ld\n",
		myPs->common_safety_cond_update.testCond_safety.faultDelta_I);
	
	idx = IDX_COM_OBJ_FAULT_FIX_DELTA_AUXV;
	myPs->common_safety_cond_update.testCond_safety.faultDelta_AuxV 
		= myData->testCond[ch].common_object[idx];
	userlog(DEBUG_LOG, psName,"PS-updata_FIX_DELTA_AUXV %ld\n", 
		myPs->common_safety_cond_update.testCond_safety.faultDelta_AuxV);
	//shhw_230605e

	for(i=0; i < MAX_P1_CAN_FUNCTION; i++) {
		idx = IDX_COM_OBJ_CAN_FUNC_DIV_TMP_1 + i;
		myPs->common_safety_cond_update.testCond_safety.can_func_div[i]
			= (short int)myData->testCond[ch].common_object[idx];

		idx = IDX_COM_OBJ_CAN_COMP_TYPE_TMP_1 + i;
		myPs->common_safety_cond_update.testCond_safety.can_compare_type[i]
			= (unsigned char)myData->testCond[ch].can_chCode[idx].compType;

		idx = IDX_COM_OBJ_CAN_VALUE_TYPE_1 + i;
		myPs->common_safety_cond_update.testCond_safety.can_data_type[i]
			= (unsigned char)myData->testCond[ch].common_object[idx];

		idx = IDX_COM_OBJ_CAN_VALUE_TMP_1 + i;
		myPs->common_safety_cond_update.testCond_safety.can_value[i]
			= (float)myData->testCond[ch].common_object[idx] / 1000.0;
	}

	for(i=0; i < MAX_P1_AUX_FUNCTION; i++) {
		idx = IDX_COM_OBJ_AUX_FUNC_DIV_TMP_1 + i;
		myPs->common_safety_cond_update.testCond_safety.aux_func_div[i]
			= (short int)myData->testCond[ch].common_object[idx];

		idx = IDX_COM_OBJ_AUX_COMP_TYPE_TMP_1 + i;
		myPs->common_safety_cond_update.testCond_safety.aux_compare_type[i]
			= (unsigned char)myData->testCond[ch].aux_chCode[idx].compType;

		idx = IDX_COM_OBJ_AUX_VALUE_TYPE_1 + i;
		myPs->common_safety_cond_update.testCond_safety.aux_data_type[i]
			= (unsigned char)myData->testCond[ch].common_object[idx];

		idx = IDX_COM_OBJ_AUX_VALUE_TMP_1 + i;
		myPs->common_safety_cond_update.testCond_safety.aux_value[i]
			= (float)myData->testCond[ch].common_object[idx] / 1000.0;
	}
}

void convert_test_cond_module_coa_step(int ch, int stepNo)
{ //kjg_170810
	int idx, i;
	unsigned int delay_time; //kjhw_170828
	long type, tmp;
	S_P1_TEST_COND_STEP *P1_stepCond;

	P1_stepCond
		= (S_P1_TEST_COND_STEP *)&(myPs->step_cond_update.testCond_step);

	memset((char *)P1_stepCond, 0, sizeof(S_P1_TEST_COND_STEP));

	//header
	idx = IDX_LOC_OBJ_TYPE;
	type = myData->testCond[ch].local_object[stepNo][idx];
	P1_stepCond->header.type = (int)convert_step_type(CONVERT_ORG_TO_P1, type);

	idx = IDX_LOC_OBJ_CLIENT_STEP_NO;
	tmp = myData->testCond[ch].local_object[stepNo][idx];
	P1_stepCond->header.stepNo = (unsigned char)tmp;

	switch(type) {
		case STEP_IDLE:
		case STEP_END:
			break;
		case STEP_CYCLE:
			//reference
			idx = IDX_LOC_OBJ_END_MULTI_CYCLE_COUNT_COMPARE;
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			P1_stepCond->reference[0].AmpareHour_Branch_MultiCycleCountId
				//= (unsigned char)tmp;
				= (short int)tmp;	//jhkw_200410

			idx = IDX_LOC_OBJ_END_SUM_AMPARE_HOUR;
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			P1_stepCond->reference[0].AmpareHour_SumAmpareHour = tmp;

			idx = IDX_LOC_OBJ_END_SUM_AMPARE_HOUR_TYPE;
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			P1_stepCond->reference[0].rangeI = (unsigned char)tmp;

			idx = IDX_LOC_OBJ_END_SUM_WATT_HOUR;
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			P1_stepCond->reference[0].WattHour_SumWattHour = tmp;

			idx = IDX_LOC_OBJ_END_SUM_WATT_HOUR_TYPE;
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			//jhkw_200410
			//P1_stepCond->reference[0].ValueRate_Compare = (unsigned char)tmp;
			P1_stepCond->reference[0].ValueRate_Compare = (short int)tmp;

			idx = IDX_LOC_OBJ_END_SUM_TIME_DAY;
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			P1_stepCond->reference[0].endTime_day = (unsigned long)tmp;

			idx = IDX_LOC_OBJ_END_SUM_TIME;
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			P1_stepCond->reference[0].endTime = (unsigned long)tmp;
			break;
		case STEP_LOOP:
			//header
			idx = IDX_LOC_OBJ_CYCLE_PAUSE;
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			P1_stepCond->header.cycle_pause = (unsigned char)tmp;

			idx = IDX_LOC_OBJ_END_CYCLE_COUNT;
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			P1_stepCond->reference[0].V_Lower_Branch_CycleCount = tmp;

			idx = IDX_LOC_OBJ_END_CYCLE_COUNT_BRANCH;
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			P1_stepCond->reference[0].V_Upper_Branch_CycleCount_Branch = tmp;

			idx = IDX_LOC_OBJ_END_MULTI_CYCLE_COUNT;
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			P1_stepCond->reference[0].V_Lower_MultiCycleCount = tmp;

			idx = IDX_LOC_OBJ_END_MULTI_CYCLE_COUNT_BRANCH;
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			P1_stepCond->reference[0].WattHour_Branch_MultiCycleCount_Branch
				//= (unsigned char)tmp;
				= (short int)tmp;	//jhkw_200410

			idx = IDX_LOC_OBJ_END_MULTI_CYCLE_COUNT_COMPARE;
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			P1_stepCond->reference[0].AmpareHour_Branch_MultiCycleCountId
				//= (unsigned char)tmp;
				= (short int)tmp;	//jhkw_200410

			idx = IDX_LOC_OBJ_END_ACC_CYCLE_COUNT;
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			P1_stepCond->reference[0].Time_Branch_AccCycleCount = tmp;

			idx = IDX_LOC_OBJ_END_ACC_CYCLE_COUNT_BRANCH;
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			//jhkw_200410
			//P1_stepCond->reference[0].AccCycleCount_Branch = (unsigned char)tmp;
			P1_stepCond->reference[0].AccCycleCount_Branch = (short int)tmp;

			idx = IDX_LOC_OBJ_END_ACC_CYCLE_COUNT_COMPARE;
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			P1_stepCond->reference[0].CVTime_Branch_AccCycleCountId = tmp;
			break;
		default:
			//reference
			idx = IDX_LOC_OBJ_END_TIME_DAY;
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			P1_stepCond->reference[0].endTime_day = (unsigned long)tmp;

			idx = IDX_LOC_OBJ_END_TIME;
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			P1_stepCond->reference[0].endTime = (unsigned long)tmp;

			idx = IDX_LOC_OBJ_END_TIME_BRANCH;
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			P1_stepCond->reference[0].Time_Branch_AccCycleCount = tmp;

			idx = IDX_LOC_OBJ_END_CV_TIME_DAY;
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			P1_stepCond->reference[0].CVTime_day = (unsigned long)tmp;

			idx = IDX_LOC_OBJ_END_CV_TIME;
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			P1_stepCond->reference[0].CVTime = (unsigned long)tmp;

			idx = IDX_LOC_OBJ_END_CV_TIME_BRANCH;
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			P1_stepCond->reference[0].CVTime_Branch_AccCycleCountId = tmp;

			idx = IDX_LOC_OBJ_END_V_UPPER;
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			P1_stepCond->reference[0].V_Upper = tmp;

			idx = IDX_LOC_OBJ_END_V_UPPER_BRANCH;
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			P1_stepCond->reference[0].V_Upper_Branch_CycleCount_Branch = tmp;

			idx = IDX_LOC_OBJ_END_V_LOWER;
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			P1_stepCond->reference[0].V_Lower_MultiCycleCount = tmp;

			idx = IDX_LOC_OBJ_END_V_LOWER_BRANCH;
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			P1_stepCond->reference[0].V_Lower_Branch_CycleCount = tmp;

			idx = IDX_LOC_OBJ_CHAMBER_STEP_CHECK_FLAG;
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			P1_stepCond->reference[0].chamber_step_check_flag
				= (unsigned char)tmp;

			//etc
			idx = IDX_LOC_OBJ_CAN_COMM_STEP_CHECK_FLAG;
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			P1_stepCond->can_comm_step_check_flag = (unsigned char)tmp;

			idx = IDX_LOC_OBJ_FAULT_CAN_CHECK_FLAG;
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			P1_stepCond->fault_can_check_flag = (unsigned char)tmp;

			idx = IDX_LOC_OBJ_FAULT_CAN_TX_STOP;
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			P1_stepCond->can_tx_stop = (unsigned char)tmp;

			//aux
			for(i=0; i < MAX_P1_AUX_FUNCTION; i++) {
				idx = IDX_LOC_OBJ_AUX_FUNC_DIV_TMP_1 + i;
				P1_stepCond->reference[0].aux_func_div[i]
					= (short int)myData->testCond[ch].local_object[stepNo][idx];

				idx = IDX_LOC_OBJ_AUX_COMP_TYPE_TMP_1 + i;
				P1_stepCond->reference[0].aux_compare_type[i]
					= (unsigned char)myData->testCond[ch]
					.local_object[stepNo][idx];

				idx = IDX_LOC_OBJ_AUX_VALUE_TYPE_1 + i;
				P1_stepCond->reference[0].aux_data_type[i]
					= (unsigned char)myData->testCond[ch]
					.local_object[stepNo][idx];

				idx = IDX_LOC_OBJ_AUX_BRANCH_TMP_1 + i;
				P1_stepCond->reference[0].aux_branch[i]
					= (short int)myData->testCond[ch].local_object[stepNo][idx];

				idx = IDX_LOC_OBJ_AUX_VALUE_TMP_1 + i;
				P1_stepCond->reference[0].aux_value[i]
					= (float)myData->testCond[ch].local_object[stepNo][idx]
					/ 1000.0;
				idx = IDX_LOC_OBJ_AUX_DELAY_TIME_1 + i; //kjhw_170828
				delay_time = (unsigned int)myData->testCond[ch]
					.local_object[stepNo][idx] / 100;
				if(delay_time == 1) delay_time = 0;
				P1_stepCond->reference[0].aux_delay_time[i] = delay_time;
			}
			
			//can
			for(i=0; i < MAX_P1_CAN_FUNCTION; i++) {
				idx = IDX_LOC_OBJ_CAN_FUNC_DIV_TMP_1 + i;
				P1_stepCond->reference[0].can_func_div[i]
					= (short int)myData->testCond[ch].local_object[stepNo][idx];

				idx = IDX_LOC_OBJ_CAN_COMP_TYPE_TMP_1 + i;
				P1_stepCond->reference[0].can_compare_type[i]
					= (unsigned char)myData->testCond[ch]
					.local_object[stepNo][idx];

				idx = IDX_LOC_OBJ_CAN_VALUE_TYPE_1 + i;
				P1_stepCond->reference[0].can_data_type[i]
					= (unsigned char)myData->testCond[ch]
					.local_object[stepNo][idx];

				idx = IDX_LOC_OBJ_CAN_BRANCH_TMP_1 + i;
				P1_stepCond->reference[0].can_branch[i]
					= (short int)myData->testCond[ch].local_object[stepNo][idx];

				idx = IDX_LOC_OBJ_CAN_VALUE_TMP_1 + i;
				P1_stepCond->reference[0].can_value[i]
					= (float)myData->testCond[ch].local_object[stepNo][idx]
					/ 1000.0;
			}
			
			//record
			idx = IDX_LOC_OBJ_SAVE_DELTA_T;
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			P1_stepCond->record.time = (unsigned long)tmp;
			
			idx = IDX_LOC_OBJ_SAVE_DELTA_V;
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			P1_stepCond->record.deltaV = tmp;
			
			idx = IDX_LOC_OBJ_SAVE_DELTA_I;
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			P1_stepCond->record.deltaI = tmp;
			
			idx = IDX_LOC_OBJ_SAVE_DELTA_TEMP;
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			P1_stepCond->record.deltaT = tmp;
			
			idx = IDX_LOC_OBJ_SAVE_DELTA_P;
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			P1_stepCond->record.deltaP = tmp;
			
			switch(myData->AppControl.config.systemModel) {
				case C_SDI_70V_50A_5A_4KW:
					idx = IDX_LOC_OBJ_SAVE_RECORD_T1;
					tmp = myData->testCond[ch].local_object[stepNo][idx];
					P1_stepCond->edlc.startT_Z = (long)tmp;
			
					idx = IDX_LOC_OBJ_SAVE_RECORD_T2;
					tmp = myData->testCond[ch].local_object[stepNo][idx];
					P1_stepCond->edlc.endT_Z = (long)tmp;
			
					idx = IDX_LOC_OBJ_SAVE_RECORD_T3;
					tmp = myData->testCond[ch].local_object[stepNo][idx];
					P1_stepCond->edlc.startT_LC = (long)tmp;
					break;
				default:
					break;
			}
			
			//fault
			idx = IDX_LOC_OBJ_FAULT_UPPER_V;
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			P1_stepCond->faultUpperV = tmp;

			idx = IDX_LOC_OBJ_FAULT_LOWER_V;
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			P1_stepCond->faultLowerV = tmp;

			//idx = IDX_LOC_OBJ_FAULT_UPPER_TEMP;
			//tmp = myData->testCond[ch].local_object[stepNo][idx];
			//P1_stepCond->faultUpperTemp = tmp;

			//idx = IDX_LOC_OBJ_FAULT_LOWER_TEMP;
			//tmp = myData->testCond[ch].local_object[stepNo][idx];
			//P1_stepCond->faultLowerTemp = tmp;
			
			idx = IDX_LOC_OBJ_LIMIT_CURRENT_UPPER;
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			P1_stepCond->faultUpperTemp = tmp;

			idx = IDX_LOC_OBJ_LIMIT_CURRENT_LOWER;
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			P1_stepCond->faultLowerTemp = tmp;
			
			//jhkw_200524s
			idx = IDX_LOC_OBJ_LIMIT_CURRENT_UPPER_FLAG;
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			P1_stepCond->crate_flag = tmp;
			//jhkw_200524e
			//grade
			//kjg_170810_w
			break;
	}

	switch(type) {
		case STEP_OCV:
		case STEP_REST:
			break;
		case STEP_CHARGE:
		case STEP_DISCHARGE:
		case STEP_Z:
			//header
			idx = IDX_LOC_OBJ_MODE;
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			P1_stepCond->header.mode
				= (unsigned char)convert_step_mode(CONVERT_ORG_TO_P1, tmp);

			//reference
			idx = IDX_LOC_OBJ_REF_V;
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			P1_stepCond->reference[0].refV_upper = tmp;

			idx = IDX_LOC_OBJ_REF_V2;
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			P1_stepCond->reference[0].refV_lower = tmp;

			idx = IDX_LOC_OBJ_REF_I;
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			if(tmp < 0) tmp *= (-1);
			P1_stepCond->reference[0].refI = tmp;

			idx = IDX_LOC_OBJ_REF_P;
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			if(tmp < 0) tmp *= (-1);
			P1_stepCond->reference[0].refP = tmp;

			idx = IDX_LOC_OBJ_REF_R;
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			if(tmp < 0) tmp *= (-1);
			P1_stepCond->reference[0].refR = tmp;

			idx = IDX_LOC_OBJ_RANGE_I;
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			P1_stepCond->reference[0].rangeI = (unsigned char)tmp;

			//end
			idx = IDX_LOC_OBJ_END_AMPARE_HOUR;
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			P1_stepCond->reference[0].AmpareHour_SumAmpareHour = tmp;

			idx = IDX_LOC_OBJ_END_AMPARE_HOUR_BRANCH;
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			P1_stepCond->reference[0].AmpareHour_Branch_MultiCycleCountId
				//= (unsigned char)tmp;
				= (short int)tmp;	//jhkw_200410

			idx = IDX_LOC_OBJ_END_WATT_HOUR;
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			P1_stepCond->reference[0].WattHour_SumWattHour = tmp;

			idx = IDX_LOC_OBJ_END_WATT_HOUR_BRANCH;
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			P1_stepCond->reference[0].WattHour_Branch_MultiCycleCount_Branch
				//= (unsigned char)tmp;
				= (short int)tmp;	//jhkw_200410

			idx = IDX_LOC_OBJ_END_AMPARE_HOUR_RATE;
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			if(tmp != 0) { //ValueRate_Item 1:AmpareHour
				P1_stepCond->reference[0].ValueRate_Item = 1;
				P1_stepCond->reference[0].ValueRate = (unsigned short)tmp;

				idx = IDX_LOC_OBJ_END_AMPARE_HOUR_RATE_BRANCH;
				tmp = myData->testCond[ch].local_object[stepNo][idx];
				//jhkw_200410
				//P1_stepCond->reference[0].ValueRate_Branch = (unsigned char)tmp;
				P1_stepCond->reference[0].ValueRate_Branch = (short int)tmp;

				idx = IDX_LOC_OBJ_END_AMPARE_HOUR_RATE_COMPARE;
				tmp = myData->testCond[ch].local_object[stepNo][idx];
				//jhkw_200410
				//P1_stepCond->reference[0].ValueRate_Compare =(unsigned char)tmp;
				P1_stepCond->reference[0].ValueRate_Compare =(short int)tmp;
			} else {
				idx = IDX_LOC_OBJ_END_WATT_HOUR_RATE;
				tmp = myData->testCond[ch].local_object[stepNo][idx];
				if(tmp != 0) { //ValueRate_Item 2:WattHour
					P1_stepCond->reference[0].ValueRate_Item = 2;
					P1_stepCond->reference[0].ValueRate = (unsigned short)tmp;

					idx = IDX_LOC_OBJ_END_WATT_HOUR_RATE_BRANCH;
					tmp = myData->testCond[ch].local_object[stepNo][idx];
					P1_stepCond->reference[0].ValueRate_Branch
						//= (unsigned char)tmp;
						= (short int)tmp;	//jhkw_200410

					idx = IDX_LOC_OBJ_END_WATT_HOUR_RATE_COMPARE;
					tmp = myData->testCond[ch].local_object[stepNo][idx];
					P1_stepCond->reference[0].ValueRate_Compare
						//= (unsigned char)tmp;
						= (short int)tmp;	//jhkw_200410
				} else { //ValueRate_Item 0:Idle
					P1_stepCond->reference[0].ValueRate_Item = 0;
				}
			}

			idx = IDX_LOC_OBJ_END_I;
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			if(type == STEP_CHARGE) {
				P1_stepCond->reference[0].Current = tmp;
			} else {
				P1_stepCond->reference[0].Current = tmp * (-1);
			}

			idx = IDX_LOC_OBJ_END_DELTA_V;
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			P1_stepCond->reference[0].DeltaV = tmp;

			idx = IDX_LOC_OBJ_END_P;
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			P1_stepCond->reference[0].Power = tmp;

			//step edlc
			idx = IDX_LOC_OBJ_CAPACITANCE_V1;
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			P1_stepCond->edlc.capacitanceV1 = tmp;

			idx = IDX_LOC_OBJ_CAPACITANCE_V2;
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			P1_stepCond->edlc.capacitanceV2 = tmp;

			//fault
			idx = IDX_LOC_OBJ_FAULT_UPPER_I;
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			if(type == STEP_CHARGE) {
				P1_stepCond->faultUpperI = tmp;
			} else {
				P1_stepCond->faultUpperI = tmp * (-1);
			}

			idx = IDX_LOC_OBJ_FAULT_LOWER_I;
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			if(type == STEP_CHARGE) {
				P1_stepCond->faultLowerI = tmp;
			} else {
				P1_stepCond->faultLowerI = tmp * (-1);
			}

			idx = IDX_LOC_OBJ_FAULT_UPPER_Z;
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			P1_stepCond->faultUpperZ = tmp;

			idx = IDX_LOC_OBJ_FAULT_LOWER_Z;
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			P1_stepCond->faultLowerZ = tmp;

			idx = IDX_LOC_OBJ_FAULT_UPPER_AMPARE_HOUR;
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			P1_stepCond->faultUpper_AmpareHour = tmp;

			idx = IDX_LOC_OBJ_FAULT_LOWER_AMPARE_HOUR;
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			P1_stepCond->faultLower_AmpareHour = tmp;
			break;
		case STEP_PATTERN:
			//header
			idx = IDX_LOC_OBJ_PATTERN_FILE_SIZE;
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			P1_stepCond->pattern_file_size = tmp;

			idx = IDX_LOC_OBJ_PATTERN_FILE_CHECKSUM;
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			P1_stepCond->pattern_file_checksum = tmp;

			idx = IDX_LOC_OBJ_MODE;
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			P1_stepCond->header.mode
				= (unsigned char)convert_step_mode(CONVERT_ORG_TO_P1, tmp);

			//reference
			idx = IDX_LOC_OBJ_REF_V;
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			P1_stepCond->reference[0].refV_upper = tmp;

			idx = IDX_LOC_OBJ_REF_V2;
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			P1_stepCond->reference[0].refV_lower = tmp;

			//end
			idx = IDX_LOC_OBJ_END_AMPARE_HOUR;
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			P1_stepCond->reference[0].AmpareHour_SumAmpareHour = tmp;

			idx = IDX_LOC_OBJ_END_AMPARE_HOUR_BRANCH;
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			P1_stepCond->reference[0].AmpareHour_Branch_MultiCycleCountId
				//= (unsigned char)tmp;
				= (short int)tmp;	//jhkw_200410

			idx = IDX_LOC_OBJ_END_WATT_HOUR;
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			P1_stepCond->reference[0].WattHour_SumWattHour = tmp;

			idx = IDX_LOC_OBJ_END_WATT_HOUR_BRANCH;
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			P1_stepCond->reference[0].WattHour_Branch_MultiCycleCount_Branch
				//= (unsigned char)tmp;
				= (short int)tmp;	//jhkw_200410

			idx = IDX_LOC_OBJ_END_AMPARE_HOUR_RATE;
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			if(tmp != 0) { //ValueRate_Item 1:AmpareHour
				P1_stepCond->reference[0].ValueRate_Item = 1;
				P1_stepCond->reference[0].ValueRate = (unsigned short)tmp;

				idx = IDX_LOC_OBJ_END_AMPARE_HOUR_RATE_BRANCH;
				tmp = myData->testCond[ch].local_object[stepNo][idx];
				//jhkw_200410
				//P1_stepCond->reference[0].ValueRate_Branch = (unsigned char)tmp;
				P1_stepCond->reference[0].ValueRate_Branch = (short int)tmp;

				idx = IDX_LOC_OBJ_END_AMPARE_HOUR_RATE_COMPARE;
				tmp = myData->testCond[ch].local_object[stepNo][idx];
				//jhkw_200410
				//P1_stepCond->reference[0].ValueRate_Compare =(unsigned char)tmp;
				P1_stepCond->reference[0].ValueRate_Compare =(short int)tmp;
			} else {
				idx = IDX_LOC_OBJ_END_WATT_HOUR_RATE;
				tmp = myData->testCond[ch].local_object[stepNo][idx];
				if(tmp != 0) { //ValueRate_Item 2:WattHour
					P1_stepCond->reference[0].ValueRate_Item = 2;
					P1_stepCond->reference[0].ValueRate = (unsigned short)tmp;

					idx = IDX_LOC_OBJ_END_WATT_HOUR_RATE_BRANCH;
					tmp = myData->testCond[ch].local_object[stepNo][idx];
					P1_stepCond->reference[0].ValueRate_Branch
						//= (unsigned char)tmp;
						= (short int)tmp;	//jhkw_200410

					idx = IDX_LOC_OBJ_END_WATT_HOUR_RATE_COMPARE;
					tmp = myData->testCond[ch].local_object[stepNo][idx];
					P1_stepCond->reference[0].ValueRate_Compare
						//= (unsigned char)tmp;
						= (short int)tmp;	//jhkw_200410
				} else { //ValueRate_Item 0:Idle
					P1_stepCond->reference[0].ValueRate_Item = 0;
				}
			}

			//fault
			idx = IDX_LOC_OBJ_FAULT_UPPER_I;
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			if(tmp < 0) tmp *= (-1);
			P1_stepCond->faultUpperI = tmp;

			idx = IDX_LOC_OBJ_FAULT_LOWER_I;
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			if(tmp < 0) tmp *= (-1);
			P1_stepCond->faultLowerI = tmp;

			idx = IDX_LOC_OBJ_FAULT_UPPER_AMPARE_HOUR;
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			P1_stepCond->faultUpper_AmpareHour = tmp;

			idx = IDX_LOC_OBJ_FAULT_LOWER_AMPARE_HOUR;
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			P1_stepCond->faultLower_AmpareHour = tmp;
			break;
		case STEP_EXTERNAL_CAN:
			//kjg_170810_w
			break;
		case STEP_USERMAP:
			//kjg_170810_w
			break;
		default:
			break;
	}

	P1_stepCond->header.testEnd = (unsigned char)tmp;
	P1_stepCond->header.subStep = (unsigned char)tmp;
	//P1_stepCond->header.patternIndex = (unsigned char)tmp;
	
	//compV
	//compI
	//deltaV
	//deltaI
	
	//etc
	P1_stepCond->pattern_time_type = (unsigned char)tmp;
	P1_stepCond->pattern_max_val = (float)tmp;
}

void StateChange_Pause(int num)
{
	int i, ch, toPs;
	S_MSG_VAL SendMsg;

	memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));

	if(myPs->config.state_change == 1) {
		for(i=0; i < myPs->misc.chInGroup; i++) {
			ch = (int)myData->CellArray1[myPs->misc.chOffset + i].number2 - 1;
			if(myData->cData[ch].op.state == C_RUN)
				myData->cData[ch].signal[C_SIG_CMD_PAUSE] = P1;
		}
	} else if(myPs->config.state_change == 2) {
		toPs = COA1_TO_MODULE + myPs->config.groupNo;
		SendMsg.msg = MSG_COA_MODULE_SAVE_MSG_FLAG;
		SendMsg.val[0] = 0;
		SendMsg.val[1] = 1; //stop
		send_msg(toPs, (char *)&SendMsg);
	}

	if(myPs->misc.network_socket1 > 0) {
		close(myPs->misc.network_socket1);
	}
	if(myPs->misc.network_socket2 > 0) {
		close(myPs->misc.network_socket2);
	}
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
