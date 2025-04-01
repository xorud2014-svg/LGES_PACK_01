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
extern volatile S_COC_CLIENT *myPs;
extern char psName[PROCESS_NAME_SIZE];

int Initialize(int argc, char *argv[])
{
	int CO_No, group;//, i, j;

	if(argc != 1) {
		printf("COC_Client start fail %d\n", argc);
		return -1;
	}

	if(Open_SystemMemory(0) < 0) return -2;
	
	CO_No = argv[0][5] - 0x31;
	myPs = &(myData->COC_Client[CO_No]);
	
	Init_SystemMemory(CO_No);
	
	if(Read_COC_Client_Config() < 0) return -3;

	//kjg_w Read_COC_Client_SetData - ConfigData
	if(Read_COC_Client_testConfig() < 0) return -4;
	
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

	myData->AppControl.signal[myPs->misc.psSignal] = P1;
	return 0;
}

void Init_SystemMemory(int CO_No)
{
	memset((char *)&psName, 0, PROCESS_NAME_SIZE);
	sprintf(psName, "COC%d", CO_No+1);
   	myPs->misc.psSignal = APP_SIG_COC1_CLIENT_PROCESS + CO_No;

	memset((char *)&myPs->signal, 0, MAX_SIGNAL);
	
	memset((char *)&myPs->rcvCmd, 0, sizeof(S_P3_RCV_COMMAND));
	memset((char *)&myPs->rcvPacket, 0, sizeof(S_P3_RCV_PACKET));
	
	memset((char *)&myPs->reply, 0, sizeof(S_P3_REPLY));

	myPs->misc.cmd_serial = 0;
	myPs->misc.net_time = 0;
	myPs->misc.net_time2 = 0;
	myPs->misc.ping_time = 0;
	myPs->misc.ping_time2 = 0;
	myPs->misc.sent_monitor_data_time = 0;
	myPs->misc.sent_monitor_data_time2 = 0;

	myPs->misc.processPointer = (int)&myData;
	myPs->misc.CO_No = CO_No;
}

int	Read_COC_Client_Config(void)
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
	// /root/system_data/config/parameter/client/COC?/Client_Config
    if((fp = fopen(fileName, "r")) == NULL) {
		userlog(DEBUG_LOG, psName, "%s_Client_Config file read error\n",
		psName);
		return -1;
	}

    tmp = fscanf(fp, "%s", temp);	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	myPs->config.groupId = (short int)atoi(buf);

    tmp = fscanf(fp, "%s", temp);	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	myPs->config.groupNo = (short int)(atoi(buf) - 1);

    tmp = fscanf(fp, "%s", temp);	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	memcpy((char *)&myPs->config.ipAddr, (char *)&buf, 16);
		
    tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->config.sendPort = atoi(buf);

    tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->config.receivePort = atoi(buf);

    tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->config.networkPort = atoi(buf);

	tmp = fscanf(fp, "%s", temp);	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	myPs->config.protocol_version = atoi(buf);
    	
    tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->config.retryCount = atoi(buf);

    tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->config.replyTimeout = atol(buf);

    tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->config.netTimeout = atol(buf);

    tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->config.pingTimeout = atol(buf);

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
    myPs->config.send_monitor_data_interval = atol(buf);

    tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->config.send_save_data_interval = atol(buf);

	tmp = fscanf(fp, "%s", temp);	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	//1:pause - network fail, 2:run - network fail
	myPs->config.state_change = (unsigned char)atoi(buf);
    	
    fclose(fp);
	return 0;
}

int	Write_COC_Client_Config(void)
{
	char fileName[128];
    FILE *fp;

	memset(fileName, 0, sizeof(fileName));
	strcpy(fileName, "/root/");
	strcat(fileName, (char *)&myData->AppControl.misc.path1);
	strcat(fileName, "/config/parameter/client/");
	strcat(fileName, psName);
	strcat(fileName, "/Client_Config");
	// /root/system_data/config/parameter/client/COC?/Client_Config
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
	fprintf(fp, "state_change        :   %d\n", myPs->config.state_change);
    	
    fclose(fp);
	return 0;
}

