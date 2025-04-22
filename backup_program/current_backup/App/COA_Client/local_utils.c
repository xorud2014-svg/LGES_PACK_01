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

	//kjh_211021
	userlog(DEBUG_LOG, psName, "-----------System Parameter Info(Client)-----------\n");
	userlog(DEBUG_LOG, psName, "Host(PC) IP		: %s\n", myPs->config.ipAddr);
	userlog(DEBUG_LOG, psName, "F/W Protocol Ver	: %04x\n", myPs->config.protocol_version);
	userlog(DEBUG_LOG, psName, "---------------------------------------------------\n");

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
	//jhkw_190503s		//jhj_250310 SK_network_timeout_merge
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

			size = sizeof(long) * MAX_CALI_POINT * 4
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

			size = sizeof(long) * MAX_CALI_POINT * 4
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
	char cmd[256];
	int fp, rtn, i, size, checksum;
	int debug_mode;		//csk_190213
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

	//csk_190213s for_debug
																		//shh_test	
	debug_mode = 0;	//0: no_use, 1~8: select debug_ch, 9: all debug_ch
	if(debug_mode == 0) {
	} else {
		if((debug_mode == 9) || (debug_mode == ch+1))
			rtn = Write_test_cond_step_file((S_P1_FILE_TEST_COND *)&test_cond, ch, div);
	}	//csk_190213e for_debug
	

	return 0;
}

int Write_test_cond_step_file(S_P1_FILE_TEST_COND *P1_testCond, int ch, int div)
{	//csk_190213s
	char temp[4], fileName[256];
	int i,j,k = 0;
	FILE *fp;

	memset(fileName, 0, sizeof(fileName));
	strcpy(fileName, "/root/");
	strcat(fileName, (char *)&myData->AppControl.misc.path1);
	strcat(fileName, "/log/sbc_schedule_ch");
	memset(temp, 0, sizeof temp);
	temp[0] = (char)(49+ch);
	strcat(fileName, temp);
	strcat(fileName, "_div");
	memset(temp, 0, sizeof temp);
	temp[0] = (char)(49+div);
	strcat(fileName, temp);
	strcat(fileName, "_info.txt");
	//root/system_data/config/log/sbc_schedule_ch####_div####_info.txt

	if((fp = fopen(fileName, "w")) == NULL) {
		userlog(DEBUG_LOG, psName, "Can not open sbc_schedule_ch%04d_div%04d_info.txt file(write)\n", ch+1, div+1);
		return -1;
	}

	fprintf(fp, "CH%d : DIV(%d) sbc_schedule_info.sch\n", ch+1, div+1);
	fprintf(fp,"\n");
		
	fprintf(fp, "TestCond_Header\n");
	fprintf(fp, "totalStep : %d\n", 
		(int)P1_testCond->header.totalStep);
	fprintf(fp, "totalPatternCount : %d\n", 
		(int)P1_testCond->header.totalPatternCount);
	fprintf(fp, "totalTimeSchCount : %d\n", 
		(int)P1_testCond->header.totalTimeSchCount);
	fprintf(fp, "reserverd : %d\n", 
		(int)P1_testCond->header.reserved);
	fprintf(fp, "reserverd1[2] : %ld, %ld\n", 
		(long)P1_testCond->header.reserved1[0],
		(long)P1_testCond->header.reserved1[1]);
	fprintf(fp, "\n");
	
	fprintf(fp, "Common_Safety\n");
	fprintf(fp, "faultLowerV : %ld\n", 
		(long)P1_testCond->safety.faultLowerV);
	fprintf(fp, "faultUpperV : %ld\n", 
		(long)P1_testCond->safety.faultUpperV);
	fprintf(fp, "faultCompAuxV : %ld\n", 
		(long)P1_testCond->safety.faultCompAuxV);
	fprintf(fp, "faultUpperI : %ld\n", 
		(long)P1_testCond->safety.faultUpperI);
	fprintf(fp, "faultLowerTemp : %ld\n", 
		(long)P1_testCond->safety.faultLowerTemp);
	fprintf(fp, "faultUpperTemp : %ld\n", 
		(long)P1_testCond->safety.faultUpperTemp);
	fprintf(fp, "faultUpper_AmpareHour : %ld\n", 
		(long)P1_testCond->safety.faultUpper_AmpareHour);
	fprintf(fp, "faultUpperP : %ld\n", 
		(long)P1_testCond->safety.faultUpperP);
	fprintf(fp, "faultUpper_WattHour : %ld\n", 
		(long)P1_testCond->safety.faultUpper_WattHour);
	fprintf(fp,"\n");

	fprintf(fp, "CAN_Common_Safety\n");
	fprintf(fp, "index, Division, Compare_Type, Data_Type, Value\n");
	for(i=0; i < MAX_P1_CAN_FUNCTION; i++) {
		fprintf(fp, "%02d, %d, %d, %d, %f\n", i+1,
			(int)P1_testCond->safety.can_func_div[i],
			(int)P1_testCond->safety.can_compare_type[i],
			(int)P1_testCond->safety.can_data_type[i],
			(float)P1_testCond->safety.can_value[i]);
	}
	fprintf(fp,"\n");

	fprintf(fp, "Aux_Common_Safety\n");
	fprintf(fp, "index, Division, Compare_Type, Data_Type, Value\n");
	for(i=0; i < MAX_P1_AUX_FUNCTION; i++) {
		fprintf(fp, "%02d, %d, %d, %d, %ld\n", i+1,
			(int)P1_testCond->safety.aux_func_div[i],
			(int)P1_testCond->safety.aux_compare_type[i],
			(int)P1_testCond->safety.aux_data_type[i],
			(long)P1_testCond->safety.aux_value[i]);
	}
	fprintf(fp,"\n");

	fprintf(fp, "reserved2[4] : %ld, %ld, %ld, %ld\n", 
		(long)P1_testCond->safety.reserved2[0],
		(long)P1_testCond->safety.reserved2[1],
		(long)P1_testCond->safety.reserved2[2],
		(long)P1_testCond->safety.reserved2[3]);
	fprintf(fp,"\n");

	fprintf(fp, "TestCond_Step\n");
	fprintf(fp,"\n");

	for(i=0; i < (int)P1_testCond->header.totalStep; i++) {
		fprintf(fp, "Step_Header : %d\n", i+1);
		fprintf(fp, "type : %d\n",
			(int)P1_testCond->step[i].header.type);
		fprintf(fp, "stepNo : %d\n",
			(int)P1_testCond->step[i].header.stepNo);
		fprintf(fp, "mode : %d\n",
			(int)P1_testCond->step[i].header.mode);
		fprintf(fp, "testEnd : %d\n",
			(int)P1_testCond->step[i].header.testEnd);
		fprintf(fp, "subStep : %d\n",
			(int)P1_testCond->step[i].header.subStep);

		fprintf(fp, "reserved1 : %d\n", 
			(int)P1_testCond->step[i].header.reserved1);
		fprintf(fp, "cycle_pause : %d\n", 
			(int)P1_testCond->step[i].header.cycle_pause);
		fprintf(fp, "patternIndex : %d\n",
			(int)P1_testCond->step[i].header.patternIndex);
		fprintf(fp, "reserved2 : %d\n", 
			(int)P1_testCond->step[i].header.reserved2);
		fprintf(fp,"\n");

		for(j=0; j < 1; j++) {
//		for(j=0; j < MAX_P1_SUB_STEP; j++) {
			fprintf(fp, "Step_Reference[%d][%d]\n", i, j);
			fprintf(fp, "refV_upper : %ld\n",
				(long)P1_testCond->step[i]
				.reference[j].refV_upper);
			fprintf(fp, "refV_lower : %ld\n",
				(long)P1_testCond->step[i]
				.reference[j].refV_lower);
			fprintf(fp, "refI : %ld\n",
				(long)P1_testCond->step[i].reference[j].refI);
			fprintf(fp, "refP : %ld\n",
				(long)P1_testCond->step[i].reference[j].refP);
			fprintf(fp, "refR : %ld\n",
				(long)P1_testCond->step[i].reference[j].refR);
			fprintf(fp, "rangeI : %d\n",
				(int)P1_testCond->step[i].reference[j].rangeI);
			fprintf(fp, "ValueRate_Compare : %d\n",
				(int)P1_testCond->step[i]
				.reference[j].ValueRate_Compare);
			fprintf(fp, "ValueRate : %d\n",
				(int)P1_testCond->step[i]
				.reference[j].ValueRate);
			fprintf(fp, "endTime_day : %ld\n",
				(long)P1_testCond->step[i].reference[j].endTime_day);
			fprintf(fp, "endTime : %ld\n",
				(long)P1_testCond->step[i].reference[j].endTime);
			fprintf(fp, "V_Upper : %ld\n",
				(long)P1_testCond->step[i].reference[j].V_Upper);
			fprintf(fp, "Current : %ld\n",
				(long)P1_testCond->step[i].reference[j].Current);
			fprintf(fp, "AmpareHour_SumAmpareHour : %ld\n",
				(long)P1_testCond->step[i]
				.reference[j].AmpareHour_SumAmpareHour);
			fprintf(fp, "V_Upper_Branch_CycleCount_Branch : %ld\n",
				(long)P1_testCond->step[i]
				.reference[j].V_Upper_Branch_CycleCount_Branch);
			fprintf(fp, "V_Lower_Branch_CycleCount : %ld\n",
				(long)P1_testCond->step[i]
				.reference[j].V_Lower_Branch_CycleCount);
			fprintf(fp, "DeltaV : %ld\n",
				(long)P1_testCond->step[i].reference[j].DeltaV);
			fprintf(fp, "Power : %ld\n",
				(long)P1_testCond->step[i].reference[j].Power);
			fprintf(fp, "WattHour_SumWattHour : %ld\n",
				(long)P1_testCond->step[i]
				.reference[j].WattHour_SumWattHour);
			fprintf(fp, "Time_Branch_AccCycleCount : %ld\n",
				(long)P1_testCond->step[i]
				.reference[j].Time_Branch_AccCycleCount);
			fprintf(fp, "CVTime_Branch_AccCycleCountId : %ld\n",
				(long)P1_testCond->step[i]
				.reference[j].CVTime_Branch_AccCycleCountId);
			fprintf(fp, "CVTime_day : %ld\n",
				(long)P1_testCond->step[i].reference[j].CVTime_day);
			fprintf(fp, "CVTime : %ld\n",
				(long)P1_testCond->step[i].reference[j].CVTime);
			fprintf(fp, "chamber_step_check_flag : %d\n",
				(int)P1_testCond->step[i].reference[j].chamber_step_check_flag);
			fprintf(fp, "reserved[2] : %d, %d\n",
				(int)P1_testCond->step[i]
				.reference[j].reserved[0],
				(int)P1_testCond->step[i]
				.reference[j].reserved[1]);
			fprintf(fp, "ValueRate_Item : %d\n",
				(int)P1_testCond->step[i]
				.reference[j].ValueRate_Item);
			fprintf(fp, "V_Lower_MultiCycleCount : %ld\n",
				(long)P1_testCond->step[i]
				.reference[j].V_Lower_MultiCycleCount);
			fprintf(fp, "AmpareHour_Branch_MultiCycleCountId : %d\n",
				(int)P1_testCond->step[i]
				.reference[j].AmpareHour_Branch_MultiCycleCountId);
			fprintf(fp, "WattHour_Branch_MultiCycleCount_Branch : %d\n",
				(int)P1_testCond->step[i]
				.reference[j].WattHour_Branch_MultiCycleCount_Branch);
			fprintf(fp, "ValueRate_Branch : %d\n",
				(int)P1_testCond->step[i]
				.reference[j].ValueRate_Branch);
			fprintf(fp, "AccCycleCount_Branch : %d\n",
				(int)P1_testCond->step[i]
				.reference[j].AccCycleCount_Branch);
			fprintf(fp,"\n");

			fprintf(fp, "Step_CAN\n");
			fprintf(fp, "index, Division, Compare_Type, Data_Type, Branch, Value\n");
			for(k=0; k < MAX_P1_CAN_FUNCTION; k++) {
				fprintf(fp, "%02d, %d, %d, %d, %d, %f\n", k+1,
					(int)P1_testCond->step[i]
					.reference[j].can_func_div[k],
					(int)P1_testCond->step[i]
					.reference[j].can_compare_type[k],
					(int)P1_testCond->step[i]
					.reference[j].can_data_type[k],
					(int)P1_testCond->step[i]
					.reference[j].can_branch[k],
					(float)P1_testCond->step[i]
					.reference[j].can_value[k]);
			}
			fprintf(fp,"\n");

			fprintf(fp, "Step_Aux\n");
			fprintf(fp, "index, Division, Compare_Type, Data_Type, Branch, Value, delay_time\n");
			for(k=0; k < MAX_P1_AUX_FUNCTION; k++) {
				fprintf(fp, "%02d, %d, %d, %d, %d, %ld, %d\n", k+1,
					(int)P1_testCond->step[i]
					.reference[j].aux_func_div[k],
					(int)P1_testCond->step[i]
					.reference[j].aux_compare_type[k],
					(int)P1_testCond->step[i]
					.reference[j].aux_data_type[k],
					(int)P1_testCond->step[i]
					.reference[j].aux_branch[k],
					(long)P1_testCond->step[i]
					.reference[j].aux_value[k],
					(int)P1_testCond->step[i]
					.reference[j].aux_delay_time[k]);
			}
			fprintf(fp,"\n");
				
			/*fprintf(fp, "reserved2[2] : %ld, %ld\n",	//ktg_220512s	//shh_220607s
				(long)P1_testCond->step[i]
				.reference[j].reserved2[0],
				(long)P1_testCond->step[i]
				.reference[j].reserved2[1]);
			fprintf(fp,"\n");*/
			fprintf(fp, "pattern_max_refI : %ld, pattern_min_refI %ld\n",
				(long)P1_testCond->step[i]
				.reference[j].pattern_max_refI,
				(long)P1_testCond->step[i]
				.reference[j].pattern_min_refI);	//ktg_220512e	//shh_220607e
			fprintf(fp,"\n");
		}

		fprintf(fp, "Step_CompV[%d]\n", i);
		fprintf(fp, "lowerVlaue, upperValue, time\n");
		for(j=0; j < MAX_P1_COMP_POINT; j++) {
			fprintf(fp, "%ld, %ld, %ld\n",
				(long)P1_testCond->step[i].compV[j].lowerValue,
				(long)P1_testCond->step[i].compV[j].upperValue,
				(long)P1_testCond->step[i].compV[j].time);
		}
		fprintf(fp,"\n");

		fprintf(fp, "Step_CompI[%d]\n", i);
		fprintf(fp, "lowerVlaue, upperValue, time\n");
		for(j=0; j < MAX_P1_COMP_POINT; j++) {
			fprintf(fp, "%ld, %ld, %ld\n",
				(long)P1_testCond->step[i].compI[j].lowerValue,
				(long)P1_testCond->step[i].compI[j].upperValue,
				(long)P1_testCond->step[i].compI[j].time);
		}
		fprintf(fp,"\n");
				
		fprintf(fp, "Step_DeltaV\n");
		fprintf(fp, "lowerValue : %ld\n",
			(long)P1_testCond->step[i].deltaV.lowerValue);
		fprintf(fp, "upperValue : %ld\n",
			(long)P1_testCond->step[i].deltaV.upperValue);
		fprintf(fp, "time : %ld\n",
			(long)P1_testCond->step[i].deltaV.time);
		fprintf(fp,"\n");

		fprintf(fp, "Step_DeltaI\n");
		fprintf(fp, "lowerValue : %ld\n",
			(long)P1_testCond->step[i].deltaI.lowerValue);
		fprintf(fp, "upperValue : %ld\n",
			(long)P1_testCond->step[i].deltaI.upperValue);
		fprintf(fp, "time : %ld\n",
			(long)P1_testCond->step[i].deltaI.time);
		fprintf(fp,"\n");

		fprintf(fp, "Step_Record\n");
		fprintf(fp, "time : %ld\n",
			(long)P1_testCond->step[i].record.time);
		fprintf(fp, "deltaV : %ld\n",
			(long)P1_testCond->step[i].record.deltaV);
		fprintf(fp, "deltaI : %ld\n",
			(long)P1_testCond->step[i].record.deltaI);
		fprintf(fp, "deltaT : %ld\n",
			(long)P1_testCond->step[i].record.deltaT);
		fprintf(fp, "deltaP : %ld\n",
			(long)P1_testCond->step[i].record.deltaP);
		fprintf(fp, "reserved : %ld\n",
			(long)P1_testCond->step[i].record.reserved);
		fprintf(fp,"\n");
		
		fprintf(fp, "Step_EDLC\n");
		fprintf(fp, "capacitanceV1 : %ld\n",
			(long)P1_testCond->step[i].edlc.capacitanceV1);
		fprintf(fp, "capacitanceV2 : %ld\n",
			(long)P1_testCond->step[i].edlc.capacitanceV2);
		fprintf(fp, "startT_Z : %ld\n",
			(long)P1_testCond->step[i].edlc.startT_Z);
		fprintf(fp, "endT_Z : %ld\n",
			(long)P1_testCond->step[i].edlc.endT_Z);
		fprintf(fp, "startT_LC : %ld\n",
			(long)P1_testCond->step[i].edlc.startT_LC);
		fprintf(fp, "endT_LC : %ld\n",
			(long)P1_testCond->step[i].edlc.endT_LC);
		fprintf(fp,"\n");

		for(j=0; j < 1; j++) {
//		for(j=0; j < MAX_P1_GRADE_ITEM; j++) {
			fprintf(fp, "Step_Grade[%d][%d]\n", i, j);
			fprintf(fp, "item : %d\n",
				(int)P1_testCond->step[i].grade[j].item);
			fprintf(fp, "gradeStepCount : %d\n",
				(int)P1_testCond->step[i]
				.grade[j].gradeStepCount);
			fprintf(fp, "reserved1 : %d\n",
				(int)P1_testCond->step[i].grade[j].reserved1);
			fprintf(fp,"\n");

			for(k=0; k < 1; k++) {
//			for(k=0; k < MAX_P1_GRADE_STEP; k++) {
				fprintf(fp, "Step_Grade_Step[%d][%d][%d]\n", i, j, k);
				fprintf(fp, "gradeCode : %d\n",
					(int)P1_testCond->step[i]
					.grade[j].gradeStep[k].gradeCode);
				fprintf(fp, "reserved1 : %d\n",
					(int)P1_testCond->step[i]
					.grade[j].gradeStep[k].reserved1);
				fprintf(fp, "reserved2 : %d\n",
					(int)P1_testCond->step[i]
					.grade[j].gradeStep[k].reserved2);
				fprintf(fp, "lowerValue : %ld\n",
					(long)P1_testCond->step[i]
					.grade[j].gradeStep[k].lowerValue);
				fprintf(fp, "upperValue : %ld\n",
					(long)P1_testCond->step[i]
					.grade[j].gradeStep[k].upperValue);
				fprintf(fp,"\n");
			}
		}

		fprintf(fp, "TestCond_Step_Fault\n");
		fprintf(fp, "faultUpperV : %ld\n",
			(long)P1_testCond->step[i].faultUpperV);
		fprintf(fp, "faultLowerV : %ld\n",
			(long)P1_testCond->step[i].faultLowerV);
		fprintf(fp, "faultUpperI : %ld\n",
			(long)P1_testCond->step[i].faultUpperI);
		fprintf(fp, "faultLowerI : %ld\n",
			(long)P1_testCond->step[i].faultLowerI);
		fprintf(fp, "faultUpper_AmpareHour : %ld\n",
			(long)P1_testCond->step[i].faultUpper_AmpareHour);
		fprintf(fp, "faultLower_AmpareHour : %ld\n",
			(long)P1_testCond->step[i].faultLower_AmpareHour);
		fprintf(fp, "faultUpperZ : %ld\n",
			(long)P1_testCond->step[i].faultUpperZ);
		fprintf(fp, "faultLowerZ : %ld\n",
			(long)P1_testCond->step[i].faultLowerZ);
		
		fprintf(fp, "faultUpperTemp : %ld\n",
			(long)P1_testCond->step[i].faultUpperTemp);
		fprintf(fp, "faultLowerTemp : %ld\n",
			(long)P1_testCond->step[i].faultLowerTemp);
		fprintf(fp, "pattern_time_type : %d\n",
			(int)P1_testCond->step[i].pattern_time_type);
		fprintf(fp, "can_comm_step_check_flag : %d\n",
			(int)P1_testCond->step[i].can_comm_step_check_flag);
		fprintf(fp, "fault_can_check_flag : %d\n",
			(int)P1_testCond->step[i].fault_can_check_flag);
		fprintf(fp, "can_tx_stop : %d\n",
			(int)P1_testCond->step[i].can_tx_stop);
		fprintf(fp, "pattern_max_val : %f\n",
			(float)P1_testCond->step[i].pattern_max_val);
		fprintf(fp, "pattern_file_size : %ld\n",
			(long)P1_testCond->step[i].pattern_file_size);
		fprintf(fp, "pattern_file_checksum : %ld\n",
			(long)P1_testCond->step[i].pattern_file_checksum);
		//fprintf(fp, "reserved2[2] : %ld, %ld\n",		//ktg_210312s
		//	(long)P1_testCond->step[i].reserved2[0],
		//	(long)P1_testCond->step[i].reserved2[1]);	//ktg_210312e
		fprintf(fp,"\n");
	}
	fclose(fp);

	return 0;
}//csk_190213e