int	Read_COC_Client_testConfig(void)
{
    int tmp;
	char temp[32], buf[12], fileName[128];
    FILE *fp;

	memset((char *)&myPs->testConfig, 0, sizeof(S_P3_TEST_CONFIG));

	memset(fileName, 0, sizeof(fileName));
	strcpy(fileName, "/root/");
	strcat(fileName, (char *)&myData->AppControl.misc.path1);
	strcat(fileName, "/config/parameter/client/");
	strcat(fileName, psName);
	strcat(fileName, "/testConfig");
	// /root/system_data/config/parameter/client/COC?/testConfig
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

void convert_test_cond(int ch)
{
	int step, idx, i, fault_count;
	long attr_count, type;

	memset((char *)&myData->testCond[ch], 0, sizeof(S_TEST_CONDITION));

	//common_header
	idx = IDX_COM_OBJ_TOTAL_STEP;
	myData->testCond[ch].common_object[idx]
		= (long)myPs->testCond.header.totalStep;

	idx = IDX_COM_OBJ_ATTRIBUTE_COUNT;
	attr_count = 1;
	if(myData->ChAttribute[ch].opType != OP_INDEPENDENT) {
		for(i=0; i < 3; i++) {
			if(myData->ChAttribute[ch].chNo_slave[i] != 0) attr_count++;
		}
	}
	myData->testCond[ch].common_object[idx] = attr_count;

	//common_safety
	fault_count = 1;

	idx = IDX_COM_OBJ_FAULT_HW_UPPER_V;
	myData->testCond[ch].common_object[idx] = 5000; //5.0%
	make2_com_chCode_cond(ch, C_CD_COM_FAULT_HW_UPPER_V,
		fault_count, COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_COM_OBJ_FAULT_HW_LOWER_V;
	myData->testCond[ch].common_object[idx] = 5000; //5.0%
	make2_com_chCode_cond(ch, C_CD_COM_FAULT_HW_LOWER_V,
		fault_count, COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_COM_OBJ_FAULT_HW_UPPER_I;
	myData->testCond[ch].common_object[idx] = 5000; //5.0%
	make2_com_chCode_cond(ch, C_CD_COM_FAULT_HW_UPPER_I, 1,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_COM_OBJ_FAULT_HW_LOWER_I;
	myData->testCond[ch].common_object[idx] = 5000; //5.0%
	make2_com_chCode_cond(ch, C_CD_COM_FAULT_HW_LOWER_I, 1,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=
/*kjg_w
	idx = IDX_COM_OBJ_FAULT_HW_UPPER_TEMP;
	myData->testCond[ch].common_object[idx] = 100000; //100.0degree/C
	idx = IDX_COM_OBJ_FAULT_HW_LOWER_TEMP;
	myData->testCond[ch].common_object[idx] = -20000; //-20.0degree/C*/

	idx = IDX_COM_OBJ_FAULT_UPPER_V;
	myData->testCond[ch].common_object[idx] = myPs->testCond.safety.faultUpperV;
	make2_com_chCode_cond(ch, C_CD_COM_FAULT_UPPER_V,
		fault_count, COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_COM_OBJ_FAULT_LOWER_V;
	myData->testCond[ch].common_object[idx] = myPs->testCond.safety.faultLowerV;
	make2_com_chCode_cond(ch, C_CD_COM_FAULT_LOWER_V,
		fault_count, COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=

	idx = IDX_COM_OBJ_FAULT_UPPER_I;
	myData->testCond[ch].common_object[idx] = myPs->testCond.safety.faultUpperI;
	make2_com_chCode_cond(ch, C_CD_COM_FAULT_UPPER_I, 1,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_COM_OBJ_FAULT_LOWER_I;
	myData->testCond[ch].common_object[idx] = myPs->testCond.safety.faultLowerI;
	make2_com_chCode_cond(ch, C_CD_COM_FAULT_LOWER_I, 1,
		COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=

	idx = IDX_COM_OBJ_FAULT_UPPER_AMPARE_HOUR;
	myData->testCond[ch].common_object[idx]
		= myPs->testCond.safety.faultUpperC;
	make2_com_chCode_cond(ch, C_CD_COM_FAULT_UPPER_AMPARE_HOUR, 1,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_COM_OBJ_FAULT_LOWER_AMPARE_HOUR;
	myData->testCond[ch].common_object[idx]
		= myPs->testCond.safety.faultLowerC;
	make2_com_chCode_cond(ch, C_CD_COM_FAULT_LOWER_AMPARE_HOUR, 1,
		COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=
/*kjg_w
	idx = IDX_COM_OBJ_FAULT_UPPER_CAPACITANCE;
	myData->testCond[ch].common_object[idx]
		= myPs->testCond.safety.faultUpper_Capacitance;
	make2_com_chCode_cond(ch, C_CD_COM_FAULT_UPPER_CAPACITANCE, 1,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_COM_OBJ_FAULT_LOWER_CAPACITANCE;
	myData->testCond[ch].common_object[idx]
		= myPs->testCond.safety.faultLower_Capacitance;
	make2_com_chCode_cond(ch, C_CD_COM_FAULT_LOWER_CAPACITANCE, 1,
		COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=
*/
	idx = IDX_COM_OBJ_FAULT_UPPER_TEMP;
	myData->testCond[ch].common_object[idx]
		= myPs->testCond.safety.faultUpperTemp;
	make2_com_chCode_cond(ch, C_CD_COM_FAULT_UPPER_TEMP, 1,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_COM_OBJ_FAULT_LOWER_TEMP;
	myData->testCond[ch].common_object[idx]
		= myPs->testCond.safety.faultLowerTemp;
	make2_com_chCode_cond(ch, C_CD_COM_FAULT_LOWER_TEMP, 1,
		COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=

	//common_cycle_branch

	//step_condition
	for(step=0; step < myPs->testCond.header.totalStep; step++) {
		//header
		idx = IDX_LOC_OBJ_CLIENT_STEP_NO;
		myData->testCond[ch].local_object[step][idx]
			= (long)myPs->testCond.step[step].header.stepNo;

		idx = IDX_LOC_OBJ_STEP_NO;
		myData->testCond[ch].local_object[step][idx]
			= (long)myPs->testCond.step[step].header.stepNo;

		idx = IDX_LOC_OBJ_TYPE;
		type = convert_step_type(CONVERT_P3_TO_ORG,
			(long)myPs->testCond.step[step].header.type);
		myData->testCond[ch].local_object[step][idx] = type;

//		idx = IDX_LOC_OBJ_TEST_END; //kjg_w
//		myData->testCond[ch].local_object[step][idx]
//			= (long)myPs->testCond.step[step].header.testEnd;

//refTemp, useSocFlag, subStep //kjg_w

		switch(type) {
			case STEP_IDLE:
			case STEP_END:
				break;
			case STEP_CYCLE:
				convert_test_cond_cycle(ch, step, type, attr_count);
				break;
			case STEP_LOOP:
				convert_test_cond_loop(ch, step, type, attr_count);
				break;
			default:
				convert_test_cond_default(ch, step, type, attr_count);
				break;
		}
	}
}

void convert_test_cond_cycle(int ch, int step, long type, long attr_count)
{
	int idx;
/*
	idx = IDX_LOC_OBJ_END_MULTI_CYCLE_COUNT_COMPARE;
	myData->testCond[ch].local_object[step][idx] = myPs->testCond.step[step]
		.reference[0].AmpareHour_Branch_MultiCycleCountId;
*/
	idx = IDX_LOC_OBJ_END_SUM_AMPARE_HOUR;
	myData->testCond[ch].local_object[step][idx]
		= myPs->testCond.step[step].reference[0].endC;
	if(myData->testCond[ch].local_object[step][idx] >= 0) {
		make2_loc_chCode_cond(ch, step, C_CD_END_SUM_AMPARE_HOUR, 1,
			COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=
	} else {
		make2_loc_chCode_cond(ch, step, C_CD_END_SUM_AMPARE_HOUR, 1,
			COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=
	}

	idx = IDX_LOC_OBJ_END_SUM_AMPARE_HOUR_TYPE;
	myData->testCond[ch].local_object[step][idx] = (long)myPs->testCond
		.step[step].reference[0].endCGoto; //0:all, 1:charge, 2:discharge

	idx = IDX_LOC_OBJ_END_SUM_AMPARE_HOUR_BRANCH;
	myData->testCond[ch].local_object[step][idx] = ACTIVE_DIV_CURRENT_LOOP;

	idx = IDX_LOC_OBJ_END_SUM_WATT_HOUR;
	myData->testCond[ch].local_object[step][idx]
		= myPs->testCond.step[step].reference[0].endWattHour;
	if(myData->testCond[ch].local_object[step][idx] >= 0) {
		make2_loc_chCode_cond(ch, step, C_CD_END_SUM_WATT_HOUR, 1,
			COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=
	} else {
		make2_loc_chCode_cond(ch, step, C_CD_END_SUM_WATT_HOUR, 1,
			COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=
	}

	idx = IDX_LOC_OBJ_END_SUM_WATT_HOUR_TYPE;
	myData->testCond[ch].local_object[step][idx] = (long)myPs->testCond
		.step[step].reference[0].endTGoto; //0:all,1:charge,2:discharge

	idx = IDX_LOC_OBJ_END_SUM_WATT_HOUR_BRANCH;
	myData->testCond[ch].local_object[step][idx] = ACTIVE_DIV_CURRENT_LOOP;

	idx = IDX_LOC_OBJ_END_SUM_TIME;
	myData->testCond[ch].local_object[step][idx]
		= myPs->testCond.step[step].reference[0].endT;
	make2_loc_chCode_cond(ch, step, C_CD_END_SUM_TIME, 1,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_LOC_OBJ_END_SUM_TIME_BRANCH;
	myData->testCond[ch].local_object[step][idx] = ACTIVE_DIV_CURRENT_LOOP;
}

void convert_test_cond_loop(int ch, int step, long type, long attr_count)
{
	int idx;

	idx = IDX_LOC_OBJ_END_CYCLE_COUNT;
	myData->testCond[ch].local_object[step][idx]
		= myPs->testCond.step[step].reference[0].cycleCount;

	idx = IDX_LOC_OBJ_END_CYCLE_COUNT_BRANCH;
	myData->testCond[ch].local_object[step][idx]
		= myPs->testCond.step[step].reference[0].GotoCondition;
	if(myData->testCond[ch].local_object[step][idx] == ACTIVE_DIV_NONE)
		myData->testCond[ch].local_object[step][idx] = ACTIVE_DIV_NEXT;

	idx = IDX_LOC_OBJ_END_CYCLE_COUNT_BRANCH;
	if(myData->testCond[ch].local_object[step][idx] == ACTIVE_DIV_NEXT) {
		idx = IDX_LOC_OBJ_END_MULTI_CYCLE_COUNT;
		myData->testCond[ch].local_object[step][idx] = 0;
	} else {
		idx = IDX_LOC_OBJ_END_MULTI_CYCLE_COUNT;
		myData->testCond[ch].local_object[step][idx]
			= myPs->testCond.step[step].reference[0].gotoCycleCount;
	}

	idx = IDX_LOC_OBJ_END_MULTI_CYCLE_COUNT_BRANCH;
	myData->testCond[ch].local_object[step][idx] = ACTIVE_DIV_NEXT;

	idx = IDX_LOC_OBJ_END_MULTI_CYCLE_COUNT_COMPARE;
	myData->testCond[ch].local_object[step][idx] = 0;

	idx = IDX_LOC_OBJ_CYCLE_PAUSE;
	myData->testCond[ch].local_object[step][idx]
		= (long)myPs->testCond.step[step].header.cycle_pause;
	//0:none, 1:pause(element_cycle count)
}

void convert_test_cond_default(int ch, int step, long type, long attr_count)
{
	int idx, i, j, grade, fault_count, use_flag;

	//end_condition
	if(type == STEP_OCV) {
		make2_loc_chCode_cond(ch, step, C_CD_END_OCV, 1, COMP_NONE, 0);
	}

	idx = IDX_LOC_OBJ_END_TIME;
	myData->testCond[ch].local_object[step][idx]
		= myPs->testCond.step[step].reference[0].endT;
	//kjg_110524 if(type == STEP_Z) myData->testCond[ch].local_object[step][idx] += 20;
	make2_loc_chCode_cond(ch, step, C_CD_END_TIME, 1,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_LOC_OBJ_END_TIME_BRANCH;
	myData->testCond[ch].local_object[step][idx]
		= myPs->testCond.step[step].reference[0].endTGoto;
	if(myData->testCond[ch].local_object[step][idx] == ACTIVE_DIV_NONE)
		myData->testCond[ch].local_object[step][idx] = ACTIVE_DIV_NEXT;
/*
	idx = IDX_LOC_OBJ_END_CV_TIME;
	myData->testCond[ch].local_object[step][idx]
		= myPs->testCond.step[step].reference[0].endT_CV;
	make2_loc_chCode_cond(ch, step, C_CD_END_CV_TIME, 1,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_LOC_OBJ_END_CV_TIME_BRANCH;
	myData->testCond[ch].local_object[step][idx]
		= myPs->testCond.step[step].reference[0].endTCVGoto;
	if(myData->testCond[ch].local_object[step][idx] == ACTIVE_DIV_NONE)
		myData->testCond[ch].local_object[step][idx] = ACTIVE_DIV_NEXT;
*/
	idx = IDX_LOC_OBJ_END_AUX_TEMP_UPPER;
	myData->testCond[ch].local_object[step][idx]
		= myPs->testCond.step[step].reference[0].endTemp;
	make2_loc_chCode_cond(ch, step, C_CD_END_TEMP_UPPER, 1,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_LOC_OBJ_END_AUX_TEMP_UPPER_BRANCH;
//	myData->testCond[ch].local_object[step][idx]
//		= myPs->testCond.step[step].reference[0].endTCVGoto;
//	if(myData->testCond[ch].local_object[step][idx] == ACTIVE_DIV_NONE)
		myData->testCond[ch].local_object[step][idx] = ACTIVE_DIV_NEXT;
/*
	idx = IDX_LOC_OBJ_END_AUX_TEMP_LOWER;
	myData->testCond[ch].local_object[step][idx]
		= myPs->testCond.step[step].reference[0].endTemp;
	make2_loc_chCode_cond(ch, step, C_CD_END_TEMP_LOWER, 1,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_LOC_OBJ_END_AUX_TEMP_LOWER_BRANCH;
//	myData->testCond[ch].local_object[step][idx]
//		= myPs->testCond.step[step].reference[0].endTCVGoto;
//	if(myData->testCond[ch].local_object[step][idx] == ACTIVE_DIV_NONE)
		myData->testCond[ch].local_object[step][idx] = ACTIVE_DIV_NEXT;
*/
	//record
	idx = IDX_LOC_OBJ_SAVE_DELTA_T;
	myData->testCond[ch].local_object[step][idx]
		= (long)myPs->testCond.step[step].record.time;
		//= 1; //kjg_d 10ms
	make2_loc_chCode_cond(ch, step, C_CD_SAVE_DELTA_T, 1,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_LOC_OBJ_SAVE_DELTA_V;
	myData->testCond[ch].local_object[step][idx]
		= myPs->testCond.step[step].record.deltaV;
	make2_loc_chCode_cond(ch, step, C_CD_SAVE_DELTA_V, 1,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_LOC_OBJ_SAVE_DELTA_I;
	myData->testCond[ch].local_object[step][idx]
		= myPs->testCond.step[step].record.deltaI;
	make2_loc_chCode_cond(ch, step, C_CD_SAVE_DELTA_I, 1,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_LOC_OBJ_SAVE_DELTA_TEMP;
	myData->testCond[ch].local_object[step][idx]
		= myPs->testCond.step[step].record.deltaT;
	make2_loc_chCode_cond(ch, step, C_CD_SAVE_DELTA_TEMP, 1,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_LOC_OBJ_SAVE_DELTA_P;
	myData->testCond[ch].local_object[step][idx]
		= myPs->testCond.step[step].record.deltaP;
	make2_loc_chCode_cond(ch, step, C_CD_SAVE_DELTA_P, 1,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	//edlc -> record
	switch(myData->AppControl.config.systemModel) {
		case C_SDI_5V_60A_10A_1A:
		case C_SDI_5V_250A_25A_5A:
		case C_SBL_5V_250A_25A_5A:
		case C_SBL_5V_250A_60A_10A:
		case C_SDI_5V_300A_60A_10A:
		case C_SDI_5V_450A_60A_10A:
		case C_SBL_5V_450A_240A_60A_10A:
		case C_SBL_5V_450A_240A_60A_10A_2:
			use_flag = 1;
			break;
		default:
			use_flag = 0;
			break;
	}

	if(use_flag == 1) {
		idx = IDX_LOC_OBJ_SAVE_RECORD_T1;
		myData->testCond[ch].local_object[step][idx]
			= (long)myPs->testCond.step[step].edlc.startT_Z;

		idx = IDX_LOC_OBJ_SAVE_RECORD_T2;
		myData->testCond[ch].local_object[step][idx]
			= (long)myPs->testCond.step[step].edlc.endT_Z;

		idx = IDX_LOC_OBJ_SAVE_RECORD_T3;
		myData->testCond[ch].local_object[step][idx]
			= (long)myPs->testCond.step[step].edlc.startT_LC;
	}

	//fault
	fault_count = 1;

	idx = IDX_LOC_OBJ_FAULT_UPPER_V;
	myData->testCond[ch].local_object[step][idx]
		= myPs->testCond.step[step].faultUpperV;
	make2_loc_chCode_cond(ch, step, C_CD_FAULT_UPPER_V, fault_count,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_LOC_OBJ_FAULT_LOWER_V;
	myData->testCond[ch].local_object[step][idx]
		= myPs->testCond.step[step].faultLowerV;
	make2_loc_chCode_cond(ch, step, C_CD_FAULT_LOWER_V, fault_count,
		COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=

	idx = IDX_LOC_OBJ_FAULT_UPPER_TEMP;
	myData->testCond[ch].local_object[step][idx]
		= myPs->testCond.step[step].faultUpperTemp;
	make2_loc_chCode_cond(ch, step, C_CD_FAULT_UPPER_TEMP, 1,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_LOC_OBJ_FAULT_LOWER_TEMP;
	myData->testCond[ch].local_object[step][idx]
		= myPs->testCond.step[step].faultLowerTemp;
	make2_loc_chCode_cond(ch, step, C_CD_FAULT_LOWER_TEMP, 1,
		COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=

	//step grade
	for(grade=0; grade < MAX_P3_GRADE_ITEM; grade++) {
		i = (int)myPs->testCond.step[step].grade[grade].item;
		if(i != P3_GRADE_ITEM_IDLE) {
			myData->testCond[ch].grade[step].item = (unsigned char)i;

			j = (int)myPs->testCond.step[step].grade[grade].gradeStepCount;
			if(j > MAX_TEST_GRADE) j = MAX_TEST_GRADE;
			myData->testCond[ch].grade[step].totalGrade = (unsigned char)j;

			for(i=0; i < j; i++) {
				myData->testCond[ch].grade[step].code[i]
					= (long)myPs->testCond.step[step].grade[grade]
					.gradeStep[i].gradeCode;

				myData->testCond[ch].grade[step].value1[i]
					= myPs->testCond.step[step].grade[grade]
					.gradeStep[i].lowerValue;

				myData->testCond[ch].grade[step].value2[i]
					= myPs->testCond.step[step].grade[grade]
					.gradeStep[i].upperValue;
			}
			break;
		}
	}

	switch(type) {
		case STEP_OCV:
		case STEP_REST:
			idx = IDX_LOC_OBJ_END_V_UPPER;
			myData->testCond[ch].local_object[step][idx]
				= myPs->testCond.step[step].reference[0].endV;
			make2_loc_chCode_cond(ch, step, C_CD_END_V_UPPER, 1,
				COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

			idx = IDX_LOC_OBJ_END_V_UPPER_BRANCH;
			myData->testCond[ch].local_object[step][idx] = myPs->testCond
				.step[step].reference[0].endVGoto;
			if(myData->testCond[ch].local_object[step][idx] == ACTIVE_DIV_NONE)
				myData->testCond[ch].local_object[step][idx] = ACTIVE_DIV_NEXT;
/*
			idx = IDX_LOC_OBJ_END_V_LOWER;
			myData->testCond[ch].local_object[step][idx]
				= myPs->testCond.step[step].reference[0].V_Lower_MultiCycleCount;
			make2_loc_chCode_cond(ch, step, C_CD_END_V_LOWER, 1,
				COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=

			idx = IDX_LOC_OBJ_END_V_LOWER_BRANCH;
			myData->testCond[ch].local_object[step][idx] = myPs->testCond
				.step[step].reference[0].V_Lower_Branch_CycleCount;
			if(myData->testCond[ch].local_object[step][idx] == ACTIVE_DIV_NONE)
				myData->testCond[ch].local_object[step][idx] = ACTIVE_DIV_NEXT;
*/
			break;
		case STEP_CHARGE:
			idx = IDX_LOC_OBJ_END_V_UPPER;
			myData->testCond[ch].local_object[step][idx]
				= myPs->testCond.step[step].reference[0].endV;
			make2_loc_chCode_cond(ch, step, C_CD_END_V_UPPER, 1,
				COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

			idx = IDX_LOC_OBJ_END_V_UPPER_BRANCH;
			myData->testCond[ch].local_object[step][idx] = myPs->testCond
				.step[step].reference[0].endVGoto;
			if(myData->testCond[ch].local_object[step][idx] == ACTIVE_DIV_NONE)
				myData->testCond[ch].local_object[step][idx] = ACTIVE_DIV_NEXT;
/*
			idx = IDX_LOC_OBJ_END_V_LOWER;
			myData->testCond[ch].local_object[step][idx]
				= myPs->testCond.step[step].reference[0].V_Lower_MultiCycleCount;
			make2_loc_chCode_cond(ch, step, C_CD_END_V_LOWER, 1,
				COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=

			idx = IDX_LOC_OBJ_END_V_LOWER_BRANCH;
			myData->testCond[ch].local_object[step][idx] = myPs->testCond
				.step[step].reference[0].V_Lower_Branch_CycleCount;
			if(myData->testCond[ch].local_object[step][idx] == ACTIVE_DIV_NONE)
				myData->testCond[ch].local_object[step][idx] = ACTIVE_DIV_NEXT;
*/
			convert_test_cond_charge_discharge_z(ch, step, type, attr_count);
			break;
		case STEP_DISCHARGE:
		case STEP_Z:
/*			idx = IDX_LOC_OBJ_END_V_UPPER;
			myData->testCond[ch].local_object[step][idx]
				= myPs->testCond.step[step].reference[0].endV;
			make2_loc_chCode_cond(ch, step, C_CD_END_V_UPPER, 1,
				COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

			idx = IDX_LOC_OBJ_END_V_UPPER_BRANCH;
			myData->testCond[ch].local_object[step][idx] = myPs->testCond
				.step[step].reference[0].endVGoto;
			if(myData->testCond[ch].local_object[step][idx] == ACTIVE_DIV_NONE)
				myData->testCond[ch].local_object[step][idx] = ACTIVE_DIV_NEXT;
*/
			idx = IDX_LOC_OBJ_END_V_LOWER;
			myData->testCond[ch].local_object[step][idx]
				= myPs->testCond.step[step].reference[0].endV;
			make2_loc_chCode_cond(ch, step, C_CD_END_V_LOWER, 1,
				COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=

			idx = IDX_LOC_OBJ_END_V_LOWER_BRANCH;
			myData->testCond[ch].local_object[step][idx] = myPs->testCond
				.step[step].reference[0].endVGoto;
			if(myData->testCond[ch].local_object[step][idx] == ACTIVE_DIV_NONE)
				myData->testCond[ch].local_object[step][idx] = ACTIVE_DIV_NEXT;

			convert_test_cond_charge_discharge_z(ch, step, type, attr_count);
			break;
		case STEP_PATTERN:
			idx = IDX_LOC_OBJ_END_V_UPPER;
			myData->testCond[ch].local_object[step][idx]
				= myPs->testCond.step[step].reference[0].endV;
			make2_loc_chCode_cond(ch, step, C_CD_END_V_UPPER, 1,
				COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

			idx = IDX_LOC_OBJ_END_V_UPPER_BRANCH;
			myData->testCond[ch].local_object[step][idx] = myPs->testCond
				.step[step].reference[0].endVGoto;
			if(myData->testCond[ch].local_object[step][idx] == ACTIVE_DIV_NONE)
				myData->testCond[ch].local_object[step][idx] = ACTIVE_DIV_NEXT;

			idx = IDX_LOC_OBJ_END_V_LOWER;
			myData->testCond[ch].local_object[step][idx]
				= myPs->testCond.step[step].reference[0].endI;
			make2_loc_chCode_cond(ch, step, C_CD_END_V_LOWER, 1,
				COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=

			idx = IDX_LOC_OBJ_END_V_LOWER_BRANCH;
			myData->testCond[ch].local_object[step][idx] = myPs->testCond
				.step[step].reference[0].endVGoto;
			if(myData->testCond[ch].local_object[step][idx] == ACTIVE_DIV_NONE)
				myData->testCond[ch].local_object[step][idx] = ACTIVE_DIV_NEXT;

			convert_test_cond_pattern(ch, step, type, attr_count);
			break;
		default: break;
	}
}

void convert_test_cond_charge_discharge_z(int ch, int step, long type, long attr_count)
{
	int idx, idx2, fault_count, use_flag;
	long mode, refV, refI, l_val; //, l_val2, tmp;
	double d_val;

	//header
	idx = IDX_LOC_OBJ_MODE;
	mode = convert_step_mode(CONVERT_P3_TO_ORG,
		(long)myPs->testCond.step[step].header.mode);
	myData->testCond[ch].local_object[step][idx] = mode;

	//kjg_w idx = IDX_LOC_OBJ_TOTAL_SUB_STEP;
	//myData->testCond[ch].local_object[step][idx]
	//	= (long)myPs->testCond.step[step].header.subStep;

	//reference
	if(type == STEP_CHARGE) {
		idx = IDX_LOC_OBJ_REF_V;
		refV = myPs->testCond.step[step].reference[0].refV;
		if(refV > myData->mData.config.maxV[0]) {
			refV = myData->mData.config.maxV[0];
		} else if(refV < myData->mData.config.minV[0]) {
			refV = myData->mData.config.minV[0];
		}
		myData->testCond[ch].local_object[step][idx] = refV;

		idx = IDX_LOC_OBJ_REF_V2;
		myData->testCond[ch].local_object[step][idx]
			= myData->mData.config.minV[0];
	} else {
		idx = IDX_LOC_OBJ_REF_V;
		myData->testCond[ch].local_object[step][idx]
			= myData->mData.config.maxV[0];

		idx = IDX_LOC_OBJ_REF_V2;
		refV = myPs->testCond.step[step].reference[0].refV;
		if(refV > myData->mData.config.maxV[0]) {
			refV = myData->mData.config.maxV[0];
		} else if(refV < myData->mData.config.minV[0]) {
			refV = myData->mData.config.minV[0];
		}
		myData->testCond[ch].local_object[step][idx] = refV;
	}

	idx = IDX_LOC_OBJ_RANGE_V; //kjg_w
	myData->testCond[ch].local_object[step][idx] = RANGE1 - 1;

	refI = myPs->testCond.step[step].reference[0].refI;
	if(type == STEP_CHARGE) {
		if(mode == MODE_CP) {
			idx = IDX_LOC_OBJ_REF_P;
			myData->testCond[ch].local_object[step][idx] = refI;
			//refI = myData->mData.config.maxI[0] * attr_count;
			//kjg_110530
			if(myData->cData[ch].op.Vsens <= 500000) { //0.5V
				d_val = myData->mData.config.maxI[0] * attr_count;
			} else if(myData->cData[ch].op.Vsens <= 1000000) { //1.0V
				d_val = (double)refI / 500000.0;
				d_val *= 1000000000.0;
				if(d_val > myData->mData.config.maxI[0] * attr_count)
					d_val = myData->mData.config.maxI[0] * attr_count;
			} else if(myData->cData[ch].op.Vsens <= 2000000) { //2.0V
				d_val = (double)refI / 1000000.0;
				d_val *= 1000000000.0;
				if(d_val > myData->mData.config.maxI[0] * attr_count)
					d_val = myData->mData.config.maxI[0] * attr_count;
			} else { //2.0V over
				d_val = (double)refI / 2000000.0;
				d_val *= 1000000000.0;
				if(d_val > myData->mData.config.maxI[0] * attr_count)
					d_val = myData->mData.config.maxI[0] * attr_count;
			}
			refI = (long)d_val;
		} else if(mode == MODE_CR) {
			idx = IDX_LOC_OBJ_REF_R;
			myData->testCond[ch].local_object[step][idx] = refI / 1000;
			d_val = (double)refV / (double)refI * 1000000.0;
			refI = (long)d_val;
		}
	} else if(type == STEP_DISCHARGE || type == STEP_Z) {
		if(mode == MODE_CP) {
			idx = IDX_LOC_OBJ_REF_P;
			myData->testCond[ch].local_object[step][idx] = refI;
			//refI = myData->mData.config.maxI[0] * attr_count;
			//kjg_110530
			d_val = (double)refI / 1000000.0; //1.0V
			d_val *= 1000000000.0;
			if(d_val > myData->mData.config.maxI[0] * attr_count)
				d_val = myData->mData.config.maxI[0] * attr_count;
			refI = (long)d_val;
		} else if(mode == MODE_CR) {
			idx = IDX_LOC_OBJ_REF_R;
			myData->testCond[ch].local_object[step][idx] = refI / 1000;
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
	myData->testCond[ch].local_object[step][idx] = refI;

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

//		l_val2 = (long)myPs->testCond.step[step].reference[0].rangeI;
//		if(l_val2 == 0) { //auto range
			myData->testCond[ch].local_object[step][idx] = l_val;
//		} else { //manual range
//			if(l_val2 < l_val) {
//				myData->testCond[ch].local_object[step][idx] = l_val2;
//			} else {
//				myData->testCond[ch].local_object[step][idx] = l_val;
//			}
//		}
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

//		l_val2 = (long)myPs->testCond.step[step].reference[0].rangeI;
//		if(l_val2 == 0) { //auto range
			myData->testCond[ch].local_object[step][idx] = l_val;
//		} else { //manual range
//			if(l_val2 < l_val) {
//				myData->testCond[ch].local_object[step][idx] = l_val2;
//			} else {
//				myData->testCond[ch].local_object[step][idx] = l_val;
//			}
//		}
	} else {
		if(step == 0) {
			l_val = RANGE1 - 1;
		} else {
			l_val = myData->testCond[ch].local_object[step-1][idx];
		}
		myData->testCond[ch].local_object[step][idx] = l_val;
	} 

	//kjg_w for(i=0; i < myPs->testCond.step[step].header.subStep; i++) {
	//	myData->testCond[ch].step[step].refV[i]
	//		= myPs->testCond.step[step].reference[i].refV;
	//	...
	//}
	
	//end_condition
	idx = IDX_LOC_OBJ_END_AMPARE_HOUR;
	myData->testCond[ch].local_object[step][idx]
		= myPs->testCond.step[step].reference[0].endC;
	if(myData->testCond[ch].local_object[step][idx] >= 0) {
		make2_loc_chCode_cond(ch, step, C_CD_END_AMPARE_HOUR, 1,
			COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=
	} else { //kjg_100103
		make2_loc_chCode_cond(ch, step, C_CD_END_AMPARE_HOUR, 1,
			COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=
	}

	idx = IDX_LOC_OBJ_END_AMPARE_HOUR_BRANCH;
	myData->testCond[ch].local_object[step][idx] = myPs->testCond.step[step]
		.reference[0].endCGoto;
	if(myData->testCond[ch].local_object[step][idx] == ACTIVE_DIV_NONE)
		myData->testCond[ch].local_object[step][idx] = ACTIVE_DIV_NEXT;

	idx = IDX_LOC_OBJ_END_WATT_HOUR;
	myData->testCond[ch].local_object[step][idx]
		= myPs->testCond.step[step].reference[0].endWattHour;
	if(myData->testCond[ch].local_object[step][idx] >= 0) {
		make2_loc_chCode_cond(ch, step, C_CD_END_WATT_HOUR, 1,
			COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=
	} else { //kjg_100103
		make2_loc_chCode_cond(ch, step, C_CD_END_WATT_HOUR, 1,
			COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=
	}
			
	idx = IDX_LOC_OBJ_END_WATT_HOUR_BRANCH;
//	myData->testCond[ch].local_object[step][idx] = myPs->testCond.step[step]
//		.reference[0].WattHour_Branch_MultiCycleCount_Branch;
//	if(myData->testCond[ch].local_object[step][idx] == ACTIVE_DIV_NONE)
		myData->testCond[ch].local_object[step][idx] = ACTIVE_DIV_NEXT;

//	tmp = (long)myPs->testCond.step[step].reference[0].ValueRate_Item;
//	if(tmp == P3_VALUE_RATE_ITEM_AMPARE_HOUR) {
		idx = IDX_LOC_OBJ_END_AMPARE_HOUR_RATE;
		myData->testCond[ch].local_object[step][idx]
			= myPs->testCond.step[step].reference[0].endSoc;
		make2_loc_chCode_cond(ch, step, C_CD_END_VALUE_RATE_AMPARE_HOUR, 1,
			COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

		idx = IDX_LOC_OBJ_END_AMPARE_HOUR_RATE_BRANCH;
//		myData->testCond[ch].local_object[step][idx]
//			= myPs->testCond.step[step].reference[0].ValueRate_Branch;
//		if(myData->testCond[ch].local_object[step][idx] == ACTIVE_DIV_NONE)
			myData->testCond[ch].local_object[step][idx] = ACTIVE_DIV_NEXT;

		idx = IDX_LOC_OBJ_END_AMPARE_HOUR_RATE_COMPARE;
		myData->testCond[ch].local_object[step][idx]
			= myPs->testCond.step[step].reference[0].socCapStepNo;
/*	} else if(tmp == P3_VALUE_RATE_ITEM_WATT_HOUR) {
		idx = IDX_LOC_OBJ_END_WATT_HOUR_RATE;
		myData->testCond[ch].local_object[step][idx]
			= myPs->testCond.step[step].reference[0].ValueRate;
		make2_loc_chCode_cond(ch, step, C_CD_END_VALUE_RATE_WATT_HOUR, 1,
			COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

		idx = IDX_LOC_OBJ_END_WATT_HOUR_RATE_BRANCH;
		myData->testCond[ch].local_object[step][idx]
			= myPs->testCond.step[step].reference[0].ValueRate_Branch;
		if(myData->testCond[ch].local_object[step][idx] == ACTIVE_DIV_NONE)
			myData->testCond[ch].local_object[step][idx] = ACTIVE_DIV_NEXT;

		idx = IDX_LOC_OBJ_END_WATT_HOUR_RATE_COMPARE;
		myData->testCond[ch].local_object[step][idx]
			= myPs->testCond.step[step].reference[0].ValueRate_Compare;
	}*/

	if(type == STEP_CHARGE) {
		idx = IDX_LOC_OBJ_END_I;
		myData->testCond[ch].local_object[step][idx]
			= myPs->testCond.step[step].reference[0].endI;
		make2_loc_chCode_cond(ch, step, C_CD_END_I, 4,
			COMP_LESS_THAN_OR_EQUAL_TO, idx); //<= kjg_111117 1 -> 4
	} else if(type == STEP_DISCHARGE || type == STEP_Z) {
		idx = IDX_LOC_OBJ_END_I;
		myData->testCond[ch].local_object[step][idx]
			= myPs->testCond.step[step].reference[0].endI * (-1);
		make2_loc_chCode_cond(ch, step, C_CD_END_I, 4,
			COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>= kjg_111117 1 -> 4
	}

	idx = IDX_LOC_OBJ_END_DELTA_V;
	myData->testCond[ch].local_object[step][idx]
		= myPs->testCond.step[step].reference[0].endDeltaV;
	make2_loc_chCode_cond(ch, step, C_CD_END_DELTA_V, 1,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_LOC_OBJ_END_P;
	myData->testCond[ch].local_object[step][idx]
		= myPs->testCond.step[step].reference[0].endWatt;
	if(type == STEP_CHARGE) {
		make2_loc_chCode_cond(ch, step, C_CD_END_P, 1,
			COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=
	} else if(type == STEP_DISCHARGE || type == STEP_Z) {
		make2_loc_chCode_cond(ch, step, C_CD_END_P, 1,
			COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=
	}

	//step comp //kjg_w
	//	= myPs->testCond.step[i].compV[comp_point];
	//	= myPs->testCond.step[i].compI[comp_point];

	//step delta //kjg_w
	//	= myPs->testCond.step[i].deltaV;
	//	= myPs->testCond.step[i].deltaI;

	//step record
//kjg_w	idx = IDX_LOC_OBJ_SAVE_DELTA_TEMP;
//	myData->testCond[ch].local_object[step][idx]
//		= myPs->testCond.step[step].record.deltaT;
//	make2_loc_chCode_cond(ch, step, C_CD_SAVE_DELTA_TEMP, 1,
//		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

//kjg_w	idx = IDX_LOC_OBJ_SAVE_DELTA_P;
//	myData->testCond[ch].local_object[step][idx]
//		= myPs->testCond.step[step].record.deltaP;
//	make2_loc_chCode_cond(ch, step, C_CD_SAVE_DELTA_P, 1,
//		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	//step edlc
	switch(myData->AppControl.config.systemModel) {
		case C_SDI_5V_60A_10A_1A:
		case C_SDI_5V_250A_25A_5A:
		case C_SBL_5V_250A_25A_5A:
		case C_SBL_5V_250A_60A_10A:
		case C_SDI_5V_300A_60A_10A:
		case C_SDI_5V_450A_60A_10A:
		case C_SBL_5V_450A_240A_60A_10A:
		case C_SBL_5V_450A_240A_60A_10A_2:
			use_flag = 0;
			break;
		default:
			use_flag = 1;
			break;
	}

	if(use_flag == 1) {
		idx = IDX_LOC_OBJ_CAPACITANCE_V1;
		myData->testCond[ch].local_object[step][idx]
			= myPs->testCond.step[step].edlc.capacitanceV1;

		idx = IDX_LOC_OBJ_CAPACITANCE_V2;
		myData->testCond[ch].local_object[step][idx]
			= myPs->testCond.step[step].edlc.capacitanceV2;

		idx = IDX_LOC_OBJ_Z_T1;
		myData->testCond[ch].local_object[step][idx] = 0;
			//kjg_w = (long)myPs->testCond.step[step].edlc.startT_Z;

		idx = IDX_LOC_OBJ_Z_T2;
		if(type == STEP_Z) {
			myData->testCond[ch].local_object[step][idx]
				= (long)myPs->testCond.step[step].reference[0].endT;
				//kjg_w = myPs->testCond.step[step].edlc.endT_Z;
		} else {
			//l_val = (long)myPs->testCond.step[step].reference[0].Time - 20;
			l_val = (long)myPs->testCond.step[step].reference[0].endT;
			if(l_val < 0) l_val = 0;
			myData->testCond[ch].local_object[step][idx] = l_val;
		}

		idx = IDX_LOC_OBJ_LC_T1;
		myData->testCond[ch].local_object[step][idx]
			= (long)myPs->testCond.step[step].edlc.startT_LC;

		idx = IDX_LOC_OBJ_LC_T2;
		myData->testCond[ch].local_object[step][idx]
			= (long)myPs->testCond.step[step].edlc.endT_LC;
	}

	//fault_condition
	fault_count = 1;

	switch(type) {
		case STEP_CHARGE:
			if(mode == MODE_CC_CV || mode == MODE_CC || mode == MODE_CV) {
				idx = IDX_LOC_OBJ_FAULT_HW_UPPER_V;
				myData->testCond[ch].local_object[step][idx] = 5000; //5.0%
				make2_loc_chCode_cond(ch, step, C_CD_FAULT_HW_UPPER_V,
					fault_count, COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=
			}

			idx = IDX_LOC_OBJ_FAULT_LOWER_DELTA_V;
			myData->testCond[ch].local_object[step][idx]
				= myPs->testConfig.fail_charge_lower_dv;
			idx2 = IDX_LOC_OBJ_FAULT_LOWER_DELTA_V_T;
			myData->testCond[ch].local_object[step][idx2] = 100; //1sec
			make2_loc_chCode_cond2(ch, step, C_CD_FAULT_LOWER_DELTA_V, 1,
				COMP_GREATER_THAN_OR_EQUAL_TO, idx, idx2); //>=
			break;
		case STEP_DISCHARGE:
			if(mode == MODE_CC_CV || mode == MODE_CC || mode == MODE_CV) {
				idx = IDX_LOC_OBJ_FAULT_HW_LOWER_V;
				myData->testCond[ch].local_object[step][idx] = 5000; //5.0%
				make2_loc_chCode_cond(ch, step, C_CD_FAULT_HW_LOWER_V,
					fault_count, COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=
			}
			break;
		case STEP_Z:
			if(mode == MODE_DC) {
				idx = IDX_LOC_OBJ_FAULT_HW_LOWER_V;
				myData->testCond[ch].local_object[step][idx] = 5000; //5.0%
				make2_loc_chCode_cond(ch, step, C_CD_FAULT_HW_LOWER_V,
					fault_count, COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=
			}
			break;
		default: break;
	}

	idx = IDX_LOC_OBJ_FAULT_HW_UPPER_I;
	myData->testCond[ch].local_object[step][idx] = 5000; //5.0%
	make2_loc_chCode_cond(ch, step, C_CD_FAULT_HW_UPPER_I, 1,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=
/*kjg_w
	idx = IDX_LOC_OBJ_FAULT_HW_LOWER_I;
	myData->testCond[ch].local_object[step][idx] = 5000; //5.0%
	make2_loc_chCode_cond(ch, step, C_CD_FAULT_HW_LOWER_I, 1,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=
*/
	idx = IDX_LOC_OBJ_FAULT_UPPER_I;
	if(type == STEP_CHARGE) {
		myData->testCond[ch].local_object[step][idx]
			= myPs->testCond.step[step].faultUpperI;
		make2_loc_chCode_cond(ch, step, C_CD_FAULT_UPPER_I, 1,
			COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=
	} else if(type == STEP_DISCHARGE || type == STEP_Z) {
		myData->testCond[ch].local_object[step][idx]
			= myPs->testCond.step[step].faultUpperI * (-1);
		make2_loc_chCode_cond(ch, step, C_CD_FAULT_UPPER_I, 1,
			COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=
	}

	idx = IDX_LOC_OBJ_FAULT_LOWER_I;
	if(type == STEP_CHARGE) {
		myData->testCond[ch].local_object[step][idx]
			= myPs->testCond.step[step].faultLowerI;
		make2_loc_chCode_cond(ch, step, C_CD_FAULT_LOWER_I, 1,
			COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=
	} else if(type == STEP_DISCHARGE || type == STEP_Z) {
		myData->testCond[ch].local_object[step][idx]
			= myPs->testCond.step[step].faultLowerI * (-1);
		make2_loc_chCode_cond(ch, step, C_CD_FAULT_LOWER_I, 1,
			COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=
	}

	if(type == STEP_Z) {
		idx = IDX_LOC_OBJ_FAULT_UPPER_Z;
		myData->testCond[ch].local_object[step][idx]
			= myPs->testCond.step[step].faultUpperZ;
		make2_loc_chCode_cond(ch, step, C_CD_FAULT_UPPER_Z, 1,
			COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

		idx = IDX_LOC_OBJ_FAULT_LOWER_Z;
		myData->testCond[ch].local_object[step][idx]
			= myPs->testCond.step[step].faultLowerZ;
		make2_loc_chCode_cond(ch, step, C_CD_FAULT_LOWER_Z, 1,
			COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=
	}

	idx = IDX_LOC_OBJ_FAULT_UPPER_AMPARE_HOUR;
	myData->testCond[ch].local_object[step][idx]
		= myPs->testCond.step[step].faultUpperC;
	make2_loc_chCode_cond(ch, step, C_CD_FAULT_UPPER_AMPARE_HOUR, 1,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_LOC_OBJ_FAULT_LOWER_AMPARE_HOUR;
	myData->testCond[ch].local_object[step][idx]
		= myPs->testCond.step[step].faultLowerC;
	make2_loc_chCode_cond(ch, step, C_CD_FAULT_LOWER_AMPARE_HOUR, 1,
		COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=
/*kjg_w
	idx = IDX_LOC_OBJ_FAULT_UPPER_CAPACITANCE;
	myData->testCond[ch].local_object[step][idx]
		= myPs->testCond.step[step].faultUpper_Capacitance;
	make2_loc_chCode_cond(ch, step, C_CD_FAULT_UPPER_CAPACITANCE, 1,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_LOC_OBJ_FAULT_LOWER_CAPACITANCE;
	myData->testCond[ch].local_object[step][idx]
		= myPs->testCond.step[step].faultLower_Capacitance;
	make2_loc_chCode_cond(ch, step, C_CD_FAULT_LOWER_CAPACITANCE, 1,
		COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=

	idx = IDX_LOC_OBJ_FAULT_UPPER_WATT_HOUR;
	myData->testCond[ch].local_object[step][idx]
		= myPs->testCond.step[step].faultUpper_WattHour;
	make2_loc_chCode_cond(ch, step, C_CD_FAULT_UPPER_WATT_HOUR, 1,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_LOC_OBJ_FAULT_LOWER_WATT_HOUR;
	myData->testCond[ch].local_object[step][idx]
		= myPs->testCond.step[step].faultLower_WattHour;
	make2_loc_chCode_cond(ch, step, C_CD_FAULT_LOWER_WATT_HOUR, 1,
		COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=
*/
}

void convert_test_cond_pattern(int ch, int step, long type, long attr_count)
{
	int idx, i, rtn;
	long pattern_index, mode, l_val, refI; //refV, tmp;

	//header
	idx = IDX_LOC_OBJ_PATTERN_INDEX;
	//pattern_index = myPs->testCond.step[step].header.patternIndex;
	pattern_index = (long)step;
	myData->testCond[ch].local_object[step][idx] = pattern_index;

//	memcpy((char *)&myData->testCond[ch].pattern[pattern_index][0],
//		(char *)&myPs->testCond.pattern_data[pattern_index][0],
//		sizeof(S_TEST_COND_PATTERN) * MAX_PATTERN_DATA);

	rtn = Read_Pattern_File_2(psName, ch, (int)pattern_index);

//	userlog(DEBUG_LOG, psName, "pattern_data %d, %d, %ld, %ld, %ld\n",
//		ch, pattern_index,
//		myData->testCond[ch].pattern[0].cmd_val[0],
//		myData->testCond[ch].pattern[1].cmd_val[0],
//		myData->testCond[ch].pattern[2].cmd_val[0]); //kjg_d

	idx = IDX_LOC_OBJ_MODE;
	mode = convert_step_mode(CONVERT_P3_TO_ORG,
		(long)myPs->testCond.step[step].header.mode);
	myData->testCond[ch].local_object[step][idx] = mode;

	//reference
	idx = IDX_LOC_OBJ_REF_V; //charge ref_v
/*	refV = myPs->testCond.step[step].reference[0].refV;
	if(refV > myData->mData.config.maxV[0]) {
		refV = myData->mData.config.maxV[0];
	} else if(refV < myData->mData.config.minV[0]) {
		refV = myData->mData.config.minV[0];
	}
	myData->testCond[ch].local_object[step][idx] = refV;*/
	myData->testCond[ch].local_object[step][idx]
		= myData->mData.config.maxV[0];

	idx = IDX_LOC_OBJ_REF_V2; //discharge ref_v
/*	refV = myPs->testCond.step[step].reference[0].refI;
	if(refV > myData->mData.config.maxV[0]) {
		refV = myData->mData.config.maxV[0];
	} else if(refV < myData->mData.config.minV[0]) {
		refV = myData->mData.config.minV[0];
	}
	myData->testCond[ch].local_object[step][idx] = refV;*/
	myData->testCond[ch].local_object[step][idx]
		= myData->mData.config.minV[0];

	idx = IDX_LOC_OBJ_RANGE_V; //kjg_w
	myData->testCond[ch].local_object[step][idx] = RANGE1 - 1;

	if(mode == MODE_CC) {
		refI = 0;
		for(i=1; i < MAX_PATTERN_DATA; i++) {
			l_val = myData->testCond[ch].pattern[i].t_val;
			if(l_val < 0) break;

			l_val = myData->testCond[ch].pattern[i].cmd_val[0];
			if(l_val < 0) l_val *= (-1);

			if(l_val > refI) refI = l_val;

			l_val = myData->testCond[ch].pattern[i].cmd_val[1];
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
	myData->testCond[ch].local_object[step][idx] = refI;

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
	myData->testCond[ch].local_object[step][idx] = l_val;

	//end_condition
	idx = IDX_LOC_OBJ_END_AMPARE_HOUR;
	myData->testCond[ch].local_object[step][idx]
		= myPs->testCond.step[step].reference[0].endC;
	if(myData->testCond[ch].local_object[step][idx] >= 0) {
		make2_loc_chCode_cond(ch, step, C_CD_END_AMPARE_HOUR, 1,
			COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=
	} else { //kjg_100103
		make2_loc_chCode_cond(ch, step, C_CD_END_AMPARE_HOUR, 1,
			COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=
	}

	idx = IDX_LOC_OBJ_END_AMPARE_HOUR_BRANCH;
//	myData->testCond[ch].local_object[step][idx] = myPs->testCond.step[step]
//		.reference[0].AmpareHour_Branch_MultiCycleCountId;
//	if(myData->testCond[ch].local_object[step][idx] == ACTIVE_DIV_NONE)
		myData->testCond[ch].local_object[step][idx] = ACTIVE_DIV_NEXT;

	idx = IDX_LOC_OBJ_END_WATT_HOUR;
	myData->testCond[ch].local_object[step][idx]
		= myPs->testCond.step[step].reference[0].endWattHour;
	if(myData->testCond[ch].local_object[step][idx] >= 0) {
		make2_loc_chCode_cond(ch, step, C_CD_END_WATT_HOUR, 1,
			COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=
	} else { //kjg_100103
		make2_loc_chCode_cond(ch, step, C_CD_END_WATT_HOUR, 1,
			COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=
	}
			
	idx = IDX_LOC_OBJ_END_WATT_HOUR_BRANCH;
//	myData->testCond[ch].local_object[step][idx] = myPs->testCond.step[step]
//		.reference[0].WattHour_Branch_MultiCycleCount_Branch;
//	if(myData->testCond[ch].local_object[step][idx] == ACTIVE_DIV_NONE)
		myData->testCond[ch].local_object[step][idx] = ACTIVE_DIV_NEXT;

//	tmp = (long)myPs->testCond.step[step].reference[0].ValueRate_Item;
//	if(tmp == P3_VALUE_RATE_ITEM_AMPARE_HOUR) {
		idx = IDX_LOC_OBJ_END_AMPARE_HOUR_RATE;
		myData->testCond[ch].local_object[step][idx]
			= myPs->testCond.step[step].reference[0].endSoc;
		make2_loc_chCode_cond(ch, step, C_CD_END_VALUE_RATE_AMPARE_HOUR, 1,
			COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

		idx = IDX_LOC_OBJ_END_AMPARE_HOUR_RATE_BRANCH;
//		myData->testCond[ch].local_object[step][idx]
//			= myPs->testCond.step[step].reference[0].ValueRate_Branch;
//		if(myData->testCond[ch].local_object[step][idx] == ACTIVE_DIV_NONE)
			myData->testCond[ch].local_object[step][idx] = ACTIVE_DIV_NEXT;

		idx = IDX_LOC_OBJ_END_AMPARE_HOUR_RATE_COMPARE;
		myData->testCond[ch].local_object[step][idx]
			= myPs->testCond.step[step].reference[0].socCapStepNo;
/*	} else if(tmp == P3_VALUE_RATE_ITEM_WATT_HOUR) {
		idx = IDX_LOC_OBJ_END_WATT_HOUR_RATE;
		myData->testCond[ch].local_object[step][idx]
			= myPs->testCond.step[step].reference[0].ValueRate;
		make2_loc_chCode_cond(ch, step, C_CD_END_VALUE_RATE_WATT_HOUR, 1,
			COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

		idx = IDX_LOC_OBJ_END_WATT_HOUR_RATE_BRANCH;
		myData->testCond[ch].local_object[step][idx]
			= myPs->testCond.step[step].reference[0].ValueRate_Branch;
		if(myData->testCond[ch].local_object[step][idx] == ACTIVE_DIV_NONE)
			myData->testCond[ch].local_object[step][idx] = ACTIVE_DIV_NEXT;

		idx = IDX_LOC_OBJ_END_WATT_HOUR_RATE_COMPARE;
		myData->testCond[ch].local_object[step][idx]
			= myPs->testCond.step[step].reference[0].ValueRate_Compare;
	}
*/
	//fault_condition
	idx = IDX_LOC_OBJ_FAULT_UPPER_I;
	myData->testCond[ch].local_object[step][idx]
		= myPs->testCond.step[step].faultUpperI;

	idx = IDX_LOC_OBJ_FAULT_LOWER_I;
	myData->testCond[ch].local_object[step][idx]
		= myPs->testCond.step[step].faultLowerI;

	idx = IDX_LOC_OBJ_FAULT_UPPER_AMPARE_HOUR;
	myData->testCond[ch].local_object[step][idx]
		= myPs->testCond.step[step].faultUpperC;
	make2_loc_chCode_cond(ch, step, C_CD_FAULT_UPPER_AMPARE_HOUR, 1,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_LOC_OBJ_FAULT_LOWER_AMPARE_HOUR;
	myData->testCond[ch].local_object[step][idx]
		= myPs->testCond.step[step].faultLowerC;
	make2_loc_chCode_cond(ch, step, C_CD_FAULT_LOWER_AMPARE_HOUR, 1,
		COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=
/*kjg_w
	idx = IDX_LOC_OBJ_FAULT_UPPER_CAPACITANCE;
	myData->testCond[ch].local_object[step][idx]
		= myPs->testCond.step[step].faultUpper_Capacitance;
	make2_loc_chCode_cond(ch, step, C_CD_FAULT_UPPER_CAPACITANCE, 1,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_LOC_OBJ_FAULT_LOWER_CAPACITANCE;
	myData->testCond[ch].local_object[step][idx]
		= myPs->testCond.step[step].faultLower_Capacitance;
	make2_loc_chCode_cond(ch, step, C_CD_FAULT_LOWER_CAPACITANCE, 1,
		COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=

	idx = IDX_LOC_OBJ_FAULT_UPPER_WATT_HOUR;
	myData->testCond[ch].local_object[step][idx]
		= myPs->testCond.step[step].faultUpper_WattHour;
	make2_loc_chCode_cond(ch, step, C_CD_FAULT_UPPER_WATT_HOUR, 1,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_LOC_OBJ_FAULT_LOWER_WATT_HOUR;
	myData->testCond[ch].local_object[step][idx]
		= myPs->testCond.step[step].faultLower_WattHour;
	make2_loc_chCode_cond(ch, step, C_CD_FAULT_LOWER_WATT_HOUR, 1,
		COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=
*/
}

void convert_test_cond_cable_check(int ch)
{
	int step, idx, i, fault_count;
	long attr_count, refV, refI, l_val, l_val2;

	memset((char *)&myData->testCond[ch], 0, sizeof(S_TEST_CONDITION));

	//cycle -> charge 1sec -> rest 1sec -> discharge 1sec -> rest 1sec -> loop -> end
	//common_header
	idx = IDX_COM_OBJ_TOTAL_STEP;
	myData->testCond[ch].common_object[idx] = 7;

	idx = IDX_COM_OBJ_ATTRIBUTE_COUNT;
	attr_count = 1;
	if(myData->ChAttribute[ch].opType != OP_INDEPENDENT) {
		for(i=0; i < 3; i++) {
			if(myData->ChAttribute[ch].chNo_slave[i] != 0) attr_count++;
		}
	}
	myData->testCond[ch].common_object[idx] = attr_count;

	//common_safety
	fault_count = 1;

	idx = IDX_COM_OBJ_FAULT_HW_UPPER_V;
	myData->testCond[ch].common_object[idx] = 5000; //5.0%
	make2_com_chCode_cond(ch, C_CD_COM_FAULT_HW_UPPER_V,
		fault_count, COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_COM_OBJ_FAULT_HW_LOWER_V;
	myData->testCond[ch].common_object[idx] = 5000; //5.0%
	make2_com_chCode_cond(ch, C_CD_COM_FAULT_HW_LOWER_V,
		fault_count, COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_COM_OBJ_FAULT_HW_UPPER_I;
	myData->testCond[ch].common_object[idx] = 5000; //5.0%
	make2_com_chCode_cond(ch, C_CD_COM_FAULT_HW_UPPER_I, 1,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_COM_OBJ_FAULT_HW_LOWER_I;
	myData->testCond[ch].common_object[idx] = 5000; //5.0%
	make2_com_chCode_cond(ch, C_CD_COM_FAULT_HW_LOWER_I, 1,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	//common_cycle_branch

	//step_condition
	//step_cycle
	step = 0;

	idx = IDX_LOC_OBJ_CLIENT_STEP_NO;
	myData->testCond[ch].local_object[step][idx] = step+1;

	idx = IDX_LOC_OBJ_STEP_NO;
	myData->testCond[ch].local_object[step][idx] = step;

	idx = IDX_LOC_OBJ_TYPE;
	myData->testCond[ch].local_object[step][idx] = STEP_CYCLE;

	idx = IDX_LOC_OBJ_ATTRIBUTE;
	myData->testCond[ch].local_object[step][idx] = ATTR_CHECK4_START;

	//step_charge
	step++;

	idx = IDX_LOC_OBJ_CLIENT_STEP_NO;
	myData->testCond[ch].local_object[step][idx] = step+1;

	idx = IDX_LOC_OBJ_STEP_NO;
	myData->testCond[ch].local_object[step][idx] = step;

	idx = IDX_LOC_OBJ_TYPE;
	myData->testCond[ch].local_object[step][idx] = STEP_CHARGE;

	idx = IDX_LOC_OBJ_ATTRIBUTE;
	myData->testCond[ch].local_object[step][idx] = ATTR_CHECK4_1;

	idx = IDX_LOC_OBJ_MODE;
	myData->testCond[ch].local_object[step][idx] = MODE_CC_CV;

	idx = IDX_LOC_OBJ_END_TIME;
	myData->testCond[ch].local_object[step][idx] = 100; //1sec
	make2_loc_chCode_cond(ch, step, C_CD_END_TIME, 1,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_LOC_OBJ_END_TIME_BRANCH;
	myData->testCond[ch].local_object[step][idx] = ACTIVE_DIV_NEXT;

	idx = IDX_LOC_OBJ_SAVE_DELTA_T;
	myData->testCond[ch].local_object[step][idx] = 10; //0.1sec
	make2_loc_chCode_cond(ch, step, C_CD_SAVE_DELTA_T, 1,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_LOC_OBJ_REF_V;
	refV = 4200000; //4.2V
	myData->testCond[ch].local_object[step][idx] = refV;

	idx = IDX_LOC_OBJ_REF_V2;
	myData->testCond[ch].local_object[step][idx] = 0;

	idx = IDX_LOC_OBJ_RANGE_V;
	myData->testCond[ch].local_object[step][idx] = RANGE1 - 1;

	idx = IDX_LOC_OBJ_REF_I;
	refI = 20000000; //20A
	myData->testCond[ch].local_object[step][idx] = refI;

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
			myData->testCond[ch].local_object[step][idx] = l_val;
		} else { //manual range
			if(l_val2 < l_val) {
				myData->testCond[ch].local_object[step][idx] = l_val2;
			} else {
				myData->testCond[ch].local_object[step][idx] = l_val;
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
			myData->testCond[ch].local_object[step][idx] = l_val;
		} else { //manual range
			if(l_val2 < l_val) {
				myData->testCond[ch].local_object[step][idx] = l_val2;
			} else {
				myData->testCond[ch].local_object[step][idx] = l_val;
			}
		}
	} else {
		if(step == 0) {
			l_val = RANGE1 - 1;
		} else {
			l_val = myData->testCond[ch].local_object[step-1][idx];
		}
		myData->testCond[ch].local_object[step][idx] = l_val;
	}

	//step_rest
	step++;

	idx = IDX_LOC_OBJ_CLIENT_STEP_NO;
	myData->testCond[ch].local_object[step][idx] = step+1;

	idx = IDX_LOC_OBJ_STEP_NO;
	myData->testCond[ch].local_object[step][idx] = step;

	idx = IDX_LOC_OBJ_TYPE;
	myData->testCond[ch].local_object[step][idx] = STEP_REST;

	idx = IDX_LOC_OBJ_ATTRIBUTE;
	myData->testCond[ch].local_object[step][idx] = ATTR_CHECK4_2;

	idx = IDX_LOC_OBJ_END_TIME;
	myData->testCond[ch].local_object[step][idx] = 100; //1sec
	make2_loc_chCode_cond(ch, step, C_CD_END_TIME, 1,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_LOC_OBJ_END_TIME_BRANCH;
	myData->testCond[ch].local_object[step][idx] = ACTIVE_DIV_NEXT;

	idx = IDX_LOC_OBJ_SAVE_DELTA_T;
	myData->testCond[ch].local_object[step][idx] = 100; //1.0sec
	make2_loc_chCode_cond(ch, step, C_CD_SAVE_DELTA_T, 1,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	//step_discharge
	step++;

	idx = IDX_LOC_OBJ_CLIENT_STEP_NO;
	myData->testCond[ch].local_object[step][idx] = step+1;

	idx = IDX_LOC_OBJ_STEP_NO;
	myData->testCond[ch].local_object[step][idx] = step;

	idx = IDX_LOC_OBJ_TYPE;
	myData->testCond[ch].local_object[step][idx] = STEP_DISCHARGE;

	idx = IDX_LOC_OBJ_ATTRIBUTE;
	myData->testCond[ch].local_object[step][idx] = ATTR_CHECK4_3;

	idx = IDX_LOC_OBJ_MODE;
	myData->testCond[ch].local_object[step][idx] = MODE_CC_CV;

	idx = IDX_LOC_OBJ_END_TIME;
	myData->testCond[ch].local_object[step][idx] = 100; //1sec
	make2_loc_chCode_cond(ch, step, C_CD_END_TIME, 1,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_LOC_OBJ_END_TIME_BRANCH;
	myData->testCond[ch].local_object[step][idx] = ACTIVE_DIV_NEXT;

	idx = IDX_LOC_OBJ_SAVE_DELTA_T;
	myData->testCond[ch].local_object[step][idx] = 10; //0.1sec
	make2_loc_chCode_cond(ch, step, C_CD_SAVE_DELTA_T, 1,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_LOC_OBJ_REF_V;
	refV = 4200000; //4.2V
	myData->testCond[ch].local_object[step][idx] = refV;

	idx = IDX_LOC_OBJ_REF_V2;
	myData->testCond[ch].local_object[step][idx] = 0;

	idx = IDX_LOC_OBJ_RANGE_V;
	myData->testCond[ch].local_object[step][idx] = RANGE1 - 1;

	idx = IDX_LOC_OBJ_REF_I;
	refI = -20000000; //-20A
	myData->testCond[ch].local_object[step][idx] = refI;

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
			myData->testCond[ch].local_object[step][idx] = l_val;
		} else { //manual range
			if(l_val2 < l_val) {
				myData->testCond[ch].local_object[step][idx] = l_val2;
			} else {
				myData->testCond[ch].local_object[step][idx] = l_val;
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
			myData->testCond[ch].local_object[step][idx] = l_val;
		} else { //manual range
			if(l_val2 < l_val) {
				myData->testCond[ch].local_object[step][idx] = l_val2;
			} else {
				myData->testCond[ch].local_object[step][idx] = l_val;
			}
		}
	} else {
		if(step == 0) {
			l_val = RANGE1 - 1;
		} else {
			l_val = myData->testCond[ch].local_object[step-1][idx];
		}
		myData->testCond[ch].local_object[step][idx] = l_val;
	}

	//step_rest
	step++;

	idx = IDX_LOC_OBJ_CLIENT_STEP_NO;
	myData->testCond[ch].local_object[step][idx] = step+1;

	idx = IDX_LOC_OBJ_STEP_NO;
	myData->testCond[ch].local_object[step][idx] = step;

	idx = IDX_LOC_OBJ_TYPE;
	myData->testCond[ch].local_object[step][idx] = STEP_REST;

	idx = IDX_LOC_OBJ_ATTRIBUTE;
	myData->testCond[ch].local_object[step][idx] = ATTR_CHECK4_4;

	idx = IDX_LOC_OBJ_END_TIME;
	myData->testCond[ch].local_object[step][idx] = 100; //1sec
	make2_loc_chCode_cond(ch, step, C_CD_END_TIME, 1,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_LOC_OBJ_END_TIME_BRANCH;
	myData->testCond[ch].local_object[step][idx] = ACTIVE_DIV_NEXT;

	idx = IDX_LOC_OBJ_SAVE_DELTA_T;
	myData->testCond[ch].local_object[step][idx] = 100; //1.0sec
	make2_loc_chCode_cond(ch, step, C_CD_SAVE_DELTA_T, 1,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	//step_loop
	step++;

	idx = IDX_LOC_OBJ_CLIENT_STEP_NO;
	myData->testCond[ch].local_object[step][idx] = step+1;

	idx = IDX_LOC_OBJ_STEP_NO;
	myData->testCond[ch].local_object[step][idx] = step;

	idx = IDX_LOC_OBJ_TYPE;
	myData->testCond[ch].local_object[step][idx] = STEP_LOOP;

	idx = IDX_LOC_OBJ_ATTRIBUTE;
	myData->testCond[ch].local_object[step][idx] = ATTR_CHECK4_5;

	idx = IDX_LOC_OBJ_END_CYCLE_COUNT;
	myData->testCond[ch].local_object[step][idx] = 1;

	idx = IDX_LOC_OBJ_END_CYCLE_COUNT_BRANCH;
	myData->testCond[ch].local_object[step][idx] = ACTIVE_DIV_NEXT;

	//step_end
	step++;

	idx = IDX_LOC_OBJ_CLIENT_STEP_NO;
	myData->testCond[ch].local_object[step][idx] = step+1;

	idx = IDX_LOC_OBJ_STEP_NO;
	myData->testCond[ch].local_object[step][idx] = step;

	idx = IDX_LOC_OBJ_TYPE;
	myData->testCond[ch].local_object[step][idx] = STEP_END;

	idx = IDX_LOC_OBJ_ATTRIBUTE;
	myData->testCond[ch].local_object[step][idx] = ATTR_CHECK4_END;
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
		toPs = COC1_TO_MODULE + myPs->config.groupNo;
		SendMsg.msg = MSG_COC_MODULE_SAVE_MSG_FLAG;
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