int read_test_cond_time_step_file(int ch, int div)
{ //kjh_160418
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

	idx = IDX_COM_OBJ_TOTAL_PATTERN;
	myTestCond->common_object[idx]
		= (long)myPs->testCond.header.totalPatternCount;

	idx = IDX_COM_OBJ_ATTRIBUTE_COUNT;
	attr_count = 1;
	if(myData->ChAttribute[ch].opType != OP_INDEPENDENT) {
		for(i=0; i < MAX_SLAVE_CH; i++) {
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

	idx = IDX_COM_OBJ_FAULT_HW_UPPER_V;
	myTestCond->common_object[idx] = 5000; //5.0%
	make_com_chCode_cond((S_TEST_CONDITION *)myTestCond,
		C_CD_COM_FAULT_HW_UPPER_V, fault_count,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_COM_OBJ_FAULT_HW_LOWER_V;
	switch(myData->AppControl.config.systemModel) {
		case C_LGC_450V_200A_10A_180KW_2:
			myTestCond->common_object[idx] = 100000; //100.0%		//ktg_210222
			break;
		case C_LGC_500V_250A_125A_50A_500KW_11: //define 10450
		case C_LGC_450V_200A_10A_180KW_3:	//define 1158	//ktg_210222
		case C_LGC_450V_200A_20A_180KW_2:	//define 1065	//ktg_210222
		case C_LGC_450V_200A_20A_180KW:	//ktg_210222
		case C_LGC_450V_250A_100A_225KW:	//define 10174	//ktg_210428
		case C_LGC_450V_250A_50A_225KW:     //define 10134	//shh_211214
			myTestCond->common_object[idx] = 20000; //20.0%
			break;
		default:
			myTestCond->common_object[idx] = 5000; //5.0%
			break;
	} //kjhw_200610
	//myTestCond->common_object[idx] = 5000; //5.0%
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

	idx = IDX_COM_OBJ_FAULT_UPPER_I;
	myTestCond->common_object[idx] = myPs->testCond.safety.faultUpperI;
	make_com_chCode_cond((S_TEST_CONDITION *)myTestCond,
		C_CD_COM_FAULT_UPPER_I, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

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

	idx = IDX_COM_OBJ_FAULT_UPPER_TEMP; //kjhw_181111 use
	myTestCond->common_object[idx] = myPs->testCond.safety.faultUpperTemp;
	make_com_chCode_cond((S_TEST_CONDITION *)myTestCond,
		C_CD_COM_FAULT_UPPER_TEMP, 500, COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_COM_OBJ_FAULT_LOWER_TEMP;
	myTestCond->common_object[idx] = myPs->testCond.safety.faultLowerTemp;
	make_com_chCode_cond((S_TEST_CONDITION *)myTestCond,
		C_CD_COM_FAULT_LOWER_TEMP, 1, COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=

	//idx = IDX_COM_OBJ_FAULT_AUX_V; //kjhw_181111
	//myTestCond->common_object[idx] = myPs->testCond.safety.faultAuxV;
	idx = IDX_COM_OBJ_FAULT_UPPER_AUX_V;	//csk_190808
	myTestCond->common_object[idx] = myPs->testCond.safety.faultUpper_AuxV;
	//make_com_chCode_cond((S_TEST_CONDITION *)myTestCond,
	//	C_CD_COM_FAULT_AUX_V, 500, COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_COM_OBJ_FAULT_LOWER_AUX_V;	//csk_190808
	myTestCond->common_object[idx] = myPs->testCond.safety.faultLower_AuxV;

	idx = IDX_COM_OBJ_FAULT_COMP_AUX_V; //kjhw_150730
	myTestCond->common_object[idx] = myPs->testCond.safety.faultCompAuxV;
	make_com_chCode_cond((S_TEST_CONDITION *)myTestCond,
		C_CD_COM_FAULT_COMP_AUX_V, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_COM_OBJ_FAULT_DELTA_V; //jhkw_160828
	myTestCond->common_object[idx] = 10000; //10.0%
	make_com_chCode_cond((S_TEST_CONDITION *)myTestCond,
		C_CD_COM_FAULT_DELTA_V, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=
		//1s delay

	idx = IDX_COM_OBJ_FAULT_COMP_AUX_REF_V1;	//csk_190808s
	myTestCond->common_object[idx] = myPs->testCond.safety.faultCompAux_RefV1;

	idx = IDX_COM_OBJ_FAULT_COMP_AUX_V2;
	myTestCond->common_object[idx] = myPs->testCond.safety.faultCompAuxV2;
	make_com_chCode_cond((S_TEST_CONDITION *)myTestCond,
		C_CD_COM_FAULT_COMP_AUX_V2, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_COM_OBJ_FAULT_COMP_AUX_REF_V2;
	myTestCond->common_object[idx] = myPs->testCond.safety.faultCompAux_RefV2;

	idx = IDX_COM_OBJ_FAULT_COMP_AUX_V3;
	myTestCond->common_object[idx] = myPs->testCond.safety.faultCompAuxV3;
	make_com_chCode_cond((S_TEST_CONDITION *)myTestCond,
		C_CD_COM_FAULT_COMP_AUX_V3, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_COM_OBJ_FAULT_COMP_AUX_VENT_FLAG;
	myTestCond->common_object[idx] = myPs->testCond.safety.faultCompAuxV_vent_flag;
	//csk_190808e
	
	idx = IDX_COM_OBJ_FAULT_COMP_TYPE_SELECT;	//ktg_241008s
	myTestCond->common_object[idx]
		= myPs->testCond.safety.faultCompType_select;

	idx = IDX_COM_OBJ_FAULT_COMP_CAN_V;
	myTestCond->common_object[idx] = myPs->testCond.safety.faultCompAuxV;
	make_com_chCode_cond((S_TEST_CONDITION *)myTestCond,
		C_CD_COM_FAULT_COMP_CAN_V, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=


	idx = IDX_COM_OBJ_FAULT_COMP_CAN_REF_V1;
	myTestCond->common_object[idx]
		= myPs->testCond.safety.faultCompAux_RefV1;

	idx = IDX_COM_OBJ_FAULT_COMP_CAN_V2;
	myTestCond->common_object[idx]
		= myPs->testCond.safety.faultCompAuxV2;
	make_com_chCode_cond((S_TEST_CONDITION *)myTestCond,
		C_CD_COM_FAULT_COMP_CAN_V2, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_COM_OBJ_FAULT_COMP_CAN_REF_V2;
	myTestCond->common_object[idx]
		= myPs->testCond.safety.faultCompAux_RefV2;

	idx = IDX_COM_OBJ_FAULT_COMP_CAN_V3;
	myTestCond->common_object[idx]
		= myPs->testCond.safety.faultCompAuxV3;
	make_com_chCode_cond((S_TEST_CONDITION *)myTestCond,
		C_CD_COM_FAULT_COMP_CAN_V3, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=	//ktg_241008e

	for(i =0; i < MAX_AUX_GROUP; i++) {	//ktg_220220s
		idx = IDX_COM_OBJ_FAULT_GROUP01_COMP_AUX_V1 + i;
		myTestCond->common_object[idx] = myPs->testCond.safety.faultCompGroupAuxV[i];
	
		idx = IDX_COM_OBJ_FAULT_GROUP01_COMP_AUX_REF_V1 + i;
		myTestCond->common_object[idx] = myPs->testCond.safety.faultCompGroupAux_RefV1[i];
	
		idx = IDX_COM_OBJ_FAULT_GROUP01_COMP_AUX_V2 + i;
		myTestCond->common_object[idx] = myPs->testCond.safety.faultCompGroupAuxV2[i];
	
		idx = IDX_COM_OBJ_FAULT_GROUP01_COMP_AUX_REF_V2 + i;
		myTestCond->common_object[idx] = myPs->testCond.safety.faultCompGroupAux_RefV2[i];
	
		idx = IDX_COM_OBJ_FAULT_GROUP01_COMP_AUX_V3 + i;
		myTestCond->common_object[idx] = myPs->testCond.safety.faultCompGroupAuxV3[i];
	
		idx = IDX_COM_OBJ_FAULT_GROUP01_COMP_AUX_VENT_FLAG + i;
		myTestCond->common_object[idx] = myPs->testCond.safety.faultCompGroupAuxV_vent_flag[i];
	}	//ktg_220220e
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
				//jhkw_200517s //shh_210906s
				if(myData->COA_Client[0].signal[P1_SIG_TEST_HEADER_UPDATE_RCV] == 1) {
					rtn = convert_test_cond_step_default(P1_stepCond, ch, step,
						type,attr_count, 1);
				} else {
					rtn = convert_test_cond_step_default(P1_stepCond, ch, step,
						type,attr_count, 0);
				}
				//rtn = convert_test_cond_step_default(P1_stepCond, ch, step,
				//	type,attr_count, 0);
				//jhkw_200517s //shh_210906e
				if(rtn < 0) return rtn;
				break;
		}
	}

	return 0;
}

int convert_test_cond_time(int ch)
{ //kjh_160418
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

	return 0;
}

void convert_test_cond_common_aux(S_P1_TEST_COND_SAFETY *P1_safety, int ch)
{
	int idx, i, compare_type;
	long func_div, data_type, data_value;

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
		//myTestCond->common_object[idx]
		//	= (long)(P1_safety->aux_value[i] * 1000.0);
		myTestCond->common_object[idx] = P1_safety->aux_value[i];

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
		data_value = P1_safety->aux_value[i];
		/*if(data_type == 2) { //1:signed, 2:float
			//data_value = (long)(P1_safety->aux_value[i] * 1000.0);
			tmp_f = P1_safety->aux_value[i] * 1000.0;
			data_value = (long)tmp_f; //kjh_190709
		} else {
			data_value = (long)P1_safety->aux_value[i];
		}*/

		//jhkw_211014s
		//if(func_div == AUX_FUNC_DIV_CV_SELECT) {
		if(func_div == AUX_FUNC_DIV_CV_SELECT
			|| func_div == AUX_FUNC_DIV_MAX_CELL_CV) {
			if(func_div == AUX_FUNC_DIV_MAX_CELL_CV) {
				idx = IDX_COM_OBJ_MAX_AUX_CELL_CV;
				myTestCond->common_object[idx] = 1;
			}
		//jhkw_211014e
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
	float tmp_f = 0.0; //kjh_190709

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
		//myTestCond->common_object[idx]
		//	= (long)(P1_safety->can_value[i] * 1000.0);
		tmp_f = P1_safety->can_value[i] * 1000.0;
		myTestCond->common_object[idx] = (long)tmp_f; //kjh_190709

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
			//data_value = (long)(P1_safety->can_value[i] * 1000.0);
			tmp_f = P1_safety->can_value[i] * 1000.0;
			data_value = (long)tmp_f; //kjh_190709
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
				|| func_div == CAN_RX_FUNC_DIV_MUX_B_FORCE_ON
				|| func_div == CAN_RX_FUNC_DIV_MUX_B_FORCE_OFF //kjhw_151021e
				|| func_div == CAN_RX_FUNC_DIV_A_ON //jhkw_170119s
				|| func_div == CAN_RX_FUNC_DIV_A_OFF
				|| func_div == CAN_RX_FUNC_DIV_B_ON
				|| func_div == CAN_RX_FUNC_DIV_B_OFF //jhkw_170119e
				|| func_div == CAN_RX_FUNC_DIV_ISOLATION_ON		//ktg_210908	//ktg_220701
				|| func_div == CAN_RX_FUNC_DIV_ISOLATION_OFF	//ktg_210908	//ktg_220701
				|| func_div == CAN_RX_FUNC_DIV_TEST5_RUN	//ktg_220724
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
	long l_val;	//jhkw_180726

	//jhkw_180726s
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

	idx2 = IDX_LOC_OBJ_END_SUM_TIME_DAY;
	myTestCond->local_object[step][idx2] = P1_stepCond->reference[0]
		.endTime_day;
	idx = IDX_LOC_OBJ_END_SUM_TIME;
	myTestCond->local_object[step][idx] = P1_stepCond->reference[0].endTime;
	make_loc_chCode_cond2((S_TEST_CONDITION *)myTestCond, step,
		C_CD_END_SUM_TIME, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx, idx2); //>=

	idx = IDX_LOC_OBJ_END_SUM_TIME_BRANCH;
	myTestCond->local_object[step][idx] = ACTIVE_DIV_CURRENT_LOOP;
}

void convert_test_cond_loop(S_P1_TEST_COND_STEP *P1_stepCond, int ch, int step, long type, long attr_count)
{
	int idx;
	int idx2, debug_mode, debug_step;	//csk_190213
	//jhkw_180726s
	long l_val;
	long l_val2;

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
	
	//csk_190208s
	idx = IDX_LOC_OBJ_END_AMPARE_HOUR_UPPER;
	myTestCond->local_object[step][idx] = P1_stepCond->reference[0].V_Upper;

	idx = IDX_LOC_OBJ_END_AMPARE_HOUR_UPPER_BRANCH;
	myTestCond->local_object[step][idx] = P1_stepCond->reference[0].endTime_day;

	idx = IDX_LOC_OBJ_END_AMPARE_HOUR_LOWER;
	myTestCond->local_object[step][idx] = P1_stepCond->reference[0].Current;

	idx = IDX_LOC_OBJ_END_AMPARE_HOUR_LOWER_BRANCH;
	myTestCond->local_object[step][idx] = P1_stepCond->reference[0].endTime;

	idx = IDX_LOC_OBJ_END_WATT_HOUR_UPPER;
	myTestCond->local_object[step][idx] = P1_stepCond->reference[0].DeltaV;

	idx = IDX_LOC_OBJ_END_WATT_HOUR_UPPER_BRANCH;
	myTestCond->local_object[step][idx] = P1_stepCond->reference[0].CVTime_day;

	idx = IDX_LOC_OBJ_END_WATT_HOUR_LOWER;
	myTestCond->local_object[step][idx] = P1_stepCond->reference[0].Power;

	idx = IDX_LOC_OBJ_END_WATT_HOUR_LOWER_BRANCH;
	myTestCond->local_object[step][idx] = P1_stepCond->reference[0].CVTime;
	//csk_190208e
	
	//csk_190213s for_debug
	debug_mode = 0;	//0: no_use, 1~8: select debug_ch, 9: all debug_ch
	debug_step = 0; //0: all_step, 1~: select_step
	if(debug_mode == 0) {
	} else {
		if((debug_mode == ch+1) || (debug_mode == 9)) {
			if((debug_step == 0) || ((debug_step+1) == step)) {
				userlog(DEBUG_LOG, psName, "ch%d[step: %d] testCond_loop_step print", ch+1, step+1);
				/*
				idx = IDX_LOC_OBJ_RANGE_V;
				l_val = myTestCond->local_object[step][idx];
				idx2 = IDX_LOC_OBJ_RANGE_V2;
				l_val2 = myTestCond->local_object[step][idx2];
				idx3 = IDX_LOC_OBJ_RANGE_I;
				l_val3 = myTestCond->local_object[step][idx3];
				userlog(DEBUG_LOG, psName,
					"refV(rangeV)[%ld (%ld) : %ld (%ld)], refI(rangeI):%ld (%ld), refP: %ld, refR: %ld\n",
					refV1, l_val, refV2, l_val2, refI, l_val3, refP, refR);
				*/
					
				idx = IDX_LOC_OBJ_END_AMPARE_HOUR_UPPER;
				l_val = myTestCond->local_object[step][idx];
				idx2 = IDX_LOC_OBJ_END_AMPARE_HOUR_UPPER_BRANCH;
				l_val2 = myTestCond->local_object[step][idx2];
				userlog(DEBUG_LOG, psName, "END_AMPARE_HOUR_UPPER: %ld, END_AMPARE_HOUR_UPPER_BRANCH: %ld\n",
					l_val, l_val2);

				idx = IDX_LOC_OBJ_END_AMPARE_HOUR_LOWER;
				l_val = myTestCond->local_object[step][idx];
				idx2 = IDX_LOC_OBJ_END_AMPARE_HOUR_LOWER_BRANCH;
				l_val2 = myTestCond->local_object[step][idx2];
				userlog(DEBUG_LOG, psName, "END_AMPARE_HOUR_LOWER: %ld, END_AMPARE_HOUR_LOWER_BRANCH: %ld\n",
					l_val, l_val2);

				idx = IDX_LOC_OBJ_END_WATT_HOUR_UPPER;
				l_val = myTestCond->local_object[step][idx];
				idx2 = IDX_LOC_OBJ_END_WATT_HOUR_UPPER_BRANCH;
				l_val2 = myTestCond->local_object[step][idx2];
				userlog(DEBUG_LOG, psName, "END_WATT_HOUR_UPPER: %ld, END_WATT_HOUR_UPPER_BRANCH: %ld\n",
					l_val, l_val2);

				idx = IDX_LOC_OBJ_END_WATT_HOUR_LOWER;
				l_val = myTestCond->local_object[step][idx];
				idx2 = IDX_LOC_OBJ_END_WATT_HOUR_LOWER_BRANCH;
				l_val2 = myTestCond->local_object[step][idx2];
				userlog(DEBUG_LOG, psName, "END_WATT_HOUR_LOWER: %ld, END_WATT_HOUR_LOWER_BRANCH: %ld\n",
					l_val, l_val2);
			}
		}
	}
	//csk_190213e for_debug	

}

int convert_test_cond_step_default(S_P1_TEST_COND_STEP *P1_stepCond, int ch, int step, long type, long attr_count, int div)
{
	int rtn, idx, idx2, i, j, grade, fault_count, use_flag;
	int slave_idx, slave_ch;		//csk_190712

	idx2 = 0;

	//end_condition
	if(type == STEP_OCV) {
		make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
			C_CD_END_OCV, 1, COMP_NONE, 0);
	}

	idx2 = IDX_LOC_OBJ_END_TIME_DAY;
	myTestCond->local_object[step][idx2] = P1_stepCond->reference[0]
		.endTime_day;
	idx = IDX_LOC_OBJ_END_TIME;
	myTestCond->local_object[step][idx] = P1_stepCond->reference[0].endTime;
	make_loc_chCode_cond2((S_TEST_CONDITION *)myTestCond, step,
		C_CD_END_TIME, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx, idx2); //>=

	idx = IDX_LOC_OBJ_END_TIME_BRANCH;
	myTestCond->local_object[step][idx] = P1_stepCond->reference[0]
		.Time_Branch_AccCycleCount;
	if(myTestCond->local_object[step][idx] == ACTIVE_DIV_NONE)
		myTestCond->local_object[step][idx] = ACTIVE_DIV_NEXT;

	idx2 = IDX_LOC_OBJ_END_CV_TIME_DAY;
	myTestCond->local_object[step][idx2] = P1_stepCond->reference[0].CVTime_day;
	idx = IDX_LOC_OBJ_END_CV_TIME;
	myTestCond->local_object[step][idx] = P1_stepCond->reference[0].CVTime;
	make_loc_chCode_cond2((S_TEST_CONDITION *)myTestCond, step,
		C_CD_END_CV_TIME, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx, idx2); //>=

	idx = IDX_LOC_OBJ_END_CV_TIME_BRANCH;
	myTestCond->local_object[step][idx] = P1_stepCond->reference[0]
		.CVTime_Branch_AccCycleCountId;
	if(myTestCond->local_object[step][idx] == ACTIVE_DIV_NONE)
		myTestCond->local_object[step][idx] = ACTIVE_DIV_NEXT;

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

	idx = IDX_LOC_OBJ_FAULT_CAN_CHECK_FLAG; //kjhw_141208
	myTestCond->local_object[step][idx] = P1_stepCond->fault_can_check_flag;

	idx = IDX_LOC_OBJ_FAULT_CAN_TX_STOP; //kjhw_141208
	myTestCond->local_object[step][idx] = P1_stepCond->can_tx_stop;

	idx = IDX_LOC_OBJ_CHAMBER_STEP_CHECK_FLAG; //kjhw_150821
	myTestCond->local_object[step][idx] = P1_stepCond->reference[0]
		.chamber_step_check_flag;
	if(myData->ChAttribute[ch].opType != OP_INDEPENDENT) {	//csk_190712s
		for(slave_idx = 0; slave_idx < MAX_SLAVE_CH; slave_idx++) {
			slave_ch = (int)myData->ChAttribute[ch].chNo_slave[slave_idx] - 1;
			if(slave_ch >= 0) {
				myData->testCond[slave_ch].local_object[step][idx]
					= myTestCond->local_object[step][idx];
			}
		}
	}														//csk_190712e

	idx = IDX_LOC_OBJ_CAN_COMM_STEP_CHECK_FLAG; //kjhw_150821
	myTestCond->local_object[step][idx] = P1_stepCond->can_comm_step_check_flag;

	idx = IDX_LOC_OBJ_FAULT_BALANCE_CHECK_FLAG; //csk_190114
	myTestCond->local_object[step][idx] = P1_stepCond->fault_balance_check_flag;

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

	idx = IDX_LOC_OBJ_FAULT_UPPER_TEMP;
	myTestCond->local_object[step][idx] = P1_stepCond->faultUpperTemp;
	make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
		C_CD_FAULT_UPPER_TEMP, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_LOC_OBJ_FAULT_LOWER_TEMP;
	myTestCond->local_object[step][idx] = P1_stepCond->faultLowerTemp;
	make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
		C_CD_FAULT_LOWER_TEMP, 1, COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=

	//csk_190527s
	idx = IDX_LOC_OBJ_FAULT_COMP_AUX_V;
	myTestCond->local_object[step][idx] = P1_stepCond->faultCompAuxV;
	make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
		C_CD_FAULT_COMP_AUX_V_STEP, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_LOC_OBJ_FAULT_COMP_AUX_V_VENT_FLAG;
	myTestCond->local_object[step][idx] = P1_stepCond->faultCompAuxV_vent_flag;
	//csk_190527e

	//csk_190808s
	idx = IDX_LOC_OBJ_FAULT_COMP_AUX_TEMP;
	myTestCond->local_object[step][idx] = P1_stepCond->faultCompAuxTemp;
	make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
		C_CD_FAULT_COMP_AUX_TEMP, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_LOC_OBJ_FAULT_COMP_AUX_TEMP_VENT_FLAG;
	myTestCond->local_object[step][idx] = P1_stepCond->faultCompAuxTemp_vent_flag;

	idx = IDX_LOC_OBJ_FAULT_COMP_AUX_TH;
	myTestCond->local_object[step][idx] = P1_stepCond->faultCompAuxTh;
	make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
		C_CD_FAULT_COMP_AUX_TH, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_LOC_OBJ_FAULT_COMP_AUX_TH_VENT_FLAG;
	myTestCond->local_object[step][idx] = P1_stepCond->faultCompAuxTh_vent_flag;

	idx = IDX_LOC_OBJ_FAULT_COMP_AUX_T;
	myTestCond->local_object[step][idx] = P1_stepCond->faultCompAuxT;
	make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
		C_CD_FAULT_COMP_AUX_T, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_LOC_OBJ_FAULT_COMP_AUX_T_VENT_FLAG;
	myTestCond->local_object[step][idx] = P1_stepCond->faultCompAuxT_vent_flag;

	idx = IDX_LOC_OBJ_FAULT_DELTA_AUX_V;
	myTestCond->local_object[step][idx] = P1_stepCond->faultDelta_AuxV;
	make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
		C_CD_FAULT_DELTA_AUX_V, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_LOC_OBJ_FAULT_DELTA_AUX_V_T;
	myTestCond->local_object[step][idx] = P1_stepCond->faultDelta_AuxV_T;

	idx = IDX_LOC_OBJ_FAULT_DELTA_AUX_V_VENT_FLAG;
	myTestCond->local_object[step][idx] = P1_stepCond->faultDelta_AuxV_vent_flag;
	//csk_190808e

	idx = IDX_LOC_OBJ_FAULT_CAN_COMP_CAN_V_USE_FLAG;
	myTestCond->local_object[step][idx] = P1_stepCond->faultCompCanV_use_flag;	//ktg_210706s

	idx = IDX_LOC_OBJ_FAULT_CAN_COMP_CAN_V_VENT_FLAG;
	myTestCond->local_object[step][idx] = P1_stepCond->faultCompCanV_vent_flag;

	idx = IDX_LOC_OBJ_FAULT_CAN_COMP_CAN_V_VALUE;
	myTestCond->local_object[step][idx] = P1_stepCond->faultCompCanV_value;
	make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
		C_CD_CAN_FAULT_COMP_V, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=		//ktg_210706e

	idx = IDX_LOC_OBJ_FAULT_AUX_SOFTVENT_USE_FLAG;	//ktg_220331s	soft aux
	myTestCond->local_object[step][idx] = P1_stepCond->faultSoftvent_use_flag;

	idx = IDX_LOC_OBJ_FAULT_AUX_SOFTVENT_VENT_FLAG;
	myTestCond->local_object[step][idx] = P1_stepCond->faultSoftvent_vent_flag;
	
	idx = IDX_LOC_OBJ_FAULT_AUX_SOFTVENT_COUNT;
	myTestCond->local_object[step][idx] = P1_stepCond->faultSoftvent_count;
	
	idx = IDX_LOC_OBJ_FAULT_AUX_SOFTVENT_VALUE;
	myTestCond->local_object[step][idx] = P1_stepCond->faultSoftvent_value;

	idx = IDX_LOC_OBJ_FAULT_AUX_HARDVENT_USE_FLAG;	//hard aux
	myTestCond->local_object[step][idx] = P1_stepCond->faultHardvent_use_flag;

	idx = IDX_LOC_OBJ_FAULT_AUX_HARDVENT_VENT_FLAG;
	myTestCond->local_object[step][idx] = P1_stepCond->faultHardvent_vent_flag;
	
	idx = IDX_LOC_OBJ_FAULT_AUX_HARDVENT_COUNT;
	myTestCond->local_object[step][idx] = P1_stepCond->faultHardvent_count;
	
	idx = IDX_LOC_OBJ_FAULT_AUX_HARDVENT_VALUE;
	myTestCond->local_object[step][idx] = P1_stepCond->faultHardvent_value;

	idx = IDX_LOC_OBJ_FAULT_CAN_SOFTVENT_USE_FLAG;	//soft can
	myTestCond->local_object[step][idx] = P1_stepCond->faultSoftvent_can_use_flag;

	idx = IDX_LOC_OBJ_FAULT_CAN_SOFTVENT_VENT_FLAG;
	myTestCond->local_object[step][idx] = P1_stepCond->faultSoftvent_can_vent_flag;
	
	idx = IDX_LOC_OBJ_FAULT_CAN_SOFTVENT_COUNT;
	myTestCond->local_object[step][idx] = P1_stepCond->faultSoftvent_can_count;
	
	idx = IDX_LOC_OBJ_FAULT_CAN_SOFTVENT_VALUE;
	myTestCond->local_object[step][idx] = P1_stepCond->faultSoftvent_can_value;

	idx = IDX_LOC_OBJ_FAULT_CAN_HARDVENT_USE_FLAG;	//hard can
	myTestCond->local_object[step][idx] = P1_stepCond->faultHardvent_can_use_flag;

	idx = IDX_LOC_OBJ_FAULT_CAN_HARDVENT_VENT_FLAG;
	myTestCond->local_object[step][idx] = P1_stepCond->faultHardvent_can_vent_flag;
	
	idx = IDX_LOC_OBJ_FAULT_CAN_HARDVENT_COUNT;
	myTestCond->local_object[step][idx] = P1_stepCond->faultHardvent_can_count;
	
	idx = IDX_LOC_OBJ_FAULT_CAN_HARDVENT_VALUE;
	myTestCond->local_object[step][idx] = P1_stepCond->faultHardvent_can_value;	//ktg_220331e

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
			//convert_test_cond_charge_discharge_z(P1_stepCond, ch, step, type,
			//	attr_count);
			//ktg_210310s
			rtn = convert_test_cond_charge_discharge_z(P1_stepCond, ch, step, type,
				attr_count);
			if(rtn < 0) return rtn;
			//ktg_210310e
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
		case STEP_USERMAP: //kjhw_140828
			rtn = convert_test_cond_usermap(P1_stepCond, ch, step, type,
				attr_count, div);
			if(rtn < 0) return rtn;
			break;
		default: break;
	}

	return 0;
}

void convert_test_cond_step_aux(S_P1_TEST_COND_STEP *P1_stepCond, int ch, int step)
{
	int idx, i, compare_type, code, branch;
	int delay_time; //kjhw_171117
	long func_div, data_type, data_value;

	for(i=0; i < MAX_P1_AUX_FUNCTION; i++) {
		func_div = (long)P1_stepCond->reference[0].aux_func_div[i];

		idx = IDX_LOC_OBJ_AUX_FUNC_DIV_TMP_1 + i; //kjg_170810_s
		myTestCond->local_object[step][idx] = func_div;

		idx = IDX_LOC_OBJ_AUX_COMP_TYPE_TMP_1 + i;
		myTestCond->local_object[step][idx]
			= (long)P1_stepCond->reference[0].aux_compare_type[i];

		idx = IDX_LOC_OBJ_AUX_VALUE_TMP_1 + i;
		//myTestCond->local_object[step][idx]
		//	= (long)(P1_stepCond->reference[0].aux_value[i] * 1000.0);
		myTestCond->local_object[step][idx] = P1_stepCond->reference[0].aux_value[i];

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

		/*if(data_type == 2) { //1:signed, 2:float
			//data_value = (long)(P1_stepCond->reference[0].aux_value[i]
			//	* 1000.0);
			tmp_f = P1_stepCond->reference[0].aux_value[i] * 1000.0;
			data_value = (long)tmp_f; //kjh_190709
		} else {
			data_value = (long)P1_stepCond->reference[0].aux_value[i];
		}*/
		data_value = P1_stepCond->reference[0].aux_value[i];
		//kjhw_170828s
		//delay_time = (long)P1_stepCond->reference[0].aux_delay_time[i]; //kjh_171117
		delay_time = (int)P1_stepCond->reference[0].aux_delay_time[i];
		delay_time *= 100;
		//if(delay_time > 1000) delay_time = 1000;
		if(delay_time > 6000) delay_time = 6000;	//ktg_220110
		else if(delay_time <= 0) delay_time = 1;

		idx = IDX_LOC_OBJ_AUX_DELAY_TIME_1 + i;
		//myTestCond->local_object[step][idx] = delay_time;  //kjh_171117
		myTestCond->local_object[step][idx] = (long)delay_time;
		//kjhw_170828s

		//jhkw_211014s
		//if(func_div == AUX_FUNC_DIV_CV_SELECT) {
		if(func_div == AUX_FUNC_DIV_CV_SELECT
			|| func_div == AUX_FUNC_DIV_MAX_CELL_CV) {
			if(func_div == AUX_FUNC_DIV_MAX_CELL_CV) {
				idx = IDX_LOC_OBJ_MAX_AUX_CELL_CV;
				myTestCond->local_object[step][idx] = 1;
			}
		//jhkw_211014e
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
	int delay_time;		//khk_201120
	long func_div, data_type, data_value;
	float tmp_f = 0.0; //kjh_190709

	for(i=0; i < MAX_P1_CAN_FUNCTION; i++) {
		func_div = (long)P1_stepCond->reference[0].can_func_div[i];

		idx = IDX_LOC_OBJ_CAN_FUNC_DIV_TMP_1 + i; //kjg_170810_s
		myTestCond->local_object[step][idx] = func_div;

		idx = IDX_LOC_OBJ_CAN_COMP_TYPE_TMP_1 + i;
		myTestCond->local_object[step][idx]
			= (long)P1_stepCond->reference[0].can_compare_type[i];

		idx = IDX_LOC_OBJ_CAN_VALUE_TMP_1 + i;
		//myTestCond->local_object[step][idx]
		//	= (long)(P1_stepCond->reference[0].can_value[i] * 1000.0);
		tmp_f = P1_stepCond->reference[0].can_value[i] * 1000.0;
		myTestCond->local_object[step][idx] = (long)tmp_f; //kjh_190709

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
			//data_value = (long)(P1_stepCond->reference[0].can_value[i]
			//	* 1000.0);
			tmp_f = P1_stepCond->reference[0].can_value[i] * 1000.0;
			data_value = (long)tmp_f; //kjh_190709
		} else {
			data_value = (long)P1_stepCond->reference[0].can_value[i];
		}
		
		//khj_201120
		delay_time = (int)P1_stepCond->reference[0].can_delay_time[i];
		delay_time *= 100;
		//if(delay_time > 1000) delay_time = 1000;
		if(delay_time > 6000) delay_time = 6000;	//ktg_220110
		else if(delay_time <= 0) delay_time = 1;
		
		idx = IDX_LOC_OBJ_CAN_DELAY_TIME_1 + i;
		myTestCond->local_object[step][idx] = (long)delay_time;
		//khj_201120
		
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
				|| func_div == CAN_RX_FUNC_DIV_ISOLATION_ON		//ktg_210908	//ktg_220701
				|| func_div == CAN_RX_FUNC_DIV_ISOLATION_OFF	//ktg_210908	//ktg_220701
				) {
				make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
					code, 100, compare_type, idx);
			} else {
				make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
					code, delay_time, compare_type, idx);		//khj_201120
				//	code, 1, compare_type, idx);
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

	//fault
	if(type == STEP_REST) {	//ktg_210105s
		idx = IDX_LOC_OBJ_FAULT_SVDF_VENT_FLAG;
		myTestCond->local_object[step][idx] = P1_stepCond->faultSVDF_vent_flag;

		idx = IDX_LOC_OBJ_FAULT_SVDF_COUNT;
		myTestCond->local_object[step][idx] = P1_stepCond->faultSVDF_count;

		idx = IDX_LOC_OBJ_FAULT_SVDF_YT_1;
		myTestCond->local_object[step][idx] = P1_stepCond->faultSVDF_yt1;
	
		idx = IDX_LOC_OBJ_FAULT_SVDF_VCELL_AVER;
		myTestCond->local_object[step][idx] = P1_stepCond->faultSVDF_vcell_aver;
	
		idx = IDX_LOC_OBJ_FAULT_SVDF_YT_2;
		myTestCond->local_object[step][idx] = P1_stepCond->faultSVDF_yt2;
	
		idx = IDX_LOC_OBJ_FAULT_SVDF_YT_3;
		myTestCond->local_object[step][idx] = P1_stepCond->faultSVDF_yt3;
	
		idx = IDX_LOC_OBJ_FAULT_SVDF_USE_FLAG;
		myTestCond->local_object[step][idx] = P1_stepCond->faultSVDF_use_flag;

		idx = IDX_LOC_OBJ_FAULT_CAN_SVDF_VENT_FLAG;	//ktg_210706s
		myTestCond->local_object[step][idx] = P1_stepCond->faultSVDF_can_vent_flag;

		idx = IDX_LOC_OBJ_FAULT_CAN_SVDF_COUNT;
		myTestCond->local_object[step][idx] = P1_stepCond->faultSVDF_can_count;

		idx = IDX_LOC_OBJ_FAULT_CAN_SVDF_YT_1;
		myTestCond->local_object[step][idx] = P1_stepCond->faultSVDF_can_yt1;
	
		idx = IDX_LOC_OBJ_FAULT_CAN_SVDF_VCELL_AVER;
		myTestCond->local_object[step][idx] = P1_stepCond->faultSVDF_can_vcell_aver;
	
		idx = IDX_LOC_OBJ_FAULT_CAN_SVDF_YT_2;
		myTestCond->local_object[step][idx] = P1_stepCond->faultSVDF_can_yt2;
	
		idx = IDX_LOC_OBJ_FAULT_CAN_SVDF_YT_3;
		myTestCond->local_object[step][idx] = P1_stepCond->faultSVDF_can_yt3;
	
		idx = IDX_LOC_OBJ_FAULT_CAN_SVDF_USE_FLAG;
		myTestCond->local_object[step][idx] = P1_stepCond->faultSVDF_can_use_flag;	//ktg_210706e
		idx = IDX_LOC_OBJ_FAULT_V_RISING_TIME_INIT;
		myTestCond->local_object[step][idx] = P1_stepCond->faultVrising_time_init;	//shhw_220916s
		
		idx = IDX_LOC_OBJ_FAULT_V_RISING_TIME_CHECK;
		myTestCond->local_object[step][idx] = P1_stepCond->faultVrising_time_check;  		
		
		idx = IDX_LOC_OBJ_FAULT_V_RISING_REFF_VALUE;
		myTestCond->local_object[step][idx] = P1_stepCond->faultVrising_reff_value;	
		make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,					
	    	C_CD_FAULT_V_RISING, 1, COMP_GREATER_THAN, idx);						//shhw_220916e 
	}	//ktg_210105e
}

int convert_test_cond_charge_discharge_z(S_P1_TEST_COND_STEP *P1_stepCond, int ch, int step, long type, long attr_count)
{

	int i, func_div;
	int idx, idx2, fault_count;
	int idx3, debug_mode, debug_step;	//csk_190213
	long mode, refV1, refV2, refI, l_val, l_val2, tmp, refP, refR;
	long l_val3;	//csk_190213
	long maxI, minI;		//csk_190718
	//unsigned long dv_time, di_time;	//jhkw_180823
	double d_val1, d_val2;
	double max_power;	//ktg_210310
	long ratioV, ratioI; //kjh_211021
	double ratioP; //kjh_211021

	ratioV = myData->mData.ratioV;	//kjh_220321
	ratioI = myData->mData.ratioI;	//kjh_220321
	ratioP = myData->mData.ratioP;	//kjh_220321

	max_power = 0;		//ktg_210310
	if(myData->mData.config.maxP[attr_count-1] != 0) 
		max_power = (double)myData->mData.config.maxP[attr_count-1];	//ktg_210310
	//dv_time = di_time = 0;	//jhkw_180823
	refP = refR = 0;
	maxI = myData->mData.config.maxI[0] * attr_count;		//csk_190718s
	if(maxI < 0) maxI = MAX_SUM_ULONG;
	minI = myData->mData.config.minI[0] * attr_count;
	if(minI > 0) minI = MAX_SUM_ULONG * (-1);				//csk_190718e

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

	if(type == STEP_CHARGE) {
		if(mode == MODE_CP) {
			refP = P1_stepCond->reference[0].refP;
			if(refP < 0) refP = 0;

			if(max_power != 0) {	//ktg_210310s
				d_val2 = (double)refP;
				if(d_val2 > max_power) {
				userlog(DEBUG_LOG, psName, "ch%d step converter error (watt limit) %lf\n",ch+1, max_power);
				return -1;
				}
			}	//ktg_210310e
			refI = maxI;	//csk_190718
			//d_val1 = (double)(refV1 / 1000) * (double)(refI / 1000) / 1000.0;
			d_val1 = (double)(refV1 / ratioV) * (double)(refI / ratioI) / ratioP;	//kjh_211021
			if(d_val1 > MAX_SUM_ULONG) d_val1 = MAX_SUM_ULONG;	//ktg_210310
			if(refP > (long)d_val1) refP = (long)d_val1;
			idx = IDX_LOC_OBJ_REF_P;
			myTestCond->local_object[step][idx] = refP;
		} else if(mode == MODE_CP_CC) {
			refP = P1_stepCond->reference[0].refP;
			if(refP < 0) refP = 0;

			refI = P1_stepCond->reference[0].refI;
			if(refI > maxI) {	//csk_190718s
				refI = maxI;
			} else if(refI < 0) refI = 0;	//csk_190718e

			//d_val1 = (double)(refV1 / 1000) * (double)(refI / 1000) / 1000.0;
			d_val1 = (double)(refV1 / ratioV) * (double)(refI / ratioI) / ratioP;	//kjh_211021
			if(refP <= (long)d_val1) d_val1 = (long)refP;

			idx = IDX_LOC_OBJ_REF_P;
			myTestCond->local_object[step][idx] = (long)d_val1;
			d_val2 = d_val1 / (double)refV1;
			//d_val2 *= 1000000000.0;
			d_val2 *= myData->mData.cp_to_cc;	//kjh_220321

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
			d_val2 = (double)maxI;		//csk_190718

			if(d_val1 > d_val2) d_val1 = d_val2;
			refI = (long)d_val1;
		} else {
			refI = P1_stepCond->reference[0].refI;
		}
	} else { //STEP_DISCHARGE, STEP_Z
		if(mode == MODE_CP) {
			refP = P1_stepCond->reference[0].refP;
			//refI = myData->mData.config.minI[0] * (-1) * attr_count;
			if(max_power != 0) {	//ktg_210310s
				d_val2 = (double)refP;
				if(d_val2 > max_power) {
				userlog(DEBUG_LOG, psName, "ch%d step converter error (watt limit) %lf\n",ch+1, max_power);
				return -2;
				}
			}	//ktg_210310e

			refI = minI * (-1);		//csk_190718
			//d_val1 = (double)(myData->mData.config.maxV[0] / 1000)
			//	* (double)(refI / 1000) / 1000.0;
			d_val1 = (double)(myData->mData.config.maxV[0] / ratioV)
				* (double)(refI / ratioI) / ratioP;	//kjh_211021
			if(d_val1 > MAX_SUM_ULONG) d_val1 = MAX_SUM_ULONG;	//ktg_210310
			if(refP > (long)d_val1) refP = (long)d_val1;
			idx = IDX_LOC_OBJ_REF_P;
			myTestCond->local_object[step][idx] = refP;
		} else if(mode == MODE_CP_CC) {
			refP = P1_stepCond->reference[0].refP;
			refI = P1_stepCond->reference[0].refI;

			if(refI > (minI * (-1))) refI = (minI * (-1));	//csk_190718
			else if(refI < 0) refI = 0;						//csk_190718e

			//d_val1 = (double)(myData->mData.config.maxV[0] / 1000)
			//	* (double)(refI / 1000) / 1000.0;
			d_val1 = (double)(myData->mData.config.maxV[0] / ratioV)
				* (double)(refI / ratioI) / ratioP;	//kjh_211021
			if(refP > (long)d_val1) refP = (long)d_val1;
			idx = IDX_LOC_OBJ_REF_P;
			myTestCond->local_object[step][idx] = refP;
			//kjh_220321	if(refV2 <= 1000) { //1mV
			//	d_val2 = (double)refP / 1000.0; //1mV
			//} else {
			//	d_val2 = (double)refP / (double)refV2;
			//}
			//d_val2 *= 1000000000.0;
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

			d_val2 *= myData->mData.cp_to_cc;	//kjh_220321

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
			d_val2 = (double)(minI * (-1));		//csk_190718

			if(d_val1 > d_val2) d_val1 = d_val2;
			refI = (long)d_val1;
		} else {
			refI = P1_stepCond->reference[0].refI;

			if(refI > (minI * (-1))) {	//csk_190718s
				refI = minI * (-1);
			} else if(refI < 0) refI = 0;	//csk_190718e
		}
		refI *= (-1);
	}

	if(refI > maxI) {	//csk_190718s
		if(myData->mData.config.maxI[0] != 0) {
			refI = maxI;
		}
	} else if(refI < minI) {
		if(myData->mData.config.minI[0] != 0) {
			refI = minI;
		}
	}	//csk_190718e

	idx = IDX_LOC_OBJ_REF_I;
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
	} else if(tmp == P1_VALUE_RATE_ITEM_CAN_AMPARE_HOUR) {	//ktg_220829s
		idx = IDX_LOC_OBJ_END_CAN_AMPARE_HOUR_RATE;
		myTestCond->local_object[step][idx] = P1_stepCond->reference[0]
			.ValueRate;
		make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
			C_CD_END_VALUE_RATE_CAN_AMPARE_HOUR, 1,
			COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

		idx = IDX_LOC_OBJ_END_CAN_AMPARE_HOUR_RATE_BRANCH;
		myTestCond->local_object[step][idx] = P1_stepCond->reference[0]
			.ValueRate_Branch;
		if(myTestCond->local_object[step][idx] == ACTIVE_DIV_NONE)
			myTestCond->local_object[step][idx] = ACTIVE_DIV_NEXT;

		idx = IDX_LOC_OBJ_END_CAN_AMPARE_HOUR_RATE_COMPARE;
		myTestCond->local_object[step][idx] = P1_stepCond->reference[0]
			.ValueRate_Compare;
	} else if(tmp == P1_VALUE_RATE_ITEM_CAN_WATT_HOUR) {
		idx = IDX_LOC_OBJ_END_CAN_WATT_HOUR_RATE;
		myTestCond->local_object[step][idx] = P1_stepCond->reference[0]
			.ValueRate;
		make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
			C_CD_END_VALUE_RATE_CAN_WATT_HOUR, 1,
			COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

		idx = IDX_LOC_OBJ_END_CAN_WATT_HOUR_RATE_BRANCH;
		myTestCond->local_object[step][idx] = P1_stepCond->reference[0]
			.ValueRate_Branch;
		if(myTestCond->local_object[step][idx] == ACTIVE_DIV_NONE)
			myTestCond->local_object[step][idx] = ACTIVE_DIV_NEXT;

		idx = IDX_LOC_OBJ_END_CAN_WATT_HOUR_RATE_COMPARE;
		myTestCond->local_object[step][idx] = P1_stepCond->reference[0]
			.ValueRate_Compare;		
	}	//ktg_220829e

	if(type == STEP_CHARGE) {
		idx = IDX_LOC_OBJ_END_I;
		myTestCond->local_object[step][idx] = P1_stepCond->reference[0].Current;
		make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
			C_CD_END_I, 1, COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=
	} else if(type == STEP_DISCHARGE || type == STEP_Z) {
		idx = IDX_LOC_OBJ_END_I;
		myTestCond->local_object[step][idx] = P1_stepCond->reference[0]
			.Current * (-1);
		make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
			C_CD_END_I, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=
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

	idx = IDX_LOC_OBJ_FAULT_HW_LOWER_I; //kjhw_130912
	myTestCond->local_object[step][idx] = 5000; //5.0%
	make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
		C_CD_FAULT_HW_LOWER_I, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

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
	
	//ktg_210315s
	if((type == STEP_CHARGE) || (type == STEP_DISCHARGE)) {
		if((mode == MODE_CC) || (mode == MODE_CC_CV) || (mode == MODE_CV) || (mode == MODE_CP)) {
			idx = IDX_LOC_OBJ_FAULT_CV_INTERLOCK_I;
			myTestCond->local_object[step][idx] = P1_stepCond->faultcv_interlockI;
			make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
				C_CD_FAULT_CV_INTERLOCK_I, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=
			idx = IDX_LOC_OBJ_FAULT_CV_INTERLOCK_VENT_FLAG;
			myTestCond->local_object[step][idx] = P1_stepCond->faultcv_interlock_vent_flag;
	
			idx = IDX_LOC_OBJ_FAULT_DROP_VOLTAGE_VALUE;		//ktg_210408s
			myTestCond->local_object[step][idx] = P1_stepCond->faultDropVoltage_value;
			idx = IDX_LOC_OBJ_FAULT_DROP_VOLTAGE_USE_FLAG;
			myTestCond->local_object[step][idx] = P1_stepCond->faultDropVoltage_use_flag;
			idx = IDX_LOC_OBJ_FAULT_DROP_VOLTAGE_VENT_FLAG;
			myTestCond->local_object[step][idx] = P1_stepCond->faultDropVoltage_vent_flag;
			idx = IDX_LOC_OBJ_FAULT_DROP_VOLTAGE_COUNT;
			myTestCond->local_object[step][idx] = P1_stepCond->faultDropVoltage_count;	//ktg_210408e

			idx = IDX_LOC_OBJ_FAULT_CAN_DROP_VOLTAGE_VALUE;		//ktg_210706s
			myTestCond->local_object[step][idx] = P1_stepCond->faultDropVoltage_can_value;
			idx = IDX_LOC_OBJ_FAULT_CAN_DROP_VOLTAGE_USE_FLAG;
			myTestCond->local_object[step][idx] = P1_stepCond->faultDropVoltage_can_use_flag;
			idx = IDX_LOC_OBJ_FAULT_CAN_DROP_VOLTAGE_VENT_FLAG;
			myTestCond->local_object[step][idx] = P1_stepCond->faultDropVoltage_can_vent_flag;
			idx = IDX_LOC_OBJ_FAULT_CAN_DROP_VOLTAGE_COUNT;
			myTestCond->local_object[step][idx] = P1_stepCond->faultDropVoltage_can_count;	//ktg_210706e
		}
	}
	//ktg_210315e
	//shhw_220919s
	if((type == STEP_CHARGE) || (type == STEP_DISCHARGE)) {
		if(mode == MODE_CC) {
			idx = IDX_LOC_OBJ_FAULT_CC_DELTA_V_TIME_INIT;
			myTestCond->local_object[step][idx] = P1_stepCond->faultCC_delta_v_time_init;
			idx = IDX_LOC_OBJ_FAULT_CC_DELTA_V_TIME_PERIOD;
			myTestCond->local_object[step][idx] = P1_stepCond->faultCC_delta_v_time_period;
			idx = IDX_LOC_OBJ_FAULT_CC_DELTA_V_REFF_VALUE;
			myTestCond->local_object[step][idx] = P1_stepCond->faultCC_delta_v_reff_value;
			make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
				 C_CD_FAULT_CC_DELTA_V_ERROR, 1, COMP_LESS_THAN, idx);
		}	

	}
	//shhw_220919e
	//ktg_230728s
	if((type == STEP_CHARGE) || (type == STEP_DISCHARGE) || type == STEP_PATTERN) {
		idx = IDX_LOC_OBJ_FAULT_CAN_FREEZE_USE_FLAG;
		myTestCond->local_object[step][idx] = P1_stepCond->faultCanFreeze_use_flag;
		idx = IDX_LOC_OBJ_FAULT_CAN_FREEZE_TIME;
		myTestCond->local_object[step][idx] = P1_stepCond->faultCanFreeze_time;
		idx = IDX_LOC_OBJ_FAULT_CAN_FREEZE_CELL_V;
		myTestCond->local_object[step][idx] = P1_stepCond->faultCanFreeze_CellV;
		idx = IDX_LOC_OBJ_FAULT_CAN_FREEZE_I;
		myTestCond->local_object[step][idx] = P1_stepCond->faultCanFreeze_I;
	}	
	//ktg_230827e
	//csk_190213s for_debug
	debug_mode = 0;	//0: no_use, 1~8: select debug_ch, 9: all debug_ch
	debug_step = 0; //0: all_step, 1~: select_step
	if(debug_mode == 0) {
	} else {
		if((debug_mode == ch+1) || (debug_mode == 9)) {
			if((debug_step == 0) || ((debug_step+1) == step)) {
				userlog(DEBUG_LOG, psName, "ch%d[step: %d/ mode: %ld] testCond_charge_discharg_z print",
						ch+1, step+1, mode);

				idx = IDX_LOC_OBJ_RANGE_V;
				l_val = myTestCond->local_object[step][idx];
				idx2 = IDX_LOC_OBJ_RANGE_V2;
				l_val2 = myTestCond->local_object[step][idx2];
				idx3 = IDX_LOC_OBJ_RANGE_I;
				l_val3 = myTestCond->local_object[step][idx3];
				userlog(DEBUG_LOG, psName,
					"refV(rangeV)[%ld (%ld) : %ld (%ld)], refI(rangeI):%ld (%ld), refP: %ld, refR: %ld\n",
					refV1, l_val, refV2, l_val2, refI, l_val3, refP, refR);

				idx = IDX_LOC_OBJ_END_AMPARE_HOUR;
				l_val = myTestCond->local_object[step][idx];
				idx2 = IDX_LOC_OBJ_END_AMPARE_HOUR_BRANCH;
				l_val2 = myTestCond->local_object[step][idx2];
				userlog(DEBUG_LOG, psName, "END_AMPARE_HOUR: %ld, END_AMPARE_HOUR_BRANCH: %ld\n", l_val, l_val2);

				idx = IDX_LOC_OBJ_END_WATT_HOUR;
				l_val = myTestCond->local_object[step][idx];
				idx2 = IDX_LOC_OBJ_END_WATT_HOUR_BRANCH;
				l_val2 = myTestCond->local_object[step][idx2];
				userlog(DEBUG_LOG, psName, "END_WATT_HOUR: %ld, END_WATT_HOUR_BRANCH: %ld\n", l_val, l_val2);

				idx = IDX_LOC_OBJ_END_AMPARE_HOUR_RATE;
				l_val = myTestCond->local_object[step][idx];
				idx2 = IDX_LOC_OBJ_END_AMPARE_HOUR_RATE_BRANCH;
				l_val2 = myTestCond->local_object[step][idx2];
				userlog(DEBUG_LOG, psName, "END_AMPARE_HOUR_RATE: %ld, END_AMPARE_HOUR_RATE_BRANCH: %ld\n",
					l_val, l_val2);

				idx = IDX_LOC_OBJ_END_WATT_HOUR_RATE;
				l_val = myTestCond->local_object[step][idx];
				idx2 = IDX_LOC_OBJ_END_WATT_HOUR_RATE_BRANCH;
				l_val2 = myTestCond->local_object[step][idx2];
				userlog(DEBUG_LOG, psName, "END_WATT_HOUR_RATE: %ld, END_WATT_HOUR_RATE_BRANCH: %ld\n", l_val, l_val2);

				idx = IDX_LOC_OBJ_END_I;
				l_val = myTestCond->local_object[step][idx];
				idx2 = IDX_LOC_OBJ_END_DELTA_V;
				l_val2 = myTestCond->local_object[step][idx2];
				idx3 = IDX_LOC_OBJ_END_P;
				l_val3 = myTestCond->local_object[step][idx3];
				userlog(DEBUG_LOG, psName, "END_I: %ld, END_DELTA_V: %ld, END_P: %ld\n", l_val, l_val2, l_val3);
			}
		}
	}
	//csk_190213e for_debug	

	return 0;	//ktg_210310
}

int convert_test_cond_pattern(S_P1_TEST_COND_STEP *P1_stepCond, int ch, int step, long type, long attr_count, int div)
{
	int idx, i, rtn, fault_count;
	int func_div;
	long refI2;
	long pattern_index, mode, l_val, refV, refI, tmp;
	long maxI, minI;		//csk_190718

	maxI = myData->mData.config.maxI[0] * attr_count;		//csk_190718s
	if(maxI < 0) maxI = MAX_SUM_ULONG;
	minI = myData->mData.config.minI[0] * attr_count;
	if(minI > 0) minI = MAX_SUM_ULONG * (-1);				//csk_190718e

	//header
	idx = IDX_LOC_OBJ_PATTERN_INDEX;
	pattern_index = (long)step;
	myTestCond->local_object[step][idx] = pattern_index;

	idx = IDX_LOC_OBJ_PATTERN_FILE_SIZE;
	myTestCond->local_object[step][idx] = (long)P1_stepCond->pattern_file_size;
	myData->testCond[ch].local_object[step][idx] = (long)P1_stepCond->pattern_file_size;
	idx = IDX_LOC_OBJ_PATTERN_FILE_CHECKSUM;			//jhkw_200816 //ktg_210529s //shh_210906s
	myTestCond->local_object[step][idx]
		= (long)P1_stepCond->pattern_file_checksum;
	myData->testCond[ch].local_object[step][idx]
		= (long)P1_stepCond->pattern_file_checksum;		//jhkw_200816	//ktg_210529e //shh_210906e

	idx = IDX_LOC_OBJ_PATTERN_UPDATED; //kjg_170810
	//jhkw_200517s //shh_210906s
	//myTestCond->local_object[step][idx] = (long)div;
	if(myData->COA_Client[0].signal[P1_SIG_TEST_HEADER_UPDATE_RCV] == 1) {
		myData->testCond_update.local_object[step][idx] = P1;
		myData->testCond[ch].local_object[step][idx] = P1;			//jhkw_200816
	} else {
		myData->testCond_update.local_object[step][idx] = P0;
		myData->testCond[ch].local_object[step][idx] = P0;			//jhkw_200816
	}
	//jhkw_200517e  //shh_210906e

	rtn = read_test_cond_pattern_file_coa(psName, ch, (int)pattern_index, 0);	//shh_220607t
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "ch%d pattern file error %d\n", ch+1, rtn);
		return rtn;
	}

	idx = IDX_LOC_OBJ_MODE;
	mode = convert_step_mode(CONVERT_P1_TO_ORG, (long)P1_stepCond->header.mode);
	myTestCond->local_object[step][idx] = mode;

	//reference
	switch(mode) { //kjhw_150210s
		case MODE_CV:
			/*refV = 0;	//ktg_220512s	//shh_220607s
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
			idx = IDX_LOC_OBJ_REF_V;	//ktg_220512e	//shh_220607e
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

			/*refV = myData->mData.config.maxV[0];	//ktg_220512s	//shh_220607s
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
			idx = IDX_LOC_OBJ_REF_V2; //discharge ref_v	//ktg_220512e	//shh_220607e
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

			/*refI = 0;	//ktg_220512s	//shh_220607s
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
			//if(refI > (myData->mData.config.maxI[0] * attr_count)) {
			//	if(myData->mData.config.maxI[0] != 0) {
			//		refI = myData->mData.config.maxI[0] * attr_count;
			//	}
			//} else if(refI < 0) refI = 0;
			refI = P1_stepCond->reference[0].pattern_max_refI;	//ktg_220512e	//shh_220607e
			if(refI > maxI) {				//csk_190718s
				if(myData->mData.config.maxI[0] != 0) refI = maxI;
			} else if(refI < 0) refI = 0;	//csk_190718e
			////jhkw_190106s
			//if(attr_count == 4) {
			//	if(refI > 2100000000) {
			//		if(myData->mData.config.maxI[0] != 0) {
			//			refI = 2100000000;
			//		}
			//	} else if(refI < 0) refI = 0;
			//} else {
			//	if(refI > (myData->mData.config.maxI[0] * attr_count)) {
			//		if(myData->mData.config.maxI[0] != 0) {
			//			refI = myData->mData.config.maxI[0] * attr_count;
			//		}
			//	} else if(refI < 0) refI = 0;
			//}
			////jhkw_190106e
			myTestCond->local_object[step][idx] = refI;

			/*refI2 = 0;	//ktg_220512s	//shh_220607s
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
			refI2 = P1_stepCond->reference[0].pattern_min_refI;	//ktg_220512e	//shh_220607e
			//if(refI2 < (myData->mData.config.minI[0] * attr_count)) {
			//	if(myData->mData.config.minI[0] != 0) {
			//		refI2 = myData->mData.config.minI[0] * attr_count;
			//	}
			//} else if(refI2 > 0) refI2 = 0;
			if(refI2 < minI) {					//csk_190718s
				if(myData->mData.config.minI[0] != 0) refI2 = minI;
			} else if(refI2 > 0) refI2 = 0;		//csk_190718e

			////jhkw_190106s
			//if(attr_count == 4) {
			//	if(refI2 < -2100000000) {
			//		if(myData->mData.config.minI[0] != 0) {
			//			refI2 = -2100000000;
			//		}
			//	} else if(refI2 > 0) refI2 = 0;
			//} else {
			//	if(refI2 < (myData->mData.config.minI[0] * attr_count)) {
			//		if(myData->mData.config.minI[0] != 0) {
			//			refI2 = myData->mData.config.minI[0] * attr_count;
			//		}
			//	} else if(refI2 > 0) refI2 = 0;
			//}
			////jhkw_190106e
			//if(refI < refI2 * (-1)) refI = refI2;
			myTestCond->local_object[step][idx] = refI2;
			if(refI < refI2 * (-1)) refI = (refI2 * (-1));	//csk_190602

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
				/*refI = 0;	//ktg_220512s	//shh_220607s
				for(i=1; i < MAX_PATTERN_DATA; i++) {
					//jhkw_161026s
					if(myTestCond->pattern[0].t_val == 2) {
					} else {
						l_val = myTestCond->pattern[i].t_val;
						if(l_val < 0) break;
					} //jhkw_161026e

					l_val = myTestCond->pattern[i].cmd_val[0];
					if(l_val < 0) l_val *= (-1);

					if(l_val > refI) refI = l_val;
				}*/
				refI = P1_stepCond->reference[0].pattern_max_refI;
				refI2 = P1_stepCond->reference[0].pattern_min_refI;
				refI2 *= (-1);
				if(refI < refI2) {
					refI = refI2;
				}	//ktg_220512e	//shh_220607e
			} else { //MODE_CP
				//refI = myData->mData.config.maxI[0] * attr_count;
				refI = maxI;		//csk_190718

				////jhkw_190106s
				//if(attr_count == 4) {
				//	refI = 2100000000;
				//} else {
				//	refI = myData->mData.config.maxI[0] * attr_count;
				//}
				////jhkw_190106e
			}

			idx = IDX_LOC_OBJ_REF_I;
			//if(refI > (myData->mData.config.maxI[0] * attr_count)) {
			//	if(myData->mData.config.maxI[0] != 0) {
			//		refI = myData->mData.config.maxI[0] * attr_count;
			//	}
			//} else if(refI < (myData->mData.config.minI[0] * attr_count)) {
			//	if(myData->mData.config.minI[0] != 0) {
			//		refI = myData->mData.config.minI[0] * attr_count;
			//	}
			//}
			if(refI > maxI) {			//csk_190718s
				if(myData->mData.config.maxI[0] != 0) refI = maxI;
			} else if(refI < minI) {
				if(myData->mData.config.minI[0] != 0) refI = minI;
			}							//csk_190718e

			////jhkw_190106s
			//if(attr_count == 4) {
			//	if(refI > 2100000000) {
			//		if(myData->mData.config.maxI[0] != 0) {
			//			refI = 2100000000;
			//		}
			//	} else if(refI < -2100000000) {
			//		if(myData->mData.config.minI[0] != 0) {
			//			refI = -2100000000;
			//		}
			//	}
			//} else {
			//	if(refI > (myData->mData.config.maxI[0] * attr_count)) {
			//		if(myData->mData.config.maxI[0] != 0) {
			//			refI = myData->mData.config.maxI[0] * attr_count;
			//		}
			//	} else if(refI < (myData->mData.config.minI[0] * attr_count)) {
			//		if(myData->mData.config.minI[0] != 0) {
			//			refI = myData->mData.config.minI[0] * attr_count;
			//		}
			//	}
			//}
			////jhkw_190106e
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
	} else if(tmp == P1_VALUE_RATE_ITEM_CAN_AMPARE_HOUR) {	//ktg_220829s
		idx = IDX_LOC_OBJ_END_CAN_AMPARE_HOUR_RATE;
		myTestCond->local_object[step][idx] = P1_stepCond->reference[0]
			.ValueRate;
		make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
			C_CD_END_VALUE_RATE_CAN_AMPARE_HOUR, 1,
			COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

		idx = IDX_LOC_OBJ_END_CAN_AMPARE_HOUR_RATE_BRANCH;
		myTestCond->local_object[step][idx] = P1_stepCond->reference[0]
			.ValueRate_Branch;
		if(myTestCond->local_object[step][idx] == ACTIVE_DIV_NONE)
			myTestCond->local_object[step][idx] = ACTIVE_DIV_NEXT;

		idx = IDX_LOC_OBJ_END_CAN_AMPARE_HOUR_RATE_COMPARE;
		myTestCond->local_object[step][idx] = P1_stepCond->reference[0]
			.ValueRate_Compare;
	} else if(tmp == P1_VALUE_RATE_ITEM_CAN_WATT_HOUR) {
		idx = IDX_LOC_OBJ_END_CAN_WATT_HOUR_RATE;
		myTestCond->local_object[step][idx] = P1_stepCond->reference[0]
			.ValueRate;
		make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
			C_CD_END_VALUE_RATE_CAN_WATT_HOUR, 1,
			COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

		idx = IDX_LOC_OBJ_END_CAN_WATT_HOUR_RATE_BRANCH;
		myTestCond->local_object[step][idx] = P1_stepCond->reference[0]
			.ValueRate_Branch;
		if(myTestCond->local_object[step][idx] == ACTIVE_DIV_NONE)
			myTestCond->local_object[step][idx] = ACTIVE_DIV_NEXT;

		idx = IDX_LOC_OBJ_END_CAN_WATT_HOUR_RATE_COMPARE;
		myTestCond->local_object[step][idx] = P1_stepCond->reference[0]
		.ValueRate_Compare;
	}	//ktg_220829e

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
	//make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
	//	C_CD_FAULT_HW_UPPER_I, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=
	switch(mode) {		//csk_190711s
		case MODE_CV:
			make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
				C_CD_FAULT_HW_UPPER_I, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx);
			break;
		default:	//CC, CP
			make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
				C_CD_FAULT_HW_UPPER_I, 5, COMP_GREATER_THAN_OR_EQUAL_TO, idx);
			break;
	}					//csk_190711e

	//ktg_230728s
	idx = IDX_LOC_OBJ_FAULT_CAN_FREEZE_USE_FLAG;
	myTestCond->local_object[step][idx] = P1_stepCond->faultCanFreeze_use_flag;
	idx = IDX_LOC_OBJ_FAULT_CAN_FREEZE_TIME;
	myTestCond->local_object[step][idx] = P1_stepCond->faultCanFreeze_time;
	idx = IDX_LOC_OBJ_FAULT_CAN_FREEZE_CELL_V;
	myTestCond->local_object[step][idx] = P1_stepCond->faultCanFreeze_CellV;
	idx = IDX_LOC_OBJ_FAULT_CAN_FREEZE_I;
	myTestCond->local_object[step][idx] = P1_stepCond->faultCanFreeze_I;
	//ktg_230827e

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
	long maxI, minI;		//csk_190718
	double d_val1;
	long ratioV, ratioI; //kjh_211021
	double ratioP; //kjh_211021

	ratioV = myData->mData.ratioV;	//kjh_220321
	ratioI = myData->mData.ratioI;	//kjh_220321
	ratioP = myData->mData.ratioP;	//kjh_220321

	refP = refR = 0;
	maxI = myData->mData.config.maxI[0] * attr_count;		//csk_190718s
	if(maxI < 0) maxI = MAX_SUM_ULONG;
	minI = myData->mData.config.minI[0] * attr_count;
	if(minI > 0) minI = MAX_SUM_ULONG * (-1);				//csk_190718e

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

	idx = IDX_COM_OBJ_FAULT_UPPER_V;	//kjh_220321s
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
	myTestCond->local_object[step][idx] = l_val;	//kjh_220321e

	//kjh_220321 idx = IDX_LOC_OBJ_RANGE_V; //kjg_w
	//kjh_220321 myTestCond->local_object[step][idx] = RANGE1 - 1;

	idx = IDX_LOC_OBJ_REF_V2; //discharge ref_v
	refV2 = P1_stepCond->reference[0].refV_lower;
	if(refV2 > myData->mData.config.maxV[0]) {
		refV2 = myData->mData.config.maxV[0];
	} else if(refV2 < myData->mData.config.minV[0]) {
		refV2 = myData->mData.config.minV[0];
	}
	myTestCond->local_object[step][idx] = refV2;

	idx = IDX_COM_OBJ_FAULT_UPPER_V;	//kjh_220321s
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
	myTestCond->local_object[step][idx] = l_val;	//kjh_220321e

	//kjh_220321 idx = IDX_LOC_OBJ_RANGE_V2; //kjg_w
	//kjh_220321 myTestCond->local_object[step][idx] = RANGE1 - 1;

	refP = P1_stepCond->reference[0].refP;
	//refI = myData->mData.config.maxI[0] * attr_count;
	refI = maxI;		//csk_190718

	//d_val1 = (double)(myData->mData.config.maxV[0] / 1000)
	//	* (double)(refI / 1000) / 1000.0;
	d_val1 = (double)(myData->mData.config.maxV[0] / ratioV)
		* (double)(refI / ratioI) / ratioP;	//kjh_211021
	if(refP > (long)d_val1) refP = (long)d_val1;
	idx = IDX_LOC_OBJ_REF_P;
	myTestCond->local_object[step][idx] = refP;

	refI = P1_stepCond->reference[0].refI;
	idx = IDX_LOC_OBJ_REF_I;

	if(refI > maxI) {			//csk_190718s
		if(myData->mData.config.maxI[0] != 0) refI = maxI;
	} else if(refI < minI) {
		if(myData->mData.config.minI[0] != 0) refI = minI;
	}							//csk_190718e

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
	} else if(tmp == P1_VALUE_RATE_ITEM_CAN_AMPARE_HOUR) {	//ktg_220829s
		idx = IDX_LOC_OBJ_END_CAN_AMPARE_HOUR_RATE;
		myTestCond->local_object[step][idx] = P1_stepCond->reference[0]
			.ValueRate;
		make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
			C_CD_END_VALUE_RATE_CAN_AMPARE_HOUR, 1,
			COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

		idx = IDX_LOC_OBJ_END_CAN_AMPARE_HOUR_RATE_BRANCH;
		myTestCond->local_object[step][idx] = P1_stepCond->reference[0]
			.ValueRate_Branch;
		if(myTestCond->local_object[step][idx] == ACTIVE_DIV_NONE)
			myTestCond->local_object[step][idx] = ACTIVE_DIV_NEXT;

		idx = IDX_LOC_OBJ_END_CAN_AMPARE_HOUR_RATE_COMPARE;
		myTestCond->local_object[step][idx] = P1_stepCond->reference[0]
			.ValueRate_Compare;
	} else if(tmp == P1_VALUE_RATE_ITEM_CAN_WATT_HOUR) {
		idx = IDX_LOC_OBJ_END_CAN_WATT_HOUR_RATE;
		myTestCond->local_object[step][idx] = P1_stepCond->reference[0]
			.ValueRate;
		make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
			C_CD_END_VALUE_RATE_CAN_WATT_HOUR, 1,
			COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

		idx = IDX_LOC_OBJ_END_CAN_WATT_HOUR_RATE_BRANCH;
		myTestCond->local_object[step][idx] = P1_stepCond->reference[0]
			.ValueRate_Branch;
		if(myTestCond->local_object[step][idx] == ACTIVE_DIV_NONE)
			myTestCond->local_object[step][idx] = ACTIVE_DIV_NEXT;

		idx = IDX_LOC_OBJ_END_CAN_WATT_HOUR_RATE_COMPARE;
		myTestCond->local_object[step][idx] = P1_stepCond->reference[0]
		.ValueRate_Compare;
	}	//ktg_220829e

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
	int master, slave, idx, rtn, fault_count;
	int i, k, func_div, func_div1, func_div2, func_div3;
	long usermap_index, mode, l_val, refV, refI, tmp;
	long maxI, minI;			//csk_190718

	maxI = myData->mData.config.maxI[0] * attr_count;		//csk_190718s
	if(maxI < 0) maxI = MAX_SUM_ULONG;
	minI = myData->mData.config.minI[0] * attr_count;
	if(minI > 0) minI = MAX_SUM_ULONG * (-1);				//csk_190718e
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

	refI = maxI;			//csk_190718

	if(refI > maxI) {			//csk_190718s
		if(myData->mData.config.maxI[0] != 0) refI = maxI;
	} else if(refI < minI) {
		if(myData->mData.config.minI[0] != 0) refI = minI;
	}							//csk_190718e

	idx = IDX_LOC_OBJ_REF_I;
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
	} else if(tmp == P1_VALUE_RATE_ITEM_CAN_AMPARE_HOUR) {	//ktg_220829s
		idx = IDX_LOC_OBJ_END_CAN_AMPARE_HOUR_RATE;
		myTestCond->local_object[step][idx] = P1_stepCond->reference[0]
			.ValueRate;
		make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
			C_CD_END_VALUE_RATE_CAN_AMPARE_HOUR, 1,
			COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

		idx = IDX_LOC_OBJ_END_CAN_AMPARE_HOUR_RATE_BRANCH;
		myTestCond->local_object[step][idx] = P1_stepCond->reference[0]
			.ValueRate_Branch;
		if(myTestCond->local_object[step][idx] == ACTIVE_DIV_NONE)
			myTestCond->local_object[step][idx] = ACTIVE_DIV_NEXT;

		idx = IDX_LOC_OBJ_END_CAN_AMPARE_HOUR_RATE_COMPARE;
		myTestCond->local_object[step][idx] = P1_stepCond->reference[0]
			.ValueRate_Compare;
	} else if(tmp == P1_VALUE_RATE_ITEM_CAN_WATT_HOUR) {
		idx = IDX_LOC_OBJ_END_CAN_WATT_HOUR_RATE;
		myTestCond->local_object[step][idx] = P1_stepCond->reference[0]
			.ValueRate;
		make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
			C_CD_END_VALUE_RATE_CAN_WATT_HOUR, 1,
			COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

		idx = IDX_LOC_OBJ_END_CAN_WATT_HOUR_RATE_BRANCH;
		myTestCond->local_object[step][idx] = P1_stepCond->reference[0]
			.ValueRate_Branch;
		if(myTestCond->local_object[step][idx] == ACTIVE_DIV_NONE)
			myTestCond->local_object[step][idx] = ACTIVE_DIV_NEXT;

		idx = IDX_LOC_OBJ_END_CAN_WATT_HOUR_RATE_COMPARE;
		myTestCond->local_object[step][idx] = P1_stepCond->reference[0]
			.ValueRate_Compare;
	}	//ktg_220829e

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

	return 0;
}

void convert_test_cond_cable_check(int ch)
{ //kjg_101221_w
	int idx2;
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
		for(i=0; i < MAX_SLAVE_CH; i++) {
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

	idx2 = IDX_LOC_OBJ_END_TIME_DAY;
	myTestCond->local_object[step][idx2] = 0;
	idx = IDX_LOC_OBJ_END_TIME;
	myTestCond->local_object[step][idx] = 100; //1sec
	make_loc_chCode_cond2((S_TEST_CONDITION *)myTestCond, step,
		C_CD_END_TIME, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx, idx2); //>=

	idx = IDX_LOC_OBJ_END_TIME_BRANCH;
	myTestCond->local_object[step][idx] = ACTIVE_DIV_NEXT;

	idx = IDX_LOC_OBJ_SAVE_DELTA_T;
	myTestCond->local_object[step][idx] = 10; //0.1sec
	make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
		C_CD_SAVE_DELTA_T, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_LOC_OBJ_REF_V;
	//refV = 4200000; //4.2V
	if(myData->mData.config.ratioV == MICRO_UNIT) {	//uV //kjh_211021s
		refV = 4200000; //4.2V
	} else if(myData->mData.config.ratioV == MILLI_UNIT) {	//mV
		refV = 4200; //4.2V
	} else {
		refV = 0; //0V
	} //kjh_211021e
	myTestCond->local_object[step][idx] = refV;

	idx = IDX_LOC_OBJ_REF_V2;
	myTestCond->local_object[step][idx] = 0;

	idx = IDX_LOC_OBJ_RANGE_V;
	myTestCond->local_object[step][idx] = RANGE1 - 1;

	idx = IDX_LOC_OBJ_REF_I;
	//refI = 20000000; //20A
	if(myData->mData.config.ratioI == MICRO_UNIT) {	//uA //kjh_211021s
		refI = 20000000; //20A
	} else if(myData->mData.config.ratioI == MILLI_UNIT) {	//mA
		refI = 20000; //20A
	} else {
		refI = 0; //0A
	} //kjh_211021e
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

	idx2 = IDX_LOC_OBJ_END_TIME_DAY;
	myTestCond->local_object[step][idx2] = 0;
	idx = IDX_LOC_OBJ_END_TIME;
	myTestCond->local_object[step][idx] = 100; //1sec
	make_loc_chCode_cond2((S_TEST_CONDITION *)myTestCond, step,
		C_CD_END_TIME, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx, idx2); //>=

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

	idx2 = IDX_LOC_OBJ_END_TIME_DAY;
	myTestCond->local_object[step][idx2] = 0;
	idx = IDX_LOC_OBJ_END_TIME;
	myTestCond->local_object[step][idx] = 100; //1sec
	make_loc_chCode_cond2((S_TEST_CONDITION *)myTestCond, step,
		C_CD_END_TIME, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx, idx2); //>=

	idx = IDX_LOC_OBJ_END_TIME_BRANCH;
	myTestCond->local_object[step][idx] = ACTIVE_DIV_NEXT;

	idx = IDX_LOC_OBJ_SAVE_DELTA_T;
	myTestCond->local_object[step][idx] = 10; //0.1sec
	make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
		C_CD_SAVE_DELTA_T, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_LOC_OBJ_REF_V;
	//refV = 4200000; //4.2V
	if(myData->mData.config.ratioV == MICRO_UNIT) {	//uV //kjh_211021s
		refV = 4200000; //4.2V
	} else if(myData->mData.config.ratioV == MILLI_UNIT) {	//mV
		refV = 4200; //4.2V
	} else {
		refV = 0; //0V
	} //kjh_211021e
	myTestCond->local_object[step][idx] = refV;

	idx = IDX_LOC_OBJ_REF_V2;
	myTestCond->local_object[step][idx] = 0;

	idx = IDX_LOC_OBJ_RANGE_V;
	myTestCond->local_object[step][idx] = RANGE1 - 1;

	idx = IDX_LOC_OBJ_REF_I;
	//refI = -20000000; //-20A
	if(myData->mData.config.ratioI == MICRO_UNIT) {	//uA //kjh_211021s
		refI = -20000000; //-20A
	} else if(myData->mData.config.ratioI == MILLI_UNIT) {	//mA
		refI = -20000; //-20A
	} else {
		refI = 0; //0A
	} //kjh_211021e
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

	idx2 = IDX_LOC_OBJ_END_TIME_DAY;
	myTestCond->local_object[step][idx2] = 0;
	idx = IDX_LOC_OBJ_END_TIME;
	myTestCond->local_object[step][idx] = 100; //1sec
	make_loc_chCode_cond2((S_TEST_CONDITION *)myTestCond, step,
		C_CD_END_TIME, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx, idx2); //>=

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
		for(i=0; i < MAX_SLAVE_CH; i++) {
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

	//ktg_210529s	//shh_210906s
	idx = IDX_LOC_OBJ_RANGE_V;
	myTestCond->local_object[stepNo][idx] = myData->testCond[ch].local_object[0][idx];
	idx = IDX_LOC_OBJ_RANGE_V2;
	myTestCond->local_object[stepNo][idx] = myData->testCond[ch].local_object[0][idx];
	//ktg_210529e	//shh_210906e
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

	idx = IDX_COM_OBJ_TOTAL_PATTERN;
	myTestCond->common_object[idx] = (long)myPs->common_safety_cond_update
		.testCond_header.totalPatternCount;

	idx = IDX_COM_OBJ_ATTRIBUTE_COUNT;
	attr_count = 1;
	if(myData->ChAttribute[ch].opType != OP_INDEPENDENT) {
		for(i=0; i < MAX_SLAVE_CH; i++) {
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
	switch(myData->AppControl.config.systemModel) {
		case C_LGC_500V_250A_125A_50A_500KW_11: //define 10450
		case C_LGC_450V_200A_20A_180KW:	//ktg_210222
		case C_LGC_450V_200A_10A_180KW_3:	//define 1158	//ktg_210222
		case C_LGC_450V_200A_20A_180KW_2:	//define 1065	//ktg_210222
		case C_LGC_450V_250A_100A_225KW:	//define 10174	//ktg_210428
		case C_LGC_450V_250A_50A_225KW:     //define 10134	//shh_211214
			myTestCond->common_object[idx] = 20000; //20.0%
			break;
		default:
			myTestCond->common_object[idx] = 5000; //5.0%
			break;
	}
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

	idx = IDX_COM_OBJ_FAULT_UPPER_I;
	myTestCond->common_object[idx] = myPs->common_safety_cond_update
		.testCond_safety.faultUpperI;
	make_com_chCode_cond((S_TEST_CONDITION *)myTestCond,
		C_CD_COM_FAULT_UPPER_I, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

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

	idx = IDX_COM_OBJ_FAULT_UPPER_TEMP; //kjhw_181111 use
	myTestCond->common_object[idx] = myPs->common_safety_cond_update
		.testCond_safety.faultUpperTemp;
	//make_com_chCode_cond((S_TEST_CONDITION *)myTestCond,
	//	C_CD_COM_FAULT_UPPER_TEMP, 500, COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_COM_OBJ_FAULT_LOWER_TEMP;
	myTestCond->common_object[idx] = myPs->common_safety_cond_update
		.testCond_safety.faultLowerTemp;
	make_com_chCode_cond((S_TEST_CONDITION *)myTestCond,
		C_CD_COM_FAULT_LOWER_TEMP, 1, COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=

	//idx = IDX_COM_OBJ_FAULT_AUX_V; //kjhw_181111
	//myTestCond->common_object[idx] = myPs->common_safety_cond_update
	//	.testCond_safety.faultAuxV;
	idx = IDX_COM_OBJ_FAULT_UPPER_AUX_V; //csk_190808
	myTestCond->common_object[idx]
		= myPs->common_safety_cond_update.testCond_safety.faultUpper_AuxV;
	//make_com_chCode_cond((S_TEST_CONDITION *)myTestCond,
	//	C_CD_COM_FAULT_AUX_V, 500, COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_COM_OBJ_FAULT_LOWER_AUX_V;	//csk_190808
	myTestCond->common_object[idx]
		= myPs->common_safety_cond_update.testCond_safety.faultLower_AuxV;

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

	//csk_190808s
	idx = IDX_COM_OBJ_FAULT_COMP_AUX_REF_V1;
	myTestCond->common_object[idx]
		= myPs->common_safety_cond_update.testCond_safety.faultCompAux_RefV1;

	idx = IDX_COM_OBJ_FAULT_COMP_AUX_V2;
	myTestCond->common_object[idx]
		= myPs->common_safety_cond_update.testCond_safety.faultCompAuxV2;
	make_com_chCode_cond((S_TEST_CONDITION *)myTestCond,
		C_CD_COM_FAULT_COMP_AUX_V2, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_COM_OBJ_FAULT_COMP_AUX_REF_V2;
	myTestCond->common_object[idx]
		= myPs->common_safety_cond_update.testCond_safety.faultCompAux_RefV2;

	idx = IDX_COM_OBJ_FAULT_COMP_AUX_V3;
	myTestCond->common_object[idx]
		= myPs->common_safety_cond_update.testCond_safety.faultCompAuxV3;
	make_com_chCode_cond((S_TEST_CONDITION *)myTestCond,
		C_CD_COM_FAULT_COMP_AUX_V3, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_COM_OBJ_FAULT_COMP_AUX_VENT_FLAG;
	myTestCond->common_object[idx]
		= myPs->common_safety_cond_update.testCond_safety.faultCompAuxV_vent_flag;
	//csk_190808e
	
	idx = IDX_COM_OBJ_FAULT_COMP_TYPE_SELECT;	//ktg_241008s
	myTestCond->common_object[idx]
		= myPs->common_safety_cond_update.testCond_safety.faultCompType_select;

	idx = IDX_COM_OBJ_FAULT_COMP_CAN_V;
	myTestCond->common_object[idx] = myPs->common_safety_cond_update
		.testCond_safety.faultCompAuxV;
	make_com_chCode_cond((S_TEST_CONDITION *)myTestCond,
		C_CD_COM_FAULT_COMP_CAN_V, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=


	idx = IDX_COM_OBJ_FAULT_COMP_CAN_REF_V1;
	myTestCond->common_object[idx]
		= myPs->common_safety_cond_update.testCond_safety.faultCompAux_RefV1;

	idx = IDX_COM_OBJ_FAULT_COMP_CAN_V2;
	myTestCond->common_object[idx]
		= myPs->common_safety_cond_update.testCond_safety.faultCompAuxV2;
	make_com_chCode_cond((S_TEST_CONDITION *)myTestCond,
		C_CD_COM_FAULT_COMP_CAN_V2, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_COM_OBJ_FAULT_COMP_CAN_REF_V2;
	myTestCond->common_object[idx]
		= myPs->common_safety_cond_update.testCond_safety.faultCompAux_RefV2;

	idx = IDX_COM_OBJ_FAULT_COMP_CAN_V3;
	myTestCond->common_object[idx]
		= myPs->common_safety_cond_update.testCond_safety.faultCompAuxV3;
	make_com_chCode_cond((S_TEST_CONDITION *)myTestCond,
		C_CD_COM_FAULT_COMP_CAN_V3, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=	//ktg_241008e
	
	for(i =0; i < MAX_AUX_GROUP; i++) {	//ktg_220220s
		idx = IDX_COM_OBJ_FAULT_GROUP01_COMP_AUX_V1 + i;
		myTestCond->common_object[idx] = myPs->common_safety_cond_update
			.testCond_safety.faultCompGroupAuxV[i];
	
		idx = IDX_COM_OBJ_FAULT_GROUP01_COMP_AUX_REF_V1 + i;
		myTestCond->common_object[idx] = myPs->common_safety_cond_update
			.testCond_safety.faultCompGroupAux_RefV1[i];
	
		idx = IDX_COM_OBJ_FAULT_GROUP01_COMP_AUX_V2 + i;
		myTestCond->common_object[idx] = myPs->common_safety_cond_update
			.testCond_safety.faultCompGroupAuxV2[i];
	
		idx = IDX_COM_OBJ_FAULT_GROUP01_COMP_AUX_REF_V2 + i;
		myTestCond->common_object[idx] = myPs->common_safety_cond_update
			.testCond_safety.faultCompGroupAux_RefV2[i];
	
		idx = IDX_COM_OBJ_FAULT_GROUP01_COMP_AUX_V3 + i;
		myTestCond->common_object[idx] = myPs->common_safety_cond_update
			.testCond_safety.faultCompGroupAuxV3[i];
	
		idx = IDX_COM_OBJ_FAULT_GROUP01_COMP_AUX_VENT_FLAG + i;
		myTestCond->common_object[idx] = myPs->common_safety_cond_update
			.testCond_safety.faultCompGroupAuxV_vent_flag[i];
	}	//ktg_220220e

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

	//idx = IDX_COM_OBJ_FAULT_AUX_V; //kjhw_181111
	//myPs->common_safety_cond_update.testCond_safety.faultAuxV
	//	= myData->testCond[ch].common_object[idx];
	idx = IDX_COM_OBJ_FAULT_UPPER_AUX_V;	//csk_190808
	myPs->common_safety_cond_update.testCond_safety.faultUpper_AuxV
		= myData->testCond[ch].common_object[idx];

	idx = IDX_COM_OBJ_FAULT_LOWER_AUX_V;	//csk_190808
	myPs->common_safety_cond_update.testCond_safety.faultLower_AuxV;

	idx = IDX_COM_OBJ_FAULT_COMP_AUX_V; //kjhw_181111
	myPs->common_safety_cond_update.testCond_safety.faultCompAuxV
		= myData->testCond[ch].common_object[idx];

	//csk_190808s
	idx = IDX_COM_OBJ_FAULT_COMP_AUX_REF_V1;
	myPs->common_safety_cond_update.testCond_safety.faultCompAux_RefV1
		= myData->testCond[ch].common_object[idx];

	idx = IDX_COM_OBJ_FAULT_COMP_AUX_V2;
	myPs->common_safety_cond_update.testCond_safety.faultCompAuxV2
		= myData->testCond[ch].common_object[idx];

	idx = IDX_COM_OBJ_FAULT_COMP_AUX_REF_V2;
	myPs->common_safety_cond_update.testCond_safety.faultCompAux_RefV2
		= myData->testCond[ch].common_object[idx];

	idx = IDX_COM_OBJ_FAULT_COMP_AUX_V3;
	myPs->common_safety_cond_update.testCond_safety.faultCompAuxV3
		= myData->testCond[ch].common_object[idx];

	idx = IDX_COM_OBJ_FAULT_COMP_AUX_VENT_FLAG;
	myPs->common_safety_cond_update.testCond_safety.faultCompAuxV_vent_flag
		= myData->testCond[ch].common_object[idx];
	//csk_190808e
	idx = IDX_COM_OBJ_FAULT_COMP_TYPE_SELECT;	//ktg_241008s
	myPs->common_safety_cond_update.testCond_safety.faultCompType_select
		= myData->testCond[ch].common_object[idx];

	idx = IDX_COM_OBJ_FAULT_COMP_CAN_V;
	myPs->common_safety_cond_update.testCond_safety.faultCompAuxV
		= myData->testCond[ch].common_object[idx];

	idx = IDX_COM_OBJ_FAULT_COMP_CAN_REF_V1;
	myPs->common_safety_cond_update.testCond_safety.faultCompAux_RefV1
		= myData->testCond[ch].common_object[idx];

	idx = IDX_COM_OBJ_FAULT_COMP_CAN_V2;
	myPs->common_safety_cond_update.testCond_safety.faultCompAuxV2
		= myData->testCond[ch].common_object[idx];

	idx = IDX_COM_OBJ_FAULT_COMP_CAN_REF_V2;
	myPs->common_safety_cond_update.testCond_safety.faultCompAux_RefV2
		= myData->testCond[ch].common_object[idx];

	idx = IDX_COM_OBJ_FAULT_COMP_CAN_V3;
	myPs->common_safety_cond_update.testCond_safety.faultCompAuxV3
		= myData->testCond[ch].common_object[idx];	//ktg_241008e

	
	for(i =0; i < MAX_AUX_GROUP; i++) {	//ktg_220220s
		idx = IDX_COM_OBJ_FAULT_GROUP01_COMP_AUX_V1 + i;
		myPs->common_safety_cond_update.testCond_safety.faultCompGroupAuxV[i]
			= myData->testCond[ch].common_object[idx];
	
		idx = IDX_COM_OBJ_FAULT_GROUP01_COMP_AUX_REF_V1 + i;
		myPs->common_safety_cond_update.testCond_safety.faultCompGroupAux_RefV1[i]
			= myData->testCond[ch].common_object[idx];
	
		idx = IDX_COM_OBJ_FAULT_GROUP01_COMP_AUX_V2 + i;
		myPs->common_safety_cond_update.testCond_safety.faultCompGroupAuxV2[i]
			= myData->testCond[ch].common_object[idx];
	
		idx = IDX_COM_OBJ_FAULT_GROUP01_COMP_AUX_REF_V2 + i;
		myPs->common_safety_cond_update.testCond_safety.faultCompGroupAux_RefV2[i]
			= myData->testCond[ch].common_object[idx];
	
		idx = IDX_COM_OBJ_FAULT_GROUP01_COMP_AUX_V3 + i;
		myPs->common_safety_cond_update.testCond_safety.faultCompGroupAuxV3[i]
			= myData->testCond[ch].common_object[idx];
	
		idx = IDX_COM_OBJ_FAULT_GROUP01_COMP_AUX_VENT_FLAG + i;
		myPs->common_safety_cond_update.testCond_safety.faultCompGroupAuxV_vent_flag[i]
			= myData->testCond[ch].common_object[idx];
	}	//ktg_220220e

	idx = IDX_COM_OBJ_FAULT_UPPER_I;
	myPs->common_safety_cond_update.testCond_safety.faultUpperI
		= myData->testCond[ch].common_object[idx];

	idx = IDX_COM_OBJ_FAULT_LOWER_TEMP;
	myPs->common_safety_cond_update.testCond_safety.faultLowerTemp
		= myData->testCond[ch].common_object[idx];

	idx = IDX_COM_OBJ_FAULT_UPPER_TEMP; //kjhw_181111 use
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
			= myData->testCond[ch].common_object[idx];
	}
}

void convert_test_cond_module_coa_step(int ch, int stepNo)
{ //kjg_170810
	int idx, i;
	unsigned int delay_time; //kjhw_170828
	long type, tmp;
	S_P1_TEST_COND_STEP *P1_stepCond;
	long mode;	//ktg_210315

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
				= (unsigned char)tmp;

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
			P1_stepCond->reference[0].ValueRate_Compare = (unsigned char)tmp;

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
				= (unsigned char)tmp;

			idx = IDX_LOC_OBJ_END_MULTI_CYCLE_COUNT_COMPARE;
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			P1_stepCond->reference[0].AmpareHour_Branch_MultiCycleCountId
				= (unsigned char)tmp;

			idx = IDX_LOC_OBJ_END_ACC_CYCLE_COUNT;
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			P1_stepCond->reference[0].Time_Branch_AccCycleCount = tmp;

			idx = IDX_LOC_OBJ_END_ACC_CYCLE_COUNT_BRANCH;
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			P1_stepCond->reference[0].AccCycleCount_Branch = (unsigned char)tmp;

			idx = IDX_LOC_OBJ_END_ACC_CYCLE_COUNT_COMPARE;
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			P1_stepCond->reference[0].CVTime_Branch_AccCycleCountId = tmp;
			//csk_190208s
			idx = IDX_LOC_OBJ_END_AMPARE_HOUR_UPPER;
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			P1_stepCond->reference[0].V_Upper = tmp;

			idx = IDX_LOC_OBJ_END_AMPARE_HOUR_UPPER_BRANCH;
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			P1_stepCond->reference[0].endTime_day = tmp;

			idx = IDX_LOC_OBJ_END_AMPARE_HOUR_LOWER;
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			P1_stepCond->reference[0].Current = tmp;

			idx = IDX_LOC_OBJ_END_AMPARE_HOUR_LOWER_BRANCH;
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			P1_stepCond->reference[0].endTime = tmp;

			idx = IDX_LOC_OBJ_END_WATT_HOUR_UPPER;
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			P1_stepCond->reference[0].DeltaV = tmp;

			idx = IDX_LOC_OBJ_END_WATT_HOUR_UPPER_BRANCH;
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			P1_stepCond->reference[0].CVTime_day = tmp;

			idx = IDX_LOC_OBJ_END_WATT_HOUR_LOWER;
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			P1_stepCond->reference[0].Power = tmp;

			idx = IDX_LOC_OBJ_END_WATT_HOUR_LOWER_BRANCH;
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			P1_stepCond->reference[0].CVTime = tmp;
			//csk_190208e
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

			idx = IDX_LOC_OBJ_FAULT_BALANCE_CHECK_FLAG; //csk_190114
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			P1_stepCond->fault_balance_check_flag = (unsigned char)tmp;

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
					= myData->testCond[ch].local_object[stepNo][idx];
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

			idx = IDX_LOC_OBJ_FAULT_UPPER_TEMP;
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			P1_stepCond->faultUpperTemp = tmp;

			idx = IDX_LOC_OBJ_FAULT_LOWER_TEMP;
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			P1_stepCond->faultLowerTemp = tmp;

			//csk_190808s
			idx = IDX_LOC_OBJ_FAULT_COMP_AUX_V;
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			P1_stepCond->faultCompAuxV = tmp;

			idx = IDX_LOC_OBJ_FAULT_COMP_AUX_V_VENT_FLAG;
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			P1_stepCond->faultCompAuxV_vent_flag = tmp;

			idx = IDX_LOC_OBJ_FAULT_COMP_AUX_TEMP;
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			P1_stepCond->faultCompAuxTemp = tmp;

			idx = IDX_LOC_OBJ_FAULT_COMP_AUX_TEMP_VENT_FLAG;
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			P1_stepCond->faultCompAuxTemp_vent_flag = tmp;

			idx = IDX_LOC_OBJ_FAULT_COMP_AUX_TH;
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			P1_stepCond->faultCompAuxTh = tmp;

			idx = IDX_LOC_OBJ_FAULT_COMP_AUX_TH_VENT_FLAG;
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			P1_stepCond->faultCompAuxTh_vent_flag = tmp;

			idx = IDX_LOC_OBJ_FAULT_COMP_AUX_T;
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			P1_stepCond->faultCompAuxT = tmp;

			idx = IDX_LOC_OBJ_FAULT_COMP_AUX_T_VENT_FLAG;
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			P1_stepCond->faultCompAuxT_vent_flag = tmp;

			idx = IDX_LOC_OBJ_FAULT_DELTA_AUX_V;
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			P1_stepCond->faultDelta_AuxV = tmp;

			idx = IDX_LOC_OBJ_FAULT_DELTA_AUX_V_T;
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			P1_stepCond->faultDelta_AuxV_T = tmp;

			idx = IDX_LOC_OBJ_FAULT_DELTA_AUX_V_VENT_FLAG;
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			P1_stepCond->faultDelta_AuxV_vent_flag = tmp;
			//csk_190808e

			idx = IDX_LOC_OBJ_FAULT_CAN_COMP_CAN_V_USE_FLAG;		//ktg_210706s
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			P1_stepCond->faultCompCanV_use_flag = (unsigned char)tmp;

			idx = IDX_LOC_OBJ_FAULT_CAN_COMP_CAN_V_VENT_FLAG;
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			P1_stepCond->faultCompCanV_vent_flag = (unsigned char)tmp;

			idx = IDX_LOC_OBJ_FAULT_CAN_COMP_CAN_V_VALUE;
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			P1_stepCond->faultCompCanV_value = tmp;	//ktg_210706e

			idx = IDX_LOC_OBJ_FAULT_AUX_SOFTVENT_USE_FLAG;	//ktg_220331s	soft aux
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			P1_stepCond->faultSoftvent_use_flag = (unsigned char)tmp;

			idx = IDX_LOC_OBJ_FAULT_AUX_SOFTVENT_VENT_FLAG;
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			P1_stepCond->faultSoftvent_vent_flag = (unsigned char)tmp;
	
			idx = IDX_LOC_OBJ_FAULT_AUX_SOFTVENT_COUNT;
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			P1_stepCond->faultSoftvent_count = (unsigned char)tmp;
	
			idx = IDX_LOC_OBJ_FAULT_AUX_SOFTVENT_VALUE;
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			P1_stepCond->faultSoftvent_value = tmp;

			idx = IDX_LOC_OBJ_FAULT_AUX_HARDVENT_USE_FLAG;	//hard aux
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			P1_stepCond->faultHardvent_use_flag = (unsigned char)tmp;

			idx = IDX_LOC_OBJ_FAULT_AUX_HARDVENT_VENT_FLAG;
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			P1_stepCond->faultHardvent_vent_flag = (unsigned char)tmp;
	
			idx = IDX_LOC_OBJ_FAULT_AUX_HARDVENT_COUNT;
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			P1_stepCond->faultHardvent_count = (unsigned char)tmp;
	
			idx = IDX_LOC_OBJ_FAULT_AUX_HARDVENT_VALUE;
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			P1_stepCond->faultHardvent_value = tmp;

			idx = IDX_LOC_OBJ_FAULT_CAN_SOFTVENT_USE_FLAG;	//soft can
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			P1_stepCond->faultSoftvent_can_use_flag = (unsigned char)tmp;

			idx = IDX_LOC_OBJ_FAULT_CAN_SOFTVENT_VENT_FLAG;
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			P1_stepCond->faultSoftvent_can_vent_flag = (unsigned char)tmp;
	
			idx = IDX_LOC_OBJ_FAULT_CAN_SOFTVENT_COUNT;
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			P1_stepCond->faultSoftvent_can_count = (unsigned char)tmp;
	
			idx = IDX_LOC_OBJ_FAULT_CAN_SOFTVENT_VALUE;
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			P1_stepCond->faultSoftvent_can_value = tmp;

			idx = IDX_LOC_OBJ_FAULT_CAN_HARDVENT_USE_FLAG;	//hard can
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			P1_stepCond->faultHardvent_can_use_flag = (unsigned char)tmp;

			idx = IDX_LOC_OBJ_FAULT_CAN_HARDVENT_VENT_FLAG;
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			P1_stepCond->faultHardvent_can_vent_flag = (unsigned char)tmp;
	
			idx = IDX_LOC_OBJ_FAULT_CAN_HARDVENT_COUNT;
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			P1_stepCond->faultHardvent_can_count = (unsigned char)tmp;
	
			idx = IDX_LOC_OBJ_FAULT_CAN_HARDVENT_VALUE;
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			P1_stepCond->faultHardvent_can_value = tmp;	//ktg_220331e
			//grade
			//kjg_170810_w
			break;
	}

	switch(type) {
		case STEP_OCV:
			break;	//ktg_210105
		case STEP_REST:
			//ktg_210105s
			idx = IDX_LOC_OBJ_FAULT_SVDF_VENT_FLAG;
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			P1_stepCond->faultSVDF_vent_flag = (unsigned char)tmp;

			idx = IDX_LOC_OBJ_FAULT_SVDF_COUNT;
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			P1_stepCond->faultSVDF_count = (unsigned char)tmp;

			idx = IDX_LOC_OBJ_FAULT_SVDF_YT_1;
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			P1_stepCond->faultSVDF_yt1 = tmp;

			idx = IDX_LOC_OBJ_FAULT_SVDF_VCELL_AVER;
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			P1_stepCond->faultSVDF_vcell_aver = tmp;

			idx = IDX_LOC_OBJ_FAULT_SVDF_YT_2;
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			P1_stepCond->faultSVDF_yt2 = tmp;

			idx = IDX_LOC_OBJ_FAULT_SVDF_YT_3;
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			P1_stepCond->faultSVDF_yt3 = tmp;

			idx = IDX_LOC_OBJ_FAULT_SVDF_USE_FLAG;
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			P1_stepCond->faultSVDF_use_flag = (unsigned char)tmp;
			//ktg_210105e
	
			idx = IDX_LOC_OBJ_FAULT_CAN_SVDF_VENT_FLAG;					//ktg_210706s
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			P1_stepCond->faultSVDF_can_vent_flag = (unsigned char)tmp;

			idx = IDX_LOC_OBJ_FAULT_CAN_SVDF_COUNT;
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			P1_stepCond->faultSVDF_can_count = (unsigned char)tmp;

			idx = IDX_LOC_OBJ_FAULT_CAN_SVDF_YT_1;
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			P1_stepCond->faultSVDF_can_yt1 = tmp;

			idx = IDX_LOC_OBJ_FAULT_CAN_SVDF_VCELL_AVER;
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			P1_stepCond->faultSVDF_can_vcell_aver = tmp;

			idx = IDX_LOC_OBJ_FAULT_CAN_SVDF_YT_2;
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			P1_stepCond->faultSVDF_can_yt2 = tmp;

			idx = IDX_LOC_OBJ_FAULT_CAN_SVDF_YT_3;
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			P1_stepCond->faultSVDF_can_yt3 = tmp;

			idx = IDX_LOC_OBJ_FAULT_CAN_SVDF_USE_FLAG;
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			P1_stepCond->faultSVDF_can_use_flag = (unsigned char)tmp;	//ktg_210706e
			idx = IDX_LOC_OBJ_FAULT_V_RISING_TIME_INIT;
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			P1_stepCond->faultVrising_time_init = tmp;						//shhw_220916s
			
			idx = IDX_LOC_OBJ_FAULT_V_RISING_TIME_CHECK;
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			P1_stepCond-> faultVrising_time_check = tmp;						
			
			idx = IDX_LOC_OBJ_FAULT_V_RISING_REFF_VALUE;
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			P1_stepCond->faultVrising_reff_value = tmp;						//shhw_220916e
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
				= (unsigned char)tmp;

			idx = IDX_LOC_OBJ_END_WATT_HOUR;
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			P1_stepCond->reference[0].WattHour_SumWattHour = tmp;

			idx = IDX_LOC_OBJ_END_WATT_HOUR_BRANCH;
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			P1_stepCond->reference[0].WattHour_Branch_MultiCycleCount_Branch
				= (unsigned char)tmp;

			idx = IDX_LOC_OBJ_END_AMPARE_HOUR_RATE;
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			if(tmp != 0) { //ValueRate_Item 1:AmpareHour
				P1_stepCond->reference[0].ValueRate_Item = 1;
				P1_stepCond->reference[0].ValueRate = (unsigned short)tmp;

				idx = IDX_LOC_OBJ_END_AMPARE_HOUR_RATE_BRANCH;
				tmp = myData->testCond[ch].local_object[stepNo][idx];
				P1_stepCond->reference[0].ValueRate_Branch = (unsigned char)tmp;

				idx = IDX_LOC_OBJ_END_AMPARE_HOUR_RATE_COMPARE;
				tmp = myData->testCond[ch].local_object[stepNo][idx];
				P1_stepCond->reference[0].ValueRate_Compare =(unsigned char)tmp;
			} else {
				idx = IDX_LOC_OBJ_END_WATT_HOUR_RATE;
				tmp = myData->testCond[ch].local_object[stepNo][idx];
				if(tmp != 0) { //ValueRate_Item 2:WattHour
					P1_stepCond->reference[0].ValueRate_Item = 2;
					P1_stepCond->reference[0].ValueRate = (unsigned short)tmp;

					idx = IDX_LOC_OBJ_END_WATT_HOUR_RATE_BRANCH;
					tmp = myData->testCond[ch].local_object[stepNo][idx];
					P1_stepCond->reference[0].ValueRate_Branch
						= (unsigned char)tmp;

					idx = IDX_LOC_OBJ_END_WATT_HOUR_RATE_COMPARE;
					tmp = myData->testCond[ch].local_object[stepNo][idx];
					P1_stepCond->reference[0].ValueRate_Compare
						= (unsigned char)tmp;
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

			//ktg_210315s
			idx = IDX_LOC_OBJ_MODE;
			mode = myData->testCond[ch].local_object[stepNo][idx];

			if((type == STEP_CHARGE) || (type == STEP_DISCHARGE)) {
				if((mode == MODE_CC) || (mode == MODE_CC_CV) || (mode == MODE_CV) || (mode == MODE_CP)) {
					idx = IDX_LOC_OBJ_FAULT_CV_INTERLOCK_I;
					tmp = myData->testCond[ch].local_object[stepNo][idx];
					P1_stepCond->faultcv_interlockI = tmp;
					
					idx = IDX_LOC_OBJ_FAULT_CV_INTERLOCK_VENT_FLAG;
					tmp = myData->testCond[ch].local_object[stepNo][idx];
					P1_stepCond->faultcv_interlock_vent_flag = (unsigned char)tmp;
			
					idx = IDX_LOC_OBJ_FAULT_DROP_VOLTAGE_VALUE;						//ktg_210408s
					tmp = myData->testCond[ch].local_object[stepNo][idx];
				   	P1_stepCond->faultDropVoltage_value = tmp;
					
					idx = IDX_LOC_OBJ_FAULT_DROP_VOLTAGE_USE_FLAG;
					tmp = myData->testCond[ch].local_object[stepNo][idx];
				   	P1_stepCond->faultDropVoltage_use_flag = (unsigned char)tmp;
					
					idx = IDX_LOC_OBJ_FAULT_DROP_VOLTAGE_VENT_FLAG;
					tmp = myData->testCond[ch].local_object[stepNo][idx];
				   	P1_stepCond->faultDropVoltage_vent_flag = (unsigned char)tmp;
					
					idx = IDX_LOC_OBJ_FAULT_DROP_VOLTAGE_COUNT;
					tmp = myData->testCond[ch].local_object[stepNo][idx];
					P1_stepCond->faultDropVoltage_count = (unsigned char)tmp;		//ktg_210408e

					idx = IDX_LOC_OBJ_FAULT_CAN_DROP_VOLTAGE_VALUE;						//ktg_210706s
					tmp = myData->testCond[ch].local_object[stepNo][idx];
				   	P1_stepCond->faultDropVoltage_can_value = tmp;
					
					idx = IDX_LOC_OBJ_FAULT_CAN_DROP_VOLTAGE_USE_FLAG;
					tmp = myData->testCond[ch].local_object[stepNo][idx];
				   	P1_stepCond->faultDropVoltage_can_use_flag = (unsigned char)tmp;
					
					idx = IDX_LOC_OBJ_FAULT_CAN_DROP_VOLTAGE_VENT_FLAG;
					tmp = myData->testCond[ch].local_object[stepNo][idx];
				   	P1_stepCond->faultDropVoltage_can_vent_flag = (unsigned char)tmp;
					
					idx = IDX_LOC_OBJ_FAULT_CAN_DROP_VOLTAGE_COUNT;
					tmp = myData->testCond[ch].local_object[stepNo][idx];
					P1_stepCond->faultDropVoltage_can_count = (unsigned char)tmp;		//ktg_210706e
					idx = IDX_LOC_OBJ_FAULT_CC_DELTA_V_TIME_INIT;						//shhw_220919s
					tmp = myData->testCond[ch].local_object[stepNo][idx];
					P1_stepCond->faultCC_delta_v_time_init = (unsigned long)tmp;
					
					idx = IDX_LOC_OBJ_FAULT_CC_DELTA_V_TIME_PERIOD;
					tmp = myData->testCond[ch].local_object[stepNo][idx];
					P1_stepCond->faultCC_delta_v_time_period = (unsigned long)tmp;
					
					idx = IDX_LOC_OBJ_FAULT_CC_DELTA_V_REFF_VALUE;
					tmp = myData->testCond[ch].local_object[stepNo][idx];
					P1_stepCond->faultCC_delta_v_reff_value = tmp;
																						//shhw_220919e	

				}
			}
			//ktg_210315e
	
			//ktg_230728s
			if((type == STEP_CHARGE) || (type == STEP_DISCHARGE)) {
				idx = IDX_LOC_OBJ_FAULT_CAN_FREEZE_USE_FLAG;
				tmp = myData->testCond[ch].local_object[stepNo][idx];
				P1_stepCond->faultCanFreeze_use_flag = (unsigned char)tmp;
				idx = IDX_LOC_OBJ_FAULT_CAN_FREEZE_TIME;
				tmp = myData->testCond[ch].local_object[stepNo][idx];
				P1_stepCond->faultCanFreeze_time = (unsigned short int)tmp;
				idx = IDX_LOC_OBJ_FAULT_CAN_FREEZE_CELL_V;
				tmp = myData->testCond[ch].local_object[stepNo][idx];
				P1_stepCond->faultCanFreeze_CellV = (unsigned long)tmp;
				idx = IDX_LOC_OBJ_FAULT_CAN_FREEZE_I;
				tmp = myData->testCond[ch].local_object[stepNo][idx];
				P1_stepCond->faultCanFreeze_I = (unsigned long)tmp;
			}	
			//ktg_230827e
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
				= (unsigned char)tmp;

			idx = IDX_LOC_OBJ_END_WATT_HOUR;
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			P1_stepCond->reference[0].WattHour_SumWattHour = tmp;

			idx = IDX_LOC_OBJ_END_WATT_HOUR_BRANCH;
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			P1_stepCond->reference[0].WattHour_Branch_MultiCycleCount_Branch
				= (unsigned char)tmp;

			idx = IDX_LOC_OBJ_END_AMPARE_HOUR_RATE;
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			if(tmp != 0) { //ValueRate_Item 1:AmpareHour
				P1_stepCond->reference[0].ValueRate_Item = 1;
				P1_stepCond->reference[0].ValueRate = (unsigned short)tmp;

				idx = IDX_LOC_OBJ_END_AMPARE_HOUR_RATE_BRANCH;
				tmp = myData->testCond[ch].local_object[stepNo][idx];
				P1_stepCond->reference[0].ValueRate_Branch = (unsigned char)tmp;

				idx = IDX_LOC_OBJ_END_AMPARE_HOUR_RATE_COMPARE;
				tmp = myData->testCond[ch].local_object[stepNo][idx];
				P1_stepCond->reference[0].ValueRate_Compare =(unsigned char)tmp;
			} else {
				idx = IDX_LOC_OBJ_END_WATT_HOUR_RATE;
				tmp = myData->testCond[ch].local_object[stepNo][idx];
				if(tmp != 0) { //ValueRate_Item 2:WattHour
					P1_stepCond->reference[0].ValueRate_Item = 2;
					P1_stepCond->reference[0].ValueRate = (unsigned short)tmp;

					idx = IDX_LOC_OBJ_END_WATT_HOUR_RATE_BRANCH;
					tmp = myData->testCond[ch].local_object[stepNo][idx];
					P1_stepCond->reference[0].ValueRate_Branch
						= (unsigned char)tmp;

					idx = IDX_LOC_OBJ_END_WATT_HOUR_RATE_COMPARE;
					tmp = myData->testCond[ch].local_object[stepNo][idx];
					P1_stepCond->reference[0].ValueRate_Compare
						= (unsigned char)tmp;
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

			//ktg_230728s
			idx = IDX_LOC_OBJ_FAULT_CAN_FREEZE_USE_FLAG;
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			P1_stepCond->faultCanFreeze_use_flag = (unsigned char)tmp;
			idx = IDX_LOC_OBJ_FAULT_CAN_FREEZE_TIME;
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			P1_stepCond->faultCanFreeze_time = (unsigned short int)tmp;
			idx = IDX_LOC_OBJ_FAULT_CAN_FREEZE_CELL_V;
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			P1_stepCond->faultCanFreeze_CellV = (unsigned long)tmp;
			idx = IDX_LOC_OBJ_FAULT_CAN_FREEZE_I;
			tmp = myData->testCond[ch].local_object[stepNo][idx];
			P1_stepCond->faultCanFreeze_I = (unsigned long)tmp;
			//ktg_230827e
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
