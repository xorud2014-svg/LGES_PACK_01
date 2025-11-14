#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include "../../INC/datastore.h"
#include "common_utils.h"
#include "main.h"
#include "time.h" //kjhw_150331_ttt

volatile S_SYSTEM_DATA *myData;
int bd, file_write;
long min[10], max[10];

void Test_Print(void)
{
	int i, j;

	i = 0; if(i == 1) TimeSlot_Print();
	i = 0; if(i == 1) TimeSlot_Print2(); //shhw_240611
	i = 0; if(i == 1) DataStructureSize_Print();
	i = 0; if(i == 1) Debug_Print();
	i = 0; if(i == 1) Sequence_Charge_Print(0);
	i = 0; if(i == 1) SOC_Tracking_Print(0);
	i = 0; if(i == 1) Charging_Count_Print(2);
	i = 0; if(i == 1) Charging_RPT_SOC_Print(2);

	i = 0; if(i == 1) AppControl_Print();
	i = 0; if(i == 1) COA_Client_Print();
	i = 0; if(i == 1) Temp_Client_Print(0);
	i = 0; if(i == 1) Can_Client_Print(0);
	i = 0; if(i == 1) {
				for(j=0; j < myData->AppControl.config.totalGroup; j++)
					COB_Client_Print(j);
			}
	i = 0; if(i == 1) COC_Client_Print();

	i = 0; if(i == 1) DataSave_Print();

	i = 0; if(i == 1) {
				for(j=0; j < myData->AppControl.config.totalGroup; j++)
					Jig_Control_Print(j);
			}
	i = 0; if(i == 1) {
				for(j=0; j < myData->AppControl.config.totalGroup; j++)
					Job_Change_Print(j);
			}
	i = 0; if(i == 1) {
				for(j=0; j < myData->AppControl.config.totalGroup; j++)
					Pitch_Change_Print(j);
			}

	i = 0; if(i == 1) ModuleState_Print();
	i = 0; if(i == 1) GroupState_Print();
	i = 0; if(i == 1) BoardState_Print();
	i = 1; if(i == 1) ChannelState_Print();
	i = 0; if(i == 1) FCH_State_Print();
	i = 0; if(i == 1) {
				for(j=0; j < myData->AppControl.config.totalGroup; j++)
					COB_Channel_Print(j);
			}
	i = 0; if(i == 1) LineCheck_Print(0);

	i = 0; if(i == 1) IO_Print();
	i = 0; if(i == 1) TestCond_Print();
	i = 0; if(i == 1) CaliData_Print();
	i = 0; if(i == 1) SubSensV_Print();
	i = 0; if(i == 1) AuxV_Print();
	i = 0; if(i == 1) AuxTemp_Print();
	i = 0; if(i == 1) Message_Print();
	i = 0; if(i == 1) COM_Print();
	i = 0; if(i == 1) CAN_Print();

	i = 0; if(i == 1) Chamber_Print();
	i = 1; if(i == 1) Shin_Print();	//shh_test

	switch(file_write) {
		case 1:
			file_write = 0;
			data_10ms_write(0, 0);
			data_10ms_write(0, 1);
			data_10ms_write(1, 0);
			data_10ms_write(1, 1);
//			data_10ms_write(0, 2);
//			data_10ms_write(0, 3);
//			data_10ms_write(1, 2);
//			data_10ms_write(1, 3);
			break;
		case 2:
			file_write = 0;
			COA_Client_TestCond_write_V1009(0); //COA?
			break;
		case 3:
			file_write = 0;
			COA_Client_TestCond_write_V100B(0); //COA?
			break;
		case 4:
			file_write = 0;
			COA_Client_TestCond_write_V100C(0); //COA?
			break;
		case 5:
			file_write = 0;
			COA_Client_TestCond_write_V100D(0); //kjhw_150210
#if defined __COA_VER_100B__ || defined __COA_VER_100B2__ || defined __COA_VER_100C__ || defined __COA_VER_100D__
#else //COA_VER_100F~
			COA_Client_TestCond_write_TimeSch(0); //kjhw_150210
#endif
			break;
		case 6:
			file_write = 0;
			COA_Client_TestCond_write_V100D_pattern(0); //COA?
			break;
		case 7:
			if(myData->mData.signal[M_SIG_EXIT_PHASE] > 0) {
				write_IO_Print(0);
			}
		default:
			file_write = 0;
			break;
	}
}

//20181219 KHK------------------------------------------------------
int Read_SOC_Tracking_File_Test(int ch, int pattern_index)
{ 
	char cmd[256], *in_delimiter = ",\t\n\r", *token;
	char temp[256], buf[256];
	int tmp, i, j, k;
	long div;
	float diff_current, diff_soc, A;
	FILE *fp;

	div = 0;
	
	memset(cmd, 0, sizeof cmd);
	if(div == 0) { //first
		sprintf(cmd, "/root/START_INFO/CH%03d/SOC_tracking_data_%05d.csv",
			ch+1, pattern_index+1);
		// /root/START_INFO/CH00#/SOC_tracking_data_######.csv
	} else { 
		sprintf(cmd,
			"/root/START_INFO/CH%03d/UPDATE/SOC_tracking_data_%05d.csv",
			ch+1, pattern_index+1);
		// /root/START_INFO/CH00#/UPDATE/SOC_tracking_data_#####.csv
	}
	if((fp = fopen(cmd, "r")) == NULL) {
//		userlog(DEBUG_LOG, psName1, "%s file(load){Open Error}\n", cmd);
		return -1;
	}

	j = k = 0;
FIRST:
	i = 0;
	while(i == 0) {
		j++;
		if(j >= 100) {
			i = -1;
			break;
		}
		memset(temp, 0, sizeof temp);
		tmp = fscanf(fp, "%s", temp);

	//	userlog2(DEBUG_LOG, psName1, " %02x:%02x:%02x:%02x",
	//		(unsigned char)temp[0], (unsigned char)temp[1],
	//		(unsigned char)temp[2], (unsigned char)temp[3]);

		if(temp[0] == 'S' && temp[1] == 'T' && temp[2] == 'X'){
			i = 1;
			break;
		}
	}
	//userlog2(DEBUG_LOG, psName1, ":end\n");

	if(i <= 0) {
//		userlog(DEBUG_LOG, psName1, "%s file(load){STX Error}\n", cmd);
    	fclose(fp);
		return -2;
	}

	memset((char *)&myData->testCond[ch].SOC_tracking[2], 0,
		sizeof(S_TEST_COND_SOC_TRACKING_DATA));

	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	token = strtok(temp, in_delimiter);
	if(token == NULL) {
//		userlog(DEBUG_LOG, psName1, "%s file(load){Index Error}\n", cmd);
    	fclose(fp);
		return -3;
	}
	memset(buf, 0, sizeof buf);
	strcpy(buf, token);
	if((strncmp(buf, "SOC", 3) == 0)) {
		//Index //SOC/Temp
	} else {
		k++;
		if(k == 1) {
			goto FIRST;
		} else if(k == 2) {
//			userlog(DEBUG_LOG, psName1, "%s file(load){Index Error Check 2}\n", cmd);
    		fclose(fp);
			return -4;
		}
	}
	token = strtok(NULL, in_delimiter);

	for(i=0; i < (MAX_SOC_TRACKING_DATA - 5) + 1; i++) { //SOC //15
		if(i > 0){
			memset(temp, 0, sizeof temp);
			tmp = fscanf(fp, "%s", temp);
			token = strtok(temp, in_delimiter);
		}
		for(j=0; j < MAX_SOC_TRACKING_DATA + 1; j++) { // Temp //20
			if(i == 0 && j == 0){
			 	continue;
			}
			memset(buf, 0, sizeof buf);
			if(token == NULL){
			}else{
				strcpy(buf, token);
			}
			if((strncmp(buf, "EOF", 3) == 0) || (strncmp(buf, "eof", 3) == 0)) {
			}else{
				if(i == 0){ 
					if(j > 0){
						if(atof(buf) != 0 && token != NULL){
							myData->testCond[ch].SOC_tracking[0].temp[j-1] = atof(buf)*1000.0;
							myData->testCond[ch].SOC_tracking[0].temp_num++;
						}
					}
				}
				if(j == 0){
					if(i > 0){
						if(atof(buf) != 0 && token != NULL){
							myData->testCond[ch].SOC_tracking[0].SOC[i-1] = atof(buf);
							myData->testCond[ch].SOC_tracking[0].soc_num++;
						}
					}
				}
				if(i > 0 && j > 0){
					myData->testCond[ch].SOC_tracking[0].limit_current[i-1][j-1] = atof(buf)*1000000.0;
				}
				token = strtok(NULL, in_delimiter);
			}
		}
	}

	for(j=0; j < MAX_SOC_TRACKING_DATA; j++) { // Temp 
		for(i=0; i < (MAX_SOC_TRACKING_DATA - 5) - 1; i++) { //SOC 
			if((i <  myData->testCond[ch].SOC_tracking[0].soc_num-1) //SOC
					&&(j <  myData->testCond[ch].SOC_tracking[0].temp_num)){ //Temp
				diff_soc = myData->testCond[ch].SOC_tracking[0].SOC[i]
					   - myData->testCond[ch].SOC_tracking[0].SOC[i+1];
				if(diff_soc != 0){
					diff_current = myData->testCond[ch].SOC_tracking[0].limit_current[i][j]
						- myData->testCond[ch].SOC_tracking[0].limit_current[i+1][j];
					if(diff_current != 0){
						A = diff_current/ diff_soc;
						myData->testCond[ch].SOC_tracking[0].tracking_data_A[i][j]
							= A;
						myData->testCond[ch].SOC_tracking[0].tracking_data_B[i][j]
							= myData->testCond[ch].SOC_tracking[0].limit_current[i+1][j]
							- A * myData->testCond[ch].SOC_tracking[0].SOC[i+1];
					}else{
						myData->testCond[ch].SOC_tracking[0].tracking_data_A[i][j] = 0.0;
						myData->testCond[ch].SOC_tracking[0].tracking_data_B[i][j]
							= myData->testCond[ch].SOC_tracking[0].limit_current[i+1][j];
					}
				}else{
					myData->testCond[ch].SOC_tracking[0].tracking_data_A[i][j] = 0.0;
					myData->testCond[ch].SOC_tracking[0].tracking_data_B[i][j]
						= myData->testCond[ch].SOC_tracking[0].limit_current[i+1][j];
				}
			}
		}
	}

    fclose(fp);
	if(div == 0) { //first
//		userlog(DEBUG_LOG, psName1,
//			"SOC tracking file read completed ch:%d index:%d \n",
//			ch+1, pattern_index+1);
	} else { //1 update
//		userlog(DEBUG_LOG, psName1, "SOC tracking file read completed(update) ch:%d index:%d\n",
//			ch+1, pattern_index+1);
	}
	return 0;
}
//--------------------------------------------------------------------


//20181219 KHK------------------------------------------------------
int Read_User_Define_Mode_Charging_Count_File_Test(int ch)
{ 
	char cmd[256], *in_delimiter = ",\t\n\r", *token;
	char temp[256], buf[256];
	int tmp, i, j, k;
	FILE *fp;

	memset(cmd, 0, sizeof cmd);
	sprintf(cmd, "/root/START_INFO/CH%03d/ChargingCount_data.csv",ch+1);
	// /root/START_INFO/CH00#/ChargingCount_data.csv
	
	if((fp = fopen(cmd, "r")) == NULL) {
//		userlog(DEBUG_LOG, psName1, "%s file(load){Open Error}\n", cmd);
		return -1;
	}

	j = k = 0;
	i = 0;
	while(i == 0) {
		j++;
		if(j >= 100) {
			i = -1;
			break;
		}
		memset(temp, 0, sizeof temp);
		tmp = fscanf(fp, "%s", temp);

	//	userlog2(DEBUG_LOG, psName1, " %02x:%02x:%02x:%02x",
	//		(unsigned char)temp[0], (unsigned char)temp[1],
	//		(unsigned char)temp[2], (unsigned char)temp[3]);

//		if(strncmp(buf, "STX_CHGCOUNT", 12) == 0) {
		if(temp[0] == 'S' && temp[1] == 'T' && temp[2] == 'X'
			&&temp[3] == '_' && temp[7] == 'C'){
			i = 1;
			break;
		}
	}
	//userlog2(DEBUG_LOG, psName1, ":end\n");

	if(i <= 0) {
//		userlog(DEBUG_LOG, psName1, "%s file(load){STX Error}\n", cmd);
    	fclose(fp);
		return -2;
	}

	memset((char *)&myData->testCond[ch].user_define_mode.charging_count_set, 0,
		sizeof(S_TEST_COND_CHARGING_COUNT_SET_DATA));
	
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	token = strtok(temp, in_delimiter);
	if(token == NULL) {
//		userlog(DEBUG_LOG, psName1, "%s file(load){Index Error}\n", cmd);
    	fclose(fp);
		return -3;
	}

	for(i=0; i < 16; i++) {
		if(i > 0){
			memset(temp, 0, sizeof temp);
			tmp = fscanf(fp, "%s", temp);
			token = strtok(temp, in_delimiter);
		}
		//Charging Count
		if(token == NULL){
		}else{
			memset(buf, 0, sizeof buf);
			strcpy(buf, token);
		}
		if((strncmp(buf, "EOF", 3) == 0) || (strncmp(buf, "eof", 3) == 0)) {
		}else{
			if(token != NULL){
				myData->testCond[ch].user_define_mode.charging_count_set.counter[i] = atol(buf);
			}
		}
		token = strtok(NULL, in_delimiter);
		//Charging Mode
		if(token == NULL){
		}else{
			memset(buf, 0, sizeof buf);
			strcpy(buf, token);
		}
		if((strncmp(buf, "EOF", 3) == 0) || (strncmp(buf, "eof", 3) == 0)) {
		}else{
			if(token != NULL){
				myData->testCond[ch].user_define_mode.charging_count_set.mode[i] = atoi(buf);
			}
		}
		token = strtok(NULL, in_delimiter);
		//SOC Tracking Use Flag
		if(token == NULL){
		}else{
			memset(buf, 0, sizeof buf);
			strcpy(buf, token);
		}
		if((strncmp(buf, "EOF", 3) == 0) || (strncmp(buf, "eof", 3) == 0)) {
		}else{
			if(token != NULL){
				myData->testCond[ch].user_define_mode.charging_count_set.soc_tracking_use[i] = atoi(buf);
			}
		}
		token = strtok(NULL, in_delimiter);
		//Power
		if(token == NULL){
		}else{
			memset(buf, 0, sizeof buf);
			strcpy(buf, token);
		}
		if((strncmp(buf, "EOF", 3) == 0) || (strncmp(buf, "eof", 3) == 0)) {
		}else{
			if(token != NULL){
				myData->testCond[ch].user_define_mode.charging_count_set.refP[i] = atol(buf);
			}
		}
		token = strtok(NULL, in_delimiter);
	}

    fclose(fp);
//	userlog(DEBUG_LOG, psName1,
//		"ChargingCount_data file read completed ch:%d \n",ch+1);
	return 0;
}
//--------------------------------------------------------------------

//20181219 KHK------------------------------------------------------
int Read_User_Define_RPT_SOC_File_Test(int ch)
{ 
	char cmd[256], *in_delimiter = ",\t\n\r", *token;
	char temp[256], buf[256];
	int tmp, i, j, k;
	FILE *fp;

	memset(cmd, 0, sizeof cmd);
	sprintf(cmd, "/root/START_INFO/CH%03d/ChargingRPTSOC_data.csv",ch+1);
	// /root/START_INFO/CH00#/ChargingRPTSOC_data.csv
	
	if((fp = fopen(cmd, "r")) == NULL) {
//		userlog(DEBUG_LOG, psName1, "%s file(load){Open Error}\n", cmd);
		return -1;
	}

	j = k = 0;
	i = 0;
	while(i == 0) {
		j++;
		if(j >= 100) {
			i = -1;
			break;
		}
		memset(temp, 0, sizeof temp);
		tmp = fscanf(fp, "%s", temp);

	//	userlog2(DEBUG_LOG, psName1, " %02x:%02x:%02x:%02x",
	//		(unsigned char)temp[0], (unsigned char)temp[1],
	//		(unsigned char)temp[2], (unsigned char)temp[3]);

//		if(strncmp(buf, "STX_CHGCOUNT", 12) == 0) {
		if(temp[0] == 'S' && temp[1] == 'T' && temp[2] == 'X'
			&&temp[3] == '_' && temp[7] == 'R'){
			i = 1;
			break;
		}
	}
	//userlog2(DEBUG_LOG, psName1, ":end\n");

	if(i <= 0) {
//		userlog(DEBUG_LOG, psName1, "%s file(load){STX Error}\n", cmd);
    	fclose(fp);
		return -2;
	}

	memset((char *)&myData->testCond[ch].user_define_mode.charging_rpt_soc_set, 0,
		sizeof(S_TEST_COND_CHARGING_RPT_SOC_SET_DATA));
	
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	token = strtok(temp, in_delimiter);
	if(token == NULL) {
//		userlog(DEBUG_LOG, psName1, "%s file(load){Index Error}\n", cmd);
    	fclose(fp);
		return -3;
	}

	for(i=0; i < 16; i++) {
		if(i > 0){
			memset(temp, 0, sizeof temp);
			tmp = fscanf(fp, "%s", temp);
			token = strtok(temp, in_delimiter);
		}
		//Charging Count
		if(token == NULL){
		}else{
			memset(buf, 0, sizeof buf);
			strcpy(buf, token);
		}
		if((strncmp(buf, "EOF", 3) == 0) || (strncmp(buf, "eof", 3) == 0)) {
		}else{
			if(token != NULL){
				myData->testCond[ch].user_define_mode.charging_rpt_soc_set.counter[i] = atol(buf);
			}
		}
		token = strtok(NULL, in_delimiter);
		//Charging RPT SOC
		if(token == NULL){
		}else{
			memset(buf, 0, sizeof buf);
			strcpy(buf, token);
		}
		if((strncmp(buf, "EOF", 3) == 0) || (strncmp(buf, "eof", 3) == 0)) {
		}else{
			if(token != NULL){
				myData->testCond[ch].user_define_mode.charging_rpt_soc_set.endSOC[i] = atol(buf)*10;
			}
		}
		token = strtok(NULL, in_delimiter);
	}

    fclose(fp);
//	userlog(DEBUG_LOG, psName1,
//		"ChargingCount_data file read completed ch:%d \n",ch+1);
	return 0;
}
//--------------------------------------------------------------------



void TimeSlot_Print(void)
{
	int i, j, end_idx;
	long tmp1, tmp2;

	printf("TimeSlot\n");

	printf("coun period: %ld\n", myData->test_val_l[0]);
	printf("count : %ld\n", myData->test_val_l[1]);
//	printf("1000ms_pre : %ld\n", myData->test_val_l[4]);
//	printf("interval : %ld\n", myData->test_val_l[5]);
	tmp1 = tmp2 = 0;
	switch(myData->mData.config.scan_period) {
		case 25: //25ms
			end_idx = 5;	break; //1ms
		case 50: //50ms
			end_idx = 10;	break; //1ms
		case 100: //100ms
			end_idx = 20;	break; //1ms
		case 200: //200ms
			end_idx = 20;	break; //2ms
		default: //10ms
			end_idx = 2;	break; //1ms
	}

	for(i=0; i < end_idx; i++) {
		printf("Time %02d ", i*5);
		for(j=0; j < 5; j++) {
			printf("%07ld ", myData->mData.runningTime[0][i*5 + j]);
			tmp1 += myData->mData.runningTime[0][i*5 + j];
			tmp2 += myData->mData.runningTime[1][i*5 + j];
		}
		printf("\n");
	}

	printf("runTime average [function]:%07ld, [all]:%07ld\n",
		tmp1/(end_idx*5), tmp2/(end_idx*5));

	printf("runTime max:%07ld[%ld], min:%07ld[%ld]\n",
		myData->mData.runningTime[2][0], myData->mData.runningTime[2][1],
		myData->mData.runningTime[3][0], myData->mData.runningTime[3][1]);

	printf("runTime/1000 max:%07ld[%ld], min:%07ld[%ld]\n",
		myData->mData.runningTime[2][0] / 1000,
		myData->mData.runningTime[2][1] / 1000,
		myData->mData.runningTime[3][0] / 1000,
		myData->mData.runningTime[3][1] / 1000);

	printf("can %d %d %d %d, %d %d %d %d\n",
		myData->CAN.signal[0][CAN_SIG_ABORT_TRANSMISSION],
		myData->CAN.signal[1][CAN_SIG_ABORT_TRANSMISSION],
		myData->CAN.signal[2][CAN_SIG_ABORT_TRANSMISSION],
		myData->CAN.signal[3][CAN_SIG_ABORT_TRANSMISSION],
		myData->CAN.signal[4][CAN_SIG_ABORT_TRANSMISSION],
		myData->CAN.signal[5][CAN_SIG_ABORT_TRANSMISSION],
		myData->CAN.signal[6][CAN_SIG_ABORT_TRANSMISSION],
		myData->CAN.signal[7][CAN_SIG_ABORT_TRANSMISSION]);

	printf("can status %ld %ld %ld %ld, %ld %ld %ld %ld\n",
		myData->test_val_l[0],
		myData->test_val_l[1],
		myData->test_val_l[2],
		myData->test_val_l[3],
		myData->test_val_l[4],
		myData->test_val_l[5],
		myData->test_val_l[6],
		myData->test_val_l[7]);

	printf("max_time : %lld, main_slot : %lld\n",
		myData->test_val_ll[0], myData->test_val_ll[1]);
	//kjhw_140520s
	/*printf("runTime_main_slot_5msec val1_max:%07ld[%ld]\n",
		myData->mData.runningTime[0][21], myData->mData.runningTime[0][22]);
	printf("runTime_main_slot_5msec val2_max:%07ld[%ld]\n",
		myData->mData.runningTime[0][31], myData->mData.runningTime[0][32]);
	printf("runTime_main_slot_5msec val3_max:%07ld[%ld]\n",
		myData->mData.runningTime[0][41], myData->mData.runningTime[0][42]);
	printf("runTime_main_slot_5msec val4_max:%07ld[%ld]\n",
		myData->mData.runningTime[0][51], myData->mData.runningTime[0][52]);
	printf("runTime_main_slot_5msec val5_max:%07ld[%ld]\n",
		myData->mData.runningTime[0][61], myData->mData.runningTime[0][62]);
	printf("runTime_main_slot_5msec val6_max:%07ld[%ld]\n",
		myData->mData.runningTime[0][71], myData->mData.runningTime[0][72]);
	printf("runTime_main_slot_5msec val7_max:%07ld[%ld]\n",
		myData->mData.runningTime[0][81], myData->mData.runningTime[0][82]);
	//kjhw_140520e
*/
	printf("\n");
}
void TimeSlot_Print2(void)
{
	int i, j, end_idx;
	long tmp1, tmp2;
	long minTime, maxTime, slotTime, avgTime, allTime; //shhw_240611

	printf("TimeSlot\n");

	tmp1 = tmp2 = 0;
	switch(myData->mData.config.scan_period) {
		case 25: //25ms
			end_idx = 5;	break; //1ms
		case 50: //50ms
			end_idx = 10;	break; //1ms
		case 100: //100ms
			end_idx = 20;	break; //1ms
		case 200: //200ms
			end_idx = 20;	break; //2ms
		default: //10ms
			end_idx = 2;	break; //1ms
	}
	for(i=0; i < end_idx; i++) {
		printf("Time %02d ", i*5);
		for(j=0; j < 5; j++) {
			printf("%07ld ", myData->mData.runningTime[0][i*5 + j]);
			tmp1 += myData->mData.runningTime[0][i*5 + j];
			tmp2 += myData->mData.runningTime[1][i*5 + j];
		}
		printf("\n");
	}
	for(i=0; i < end_idx; i++) {
		printf("Time(%%) %02d ", i*5);
		for(j=0; j < 5; j++) {
			slotTime = makePerTime(myData->mData.runningTime[0][i*5 + j]);
			printf("%ld%% ", slotTime);
			tmp1 += myData->mData.runningTime[0][i*5 + j];
			tmp2 += myData->mData.runningTime[1][i*5 + j];
		}
		printf("\n");
	}
/*
	printf("runTime average [function]:%07ld, [all]:%07ld\n",
		tmp1/(end_idx*5), tmp2/(end_idx*5));
*/	
/*
	avgTime = makePerTime(tmp1/(end_idx*5));
	allTime = makePerTime(tmp2/(end_idx*5));	
	printf("runTime average2 [function]:%ld%%, [all2]:%ld%%\n",
		avgTime, allTime);
*/
/*
	printf("runTime max:%07ld[%ld], min:%07ld[%ld]\n",
		myData->mData.runningTime[2][0], myData->mData.runningTime[2][1],
		myData->mData.runningTime[3][0], myData->mData.runningTime[3][1]);
*/
	//shhw_240611s
	maxTime = makePerTime(myData->mData.runningTime[2][0]); 
	minTime = makePerTime(myData->mData.runningTime[3][0]); 
/*	
	printf("runTime max_slot : %ld runTime max: %ld%% ,  min_slot: %ld, min :%ld%%\n"
		,myData->mData.runningTime[2][1], maxTime
		,myData->mData.runningTime[3][1], minTime);
*/
	//shhw_240611e
	

	printf("\n");
}


void DataStructureSize_Print(void)
{
	int i, j, size[100];

	memset(size, 0, sizeof size);
	i = j = 0;
	size[i] = sizeof(S_SYSTEM_DATA);
    printf("SysData size %d\n", size[i]);

	i++;
	size[i] = sizeof(S_MSG) * MAX_MSG_RING;
	printf("msg size %d = %d * %d\n", size[i], sizeof(S_MSG), MAX_MSG_RING);
	i++;
	size[i] = sizeof(S_SAVE_MSG) * MAX_SAVE_MSG_RING;
	printf("save_msg size %d = %d * %d\n", size[i], sizeof(S_SAVE_MSG),
		MAX_SAVE_MSG_RING);
#if defined __COB__ || defined __COC__
	i++;
	size[i] = sizeof(S_SAVE_MSG_1) * MAX_SAVE_MSG_RING;
	printf("save_msg_1 size %d = %d * %d\n", size[i], sizeof(S_SAVE_MSG_1),
		MAX_SAVE_MSG_RING);
	i++;
	size[i] = sizeof(S_RECORD_MSG_1) * MAX_RECORD_MSG_RING;
	printf("record_msg_1 size %d = %d * %d\n", size[i], sizeof(S_RECORD_MSG_1),
		MAX_RECORD_MSG_RING);
#endif
	i++;
	size[i] = sizeof(S_PULSE_MSG_1) * MAX_PULSE_MSG_RING;
	printf("pulse_msg_1 size %d = %d * %d\n", size[i], sizeof(S_PULSE_MSG_1),
		MAX_PULSE_MSG_RING);

#if defined __COB__
	i++;
	size[i] = sizeof(S_F_SAVE_MSG) * MAX_F_SAVE_MSG_RING;
	printf("f_save_msg size %d = %d * %d\n", size[i], sizeof(S_F_SAVE_MSG),
		MAX_F_SAVE_MSG_RING);
	i++;
	size[i] = sizeof(S_F_DCIR_SAVE_MSG) * MAX_GROUP_2;
	printf("f_save_dcir_msg size %d = %d * %d\n", size[i],
		sizeof(S_F_DCIR_SAVE_MSG), MAX_GROUP_2);
	i++;
	size[i] = sizeof(S_F_SAVE_MSG) * MAX_GROUP_2;
	printf("f_save_msg(real_data) size %d = %d * %d\n", size[i],
		sizeof(S_F_SAVE_MSG), MAX_GROUP_2);
#endif

	i++;
	size[i] = sizeof(S_LOGFILE) * MAX_LOG;
	printf("log size %d = %d * %d\n", size[i], sizeof(S_LOGFILE), MAX_LOG);

	i++;
	size[i] = sizeof(S_CALIBRATION) * MAX_CH_8;
	printf("cali size %d = %d * %d\n", size[i], sizeof(S_CALIBRATION),
		MAX_CH_8);

#if defined __COB__
	i++;
	size[i] = sizeof(S_F_CALIBRATION);
	printf("f_cali size %d\n", size[i]);
	i++;
	size[i] = sizeof(S_CALI_SET_DATA);
	printf("cali_set_data size %d\n", size[i]);
	i++;
	size[i] = sizeof(S_MSG_CH_DATA_FLAG);
	printf("cali_ch_flag size %d\n", size[i]);
	i++;
	size[i] = sizeof(S_MSG_CH_DATA_FLAG);
	printf("measure_ch_flag size %d\n", size[i]);

	i++;
	size[i] = sizeof(S_CH_TEMP) * MAX_CH_256;
	printf("TempArray1 size %d = %d * %d\n", size[i], sizeof(S_CH_TEMP),
		MAX_CH_256);
	i++;
	size[i] = sizeof(S_CH_TEMP) * MAX_CH_256;
	printf("TempArray2 size %d = %d * %d\n", size[i], sizeof(S_CH_TEMP),
		MAX_CH_256);
#endif

	i++;
	size[i] = sizeof(S_TEST_CONDITION) * MAX_TEST_COND_COUNT;
	printf("testCond size %d = %d * %d\n", size[i], sizeof(S_TEST_CONDITION),
		MAX_TEST_COND_COUNT);
	printf("	common_object %d = %d * %d\n",
		sizeof(long) * MAX_COM_TEST_OBJECT, sizeof(long), MAX_COM_TEST_OBJECT);
	printf("	local_object %d = %d * %d * %d\n",
		sizeof(long) * MAX_TEST_STEP * MAX_LOC_TEST_OBJECT, sizeof(long),
		MAX_TEST_STEP, MAX_LOC_TEST_OBJECT);
	printf("	grade %d = %d * %d\n",
		sizeof(S_CH_GRADE) * MAX_TEST_STEP, sizeof(S_CH_GRADE), MAX_TEST_STEP);
	printf("	common_chCode %d = %d * %d\n",
		sizeof(S_CH_CODE) * MAX_COM_CODE, sizeof(S_CH_CODE), MAX_COM_CODE);
	printf("	local_chCode %d = %d * %d * %d\n",
		sizeof(S_CH_CODE) * MAX_TEST_STEP * MAX_LOC_CODE,
		sizeof(S_CH_CODE), MAX_TEST_STEP, MAX_LOC_CODE);
	printf("	aux_chCode %d = %d * %d\n",
		sizeof(S_CH_CODE) * MAX_AUX_CODE, sizeof(S_CH_CODE), MAX_AUX_CODE);
	printf("	reserved %d\n", sizeof(S_TEST_COND_RESERVED));
	printf("	pattern %d = %d * %d\n",
		sizeof(S_TEST_COND_PATTERN_DATA) * MAX_PATTERN_DATA,
		sizeof(S_TEST_COND_PATTERN_DATA), MAX_PATTERN_DATA);
	printf("    SOC_tracking %d = %d * %d\n", sizeof(S_TEST_COND_SOC_TRACKING_DATA) * 2,
		sizeof(S_TEST_COND_SOC_TRACKING_DATA), 2);
	printf("    user_define_mode %d\n", sizeof(S_TEST_COND_USER_DEFINE_MODE_DATA));
	printf("	external_data %d = %d * %d\n",
		sizeof(S_TEST_COND_EXTERNAL_DATA) * 2,
		sizeof(S_TEST_COND_EXTERNAL_DATA), 2);
	printf("    usermap %d\n", sizeof(S_TEST_COND_USERMAP_DATA));
	printf("    fault_can_check_flag %d\n", sizeof(unsigned char));
	printf("    reserved1[3] %d\n", sizeof(unsigned char) * 3);

	i++;
	size[i] = sizeof(S_TEST_CONDITION);
	printf("testCond_update size %d\n", sizeof(S_TEST_CONDITION));
	
	i++;
	size[i] = sizeof(S_TIMESCH_CONTROL);
	printf("TimeSch size %d\n", sizeof(S_TIMESCH_CONTROL));
	
	i++;
	size[i] = sizeof(S_CH_NUMBER) * MAX_CH_256;
	printf("size CellArray1 %d = %d * %d\n", size[i], sizeof(S_CH_NUMBER),
		MAX_CH_256);
	i++;
	size[i] = sizeof(S_CH_NUMBER) * MAX_CH_256;
	printf("size CellArray2 %d = %d * %d\n", size[i], sizeof(S_CH_NUMBER),
		MAX_CH_256);

	i++;
	size[i] = sizeof(S_APP_CONTROL);
	printf("size AppControl %d\n", size[i]);

#if defined __COA__
	i++;
	size[i] = sizeof(S_COA_CLIENT) * MAX_GROUP_1;
	printf("size COA_Client %d = %d * %d\n", size[i], sizeof(S_COA_CLIENT),
		MAX_GROUP_1);
	printf("	signal %d\n", sizeof(unsigned char) * MAX_SIGNAL);
	printf("	config %d\n", sizeof(S_P1_CONFIG));
	printf("	misc %d\n", sizeof(S_P1_MISC));
	printf("	testConfig %d\n", sizeof(S_P1_TEST_CONFIG));
	printf("	rcv_packet %d\n", sizeof(S_P1_RCV_PACKET));
	printf("	rcv_command %d\n", sizeof(S_P1_RCV_COMMAND));
	printf("	reply %d\n", sizeof(S_P1_REPLY));
	printf("	testCond %d\n", sizeof(S_P1_TEST_CONDITION));
	printf("    testCond_time %d\n", sizeof(S_P1_TEST_CONDITION_TIME));
	printf("    common_safety_cond_update %d\n", sizeof(S_P1_RCV_CMD_COMMON_SAFETY_COND_UPDATE));
	printf("    step_cond_update %d\n", sizeof(S_P1_RCV_CMD_STEP_COND_UPDATE));
	printf("	pulse_data %d = %d * %d\n",
		sizeof(S_P1_PULSE_DATA) * MAX_CH_8, sizeof(S_P1_PULSE_DATA), MAX_CH_8);
	printf("    ch_data2 %d\n",sizeof(S_P1_SEND_CMD_CH_DATA2));
#ifdef __10MS__
	printf("    ch_data3 %d\n",sizeof(S_P1_SEND_CMD_CH_DATA3));
	printf("    ch_data4 %d\n",sizeof(S_P1_SEND_CMD_CH_DATA4));
#endif
	printf("    header %d\n",sizeof(S_P1_CMD_HEADER));
	printf("    ch_data %d\n",sizeof(S_P1_CH_DATA));
	printf("    aux_data %d\n",sizeof(S_P1_AUX_DATA));
	printf("    can_data %d\n",sizeof(S_P1_CAN_DATA));
	printf(" %d, %d, %d, %d, %d, %d, %d\n", sizeof(unsigned long), sizeof(unsigned short)
		,sizeof(unsigned char), sizeof(long), sizeof(short int), sizeof(float), sizeof(double));
#endif

#if defined __COB__
	i++;
	size[i] = sizeof(S_COB_CLIENT) * MAX_GROUP_2;
	printf("size COB_Client %d = %d * %d\n", size[i], sizeof(S_COB_CLIENT),
		MAX_GROUP_2);
	printf("	signal %d\n", sizeof(unsigned char) * MAX_SIGNAL);
	printf("	config %d\n", sizeof(S_P2_CONFIG));
	printf("	configData %d\n", sizeof(S_P2_CONFIG_DATA));
	printf("	misc %d\n", sizeof(S_P2_MISC));
	printf("	testConfig %d\n", sizeof(S_P2_TEST_CONFIG));
	printf("	rcv_packet %d\n", sizeof(S_P2_RCV_PACKET));
	printf("	rcv_command %d\n", sizeof(S_P2_RCV_COMMAND));
	printf("	bcr %d\n", sizeof(S_P2_CMD_BCR_INFO));
	printf("	reply %d\n", sizeof(S_P2_REPLY));
	printf("	sensor_limit %d\n", sizeof(S_P2_SENSOR_LIMIT));
	printf("	chamber_set %d\n", sizeof(S_P2_CHAMBER_SET));
	printf("	testCond %d\n", sizeof(S_P2_TEST_CONDITION));
#endif

#if defined __COC__
	i++;
	size[i] = sizeof(S_COC_CLIENT) * MAX_GROUP_1;
	printf("size COC_Client %d = %d * %d\n", size[i], sizeof(S_COC_CLIENT),
		MAX_GROUP_1);
#endif

#if defined __COD__
	i++;
	size[i] = sizeof(S_COC_CLIENT) * MAX_GROUP_1; //kjg_w
	printf("size COD_Client %d = %d * %d\n", size[i], sizeof(S_COC_CLIENT),
		MAX_GROUP_1);
#endif
	i++;
	size[i] = sizeof(S_TEMP_CLIENT) * 2;
	printf("size Temp_Client %d = %d * %d\n", size[i], sizeof(S_TEMP_CLIENT), 2);
	
	i++;
	size[i] = sizeof(S_DATA_SAVE);
	printf("size DataSave %d\n", size[i]);

	i++;
	size[i] = sizeof(S_MODULE_DATA);
	printf("size mData %d\n", size[i]);

	i++;
	size[i] = sizeof(S_GROUP_DATA) * MAX_GROUP_2;
	printf("size gData %d = %d * %d\n", size[i], sizeof(S_GROUP_DATA),
		MAX_GROUP_2);

#if defined __COB__
	i++;
	size[i] = sizeof(S_BD_DATA) * MAX_BD_32;
	printf("size bdData %d = %d * %d\n", size[i], sizeof(S_BD_DATA), MAX_BD_32);
#else
	i++;
	size[i] = sizeof(S_BD_DATA) * MAX_BD_16;
	printf("size bdData %d = %d * %d\n", size[i], sizeof(S_BD_DATA), MAX_BD_16);
#endif

	i++;
	size[i] = sizeof(S_CH_DATA) * MAX_CH_256;
	printf("size cData %d = %d * %d\n", size[i], sizeof(S_CH_DATA), MAX_CH_256);
	printf("     op = %d\n", sizeof(S_CH_OP_DATA));
	printf("     opSave = %d\n", sizeof(S_CH_OP_DATA));
	printf("     ccv = %d = %d * %d\n", sizeof(S_CH_CCV) * 2, sizeof(S_CH_CCV), 2);
	printf("     elementCycle = %d\n", sizeof(S_CH_CYCLE));
	printf("     multiCycle = %d = %d * %d\n ", sizeof(S_CH_CYCLE) * MAX_MULTI_CYCLE,
					sizeof(S_CH_CYCLE), MAX_MULTI_CYCLE);
	printf("     accCycle = %d = %d * %d\n ", sizeof(S_CH_CYCLE) * MAX_ACC_CYCLE,
					sizeof(S_CH_CYCLE), MAX_ACC_CYCLE);
	printf("     misc = %d\n", sizeof(S_CH_MISC));
	printf("     capa = %d\n", sizeof(S_CH_CAPACITANCE));
	printf("     signal = %d = %d * %d\n", sizeof(unsigned char) * MAX_SIGNAL, 
					sizeof(unsigned char), MAX_SIGNAL);
	printf("     cmd = %d = %d * %d\n", sizeof(unsigned char) * MAX_CMD_MODE, 
					sizeof(unsigned char), MAX_CMD_MODE);

#if defined __COC__
	i++;
	size[i] = sizeof(S_FCH_DATA);
	printf("size fch %d\n", size[i]);
#endif

#if defined __COA__
	i++;
	size[i] = sizeof(S_PWM3_DATA);
	printf("size pwm3_ch %d\n", size[i]);
#endif

	i++;
	size[i] = sizeof(S_LINE_CHECK) * MAX_GROUP_2 * MAX_CH_256;
	printf("size line_check %d = %d %d\n", size[i],
		MAX_GROUP_2, MAX_CH_256);

#if defined __COB__
	i++;
	size[i] = sizeof(int) * MAX_GROUP_2 * MAX_CH_256;
	printf("size COB_save_step %d = %d * %d * %d\n", size[i], sizeof(int),
		MAX_GROUP_2, MAX_CH_256);
	i++;
	size[i] = sizeof(S_CH_OP_DATA)
		* MAX_GROUP_2 *  MAX_CH_256 * MAX_P2_STEP;
	printf("size COB_opSave %d = %d * %d * %d * %d\n", size[i],
		sizeof(S_CH_OP_DATA), MAX_GROUP_2, MAX_CH_256, MAX_P2_STEP);
	i++;
	size[i] = sizeof(long) * MAX_GROUP_2 * MAX_CH_256;
	printf("size dcr_check %d = %d * %d * %d\n", size[i], sizeof(long),
		MAX_GROUP_2, MAX_CH_256);
#endif

	i++;
	size[i] = sizeof(S_ADDR_MAP);
	printf("size addr_map %d\n", size[i]);
	i++;
	size[i] = sizeof(S_DIO);
	printf("size dio %d\n", size[i]);
#if defined __COB__
	i++;
	size[i] = sizeof(S_JIG_DATA) * MAX_JIG_8;
	printf("size jigData %d = %d * %d\n", size[i], sizeof(S_JIG_DATA),
		MAX_JIG_8);
#endif

	i++;
	size[i] = sizeof(S_CH_ATTRIBUTE) * MAX_CH_256;
	printf("size chAttr %d = %d * %d\n", size[i], sizeof(S_CH_ATTRIBUTE),
		MAX_CH_256);

	i++;
	size[i] = sizeof(S_SUB_SENS_V);
	printf("size SubSensV %d\n", size[i]);

	i++;
	size[i] = sizeof(S_DAQ_DATA);
	printf("size daq %d\n", size[i]);
	
	i++;
	size[i] = sizeof(int) * MAX_CH_256 * MAX_AUX_TYPE;
	printf("size auxDataCount %d = %d * %d\n", size[i], sizeof(int),
		MAX_CH_256 * MAX_AUX_TYPE);
	i++;
	size[i] = sizeof(S_AUX_SET_DATA) * MAX_AUX_DATA;
	printf("size auxSetData %d = %d * %d\n", size[i], sizeof(S_AUX_SET_DATA),
		MAX_AUX_DATA);

	i++;
	size[i] = sizeof(S_CAN);
	printf("size CAN %d\n", size[i]);
	i++;
	size[i] = sizeof(int) * MAX_CH_8 * MAX_CAN_TYPE;
	printf("size canReceiveDataCount %d = %d * %d\n", size[i], sizeof(int),
		MAX_CH_8 * MAX_CAN_TYPE);
	i++;
	size[i] = sizeof(S_CAN_RECEIVE_SET_DATA);
	printf("size canReceiveSetData %d\n", size[i]);
	i++;
	size[i] = sizeof(int) * MAX_CH_8 * MAX_CAN_TYPE;
	printf("size canTransmitDataCount %d = %d * %d\n", size[i], sizeof(int),
		MAX_CH_8 * MAX_CAN_TYPE);
	i++;
	size[i] = sizeof(S_CAN_TRANSMIT_SET_DATA);
	printf("size canTransmitSetData %d\n", size[i]);
	i++;
	size[i] = sizeof(long) * MAX_CH_8 * MAX_CAN_DATA;
	printf("size canTransmitTime %d = %d * %d\n", size[i], sizeof(long),
		MAX_CH_8 * MAX_CAN_DATA);
	i++;
	size[i] = sizeof(S_CAN_TRANSMIT_CHANGE);
	printf("size canTransmitChange %d\n", size[i]);
	i++;
	size[i] = sizeof(U_CAN_VAL) * MAX_CH_8 * MAX_CAN_DATA;
	printf("size CanData %d = %d * %d * %d\n", size[i], sizeof(U_CAN_VAL),
		MAX_CH_8, MAX_CAN_DATA);

	i++;
	size[i] = sizeof(U_CAN_VAL) * MAX_CH_8 * MAX_CAN_DATA * 12;
	printf("size tmpCanData %d = %d * %d * %d * %d\n", size[i], sizeof(U_CAN_VAL),
		MAX_CH_8, MAX_CAN_DATA, 12);

	i++;
	size[i] = sizeof(U_CAN_VAL) * MAX_CH_8 * MAX_CAN_DATA * 12;
	printf("size tmpCanData_sort %d = %d * %d * %d * %d\n", size[i], sizeof(U_CAN_VAL),
		MAX_CH_8, MAX_CAN_DATA, 12);

	i++;
	size[i] = sizeof(unsigned char) * MAX_CH_8 * MAX_CAN_DATA * 12;
	printf("size tmpCanData_sort_flag %d = %d * %d * %d * %d\n", size[i], sizeof(unsigned char),
		MAX_CH_8, MAX_CAN_DATA, 12);

	i++;
	size[i] = sizeof(unsigned char) * MAX_CH_8;
	printf("size tmpCanData_sort_count %d = %d * %d\n", size[i], sizeof(unsigned char),
		MAX_CH_8);

	i++;
	size[i] = sizeof(unsigned char) * MAX_CH_8 * MAX_CAN_DATA;
	printf("size tmpCanData_index %d = %d * %d * %d\n", size[i], sizeof(unsigned char),
		MAX_CH_8, MAX_CAN_DATA);

	i++;
	size[i] = sizeof(unsigned char) * MAX_CH_8 * MAX_CAN_DATA;
	printf("size tmpCanData_count %d = %d * %d * %d\n", size[i], sizeof(unsigned char),
		MAX_CH_8, MAX_CAN_DATA);

	i++;
	size[i] = sizeof(S_COM);
	printf("size COM %d\n", size[i]);

	i++;
	size[i] = sizeof(S_MEASURE_CALI) * 2 * MAX_SUB_SENS_V_DATA;
	printf("size measure_cali %d = %d * %d\n", size[i], sizeof(S_MEASURE_CALI),
		2 * MAX_SUB_SENS_V_DATA);

	i++;
	size[i] = sizeof(S_TH_TABLE) * MAX_TH_TABLE;
	printf("size th_table %d = %d * %d\n", size[i], sizeof(S_TH_TABLE),
		MAX_TH_TABLE);

	i++;
	size[i] = sizeof(S_AUX_CH_NUM) * MAX_SUB_SENS_V_DATA;
	printf("size aux_ch_num %d = %d * %d\n", size[i], sizeof(S_AUX_CH_NUM),
		MAX_SUB_SENS_V_DATA);

	i++;
	size[i] = sizeof(unsigned char) * 8 * MAX_TEST_VALUE;
	printf("size test_val_uc %d = %d * %d\n", size[i], sizeof(unsigned char),
		8 * MAX_TEST_VALUE);
	i++;
	size[i] = sizeof(int) * 8 * MAX_TEST_VALUE;
	printf("size test_val_i %d = %d * %d\n", size[i], sizeof(int),
		8 * MAX_TEST_VALUE);
	i++;
	size[i] = sizeof(long) * MAX_TEST_VALUE;
	printf("size test_val_l %d = %d * %d\n", size[i], sizeof(long),
		MAX_TEST_VALUE);
	i++;
	size[i] = sizeof(long) * MAX_TEST_VALUE;
	printf("size test_val_l1 %d = %d * %d\n", size[i], sizeof(long),
		MAX_TEST_VALUE);
	i++;
	size[i] = sizeof(long long) * MAX_TEST_VALUE;
	printf("size test_val_ll %d = %d * %d\n", size[i], sizeof(long long),
		MAX_TEST_VALUE);
	i++;
	size[i] = sizeof(float) * MAX_TEST_VALUE;
	printf("size test_val_f %d = %d * %d\n", size[i], sizeof(float),
		MAX_TEST_VALUE);

	i++;
	size[i] = sizeof(int) * 2;
	printf("size data_10ms_count %d\n", size[i]);
	i++;
	size[i] = sizeof(long) * 2 * 4 * 2000;
	printf("size data_10ms %d = %d * %d\n", size[i], sizeof(long),
		2 * 4 * 2000);
	i++;
	size[i] = sizeof(long) * 2 * 4 * 2000;
	printf("size data_10ms_tmp %d = %d * %d\n", size[i], sizeof(long),
		2 * 4 * 2000);
	i++;
	size[i] = sizeof(long) * 6;
	printf("size last_value %d = %d * %d\n", size[i], sizeof(long), 6);

	i++;
	j = i;
	for(i=1; i < j; i++) {
		size[j] += size[i];
	}
	printf("total %d : %d\n", size[j], size[0]);

	printf("\n");
}

void Debug_Print(void)
{
	int i, j;

	printf("Debug \n");

	printf("test_val_f ");
	for(i=0; i < MAX_TEST_VALUE; i++) {
		printf("%f ", myData->test_val_f[i]);
	}
	printf("\n");

	printf("test_val_l ");
	for(i=0; i < MAX_TEST_VALUE; i++) {
		printf("%ld ", myData->test_val_l[i]);
		if(i % 8 == 7)
			printf("\n");
	}
	printf("\n");
/*	
	printf("delay_count1 %ld\n", myData->cData[0].misc.delay_count1);	//csk_190904

	printf("test_val_l1 ");
	for(i=0; i < MAX_TEST_VALUE; i++) {
		printf("%ld ", myData->test_val_l1[i]);
		if(i % 8 == 7)
			printf("\n");
	}
	printf("\n");

	printf("test_val_ll ");
	for(i=0; i < MAX_TEST_VALUE; i++) {
		//printf("%llx ", myData->test_val_ll[i]);
		printf("%lld ", myData->test_val_ll[i]);
	}
	printf("\n");
*/	

	for(j=0; j < 8; j++) {
		printf("test_val_uc[%d] ", j);
		for(i=0; i < MAX_TEST_VALUE; i++) {
			printf("%x ", myData->test_val_uc[j][i]);
			//printf("%d ", myData->test_val_uc[j][i]);
		}
		printf("\n");
	}
	

/*	//kjg_141114
	printf("sort_count:%d\n",
		myData->tmpCanData_sort_count[0]);
	printf("(1) %f, %f, %f\n",
		myData->tmpCanData_sort[0][0][0].f_val[0],
		myData->tmpCanData_sort[0][0][1].f_val[0],
		myData->tmpCanData_sort[0][0][2].f_val[0]);
	printf("(2) %f, %f, %f\n",
		myData->tmpCanData_sort[0][1][0].f_val[0],
		myData->tmpCanData_sort[0][1][1].f_val[0],
		myData->tmpCanData_sort[0][1][2].f_val[0]);
	printf("(3) %f, %f, %f\n",
		myData->tmpCanData_sort[0][2][0].f_val[0],
		myData->tmpCanData_sort[0][2][1].f_val[0],
		myData->tmpCanData_sort[0][2][2].f_val[0]);
	printf("(4) %f, %f, %f\n",
		myData->tmpCanData_sort[0][3][0].f_val[0],
		myData->tmpCanData_sort[0][3][1].f_val[0],
		myData->tmpCanData_sort[0][3][2].f_val[0]);
	printf("(5) %f, %f, %f\n",
		myData->tmpCanData_sort[0][4][0].f_val[0],
		myData->tmpCanData_sort[0][4][1].f_val[0],
		myData->tmpCanData_sort[0][4][2].f_val[0]);
	printf("(6) %f, %f, %f\n",
		myData->tmpCanData_sort[0][5][0].f_val[0],
		myData->tmpCanData_sort[0][5][1].f_val[0],
		myData->tmpCanData_sort[0][5][2].f_val[0]);
	printf("(7) %f, %f, %f\n",
		myData->tmpCanData_sort[0][6][0].f_val[0],
		myData->tmpCanData_sort[0][6][1].f_val[0],
		myData->tmpCanData_sort[0][6][2].f_val[0]);
*/
	printf("\n");
}
void Sequence_Charge_Print(int ch)
{
	int i, j, print;
	int temp_num, soc_ah_num, cell_v_num;
	ch=0;

	temp_num = myData->testCond[ch].SQ_Charge.col_num;
   	soc_ah_num = myData->testCond[ch].SQ_Charge.row_num;
	cell_v_num = myData->testCond[ch].SQ_Charge.row2_num;	
	

 	printf("sequence_count: %d\n", myData->cData[0].misc.sequence_count);
			
	printf("CutOff Voltage \n");

	print = 1;
	if(print){
		printf("COL Value[TEMP] \n");
		for(j=0; j < temp_num; j++) {
		//for(j=0; j < MAX_SQ_COL_DATA; j++) {
			printf("%ld ", myData->testCond[ch].SQ_Charge.COL[j]);
		}
		printf("\n");
		printf("Col Cnt: %d\n",myData->testCond[ch].SQ_Charge.col_num);
	}

	print = 1;
	if(print){
		printf("ROW Value[SOC or Ah] \n");
		for(j=0; j < soc_ah_num; j++) {
		//for(j=0; j < MAX_SQ_ROW_DATA; j++) {
			printf("%ld ", myData->testCond[ch].SQ_Charge.ROW[j]);
		}
		printf("\n");
		printf("ROW Cnt: %d\n",myData->testCond[ch].SQ_Charge.row_num);
		printf("ROW2 Cnt: %d\n",myData->testCond[ch].SQ_Charge.row2_num);
	}

	print = 1;
	if(print){
		printf("[CELL_V] \n");
		for(i=0; i < soc_ah_num; i++) {
			for(j=0; j < temp_num; j++) {
		//for(i=0; i < MAX_SQ_ROW_DATA; i++) {
			//for(j=0; j < MAX_SQ_COL_DATA; j++) {
				printf("ROW2[%d][%d] : %ld", i,j,myData->testCond[ch].SQ_Charge.div_voltage[i][j]);
				printf("\n");
			}
		}
	}
	printf("\n");
}


void SOC_Tracking_Print(int ch)
{
	unsigned char type;
	int i, j, print;

	printf("SOC Tracking \n");

	type = 0;	//charge
	//type = 1;	//discharge
	print = 1;
	if(print){
		printf("SOC Temp \n");
		for(j=0; j < MAX_SOC_TRACKING_DATA; j++) {
			printf("%ld ", myData->testCond[ch].SOC_tracking[type].temp[j]);
		}
		printf("\n");
		printf("Temp Cnt: %d\n",myData->testCond[ch].SOC_tracking[type].temp_num);
	}



	print = 1;
	if(print){
		printf("SOC Vlaue \n");
		for(j=0; j < MAX_SOC_TRACKING_DATA; j++) {
			printf("%ld ", myData->testCond[ch].SOC_tracking[type].SOC[j]);
		}
		printf("\n");
		printf("SOC Cnt: %d\n",myData->testCond[ch].SOC_tracking[type].soc_num);
	}

	print = 1;
	if(print){
		for(i=0; i < MAX_SOC_TRACKING_DATA; i++) {
			for(j=0; j < MAX_SOC_TRACKING_DATA; j++) {
				printf("%ld ", myData->testCond[ch].SOC_tracking[type].limit_current[i][j]);
			}
			printf("\n");
		}
	}

	print = 1;
	if(print){
//		for(i=0; i < MAX_SOC_TRACKING_DATA; i++){
		for(i=0; i < 6; i++){
			for(j=0; j < MAX_SOC_TRACKING_DATA; j++) {
				printf("A:%f ", myData->testCond[ch].SOC_tracking[type].tracking_data_A[i][j]);
				printf("B:%f ", myData->testCond[ch].SOC_tracking[type].tracking_data_B[i][j]);
			printf("\n");
			}
			printf("\n\n");
		}
	}
	printf("\n");
}

void Charging_Count_Print(int ch)
{
	int j, print;

	printf("Charging Count \n");

	print = 1;
	if(print){
		printf("Count \n");
		for(j=0; j < 16; j++) {
			printf("%ld ", myData->testCond[ch].user_define_mode.charging_count_set.counter[j]);
		}
		printf("\n");
	}
	print = 1;
	if(print){
		printf("mode \n");
		for(j=0; j < 16; j++) {
			printf("%d ", myData->testCond[ch].user_define_mode.charging_count_set.mode[j]);
		}
		printf("\n");
	}
	print = 1;
	if(print){
		printf("SOC Flag \n");
		for(j=0; j < 16; j++) {
			printf("%d ", myData->testCond[ch].user_define_mode.charging_count_set.soc_tracking_use[j]);
		}
		printf("\n");
	}
	print = 1;
	if(print){
		printf("Power \n");
		for(j=0; j < 16; j++) {
			printf("%ld ", myData->testCond[ch].user_define_mode.charging_count_set.refP[j]);
		}
		printf("\n");
	}
	print = 1;
	if(print){
		printf("maxCounter : ");
		printf("%ld ", myData->testCond[ch].user_define_mode.charging_count_set.maxCounter);
		printf("\n");
	}

	printf("\n");
}

void Charging_RPT_SOC_Print(int ch)
{
	int j, print;

	printf("Charging RPT SOC \n");

	print = 1;
	if(print){
		printf("Count \n");
		for(j=0; j < 16; j++) {
			printf("%ld ", myData->testCond[ch].user_define_mode.charging_rpt_soc_set.counter[j]);
		}
		printf("\n");
	}
	print = 1;
	if(print){
		printf("end SOC \n");
		for(j=0; j < 16; j++) {
			printf("%ld ", myData->testCond[ch].user_define_mode.charging_rpt_soc_set.endSOC[j]);
		}
		printf("\n");
	}
	print = 1;
	if(print){
		printf("maxCounter : ");
		printf("%ld ", myData->testCond[ch].user_define_mode.charging_rpt_soc_set.maxCounter);
		printf("\n");
	}


	printf("\n");
}


void AppControl_Print(void)
{
	int i, j;

	printf("AppControl %d\n", myData->AppControl.config.systemModel);
	printf("app signal %d, %d, %d(%d)\n",
		myData->AppControl.signal[APP_SIG_APP_CONTROL_PROCESS],
		myData->AppControl.signal[APP_SIG_COA1_CLIENT_PROCESS],
		myData->AppControl.signal[APP_SIG_ANALOG_METER_PROCESS],
		myData->AppControl.signal[APP_SIG_ANALOG_METER_PROCESS_CHECK]);
	printf("totalGroup %d\n", myData->AppControl.config.totalGroup);

	printf("process_etc");
	for(i=0; i < MAX_PROCESS_GROUP; i++) {
		for(j=0; j < MAX_GROUP_8; j++) {
			printf(" %d",
				myData->AppControl.misc.Load_Process[i][j]);
		}
		printf("\n");
	}

	printf("signal %d\n",
		myData->AppControl.signal[APP_SIG_CALI_UPDATE]);

	printf("\n");
}

void Can_Client_Print(int CanNo)
{
	int i, print, ch;	

	ch = 0;
	printf("Can_Client\n");

	print = 0;
	if(print){
		printf("networkPort %d, network_socket %d, fd_socket: %d, ipAddr %s\n",
			myData->Can_Client.config.networkPort,
			myData->Can_Client.config.network_socket,
			myData->Can_Client.config.fd_socket,
			myData->Can_Client.config.ipAddr);
		printf("net connected signal %d\n",
			myData->Can_Client.signal[CAN_SIG_NET_CONNECTED]);
	}

	print = 1;
	if(print){
		for(i =0; i < 50; i++){
			printf("%d ,", myData->Can_Client.misc.auxV_ch[0][i]);
				if(i % 5 == 4) printf("\n");
		}
		for(i =0; i < 50; i++){
			printf("%d ,", myData->Can_Client.misc.auxV_ch[1][i]);
				if(i % 5 == 4) printf("\n");
		}
	}

	print = 1;
	/*
	if(print) {
		printf("can_rx_data\n");
		for(i =0; i < 10; i++){
			printf("ID - %03x  DATA %02x : %02x : %02x : %02x : %02x : %02x : %02x : %02x \n", 
				myData->Can_Client.can_msg[ch].msg[i].can_id,
				myData->Can_Client.can_msg[ch].msg[i].data[0],
				myData->Can_Client.can_msg[ch].msg[i].data[1],
				myData->Can_Client.can_msg[ch].msg[i].data[2],
				myData->Can_Client.can_msg[ch].msg[i].data[3],
				myData->Can_Client.can_msg[ch].msg[i].data[4],
				myData->Can_Client.can_msg[ch].msg[i].data[5],
				myData->Can_Client.can_msg[ch].msg[i].data[6],
				myData->Can_Client.can_msg[ch].msg[i].data[7]);
		}
	}
	*/
	printf("can_comm_step_check = %ld, %ld, %ld, %ld\n", 
		myData->testCond[0].local_object[1][IDX_LOC_OBJ_CAN_COMM_STEP_CHECK_FLAG],
		myData->testCond[0].local_object[2][IDX_LOC_OBJ_CAN_COMM_STEP_CHECK_FLAG],
		myData->testCond[0].local_object[3][IDX_LOC_OBJ_CAN_COMM_STEP_CHECK_FLAG],
		myData->testCond[0].local_object[4][IDX_LOC_OBJ_CAN_COMM_STEP_CHECK_FLAG]);
	printf("test_val_l %d, %ld, %ld\n" , myData->cData[0].misc.external_comm_state, myData->test_val_l[1], myData->test_val_l[2]);
	
	print = 0;
	if(print) {
		printf("ipAddress %s, sendPort %d, receivePort %d, networkPort %d",
			myData->Can_Client.config.ipAddr,
			myData->Can_Client.config.sendPort,
			myData->Can_Client.config.receivePort,
			myData->Can_Client.config.networkPort);
	}

	printf("\n");
}

void Temp_Client_Print(int TempNo)
{ //debug_size_coa
	int i, print;		

	printf("Temp_Client\n");

	print = 1;
	if(print){
		printf("networkPort %d, network_socket %d, fd_socket: %d, heartbeatTimeout %d\n",
			myData->Temp_Client[TempNo].config.networkPort,
			myData->Temp_Client[TempNo].config.network_socket,
			myData->Temp_Client[TempNo].config.fd_socket,
			myData->Temp_Client[TempNo].config.heartbeatTimeout);
		printf("net connected signal %d\n",
			myData->Temp_Client[TempNo].signal[TEMP_SIG_NET_CONNECTED]);
		printf("TempNo %d\n",
			myData->Temp_Client[TempNo].misc.TempNo);
	}

	print = 1;
	if(print){
		for(i =0; i < 130; i++){
			printf("%ld ,", myData->COM.com_port[AUX_TEMP].value[i]);
			if(i % 10 == 9)
				printf("\n");
		}
	}
	printf("\n");
}



void COA_Client_Print(void)
{ //debug_size_coa
	int group;

	printf("COA_Client\n");

	group = 0;
	printf("data_save_10ms = %d\n",myData->COA_Client[0].config.data_save_10ms);
	printf("groupId %d, groupNo %d, time %ld\n",
		myData->COA_Client[group].config.groupId,
		myData->COA_Client[group].config.groupNo,
		myData->mData.misc.timer_1sec);
	printf("netTimeout %ld, ping_time %ld, %ld, net_time %ld, %ld, pause_time %ld, %ld\n",
		myData->COA_Client[group].config.netTimeout,
		myData->COA_Client[group].misc.ping_time,
		myData->COA_Client[group].misc.ping_time2,
		myData->COA_Client[group].misc.net_time,
		myData->COA_Client[group].misc.net_time2,
		myData->COA_Client[group].misc.pause_time,
		myData->COA_Client[group].misc.pause_time2);
	printf("send_monitor_data_interval %ld, sent_monitor_data_time %ld, %ld\n",
		myData->COA_Client[group].config.send_monitor_data_interval,
		myData->COA_Client[group].misc.sent_monitor_data_time,
		myData->COA_Client[group].misc.sent_monitor_data_time2);
	printf("send_save_data_interval %ld\n",
		myData->COA_Client[group].config.send_save_data_interval);
	printf("signal %d, %d\n",
		myData->COA_Client[group].signal[P1_SIG_NET_CONNECTED],
		myData->COA_Client[group].signal[P1_SIG_SEND_REAL_TIME_REQUEST]);

	printf("\n");
	printf("testCond.header\n");
	printf("totalStep:%d, totalPatternCount:%d, totalTimeSchCount:%d\n",
		myData->COA_Client[group].common_safety_cond_update
			.testCond_header.totalStep,
		myData->COA_Client[group].common_safety_cond_update
			.testCond_header.totalPatternCount,
		myData->COA_Client[group].common_safety_cond_update
			.testCond_header.totalTimeSchCount);

	printf("testCond.safety\n");
	printf("faultLowerV:%ld, faultUpperV:%ld, faultCompAuxV:%ld\n",
		myData->COA_Client[group].common_safety_cond_update
			.testCond_safety.faultLowerV,
		myData->COA_Client[group].common_safety_cond_update
			.testCond_safety.faultUpperV,
		myData->COA_Client[group].common_safety_cond_update
			.testCond_safety.faultCompAuxV);

	printf("\n");
	printf("testCond.step\n");
	printf("type:%d, stepNo:%d, mode:%d, testEnd:%d, subStep:%d\n",
		myData->COA_Client[group].step_cond_update
			.testCond_step.header.type,
		myData->COA_Client[group].step_cond_update
			.testCond_step.header.stepNo,
		myData->COA_Client[group].step_cond_update
			.testCond_step.header.mode,
		myData->COA_Client[group].step_cond_update
			.testCond_step.header.testEnd,
		myData->COA_Client[group].step_cond_update
			.testCond_step.header.subStep);
	printf("endTime_day:%ld, endTime:%ld\n",
		myData->COA_Client[group].step_cond_update
			.testCond_step.reference[0].endTime_day,
		myData->COA_Client[group].step_cond_update
			.testCond_step.reference[0].endTime);

	printf("\n");
}

void COB_Client_Print(int group)
{ //debug_size_cob
/*	printf("COB_Client(%d) state:%d, tmpState:%d, code:%d, tmpCode:%d, mode:%d\n", group+1,
		(int)myData->COB_Client[group].misc.state,
		(int)myData->COB_Client[group].misc.tmpState,
		(int)myData->COB_Client[group].misc.code,
		(int)myData->COB_Client[group].misc.tmpCode,
		(int)myData->COB_Client[group].config.workMode);

	printf("groupId %d, groupNo %d, time %ld\n",
		myData->COB_Client[group].config.groupId,
		myData->COB_Client[group].config.groupNo,
		myData->mData.misc.timer_1sec);
	printf("netTimer %ld %ld, netTimeout %ld\n",
		myData->COB_Client[group].misc.net_time,
		myData->COB_Client[group].misc.net_time2,
		myData->COB_Client[group].config.netTimeout);
	printf("send_monitor_data_interval %ld, sent_monitor_data_time %ld, %ld\n",
		myData->COB_Client[group].config.send_monitor_data_interval,
		myData->COB_Client[group].misc.sent_monitor_data_time,
		myData->COB_Client[group].config.send_save_data_interval);
	printf("signal %d\n",
		myData->COB_Client[group].signal[P2_SIG_NET_CONNECTED]);
	printf("chOffset %d\n",
		myData->COB_Client[group].misc.chOffset);
	printf("sent_sensor_data_time %ld, %ld, sent_chamber_data_time %ld, %ld\n",
		myData->COB_Client[group].misc.sent_sensor_data_time,
		myData->COB_Client[group].misc.sent_sensor_data_time2,
		myData->COB_Client[group].misc.sent_chamber_data_time,
		myData->COB_Client[group].misc.sent_chamber_data_time2);

	printf("\n");*/
}

void COC_Client_Print(void)
{ //debug_size_coc
#if defined __COC__ //kjg_171223
	int group;

	printf("COC_Client\n");

	group = 0;
	printf("groupId %d, groupNo %d, time %ld\n",
		myData->COC_Client[group].config.groupId,
		myData->COC_Client[group].config.groupNo,
		myData->mData.misc.timer_1sec);
	printf("netTimer %ld %ld, netTimeout %ld\n",
		myData->COC_Client[group].misc.net_time,
		myData->COC_Client[group].misc.net_time2,
		myData->COC_Client[group].config.netTimeout);
	printf("send_monitor_data_interval %ld, sent_monitor_data_time %ld, %ld\n",
		myData->COC_Client[group].config.send_monitor_data_interval,
		myData->COC_Client[group].misc.sent_monitor_data_time,
		myData->COC_Client[group].misc.sent_monitor_data_time2);
	printf("send_save_data_interval %ld\n",
		myData->COC_Client[group].config.send_save_data_interval);
	printf("signal %d, %d\n",
		myData->COC_Client[group].signal[P3_SIG_NET_CONNECTED],
		myData->COC_Client[group].signal[P3_SIG_SEND_REAL_TIME_REQUEST]);

	printf("\n");
#endif
}

void DataSave_Print(void)
{
	printf("DataSave\n");
	printf("\n");
}

void Jig_Control_Print(int jig)
{
#if defined __COB__
	printf("Jig_Control(%d) state:%d, code:%d, phase:%d, emgPhase:%d\n", jig+1,
		(int)myData->jData[jig].jigState,
		(int)myData->jData[jig].code,
		(int)myData->jData[jig].microState,
		(int)myData->jData[jig].emgPhase);
	printf("timer_1sec %ld, emgCheckDelay %ld\n",
		myData->mData.misc.timer_1sec, myData->jData[jig].emgCheckDelay);
/*
	printf("jigUseFlag:%d, totalTrayNo:%d\n",
		myData->jData[jig].config.jigUseFlag,
		myData->jData[jig].config.totalTrayNo);
*/
	printf("tray_state:%d, tmp_tray_state:%d, stopper_state:%d, door_state:%d\n",
		(int)myData->jData[jig].trayState[0],
		(int)myData->jData[jig].tmpTrayState[0],
		(int)myData->jData[jig].stopperState,
		(int)myData->jData[jig].doorState[0]);

	printf("jigNotMove %d\n", myData->jData[jig].jigNotMove);
/*
	printf("tmpTrayState:%d %ld %ld\n",
		(int)myData->jData[jig].tmpTrayState[0],
		myData->mData.misc.timer_1sec,
		myData->jData[jig].traySensingDelay);

	printf("tray_state_ext %d, tray_state_dir %d\n\n",
		Read_InPoint(1, 1, I_JIG_TRAY_STATE),
		Read_InPoint(1, 2, I_JIG_TRAY_STATE));

	printf("latch_left_open %d, latch_right_open %d\n",
		Read_InPoint(1, 0, I_JIG_LATCH_CYLINDER_L_OFF),
		Read_InPoint(1, 0, I_JIG_LATCH_CYLINDER_R_OFF));

	printf("latch_left_close %d, latch_right_close %d\n",
		Read_InPoint(1, 0, I_JIG_LATCH_CYLINDER_L_ON),
		Read_InPoint(1, 0, I_JIG_LATCH_CYLINDER_R_ON));

	printf("main_off %d, main_on %d\n",
		Read_InPoint(1, 0, I_JIG_MAIN_CYLINDER_L_OFF),
		Read_InPoint(1, 0, I_JIG_MAIN_CYLINDER_L_ON));
*/
	printf("fire1 %d, fire2 %d, chamber_fault %d, chamber_power %d\n",
		Read_InPoint(1, 0, I_JIG_FIRE_SENS),
		Read_InPoint(1, 0, I_JIG_SMOKE_SENS),
		Read_InPoint(1, 0, I_CHAMBER_FAULT),
		Read_InPoint(0, 0, I_JIG_CHAMBER_POWER));

	printf("door %d\n", myData->jData[jig].signal[J_SIG_DOOR_OPEN]);

	printf("\n");
#endif
}

void Job_Change_Print(int jig)
{
#if defined __COB__
	printf("Job_Change_Control[%d], state:%d, phase:%d, end:%d\n",
		jig+1,
		(int)myData->jData[jig].signal[J_SIG_JOB_CHANGE],
		(int)myData->jData[jig].jobChangePhase,
		(int)myData->jData[jig].jobChangeEnd);
	printf("\n");
#endif
}

void Pitch_Change_Print(int jig)
{
#if defined __COB__
	printf("Pitch_Change_Control[%d], state:%d, phase:%d, end:%d\n",
		jig+1,
		(int)myData->jData[jig].signal[J_SIG_PITCH_CHANGE],
		(int)myData->jData[jig].pitchChangePhase,
		(int)myData->jData[jig].pitchChangeEnd);
	printf("\n");
#endif
}

void ModuleState_Print(void)
{
	char buf[24];
	int i;
	long realDate, realClock;

	realDate = realClock = 0;

	printf("Module state : ");
	memset(buf, 0, sizeof buf);
	switch(myData->mData.state) {
		case M_IDLE:	strcpy(buf, "IDLE"); 		break;
		case M_STANDBY:	strcpy(buf, "STANDBY");		break;
		case M_RUN: 	strcpy(buf, "RUN"); 		break;
		case M_PAUSE:	strcpy(buf, "PAUSE"); 		break;
		case M_CALI:	strcpy(buf, "CALI"); 		break;
		case M_FAIL:	strcpy(buf, "FAIL");		break;
		default: 		strcpy(buf, "UNKNOWN");		break;
	}
	printf("%s\n", buf);
	
	printf("phase : %d, code : %d\n", myData->mData.phase, myData->mData.code);
	printf("timer_1sec %ld, increment_period %ld\n",
		myData->mData.misc.timer_1sec,
		myData->mData.misc.increment_period);
	printf("chPerBd %d, installedBd %d, installedCh %d\n",
		myData->mData.config.chPerBd,
		myData->mData.config.installedBd,
		myData->mData.config.installedCh);
	printf("fan_run_count %ld, fan_stop_count %ld\n",
		myData->mData.misc.fan_run_count,
		myData->mData.misc.fan_stop_count);

	printf("maxV ");
	for(i=0; i < MAX_RANGE; i++) {
		printf("%ld ", myData->mData.config.maxV[i]);
	}
	printf("\n");

	printf("maxI ");
	for(i=0; i < MAX_RANGE; i++) {
		printf("%ld ", myData->mData.config.maxI[i]);
	}
	printf("\n");

	printf("signal exit_type %d, exit_phase %d, fan_relay %d, fan_active %d\n",
		myData->mData.signal[M_SIG_EXIT_TYPE],
		myData->mData.signal[M_SIG_EXIT_PHASE],
		myData->mData.signal[M_SIG_FAN_RELAY],
		myData->mData.signal[M_SIG_FAN_ACTIVE]);
/*
	printf("fan_delay_time %ld, timer_1sec %ld\n",
		myData->mData.misc.fan_delay_time, myData->mData.misc.timer_1sec);
	printf("fan_fail_detect_time %ld, time2 %ld\n",
		myData->mData.misc.fan_fail_detect_time,
		myData->mData.misc.fan_fail_detect_time2);

	printf("real_time ");
	for(i=0; i < 7; i++) {
		printf("%ld ", myData->mData.real_time[i]);
	}
	printf("\n");

	realDate
		= myData->mData.real_time[6] * 10000 //year
		+ myData->mData.real_time[5] * 100 //month
		+ myData->mData.real_time[4]; //day
	realClock
		= myData->mData.real_time[3] * 10000000 //hour
		+ myData->mData.real_time[2] * 100000 //min
		+ myData->mData.real_time[1] * 1000 //sec
		+ myData->mData.real_time[0]; //msec
	printf("real_clock %ld, %ld\n", realDate, realClock);
*/
/*	printf("ref_muxVal %d\n",
		myData->mData.misc.ref_muxVal);

	printf("signal %d %d %d %d %d\n",
		myData->mData.signal[M_SIG_CALI_GROUP],
		myData->mData.signal[M_SIG_CALI_BD_START],
		myData->mData.signal[M_SIG_CALI_VI_SELECT],
		myData->mData.signal[M_SIG_CALI_CD_SELECT],
		myData->mData.signal[M_SIG_CALI_STEP]);
*/
	printf("exit_value %d\n", myData->mData.misc.exit_value);
/*
	printf("signal %d %d %d\n",
		myData->mData.signal[M_SIG_RUNNING_GROUP],
		myData->mData.signal[M_SIG_CABLE_CHECK_PHASE],
		myData->mData.signal[M_SIG_CELL_CHECK_PHASE]);
*/
/*
	printf("cvtn_id %x, sw_version %x %x %x %x, hvil %d, dtc %x %x %x\n",
		myData->mData.bms_eol_data.cvtn_id,
		myData->mData.bms_eol_data.sw_version[0],
		myData->mData.bms_eol_data.sw_version[1],
		myData->mData.bms_eol_data.sw_version[2],
		myData->mData.bms_eol_data.sw_version[3],
		myData->mData.bms_eol_data.HvBattHvil_D_Fault,
		myData->mData.bms_eol_data.DTC[0],
		myData->mData.bms_eol_data.DTC[1],
		myData->mData.bms_eol_data.DTC[2]);

	printf("module_dt %f, thermistor %f, cell_dv %f\n",
		myData->mData.bms_eol_data.module_delta_temp,
		myData->mData.bms_eol_data.thermistor_sensor,
		myData->mData.bms_eol_data.cell_delta_v);

	printf("Aux %d %d %d %d, Main %d %d %d %d, Neg %d %d %d %d\n",
		myData->mData.bms_eol_data.HvBattAuxCntct_D_Actl[0],
		myData->mData.bms_eol_data.HvBattAuxCntct_D_Actl[1],
		myData->mData.bms_eol_data.HvBattAuxCntct_D_Actl[2],
		myData->mData.bms_eol_data.HvBattAuxCntct_D_Actl[3],
		myData->mData.bms_eol_data.HvBattCntct_D_Actl[0],
		myData->mData.bms_eol_data.HvBattCntct_D_Actl[1],
		myData->mData.bms_eol_data.HvBattCntct_D_Actl[2],
		myData->mData.bms_eol_data.HvBattCntct_D_Actl[3],
		myData->mData.bms_eol_data.HvBattNeg_D_Actl[0],
		myData->mData.bms_eol_data.HvBattNeg_D_Actl[1],
		myData->mData.bms_eol_data.HvBattNeg_D_Actl[2],
		myData->mData.bms_eol_data.HvBattNeg_D_Actl[3]);

	printf("Pack_V %f %f %f %f, HvBatt_V %f %f %f %f\n",
		myData->mData.bms_eol_data.Cycler_Pack_V[0],
		myData->mData.bms_eol_data.Cycler_Pack_V[1],
		myData->mData.bms_eol_data.Cycler_Pack_V[2],
		myData->mData.bms_eol_data.Cycler_Pack_V[3],
		myData->mData.bms_eol_data.HvBatt_U_Actl[0],
		myData->mData.bms_eol_data.HvBatt_U_Actl[1],
		myData->mData.bms_eol_data.HvBatt_U_Actl[2],
		myData->mData.bms_eol_data.HvBatt_U_Actl[3]);

	printf("480D_V %f %f %f %f, Link_V %f %f %f %f, Charg_V %f %f %f %f\n",
		myData->mData.bms_eol_data.DID_480D_Pack_V[0],
		myData->mData.bms_eol_data.DID_480D_Pack_V[1],
		myData->mData.bms_eol_data.DID_480D_Pack_V[2],
		myData->mData.bms_eol_data.DID_480D_Pack_V[3],
		myData->mData.bms_eol_data.Link_V[0],
		myData->mData.bms_eol_data.Link_V[1],
		myData->mData.bms_eol_data.Link_V[2],
		myData->mData.bms_eol_data.Link_V[3],
		myData->mData.bms_eol_data.Charge_V[0],
		myData->mData.bms_eol_data.Charge_V[1],
		myData->mData.bms_eol_data.Charge_V[2],
		myData->mData.bms_eol_data.Charge_V[3]);

	printf("Pack2_V %f %f %f %f, Cha_Fuse_V %f %f %f %f, Elac_Fuse_V %f %f %f %f\n",
		myData->mData.bms_eol_data.Pack2_V[0],
		myData->mData.bms_eol_data.Pack2_V[1],
		myData->mData.bms_eol_data.Pack2_V[2],
		myData->mData.bms_eol_data.Pack2_V[3],
		myData->mData.bms_eol_data.Charger_Fuse_V[0],
		myData->mData.bms_eol_data.Charger_Fuse_V[1],
		myData->mData.bms_eol_data.Charger_Fuse_V[2],
		myData->mData.bms_eol_data.Charger_Fuse_V[3],
		myData->mData.bms_eol_data.Elac_Fuse_V[0],
		myData->mData.bms_eol_data.Elac_Fuse_V[1],
		myData->mData.bms_eol_data.Elac_Fuse_V[2],
		myData->mData.bms_eol_data.Elac_Fuse_V[3]);

	printf("iso_res %f, iso_Va %f, iso_Vb %f\n",
		myData->mData.bms_eol_data.isolation_resistance,
		myData->mData.bms_eol_data.isolation_Va,
		myData->mData.bms_eol_data.isolation_Vb);

	i = 0;
	printf("dtc list\n");
	printf("%x %x %x %x, %x %x %x %x, %x %x %x %x, %x %x %x %x, %x %x %x %x\n",
		myData->mData.bms_eol_data.DTC_List[i][0],
		myData->mData.bms_eol_data.DTC_List[i][1],
		myData->mData.bms_eol_data.DTC_List[i][2],
		myData->mData.bms_eol_data.DTC_List[i][3],
		myData->mData.bms_eol_data.DTC_List[i+1][0],
		myData->mData.bms_eol_data.DTC_List[i+1][1],
		myData->mData.bms_eol_data.DTC_List[i+1][2],
		myData->mData.bms_eol_data.DTC_List[i+1][3],
		myData->mData.bms_eol_data.DTC_List[i+2][0],
		myData->mData.bms_eol_data.DTC_List[i+2][1],
		myData->mData.bms_eol_data.DTC_List[i+2][2],
		myData->mData.bms_eol_data.DTC_List[i+2][3],
		myData->mData.bms_eol_data.DTC_List[i+3][0],
		myData->mData.bms_eol_data.DTC_List[i+3][1],
		myData->mData.bms_eol_data.DTC_List[i+3][2],
		myData->mData.bms_eol_data.DTC_List[i+3][3],
		myData->mData.bms_eol_data.DTC_List[i+4][0],
		myData->mData.bms_eol_data.DTC_List[i+4][1],
		myData->mData.bms_eol_data.DTC_List[i+4][2],
		myData->mData.bms_eol_data.DTC_List[i+4][3]);

	printf("dtc_data ");
	for(i=0; i < 100; i++) {
		printf("%02x ", myData->mData.misc.dtc_data[i]);
	}
	printf("\n");

	printf("tx_phase_uds_data %d %d %d %d\n",
		myData->CAN.signal[0][CAN_SIG_TX_PHASE_UDS_DATA1],
		myData->CAN.signal[1][CAN_SIG_TX_PHASE_UDS_DATA1],
		myData->CAN.signal[2][CAN_SIG_TX_PHASE_UDS_DATA1],
		myData->CAN.signal[3][CAN_SIG_TX_PHASE_UDS_DATA1]);
*/
	printf("can_flash total_block:%d, point:%d %d %d %d, size:%d %d %d %d\n",
		myData->CAN.can_flash_data_total_block,
		myData->CAN.can_flash_data_block_point[0],
		myData->CAN.can_flash_data_block_point[1],
		myData->CAN.can_flash_data_block_point[2],
		myData->CAN.can_flash_data_block_point[3],
		myData->CAN.can_flash_data_block_size[0],
		myData->CAN.can_flash_data_block_size[1],
		myData->CAN.can_flash_data_block_size[2],
		myData->CAN.can_flash_data_block_size[3]);

	printf("block_count %d, block_point %d, uds_ff_count %d, uds_tx_size %d, uds_tx_block_size %d\n",
		myData->CAN.block_count, myData->CAN.block_point, myData->CAN.uds_ff_count, myData->CAN.uds_tx_size, myData->CAN.uds_tx_block_size);

	printf("can_flash_file b1 : %02x %02x %02x %02x, %02x %02x %02x %02x, %02x %02x %02x %02x\n",
		myData->CAN.can_flash_file[1065-4],
		myData->CAN.can_flash_file[1065-3],
		myData->CAN.can_flash_file[1065-2],
		myData->CAN.can_flash_file[1065-1],
		myData->CAN.can_flash_file[1065],
		myData->CAN.can_flash_file[1065+1],
		myData->CAN.can_flash_file[1065+2],
		myData->CAN.can_flash_file[1065+3],
		myData->CAN.can_flash_file[1065+4],
		myData->CAN.can_flash_file[1065+5],
		myData->CAN.can_flash_file[1065+6],
		myData->CAN.can_flash_file[1065+7]);

	printf("can_flash_file b2 : %02x %02x %02x %02x, %02x %02x %02x %02x, %02x %02x %02x %02x\n",
		myData->CAN.can_flash_file[1123-4],
		myData->CAN.can_flash_file[1123-3],
		myData->CAN.can_flash_file[1123-2],
		myData->CAN.can_flash_file[1123-1],
		myData->CAN.can_flash_file[1123],
		myData->CAN.can_flash_file[1123+1],
		myData->CAN.can_flash_file[1123+2],
		myData->CAN.can_flash_file[1123+3],
		myData->CAN.can_flash_file[1123+4],
		myData->CAN.can_flash_file[1123+5],
		myData->CAN.can_flash_file[1123+6],
		myData->CAN.can_flash_file[1123+7]);

	printf("can_flash_file b3 : %02x %02x %02x %02x, %02x %02x %02x %02x, %02x %02x %02x %02x\n",
		myData->CAN.can_flash_file[1141-4],
		myData->CAN.can_flash_file[1141-3],
		myData->CAN.can_flash_file[1141-2],
		myData->CAN.can_flash_file[1141-1],
		myData->CAN.can_flash_file[1141],
		myData->CAN.can_flash_file[1141+1],
		myData->CAN.can_flash_file[1141+2],
		myData->CAN.can_flash_file[1141+3],
		myData->CAN.can_flash_file[1141+4],
		myData->CAN.can_flash_file[1141+5],
		myData->CAN.can_flash_file[1141+6],
		myData->CAN.can_flash_file[1141+7]);

	printf("signal eol_procedure_index %d, eol_procedure_phase %d\n",
		myData->mData.signal[M_SIG_EOL_PROCEDURE_INDEX],
		myData->mData.signal[M_SIG_EOL_PROCEDURE_PHASE]);

	printf("signal eol %d, eol_flash %d, uds_data_index %d, uds_tx_size %d\n",
		myData->mData.signal[M_SIG_BMS_EOL_PHASE],
		myData->mData.signal[M_SIG_BMS_EOL_FLASH_PHASE],
		myData->CAN.signal[0][CAN_SIG_TX_PHASE_UDS_DATA_INDEX],
		myData->CAN.uds_tx_size);

	printf("coa %s, pack_id %s, %s, %s, %s\n",
		myData->COA_Client[0].misc.write_pack_id,
		myData->mData.bms_eol_data2.pack_id[0],
		myData->mData.bms_eol_data2.pack_id[1],
		myData->mData.bms_eol_data2.pack_id[2],
		myData->mData.bms_eol_data2.pack_id[3]);
/*
	printf("signal eol3 %d\n",
		myData->mData.signal[M_SIG_BMS_EOL3_PHASE]);
	printf("eol3 ubms_state %x, sw %x, hw %x, id %x\n",
		myData->mData.bms_eol_data3.UBMS_State,
		myData->mData.bms_eol_data3.SW_Version,
		myData->mData.bms_eol_data3.HW_Version,
		myData->mData.bms_eol_data3.UBMS_ID);
	printf("eol3 cali_v %d %d %d %d\n", myData->mData.bms_eol_data3.cali_v[0],
		myData->mData.bms_eol_data3.cali_v[1],
		myData->mData.bms_eol_data3.cali_v[2],
		myData->mData.bms_eol_data3.cali_v[3]);
	printf("eol3 cali_val %d %d %d %d\n",
		myData->mData.bms_eol_data3.cali_val[0],
		myData->mData.bms_eol_data3.cali_val[1],
		myData->mData.bms_eol_data3.cali_val[2],
		myData->mData.bms_eol_data3.cali_val[3]);
	printf("eol3 cell_v %d %d %d %d\n", myData->mData.bms_eol_data3.cell_v[0],
		myData->mData.bms_eol_data3.cell_v[1],
		myData->mData.bms_eol_data3.cell_v[2],
		myData->mData.bms_eol_data3.cell_v[3]);
	printf("eol3 temp %d, m_v %d, obd %d\n",
		myData->mData.bms_eol_data3.temp,
		myData->mData.bms_eol_data3.module_v,
		myData->mData.bms_eol_data3.obd);
*/
	printf("\n");
}
	
void GroupState_Print(void)
{
	char buf[24];
	int i;

	printf("Group state ");
	for(i=0; i < myData->AppControl.config.totalGroup; i++) {
		memset(buf, 0, sizeof buf);
		switch(myData->gData[i].state) {
			case G_IDLE:	strcpy(buf, "IDLE"); 		break;
			case G_STANDBY:	strcpy(buf, "STANDBY");		break;
			case G_RUN: 	strcpy(buf, "RUN"); 		break;
			case G_PAUSE:	strcpy(buf, "PAUSE"); 		break;
			case G_CALI:	strcpy(buf, "CALI"); 		break;
			default: 		strcpy(buf, "UNKNOWN");		break;
		}
		printf("%s ", buf);
	}
	printf("\n");

	printf("phase ");
	for(i=0; i < myData->AppControl.config.totalGroup; i++) {
		printf("%d ", myData->gData[i].phase);
	}
	printf("\n");

	printf("code ");
	for(i=0; i < myData->AppControl.config.totalGroup; i++) {
		printf("%d ", myData->gData[i].code);
	}
	printf("\n");

	printf("attribute ");
	for(i=0; i < myData->AppControl.config.totalGroup; i++) {
		printf("%d ", myData->gData[i].attribute);
	}
	printf("\n");
/*
	printf("workMode ");
	for(i=0; i < myData->AppControl.config.totalGroup; i++) {
		printf("%d ", myData->gData[i].workMode);
	}
	printf("\n");

	printf("group_control_mode ");
	for(i=0; i < myData->AppControl.config.totalGroup; i++) {
		printf("%d ", myData->gData[i].group_control_mode);
	}
	printf("\n");
*/
	for(i=0; i < myData->AppControl.config.totalGroup; i++) {
		printf("scan_ch:%d %d, scan_step:%d %d, scan_idxStepNo_offset:%d %d, rest_area:%d %d, %d %d\n",
			myData->gData[i].misc.scan_ch[0],
			myData->gData[i].misc.scan_ch[1],
			myData->gData[i].misc.scan_step[0],
			myData->gData[i].misc.scan_step[1],
			myData->gData[i].misc.scan_idxStepNo_offset[0],
			myData->gData[i].misc.scan_idxStepNo_offset[1],
			myData->gData[i].misc.long_rest_area1[10],
			myData->gData[i].misc.long_rest_area2[10],
			myData->gData[i].misc.long_rest_area1[11],
			myData->gData[i].misc.long_rest_area2[11]);
	}

	printf("total_ng_ch:%d %d, tmp_scan_step:%d %d, %d %d\n",
		myData->gData[0].misc.total_ng_ch,
		myData->gData[1].misc.total_ng_ch,
		myData->gData[0].misc.tmp_scan_step[0],
		myData->gData[0].misc.tmp_scan_step[1],
		myData->gData[1].misc.tmp_scan_step[0],
		myData->gData[1].misc.tmp_scan_step[1]);
/*
	printf("chOffset ");
	for(i=0; i < myData->AppControl.config.totalGroup; i++) {
		printf("%d ", myData->gData[i].misc.chOffset);
	}
	printf("\n");

	printf("chInGroup ");
	for(i=0; i < myData->AppControl.config.totalGroup; i++) {
		printf("%d ", myData->mData.config.chInGroup[i]);
	}
	printf("\n");

	printf("runTime ");
	for(i=0; i < myData->AppControl.config.totalGroup; i++) {
		printf("%ld ", myData->gData[i].runTime);
	}
	printf("\n");

	printf("rest1 ");
	for(i=0; i < myData->mData.config.chInGroup[0]; i++) {
		printf("%ld ", myData->gData[0].misc.long_rest_time1[i]);
	}
	printf("\n");

	printf("rest2 ");
	for(i=0; i < myData->mData.config.chInGroup[0]; i++) {
		printf("%ld ", myData->gData[0].misc.long_rest_time2[i]);
	}
	printf("\n");
*/
	printf("signal exit_phase ");
	for(i=0; i < myData->AppControl.config.totalGroup; i++) {
		printf("%d ", myData->gData[i].signal[G_SIG_EXIT_PHASE]);
	}
	printf("\n");
/*
	printf("signal net_check ");
	for(i=0; i < myData->AppControl.config.totalGroup; i++) {
		printf("%d ", myData->gData[i].signal[G_SIG_NET_CHECK]);
	}
	printf("\n");
*/
	printf("\n");
}
	
void BoardState_Print(void)
{
	int i;
	
	printf("BoardState bd(%d)\n", bd+1);

	printf("sumV(+) ");
	for(i=0; i < myData->mData.config.ad_count; i++) {
		printf("%ld ", myData->bData[bd].misc.source[0].sumV[i]);
	}
	printf("\n");

	if(min[0] == 0) min[0] = myData->bData[bd].misc.source[0].sumV[0];
	if(max[0] == 0) max[0] = myData->bData[bd].misc.source[0].sumV[0];
	for(i=0; i < myData->mData.config.ad_count; i++) {
		if(myData->bData[bd].misc.source[0].sumV[i] < min[0])
			min[0] = myData->bData[bd].misc.source[0].sumV[i];
		if(myData->bData[bd].misc.source[0].sumV[i] > max[0])
			max[0] = myData->bData[bd].misc.source[0].sumV[i];
	}
	printf("min %ld, max %ld : %ld\n", min[0], max[0], min[0]-max[0]);

	printf("sumV(-) ");
	for(i=0; i < myData->mData.config.ad_count; i++) {
		printf("%ld ", myData->bData[bd].misc.source[1].sumV[i]);
	}
	printf("\n");

	if(min[1] == 0) min[1] = myData->bData[bd].misc.source[1].sumV[0];
	if(max[1] == 0) max[1] = myData->bData[bd].misc.source[1].sumV[0];
	for(i=0; i < myData->mData.config.ad_count; i++) {
		if(myData->bData[bd].misc.source[1].sumV[i] < min[1])
			min[1] = myData->bData[bd].misc.source[1].sumV[i];
		if(myData->bData[bd].misc.source[1].sumV[i] > max[1])
			max[1] = myData->bData[bd].misc.source[1].sumV[i];
	}
	printf("min %ld, max %ld : %ld\n", min[1], max[1], min[1]-max[1]);

	printf("sumV(0) ");
	for(i=0; i < myData->mData.config.ad_count; i++) {
		printf("%ld ", myData->bData[bd].misc.source[2].sumV[i]);
	}
	printf("\n");

	printf("sensSumV(+) ");
	for(i=0; i < myData->mData.config.filter_ad_count; i++) {
		printf("%ld ", myData->bData[bd].misc.source[0].sensSumV[i]);
	}
	printf("\n");

	if(min[4] == 0) min[4] = myData->bData[bd].misc.source[0].sensSumV[0];
	if(max[4] == 0) max[4] = myData->bData[bd].misc.source[0].sensSumV[0];
	for(i=0; i < myData->mData.config.filter_ad_count; i++) {
		if(myData->bData[bd].misc.source[0].sensSumV[i] < min[4])
			min[4] = myData->bData[bd].misc.source[0].sensSumV[i];
		if(myData->bData[bd].misc.source[0].sensSumV[i] > max[4])
			max[4] = myData->bData[bd].misc.source[0].sensSumV[i];
	}
	printf("min %ld, max %ld : %ld\n", min[4], max[4], min[4]-max[4]);

	printf("sensSumV(-) ");
	for(i=0; i < myData->mData.config.filter_ad_count; i++) {
		printf("%ld ", myData->bData[bd].misc.source[1].sensSumV[i]);
	}
	printf("\n");

	if(min[5] == 0) min[5] = myData->bData[bd].misc.source[1].sensSumV[0];
	if(max[5] == 0) max[5] = myData->bData[bd].misc.source[1].sensSumV[0];
	for(i=0; i < myData->mData.config.filter_ad_count; i++) {
		if(myData->bData[bd].misc.source[1].sensSumV[i] < min[5])
			min[5] = myData->bData[bd].misc.source[1].sensSumV[i];
		if(myData->bData[bd].misc.source[1].sensSumV[i] > max[5])
			max[5] = myData->bData[bd].misc.source[1].sensSumV[i];
	}
	printf("min %ld, max %ld : %ld\n", min[5], max[5], min[5]-max[5]);

	printf("ref_muxVal:%d, bd_sensCount:%d\n",
		myData->mData.misc.ref_muxVal, 
		myData->mData.misc.bd_sensCount);
	printf("sensSumV(0) ");
	for(i=0; i < myData->mData.config.filter_ad_count; i++) {
		printf("%ld ", myData->bData[bd].misc.source[2].sensSumV[i]);
	}
	printf("\n");

	if(min[8] == 0) min[8] = myData->bData[bd].misc.source[2].sensSumV[0];
	if(max[8] == 0) max[8] = myData->bData[bd].misc.source[2].sensSumV[0];
	for(i=0; i < myData->mData.config.filter_ad_count; i++) {
		if(myData->bData[bd].misc.source[2].sensSumV[i] < min[8])
			min[8] = myData->bData[bd].misc.source[2].sensSumV[i];
		if(myData->bData[bd].misc.source[2].sensSumV[i] > max[8])
			max[8] = myData->bData[bd].misc.source[2].sensSumV[i];
	}
	printf("min %ld, max %ld : %ld\n", min[8], max[8], min[8]-max[8]);

	printf("sourceV ");
	for(i=0; i < 4; i++) {
		printf("%ld ", myData->bData[bd].misc.source[i].sourceV);
	}
	printf("\n");

	printf("source2_sumV(+) ");
	for(i=0; i < 10; i++) {
		printf("%ld ", myData->bData[bd].misc.source2[0].sumV[i]);
	}
	printf("\n");

	printf("source2_totalV ");
	for(i=0; i < 4; i++) {
		printf("%f ", myData->bData[bd].misc.source2[i].totalV);
	}
	printf("\n");

	printf("source2V ");
	for(i=0; i < 4; i++) {
		printf("%ld ", myData->bData[bd].misc.source2[i].sourceV);
	}
	printf("\n");
	printf("\n");

	printf("sumI(+) ");
	for(i=0; i < myData->mData.config.ad_count; i++) {
		printf("%ld ", myData->bData[bd].misc.source[0].sumI[i]);
	}
	printf("\n");

	if(min[2] == 0) min[2] = myData->bData[bd].misc.source[0].sumI[0];
	if(max[2] == 0) max[2] = myData->bData[bd].misc.source[0].sumI[0];
	for(i=0; i < myData->mData.config.ad_count; i++) {
		if(myData->bData[bd].misc.source[0].sumI[i] < min[2])
			min[2] = myData->bData[bd].misc.source[0].sumI[i];
		if(myData->bData[bd].misc.source[0].sumI[i] > max[2])
			max[2] = myData->bData[bd].misc.source[0].sumI[i];
	}
	printf("min %ld, max %ld : %ld\n", min[2], max[2], min[2]-max[2]);

	printf("sumI(-) ");
	for(i=0; i < myData->mData.config.ad_count; i++) {
		printf("%ld ", myData->bData[bd].misc.source[1].sumI[i]);
	}
	printf("\n");

	if(min[3] == 0) min[3] = myData->bData[bd].misc.source[1].sumI[0];
	if(max[3] == 0) max[3] = myData->bData[bd].misc.source[1].sumI[0];
	for(i=0; i < myData->mData.config.ad_count; i++) {
		if(myData->bData[bd].misc.source[1].sumI[i] < min[3])
			min[3] = myData->bData[bd].misc.source[1].sumI[i];
		if(myData->bData[bd].misc.source[1].sumI[i] > max[3])
			max[3] = myData->bData[bd].misc.source[1].sumI[i];
	}
	printf("min %ld, max %ld : %ld\n", min[3], max[3], min[3]-max[3]);

	printf("sumI(0) ");
	for(i=0; i < myData->mData.config.ad_count; i++) {
		printf("%ld ", myData->bData[bd].misc.source[2].sumI[i]);
	}
	printf("\n");

	printf("sensSumI(+) ");
	for(i=0; i < myData->mData.config.filter_ad_count; i++) {
		printf("%ld ", myData->bData[bd].misc.source[0].sensSumI[i]);
	}
	printf("\n");

	if(min[6] == 0) min[6] = myData->bData[bd].misc.source[0].sensSumI[0];
	if(max[6] == 0) max[6] = myData->bData[bd].misc.source[0].sensSumI[0];
	for(i=0; i < myData->mData.config.filter_ad_count; i++) {
		if(myData->bData[bd].misc.source[0].sensSumI[i] < min[6])
			min[6] = myData->bData[bd].misc.source[0].sensSumI[i];
		if(myData->bData[bd].misc.source[0].sensSumI[i] > max[6])
			max[6] = myData->bData[bd].misc.source[0].sensSumI[i];
	}
	printf("min %ld, max %ld : %ld\n", min[6], max[6], min[6]-max[6]);

	printf("sensSumI(-) ");
	for(i=0; i < myData->mData.config.filter_ad_count; i++) {
		printf("%ld ", myData->bData[bd].misc.source[1].sensSumI[i]);
	}
	printf("\n");

	if(min[7] == 0) min[7] = myData->bData[bd].misc.source[1].sensSumI[0];
	if(max[7] == 0) max[7] = myData->bData[bd].misc.source[1].sensSumI[0];
	for(i=0; i < myData->mData.config.filter_ad_count; i++) {
		if(myData->bData[bd].misc.source[1].sensSumI[i] < min[7])
			min[7] = myData->bData[bd].misc.source[1].sensSumI[i];
		if(myData->bData[bd].misc.source[1].sensSumI[i] > max[7])
			max[7] = myData->bData[bd].misc.source[1].sensSumI[i];
	}
	printf("min %ld, max %ld : %ld\n", min[7], max[7], min[7]-max[7]);

	printf("sourceI ");
	for(i=0; i < 4; i++) {
		printf("%ld ", myData->bData[bd].misc.source[i].sourceI);
	}
	printf("\n");

	printf("source2I ");
	for(i=0; i < 4; i++) {
		printf("%ld ", myData->bData[bd].misc.source2[i].sourceI);
	}
	printf("\n");
	printf("\n");

	printf("calSourceV ");
	for(i=0; i < 4; i++) {
		printf("%ld ", myData->bData[bd].misc.source[i].calSourceV);
	}
	printf("\n");

	printf("calSourceV AD_A:%f, AD_B:%f, AD_A_N:%f, AD_B_N:%f\n",
		myData->bData[bd].misc.Vsource_AD_a,
		myData->bData[bd].misc.Vsource_AD_b,
		myData->bData[bd].misc.Vsource_AD_a_N,
		myData->bData[bd].misc.Vsource_AD_b_N);

	printf("calSourceI ");
	for(i=0; i < 4; i++) {
		printf("%ld ", myData->bData[bd].misc.source[i].calSourceI);
	}
	printf("\n");

	printf("calSourceI AD_A:%f, AD_B:%f, AD_A_N:%f, AD_B_N:%f\n",
		myData->bData[bd].misc.Isource_AD_a,
		myData->bData[bd].misc.Isource_AD_b,
		myData->bData[bd].misc.Isource_AD_a_N,
		myData->bData[bd].misc.Isource_AD_b_N);

	printf("source_sensCount %d, totalV %f, totaI %f\n",
		myData->mData.misc.source_sensCount,
		myData->bData[bd].misc.source2[0].totalV,
		myData->bData[bd].misc.source2[0].totalI);

	printf("\n");
}
	
void ChannelState_Print(void)
{
	char buf[24];
	int i, j, tmp, ch_start, ch_end;
	
	j = 0;
	printf("ChannelState bd(%d) %d %d\n", bd+1,
		myData->gData[0].misc.scan_ch[0]+1, myData->gData[0].misc.scan_ch[1]+1);
	
	printf("TH ---------- %d\n", myData->mData.config.installedTH);
	ch_start = myData->mData.config.chPerBd * bd;
	//ch_end = ch_start + myData->mData.config.chPerBd;
	ch_end = ch_start + 4;


	printf("state : ");
	for(i=ch_start; i < ch_end; i++) {
		memset(buf, 0, sizeof buf);
		tmp = (int)myData->cData[i].op.state;
		switch(tmp) {
			case C_IDLE:	strcpy(buf, "IDLE"); 		break;
			case C_STANDBY:	strcpy(buf, "STANDBY");		break;
			case C_RUN: 	strcpy(buf, "RUN"); 		break;
			case C_PAUSE:	strcpy(buf, "PAUSE"); 		break;
			case C_CALI:	strcpy(buf, "CALI"); 		break;
			case C_FAULT:	strcpy(buf, "FAULT");		break;
			default: 		strcpy(buf, "UNKNOWN");		break;
		}
		printf("%s ", buf);
	}
	printf("\n");
/*	
	printf("tmpState : ");
	for(i=ch_start; i < ch_end; i++) {
		memset(buf, 0, sizeof buf);
		tmp = (int)myData->cData[i].misc.tmpState;
		switch(tmp) {
			case C_IDLE:	strcpy(buf, "IDLE"); 		break;
			case C_STANDBY:	strcpy(buf, "STANDBY");		break;
			case C_RUN: 	strcpy(buf, "RUN"); 		break;
			case C_PAUSE:	strcpy(buf, "PAUSE"); 		break;
			case C_CALI:	strcpy(buf, "CALI"); 		break;
			case C_FAULT:	strcpy(buf, "FAULT");		break;
			default: 		strcpy(buf, "UNKNOWN");		break;
		}
		printf("%s ", buf);
	}
	printf("\n");
*/	
	printf("phase : ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%d ", myData->cData[i].op.phase);
	}
	printf("\n");

	printf("stepType : ");
	for(i=ch_start; i < ch_end; i++) {
		memset(buf, 0, sizeof buf);
		tmp = (int)myData->cData[i].op.stepType;
		switch(tmp) {
			case STEP_IDLE:			strcpy(buf, "IDLE"); 		break;
			case STEP_CHARGE:		strcpy(buf, "CHARGE"); 		break;
			case STEP_DISCHARGE:	strcpy(buf, "DISCHARGE"); 	break;
			case STEP_REST:			strcpy(buf, "REST"); 		break;
			case STEP_LONG_TIME_REST:	strcpy(buf, "LONG_TIME_REST"); 	break;
			case STEP_OCV:			strcpy(buf, "OCV"); 		break;
			case STEP_Z:			strcpy(buf, "Z"); 			break;
			case STEP_END:			strcpy(buf, "END");			break;
			case STEP_CYCLE:		strcpy(buf, "CYCLE");		break;
			case STEP_LOOP:			strcpy(buf, "LOOP");		break;
			case STEP_PATTERN:		strcpy(buf, "PATTERN");		break;
			case STEP_EXTERNAL_CAN:	strcpy(buf, "EXT_CAN");		break;
			case STEP_USERMAP:		strcpy(buf, "USERMAP");		break;
			case STEP_EXTERNAL_CAN_2:	strcpy(buf, "EXT_CAN_2");	break;
			default:				strcpy(buf, "UNKNOWN");		break;
		}
		printf("%s ", buf);
	}
	printf("\n");

	printf("stepMode : ");
	for(i=ch_start; i < ch_end; i++) {
		memset(buf, 0, sizeof buf);
		tmp = (int)myData->cData[i].op.stepMode;
		switch(tmp) {
			case MODE_IDLE:	strcpy(buf, "IDLE");	break;
			case MODE_CC_CV:	strcpy(buf, "CCCV");	break;
			case MODE_CC:	strcpy(buf, "CC");		break;
			case MODE_CV:	strcpy(buf, "CV");		break;
			case MODE_DC:	strcpy(buf, "DC");		break;
			case MODE_AC:	strcpy(buf, "AC");		break;
			case MODE_CP:	strcpy(buf, "CP");		break;
			case MODE_CCP:	strcpy(buf, "CCP");		break;
			case MODE_CR:	strcpy(buf, "CR");		break;
			default:		strcpy(buf, "UNKNOWN");	break;
		}
		printf("%s ", buf);
	}
	printf("\n");
	printf("user_Mode : ");
	for(i=ch_start; i < ch_end; i++) {
		memset(buf, 0, sizeof buf);
		tmp = (int)myData->test_val_l[0];
		switch(tmp) {
			case MODE_IDLE:	strcpy(buf, "IDLE");	break;
			case MODE_CC_CV:	strcpy(buf, "CCCV");	break;
			case MODE_CC:	strcpy(buf, "CC");		break;
			case MODE_CV:	strcpy(buf, "CV");		break;
			case MODE_DC:	strcpy(buf, "DC");		break;
			case MODE_AC:	strcpy(buf, "AC");		break;
			case MODE_CP:	strcpy(buf, "CP");		break;
			case MODE_CCP:	strcpy(buf, "CCP");		break;
			case MODE_CR:	strcpy(buf, "CR");		break;
			default:		strcpy(buf, "UNKNOWN");	break;
		}
		printf("%s ", buf);
	}
	printf("\n");

	printf("code : ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%d ", myData->cData[i].op.code);
	}
	printf("\n");
/*
	printf("sub_code : ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%d ", myData->cData[i].misc.sub_code);
	}
	printf("\n");

	printf("tmpCode : ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%d ", myData->cData[i].misc.tmpCode);
	}
	printf("\n");
*/
	printf("saveCode : ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%d ", myData->cData[i].misc.saveCode);
	}
	printf("\n");
/*	
	printf("attr : ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%d ", myData->cData[i].op.attribute);
	}
	printf("\n");
	*/
	printf("idxStepNo : ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%d ", myData->cData[i].op.idxStepNo);
	}
	printf("\n");
	printf("aux_ch_num : ");
	for(i=63; i < 70; i++) {
		printf("%d ", myData->aux_ch_num[i].type);
	}
	printf("\n");
	/*
	printf("comm_state : ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%x ", myData->cData[i].misc.external_comm_state);
	}
	printf("\n");

	printf("can_comm_check           : ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%d ", myData->ChAttribute[i].can_comm_check);
	}
	printf("\n");

	printf("can_comm_step_check_flag : ");
	for(i=ch_start; i < ch_end; i++) {
		j = myData->cData[i].op.idxStepNo;
		printf("%ld ", myData->testCond[i]
			.local_object[j][IDX_LOC_OBJ_CAN_COMM_STEP_CHECK_FLAG]);
	}
	printf("\n");

	printf("chamber_con              : ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%d ", myData->ChAttribute[i].chamber_control);
	}
	printf("\n");

	printf("chamber_step_check_flag  : ");
	for(i=ch_start; i < ch_end; i++) {
		j = myData->cData[i].op.idxStepNo;
		printf("%ld ", myData->testCond[i]
			.local_object[j][IDX_LOC_OBJ_CHAMBER_STEP_CHECK_FLAG]);
	}
	printf("\n");

	printf("chamber_standby          : ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%d ", myData->ChAttribute[i].chamber_standby);
	}
	printf("\n");

	printf("out_mux_use              : ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%d ", myData->ChAttribute[i].out_mux_use);
	}
	printf("\n");

	printf("ch_output_state : ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%x ", myData->cData[i].misc.ch_output_state);
	}
	printf("\n");

	printf("ch_input_state : ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%x ", myData->cData[i].misc.ch_input_state);
	}
	printf("\n");

	printf("active_div : ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%d ", myData->cData[i].misc.active_division);
	}
	printf("\n");

	printf("grade\n");
	for(i=ch_start; i < ch_end; i++) {
		printf("%d ", myData->cData[i].op.grade);
	}
	printf("\n");

	printf("temp\n");
	for(i=ch_start; i < ch_end; i++) {
		printf("%ld ", myData->cData[i].op.temp);
	}
	printf("\n");

	printf("sensCount\n");
	for(i=ch_start; i < ch_end; i++) {
		printf("%d ", myData->cData[i].misc.sensCount);
	}
	printf("\n");
*/
	printf("sumV ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%ld ", myData->cData[i].misc.sumV[0]);
	}
	printf("\n");
/*
	printf("sensSumV ");
	for(tmp=ch_start; tmp < ch_end; tmp++) {
		for(i=0; i < myData->mData.config.filter_ad_count; i++) {
			printf("%ld ", myData->cData[tmp].misc.sensSumV[i]);
		}
		printf(",");
	}
	printf("\n");
	
	if(min[8] == 0) min[8] = myData->cData[0].op.Vsens;
	if(max[8] == 0) max[8] = myData->cData[0].op.Vsens;
	if(min[8] > myData->cData[0].op.Vsens) {
		min[8] = myData->cData[0].op.Vsens;
	}
	if(max[8] < myData->cData[0].op.Vsens) {
		max[8] = myData->cData[0].op.Vsens;
	}

	printf("meanV\n");
	for(i=ch_start; i < ch_end; i++) {
		printf("%ld ", myData->cData[i].op.meanV);
	}
	printf("\n");

	printf("meanI\n");
	for(i=ch_start; i < ch_end; i++) {
		printf("%ld ", myData->cData[i].op.meanI);
	}
	printf("\n");
*/
	printf("voltage ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%ld ", myData->cData[i].op.Vsens);
	}
	printf(" min:%ld, max:%ld\n", min[8], max[8]);

	printf("seed_CycleAmpareHourSOC ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%ld ", (long)myData->cData[i].op.seed_CycleAmpareHourSOC);
	}
	printf("\n");

	printf("rptSOC ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%ld ", (long)myData->cData[i].op.rptSOC);
	}
	printf("\n");

	printf("charge_AmpareHour\n");
	for(i=ch_start; i < ch_end; i++) {
		printf("%ld ", myData->cData[i].op.charge_AmpareHour);
	}
	printf("\n");

	printf("SOC ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%ld ", (long)myData->cData[i].op.SOC);
	}
	printf("\n");
	printf("maxI ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%ld : %ld", 
			(long)myData->testCond[i].SOC_tracking[0].maxI,
			(long)myData->testCond[i].SOC_tracking[1].maxI);
	}
	printf("\n");

	printf("opSave.rptSOC ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%ld ", (long)myData->cData[i].opSave.rptSOC);
	}
	printf("\n");

	printf("opSave.SOC ");
	for(i=ch_start; i < ch_end; i++) {
		//printf("%ld ", (long)myData->cData[i].opSave.SOC);
		printf("%ld ", (long)myData->testCond[i].local_object[352][IDX_LOC_OBJ_CELL_BALANCING_FLAG]);
		printf("%ld ", (long)myData->testCond[i].local_object[353][IDX_LOC_OBJ_CELL_BALANCING_FLAG]);
		printf("%ld ", (long)myData->testCond[i].local_object[354][IDX_LOC_OBJ_CELL_BALANCING_FLAG]);
	}
	printf("\n");

	printf("charging_counter ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%ld ", (long)myData->cData[i].op.charging_counter);
	}
	printf("\n");

	printf("charging_counter_rpt_soc ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%ld ", (long)myData->cData[i].op.charging_counter_rpt_soc);
	}
	printf("\n");


	printf("tmpVsens ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%ld ", myData->cData[i].misc.tmpVsens);
	}
	printf("\n");

	printf("Vpower ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%ld ", myData->cData[i].misc.Vpower);
	}
	printf("\n");

	printf("Vbus ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%ld ", myData->cData[i].misc.Vbus);
	}
	printf("\n");

	printf("vs_value\n");
	for(i=ch_start; i < ch_end; i++) {
		printf("%ld ", myData->cData[i].misc.vs_value);
	}
	printf("\n");
	printf("test_val[0] : %ld, test_val[1] : %ld, test_val[2] : %ld, test_val[3] : %ld, test_val[4] : %ld, 
				test_val[5] : %ld\n ", 
			myData->test_val_l[0], myData->test_val_l[1], myData->test_val_l[2], myData->test_val_l[3],
			myData->test_val_l[4], myData->test_val_l[5]);
/*
	i=1;
	if(i == 0) {
		if(min[8] == 0) min[8] = myData->cData[0].misc.tmpVsens;
		if(max[8] == 0) max[8] = myData->cData[0].misc.tmpVsens;
		if(myData->cData[0].misc.tmpVsens < min[8])
			min[8] = myData->cData[0].misc.tmpVsens;
		if(myData->cData[0].misc.tmpVsens > max[8])
			max[8] = myData->cData[0].misc.tmpVsens;
	} else {
		if(min[8] == 0) min[8] = myData->cData[0].op.Vsens;
		if(max[8] == 0) max[8] = myData->cData[0].op.Vsens;
		if(myData->cData[0].op.Vsens < min[8])
			min[8] = myData->cData[0].op.Vsens;
		if(myData->cData[0].op.Vsens > max[8])
			max[8] = myData->cData[0].op.Vsens;
	}
	printf("min=%ld, max=%ld : %ld\n", min[8], max[8], min[8]-max[8]);
*/	
	printf("sumI ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%ld ", myData->cData[i].misc.sumI[0]);
	}
	printf("\n");
/*
	printf("sensSumI ");
	for(i=0; i < myData->mData.config.filter_ad_count; i++) {
		printf("%ld ", myData->cData[bd*4].misc.sensSumI[i]);
	}
	printf("\n");
	
	printf("total_Isens\n");
	for(i=ch_start; i < ch_end; i++) {
		printf("%ld ", myData->cData[i].misc.total_Isens);
	}
	printf("\n");
*/
	printf("current ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%ld ", myData->cData[i].op.Isens);
	}
	printf("\n");
/*
	printf("current(save) ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%ld ", myData->cData[i].opSave.Isens);
	}
	printf("\n");
	
	printf("misc.maxV ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%ld ", myData->cData[i].misc.maxV);
	}
	printf("\n");
	
	printf("misc.minV ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%ld ", myData->cData[i].misc.minV);
	}
	printf("\n");
	
	printf("misc.maxI ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%ld ", myData->cData[i].misc.maxI);
	}
	printf("\n");
	
	printf("misc.minI ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%ld ", myData->cData[i].misc.minI);
	}
	printf("\n");
*/
/*
	printf("tmpIsens\n");
	for(i=ch_start; i < ch_end; i++) {
		printf("%ld ", myData->cData[i].misc.tmpIsens);
	}
	printf("\n");

	i=1;
	if(i == 0) {
		if(min[9] == 0) min[9] = myData->cData[0].misc.tmpIsens;
		if(max[9] == 0) max[9] = myData->cData[0].misc.tmpIsens;
		if(myData->cData[0].misc.tmpIsens < min[9])
			min[9] = myData->cData[0].misc.tmpIsens;
		if(myData->cData[0].misc.tmpIsens > max[9])
			max[9] = myData->cData[0].misc.tmpIsens;
	} else {
		if(min[9] == 0) min[9] = myData->cData[0].op.Isens;
		if(max[9] == 0) max[9] = myData->cData[0].op.Isens;
		if(myData->cData[0].op.Isens < min[9])
			min[9] = myData->cData[0].op.Isens;
		if(myData->cData[0].op.Isens > max[9])
			max[9] = myData->cData[0].op.Isens;
	}
	printf("min=%ld, max=%ld : %ld\n", min[9], max[9], min[9]-max[9]);
	
	if(myData->cData[ch].Isens < min[8]) {
		min[8] = myData->cData[ch].op.Isens;
		for(i=0; i < myData->mData.config.filter_ad_count; i++) {
			min[i] = myData->cData[ch].misc.sensSumI[i];
		}
	}
	if(myData->cData[ch].Isens > max[8]) {
		max[8] = myData->cData[ch].op.Isens;
		for(i=0; i < myData->mData.config.filter_ad_count; i++) {
			max[i] = myData->cData[ch].misc.sensSumI[i];
		}
	}
	printf("min %ld : ", min[8]);
	for(i=0; i < myData->mData.config.filter_ad_count; i++) {
		printf("%ld ", min[i]);	
	}
	printf("\n");
	printf("max %ld : ", max[8]);
	for(i=0; i < myData->mData.config.filter_ad_count; i++) {
		printf("%ld ", max[i]);	
	}
	printf("\n");
	
	printf("v_power_bus\n");
	for(i=ch_start; i < ch_end; i++) {
		printf("%ld ", myData->fch.ch[i].v_power_bus);
	}
	printf("\n");

	printf("v_output_bus\n");
	for(i=ch_start; i < ch_end; i++) {
		printf("%ld ", myData->fch.ch[i].v_output_bus);
	}
	printf("\n");

	printf("d_r1\n");
	for(i=ch_start; i < ch_end; i++) {
		printf("%ld ", myData->cData[i].misc.d_r1);
	}
	printf("\n");

	printf("d_r2\n");
	for(i=ch_start; i < ch_end; i++) {
		printf("%ld ", myData->cData[i].misc.d_r2);
	}
	printf("\n");

	printf("patternTime\n");
	for(i=ch_start; i < ch_end; i++) {
		printf("%ld ", myData->cData[i].misc.pattern_time);
	}
	printf("\n");

	printf("runTime ");
	for(i=ch_start; i < ch_end; i++) {
		printf("(%ld):%ld ", myData->cData[i].op.runTime_day, myData->cData[i].op.runTime);
	}
	printf("\n");
	
	printf("totalRunTime\n");
	for(i=ch_start; i < ch_end; i++) {
		printf("(%ld):%ld ", myData->cData[i].op.totalRunTime_day, myData->cData[i].op.totalRunTime);
	}
	printf("\n");

	printf("sendDataCount\n");
	for(i=ch_start; i < ch_end; i++) {
		printf("%d ", myData->cData[i].misc.sendDataCount);
	}
	printf("\n");

	//kjhw_131206s
	printf("sum_CycleTime\n");
	for(i=ch_start; i < ch_end; i++) {
		printf("(%ld):%ld ",
		myData->cData[i].misc.sum_CycleTime_day,
		myData->cData[i].misc.sum_CycleTime);
	}
	printf("\n");
	printf("increment_period %ld\n", myData->mData.misc.increment_period);

	printf("elementCycle.cycle_idxStepNo\n");
	for(i=ch_start; i < ch_end; i++) {
		printf("%d ", myData->cData[i].elementCycle.cycle_idxStepNo);
	}
	printf("\n");
	printf("elementCycle.loop_idxStepNo\n");
	for(i=ch_start; i < ch_end; i++) {
		printf("%d ", myData->cData[i].elementCycle.loop_idxStepNo);
	}
	printf("\n");
	//kjhw_131206e


	printf("checkDelayTime : ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%ld ", myData->cData[i].misc.checkDelayTime);
	}
	printf("\n");

	printf("cvTime\n");
	for(i=ch_start; i < ch_end; i++) {
		printf("%ld ", myData->cData[i].misc.cvTime);
	}
	printf("\n");

	printf("cvTime\n");
	for(i=ch_start; i < ch_end; i++) {
		printf("(%ld):%ld ", myData->cData[i].op.cvTime_day, myData->cData[i].op.cvTime);
	}
	printf("\n");

	printf("maxI\n");
	for(i=ch_start; i < ch_end; i++) {
		printf("%ld ", myData->cData[i].misc.maxI);
	}
	printf("\n");
	
	printf("minI\n");
	for(i=ch_start; i < ch_end; i++) {
		printf("%ld ", myData->cData[i].misc.minI);
	}
	printf("\n");

	printf("watt ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%010ld ", myData->cData[i].op.watt);
	}
	printf("\n");

	printf("tmpWatt ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%010ld ", myData->cData[i].misc.tmpWatt);
	}
	printf("\n");
	
	printf("wattHour ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%010ld ", myData->cData[i].op.wattHour);
	}
	printf("\n");
	
	printf("charge_AmpareHour\n");
	for(i=ch_start; i < ch_end; i++) {
		printf("%ld ", myData->cData[i].op.charge_AmpareHour);
	}
	printf("\n");

	printf("discharge_AmpareHour\n");
	for(i=ch_start; i < ch_end; i++) {
		printf("%ld ", myData->cData[i].op.discharge_AmpareHour);
	}
	printf("\n");
	printf("sum_AmpareHourSOC\n");
	for(i=ch_start; i < ch_end; i++) {
		printf("%ld ", myData->cData[i].misc.sum_AmpareHourSOC);
	}
	printf("\n");
	printf("sum_CycleAmpareHourSOC\n");
	for(i=ch_start; i < ch_end; i++) {
		printf("%ld ", myData->cData[i].misc.sum_CycleAmpareHourSOC);
	}
	printf("\n");

	printf("sumCapacity\n");
	for(i=ch_start; i < ch_end; i++) {
		printf("%f ", myData->cData[i].misc.sumCapacity);
	}
	printf("\n");

	printf("seedCapacity\n");
	for(i=ch_start; i < ch_end; i++) {
		printf("%f ", myData->cData[i].misc.seedCapacity);
	}
	printf("\n");

	printf("capacitiance\n");
	for(i=ch_start; i < ch_end; i++) {
		printf("%ld ", myData->cData[i].op.capacitance);
	}
	printf("\n");

	printf("ccv\n");
	for(i=ch_start; i < ch_end; i++) {
		printf("%ld ", myData->mData.ccv[bd][i].value);
	}
	printf("\n");

	printf("d_voltage\n");
	for(i=ch_start; i < ch_end; i++) {
		printf("%ld ", myData->cData[i].misc.d_voltage);
	}
	printf("\n");

	printf("z\n");
	for(i=ch_start; i < ch_end; i++) {
		printf("%ld ", myData->cData[i].op.z);
	}
	printf("\n");

	printf("pid_ui1\n");
	for(i=ch_start; i < ch_end; i++) {
		printf("%f ", myData->cData[i].misc.pid_ui1[1]);
	}
	printf("\n");

	printf("pid_error1\n");
	for(i=ch_start; i < ch_end; i++) {
		printf("%f ", myData->cData[i].misc.pid_error1[1]);
	}
	printf("\n");

	printf("elementCycle.cycle_idxStepNo\n");
	for(i=ch_start; i < ch_end; i++) {
		printf("%d ", myData->cData[i].elementCycle.cycle_idxStepNo);
	}
	printf("\n");

	printf("elementCycle.cycle_count\n");
	for(i=ch_start; i < ch_end; i++) {
		printf("%ld ", myData->cData[i].elementCycle.cycle_count);
	}
	printf("\n");

	printf("totalCycle\n");
	for(i=ch_start; i < ch_end; i++) {
		printf("%ld ", myData->cData[i].misc.totalCycle);
	}
	printf("\n");

	printf("semiSwitch : ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%d ", myData->cData[i].misc.semiSwitchState);
	}
	printf("\n");

	printf("reservedCmd\n");
	for(i=ch_start; i < ch_end; i++) {
		printf("%d ", myData->cData[i].op.reservedCmd);
	}
	printf("\n");

	printf("endState ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%d ", myData->cData[i].misc.endState);
	}
	printf("\n");

	printf("rangeV ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%d ", myData->cData[i].op.rangeV);
	}
	printf("\n");

	printf("rangeI ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%d ", myData->cData[i].op.rangeI);
	}
	printf("\n");

	printf("accumlateGroupCycleID ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%d ", myData->cData[i].misc.accumulateGroupCycleID);
	}
	printf("\n");
	
	printf("accumulateGroupCycle ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%ld ", myData->cData[i].misc.accumulateGroupCycle);
	}
	printf("\n");
*/	
	printf("cmd_v ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%ld:%ld ", myData->cData[i].misc.cmd_v[0],
			myData->cData[i].misc.cmd_v[1]);
	}
	printf("\n");

	printf("cmd_i ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%ld:%ld, ", myData->cData[i].misc.cmd_i[0],
			myData->cData[i].misc.cmd_i[1]);
	}
	printf("\n");

	printf("cmd_4 = %ld, cmd_5 = %ld, cmd_6 = %ld, cmd_7 = %ld, cmd_8 = %ld, cmd_9 = %ld\n",
		myData->testCond[0].pattern[4].cmd_val[0],
		myData->testCond[0].pattern[5].cmd_val[0],
		myData->testCond[0].pattern[6].cmd_val[0],
		myData->testCond[0].pattern[7].cmd_val[0],
		myData->testCond[0].local_object[2][IDX_LOC_OBJ_REF_I2],
		myData->testCond[0].local_object[2][IDX_LOC_OBJ_REF_I]);

	printf("test_cond_I :  ");
		printf("%ld:, %ld: , %ld: , %ld: , %ld: ,%ld ", 
			myData->testCond[0].local_object[myData->cData[0].op.idxStepNo][IDX_LOC_OBJ_REF_I],
			myData->testCond[1].local_object[myData->cData[1].op.idxStepNo][IDX_LOC_OBJ_REF_I],
			myData->testCond[2].local_object[myData->cData[2].op.idxStepNo][IDX_LOC_OBJ_REF_I],
			myData->testCond[3].local_object[myData->cData[3].op.idxStepNo][IDX_LOC_OBJ_REF_I],
			myData->testCond[4].local_object[myData->cData[4].op.idxStepNo][IDX_LOC_OBJ_REF_I],
			myData->testCond[5].local_object[myData->cData[5].op.idxStepNo][IDX_LOC_OBJ_REF_I]);
	printf("\n");
				printf("setpointNum = %d\n",myData->cali[0]
					.tmpCond[0][0].point.setPointNum-1);
/*	printf("maxAuxT ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%ld ", myData->cData[i].misc.maxAuxT);
	}
	printf("\n");

	printf("[0] : %ld, [1] : %ld, [2] : %ld, [3] : %ld, [4] : %ld, [5] : %ld, [6] : %ld
			[7] : %ld, [8] : %ld, [9] : %ld, [10] : %ld\n ", 
			myData->test_val_l[0], myData->test_val_l[1], myData->test_val_l[2], 
			myData->test_val_l[3], myData->test_val_l[4], myData->test_val_l[5], 
			myData->test_val_l[6], myData->test_val_l[7], myData->test_val_l[8], 
			myData->test_val_l[9], myData->test_val_l[10]);

		printf("%f ", myData->test_val_f[0]);

	printf("test_val_i ");
	printf("1 : %d, 2 : %d, 3 : %d, 4 : %d, 5 : %d, 6 : %d, 7 : %d\n  ",
		myData->test_val_i[0][0], myData->test_val_i[0][1], myData->test_val_i[0][2],
		myData->test_val_i[0][3], myData->test_val_i[0][4], myData->test_val_i[0][5],
		myData->test_val_i[0][6]);
*/
/*
	printf("cmd_val ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%ld : %d, ", myData->testCond[0].pattern[1].cmd_val[3],
				myData->test_val_i[0][i]);
	}
	printf("\n");

	printf("t_val ");
	for(i=0; i < 5; i++) {
		printf("%ld, ", myData->testCond[0].pattern[i].t_val);
	}
	printf("\n");*/
/*
	printf("cmd_i_div ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%d ", myData->cData[i].misc.cmd_i_div);
	}
	printf("\n");
*/
	printf("cmd_p ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%ld:%ld ", myData->cData[i].misc.cmd_p[0],
			myData->cData[i].misc.cmd_p[1]);
	}
	printf("\n");
/*
	printf("bms_link_power ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%d ", myData->cData[i].misc.bms_link_flag);
	}
	printf("\n");

	printf("waveform_type ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%d ", myData->cData[i].misc.waveform_type);
	}
	printf("\n");

	printf("cv_select ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%d ", myData->cData[i].misc.cv_select);
	}
	printf("\n");

	printf("fbI ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%ld ", myData->cData[i].misc.fbI);
	}
	printf("\n");
*/
	//131011
/*	printf("external_data : ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%d ", myData->testCond[i].external_data[1].control);
	}
	printf("\n");

	printf("signal c_sig_cmd_continue : ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%d ", myData->cData[i].signal[C_SIG_CMD_CONTINUE]);
	}
	printf("\n");

	printf("signal c_sig_cmd_pause : ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%d ", myData->cData[i].signal[C_SIG_CMD_PAUSE]);
	}
	printf("\n");

	printf("signal : %d %d\n",
			myData->cData[i].signal[C_SIG_EXT_CAN_NM_STAT_1],
			myData->cData[i].signal[C_SIG_EXT_CAN_NM_FLAG_1]);

	printf("signal out_switch : ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%d ", myData->cData[i].signal[C_SIG_OUT_SWITCH]);
	}
	
	printf("\n");
	printf("signal semi_switch ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%d ", myData->cData[i].signal[C_SIG_SEMI_SWITCH]);
	}
	printf("\n");

	printf("signal selected_range_i ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%d ", myData->cData[i].signal[C_SIG_SELECTED_RANGE_I]);
	}
	printf("\n");

	printf("signal cali_normal : ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%d ", myData->cData[i].signal[C_SIG_CALI_NORMAL]);
	}
	printf("\n");

	printf("signal cali_point : ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%d ", myData->cData[i].signal[C_SIG_CALI_POINT]);
	}
	printf("\n");

	printf("signal cali_phase : ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%d ", myData->cData[i].signal[C_SIG_CALI_PHASE]);
	}
	printf("\n");

	printf("signal cali_update : ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%d ", myData->cData[i].signal[C_SIG_CALI_UPDATE]);
	}
	printf("\n");

	printf("signal cali_update_phase : ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%d ", myData->cData[i].signal[C_SIG_CALI_UPDATE_PHASE]);
	}
	printf("\n");

	printf("signal meter_reply : ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%d ", myData->cData[i].signal[C_SIG_METER_REPLY]);
	}
	printf("\n");

	printf("signal cmd_run\n");
	for(i=ch_start; i < ch_end; i++) {
		printf("%d ", myData->cData[i].signal[C_SIG_CMD_RUN]);
	}
	printf("\n");

	printf("signal cmd_next_step\n");
	for(i=ch_start; i < ch_end; i++) {
		printf("%d ", myData->cData[i].signal[C_SIG_CMD_NEXT_STEP]);
	}
	printf("\n");

	printf("signal cmd_pause\n");
	for(i=ch_start; i < ch_end; i++) {
		printf("%d ", myData->cData[i].signal[C_SIG_CMD_PAUSE]);
	}
	printf("\n");

	printf("signal cmd_goto_step\n");
	for(i=ch_start; i < ch_end; i++) {
		printf("%d ", myData->cData[i].signal[C_SIG_CMD_GOTO_STEP]);
	}
	printf("\n");

	printf("signal module_fault : ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%d ", myData->cData[i].signal[C_SIG_MODULE_FAULT]);
	}
	printf("\n");

	printf("pattern_count ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%d ", myData->cData[i].misc.pattern_count);
	}
	printf("\n");

	printf("pattern_t_val ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%ld ", myData->testCond[i].pattern[myData->cData[i].misc.pattern_count].t_val);
	}
	printf("\n");

	printf("signal ext_op_type\n");
	for(i=ch_start; i < ch_end; i++) {
		printf("%d ", myData->cData[i].signal[C_SIG_EXT_OP_TYPE]);
	}
	printf("\n");

	printf("signal cmd_run\n");
	for(i=ch_start; i < ch_end; i++) {
		printf("%d ", myData->cData[i].signal[C_SIG_CMD_RUN]);
	}
	printf("\n");

	printf("signal aux_dav ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%d ", myData->cData[i].signal[C_SIG_AUX_DAV]);
	}
	printf("\n");

	printf("signal aux_dai ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%d ", myData->cData[i].signal[C_SIG_AUX_DAI]);
	}
	printf("\n");

	printf("signal cable_check\n");
	for(i=ch_start; i < ch_end; i++) {
		printf("%d ", myData->cData[i].signal[C_SIG_CABLE_CHECK]);
	}
	printf("\n");

	printf("resultIndex \n");
	for(i=ch_start; i < ch_end; i++) {
		printf("%ld ", myData->cData[i].op.resultIndex);
	}
	printf("\n");

	printf("tmp_val(Vsens) \n");
	for(i=0; i < 6; i++) {
		printf("%08ld ", myData->cData[0].misc.tmp_val[0][i]);
	}
	printf("\n");
	printf("tmp_val(Vpower) \n");
	for(i=0; i < 6; i++) {
		printf("%08ld ", myData->cData[0].misc.tmp_val[2][i]);
	}
	printf("\n");
	printf("tmp_val(Vbus) \n");
	for(i=0; i < 6; i++) {
		printf("%08ld ", myData->cData[0].misc.tmp_val[3][i]);
	}
	printf("\n");

	printf("canTrans.common.value \n");
	for(i=0; i < 10; i++) {
		printf("%f ", myData->canTransmitSetData.commonData[0][0].value[i]);
	}
	printf("\n");

	printf("C_SIG_EXT_CAN_E12_STAT_ERROR: %d, E12_NM_STAT: %d\n",
		myData->cData[0].signal[C_SIG_EXT_CAN_E12_STAT_ERROR],
		myData->cData[0].signal[C_SIG_EXT_CAN_E12_NM_STAT]);
	printf("C_SIG_EXT_RECEIVE_PARSING_COUNT: %d\n",
		myData->cData[0].signal[C_SIG_EXT_RECEIVE_PARSING_COUNT]);
	printf("CAN_SIG_SET_CONTROL: %d\n",
		myData->CAN.signal[0][CAN_SIG_SET_CONTROL]);

	printf("-----kimjaeho test-----\n");
	printf("0. attr_count: %ld\n", myData->test_val_l[0]);
	printf("1. max_power1(1ch): %ld\n", myData->test_val_l[1]);
	printf("2. max_power(attr_count): %ld\n", myData->test_val_l[2]);
	printf("3. Vsens: %ld\n", myData->test_val_l[3]);
	if(myData->test_val_l[15] == 1) {
		printf("15. stepType: %s\n", "CHARGE");
	} else if(myData->test_val_l[15] == 2) {
		printf("15. stepType: %s\n", "DISCHARGE");
	} else if(myData->test_val_l[15] == 3) {
		printf("15. stepType: %s\n", "PATTERN");
	} else {
		printf("15. stepType: %s\n", "REST");
	}
	printf("4.5. MODE_CC_CV, CV, CC: d_val1: %ld, d_val2: %ld\n",
		myData->test_val_l[4], myData->test_val_l[5]);
	printf("6.7. MODE_CP: refP: %ld, refP1: %ld\n",
		myData->test_val_l[6], myData->test_val_l[7]);
	printf("14. RETURN: %ld\n", myData->test_val_l[14]);
	printf("\n");

	//kkkkkkkkkkkkkkkkkkkkkkkkkkkkkkk
	printf("C_SIG_CMD_CHAMBER_CONTINUE: %d\n",
		myData->cData[0].signal[C_SIG_CMD_CHAMBER_CONTINUE]);
*/

/*	for(i=0; i < MAX_P1_USERMAP_ROW_DATA; i++) {
		for(j=0; j < MAX_P1_USERMAP_COL_DATA; j++) {
			printf("%ld ", myData->testCond[0].usermap.table[i][j]);
		}
		printf("\n");
	}
	printf("\n");
*/
/*	printf("CAN_SIG_TX_PHASE_SEC: ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%d ", myData->CAN.signal[i][CAN_SIG_TX_PHASE_SEC]);
	}
	printf("\n");

	printf("can_timer_1sec_count: ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%d ", myData->CAN.can_timer_1sec_count[i]);
	}
	printf("\n");

	printf("can_timer_1sec: ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%ld ", myData->CAN.can_timer_1sec[i]);
	}
	printf("\n");

	printf("CAN_SIG_TX_PHASE_MIN: ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%d ", myData->CAN.signal[i][CAN_SIG_TX_PHASE_MIN]);
	}
	printf("\n");

	printf("can_timer_1min_count: ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%d ", myData->CAN.can_timer_1min_count[i]);
	}
	printf("\n");

	printf("can_timer_1min_count2: ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%d ", myData->CAN.can_timer_1min_count2[i]);
	}
	printf("\n");

	printf("can_timer_1min: ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%ld ", myData->CAN.can_timer_1min[i]);
	}
	printf("\n");

	printf("signal_out_mux_select ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%d ", myData->cData[i].signal[C_SIG_OUT_MUX_SELECT]);
	}
	printf("\n");
	printf("misc.out_mux          ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%d ", myData->cData[i].misc.out_mux);
	}
	printf("\n");
	printf("misc.out_mux_backup   ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%d ", myData->cData[i].misc.out_mux_backup);
	}
	printf("\n");
	printf("temp_continue : %d\n", 
		myData->cData[0].signal[C_SIG_TEMP_CONTINUE]);*/
	printf("\n");
	for(i=ch_start; i < ch_end; i++) {
		printf("ch:%d maxTemp:%ld, ChNo:%d, minTemp:%ld, ChNo:%d, diff:%ld\n",
			i, myData->cData[i].misc.maxAuxTemp,
			myData->cData[i].misc.maxAuxTemp_auxChNo,
			myData->cData[i].misc.minAuxTemp,
			myData->cData[i].misc.minAuxTemp_auxChNo,
			myData->cData[i].misc.diffAuxTemp);
		printf("ch:%d maxAuxV:%ld, ChNo:%d, minAuxV:%ld, ChNo:%d, diff:%ld\n",
			i, myData->cData[i].misc.maxAuxV,
			myData->cData[i].misc.maxAuxV_auxChNo,
			myData->cData[i].misc.minAuxV,
			myData->cData[i].misc.minAuxV_auxChNo,
			myData->cData[i].misc.diffAuxV);
		printf("ch:%d maxAuxTH:%ld, ChNo:%d, minAuxTH:%ld, ChNo:%d, diff:%ld\n",
			i, myData->cData[i].misc.maxAuxTH,
			myData->cData[i].misc.maxAuxTH_auxChNo,
			myData->cData[i].misc.minAuxTH,
			myData->cData[i].misc.minAuxTH_auxChNo,
			myData->cData[i].misc.diffAuxTH);
		printf("ch:%d maxAuxT:%ld, minAuxT:%ld, diffAuxT:%ld, avgAuxT:%ld\n",
			i, myData->cData[i].misc.maxAuxT,
			myData->cData[i].misc.minAuxT,
			myData->cData[i].misc.diffAuxT,
			myData->cData[i].misc.avgAuxT);
		printf("--------------------------------------------------------\n");
	}
/*	printf("testcond = %ld\n",myData->testCond[2].local_object[6]
		[myData->testCond[2].local_chCode[6][324].compIndex1]);
	printf("test_val[15]:%ld\n", myData->test_val_l[15]);
	
	printf("-------canTransmitChange--------\n"); //kjhw_170911
	for(i=ch_start; i < ch_end; i++) {
		for(j=0; j < MAX_CAN_TRANSMIT_CHANGE_DATA; j++) {
			printf("ch:%02d, function_div: %d, default_value: %f\n ", i+1,
			myData->canTransmitChange.changeData[i][j].function_div,
			myData->canTransmitChange.changeData[i][j].default_value);
		}
	}
	printf("fbCountI : ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%d ",myData->cData[i].misc.fbCountI);
	}
*/
	for(i=ch_start; i < 4; i++) {
		printf("ch:%d, PHASE: %d, STATE: %d, TX_SIG: %d, runTime: %ld t:%ld diffV: %ld\n", i+1,
		myData->cData[i].signal[C_SIG_CELL_BALANCING_PHASE],
		myData->cData[i].signal[C_SIG_CELL_BALANCING_STATE],
		myData->cData[i].signal[C_SIG_TX_CELL_BALANCING],
		myData->cData[i].op.runTime,
		myData->test_val_l[10],
		myData->cData[i].misc.diffAuxV);
	}
/*			for(i=0; i < 5; i++) {
				printf("%02d, %d, %d, %d, %d, %f, %d\n", i+1,
					myData->COA_Client[0].testCond.step[i]
					.reference[0].aux_func_div[0],
					myData->COA_Client[0].testCond.step[i]
					.reference[0].aux_compare_type[0],
					myData->COA_Client[0].testCond.step[i]
					.reference[0].aux_data_type[0],
					myData->COA_Client[0].testCond.step[i]
					.reference[0].aux_branch[0],
					myData->COA_Client[0].testCond.step[i]
					.reference[0].aux_value[0],
					myData->COA_Client[0].testCond.step[i]
					.reference[0].aux_delay_time[0]);
			}
	printf("\n");*/
}

void FCH_State_Print(void)
{
#if defined __COC__
	int i, j, ch_start, ch_end;

	j = 0;
	printf("FCH_State\n");
	
	ch_start = myData->mData.config.chPerBd * bd;
	ch_end = ch_start + myData->mData.config.chPerBd;
/*
	for(i=ch_start; i < ch_end; i++) {
		printf("model_name %d ", i+1);
		for(j=0; j < 8; j++) {
			printf("%c", myData->fch.ch[i].model_name[j]);
		}
		printf("\n");
	}
*/
	for(i=ch_start; i < ch_end; i++) {
		printf("firmware_version %d ", i+1);
		for(j=0; j < 8; j++) {
			printf("%c", myData->fch.ch[i].firmware_version[j]);
		}
		printf("\n");
	}
/*
	printf("ah ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%ld ", myData->fch.ch[i].ah);
	}
	printf("\n");

	printf("wh ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%ld ", myData->fch.ch[i].wh);
	}
	printf("\n");
*/
	printf("i_val ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%x ", (int)myData->fch.ch[i].i_val[0]);
	}
	printf("\n");
/*
	printf("v_power_bus ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%ld ", myData->fch.ch[i].v_power_bus);
	}
	printf("\n");

	printf("v_output_bus ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%ld ", myData->fch.ch[i].v_output_bus);
	}
	printf("\n");

	printf("cable_check ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%d ", myData->cData[i].signal[C_SIG_CABLE_CHECK]);
	}
	printf("\n");

	printf("cable_check_delay ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%d ", myData->cData[i].signal[C_SIG_CABLE_CHECK_DELAY]);
	}
	printf("\n");

	printf("Vsens ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%ld ", myData->cData[i].op.Vsens);
	}
	printf("\n");
*/

	printf("\n");
#endif
}

void COB_Channel_Print(int group)
{ //debug_size_cob
#if defined __COB__
	char buf[24];
	int i, tmp, ch_start, ch_end, scan_step=0;

	//printf("COB_Channel bd(%d)\n", bd+1);
	//ch_start = myData->mData.config.chPerBd * bd;
	//ch_end = ch_start + myData->mData.config.chPerBd;
	
	printf("COB_Channel group(%d)\n", group+1);
	ch_start = 0;
	ch_end = myData->mData.config.chInGroup[group];

	printf("save_step : ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%d ", myData->COB_save_step[group][i]);
	}
	printf("\n");

	printf("state : ");
	for(i=ch_start; i < ch_end; i++) {
		memset(buf, 0, sizeof buf);
		tmp = (int)myData->COB_opSave[group][i][scan_step].state;
		switch(tmp) {
			case C_IDLE:	strcpy(buf, "IDLE");		break;
			case C_STANDBY:	strcpy(buf, "STANDBY");		break;
			case C_RUN:		strcpy(buf, "RUN");			break;
			case C_PAUSE:	strcpy(buf, "PAUSE");		break;
			case C_CALI:	strcpy(buf, "CALI");		break;
			case C_FAULT:	strcpy(buf, "FAULT");		break;
			default:		strcpy(buf, "UNKNOWN");		break;
		}
		printf("%s ", buf);
	}
	printf("\n");

	printf("phase : ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%d ", myData->COB_opSave[group][i][scan_step].phase);
	}
	printf("\n");

	printf("stepType : ");
	for(i=ch_start; i < ch_end; i++) {
		memset(buf, 0, sizeof buf);
		tmp = (int)myData->COB_opSave[group][i][scan_step].stepType;
		switch(tmp) {
			case STEP_IDLE:			strcpy(buf, "IDLE");		break;
			case STEP_CHARGE:		strcpy(buf, "CHARGE");		break;
			case STEP_DISCHARGE:	strcpy(buf, "DISCHARGE");	break;
			case STEP_REST:			strcpy(buf, "REST");		break;
			case STEP_LONG_TIME_REST:
				strcpy(buf, "LONG_TIME_REST");		break;
			case STEP_OCV:			strcpy(buf, "OCV");			break;
			case STEP_Z:			strcpy(buf, "Z");			break;
			case STEP_END:			strcpy(buf, "END");			break;
			case STEP_CYCLE:		strcpy(buf, "CYCLE");		break;
			case STEP_LOOP:			strcpy(buf, "LOOP");		break;
			case STEP_PATTERN:		strcpy(buf, "PATTERN");		break;
			default:				strcpy(buf, "UNKNOWN");		break;
		}
		printf("%s ", buf);
	}
	printf("\n");

	printf("code : ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%d ", myData->COB_opSave[group][i][scan_step].code);
	}
	printf("\n");

	printf("code : ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%d ", myData->COB_opSave[group][i][scan_step].code);
	}
	printf("\n");

	printf("voltage ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%ld ", myData->COB_opSave[group][i][scan_step].Vsens);
	}
	printf("\n");

	printf("current ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%ld ", myData->COB_opSave[group][i][scan_step].Isens);
	}
	printf("\n");

	printf("\n");
#endif
}

void LineCheck_Print(int group)
{
	int i;
/*
	printf("lineCheck val\n");
	for(i=0; i < 24; i++) {
		printf("%02d(%ld:%ld) ", i+1,
			myData->lineCheck[group][i].avg_val[0],
			myData->lineCheck[group][i].avg_val[1]);
	}
	printf("\n");
	printf("lineCheck diff\n");
	for(i=0; i < 24; i++) {
		printf("%02d(%ld) ", i+1,
			myData->lineCheck[group][i].avg_val[0]
			- myData->lineCheck[group][i].avg_val[1]);
	}
	printf("\n");
*/
	printf("lineCheck2 val\n");
	for(i=0; i < 24; i++) {
		printf("%02d(%ld:%ld) ", i+1,
			myData->lineCheck[group][i].avg_val[2],
			myData->lineCheck[group][i].avg_val[3]);
	}
	printf("\n");
	printf("lineCheck2 diff\n");
	for(i=0; i < 24; i++) {
		printf("%02d(%ld) ", i+1,
			myData->lineCheck[group][i].avg_val[2]
			- myData->lineCheck[group][i].avg_val[3]);
	}
	printf("\n");
/*
	printf("lineCheck path_p:n:result ");
	for(i=0; i < 24; i++) {
		printf("%02d(%d:%d:%d) ", i+1,
			myData->lineCheck[group][i].path_val_p,
			myData->lineCheck[group][i].path_val_n,
			myData->lineCheck[group][i].result_code);
	}
	printf("\n");
*/
	printf("lineCheck4 val\n");
	for(i=0; i < 24; i++) {
		printf("%02d(%ld) ", i+1,
			myData->lineCheck[group][i].avg_val[4]);
	}
	printf("\n");

	printf("lineCheck4 diff\n");
	for(i=0; i < 24; i++) {
		printf("%02d(%ld) ", i+1,
			myData->lineCheck[group][i].avg_val[4]
			- myData->lineCheck[group][i].avg_val[2]);
	}
	printf("\n");
}

void IO_Print(void)
{
	int i, j;

	printf("IO\n");

	printf("ac_power_detect %ld, %ld, %ld\n",
		myData->dio.misc.ac_power_detect1,
		myData->dio.misc.ac_power_detect2,
		myData->dio.misc.ac_power_detect3);

	printf("delayTimer %ld\n", myData->dio.misc.delayTimer);

	printf("powerSwitchTimer %ld\n", myData->dio.misc.powerSwitchTimer);

	printf("signal remote_ps %d, in_external1 %d, in_external2 %d\n",
		myData->dio.signal[DIO_SIG_REMOTE_PS],
		myData->dio.signal[DIO_SIG_IN_EXTERNAL1],
		myData->dio.signal[DIO_SIG_IN_EXTERNAL2]);

	printf("dio_sig_remote_ps %d\n", myData->dio.misc.dio_sig_remote_ps);

	printf("in_address\n");
	for(i=0; i < MAX_DIO_IN_BYTES; i++) {
		printf("%x ", myData->dio.function_set.in_address[i]);
	}
	printf("\n");

	printf("in bytes\n");
	for(i=0; i < 4; i++) {
		for(j=0; j < 8; j++) {
			printf("%03x:", myData->dio.function_set.in_address[i*8+j]);
			printf("%02x ", myData->dio.in.bytes[i*8+j]);
		}
		printf("\n");
	}
	printf("\n");

	printf("out_address\n");
	for(i=0; i < MAX_DIO_OUT_BYTES; i++) {
		printf("%x ", myData->dio.function_set.out_address[i]);
	}
	printf("\n");
/*
	printf("out use1\n");
	for(i=0; i < 16; i++) {
		printf("%x : ", myData->dio.function_set.out_address[i]);
		for(j=0; j < 8; j++) {
			printf("%d ", myData->dio.function_set.out_set[i*8+j].use);
		}
		printf("\n");
	}
	printf("\n");

	printf("out use2\n");
	for(i=16; i < 32; i++) {
		printf("%x : ", myData->dio.function_set.out_address[i]);
		for(j=0; j < 8; j++) {
			printf("%d ", myData->dio.function_set.out_set[i*8+j].use);
		}
		printf("\n");
	}
	printf("\n");

	printf("out_set function1\n");
	for(i=0; i < 16; i++) {
		printf("%x : ", myData->dio.function_set.out_address[i]);
		for(j=0; j < 8; j++) {
			printf("%d ", myData->dio.function_set.out_set[i*8+j].function);
		}
		printf("\n");
	}
	printf("\n");

	printf("out_set function2\n");
	for(i=16; i < 32; i++) {
		printf("%x : ", myData->dio.function_set.out_address[i]);
		for(j=0; j < 8; j++) {
			printf("%d ", myData->dio.function_set.out_set[i*8+j].function);
		}
		printf("\n");
	}
	printf("\n");*/
/*
	printf("out function1\n");
	for(i=0; i < 16; i++) {
		printf("%x : ", myData->dio.function_set.out_address[i]);
		for(j=0; j < 8; j++) {
			printf("%d ", myData->dio.out.function[i*8+j]);
		}
		printf("\n");
	}
	printf("\n");

	printf("out function2\n");
	for(i=16; i < 32; i++) {
		printf("%x : ", myData->dio.function_set.out_address[i]);
		for(j=0; j < 8; j++) {
			printf("%d ", myData->dio.out.function[i*8+j]);
		}
		printf("\n");
	}
	printf("\n");
*/
/*	printf("out bytes\n");
	for(i=0; i < 4; i++) {
		for(j=0; j < 8; j++) {
			printf("%03x:", myData->dio.function_set.out_address[i*8+j]);
			printf("%02x ", myData->dio.out.bytes[i*8+j]);
		}
		printf("\n");
	}
	printf("\n");*/

	printf("signal in_start %d %d, in_stop %d %d\n",
		myData->dio.signal[DIO_SIG_IN_START1],
		myData->dio.signal[DIO_SIG_IN_START2],
		myData->dio.signal[DIO_SIG_IN_STOP1],
		myData->dio.signal[DIO_SIG_IN_STOP2]);

	printf("\n");
}

void TestCond_Print(void)
{
	int i;
	int step, monitor_ch, channel, totalStep, idx, idx2, idx3, idx4;
	long val1, val2, val3, val4, val5, val6;

	idx = idx2 = idx3 = idx4 = 0;
	i = 0;
	monitor_ch = 3;
	//monitor_ch = 2;
	//monitor_ch = 13;
	//channel = myData->CellArray1[monitor_ch-1].number2 - 1;
	channel = 2;
	printf("TestCond ch:%d\n", monitor_ch);

	idx = IDX_COM_OBJ_TOTAL_STEP;
	totalStep = (int)myData->testCond[channel].common_object[idx];

	idx = IDX_COM_OBJ_ATTRIBUTE_COUNT;
	val1 = myData->testCond[channel].common_object[idx];
	printf("totalStep %d, attr_count %ld\n", totalStep, val1);

	printf("cmd_i[0] : %ld,%ld, cmd_i[1] : %ld,%ld\n" 
					, myData->cData[0].misc.cmd_i[0] 
					, myData->cData[0].misc.cmd_i[1] 
					, myData->cData[1].misc.cmd_i[0] 
					, myData->cData[1].misc.cmd_i[1]); 

	for(step=0; step < totalStep; step++) {
	//for(step=30; step < (30+5); step++) {
	//for(step=0; step < totalStep; step++) {
		idx = IDX_LOC_OBJ_STEP_NO;
		val1 = myData->testCond[channel].local_object[step][idx];

		idx = IDX_LOC_OBJ_TYPE;
		val2 = myData->testCond[channel].local_object[step][idx];

		idx = IDX_LOC_OBJ_MODE;
		val3 = myData->testCond[channel].local_object[step][idx];

		idx = IDX_LOC_OBJ_ATTRIBUTE;
		val4 = myData->testCond[channel].local_object[step][idx];

		printf("step %ld(%d), type %ld, mode %ld, attr %ld\n",
			val1, step, val2, val3, val4);

		idx = IDX_LOC_OBJ_SAVE_DELTA_T;
		val1 = myData->testCond[channel].local_object[step][idx];

		idx = IDX_LOC_OBJ_SAVE_DELTA_V;
		val2 = myData->testCond[channel].local_object[step][idx];

		idx = IDX_LOC_OBJ_SAVE_DELTA_I;
		val3 = myData->testCond[channel].local_object[step][idx];

		printf("saveDt %ld, saveDv %ld, saveDi %ld\n", val1, val2, val3);

		idx = IDX_LOC_OBJ_REF_V;
		val1 = myData->testCond[channel].local_object[step][idx];

		idx = IDX_LOC_OBJ_REF_V2;
		val2 = myData->testCond[channel].local_object[step][idx];

		idx = IDX_LOC_OBJ_REF_I;
		val3 = myData->testCond[channel].local_object[step][idx];

		idx = IDX_LOC_OBJ_REF_P;
		val4 = myData->testCond[channel].local_object[step][idx];

		printf("refV %ld, refV2 %ld, refI %ld, refP %ld\n",
			val1, val2, val3, val4);

		idx = IDX_LOC_OBJ_REF_R;
		val1 = myData->testCond[channel].local_object[step][idx];

		idx = IDX_LOC_OBJ_RANGE_V;
		val2 = myData->testCond[channel].local_object[step][idx];

		idx = IDX_LOC_OBJ_RANGE_I;
		val3 = myData->testCond[channel].local_object[step][idx];

		printf("refR %ld, rangeV %ld, rangeI %ld\n", val1, val2, val3);

		idx = IDX_LOC_OBJ_END_I;
		printf("endI %ld\n",
			myData->testCond[channel].local_object[step][idx]);
/*		idx = IDX_LOC_OBJ_END_DELTA_V;
		printf("endDeltaV %ld\n",
			myData->testCond[channel].local_object[step][idx]);
		idx = IDX_LOC_OBJ_FAULT_UPPER_I;
		printf("faultUpperI %ld\n",
			myData->testCond[channel].local_object[step][idx]);
		idx = IDX_LOC_OBJ_FAULT_LOWER_I;
		printf("faultLowerI %ld\n",
			myData->testCond[channel].local_object[step][idx]);
		idx = IDX_LOC_OBJ_SAVE_DELTA_T;
		printf("saveDt %ld\n",
			myData->testCond[channel].local_object[step][idx]);
		idx = IDX_LOC_OBJ_Z_T1;
		printf("z_t1 %ld\n",
			myData->testCond[channel].local_object[step][idx]);
		idx = IDX_LOC_OBJ_Z_T2;
		printf("z_t2 %ld\n",
			myData->testCond[channel].local_object[step][idx]);
*/
		idx = IDX_LOC_OBJ_END_TIME;
		idx2 = IDX_LOC_OBJ_END_TIME_BRANCH;
		printf("endT %ld, endT_branch %ld\n",
			myData->testCond[channel].local_object[step][idx],
			myData->testCond[channel].local_object[step][idx2]);
/*
		idx = IDX_LOC_OBJ_END_CV_TIME;
		idx2 = IDX_LOC_OBJ_END_CV_TIME_BRANCH;
		printf("endCVT %ld, endCVT_branch %ld\n",
			myData->testCond[channel].local_object[step][idx],
			myData->testCond[channel].local_object[step][idx2]);
*/
		idx = IDX_LOC_OBJ_END_V_UPPER;
		idx2 = IDX_LOC_OBJ_END_V_UPPER_BRANCH;
		printf("endV_upper %ld, endV_upper_branch %ld\n",
			myData->testCond[channel].local_object[step][idx],
			myData->testCond[channel].local_object[step][idx2]);

		idx = IDX_LOC_OBJ_END_V_LOWER;
		idx2 = IDX_LOC_OBJ_END_V_LOWER_BRANCH;
		printf("endV_lower %ld, endV_lower_branch %ld\n",
			myData->testCond[channel].local_object[step][idx],
			myData->testCond[channel].local_object[step][idx2]);

		idx = IDX_LOC_OBJ_END_AMPARE_HOUR;
		idx2 = IDX_LOC_OBJ_END_AMPARE_HOUR_BRANCH;
		printf("endAmpareHour %ld, endAmpareHour_branch %ld\n",
			myData->testCond[channel].local_object[step][idx],
			myData->testCond[channel].local_object[step][idx2]);

		idx = IDX_LOC_OBJ_END_WATT_HOUR;
		idx2 = IDX_LOC_OBJ_END_WATT_HOUR_BRANCH;
		printf("endWattHour %ld, endWattHour_branch %ld\n",
			myData->testCond[channel].local_object[step][idx],
			myData->testCond[channel].local_object[step][idx2]);

		idx = IDX_LOC_OBJ_END_AMPARE_HOUR_RATE;
		idx2 = IDX_LOC_OBJ_END_AMPARE_HOUR_RATE_BRANCH;
		idx3 = IDX_LOC_OBJ_END_AMPARE_HOUR_RATE_COMPARE;
		printf("endAmpareHourRate %ld, branch %ld, compare %ld\n",
			myData->testCond[channel].local_object[step][idx],
			myData->testCond[channel].local_object[step][idx2],
			myData->testCond[channel].local_object[step][idx3]);

		idx = IDX_LOC_OBJ_END_WATT_HOUR_RATE;
		idx2 = IDX_LOC_OBJ_END_WATT_HOUR_RATE_BRANCH;
		idx3 = IDX_LOC_OBJ_END_WATT_HOUR_RATE_COMPARE;
		printf("endWattHourRate %ld, branch %ld, compare %ld\n",
			myData->testCond[channel].local_object[step][idx],
			myData->testCond[channel].local_object[step][idx2],
			myData->testCond[channel].local_object[step][idx3]);

/*		idx = IDX_LOC_OBJ_END_CYCLE_COUNT;
		idx2 = IDX_LOC_OBJ_END_CYCLE_COUNT_BRANCH;
		printf("cycleCount %ld, branch %ld\n",
			myData->testCond[channel].local_object[step][idx],
			myData->testCond[channel].local_object[step][idx2]);

		idx = IDX_LOC_OBJ_END_MULTI_CYCLE_COUNT;
		idx2 = IDX_LOC_OBJ_END_MULTI_CYCLE_COUNT_BRANCH;
		idx3 = IDX_LOC_OBJ_END_MULTI_CYCLE_COUNT_COMPARE;
		printf("multiCycleCount %ld, branch %ld, compare %ld\n",
			myData->testCond[channel].local_object[step][idx],
			myData->testCond[channel].local_object[step][idx2],
			myData->testCond[channel].local_object[step][idx3]);

		idx = IDX_LOC_OBJ_END_ACC_CYCLE_COUNT;
		idx2 = IDX_LOC_OBJ_END_ACC_CYCLE_COUNT_BRANCH;
		idx3 = IDX_LOC_OBJ_END_ACC_CYCLE_COUNT_COMPARE;
		printf("accCycleCount %ld, branch %ld, compare %ld\n",
			myData->testCond[channel].local_object[step][idx],
			myData->testCond[channel].local_object[step][idx2],
			myData->testCond[channel].local_object[step][idx3]);*/
/*
		idx = IDX_LOC_OBJ_END_SUM_TIME;
		idx2 = IDX_LOC_OBJ_END_SUM_TIME_BRANCH;
		printf("endSumT %ld, endSumT_branch %ld\n",
			myData->testCond[channel].local_object[step][idx],
			myData->testCond[channel].local_object[step][idx2]);

		idx = IDX_LOC_OBJ_END_SUM_AMPARE_HOUR;
		idx2 = IDX_LOC_OBJ_END_SUM_AMPARE_HOUR_BRANCH;
		printf("endSumAmpareHour %ld, endSumAmpareHour_branch %ld\n",
			myData->testCond[channel].local_object[step][idx],
			myData->testCond[channel].local_object[step][idx2]);
*/
		idx = IDX_LOC_OBJ_END_SUM_WATT_HOUR;
		idx2 = IDX_LOC_OBJ_END_SUM_WATT_HOUR_BRANCH;
		printf("endSumWattHour %ld, endSumWattHour_branch %ld\n",
			myData->testCond[channel].local_object[step][idx],
			myData->testCond[channel].local_object[step][idx2]);
/*
		idx = IDX_LOC_OBJ_END_AUX_V_UPPER;
		idx2 = IDX_LOC_OBJ_END_AUX_V_UPPER_BRANCH;
		printf("aux_v_upper %ld, %ld\n",
			myData->testCond[channel].local_object[step][idx],
			myData->testCond[channel].local_object[step][idx2]);

		idx = IDX_LOC_OBJ_END_AUX_V_LOWER;
		idx2 = IDX_LOC_OBJ_END_AUX_V_LOWER_BRANCH;
		printf("aux_v_lower %ld, %ld\n",
			myData->testCond[channel].local_object[step][idx],
			myData->testCond[channel].local_object[step][idx2]);
*/
		idx = IDX_LOC_OBJ_FAULT_HW_UPPER_V;
		val1 = myData->testCond[channel].local_object[step][idx];

		idx2 = C_CD_FAULT_HW_UPPER_V - C_CD_END_START;
		val2 = myData->testCond[channel].local_chCode[step][idx2].compCount;

		val3 = myData->testCond[channel].local_chCode[step][idx2].check_delay_count;

		idx = IDX_LOC_OBJ_FAULT_HW_LOWER_V;
		val4 = myData->testCond[channel].local_object[step][idx];

		idx2 = C_CD_FAULT_HW_LOWER_V - C_CD_END_START;
		val5 = myData->testCond[channel].local_chCode[step][idx2].compCount;

		val6 = myData->testCond[channel].local_chCode[step][idx2].check_delay_count;

		printf("hw_faultUpperV %ld %ld %ld, LowerV %ld %ld %ld\n",
			val1, val2, val3, val4, val5, val6);

		idx = IDX_LOC_OBJ_FAULT_HW_UPPER_I;
		val1 = myData->testCond[channel].local_object[step][idx];

		idx2 = C_CD_FAULT_HW_UPPER_I - C_CD_END_START;
		val2 = myData->testCond[channel].local_chCode[step][idx2].compCount;

		val3 = myData->testCond[channel].local_chCode[step][idx2].check_delay_count;

		idx = IDX_LOC_OBJ_FAULT_HW_LOWER_I;
		val4 = myData->testCond[channel].local_object[step][idx];

		idx2 = C_CD_FAULT_HW_LOWER_I - C_CD_END_START;
		val5 = myData->testCond[channel].local_chCode[step][idx2].compCount;

		val6 = myData->testCond[channel].local_chCode[step][idx2].check_delay_count;

		printf("hw_faultUpperI %ld %ld %ld, LowerI %ld %ld %ld\n",
			val1, val2, val3, val4, val5, val6);

		idx = IDX_LOC_OBJ_FAULT_UPPER_V;
		val1 = myData->testCond[channel].local_object[step][idx];

		idx = IDX_LOC_OBJ_FAULT_LOWER_V;
		val2 = myData->testCond[channel].local_object[step][idx];
		printf("faultUpperV %ld, LowerV %ld\n", val1, val2);

		idx = IDX_LOC_OBJ_FAULT_UPPER_I;
		val1 = myData->testCond[channel].local_object[step][idx];

		idx = IDX_LOC_OBJ_FAULT_LOWER_I;
		val2 = myData->testCond[channel].local_object[step][idx];
		printf("faultUpperI %ld, LowerI %ld\n", val1, val2);
/*
		idx = IDX_LOC_OBJ_FAULT_CHECK_UPPER_I;
		printf("faultCheck_UpperI %ld\n",
			myData->testCond[channel].local_object[step][idx]);
		idx = IDX_LOC_OBJ_FAULT_CHECK_LOWER_I;
		printf("faultCheck_LowerI %ld\n",
			myData->testCond[channel].local_object[step][idx]);

		//for sbl ng cond
		idx = IDX_LOC_OBJ_FAULT_VOLTAGE_DATA_MIN;
		idx2 = IDX_LOC_OBJ_FAULT_VOLTAGE_DATA_MAX;
		idx3 = IDX_LOC_OBJ_FAULT_VOLTAGE_AVG_MIN;
		idx4 = IDX_LOC_OBJ_FAULT_VOLTAGE_AVG_MAX;
		printf("fault_ng_Voltage %ld, %ld, %ld, %ld\n",
			myData->testCond[channel].local_object[step][idx],
			myData->testCond[channel].local_object[step][idx2],
			myData->testCond[channel].local_object[step][idx3],
			myData->testCond[channel].local_object[step][idx4]);
		idx = IDX_LOC_OBJ_FAULT_AMPARE_HOUR_DATA_MIN;
		idx2 = IDX_LOC_OBJ_FAULT_AMPARE_HOUR_DATA_MAX;
		idx3 = IDX_LOC_OBJ_FAULT_AMPARE_HOUR_AVG_MIN;
		idx4 = IDX_LOC_OBJ_FAULT_AMPARE_HOUR_AVG_MAX;
		printf("fault_ng_Ah %ld, %ld, %ld, %ld\n",
			myData->testCond[channel].local_object[step][idx],
			myData->testCond[channel].local_object[step][idx2],
			myData->testCond[channel].local_object[step][idx3],
			myData->testCond[channel].local_object[step][idx4]);
		idx = IDX_LOC_OBJ_FAULT_POWER_DATA_MIN;
		idx2 = IDX_LOC_OBJ_FAULT_POWER_DATA_MAX;
		idx3 = IDX_LOC_OBJ_FAULT_POWER_AVG_MIN;
		idx4 = IDX_LOC_OBJ_FAULT_POWER_AVG_MAX;
		printf("fault_ng_Power %ld, %ld, %ld, %ld\n",
			myData->testCond[channel].local_object[step][idx],
			myData->testCond[channel].local_object[step][idx2],
			myData->testCond[channel].local_object[step][idx3],
			myData->testCond[channel].local_object[step][idx4]);
*/
/*		idx = IDX_LOC_OBJ_CAN_VALUE_1;
		printf("can_value %ld %ld %ld %ld %ld, %ld %ld %ld %ld %ld\n",
			myData->testCond[channel].local_object[step][idx],
			myData->testCond[channel].local_object[step][idx+1],
			myData->testCond[channel].local_object[step][idx+2],
			myData->testCond[channel].local_object[step][idx+3],
			myData->testCond[channel].local_object[step][idx+4],
			myData->testCond[channel].local_object[step][idx+5],
			myData->testCond[channel].local_object[step][idx+6],
			myData->testCond[channel].local_object[step][idx+7],
			myData->testCond[channel].local_object[step][idx+8],
			myData->testCond[channel].local_object[step][idx+9]
			);

		idx = IDX_LOC_OBJ_CAN_FUNC_DIV_1;
		printf("can_func %ld %ld %ld %ld %ld, %ld %ld %ld %ld %ld\n",
			myData->testCond[channel].local_object[step][idx],
			myData->testCond[channel].local_object[step][idx+1],
			myData->testCond[channel].local_object[step][idx+2],
			myData->testCond[channel].local_object[step][idx+3],
			myData->testCond[channel].local_object[step][idx+4],
			myData->testCond[channel].local_object[step][idx+5],
			myData->testCond[channel].local_object[step][idx+6],
			myData->testCond[channel].local_object[step][idx+7],
			myData->testCond[channel].local_object[step][idx+8],
			myData->testCond[channel].local_object[step][idx+9]
			);
		printf("\n");

		idx = IDX_LOC_OBJ_CAN_BRANCH_1;
		printf("can_branch %ld %ld %ld %ld %ld, %ld %ld %ld %ld %ld\n",
			myData->testCond[channel].local_object[step][idx],
			myData->testCond[channel].local_object[step][idx+1],
			myData->testCond[channel].local_object[step][idx+2],
			myData->testCond[channel].local_object[step][idx+3],
			myData->testCond[channel].local_object[step][idx+4],
			myData->testCond[channel].local_object[step][idx+5],
			myData->testCond[channel].local_object[step][idx+6],
			myData->testCond[channel].local_object[step][idx+7],
			myData->testCond[channel].local_object[step][idx+8],
			myData->testCond[channel].local_object[step][idx+9]
			);
		printf("\n");

		idx = IDX_LOC_OBJ_AUX_VALUE_1;
		printf("aux_value %ld %ld %ld %ld %ld, %ld %ld %ld %ld %ld\n",
			myData->testCond[channel].local_object[step][idx],
			myData->testCond[channel].local_object[step][idx+1],
			myData->testCond[channel].local_object[step][idx+2],
			myData->testCond[channel].local_object[step][idx+3],
			myData->testCond[channel].local_object[step][idx+4],
			myData->testCond[channel].local_object[step][idx+5],
			myData->testCond[channel].local_object[step][idx+6],
			myData->testCond[channel].local_object[step][idx+7],
			myData->testCond[channel].local_object[step][idx+8],
			myData->testCond[channel].local_object[step][idx+9]
			);

		idx = IDX_LOC_OBJ_AUX_FUNC_DIV_1;
		printf("aux_func %ld %ld %ld %ld %ld, %ld %ld %ld %ld %ld\n",
			myData->testCond[channel].local_object[step][idx],
			myData->testCond[channel].local_object[step][idx+1],
			myData->testCond[channel].local_object[step][idx+2],
			myData->testCond[channel].local_object[step][idx+3],
			myData->testCond[channel].local_object[step][idx+4],
			myData->testCond[channel].local_object[step][idx+5],
			myData->testCond[channel].local_object[step][idx+6],
			myData->testCond[channel].local_object[step][idx+7],
			myData->testCond[channel].local_object[step][idx+8],
			myData->testCond[channel].local_object[step][idx+9]
			);
		printf("\n");

		idx = IDX_LOC_OBJ_AUX_BRANCH_1;
		printf("aux_branch %ld %ld %ld %ld %ld, %ld %ld %ld %ld %ld\n",
			myData->testCond[channel].local_object[step][idx],
			myData->testCond[channel].local_object[step][idx+1],
			myData->testCond[channel].local_object[step][idx+2],
			myData->testCond[channel].local_object[step][idx+3],
			myData->testCond[channel].local_object[step][idx+4],
			myData->testCond[channel].local_object[step][idx+5],
			myData->testCond[channel].local_object[step][idx+6],
			myData->testCond[channel].local_object[step][idx+7],
			myData->testCond[channel].local_object[step][idx+8],
			myData->testCond[channel].local_object[step][idx+9]
			);
*/
		printf("\n");
	}

	//common_condition
	idx = IDX_COM_OBJ_FAULT_HW_UPPER_V;
	val1 = myData->testCond[channel].common_object[idx];

	idx2 = C_CD_COM_FAULT_HW_UPPER_V - C_CD_COM_START;
	val2 = myData->testCond[channel].common_chCode[idx2].compCount;

	val3 = myData->testCond[channel].common_chCode[idx2].check_delay_count;

	idx = IDX_COM_OBJ_FAULT_HW_LOWER_V;
	val4 = myData->testCond[channel].common_object[idx];

	idx2 = C_CD_COM_FAULT_HW_LOWER_V - C_CD_COM_START;
	val5 = myData->testCond[channel].common_chCode[idx2].compCount;

	val6 = myData->testCond[channel].common_chCode[idx2].check_delay_count;
	printf("hw_upper_v %ld %ld %ld, hw_lower_v %ld %ld %ld\n",
		val1, val2, val3, val4, val5, val6);

	idx = IDX_COM_OBJ_FAULT_HW_UPPER_I;
	val1 = myData->testCond[channel].common_object[idx];

	idx2 = C_CD_COM_FAULT_HW_UPPER_I - C_CD_COM_START;
	val2 = myData->testCond[channel].common_chCode[idx2].compCount;

	val3 = myData->testCond[channel].common_chCode[idx2].check_delay_count;

	idx = IDX_COM_OBJ_FAULT_HW_LOWER_I;
	val4 = myData->testCond[channel].common_object[idx];

	idx2 = C_CD_COM_FAULT_HW_LOWER_I - C_CD_COM_START;
	val5 = myData->testCond[channel].common_chCode[idx2].compCount;

	val6 = myData->testCond[channel].common_chCode[idx2].check_delay_count;
	printf("hw_upper_i %ld %ld %ld, hw_lower_i %ld %ld %ld\n",
		val1, val2, val3, val4, val5, val6);

	idx = IDX_COM_OBJ_FAULT_UPPER_V;
	val1 = myData->testCond[channel].common_object[idx];

	idx = IDX_COM_OBJ_FAULT_LOWER_V;
	val2 = myData->testCond[channel].common_object[idx];

	idx = IDX_COM_OBJ_FAULT_UPPER_I;
	val3 = myData->testCond[channel].common_object[idx];

	idx = IDX_COM_OBJ_FAULT_UPPER_AMPARE_HOUR;
	val4 = myData->testCond[channel].common_object[idx];

	printf("safetyUpperV %ld, LowerV %ld, UpperI %ld, UpperAh %ld\n",
		val1, val2, val3, val4);

	idx = IDX_COM_OBJ_FAULT_UPPER_WATT_HOUR;
	val1 = myData->testCond[channel].common_object[idx];

	idx = IDX_COM_OBJ_FAULT_UPPER_P;
	val2 = myData->testCond[channel].common_object[idx];

	idx = IDX_COM_OBJ_FAULT_UPPER_TEMP;
	val3 = myData->testCond[channel].common_object[idx];

	idx = IDX_COM_OBJ_FAULT_LOWER_TEMP;
	val4 = myData->testCond[channel].common_object[idx];

	printf("safetyUpperWh %ld, UpperP %ld, UpperTemp %ld, LowerTemp %ld\n",
		val1, val2, val3, val4);

	idx = IDX_COM_OBJ_FAULT_COMP_AUX_V;
	val1 = myData->testCond[channel].common_object[idx];
	printf("comp_aux_V %ld", val1);

/*	idx = IDX_COM_OBJ_CAN_FUNC_DIV_1;
	printf("safety_can_func %ld %ld %ld %ld %ld, %ld %ld %ld %ld %ld\n",
		myData->testCond[channel].common_object[idx],
		myData->testCond[channel].common_object[idx+1],
		myData->testCond[channel].common_object[idx+2],
		myData->testCond[channel].common_object[idx+3],
		myData->testCond[channel].common_object[idx+4],
		myData->testCond[channel].common_object[idx+5],
		myData->testCond[channel].common_object[idx+6],
		myData->testCond[channel].common_object[idx+7],
		myData->testCond[channel].common_object[idx+8],
		myData->testCond[channel].common_object[idx+9]
		);

	idx = IDX_COM_OBJ_CAN_VALUE_1;
	printf("safety_can_val %ld %ld %ld %ld %ld, %ld %ld %ld %ld %ld\n",
		myData->testCond[channel].common_object[idx],
		myData->testCond[channel].common_object[idx+1],
		myData->testCond[channel].common_object[idx+2],
		myData->testCond[channel].common_object[idx+3],
		myData->testCond[channel].common_object[idx+4],
		myData->testCond[channel].common_object[idx+5],
		myData->testCond[channel].common_object[idx+6],
		myData->testCond[channel].common_object[idx+7],
		myData->testCond[channel].common_object[idx+8],
		myData->testCond[channel].common_object[idx+9]
		);

	idx = C_CD_COM_CAN_FAULT_FUNC_DIV_1 - C_CD_CAN_START;
	printf("safety_can_comp_type %d %d %d %d %d, %d %d %d %d %d\n",
		myData->testCond[channel].can_chCode[idx].compType,
		myData->testCond[channel].can_chCode[idx+1].compType,
		myData->testCond[channel].can_chCode[idx+2].compType,
		myData->testCond[channel].can_chCode[idx+3].compType,
		myData->testCond[channel].can_chCode[idx+4].compType,
		myData->testCond[channel].can_chCode[idx+5].compType,
		myData->testCond[channel].can_chCode[idx+6].compType,
		myData->testCond[channel].can_chCode[idx+7].compType,
		myData->testCond[channel].can_chCode[idx+8].compType,
		myData->testCond[channel].can_chCode[idx+9].compType
		);

	idx = IDX_COM_OBJ_AUX_FUNC_DIV_1;
	printf("safety_aux_func %ld %ld %ld %ld %ld, %ld %ld %ld %ld %ld\n",
		myData->testCond[channel].common_object[idx],
		myData->testCond[channel].common_object[idx+1],
		myData->testCond[channel].common_object[idx+2],
		myData->testCond[channel].common_object[idx+3],
		myData->testCond[channel].common_object[idx+4],
		myData->testCond[channel].common_object[idx+5],
		myData->testCond[channel].common_object[idx+6],
		myData->testCond[channel].common_object[idx+7],
		myData->testCond[channel].common_object[idx+8],
		myData->testCond[channel].common_object[idx+9]
		);

	idx = IDX_COM_OBJ_AUX_VALUE_1;
	printf("safety_aux_val %ld %ld %ld %ld %ld, %ld %ld %ld %ld %ld\n",
		myData->testCond[channel].common_object[idx],
		myData->testCond[channel].common_object[idx+1],
		myData->testCond[channel].common_object[idx+2],
		myData->testCond[channel].common_object[idx+3],
		myData->testCond[channel].common_object[idx+4],
		myData->testCond[channel].common_object[idx+5],
		myData->testCond[channel].common_object[idx+6],
		myData->testCond[channel].common_object[idx+7],
		myData->testCond[channel].common_object[idx+8],
		myData->testCond[channel].common_object[idx+9]
		);

	idx = C_CD_COM_AUX_FAULT_FUNC_DIV_1 - C_CD_AUX_START;
	printf("safety_aux_comp_type %d %d %d %d %d, %d %d %d %d %d\n",
		myData->testCond[channel].aux_chCode[idx].compType,
		myData->testCond[channel].aux_chCode[idx+1].compType,
		myData->testCond[channel].aux_chCode[idx+2].compType,
		myData->testCond[channel].aux_chCode[idx+3].compType,
		myData->testCond[channel].aux_chCode[idx+4].compType,
		myData->testCond[channel].aux_chCode[idx+5].compType,
		myData->testCond[channel].aux_chCode[idx+6].compType,
		myData->testCond[channel].aux_chCode[idx+7].compType,
		myData->testCond[channel].aux_chCode[idx+8].compType,
		myData->testCond[channel].aux_chCode[idx+9].compType
		);*/
/*
	//for sbl safety
	idx = IDX_COM_OBJ_FAULT_CHARGE_UPPER_V;
	printf("safetyChargeUpperV %ld\n",
		myData->testCond[channel].common_object[idx]);
	idx = IDX_COM_OBJ_FAULT_CHARGE_UPPER_AMPARE_HOUR;
	printf("safetyChargeUpperAh %ld\n",
		myData->testCond[channel].common_object[idx]);
	idx = IDX_COM_OBJ_FAULT_CHARGE_END_UPPER_V;
	printf("safetyChargeEndUpperV %ld\n",
		myData->testCond[channel].common_object[idx]);
	idx = IDX_COM_OBJ_FAULT_CHARGE_LOWER_I;
	printf("safetyChargeLowerI %ld\n",
		myData->testCond[channel].common_object[idx]);

	idx = IDX_COM_OBJ_FAULT_DISCHARGE_LOWER_V;
	printf("safetyDischargeLowerV %ld\n",
		myData->testCond[channel].common_object[idx]);
	idx = IDX_COM_OBJ_FAULT_DISCHARGE_RUN_TIME;
	printf("safetyDischargeRunTime %ld\n",
		myData->testCond[channel].common_object[idx]);
	idx = IDX_COM_OBJ_FAULT_DISCHARGE_LOWER_AMPARE_HOUR;
	printf("safetyDischargeLowerAh %ld\n",
		myData->testCond[channel].common_object[idx]);

	idx = IDX_COM_OBJ_FAULT_OVER_CURRENT_LIMIT;
	printf("safetyOverCurrent %ld\n",
		myData->testCond[channel].common_object[idx]);

	idx = IDX_COM_OBJ_FAULT_COMP_V_T1;
	printf("safetyComp_CheckV_T %ld\n",
		myData->testCond[channel].common_object[idx]);
	idx = IDX_COM_OBJ_FAULT_COMP_LOWER_V1;
	printf("safetyComp_CheckLowerV %ld\n",
		myData->testCond[channel].common_object[idx]);

	idx = IDX_COM_OBJ_FAULT_COMP_V_T2;
	printf("safetyComp_V_T1 %ld\n",
		myData->testCond[channel].common_object[idx]);
	idx = IDX_COM_OBJ_FAULT_COMP_LOWER_V2;
	printf("safetyComp_LowerV1 %ld\n",
		myData->testCond[channel].common_object[idx]);
	idx = IDX_COM_OBJ_FAULT_COMP_UPPER_V2;
	printf("safetyComp_UpperV1 %ld\n",
		myData->testCond[channel].common_object[idx]);

	idx = IDX_COM_OBJ_FAULT_COMP_V_T3;
	printf("safetyComp_V_T2 %ld\n",
		myData->testCond[channel].common_object[idx]);
	idx = IDX_COM_OBJ_FAULT_COMP_LOWER_V3;
	printf("safetyComp_LowerV2 %ld\n",
		myData->testCond[channel].common_object[idx]);
	idx = IDX_COM_OBJ_FAULT_COMP_UPPER_V3;
	printf("safetyComp_UpperV2 %ld\n",
		myData->testCond[channel].common_object[idx]);

	idx = IDX_COM_OBJ_FAULT_COMP_LOWER_CV_TIME;
	printf("safetyComp_Lower_CV_T %ld\n",
		myData->testCond[channel].common_object[idx]);
	idx = IDX_COM_OBJ_FAULT_COMP_UPPER_CV_TIME;
	printf("safetyComp_Upper_CV_T %ld\n",
		myData->testCond[channel].common_object[idx]);

	idx = IDX_COM_OBJ_FAULT_COMP_I_T1;
	printf("safetyComp_I_T1 %ld\n",
		myData->testCond[channel].common_object[idx]);
	idx = IDX_COM_OBJ_FAULT_COMP_LOWER_I1;
	printf("safetyComp_LowerI1 %ld\n",
		myData->testCond[channel].common_object[idx]);
	idx = IDX_COM_OBJ_FAULT_COMP_UPPER_I1;
	printf("safetyComp_UpperI1 %ld\n",
		myData->testCond[channel].common_object[idx]);
	printf("\n");

	//for sbl check
	idx = IDX_LOC_OBJ_FAULT_CHECK_DETECT_V;
	printf("checkDetectV %ld\n",
		myData->testCond[channel].local_object[1][idx]);
	idx = IDX_LOC_OBJ_FAULT_CHECK_REVERSE_V;
	printf("checkReverseV %ld\n",
		myData->testCond[channel].local_object[1][idx]);
	idx = IDX_LOC_OBJ_FAULT_CHECK_LOWER_OCV;
	printf("checkLowerOCV %ld\n",
		myData->testCond[channel].local_object[1][idx]);
	idx = IDX_LOC_OBJ_FAULT_CHECK_UPPER_OCV;
	printf("checkUpperOCV %ld\n",
		myData->testCond[channel].local_object[1][idx]);

	idx = IDX_LOC_OBJ_END_TIME;
	printf("checkEndT %ld\n",
		myData->testCond[channel].local_object[2][idx]);
	idx = IDX_LOC_OBJ_REF_V;
	printf("checkRefV %ld\n",
		myData->testCond[channel].local_object[2][idx]);
	idx = IDX_LOC_OBJ_REF_I;
	printf("checkRefI %ld\n",
		myData->testCond[channel].local_object[2][idx]);
	idx = IDX_LOC_OBJ_FAULT_CHECK_I_JUDGE_RATIO;
	printf("checkI_judgeRatio %ld\n",
		myData->testCond[channel].local_object[2][idx]);
	idx = IDX_LOC_OBJ_FAULT_CHECK_UPPER_DELTA_V;
	printf("checkUpperDV %ld\n",
		myData->testCond[channel].local_object[2][idx]);
	idx = IDX_LOC_OBJ_FAULT_CHECK_LOWER_DELTA_V;
	printf("checkLowerDV %ld\n",
		myData->testCond[channel].local_object[2][idx]);
*/
/*	printf("reserved_cmd:%d, reserved_stepNo:%ld, reserved_cycleNo:%ld\n",
		(int)myData->testCond[channel].reserved.reserved_cmd,
		myData->testCond[channel].reserved.reserved_stepNo,
		myData->testCond[channel].reserved.reserved_cycleNo);

	printf("select_run:%d, select_stepNo:%ld, select_cycleNo:%ld, select_advCycleStep:%ld\n",
		(int)myData->testCond[channel].reserved.select_run,
		myData->testCond[channel].reserved.select_stepNo,
		myData->testCond[channel].reserved.select_cycleNo,
		myData->testCond[channel].reserved.select_advCycleStep);

	printf("pattern %d", channel);
	for(i=0; i < 25; i++) {
		printf("(%d)%ld:%ld:%ld:%ld:%d ", i,
			myData->testCond[channel].pattern[i].t_val,
			myData->testCond[channel].pattern[i].cmd_val[0],
			myData->testCond[channel].pattern[i].cmd_val[1],
			myData->testCond[channel].pattern[i].cmd_val[2], //kjhw_150914
			myData->testCond[channel].pattern[i].waveform_type);
	}
	printf("\n");*/
/*
	printf("can compCount(type) ");
	for(i=0; i < 5; i++) {
		printf("%d(%d) ",
			myData->testCond[channel].can_chCode[i].compCount,
			myData->testCond[channel].can_chCode[i].compType);
	}
	printf("\n");
*/
/*	printf("external_can %d", channel);
	printf("%ld:%ld:%ld:%ld:%ld:%ld:%ld:%d:%d:%d:%d\n",
		myData->testCond[channel].external_data[1].cmd_v[0],
		myData->testCond[channel].external_data[1].cmd_v[1],
		myData->testCond[channel].external_data[1].cmd_i[0],
		myData->testCond[channel].external_data[1].cmd_i[1],
		myData->testCond[channel].external_data[1].cmd_p[0],
		myData->testCond[channel].external_data[1].cmd_p[1],
		myData->testCond[channel].external_data[1].t_val,
		myData->testCond[channel].external_data[1].version,
		myData->testCond[channel].external_data[1].mode,
		myData->testCond[channel].external_data[1].waveform_type,
		myData->testCond[channel].external_data[1].control);
*/
	printf("\n");
}

void CaliData_Print(void)
{
	int i, channel, type, range;
	int j,k,l;

	printf("CaliData------------\n");

	channel = 0;
	type = 0;
	range = 0;

	/*
	for(j=0; j < 5; j++) {
		printf("ch %d------\n", j);
		for(k=0; k < 1; k++) {
			for(i=0; i < MAX_CALI_POINT; i++) {
//		printf("%d DA_A:%f, DA_B:%f\n", i,
//		myData->cali[channel].tmpData[type][range].DA_A[i],
//		myData->cali[channel].tmpData[type][range].DA_B[i]);
				printf("%d DA_A:%f, DA_B:%f\n", i,
					myData->cali[j].data[k][range].DA_A[i],
					myData->cali[j].data[k][range].DA_B[i]);
			}

			for(i=0; i < MAX_CALI_POINT; i++) {
//		printf("%d AD_A:%f, AD_B:%f\n", i,
//			myData->cali[channel].tmpData[type][range].AD_A[i],
//			myData->cali[channel].tmpData[type][range].AD_B[i]);
				printf("%d AD_A:%f, AD_B:%f\n", i,
					myData->cali[j].data[k][range].AD_A[i],
					myData->cali[j].data[k][range].AD_B[i]);
			}
		}
	}*/
	for(i=0; i < myData->mData.config.installedCh; i++) {
		for(j=0; j < MAX_TYPE; j++) {
			for(k=0; k < MAX_RANGE; k++) {
				printf("ch %d, type %d, range %d, count %d\n",
					i, j, k, myData->cali[i].count);	
				printf("CALI caliPoint ");
				for(l=0; l < myData->cali[channel].tmpCond[type][range]
					.point.setPointNum; l++) {
					printf("%ld ", myData->cali[i].tmpCond[j][k]
						.point.setPoint[l]);
				}
				printf("\n");

				printf("caliCheckPoint ");
				for(l=0; l < myData->cali[i].tmpCond[j][k]
					.point.checkPointNum; l++) {
					printf("%ld ", myData->cali[i].tmpCond[j][k]
						.point.checkPoint[l]);
				}
				printf("\n");
			}
		}
	}
	printf("CALI caliPointNum %d, caliCheckPointNum %d\n",
		myData->cali[channel].tmpCond[type][range].point.setPointNum,
		myData->cali[channel].tmpCond[type][range].point.checkPointNum);

	printf("CALI caliPoint ");
	for(i=0; i < myData->cali[channel].tmpCond[type][range]
		.point.setPointNum; i++) {
		printf("%ld ", myData->cali[channel].tmpCond[type][range]
			.point.setPoint[i]);
	}
	printf("\n");

	printf("caliCheckPoint ");
	for(i=0; i < myData->cali[channel].tmpCond[type][range]
		.point.checkPointNum; i++) {
		printf("%ld ", myData->cali[channel].tmpCond[type][range]
			.point.checkPoint[i]);
	}
	printf("\n");
/*
	printf("set ad    ");
	for(i=0; i < MAX_CALI_POINT; i++) {
		printf("%f, ", (float)myData->cali[channel].tmpData[type][range]
			.set_ad[i]);
	}
	printf("\n");
	
	printf("set meter ");
	for(i=0; i < MAX_CALI_POINT; i++) {
		printf("%f, ", (float)myData->cali[channel].tmpData[type][range]
			.set_meter[i]);
	}
	printf("\n");
	
	printf("check ad    ");
	for(i=0; i < MAX_CALI_POINT; i++) {
		printf("%f, ", (float)myData->cali[channel].tmpData[type][range]
			.check_ad[i]);
	}
	printf("\n");
	
	printf("check meter ");
	for(i=0; i < MAX_CALI_POINT; i++) {
		printf("%f, ", (float)myData->cali[channel].tmpData[type][range]
			.check_meter[i]);
	}
	printf("\n");
	
	printf("check AD_ratio %f, %f\n",
		(float)myData->cali[channel].tmpData[type][range].AD_Ratio[0],
		(float)myData->cali[channel].tmpData[type][range].AD_Ratio[1]);
	printf("\n");

	printf("check DA_A[1] %f, DA_B[1] %f\n",
		(float)myData->cali[channel].tmpData[type][range].DA_A[1],
		(float)myData->cali[channel].tmpData[type][range].DA_B[1]);
	printf("check DA_A[2] %f, DA_B[2] %f\n",
		(float)myData->cali[channel].tmpData[type][range].DA_A[2],
		(float)myData->cali[channel].tmpData[type][range].DA_B[2]);
	printf("\n");

	for(i=0; i < myData->cali[channel].tmpCond[type][range]
		.point.setPointNum-1; i++) {
		printf("check DA_A[%d] %f, DA_B[%d] %f\n", i,
			(float)myData->cali[channel].tmpData[type][range].DA_A[0], i,
			(float)myData->cali[channel].tmpData[type][range].DA_B[0]);
	}
	printf("\n");
	*/
}

void SubSensV_Print(void)
{
	int i, j, k, bd=0;
/*
	printf("SubSensV %d\n",
		myData->SubSensV.signal[SUB_SENS_SIG_DAQ_ISOLATION]);
	i = j = k = 0;
	bd = 0;
*/

	printf("org_data\n");
	for(i=0; i < 6; i++) {
		printf("%02x ", myData->SubSensV.org_data[bd].zero_val[i]);
	}
	printf("\n");

	for(i=0; i < 3; i++) {
		for(j=0; j < 4; j++) {
			if(myData->mData.config.daq_type == DAQ_TYPE1) {
				printf("%02x %02x %02x %02x ",
					myData->SubSensV.org_data[bd].ref_val[i][j][0].byte[1],
					myData->SubSensV.org_data[bd].ref_val[i][j][0].byte[0],
					myData->SubSensV.org_data[bd].ref_val[i][j][1].byte[1],
					myData->SubSensV.org_data[bd].ref_val[i][j][1].byte[0]);
			} else {
				printf("%02x %02x %02x %02x %02x %02x %02x %02x\n",
					myData->SubSensV.org_data[bd].ref_val[i][j][0].byte[1],
					myData->SubSensV.org_data[bd].ref_val[i][j][0].byte[0],
					myData->SubSensV.org_data[bd].ref_val[i][j][1].byte[1],
					myData->SubSensV.org_data[bd].ref_val[i][j][1].byte[0],
					myData->SubSensV.org_data[bd].ref_val[i][j][2].byte[1],
					myData->SubSensV.org_data[bd].ref_val[i][j][2].byte[0],
					myData->SubSensV.org_data[bd].ref_val[i][j][3].byte[1],
					myData->SubSensV.org_data[bd].ref_val[i][j][3].byte[0]);
				
				printf("%d %d %d %d\n",
					myData->SubSensV.org_data[bd].ref_val[i][j][0].val,
					myData->SubSensV.org_data[bd].ref_val[i][j][1].val,
					myData->SubSensV.org_data[bd].ref_val[i][j][2].val,
					myData->SubSensV.org_data[bd].ref_val[i][j][3].val);
			}
		}
	}
	printf("\n");

	printf("org_data.sub_val\n");
	//for(i=0; i < 8; i++) {
	for(i=0; i < 8; i++) {
		for(j=0; j < 4; j++) {
			if(myData->mData.config.daq_type == DAQ_TYPE1) {
				printf("%02x %02x, %02x %02x ",
					myData->SubSensV.org_data[bd].sub_val[i][j][0].byte[1],
					myData->SubSensV.org_data[bd].sub_val[i][j][0].byte[0],
					myData->SubSensV.org_data[bd].sub_val[i][j][1].byte[1],
					myData->SubSensV.org_data[bd].sub_val[i][j][1].byte[0]);
			} else {
					/*		//csk_190904d
				printf("%02x %02x, %02x %02x, %02x %02x, %02x %02x\n",
					myData->SubSensV.org_data[bd].sub_val[i][j][0].byte[1],
					myData->SubSensV.org_data[bd].sub_val[i][j][0].byte[0],
					myData->SubSensV.org_data[bd].sub_val[i][j][1].byte[1],
					myData->SubSensV.org_data[bd].sub_val[i][j][1].byte[0],
					myData->SubSensV.org_data[bd].sub_val[i][j][2].byte[1],
					myData->SubSensV.org_data[bd].sub_val[i][j][2].byte[0],
					myData->SubSensV.org_data[bd].sub_val[i][j][3].byte[1],
					myData->SubSensV.org_data[bd].sub_val[i][j][3].byte[0]);
				
				printf("%d %d %d %d\n",
					myData->SubSensV.org_data[bd].sub_val[i][j][0].val,
					myData->SubSensV.org_data[bd].sub_val[i][j][1].val,
					myData->SubSensV.org_data[bd].sub_val[i][j][2].val,
					myData->SubSensV.org_data[bd].sub_val[i][j][3].val);
				*/		//csk_190904d
				
				printf("--%ld %ld %ld %ld\n",
					(long)(myData->SubSensV.org_data[bd].sub_val[i][j][0].val * 204.2675912 * 0.994340 + 683.111456),
					(long)(myData->SubSensV.org_data[bd].sub_val[i][j][1].val * 204.2675912 * 0.995799 + 1837.249380),
					(long)(myData->SubSensV.org_data[bd].sub_val[i][j][2].val * 204.2675912 * 0.994471 + 972.592580),
					(long)(myData->SubSensV.org_data[bd].sub_val[i][j][3].val * 204.2675912 * 0.994318 + 2049.290355));
			}
		}
	}
/*
//	printf("%02x %02x\n", myData->SubSensV.org_data[bd].equal_val[0],
//		myData->SubSensV.org_data[bd].equal_val[1]);


	printf("SubSensV count:%d flag:%d\n",
		myData->mData.misc.SubSensV_SourceSensCount,
		myData->mData.misc.SubSensV_SourceSensCountFlag);
	//for(i=0; i < MAX_SUB_SENS_V_SOURCE_SENS_COUNT; i++) {
	for(i=0; i < 200; i++) {
		printf("%d ", myData->SubSensV.source_div[bd].source[0][0].tmpV[0][i]);
	}
	printf("\n");
*/

	switch(myData->mData.config.daq_type) {
		case DAQ_TYPE1:	k = 2; break;
		case DAQ_TYPE2:	k = 4; break;
		case DAQ_TYPE3:	k = 4; break;
		default:		k = 4; break;
	}

	for(j=0; j < k; j++) {
		switch(j) {
			case 0: printf("sourceA\n"); break;
			case 1: printf("sourceB\n"); break;
			case 2: printf("sourceC\n"); break;
			case 3: printf("sourceD\n"); break;
			default:	break;
		}
		for(i=0; i < 4; i++) {
   			printf("sensV[%d] %ld, %ld, %ld\n", i,
			myData->SubSensV.source_div[bd].source[i][j].sensV[0],
			myData->SubSensV.source_div[bd].source[i][j].sensV[1],
			myData->SubSensV.source_div[bd].source[i][j].sensV[2]);
	   		printf("calV[%d] %ld, %ld, %ld\n", i,
			myData->SubSensV.source_div[bd].source[i][j].calV[0],
			myData->SubSensV.source_div[bd].source[i][j].calV[1],
			myData->SubSensV.source_div[bd].source[i][j].calV[2]);
			printf("V_AD a:%f, b:%f, a_N:%f, b_N:%f\n",
				myData->SubSensV.source_div[bd].source[i][j].V_AD_a,
				myData->SubSensV.source_div[bd].source[i][j].V_AD_b,
				myData->SubSensV.source_div[bd].source[i][j].V_AD_a_N,
				myData->SubSensV.source_div[bd].source[i][j].V_AD_b_N);
		}
		printf("\n");
	}
	
#if defined __COA_VER_100B__ || defined __COA_VER_100B2__ || defined __COA_VER_100C__ || defined __COA_VER_100D__
	j = myData->mData.config.installedAuxV;
#else //COA_VER_100F~
	j = 104;
#endif

	printf("sub_val\n");
	printf("tmpSensV ");
	for(i=0; i < j; i++) {
		printf("%ld[%d] ", myData->SubSensV.ch[i].tmpSensV, i);
	}
	printf("\n");

	printf("sensV ");
	for(i=0; i < j; i++) {
		printf("%ld[%d] ", myData->SubSensV.ch[i].sensV, i);
	}
	printf("\n");

	printf("\n");
}

void AuxV_Print(void)
{
	int i;

	printf("AuxSetData\n");
	for(i=0; i < 18; i++) {
		printf("auxChNo : %d, auxType : %d, chNo : %d, name : %s\n",
			(int)myData->auxSetData[i].auxChNo,
			(int)myData->auxSetData[i].auxType,
			(int)myData->auxSetData[i].chNo,
			myData->auxSetData[i].name);
	}
	printf("temp_count ch1:%d ch2:%d\n", myData->auxDataCount[0][0],
		myData->auxDataCount[1][0]);
	printf("auxV_count ch1:%d ch2:%d\n", myData->auxDataCount[0][1],
		myData->auxDataCount[1][1]);
#if defined __COA_VER_100B__ || defined __COA_VER_100B2__ || defined __COA_VER_100C__ || defined __COA_VER_100D__
#else //COA_VER_100F~
	printf("T  H_count ch1:%d ch2:%d\n", myData->auxDataCount[0][2],
		myData->auxDataCount[1][2]);
#endif
	printf("\n");
	//kjhw_150731
	printf("ch1 minV: %ld, maxV: %ld, diffAuxV: %ld\n",
		myData->cData[0].misc.minAuxV,
		myData->cData[0].misc.maxAuxV,
		myData->cData[0].misc.diffAuxV);
	printf("ch2 minV: %ld, maxV: %ld, diffAuxV: %ld\n",
		myData->cData[1].misc.minAuxV,
		myData->cData[1].misc.maxAuxV,
		myData->cData[1].misc.diffAuxV);
	printf("\n");
}

void AuxTemp_Print(void)
{
	int i, ch_per_meter;
	char fileName[256];
	FILE *fp;

	printf("AuxTemp\n");

	if(myData->COM.config.functionModel[1] == 0) ch_per_meter = 8;
	else ch_per_meter = 16;

	printf("value ");
	for(i=0; i < (myData->COM.config.countMeter[1] * ch_per_meter); i++) {
		printf("%ld(%d) ",
		myData->COM.com_port[1].value[i],
		myData->COM.com_port[1].fault_count[i]);
	}
	printf("\n");

	printf("kkkkkkkkkkkkkkkk\n");
	for(i=0; i < (myData->COM.config.countMeter[1] * ch_per_meter); i++) {
		if(myData->COM.com_port[1].fault_count[i] > 0) {
			printf("%ld(%d)\n ",
			myData->COM.com_port[1].value[i],
			myData->COM.com_port[1].fault_count[i]);

			memset(fileName, 0, sizeof(fileName));

			sprintf(fileName, "./TEMP_fault_list");
			if((fp = fopen(fileName, "w")) == NULL) {
				printf("Can not open\n");
				break;
			}
			fprintf(fp, "temp_count ");
			fprintf(fp, "%d : %ld(%d)\n ",
				i,
				myData->COM.com_port[1].value[i],
				myData->COM.com_port[1].fault_count[i]);
			fprintf(fp,"\n");
			fclose(fp);
		}
	}
	printf("kkkkkkkkkkkkkkkk\n");
	/*
	for(i=0; i < (myData->COM.config.countMeter[1] * ch_per_meter); i++) {
		printf("%ld ", myData->COM.com_port[1].value[i]);
	}*/
	printf("\n");

	printf("\n");
}

void Message_Print(void)
{
	unsigned char ch;
	int msg, fromPs, write_idx;

	ch = 0;
	printf("Message\n");

	printf("[0] : %ld, [1] : %ld, [2] : %ld, [3] : %ld, [4] : %ld, [5] : %ld, [6] : %ld
			[7] : %ld, [8] : %ld, [9] : %ld, [10] : %ld\n ", 
			myData->test_val_l[0], myData->test_val_l[1], myData->test_val_l[2], 
			myData->test_val_l[3], myData->test_val_l[4], myData->test_val_l[5], 
			myData->test_val_l[6], myData->test_val_l[7], myData->test_val_l[8], 
			myData->test_val_l[9], myData->test_val_l[10]);
	fromPs = MODULE_TO_APP;
	write_idx = myData->msg[fromPs].write_idx;
	printf("write_idx %d, read_idx %d, send_msg %d\n",
		write_idx, myData->msg[fromPs].read_idx,
		myData->msg[fromPs].msg_val[write_idx].msg);

	msg = 0;
	printf("save send_flag %d\n",
		myData->save_msg[msg].send_flag);
	printf("save count %d, write_idx %d, read_idx %d\n",
		myData->save_msg[msg].total_count,
		myData->save_msg[msg].write_idx,
		myData->save_msg[msg].read_idx);

	msg = 1;
	printf("save send_flag %d\n",
		myData->save_msg[msg].send_flag);
	printf("save count %d, write_idx %d, read_idx %d\n",
		myData->save_msg[msg].total_count,
		myData->save_msg[msg].write_idx,
		myData->save_msg[msg].read_idx);
#ifdef __10MS__	
	printf("=================10ms_start==============\n");
	ch = 0;
	printf("myData->cData[ch].op.resultIndex = %ld", myData->cData[0].op.resultIndex);
	printf("M_SIG_DATA_SAVE_ERROR = %d,  phase = %d\n", 
		myData->mData.signal[M_SIG_DATA_SAVE_ERROR],
		myData->mData.signal[M_SIG_DATA_SAVE_ERROR_PHASE]);
	printf("ch = %d\n", ch+1);
	printf("msg_count %d, send_msg_count %d, response_msg_count %d\n",
		myData->signal_10ms[ch].msg_count,
		myData->signal_10ms[ch].send_msg_count,
		myData->signal_10ms[ch].response_msg_count);
	msg = 0;
	printf("msg == 0\n");
	printf("save send_flag %d\n",
		myData->save_msg_10ms[ch][msg].send_flag);
	printf("save count %d, write_idx %d, read_idx %d\n",
		myData->save_msg_10ms[ch][msg].total_count,
		myData->save_msg_10ms[ch][msg].write_idx,
		myData->save_msg_10ms[ch][msg].read_idx);
	printf("count_100 %d\n", myData->save_msg_10ms[ch][msg].count_100);

	msg = 1;
	printf("msg == 1\n");
	printf("save send_flag %d\n",
		myData->save_msg_10ms[ch][msg].send_flag);
	printf("save count %d, write_idx %d, read_idx %d\n",
		myData->save_msg_10ms[ch][msg].total_count,
		myData->save_msg_10ms[ch][msg].write_idx,
		myData->save_msg_10ms[ch][msg].read_idx);
	printf("count_100 %d\n", myData->save_msg_10ms[ch][msg].count_100);
	msg = 2;
	printf("msg == 2\n");
	printf("save send_flag %d\n",
		myData->save_msg_10ms[ch][msg].send_flag);
	printf("save count %d, write_idx %d, read_idx %d\n",
		myData->save_msg_10ms[ch][msg].total_count,
		myData->save_msg_10ms[ch][msg].write_idx,
		myData->save_msg_10ms[ch][msg].read_idx);
	printf("count_100 %d\n", myData->save_msg_10ms[ch][msg].count_100);
	printf("=================10ms_end==============\n");
#endif
	printf("\n");
}

void COM_Print(void)
{
	int comPort, i;

	i = 0;
	printf("Load_Process_COM ");
	for(i=0; i < 12; i++) {
		printf("%d ", myData->AppControl.misc.Load_Process_COM[i]);
	}
	printf("\n");

	for(comPort=0; comPort < 5; comPort++) {
		printf("Type:%d, Model:%d, port:%d\n",
			myData->COM.config.functionType[comPort],
			myData->COM.config.functionModel[comPort],
			myData->COM.config.comPortId[comPort]
			);
/*		printf("com:%d rcvPacket:", comPort);
		for(i=0; i < 64; i++) {
			printf("%02x ", (unsigned char)myData->COM.com_port[comPort]
				.rcvPacket.rcvPacketBuf[i]);
		}
		printf("\n");

		printf("com:%d cmdBuf:", comPort);
		for(i=0; i < 64; i++) {
			//if(myData->COM.com_port[comPort].rcvCmd.cmd[i] == 0x2B)
			//	printf("\n");
			printf("%02x ",
				(unsigned char)myData->COM.com_port[comPort].rcvCmd.cmdBuf[i]);
		}
		printf("\n");

		printf("com:%d cmd:", comPort);
		for(i=0; i < 64; i++) {
			printf("%02x ",
				(unsigned char)myData->COM.com_port[comPort].rcvCmd.cmd[i]);
		}
		printf("\n");*/

/*		printf("rcvCount %d, parseCount %d\n",
			myData->COM.com_port[comPort].rcvPacket.rcvCount,
			myData->COM.com_port[comPort].rcvPacket.parseCount);
		printf("cmdBufSize %d, cmdSize %d\n",
			myData->COM.com_port[comPort].rcvCmd.cmdBufSize,
			myData->COM.com_port[comPort].rcvCmd.cmdSize);*/
/*
		if(comPort == 0) {
			printf("signal[REQUEST_PHASE]:%d, READ_COUNT:%d, RETRY_COUNT:%d\n",
				myData->COM.com_port[comPort].signal[COM_SIG_BCR_REQUEST_PHASE],
				myData->COM.com_port[comPort].signal[COM_SIG_BCR_READ_COUNT],
				myData->COM.com_port[comPort].signal[COM_SIG_BCR_RETRY_COUNT]);
			printf("\n");

			printf("com:%d tray_id[0]:", comPort);
			for(i=0; i < COM_BCR_SIZE; i++) {
				if(myData->COM.com_port[comPort].rcvCmd.cmd[i] == 0x2B)
					printf("\n");
				printf("%c ", (unsigned char)myData->COM.com_port[comPort]
					.misc.tmp_tray_id[0][i]);
			}
			printf("\n");

			printf("com:%d tray_id[1]:", comPort);
			for(i=0; i < COM_BCR_SIZE; i++) {
				if(myData->COM.com_port[comPort].rcvCmd.cmd[i] == 0x2B)
					printf("\n");
				printf("%c ", (unsigned char)myData->COM.com_port[comPort]
					.misc.tmp_tray_id[1][i]);
			}
			printf("\n");

			printf("com:%d tray_id[2]:", comPort);
			for(i=0; i < COM_BCR_SIZE; i++) {
				if(myData->COM.com_port[comPort].rcvCmd.cmd[i] == 0x2B)
					printf("\n");
				printf("%c ", (unsigned char)myData->COM.com_port[comPort]
					.misc.tmp_tray_id[2][i]);
			}
			printf("\n");
		}*/
/*
		printf("com:%d, signal[DISPLAY]:%d, countMeter:%d\n",
			comPort,
			myData->COM.com_port[comPort].signal[COM_SIG_DISPLAY],
			myData->COM.config.countMeter[comPort]);
		printf("retry_time %d, retry_count %d\n",
			myData->COM.com_port[comPort].misc.retry_time,
			myData->COM.com_port[comPort].misc.retry_count);
		printf("singal[METER_INIT]:%d, signal[METER_REQ_PHA]:%d\n",
			myData->COM.com_port[comPort].signal[COM_SIG_CALI_METER_INITIALIZE],
			myData->COM.com_port[comPort]
				.signal[COM_SIG_CALI_METER_REQUEST_PHASE]);*/

		printf("plc_idle_time %ld\n",
			myData->COM.com_port[comPort].misc.plc_idle_time);
		printf("signal[PLC_REQUEST_PHASE]:%d\n",
			myData->COM.com_port[comPort].signal[COM_SIG_PLC_REQUEST_PHASE]);
		printf("\n");
	}
	printf("\n");
}

void CAN_Print(void)
{
	int i, j, ch;
//	long tmp;

	ch = 0;

	printf("canReceiveDataCount %d %d\n",
		myData->canReceiveDataCount[ch][0], myData->canReceiveDataCount[ch][1]);
	printf("canTransmitDataCount %d %d\n",
		myData->canTransmitDataCount[ch][0],
		myData->canTransmitDataCount[ch][1]);

	printf("val1 ");
	for(i=0; i < 15; i++) {
		printf("%f ", myData->CanData[ch][i].f_val[0]);
	}
	printf("\n");

	printf("val2\n");
	j = 0;
	for(i=20; i < 40; i++) {
		printf("%x ", (unsigned int)myData->CanData[ch][i].ul_val[0]);
		j++;
		if((j % 2) == 0) printf("\n");
	}
	printf("\n");

/*
	for(i=20; i < 23; i++) {
		printf("%d canType:%d, canID:%ld, startBit:%d, bitCount:%d, factor(ud):%ld, factor(ld):%ld, factor(f):%f, func_div:%d\n", i,
			myData->canReceiveSetData.normalData[ch][i].canType,
			myData->canReceiveSetData.normalData[ch][i].canID,
			myData->canReceiveSetData.normalData[ch][i].startBit,
			myData->canReceiveSetData.normalData[ch][i].bitCount,
			(unsigned long)myData->canReceiveSetData.normalData[ch][i].factor,
			(long)myData->canReceiveSetData.normalData[ch][i].factor,
			myData->canReceiveSetData.normalData[ch][i].factor,
			myData->canReceiveSetData.normalData[ch][i].function_div);
		printf("value_ul(%d) %ld %ld\n", i, 
			myData->CanData[ch][i].ul_val[0],
			myData->CanData[ch][i].ul_val[1]);
		//printf("value_l(%d) %ld %ld\n", i, 
		//	myData->CanData[ch][i].l_val[0],
		//	myData->CanData[ch][i].l_val[1]);
		printf("value_f(%d) %f %f\n", i, 
			myData->CanData[ch][i].f_val[0],
			myData->CanData[ch][i].f_val[1]);
		tmp = (long)myData->CanData[ch][i].f_val[0];
		//printf("tmp %ld %d\n", tmp, (unsigned char)tmp);
		printf("value_c(%d) %02x %02x %02x %02x %02x %02x %02x %02x\n", i, 
			myData->CanData[ch][i].c_val[0],
			myData->CanData[ch][i].c_val[1],
			myData->CanData[ch][i].c_val[2],
			myData->CanData[ch][i].c_val[3],
			myData->CanData[ch][i].c_val[4],
			myData->CanData[ch][i].c_val[5],
			myData->CanData[ch][i].c_val[6],
			myData->CanData[ch][i].c_val[7]);
		printf("\n");
	}*/
/*
	j = myData->canReceiveDataCount[ch][0];
	for(i=j; i < (j+5); i++) {
		printf("%d canType:%d, canID:%ld, startBit:%d, bitCount:%d, factor(ud):%ld, factor(ld):%ld, factor(f):%f\n", i,
			myData->canReceiveSetData.normalData[ch][i].canType,
			myData->canReceiveSetData.normalData[ch][i].canID,
			myData->canReceiveSetData.normalData[ch][i].startBit,
			myData->canReceiveSetData.normalData[ch][i].bitCount,
			(unsigned long)myData->canReceiveSetData.normalData[ch][i].factor,
			(long)myData->canReceiveSetData.normalData[ch][i].factor,
			myData->canReceiveSetData.normalData[ch][i].factor);
		printf("value_ul(%d) %ld %ld\n", i, 
			myData->CanData[ch][i].ul_val[0],
			myData->CanData[ch][i].ul_val[1]);
		printf("value_l(%d) %ld %ld\n", i, 
			myData->CanData[ch][i].l_val[0],
			myData->CanData[ch][i].l_val[1]);
		printf("value_f(%d) %f %f\n", i, 
			myData->CanData[ch][i].f_val[0],
			myData->CanData[ch][i].f_val[1]);
		printf("value_c(%d) %02x %02x %02x %02x %02x %02x %02x %02x\n", i, 
			myData->CanData[ch][i].c_val[0],
			myData->CanData[ch][i].c_val[1],
			myData->CanData[ch][i].c_val[2],
			myData->CanData[ch][i].c_val[3],
			myData->CanData[ch][i].c_val[4],
			myData->CanData[ch][i].c_val[5],
			myData->CanData[ch][i].c_val[6],
			myData->CanData[ch][i].c_val[7]);
		printf("\n");
	}
*/
/*	for(i=0; i < 20; i++) {
		printf("%d %ld %ld\n", i, myData->canTransmitTime[ch][i],
			myData->canTransmitSetData.normalData[ch][i].send_period);
	}*/

	printf("comm_time ");
	for(i=0; i < myData->mData.config.installedCAN; i++) {
		printf("(%ld, %ld) %ld", myData->CAN.receive_time[i],
			myData->CAN.transmit_time[i], myData->mData.misc.timer_1sec);
	}
	printf("\n");

	printf("signal ");
	for(i=0; i < myData->mData.config.installedCAN; i++) {
		printf("%d ",
			myData->CAN.signal[i][CAN_SIG_ABORT_TRANSMISSION]);
	}
	printf("\n");
/*	printf("signal %d %d %d\n",
		myData->dio.signal[DIO_SIG_IN_EXTERNAL1],
		myData->CAN.signal[ch][CAN_SIG_START_TRANSMISSION],
		myData->CAN.signal[ch][CAN_SIG_TX_PHASE_INV_CAP_V]);
	printf("signal %d %d %d\n",
		myData->CAN.signal[ch][CAN_SIG_TX_PHASE_MCU_TEMP],
		myData->CAN.signal[ch][CAN_SIG_RX_PHASE_FAN_SPEED],
		myData->CAN.signal[ch][CAN_SIG_FAN_TEST]);
	printf("signal %d %d %d\n",
		myData->CAN.signal[ch][CAN_SIG_ECU_ID_REQUEST],
		myData->CAN.signal[ch][CAN_SIG_TX_PHASE_ECU_ID],
		myData->CAN.signal[ch][CAN_SIG_RX_PHASE_ECU_ID]);
	printf("signal daq_mux %d %d %d %d\n",
		myData->CAN.signal[0][CAN_SIG_TX_PHASE_DAQ_MUX],
		myData->CAN.signal[1][CAN_SIG_TX_PHASE_DAQ_MUX],
		myData->CAN.signal[2][CAN_SIG_TX_PHASE_DAQ_MUX],
		myData->CAN.signal[3][CAN_SIG_TX_PHASE_DAQ_MUX]);
	printf("signal charger_phase %d %d %d\n",
		myData->CAN.signal[ch][CAN_SIG_CHARGER_PHASE],
		myData->dio.signal[DIO_SIG_IN_CHARGER_WAKEUP],
		myData->dio.signal[DIO_SIG_IN_CHARGER_PHASE]);
	printf("signal %d %d %d\n",
		myData->CAN.signal[ch][CAN_SIG_TX_PHASE_DEBUG_ON],
		myData->CAN.signal[ch][CAN_SIG_TX_PHASE_CALIBRATION],
		myData->CAN.signal[ch][CAN_SIG_TX_PHASE_FLASH_CMD]);*/
	printf("signal %d %d %d, %d %d %d, %d %d %d, %d %d %d\n",
		myData->CAN.signal[0][CAN_SIG_TX_PHASE_TEST1],
		myData->CAN.signal[0][CAN_SIG_TX_PHASE_OP_TYPE],
		myData->CAN.signal[0][CAN_SIG_TX_PHASE_WAKE_UP],
		myData->CAN.signal[1][CAN_SIG_TX_PHASE_TEST1],
		myData->CAN.signal[1][CAN_SIG_TX_PHASE_OP_TYPE],
		myData->CAN.signal[1][CAN_SIG_TX_PHASE_WAKE_UP],
		myData->CAN.signal[2][CAN_SIG_TX_PHASE_TEST1],
		myData->CAN.signal[2][CAN_SIG_TX_PHASE_OP_TYPE],
		myData->CAN.signal[2][CAN_SIG_TX_PHASE_WAKE_UP],
		myData->CAN.signal[3][CAN_SIG_TX_PHASE_TEST1],
		myData->CAN.signal[3][CAN_SIG_TX_PHASE_OP_TYPE],
		myData->CAN.signal[3][CAN_SIG_TX_PHASE_WAKE_UP]);

	for(i=0; i < myData->mData.config.installedCAN; i++) {
		printf("uds_rx[%d] tx_uds_phase(%d, %d) %x, %x, %x, %x, %x, %x, %x, %x\n", i,
			myData->CAN.signal[i][CAN_SIG_TX_PHASE_UDS_DATA1],
			myData->CAN.signal[i][CAN_SIG_TX_PHASE_UDS_DATA_INDEX],
			myData->CAN.uds_rx[i].uc_val[0], myData->CAN.uds_rx[i].uc_val[1],
			myData->CAN.uds_rx[i].uc_val[2], myData->CAN.uds_rx[i].uc_val[3],
			myData->CAN.uds_rx[i].uc_val[4], myData->CAN.uds_rx[i].uc_val[5],
			myData->CAN.uds_rx[i].uc_val[6], myData->CAN.uds_rx[i].uc_val[7]);
	}

	for(i=0; i < myData->mData.config.installedCAN; i++) {
		printf("%d main_on %d, main_off %d\n", i,
			myData->CAN.signal[i][CAN_SIG_TX_PHASE_MAIN_ON],
			myData->CAN.signal[i][CAN_SIG_TX_PHASE_MAIN_OFF]);
	}

	printf("\n");
}

void Watt_limited(void)
{
}

void Chamber_Print(void)
{
	int i;

	for(i=0; i < MAX_COM_PORT; i++) {
		if(myData->COM.config.functionType[i] == COM_FUNC_TYPE_CHAMBER1) {
			break;
		}
	}
	if(i >= MAX_COM_PORT) return;

	printf("TEMP_NPV %d, TEMP_NSP %d, NOWSTS %x,OTHERSTS %x\n",
		myData->COM.com_port[i].misc.TEMP_NPV,
		myData->COM.com_port[i].misc.TEMP_NSP,
		myData->COM.com_port[i].misc.NOWSTS,
		myData->COM.com_port[i].misc.OTHERSTS);
	printf("PROC_TIME_H %d, PROC_TIME_L %d\n",
		myData->COM.com_port[i].misc.PROC_TIME_H,
		myData->COM.com_port[i].misc.PROC_TIME_L);

	printf("\n");
}
void Shin_Print(void) { //shh_test

	//int i = 0;
/*
	printf("FAN_MC_FAIL :%d\n", Read_InPoint(0, 1, I_FAN_MC_FAIL));
	printf("FAN_ERROR1 :%d\n", Read_InPoint(0, 1, I_FAN_ERROR));
	printf("FAN_ERROR2 :%d\n",  Read_InPoint(0, 2, I_FAN_ERROR));
	printf("I_CONVERTER_OT: %d\n", Read_InPoint(0, 2, I_CONVERTER_OT));
	printf("\n");
*/
	printf("SOC4 : %ld\n",
	myData->testCond[0].local_object[4][IDX_LOC_OBJ_END_RPT_SOC]);
	printf("TYPE4 : %ld\n",
	myData->testCond[0].local_object[4][IDX_LOC_OBJ_TYPE]);
	
	printf("SOC5 : %ld\n",
	myData->testCond[0].local_object[5][IDX_LOC_OBJ_END_RPT_SOC]);
	printf("TYPE5 : %ld\n",
	myData->testCond[0].local_object[5][IDX_LOC_OBJ_TYPE]);

	/*for(i = 0; i < 1; i++) {
		printf(" i val: %d\n",i);		
	}*/
		
}
//shhw_240611s	
long makePerTime(long time) {

 long timePerVal;		
 double time_std; //1ms tic

 time_std = 1000000.0; //1ms tic

 timePerVal =  ((double)time / time_std) * 100;

 return timePerVal;
 
}
//shhw_240611e
int data_10ms_write(int scan_ch, int div)
{
	char fileName[256];
	int i;
	FILE *fp;

	memset(fileName, 0, sizeof(fileName));
	if(div == 0) {
		sprintf(fileName, "./data_10ms_data_%d.csv", scan_ch+1);
	} else if(div == 1) {
		sprintf(fileName, "./data_10ms_data_tmp_%d.csv", scan_ch+1);
	} else if(div == 2) {
		sprintf(fileName, "./data_10ms_data_d_%d.csv", scan_ch+1);
	} else {
		sprintf(fileName, "./data_10ms_data_tmp_d_%d.csv", scan_ch+1);
	}

	if((fp = fopen(fileName, "w")) == NULL) {
		if(div == 0) {
			printf("Can not open 10ms_data file(write)\n");
		} else if(div == 1) {
			printf("Can not open 10ms_data_tmp file(write)\n");
		} else if(div == 2) {
			printf("Can not open 10ms_data_d file(write)\n");
		} else {
			printf("Can not open 10ms_data_tmp_d file(write)\n");
		}
		return -1;
	}

	fprintf(fp, "t, v, i, w\n");
	if(div == 0) {
		for(i=0; i < myData->data_10ms_count[scan_ch]; i++) {
			fprintf(fp, "%ld, %ld, %ld, %ld\n",
				myData->data_10ms[scan_ch][0][i],
				myData->data_10ms[scan_ch][1][i],
				myData->data_10ms[scan_ch][2][i],
				myData->data_10ms[scan_ch][3][i]);
		}
	} else if(div == 1) {
		for(i=0; i < myData->data_10ms_count[scan_ch]; i++) {
			fprintf(fp, "%ld, %ld, %ld, %ld\n",
				myData->data_10ms_tmp[scan_ch][0][i],
				myData->data_10ms_tmp[scan_ch][1][i],
				myData->data_10ms_tmp[scan_ch][2][i],
				myData->data_10ms_tmp[scan_ch][3][i]);
		}
	} /*else if(div == 2) {
		for(i=0; i < myData->data_10ms_count_d[scan_ch]; i++) {
			fprintf(fp, "%ld, %ld, %ld, %ld\n",
				myData->data_10ms_d[scan_ch][0][i],
				myData->data_10ms_d[scan_ch][1][i],
				myData->data_10ms_d[scan_ch][2][i],
				myData->data_10ms_d[scan_ch][3][i]);
		}
	} else {
		for(i=0; i < myData->data_10ms_count_d[scan_ch]; i++) {
			fprintf(fp, "%ld, %ld, %ld, %ld\n",
				myData->data_10ms_tmp_d[scan_ch][0][i],
				myData->data_10ms_tmp_d[scan_ch][1][i],
				myData->data_10ms_tmp_d[scan_ch][2][i],
				myData->data_10ms_tmp_d[scan_ch][3][i]);
		}
	}*/

	fclose(fp);

	return 0;
}

int COA_Client_TestCond_write_V1009(int div)
{/*
	char fileName[256];
	int i,j,k = 0;
	FILE *fp;

	memset(fileName, 0, sizeof(fileName));

	if(div == 0) {
		sprintf(fileName, "./COA1_Client_Receive_Data_V1009");
//	} else if(div == 1) {
//		sprintf(fileName, "./data_10ms_data_tmp_%d.csv", scan_ch+1);
//	} else if(div == 2) {
//		sprintf(fileName, "./data_10ms_data_d_%d.csv", scan_ch+1); 
	} else {
		sprintf(fileName, "./Pack_Client_Receive_Data_V1009");
	}

	if((fp = fopen(fileName, "w")) == NULL) {
		if(div == 0) {
			printf("Can not open COA1_Client_Receive_Data file(write)\n");
//		} else if(div == 1) {
//			printf("Can not open 10ms_data_tmp file(write)\n");
//		} else if(div == 2) {
//			printf("Can not open 10ms_data_d file(write)\n"); 
		} else {
			printf("Can not open Pack_Client_Receive_Data file(write)\n");
		}
		return -1;
	}

//	fprintf(fp, "t, v, i, w\n");
	
	if(div == 0) {
		fprintf(fp, "COA1_Client_Receive_Data_V1009 \n");
		fprintf(fp,"\n");
		
		fprintf(fp, "Test_Cond_Header \n");
		fprintf(fp, "totalStep	:	%d \n", 
				myData->COA_Client[div].testCond.header.totalStep);
		fprintf(fp, "reserverd1[3]	:	%d, %d, %d \n", 
				myData->COA_Client[div].testCond.header.reserved1[0],
				myData->COA_Client[div].testCond.header.reserved1[1],
				myData->COA_Client[div].testCond.header.reserved1[2]);
		fprintf(fp, "reserverd2[2]	:	%ld, %ld \n", 
				myData->COA_Client[div].testCond.header.reserved2[0],
				myData->COA_Client[div].testCond.header.reserved2[1]);
		printf("\n");
		
		fprintf(fp, "Test_Cond_Safety \n");
		fprintf(fp, "faultLowerV	:	%ld \n", 
				myData->COA_Client[div].testCond.safety.faultLowerV);
		fprintf(fp, "faultUpperV	:	%ld \n", 
				myData->COA_Client[div].testCond.safety.faultUpperV);
		fprintf(fp, "faultLowerI	:	%ld \n", 
				myData->COA_Client[div].testCond.safety.faultLowerI);
		fprintf(fp, "faultUpperI	:	%ld \n", 
				myData->COA_Client[div].testCond.safety.faultUpperI);
		fprintf(fp, "faultLower_AmpareHour	:	%ld \n", 
				myData->COA_Client[div].testCond.safety.faultLower_AmpareHour);
		fprintf(fp, "faultUpper_AmpareHour	:	%ld \n", 
				myData->COA_Client[div].testCond.safety.faultUpper_AmpareHour);
		fprintf(fp, "faultLowerTemp	:	%ld \n", 
				myData->COA_Client[div].testCond.safety.faultLowerTemp);
		fprintf(fp, "faultUpperTemp	:	%ld \n", 
				myData->COA_Client[div].testCond.safety.faultUpperTemp);
		fprintf(fp, "reserved1[4]	:	%ld, %ld, %ld, %ld \n", 
				myData->COA_Client[div].testCond.safety.reserved1[0],
				myData->COA_Client[div].testCond.safety.reserved1[1],
				myData->COA_Client[div].testCond.safety.reserved1[2],
				myData->COA_Client[div].testCond.safety.reserved1[3]);
		fprintf(fp,"\n");

		fprintf(fp, "Test_Cond_Step \n");
		fprintf(fp,"\n");
		for(i=0; i < myData->COA_Client[div].testCond.header.totalStep; i++){
			fprintf(fp, "Step %d \n",i+1);
			fprintf(fp, "Step %d Header \n",i+1);
			fprintf(fp, "type	:	%d \n",
				myData->COA_Client[div].testCond.step[i].header.type);
			fprintf(fp, "stepNo	:	%d \n",
				myData->COA_Client[div].testCond.step[i].header.stepNo);
			fprintf(fp, "mode	:	%d \n",
				myData->COA_Client[div].testCond.step[i].header.mode);
			fprintf(fp, "testEnd	:	%d \n",
				myData->COA_Client[div].testCond.step[i].header.testEnd);
			fprintf(fp, "subStep	:	%d \n",
				myData->COA_Client[div].testCond.step[i].header.subStep);

			fprintf(fp, "reserved1	:	%d, \n", 
				myData->COA_Client[div].testCond.step[i].header.reserved1);
			fprintf(fp, "patternIndex	:	%d \n",
				myData->COA_Client[div].testCond.step[i].header.patternIndex);
			fprintf(fp, "reserved2[2]	:	%d, %d \n", 
				myData->COA_Client[div].testCond.step[i].header.reserved2[0],
				myData->COA_Client[div].testCond.step[i].header.reserved2[1]);
		fprintf(fp,"\n");
			j = 0;
//			for(j=0;j < MAX_P1_SUB_STEP ; j++){
				fprintf(fp, "Step Reference[%d][%d] \n",i,j);
				fprintf(fp, "refV	:	%ld \n",
					myData->COA_Client[div].testCond.step[i].reference[j].refV);
				fprintf(fp, "refI	:	%ld \n",
					myData->COA_Client[div].testCond.step[i].reference[j].refI);
				fprintf(fp, "rangeI	:	%d \n",
					myData->COA_Client[div].testCond.step[i].reference[j].rangeI);
				fprintf(fp, "ValueRate_Compare	:	%d \n",
					myData->COA_Client[div].testCond.step[i].reference[j].ValueRate_Compare);
				fprintf(fp, "ValueRate	:	%d \n",
					myData->COA_Client[div].testCond.step[i].reference[j].ValueRate);
				fprintf(fp, "Time	:	%ld \n",
					myData->COA_Client[div].testCond.step[i].reference[j].Time);
				fprintf(fp, "V_Upper	:	%ld \n",
					myData->COA_Client[div].testCond.step[i].reference[j].V_Upper);
				fprintf(fp, "Current	:	%ld \n",
					myData->COA_Client[div].testCond.step[i].reference[j].Current);
				fprintf(fp, "AmpareHour_SumAmpareHour	:	%ld \n",
					myData->COA_Client[div].testCond.step[i].reference[j].AmpareHour_SumAmpareHour);
				fprintf(fp, "V_Upper_Branch_CycleCount_Branch	:	%ld \n",
					myData->COA_Client[div].testCond.step[i].reference[j].V_Upper_Branch_CycleCount_Branch);
				fprintf(fp, "V_Lower_Branch_CycleCount	:	%ld \n",
					myData->COA_Client[div].testCond.step[i].reference[j].V_Lower_Branch_CycleCount);
				fprintf(fp, "DeltaV	:	%ld \n",
					myData->COA_Client[div].testCond.step[i].reference[j].DeltaV);
				fprintf(fp, "Power	:	%ld \n",
					myData->COA_Client[div].testCond.step[i].reference[j].Power);
				fprintf(fp, "WattHour_SumWattHour	:	%ld \n",
					myData->COA_Client[div].testCond.step[i].reference[j].WattHour_SumWattHour);
				fprintf(fp, "Time_Branch_AccCycleCount	:	%ld \n",
					myData->COA_Client[div].testCond.step[i].reference[j].Time_Branch_AccCycleCount);
				fprintf(fp, "CVTime_Branch_AccCycleCountId	:	%ld \n",
					myData->COA_Client[div].testCond.step[i].reference[j].CVTime_Branch_AccCycleCountId);
				fprintf(fp, "CVTime	:	%ld \n",
					myData->COA_Client[div].testCond.step[i].reference[j].CVTime);
				fprintf(fp, "Aux_V_Upper	:	%ld \n",
					myData->COA_Client[div].testCond.step[i].reference[j].Aux_V_Upper);
				fprintf(fp, "Aux_V_Lower	:	%ld \n",
					myData->COA_Client[div].testCond.step[i].reference[j].Aux_V_Lower);
				fprintf(fp, "Aux_Temp_Upper	:	%ld \n",
					myData->COA_Client[div].testCond.step[i].reference[j].Aux_Temp_Upper);
				fprintf(fp, "Aux_Temp_Lower	:	%ld \n",
					myData->COA_Client[div].testCond.step[i].reference[j].Aux_Temp_Lower);
				fprintf(fp, "Aux_V_Upper_Branch	:	%d \n",
					myData->COA_Client[div].testCond.step[i].reference[j].Aux_V_Upper_Branch);
				fprintf(fp, "Aux_V_Lower_Branch	:	%d \n",
					myData->COA_Client[div].testCond.step[i].reference[j].Aux_V_Lower_Branch);
				fprintf(fp, "Aux_Temp_Upper_Branch	:	%d \n",
					myData->COA_Client[div].testCond.step[i].reference[j].Aux_Temp_Upper_Branch);
				fprintf(fp, "Aux_Temp_Lower_Branch	:	%d \n",
					myData->COA_Client[div].testCond.step[i].reference[j].Aux_Temp_Lower_Branch);
				fprintf(fp, "CAN_V_Upper	:	%ld \n",
					myData->COA_Client[div].testCond.step[i].reference[j].CAN_V_Upper);
				fprintf(fp, "CAN_V_Lower	:	%ld \n",
					myData->COA_Client[div].testCond.step[i].reference[j].CAN_V_Lower);
				fprintf(fp, "CAN_Temp_Upper	:	%ld \n",
					myData->COA_Client[div].testCond.step[i].reference[j].CAN_Temp_Upper);
				fprintf(fp, "CAN_Temp_Lower	:	%ld \n",
					myData->COA_Client[div].testCond.step[i].reference[j].CAN_Temp_Lower);
				fprintf(fp, "CAN_Soc_Upper	:	%ld \n",
					myData->COA_Client[div].testCond.step[i].reference[j].CAN_Soc_Upper);
				fprintf(fp, "CAN_Soc_Lower	:	%ld \n",
					myData->COA_Client[div].testCond.step[i].reference[j].CAN_Soc_Lower);
				fprintf(fp, "CAN_Fault	:	%ld \n",
					myData->COA_Client[div].testCond.step[i].reference[j].CAN_Fault);
				fprintf(fp, "CAN_V_Upper_Branch	:	%d \n",
					myData->COA_Client[div].testCond.step[i].reference[j].CAN_V_Upper_Branch);
				fprintf(fp, "CAN_V_Lower_Branch	:	%d \n",
					myData->COA_Client[div].testCond.step[i].reference[j].CAN_V_Lower_Branch);
				fprintf(fp, "CAN_Temp_Upper_Branch	:	%d \n",
					myData->COA_Client[div].testCond.step[i].reference[j].CAN_Temp_Upper_Branch);
				fprintf(fp, "CAN_Temp_Lower_Branch	:	%d \n",
					myData->COA_Client[div].testCond.step[i].reference[j].CAN_Temp_Lower_Branch);
				fprintf(fp, "CAN_Soc_Upper_Branch	:	%d \n",
					myData->COA_Client[div].testCond.step[i].reference[j].CAN_Soc_Upper_Branch);
				fprintf(fp, "CAN_Soc_Lower_Branch	:	%d \n",
					myData->COA_Client[div].testCond.step[i].reference[j].CAN_Soc_Lower_Branch);
				fprintf(fp, "CAN_Fault_Branch	:	%d \n",
					myData->COA_Client[div].testCond.step[i].reference[j].CAN_Fault_Branch);
				fprintf(fp, "ValueRate_Item	:	%d \n",
					myData->COA_Client[div].testCond.step[i].reference[j].ValueRate_Item);
				fprintf(fp, "V_Lower_MultiCycleCount	:	%ld \n",
					myData->COA_Client[div].testCond.step[i].reference[j].V_Lower_MultiCycleCount);
				fprintf(fp, "AmpareHour_Branch_MultiCycleCountId	:	%d \n",
					myData->COA_Client[div].testCond.step[i].reference[j].AmpareHour_Branch_MultiCycleCountId);
				fprintf(fp, "WattHour_Branch_MultiCycleCount_Branch	:	%d \n",
					myData->COA_Client[div].testCond.step[i].reference[j].WattHour_Branch_MultiCycleCount_Branch);
				fprintf(fp, "ValueRate_Branch	:	%d \n",
					myData->COA_Client[div].testCond.step[i].reference[j].ValueRate_Branch);
				fprintf(fp, "AccCycleCount_Branch	:	%d \n",
					myData->COA_Client[div].testCond.step[i].reference[j].AccCycleCount_Branch);
				fprintf(fp, "reserved2[2]	:	%ld, %ld \n",
					myData->COA_Client[div].testCond.step[i].reference[j].reserved2[0],
					myData->COA_Client[div].testCond.step[i].reference[j].reserved2[1]);
		fprintf(fp,"\n");
//			}

			for(j=0;j < MAX_P1_COMP_POINT ; j++){
				fprintf(fp, "Step CompV[%d][%d] \n",i,j);
				fprintf(fp, "lowerValue	:	%ld \n",
					myData->COA_Client[div].testCond.step[i].compV[j].lowerValue);
				fprintf(fp, "upperValue	:	%ld \n",
					myData->COA_Client[div].testCond.step[i].compV[j].upperValue);
				fprintf(fp, "time	:	%ld \n",
					myData->COA_Client[div].testCond.step[i].compV[j].time);
		fprintf(fp,"\n");
			}

			for(j=0;j < MAX_P1_COMP_POINT ; j++){
				fprintf(fp, "Step CompI[%d][%d] \n",i,j);
				fprintf(fp, "lowerValue	:	%ld \n",
					myData->COA_Client[div].testCond.step[i].compI[j].lowerValue);
				fprintf(fp, "upperValue	:	%ld \n",
					myData->COA_Client[div].testCond.step[i].compI[j].upperValue);
				fprintf(fp, "time	:	%ld \n",
					myData->COA_Client[div].testCond.step[i].compI[j].time);
		fprintf(fp,"\n");
			}
				
			fprintf(fp, "Step %d DeltaV \n",i+1);
			fprintf(fp, "lowerValue	:	%ld \n",
				myData->COA_Client[div].testCond.step[i].deltaV.lowerValue);
			fprintf(fp, "upperValue	:	%ld \n",
				myData->COA_Client[div].testCond.step[i].deltaV.upperValue);
			fprintf(fp, "time	:	%ld \n",
				myData->COA_Client[div].testCond.step[i].deltaV.time);
		fprintf(fp,"\n");

			fprintf(fp, "Step %d DeltaI  \n",i+1);
			fprintf(fp, "lowerValue	:	%ld \n",
				myData->COA_Client[div].testCond.step[i].deltaI.lowerValue);
			fprintf(fp, "upperValue	:	%ld \n",
				myData->COA_Client[div].testCond.step[i].deltaI.upperValue);
			fprintf(fp, "time	:	%ld \n",
				myData->COA_Client[div].testCond.step[i].deltaI.time);
		fprintf(fp,"\n");

			fprintf(fp, "Step %d Record \n",i+1);
			fprintf(fp, "deltaT	:	%ld \n",
				myData->COA_Client[div].testCond.step[i].record.deltaT);
			fprintf(fp, "deltaV	:	%ld \n",
				myData->COA_Client[div].testCond.step[i].record.deltaV);
			fprintf(fp, "deltaI	:	%ld \n",
				myData->COA_Client[div].testCond.step[i].record.deltaI);
			fprintf(fp, "deltaTemp	:	%ld \n",
				myData->COA_Client[div].testCond.step[i].record.deltaTemp);
			fprintf(fp, "deltaP	:	%ld \n",
				myData->COA_Client[div].testCond.step[i].record.deltaP);
			fprintf(fp, "reserved	:	%ld \n",
				myData->COA_Client[div].testCond.step[i].record.reserved);
		fprintf(fp,"\n");
		
			fprintf(fp, "Step %d Edlc \n",i+1);
			fprintf(fp, "capacitanceV1	:	%ld \n",
				myData->COA_Client[div].testCond.step[i].edlc.capacitanceV1);
			fprintf(fp, "capacitanceV2	:	%ld \n",
				myData->COA_Client[div].testCond.step[i].edlc.capacitanceV2);
			fprintf(fp, "startT_Z	:	%ld \n",
				myData->COA_Client[div].testCond.step[i].edlc.startT_Z);
			fprintf(fp, "endT_Z	:	%ld \n",
				myData->COA_Client[div].testCond.step[i].edlc.endT_Z);
			fprintf(fp, "startT_LC	:	%ld \n",
				myData->COA_Client[div].testCond.step[i].edlc.startT_LC);
			fprintf(fp, "endT_LC	:	%ld \n",
				myData->COA_Client[div].testCond.step[i].edlc.endT_LC);
		fprintf(fp,"\n");

			j = 0;
//			for(j=0;j < MAX_P1_GRADE_ITEM ; j++){
				fprintf(fp, "Step Grade[%d][%d] \n",i,j);
				fprintf(fp, "item	:	%d \n",
					myData->COA_Client[div].testCond.step[i].grade[j].item);
				fprintf(fp, "gradeStepCount	:	%d \n",
					myData->COA_Client[div].testCond.step[i].grade[j].gradeStepCount);
				fprintf(fp, "reserved1	:	%d \n",
					myData->COA_Client[div].testCond.step[i].grade[j].reserved1);
		fprintf(fp,"\n");

				k = 0;
		//		for(k=0;k < MAX_P1_GRADE_STEP ; k++){
					fprintf(fp, "Step GradeStep[%d][%d][%d] \n",i,j,k);
					fprintf(fp, "gradeCode	:	%d \n",
					myData->COA_Client[div].testCond.step[i].grade[j].gradeStep[k].gradeCode);
					fprintf(fp, "reserved1	:	%d \n",
					myData->COA_Client[div].testCond.step[i].grade[j].gradeStep[k].reserved1);
					fprintf(fp, "reserved2	:	%d \n",
					myData->COA_Client[div].testCond.step[i].grade[j].gradeStep[k].reserved2);
					fprintf(fp, "lowerValue	:	%ld \n",
					myData->COA_Client[div].testCond.step[i].grade[j].gradeStep[k].lowerValue);
					fprintf(fp, "upperValue	:	%ld \n",
					myData->COA_Client[div].testCond.step[i].grade[j].gradeStep[k].upperValue);
		fprintf(fp,"\n");

//				}
//			}

		fprintf(fp, "Test_Cond_Step \n");
		
			fprintf(fp, "faultUpperV	:	%ld \n",
				myData->COA_Client[div].testCond.step[i].faultUpperV);
			fprintf(fp, "faultLowerV	:	%ld \n",
				myData->COA_Client[div].testCond.step[i].faultLowerV);
			fprintf(fp, "faultUpperI	:	%ld \n",
				myData->COA_Client[div].testCond.step[i].faultUpperI);
			fprintf(fp, "faultLowerI	:	%ld \n",
				myData->COA_Client[div].testCond.step[i].faultLowerI);
			fprintf(fp, "faultUpper_AmpareHour	:	%ld \n",
				myData->COA_Client[div].testCond.step[i].faultUpper_AmpareHour);
			fprintf(fp, "faultLower_AmpareHour	:	%ld \n",
				myData->COA_Client[div].testCond.step[i].faultLower_AmpareHour);
			fprintf(fp, "faultUpperZ	:	%ld \n",
				myData->COA_Client[div].testCond.step[i].faultUpperZ);
			fprintf(fp, "faultLowerZ	:	%ld \n",
				myData->COA_Client[div].testCond.step[i].faultLowerZ);
			fprintf(fp, "faultUpperTemp	:	%ld \n",
				myData->COA_Client[div].testCond.step[i].faultUpperTemp);
			fprintf(fp, "faultLowerTemp	:	%ld \n",
				myData->COA_Client[div].testCond.step[i].faultLowerTemp);
			fprintf(fp, "reserved1[2]	:	%ld, %ld \n",
				myData->COA_Client[div].testCond.step[i].reserved1[0],
				myData->COA_Client[div].testCond.step[i].reserved1[1]);
		fprintf(fp,"\n");
		}
		
		fprintf(fp, "Test_Cond \n");
		fprintf(fp, "stepCount	:	%d \n",
			myData->COA_Client[div].testCond.stepCount);
		fprintf(fp, "reserved1	:	%d \n",
			myData->COA_Client[div].testCond.reserved1);
	}

	fclose(fp);
*/
	return 0;
}

int COA_Client_TestCond_write_V100B(int div)
{
#ifdef __COA_VER_100B__
	char fileName[256];
	int i,j,k = 0;
	FILE *fp;

	memset(fileName, 0, sizeof(fileName));

	sprintf(fileName, "./COA%d_Client_TestCond_V100B", div+1);

	if((fp = fopen(fileName, "w")) == NULL) {
		printf("Can not open COA%d_Client_TestCond file(write)\n", div+1);
		return -1;
	}

	fprintf(fp, "COA%d_Client_Receive_Data_V100B\n", div+1);
	fprintf(fp,"\n");
		
	fprintf(fp, "TestCond_Header\n");
	fprintf(fp, "totalStep : %d\n", 
		myData->COA_Client[div].testCond.header.totalStep);
	fprintf(fp, "reserverd1[3] : %d, %d, %d\n", 
		myData->COA_Client[div].testCond.header.reserved1[0],
		myData->COA_Client[div].testCond.header.reserved1[1],
		myData->COA_Client[div].testCond.header.reserved1[2]);
	fprintf(fp, "reserverd2[2] : %ld, %ld\n", 
		myData->COA_Client[div].testCond.header.reserved2[0],
		myData->COA_Client[div].testCond.header.reserved2[1]);
	fprintf(fp, "\n");
		
	fprintf(fp, "Common_Safety\n");
	fprintf(fp, "faultLowerV : %ld\n", 
		myData->COA_Client[div].testCond.safety.faultLowerV);
	fprintf(fp, "faultUpperV : %ld\n", 
		myData->COA_Client[div].testCond.safety.faultUpperV);
	fprintf(fp, "reserved1 : %ld\n", 
		myData->COA_Client[div].testCond.safety.reserved1);
	fprintf(fp, "faultUpperI : %ld\n", 
		myData->COA_Client[div].testCond.safety.faultUpperI);
	fprintf(fp, "faultLowerTemp : %ld\n", 
		myData->COA_Client[div].testCond.safety.faultLowerTemp);
	fprintf(fp, "faultUpperTemp : %ld\n", 
		myData->COA_Client[div].testCond.safety.faultUpperTemp);
	fprintf(fp, "faultUpper_AmpareHour : %ld\n", 
		myData->COA_Client[div].testCond.safety.faultUpper_AmpareHour);
	fprintf(fp, "faultUpperP : %ld\n", 
		myData->COA_Client[div].testCond.safety.faultUpperP);
	fprintf(fp, "faultUpper_WattHour : %ld\n", 
		myData->COA_Client[div].testCond.safety.faultUpper_WattHour);
	fprintf(fp,"\n");

	fprintf(fp, "CAN_Common_Safety\n");
	fprintf(fp, "index, Division, Compare_Type, Data_Type, Value\n");
	for(i=0; i < MAX_P1_CAN_FUNCTION; i++) {
		fprintf(fp, "%02d, %d, %d, %d, %f\n", i+1,
			myData->COA_Client[div].testCond.safety.can_func_div[i],
			myData->COA_Client[div].testCond.safety.can_compare_type[i],
			myData->COA_Client[div].testCond.safety.can_data_type[i],
			myData->COA_Client[div].testCond.safety.can_value[i]);
	}
	fprintf(fp,"\n");

	fprintf(fp, "Aux_Common_Safety\n");
	fprintf(fp, "index, Division, Compare_Type, Data_Type, Value\n");
	for(i=0; i < MAX_P1_AUX_FUNCTION; i++) {
		fprintf(fp, "%02d, %d, %d, %d, %f\n", i+1,
			myData->COA_Client[div].testCond.safety.aux_func_div[i],
			myData->COA_Client[div].testCond.safety.aux_compare_type[i],
			myData->COA_Client[div].testCond.safety.aux_data_type[i],
			myData->COA_Client[div].testCond.safety.aux_value[i]);
	}
	fprintf(fp,"\n");

	fprintf(fp, "reserved2[4] : %ld, %ld, %ld, %ld\n", 
		myData->COA_Client[div].testCond.safety.reserved2[0],
		myData->COA_Client[div].testCond.safety.reserved2[1],
		myData->COA_Client[div].testCond.safety.reserved2[2],
		myData->COA_Client[div].testCond.safety.reserved2[3]);
	fprintf(fp,"\n");

	fprintf(fp, "TestCond_Step\n");
	fprintf(fp,"\n");

	for(i=0; i < myData->COA_Client[div].testCond.header.totalStep; i++) {
		fprintf(fp, "Step_Header : %d\n", i+1);
		fprintf(fp, "type : %d\n",
			myData->COA_Client[div].testCond.step[i].header.type);
		fprintf(fp, "stepNo : %d\n",
			myData->COA_Client[div].testCond.step[i].header.stepNo);
		fprintf(fp, "mode : %d\n",
			myData->COA_Client[div].testCond.step[i].header.mode);
		fprintf(fp, "testEnd : %d\n",
			myData->COA_Client[div].testCond.step[i].header.testEnd);
		fprintf(fp, "subStep : %d\n",
			myData->COA_Client[div].testCond.step[i].header.subStep);

		fprintf(fp, "reserved1 : %d\n", 
			myData->COA_Client[div].testCond.step[i].header.reserved1);
		fprintf(fp, "cycle_pause : %d\n", 
			myData->COA_Client[div].testCond.step[i].header.cycle_pause);
		fprintf(fp, "patternIndex : %d\n",
			myData->COA_Client[div].testCond.step[i].header.patternIndex);
		fprintf(fp, "reserved2 : %d\n", 
			myData->COA_Client[div].testCond.step[i].header.reserved2);
		fprintf(fp,"\n");

		for(j=0; j < 1; j++) {
//		for(j=0; j < MAX_P1_SUB_STEP; j++) {
			fprintf(fp, "Step_Reference[%d][%d]\n", i, j);
			fprintf(fp, "refV_upper : %ld\n",
				myData->COA_Client[div].testCond.step[i]
				.reference[j].refV_upper);
			fprintf(fp, "refV_lower : %ld\n",
				myData->COA_Client[div].testCond.step[i]
				.reference[j].refV_lower);
			fprintf(fp, "refI : %ld\n",
				myData->COA_Client[div].testCond.step[i].reference[j].refI);
			fprintf(fp, "refP : %ld\n",
				myData->COA_Client[div].testCond.step[i].reference[j].refP);
			fprintf(fp, "refR : %ld\n",
				myData->COA_Client[div].testCond.step[i].reference[j].refR);
			fprintf(fp, "rangeI : %d\n",
				myData->COA_Client[div].testCond.step[i].reference[j].rangeI);
			fprintf(fp, "ValueRate_Compare : %d\n",
				myData->COA_Client[div].testCond.step[i]
				.reference[j].ValueRate_Compare);
			fprintf(fp, "ValueRate : %d\n",
				myData->COA_Client[div].testCond.step[i]
				.reference[j].ValueRate);
			fprintf(fp, "Time : %ld\n",
				myData->COA_Client[div].testCond.step[i].reference[j].Time);
			fprintf(fp, "V_Upper : %ld\n",
				myData->COA_Client[div].testCond.step[i].reference[j].V_Upper);
			fprintf(fp, "Current : %ld\n",
				myData->COA_Client[div].testCond.step[i].reference[j].Current);
			fprintf(fp, "AmpareHour_SumAmpareHour : %ld\n",
				myData->COA_Client[div].testCond.step[i]
				.reference[j].AmpareHour_SumAmpareHour);
			fprintf(fp, "V_Upper_Branch_CycleCount_Branch : %ld\n",
				myData->COA_Client[div].testCond.step[i]
				.reference[j].V_Upper_Branch_CycleCount_Branch);
			fprintf(fp, "V_Lower_Branch_CycleCount : %ld\n",
				myData->COA_Client[div].testCond.step[i]
				.reference[j].V_Lower_Branch_CycleCount);
			fprintf(fp, "DeltaV : %ld\n",
				myData->COA_Client[div].testCond.step[i].reference[j].DeltaV);
			fprintf(fp, "Power : %ld\n",
				myData->COA_Client[div].testCond.step[i].reference[j].Power);
			fprintf(fp, "WattHour_SumWattHour : %ld\n",
				myData->COA_Client[div].testCond.step[i]
				.reference[j].WattHour_SumWattHour);
			fprintf(fp, "Time_Branch_AccCycleCount : %ld\n",
				myData->COA_Client[div].testCond.step[i]
				.reference[j].Time_Branch_AccCycleCount);
			fprintf(fp, "CVTime_Branch_AccCycleCountId : %ld\n",
				myData->COA_Client[div].testCond.step[i]
				.reference[j].CVTime_Branch_AccCycleCountId);
			fprintf(fp, "CVTime : %ld\n",
				myData->COA_Client[div].testCond.step[i].reference[j].CVTime);
			fprintf(fp, "reserved[3] : %d, %d, %d\n",
				myData->COA_Client[div].testCond.step[i]
				.reference[j].reserved[0],
				myData->COA_Client[div].testCond.step[i]
				.reference[j].reserved[1],
				myData->COA_Client[div].testCond.step[i]
				.reference[j].reserved[2]);
			fprintf(fp, "ValueRate_Item : %d\n",
				myData->COA_Client[div].testCond.step[i]
				.reference[j].ValueRate_Item);
			fprintf(fp, "V_Lower_MultiCycleCount : %ld\n",
				myData->COA_Client[div].testCond.step[i]
				.reference[j].V_Lower_MultiCycleCount);
			fprintf(fp, "AmpareHour_Branch_MultiCycleCountId : %d\n",
				myData->COA_Client[div].testCond.step[i]
				.reference[j].AmpareHour_Branch_MultiCycleCountId);
			fprintf(fp, "WattHour_Branch_MultiCycleCount_Branch : %d\n",
				myData->COA_Client[div].testCond.step[i]
				.reference[j].WattHour_Branch_MultiCycleCount_Branch);
			fprintf(fp, "ValueRate_Branch : %d\n",
				myData->COA_Client[div].testCond.step[i]
				.reference[j].ValueRate_Branch);
			fprintf(fp, "AccCycleCount_Branch : %d\n",
				myData->COA_Client[div].testCond.step[i]
				.reference[j].AccCycleCount_Branch);
			fprintf(fp,"\n");

			fprintf(fp, "Step_CAN\n");
			fprintf(fp, "index, Division, Compare_Type, Data_Type, Branch, Value\n");
			for(k=0; k < MAX_P1_CAN_FUNCTION; k++) {
				fprintf(fp, "%02d, %d, %d, %d, %d, %f\n", k+1,
					myData->COA_Client[div].testCond.step[i]
					.reference[j].can_func_div[k],
					myData->COA_Client[div].testCond.step[i]
					.reference[j].can_compare_type[k],
					myData->COA_Client[div].testCond.step[i]
					.reference[j].can_data_type[k],
					myData->COA_Client[div].testCond.step[i]
					.reference[j].can_branch[k],
					myData->COA_Client[div].testCond.step[i]
					.reference[j].can_value[k]);
			}
			fprintf(fp,"\n");

			fprintf(fp, "Step_Aux\n");
			fprintf(fp, "index, Division, Compare_Type, Data_Type, Branch, Value\n");
			for(k=0; k < MAX_P1_AUX_FUNCTION; k++) {
				fprintf(fp, "%02d, %d, %d, %d, %d, %f\n", k+1,
					myData->COA_Client[div].testCond.step[i]
					.reference[j].aux_func_div[k],
					myData->COA_Client[div].testCond.step[i]
					.reference[j].aux_compare_type[k],
					myData->COA_Client[div].testCond.step[i]
					.reference[j].aux_data_type[k],
					myData->COA_Client[div].testCond.step[i]
					.reference[j].aux_branch[k],
					myData->COA_Client[div].testCond.step[i]
					.reference[j].aux_value[k]);
			}
			fprintf(fp,"\n");
				
			fprintf(fp, "reserved2[2] : %ld, %ld\n",
				myData->COA_Client[div].testCond.step[i]
				.reference[j].reserved2[0],
				myData->COA_Client[div].testCond.step[i]
				.reference[j].reserved2[1]);
			fprintf(fp,"\n");
		}

		fprintf(fp, "Step_CompV[%d]\n", i);
		fprintf(fp, "lowerVlaue, upperValue, time\n");
		for(j=0; j < MAX_P1_COMP_POINT; j++) {
			fprintf(fp, "%ld, %ld, %ld\n",
				myData->COA_Client[div].testCond.step[i].compV[j].lowerValue,
				myData->COA_Client[div].testCond.step[i].compV[j].upperValue,
				myData->COA_Client[div].testCond.step[i].compV[j].time);
		}
		fprintf(fp,"\n");

		fprintf(fp, "Step_CompI[%d]\n", i);
		fprintf(fp, "lowerVlaue, upperValue, time\n");
		for(j=0; j < MAX_P1_COMP_POINT; j++) {
			fprintf(fp, "%ld, %ld, %ld\n",
				myData->COA_Client[div].testCond.step[i].compI[j].lowerValue,
				myData->COA_Client[div].testCond.step[i].compI[j].upperValue,
				myData->COA_Client[div].testCond.step[i].compI[j].time);
		}
		fprintf(fp,"\n");
				
		fprintf(fp, "Step_DeltaV\n");
		fprintf(fp, "lowerValue : %ld\n",
			myData->COA_Client[div].testCond.step[i].deltaV.lowerValue);
		fprintf(fp, "upperValue : %ld\n",
			myData->COA_Client[div].testCond.step[i].deltaV.upperValue);
		fprintf(fp, "time : %ld\n",
			myData->COA_Client[div].testCond.step[i].deltaV.time);
		fprintf(fp,"\n");

		fprintf(fp, "Step_DeltaI\n");
		fprintf(fp, "lowerValue : %ld\n",
			myData->COA_Client[div].testCond.step[i].deltaI.lowerValue);
		fprintf(fp, "upperValue : %ld\n",
			myData->COA_Client[div].testCond.step[i].deltaI.upperValue);
		fprintf(fp, "time : %ld\n",
			myData->COA_Client[div].testCond.step[i].deltaI.time);
		fprintf(fp,"\n");

		fprintf(fp, "Step_Record\n");
		fprintf(fp, "time : %ld\n",
			myData->COA_Client[div].testCond.step[i].record.time);
		fprintf(fp, "deltaV : %ld\n",
			myData->COA_Client[div].testCond.step[i].record.deltaV);
		fprintf(fp, "deltaI : %ld\n",
			myData->COA_Client[div].testCond.step[i].record.deltaI);
		fprintf(fp, "deltaT : %ld\n",
			myData->COA_Client[div].testCond.step[i].record.deltaT);
		fprintf(fp, "deltaP : %ld\n",
			myData->COA_Client[div].testCond.step[i].record.deltaP);
		fprintf(fp, "reserved : %ld\n",
			myData->COA_Client[div].testCond.step[i].record.reserved);
		fprintf(fp,"\n");
		
		fprintf(fp, "Step_EDLC\n");
		fprintf(fp, "capacitanceV1 : %ld\n",
			myData->COA_Client[div].testCond.step[i].edlc.capacitanceV1);
		fprintf(fp, "capacitanceV2 : %ld\n",
			myData->COA_Client[div].testCond.step[i].edlc.capacitanceV2);
		fprintf(fp, "startT_Z : %ld\n",
			myData->COA_Client[div].testCond.step[i].edlc.startT_Z);
		fprintf(fp, "endT_Z : %ld\n",
			myData->COA_Client[div].testCond.step[i].edlc.endT_Z);
		fprintf(fp, "startT_LC : %ld\n",
			myData->COA_Client[div].testCond.step[i].edlc.startT_LC);
		fprintf(fp, "endT_LC : %ld\n",
			myData->COA_Client[div].testCond.step[i].edlc.endT_LC);
		fprintf(fp,"\n");

		for(j=0; j < 1; j++) {
//		for(j=0; j < MAX_P1_GRADE_ITEM; j++) {
			fprintf(fp, "Step_Grade[%d][%d]\n", i, j);
			fprintf(fp, "item : %d\n",
				myData->COA_Client[div].testCond.step[i].grade[j].item);
			fprintf(fp, "gradeStepCount : %d\n",
				myData->COA_Client[div].testCond.step[i]
				.grade[j].gradeStepCount);
			fprintf(fp, "reserved1 : %d\n",
				myData->COA_Client[div].testCond.step[i].grade[j].reserved1);
			fprintf(fp,"\n");

			for(k=0; k < 1; k++) {
//			for(k=0; k < MAX_P1_GRADE_STEP; k++) {
				fprintf(fp, "Step_Grade_Step[%d][%d][%d]\n", i, j, k);
				fprintf(fp, "gradeCode : %d\n",
					myData->COA_Client[div].testCond.step[i]
					.grade[j].gradeStep[k].gradeCode);
				fprintf(fp, "reserved1 : %d\n",
					myData->COA_Client[div].testCond.step[i]
					.grade[j].gradeStep[k].reserved1);
				fprintf(fp, "reserved2 : %d\n",
					myData->COA_Client[div].testCond.step[i]
					.grade[j].gradeStep[k].reserved2);
				fprintf(fp, "lowerValue : %ld\n",
					myData->COA_Client[div].testCond.step[i]
					.grade[j].gradeStep[k].lowerValue);
				fprintf(fp, "upperValue : %ld\n",
					myData->COA_Client[div].testCond.step[i]
					.grade[j].gradeStep[k].upperValue);
				fprintf(fp,"\n");
			}
		}

		fprintf(fp, "TestCond_Step_Fault\n");
		fprintf(fp, "faultUpperV : %ld\n",
			myData->COA_Client[div].testCond.step[i].faultUpperV);
		fprintf(fp, "faultLowerV : %ld\n",
			myData->COA_Client[div].testCond.step[i].faultLowerV);
		fprintf(fp, "faultUpperI : %ld\n",
			myData->COA_Client[div].testCond.step[i].faultUpperI);
		fprintf(fp, "faultLowerI : %ld\n",
			myData->COA_Client[div].testCond.step[i].faultLowerI);
		fprintf(fp, "faultUpper_AmpareHour : %ld\n",
			myData->COA_Client[div].testCond.step[i].faultUpper_AmpareHour);
		fprintf(fp, "faultLower_AmpareHour : %ld\n",
			myData->COA_Client[div].testCond.step[i].faultLower_AmpareHour);
		fprintf(fp, "faultUpperZ : %ld\n",
			myData->COA_Client[div].testCond.step[i].faultUpperZ);
		fprintf(fp, "faultLowerZ : %ld\n",
			myData->COA_Client[div].testCond.step[i].faultLowerZ);
		fprintf(fp, "faultUpperTemp : %ld\n",
			myData->COA_Client[div].testCond.step[i].faultUpperTemp);
		fprintf(fp, "faultLowerTemp : %ld\n",
			myData->COA_Client[div].testCond.step[i].faultLowerTemp);
		fprintf(fp, "reserved1[2] : %ld, %ld\n",
			myData->COA_Client[div].testCond.step[i].reserved1[0],
			myData->COA_Client[div].testCond.step[i].reserved1[1]);
		fprintf(fp,"\n");
	}
		
	fprintf(fp, "TestCond_ETC\n");
	fprintf(fp, "stepCount : %d\n", myData->COA_Client[div].testCond.stepCount);
	fprintf(fp, "reserved1 : %d\n", myData->COA_Client[div].testCond.reserved1);

	fclose(fp);
#endif
	return 0;
}

int COA_Client_TestCond_write_V100C(int div)
{
#ifdef __COA_VER_100C__
	char fileName[256];
	int i,j,k = 0;
	FILE *fp;

	memset(fileName, 0, sizeof(fileName));

	sprintf(fileName, "./COA%d_Client_TestCond_V100C", div+1);

	if((fp = fopen(fileName, "w")) == NULL) {
		printf("Can not open COA%d_Client_TestCond file(write)\n", div+1);
		return -1;
	}

	fprintf(fp, "COA%d_Client_Receive_Data_V100C\n", div+1);
	fprintf(fp,"\n");
		
	fprintf(fp, "TestCond_Header\n");
	fprintf(fp, "totalStep : %d\n", 
		myData->COA_Client[div].testCond.header.totalStep);
	fprintf(fp, "reserverd1[3] : %d, %d, %d\n", 
		myData->COA_Client[div].testCond.header.reserved1[0],
		myData->COA_Client[div].testCond.header.reserved1[1],
		myData->COA_Client[div].testCond.header.reserved1[2]);
	fprintf(fp, "reserverd2[2] : %ld, %ld\n", 
		myData->COA_Client[div].testCond.header.reserved2[0],
		myData->COA_Client[div].testCond.header.reserved2[1]);
	fprintf(fp, "\n");
		
	fprintf(fp, "Common_Safety\n");
	fprintf(fp, "faultLowerV : %ld\n", 
		myData->COA_Client[div].testCond.safety.faultLowerV);
	fprintf(fp, "faultUpperV : %ld\n", 
		myData->COA_Client[div].testCond.safety.faultUpperV);
	fprintf(fp, "reserved1 : %ld\n", 
		myData->COA_Client[div].testCond.safety.reserved1);
	fprintf(fp, "faultUpperI : %ld\n", 
		myData->COA_Client[div].testCond.safety.faultUpperI);
	fprintf(fp, "faultLowerTemp : %ld\n", 
		myData->COA_Client[div].testCond.safety.faultLowerTemp);
	fprintf(fp, "faultUpperTemp : %ld\n", 
		myData->COA_Client[div].testCond.safety.faultUpperTemp);
	fprintf(fp, "faultUpper_AmpareHour : %ld\n", 
		myData->COA_Client[div].testCond.safety.faultUpper_AmpareHour);
	fprintf(fp, "faultUpperP : %ld\n", 
		myData->COA_Client[div].testCond.safety.faultUpperP);
	fprintf(fp, "faultUpper_WattHour : %ld\n", 
		myData->COA_Client[div].testCond.safety.faultUpper_WattHour);
	fprintf(fp,"\n");

	fprintf(fp, "CAN_Common_Safety\n");
	fprintf(fp, "index, Division, Compare_Type, Data_Type, Value\n");
	for(i=0; i < MAX_P1_CAN_FUNCTION; i++) {
		fprintf(fp, "%02d, %d, %d, %d, %f\n", i+1,
			myData->COA_Client[div].testCond.safety.can_func_div[i],
			myData->COA_Client[div].testCond.safety.can_compare_type[i],
			myData->COA_Client[div].testCond.safety.can_data_type[i],
			myData->COA_Client[div].testCond.safety.can_value[i]);
	}
	fprintf(fp,"\n");

	fprintf(fp, "Aux_Common_Safety\n");
	fprintf(fp, "index, Division, Compare_Type, Data_Type, Value\n");
	for(i=0; i < MAX_P1_AUX_FUNCTION; i++) {
		fprintf(fp, "%02d, %d, %d, %d, %f\n", i+1,
			myData->COA_Client[div].testCond.safety.aux_func_div[i],
			myData->COA_Client[div].testCond.safety.aux_compare_type[i],
			myData->COA_Client[div].testCond.safety.aux_data_type[i],
			myData->COA_Client[div].testCond.safety.aux_value[i]);
	}
	fprintf(fp,"\n");

	fprintf(fp, "reserved2[4] : %ld, %ld, %ld, %ld\n", 
		myData->COA_Client[div].testCond.safety.reserved2[0],
		myData->COA_Client[div].testCond.safety.reserved2[1],
		myData->COA_Client[div].testCond.safety.reserved2[2],
		myData->COA_Client[div].testCond.safety.reserved2[3]);
	fprintf(fp,"\n");

	fprintf(fp, "TestCond_Step\n");
	fprintf(fp,"\n");

	for(i=0; i < myData->COA_Client[div].testCond.header.totalStep; i++) {
		fprintf(fp, "Step_Header : %d\n", i+1);
		fprintf(fp, "type : %d\n",
			myData->COA_Client[div].testCond.step[i].header.type);
		fprintf(fp,

		fp
			myData->COA_Client[div].testCo
		fprintf(fp, "testEnd : %d\n",
			myData->COA_Client[div].testCond.step[i].header.testEnd);
		fprintf(fp, "subStep : %d\n",
			myData->COA_Client[div].testCond.step[i].header.subStep);

		fprintf(fp, "reserved1 : %d\n", 
			myData->COA_Client[div].testCond.step[i].header.reserved1);
		fprintf(fp, "cycle_pause : %d\n", 
			myData->COA_Client[div].testCond.step[i].header.cycle_pause);
		fprintf(fp, "patternIndex : %d\n",
			myData->COA_Client[div].testCond.step[i].header.patternIndex);
		fprintf(fp, "reserved2 : %d\n", 
			myData->COA_Client[div].testCond.step[i].header.reserved2);
		fprintf(fp,"\n");

		for(j=0; j < 1; j++) {
//		for(j=0; j < MAX_P1_SUB_STEP; j++) {
			fprintf(fp, "Step_Reference[%d][%d]\n", i, j);
			fprintf(fp, "refV_upper : %ld\n",
				myData->COA_Client[div].testCond.step[i]
				.reference[j].refV_upper);
			fprintf(fp, "refV_lower : %ld\n",
				myData->COA_Client[div].testCond.step[i]
				.reference[j].refV_lower);
			fprintf(fp, "refI : %ld\n",
				myData->COA_Client[div].testCond.step[i].reference[j].refI);
			fprintf(fp, "refP : %ld\n",
				myData->COA_Client[div].testCond.step[i].reference[j].refP);
			fprintf(fp, "refR : %ld\n",
				myData->COA_Client[div].testCond.step[i].reference[j].refR);
			fprintf(fp, "rangeI : %d\n",
				myData->COA_Client[div].testCond.step[i].reference[j].rangeI);
			fprintf(fp, "ValueRate_Compare : %d\n",
				myData->COA_Client[div].testCond.step[i]
				.reference[j].ValueRate_Compare);
			fprintf(fp, "ValueRate : %d\n",
				myData->COA_Client[div].testCond.step[i]
				.reference[j].ValueRate);
			fprintf(fp, "endTime_day : %ld\n",
				myData->COA_Client[div].testCond.step[i].reference[j].endTime_day);
			fprintf(fp, "endTime : %ld\n",
				myData->COA_Client[div].testCond.step[i].reference[j].endTime);
			fprintf(fp, "V_Upper : %ld\n",
				myData->COA_Client[div].testCond.step[i].reference[j].V_Upper);
			fprintf(fp, "Current : %ld\n",
				myData->COA_Client[div].testCond.step[i].reference[j].Current);
			fprintf(fp, "AmpareHour_SumAmpareHour : %ld\n",
				myData->COA_Client[div].testCond.step[i]
				.reference[j].AmpareHour_SumAmpareHour);
			fprintf(fp, "V_Upper_Branch_CycleCount_Branch : %ld\n",
				myData->COA_Client[div].testCond.step[i]
				.reference[j].V_Upper_Branch_CycleCount_Branch);
			fprintf(fp, "V_Lower_Branch_CycleCount : %ld\n",
				myData->COA_Client[div].testCond.step[i]
				.reference[j].V_Lower_Branch_CycleCount);
			fprintf(fp, "DeltaV : %ld\n",
				myData->COA_Client[div].testCond.step[i].reference[j].DeltaV);
			fprintf(fp, "Power : %ld\n",
				myData->COA_Client[div].testCond.step[i].reference[j].Power);
			fprintf(fp, "WattHour_SumWattHour : %ld\n",
				myData->COA_Client[div].testCond.step[i]
				.reference[j].WattHour_SumWattHour);
			fprintf(fp, "Time_Branch_AccCycleCount : %ld\n",
				myData->COA_Client[div].testCond.step[i]
				.reference[j].Time_Branch_AccCycleCount);
			fprintf(fp, "CVTime_Branch_AccCycleCountId : %ld\n",
				myData->COA_Client[div].testCond.step[i]
				.reference[j].CVTime_Branch_AccCycleCountId);
			fprintf(fp, "CVTime_day : %ld\n",
				myData->COA_Client[div].testCond.step[i].reference[j].CVTime_day);
			fprintf(fp, "CVTime : %ld\n",
				myData->COA_Client[div].testCond.step[i].reference[j].CVTime);
			fprintf(fp, "reserved[3] : %d, %d, %d\n",
				myData->COA_Client[div].testCond.step[i]
				.reference[j].reserved[0],
				myData->COA_Client[div].testCond.step[i]
				.reference[j].reserved[1],
				myData->COA_Client[div].testCond.step[i]
				.reference[j].reserved[2]);
			fprintf(fp, "ValueRate_Item : %d\n",
				myData->COA_Client[div].testCond.step[i]
				.reference[j].ValueRate_Item);
			fprintf(fp, "V_Lower_MultiCycleCount : %ld\n",
				myData->COA_Client[div].testCond.step[i]
				.reference[j].V_Lower_MultiCycleCount);
			fprintf(fp, "AmpareHour_Branch_MultiCycleCountId : %d\n",
				myData->COA_Client[div].testCond.step[i]
				.reference[j].AmpareHour_Branch_MultiCycleCountId);
			fprintf(fp, "WattHour_Branch_MultiCycleCount_Branch : %d\n",
				myData->COA_Client[div].testCond.step[i]
				.reference[j].WattHour_Branch_MultiCycleCount_Branch);
			fprintf(fp, "ValueRate_Branch : %d\n",
				myData->COA_Client[div].testCond.step[i]
				.reference[j].ValueRate_Branch);
			fprintf(fp, "AccCycleCount_Branch : %d\n",
				myData->COA_Client[div].testCond.step[i]
				.reference[j].AccCycleCount_Branch);
			fprintf(fp,"\n");

			fprintf(fp, "Step_CAN\n");
			fprintf(fp, "index, Division, Compare_Type, Data_Type, Branch, Value\n");
			for(k=0; k < MAX_P1_CAN_FUNCTION; k++) {
				fprintf(fp, "%02d, %d, %d, %d, %d, %f\n", k+1,
					myData->COA_Client[div].testCond.step[i]
					.reference[j].can_func_div[k],
					myData->COA_Client[div].testCond.step[i]
					.reference[j].can_compare_type[k],
					myData->COA_Client[div].testCond.step[i]
					.reference[j].can_data_type[k],
					myData->COA_Client[div].testCond.step[i]
					.reference[j].can_branch[k],
					myData->COA_Client[div].testCond.step[i]
					.reference[j].can_value[k]);
			}
			fprintf(fp,"\n");

			fprintf(fp, "Step_Aux\n");
			fprintf(fp, "index, Division, Compare_Type, Data_Type, Branch, Value\n");
			for(k=0; k < MAX_P1_AUX_FUNCTION; k++) {
				fprintf(fp, "%02d, %d, %d, %d, %d, %f\n", k+1,
					myData->COA_Client[div].testCond.step[i]
					.reference[j].aux_func_div[k],
					myData->COA_Client[div].testCond.step[i]
					.reference[j].aux_compare_type[k],
					myData->COA_Client[div].testCond.step[i]
					.reference[j].aux_data_type[k],
					myData->COA_Client[div].testCond.step[i]
					.reference[j].aux_branch[k],
					myData->COA_Client[div].testCond.step[i]
					.reference[j].aux_value[k]);
			}
			fprintf(fp,"\n");
				
			fprintf(fp, "reserved2[2] : %ld, %ld\n",
				myData->COA_Client[div].testCond.step[i]
				.reference[j].reserved2[0],
				myData->COA_Client[div].testCond.step[i]
				.reference[j].reserved2[1]);
			fprintf(fp,"\n");
		}

		fprintf(fp, "Step_CompV[%d]\n", i);
		fprintf(fp, "lowerVlaue, upperValue, time\n");
		for(j=0; j < MAX_P1_COMP_POINT; j++) {
			fprintf(fp, "%ld, %ld, %ld\n",
				myData->COA_Client[div].testCond.step[i].compV[j].lowerValue,
				myData->COA_Client[div].testCond.step[i].compV[j].upperValue,
				myData->COA_Client[div].testCond.step[i].compV[j].time);
		}
		fprintf(fp,"\n");

		fprintf(fp, "Step_CompI[%d]\n", i);
		fprintf(fp, "lowerVlaue, upperValue, time\n");
		for(j=0; j < MAX_P1_COMP_POINT; j++) {
			fprintf(fp, "%ld, %ld, %ld\n",
				myData->COA_Client[div].testCond.step[i].compI[j].lowerValue,
				myData->COA_Client[div].testCond.step[i].compI[j].upperValue,
				myData->COA_Client[div].testCond.step[i].compI[j].time);
		}
		fprintf(fp,"\n");
				
		fprintf(fp, "Step_DeltaV\n");
		fprintf(fp, "lowerValue : %ld\n",
			myData->COA_Client[div].testCond.step[i].deltaV.lowerValue);
		fprintf(fp, "upperValue : %ld\n",
			myData->COA_Client[div].testCond.step[i].deltaV.upperValue);
		fprintf(fp, "time : %ld\n",
			myData->COA_Client[div].testCond.step[i].deltaV.time);
		fprintf(fp,"\n");

		fprintf(fp, "Step_DeltaI\n");
		fprintf(fp, "lowerValue : %ld\n",
			myData->COA_Client[div].testCond.step[i].deltaI.lowerValue);
		fprintf(fp, "upperValue : %ld\n",
			myData->COA_Client[div].testCond.step[i].deltaI.upperValue);
		fprintf(fp, "time : %ld\n",
			myData->COA_Client[div].testCond.step[i].deltaI.time);
		fprintf(fp,"\n");

		fprintf(fp, "Step_Record\n");
		fprintf(fp, "time : %ld\n",
			myData->COA_Client[div].testCond.step[i].record.time);
		fprintf(fp, "deltaV : %ld\n",
			myData->COA_Client[div].testCond.step[i].record.deltaV);
		fprintf(fp, "deltaI : %ld\n",
			myData->COA_Client[div].testCond.step[i].record.deltaI);
		fprintf(fp, "deltaT : %ld\n",
			myData->COA_Client[div].testCond.step[i].record.deltaT);
		fprintf(fp, "deltaP : %ld\n",
			myData->COA_Client[div].testCond.step[i].record.deltaP);
		fprintf(fp, "reserved : %ld\n",
			myData->COA_Client[div].testCond.step[i].record.reserved);
		fprintf(fp,"\n");
		
		fprintf(fp, "Step_EDLC\n");
		fprintf(fp, "capacitanceV1 : %ld\n",
			myData->COA_Client[div].testCond.step[i].edlc.capacitanceV1);
		fprintf(fp, "capacitanceV2 : %ld\n",
			myData->COA_Client[div].testCond.step[i].edlc.capacitanceV2);
		fprintf(fp, "startT_Z : %ld\n",
			myData->COA_Client[div].testCond.step[i].edlc.startT_Z);
		fprintf(fp, "endT_Z : %ld\n",
			myData->COA_Client[div].testCond.step[i].edlc.endT_Z);
		fprintf(fp, "startT_LC : %ld\n",
			myData->COA_Client[div].testCond.step[i].edlc.startT_LC);
		fprintf(fp, "endT_LC : %ld\n",
			myData->COA_Client[div].testCond.step[i].edlc.endT_LC);
		fprintf(fp,"\n");

		for(j=0; j < 1; j++) {
//		for(j=0; j < MAX_P1_GRADE_ITEM; j++) {
			fprintf(fp, "Step_Grade[%d][%d]\n", i, j);
			fprintf(fp, "item : %d\n",
				myData->COA_Client[div].testCond.step[i].grade[j].item);
			fprintf(fp, "gradeStepCount : %d\n",
				myData->COA_Client[div].testCond.step[i]
				.grade[j].gradeStepCount);
			fprintf(fp, "reserved1 : %d\n",
				myData->COA_Client[div].testCond.step[i].grade[j].reserved1);
			fprintf(fp,"\n");

			for(k=0; k < 1; k++) {
//			for(k=0; k < MAX_P1_GRADE_STEP; k++) {
				fprintf(fp, "Step_Grade_Step[%d][%d][%d]\n", i, j, k);
				fprintf(fp, "gradeCode : %d\n",
					myData->COA_Client[div].testCond.step[i]
					.grade[j].gradeStep[k].gradeCode);
				fprintf(fp, "reserved1 : %d\n",
					myData->COA_Client[div].testCond.step[i]
					.grade[j].gradeStep[k].reserved1);
				fprintf(fp, "reserved2 : %d\n",
					myData->COA_Client[div].testCond.step[i]
					.grade[j].gradeStep[k].reserved2);
				fprintf(fp, "lowerValue : %ld\n",
					myData->COA_Client[div].testCond.step[i]
					.grade[j].gradeStep[k].lowerValue);
				fprintf(fp, "upperValue : %ld\n",
					myData->COA_Client[div].testCond.step[i]
					.grade[j].gradeStep[k].upperValue);
				fprintf(fp,"\n");
			}
		}

		fprintf(fp, "TestCond_Step_Fault\n");
		fprintf(fp, "faultUpperV : %ld\n",
			myData->COA_Client[div].testCond.step[i].faultUpperV);
		fprintf(fp, "faultLowerV : %ld\n",
			myData->COA_Client[div].testCond.step[i].faultLowerV);
		fprintf(fp, "faultUpperI : %ld\n",
			myData->COA_Client[div].testCond.step[i].faultUpperI);
		fprintf(fp, "faultLowerI : %ld\n",
			myData->COA_Client[div].testCond.step[i].faultLowerI);
		fprintf(fp, "faultUpper_AmpareHour : %ld\n",
			myData->COA_Client[div].testCond.step[i].faultUpper_AmpareHour);
		fprintf(fp, "faultLower_AmpareHour : %ld\n",
			myData->COA_Client[div].testCond.step[i].faultLower_AmpareHour);
		fprintf(fp, "faultUpperZ : %ld\n",
			myData->COA_Client[div].testCond.step[i].faultUpperZ);
		fprintf(fp, "faultLowerZ : %ld\n",
			myData->COA_Client[div].testCond.step[i].faultLowerZ);
		fprintf(fp, "faultUpperTemp : %ld\n",
			myData->COA_Client[div].testCond.step[i].faultUpperTemp);
		fprintf(fp, "faultLowerTemp : %ld\n",
			myData->COA_Client[div].testCond.step[i].faultLowerTemp);
		fprintf(fp, "reserved1[2] : %ld, %ld\n",
			myData->COA_Client[div].testCond.step[i].reserved1[0],
			myData->COA_Client[div].testCond.step[i].reserved1[1]);
		fprintf(fp,"\n");
	}
		
	fprintf(fp, "TestCond_ETC\n");
	fprintf(fp, "stepCount : %d\n", myData->COA_Client[div].testCond.stepCount);
	fprintf(fp, "reserved1 : %d\n", myData->COA_Client[div].testCond.reserved1);

	fclose(fp);
#endif
	return 0;
}

int COA_Client_TestCond_write_V100D(int div)
{
	char fileName[256];
	int i,j,k = 0;
	FILE *fp;

	memset(fileName, 0, sizeof(fileName));

	sprintf(fileName, "./COA%d_Client_TestCond_V100D", div+1);

	if((fp = fopen(fileName, "w")) == NULL) {
		printf("Can not open COA%d_Client_TestCond file(write)\n", div+1);
		return -1;
	}

	fprintf(fp, "COA%d_Client_Receive_Data_V100D\n", div+1);
	fprintf(fp,"\n");
		
	fprintf(fp, "TestCond_Header\n");
	fprintf(fp, "totalStep : %d\n", 
		myData->COA_Client[div].testCond.header.totalStep);
	fprintf(fp, "totalPatternCount : %d\n", 
		myData->COA_Client[div].testCond.header.totalPatternCount);
	fprintf(fp, "reserverd1[2] : %ld, %ld\n", 
		myData->COA_Client[div].testCond.header.reserved1[0],
		myData->COA_Client[div].testCond.header.reserved1[1]);
	fprintf(fp, "\n");
		
	fprintf(fp, "Common_Safety\n");
	fprintf(fp, "faultLowerV : %ld\n", 
		myData->COA_Client[div].testCond.safety.faultLowerV);
	fprintf(fp, "faultUpperV : %ld\n", 
		myData->COA_Client[div].testCond.safety.faultUpperV);
	//fprintf(fp, "reserved1 : %ld\n", 
	//	myData->COA_Client[div].testCond.safety.reserved1);
	fprintf(fp, "faultCompAuxV : %ld\n", 
		myData->COA_Client[div].testCond.safety.faultCompAuxV);
	fprintf(fp, "faultUpperI : %ld\n", 
		myData->COA_Client[div].testCond.safety.faultUpperI);
	fprintf(fp, "faultLowerTemp : %ld\n", 
		myData->COA_Client[div].testCond.safety.faultLowerTemp);
	fprintf(fp, "faultUpperTemp : %ld\n", 
		myData->COA_Client[div].testCond.safety.faultUpperTemp);
	fprintf(fp, "faultUpper_AmpareHour : %ld\n", 
		myData->COA_Client[div].testCond.safety.faultUpper_AmpareHour);
	fprintf(fp, "faultUpperP : %ld\n", 
		myData->COA_Client[div].testCond.safety.faultUpperP);
	fprintf(fp, "faultUpper_WattHour : %ld\n", 
		myData->COA_Client[div].testCond.safety.faultUpper_WattHour);
	fprintf(fp,"\n");

	fprintf(fp, "CAN_Common_Safety\n");
	fprintf(fp, "index, Division, Compare_Type, Data_Type, Value\n");
	for(i=0; i < MAX_P1_CAN_FUNCTION; i++) {
		fprintf(fp, "%02d, %d, %d, %d, %f\n", i+1,
			myData->COA_Client[div].testCond.safety.can_func_div[i],
			myData->COA_Client[div].testCond.safety.can_compare_type[i],
			myData->COA_Client[div].testCond.safety.can_data_type[i],
			myData->COA_Client[div].testCond.safety.can_value[i]);
	}
	fprintf(fp,"\n");

	fprintf(fp, "Aux_Common_Safety\n");
	fprintf(fp, "index, Division, Compare_Type, Data_Type, Value\n");
	for(i=0; i < MAX_P1_AUX_FUNCTION; i++) {
		fprintf(fp, "%02d, %d, %d, %d, %f\n", i+1,
			myData->COA_Client[div].testCond.safety.aux_func_div[i],
			myData->COA_Client[div].testCond.safety.aux_compare_type[i],
			myData->COA_Client[div].testCond.safety.aux_data_type[i],
			myData->COA_Client[div].testCond.safety.aux_value[i]);
	}
	fprintf(fp,"\n");

	fprintf(fp, "reserved2[4] : %ld, %ld, %ld\n", 
		myData->COA_Client[div].testCond.safety.rptsoc,
		myData->COA_Client[div].testCond.safety.soc,
		myData->COA_Client[div].testCond.safety.reserved2);

	fprintf(fp, "current_AH : %ld\n", 
		myData->COA_Client[div].testCond.safety.current_AH);
	fprintf(fp, "initial_AH : %ld\n", 
		myData->COA_Client[div].testCond.safety.initial_AH);
	fprintf(fp, "crate_factor : %ld\n", 
		myData->COA_Client[div].testCond.safety.crate_factor);
	fprintf(fp,"\n");

	fprintf(fp, "TestCond_Step\n");
	fprintf(fp,"\n");

	for(i=0; i < myData->COA_Client[div].testCond.header.totalStep; i++) {
	//for(i=350; i < myData->COA_Client[div].testCond.header.totalStep; i++) {
		fprintf(fp, "Step_Header : %d\n", i+1);
		fprintf(fp, "type : %d\n",
			myData->COA_Client[div].testCond.step[i].header.type);
		fprintf(fp, "stepNo : %d\n",
			myData->COA_Client[div].testCond.step[i].header.stepNo);
		fprintf(fp, "mode : %d\n",
			myData->COA_Client[div].testCond.step[i].header.mode);
		fprintf(fp, "testEnd : %d\n",
			myData->COA_Client[div].testCond.step[i].header.testEnd);
		fprintf(fp, "subStep : %d\n",
			myData->COA_Client[div].testCond.step[i].header.subStep);

		fprintf(fp, "reserved1 : %d\n", 
			myData->COA_Client[div].testCond.step[i].header.reserved1);
		fprintf(fp, "cycle_pause : %d\n", 
			myData->COA_Client[div].testCond.step[i].header.cycle_pause);
		fprintf(fp, "patternIndex : %d\n",
			myData->COA_Client[div].testCond.step[i].header.patternIndex);
//		fprintf(fp, "reserved2 : %d\n", 
//			myData->COA_Client[div].testCond.step[i].header.reserved2);
		fprintf(fp,"\n");

		for(j=0; j < 1; j++) {
//		for(j=0; j < MAX_P1_SUB_STEP; j++) {
			fprintf(fp, "Step_Reference[%d][%d]\n", i, j);
			fprintf(fp, "refV_upper : %ld\n",
				myData->COA_Client[div].testCond.step[i]
				.reference[j].refV_upper);
			fprintf(fp, "refV_lower : %ld\n",
				myData->COA_Client[div].testCond.step[i]
				.reference[j].refV_lower);
			fprintf(fp, "refI : %ld\n",
				myData->COA_Client[div].testCond.step[i].reference[j].refI);
			fprintf(fp, "refP : %ld\n",
				myData->COA_Client[div].testCond.step[i].reference[j].refP);
			fprintf(fp, "refR : %ld\n",
				myData->COA_Client[div].testCond.step[i].reference[j].refR);
			fprintf(fp, "rangeI : %d\n",
				myData->COA_Client[div].testCond.step[i].reference[j].rangeI);
		//	fprintf(fp, "limitCurrent_Lower : %ld\n",
		//		myData->COA_Client[div].testCond.step[i].reference[j].limitCurrent_Lower);
		//	fprintf(fp, "limitCurrent_Upper : %ld\n",
		//		myData->COA_Client[div].testCond.step[i].reference[j].limitCurrent_Upper);
			fprintf(fp, "ValueRate_Compare : %d\n",
				myData->COA_Client[div].testCond.step[i]
				.reference[j].ValueRate_Compare);
			fprintf(fp, "ValueRate : %d\n",
				myData->COA_Client[div].testCond.step[i]
				.reference[j].ValueRate);
			fprintf(fp, "endTime_day : %ld\n",
				myData->COA_Client[div].testCond.step[i].reference[j].endTime_day);
			fprintf(fp, "endTime : %ld\n",
				myData->COA_Client[div].testCond.step[i].reference[j].endTime);
			fprintf(fp, "V_Upper : %ld\n",
				myData->COA_Client[div].testCond.step[i].reference[j].V_Upper);
			fprintf(fp, "Current : %ld\n",
				myData->COA_Client[div].testCond.step[i].reference[j].Current);
			fprintf(fp, "AmpareHour_SumAmpareHour : %ld\n",
				myData->COA_Client[div].testCond.step[i]
				.reference[j].AmpareHour_SumAmpareHour);
			fprintf(fp, "V_Upper_Branch_CycleCount_Branch : %ld\n",
				myData->COA_Client[div].testCond.step[i]
				.reference[j].V_Upper_Branch_CycleCount_Branch);
			fprintf(fp, "V_Lower_Branch_CycleCount : %ld\n",
				myData->COA_Client[div].testCond.step[i]
				.reference[j].V_Lower_Branch_CycleCount);
			fprintf(fp, "DeltaV : %ld\n",
				myData->COA_Client[div].testCond.step[i].reference[j].DeltaV);
			fprintf(fp, "Power : %ld\n",
				myData->COA_Client[div].testCond.step[i].reference[j].Power);
			fprintf(fp, "WattHour_SumWattHour : %ld\n",
				myData->COA_Client[div].testCond.step[i]
				.reference[j].WattHour_SumWattHour);
			fprintf(fp, "Time_Branch_AccCycleCount : %ld\n",
				myData->COA_Client[div].testCond.step[i]
				.reference[j].Time_Branch_AccCycleCount);
			fprintf(fp, "CVTime_Branch_AccCycleCountId : %ld\n",
				myData->COA_Client[div].testCond.step[i]
				.reference[j].CVTime_Branch_AccCycleCountId);
			fprintf(fp, "CVTime_day : %ld\n",
				myData->COA_Client[div].testCond.step[i].reference[j].CVTime_day);
			fprintf(fp, "CVTime : %ld\n",
				myData->COA_Client[div].testCond.step[i].reference[j].CVTime);
//			fprintf(fp, "reserved[3] : %d, %d, %d\n",
//				myData->COA_Client[div].testCond.step[i]
//				.reference[j].reserved[0],
//				myData->COA_Client[div].testCond.step[i]
//				.reference[j].reserved[1],
//				myData->COA_Client[div].testCond.step[i]
//				.reference[j].reserved[2]);
			fprintf(fp, "ValueRate_Item : %d\n",
				myData->COA_Client[div].testCond.step[i]
				.reference[j].ValueRate_Item);
			fprintf(fp, "V_Lower_MultiCycleCount : %ld\n",
				myData->COA_Client[div].testCond.step[i]
				.reference[j].V_Lower_MultiCycleCount);
			fprintf(fp, "AmpareHour_Branch_MultiCycleCountId : %d\n",
				myData->COA_Client[div].testCond.step[i]
				.reference[j].AmpareHour_Branch_MultiCycleCountId);
			fprintf(fp, "WattHour_Branch_MultiCycleCount_Branch : %d\n",
				myData->COA_Client[div].testCond.step[i]
				.reference[j].WattHour_Branch_MultiCycleCount_Branch);
			fprintf(fp, "ValueRate_Branch : %d\n",
				myData->COA_Client[div].testCond.step[i]
				.reference[j].ValueRate_Branch);
			fprintf(fp, "AccCycleCount_Branch : %d\n",
				myData->COA_Client[div].testCond.step[i]
				.reference[j].AccCycleCount_Branch);
			fprintf(fp, "Sequence_Charge_flag : %d\n",
				myData->COA_Client[div].testCond.step[i]
				.reference[j].sequence_charge_flag);
			fprintf(fp,"\n");

			fprintf(fp, "Step_CAN\n");
			fprintf(fp, "index, Division, Compare_Type, Data_Type, Branch, Value\n");
			for(k=0; k < MAX_P1_CAN_FUNCTION; k++) {
				fprintf(fp, "%02d, %d, %d, %d, %d, %f\n", k+1,
					myData->COA_Client[div].testCond.step[i]
					.reference[j].can_func_div[k],
					myData->COA_Client[div].testCond.step[i]
					.reference[j].can_compare_type[k],
					myData->COA_Client[div].testCond.step[i]
					.reference[j].can_data_type[k],
					myData->COA_Client[div].testCond.step[i]
					.reference[j].can_branch[k],
					myData->COA_Client[div].testCond.step[i]
					.reference[j].can_value[k]);
			}
			fprintf(fp,"\n");

			fprintf(fp, "Step_Aux\n");
			fprintf(fp, "index, Division, Compare_Type, Data_Type, Branch, Value, delay_time\n");
			for(k=0; k < MAX_P1_AUX_FUNCTION; k++) {
				fprintf(fp, "%02d, %d, %d, %d, %d, %f, %d\n", k+1,
					myData->COA_Client[div].testCond.step[i]
					.reference[j].aux_func_div[k],
					myData->COA_Client[div].testCond.step[i]
					.reference[j].aux_compare_type[k],
					myData->COA_Client[div].testCond.step[i]
					.reference[j].aux_data_type[k],
					myData->COA_Client[div].testCond.step[i]
					.reference[j].aux_branch[k],
					myData->COA_Client[div].testCond.step[i]
					.reference[j].aux_value[k],
					myData->COA_Client[div].testCond.step[i]
					.reference[j].aux_delay_time[k]);
			}
			fprintf(fp,"\n");
			
			fprintf(fp, "pattern_max_refI : %ld, pattern_min_refI %ld\n",
				myData->COA_Client[div].testCond.step[i].reference[j].pattern_max_refI,
				myData->COA_Client[div].testCond.step[i].reference[j].pattern_min_refI);
			fprintf(fp,"\n");
			
			fprintf(fp, "reserved2[2] : %d, %d\n",
				myData->COA_Client[div].testCond.step[i]
				.reference[j].reserved2,
				myData->COA_Client[div].testCond.step[i]
				.reference[j].reserved2);
			fprintf(fp,"\n");
		}

		fprintf(fp, "Step_CompV[%d]\n", i);
		fprintf(fp, "lowerVlaue, upperValue, time\n");
		for(j=0; j < MAX_P1_COMP_POINT; j++) {
			fprintf(fp, "%ld, %ld, %ld\n",
				myData->COA_Client[div].testCond.step[i].compV[j].lowerValue,
				myData->COA_Client[div].testCond.step[i].compV[j].upperValue,
				myData->COA_Client[div].testCond.step[i].compV[j].time);
		}
		fprintf(fp,"\n");

		fprintf(fp, "Step_CompI[%d]\n", i);
		fprintf(fp, "lowerVlaue, upperValue, time\n");
		for(j=0; j < MAX_P1_COMP_POINT; j++) {
			fprintf(fp, "%ld, %ld, %ld\n",
				myData->COA_Client[div].testCond.step[i].compI[j].lowerValue,
				myData->COA_Client[div].testCond.step[i].compI[j].upperValue,
				myData->COA_Client[div].testCond.step[i].compI[j].time);
		}
		fprintf(fp,"\n");
				
		fprintf(fp, "Step_DeltaV\n");
		fprintf(fp, "lowerValue : %ld\n",
			myData->COA_Client[div].testCond.step[i].deltaV.lowerValue);
		fprintf(fp, "upperValue : %ld\n",
			myData->COA_Client[div].testCond.step[i].deltaV.upperValue);
		fprintf(fp, "time : %ld\n",
			myData->COA_Client[div].testCond.step[i].deltaV.time);
		fprintf(fp,"\n");

		fprintf(fp, "Step_DeltaI\n");
		fprintf(fp, "lowerValue : %ld\n",
			myData->COA_Client[div].testCond.step[i].deltaI.lowerValue);
		fprintf(fp, "upperValue : %ld\n",
			myData->COA_Client[div].testCond.step[i].deltaI.upperValue);
		fprintf(fp, "time : %ld\n",
			myData->COA_Client[div].testCond.step[i].deltaI.time);
		fprintf(fp,"\n");

		fprintf(fp, "Step_Record\n");
		fprintf(fp, "time : %ld\n",
			myData->COA_Client[div].testCond.step[i].record.time);
		fprintf(fp, "deltaV : %ld\n",
			myData->COA_Client[div].testCond.step[i].record.deltaV);
		fprintf(fp, "deltaI : %ld\n",
			myData->COA_Client[div].testCond.step[i].record.deltaI);
		fprintf(fp, "deltaT : %ld\n",
			myData->COA_Client[div].testCond.step[i].record.deltaT);
		fprintf(fp, "deltaP : %ld\n",
			myData->COA_Client[div].testCond.step[i].record.deltaP);
		fprintf(fp, "reserved : %ld\n",
			myData->COA_Client[div].testCond.step[i].record.reserved);
		fprintf(fp,"\n");
		
		fprintf(fp, "Step_EDLC\n");
		fprintf(fp, "capacitanceV1 : %ld\n",
			myData->COA_Client[div].testCond.step[i].edlc.capacitanceV1);
		fprintf(fp, "capacitanceV2 : %ld\n",
			myData->COA_Client[div].testCond.step[i].edlc.capacitanceV2);
		fprintf(fp, "startT_Z : %ld\n",
			myData->COA_Client[div].testCond.step[i].edlc.startT_Z);
		fprintf(fp, "endT_Z : %ld\n",
			myData->COA_Client[div].testCond.step[i].edlc.endT_Z);
		fprintf(fp, "startT_LC : %ld\n",
			myData->COA_Client[div].testCond.step[i].edlc.startT_LC);
		fprintf(fp, "endT_LC : %ld\n",
			myData->COA_Client[div].testCond.step[i].edlc.endT_LC);
		fprintf(fp,"\n");

		for(j=0; j < 1; j++) {
//		for(j=0; j < MAX_P1_GRADE_ITEM; j++) {
			fprintf(fp, "Step_Grade[%d][%d]\n", i, j);
			fprintf(fp, "item : %d\n",
				myData->COA_Client[div].testCond.step[i].grade[j].item);
			fprintf(fp, "gradeStepCount : %d\n",
				myData->COA_Client[div].testCond.step[i]
				.grade[j].gradeStepCount);
			fprintf(fp, "reserved1 : %d\n",
				myData->COA_Client[div].testCond.step[i].grade[j].reserved1);
			fprintf(fp,"\n");

			for(k=0; k < 1; k++) {
//			for(k=0; k < MAX_P1_GRADE_STEP; k++) {
				fprintf(fp, "Step_Grade_Step[%d][%d][%d]\n", i, j, k);
				fprintf(fp, "gradeCode : %d\n",
					myData->COA_Client[div].testCond.step[i]
					.grade[j].gradeStep[k].gradeCode);
				fprintf(fp, "reserved1 : %d\n",
					myData->COA_Client[div].testCond.step[i]
					.grade[j].gradeStep[k].reserved1);
				fprintf(fp, "reserved2 : %d\n",
					myData->COA_Client[div].testCond.step[i]
					.grade[j].gradeStep[k].reserved2);
				fprintf(fp, "lowerValue : %ld\n",
					myData->COA_Client[div].testCond.step[i]
					.grade[j].gradeStep[k].lowerValue);
				fprintf(fp, "upperValue : %ld\n",
					myData->COA_Client[div].testCond.step[i]
					.grade[j].gradeStep[k].upperValue);
				fprintf(fp,"\n");
			}
		}

		fprintf(fp, "TestCond_Step_Fault\n");
		fprintf(fp, "faultUpperV : %ld\n",
			myData->COA_Client[div].testCond.step[i].faultUpperV);
		fprintf(fp, "faultLowerV : %ld\n",
			myData->COA_Client[div].testCond.step[i].faultLowerV);
		fprintf(fp, "faultUpperI : %ld\n",
			myData->COA_Client[div].testCond.step[i].faultUpperI);
		fprintf(fp, "faultLowerI : %ld\n",
			myData->COA_Client[div].testCond.step[i].faultLowerI);
		fprintf(fp, "faultUpper_AmpareHour : %ld\n",
			myData->COA_Client[div].testCond.step[i].faultUpper_AmpareHour);
		fprintf(fp, "faultLower_AmpareHour : %ld\n",
			myData->COA_Client[div].testCond.step[i].faultLower_AmpareHour);
		fprintf(fp, "faultUpperZ : %ld\n",
			myData->COA_Client[div].testCond.step[i].faultUpperZ);
		fprintf(fp, "faultLowerZ : %ld\n",
			myData->COA_Client[div].testCond.step[i].faultLowerZ);
		/*//jhkw_180823s
		fprintf(fp, "faultDeltaV : %ld\n",
			myData->COA_Client[div].testCond.step[i].faultDeltaV);
		fprintf(fp, "faultDeltaV_T : %ld\n",
			myData->COA_Client[div].testCond.step[i].faultDeltaV_T);
		fprintf(fp, "faultDeltaI : %ld\n",
			myData->COA_Client[div].testCond.step[i].faultDeltaI);
		fprintf(fp, "faultDeltaI_T : %ld\n",
			myData->COA_Client[div].testCond.step[i].faultDeltaI_T);
		*///jhkw_180823s
		fprintf(fp, "faultUpperTemp : %ld\n",
			myData->COA_Client[div].testCond.step[i].faultUpperTemp);
		fprintf(fp, "faultLowerTemp : %ld\n",
			myData->COA_Client[div].testCond.step[i].faultLowerTemp);
		fprintf(fp, "pattern_time_type : %d\n",
			myData->COA_Client[div].testCond.step[i].pattern_time_type);
		fprintf(fp, "can_comm_step_check_flag : %d\n",
			myData->COA_Client[div].testCond.step[i].can_comm_step_check_flag);
		fprintf(fp, "fault_can_check_flag : %d\n",
			myData->COA_Client[div].testCond.step[i].fault_can_check_flag);
		fprintf(fp, "can_tx_stop : %d\n",
			myData->COA_Client[div].testCond.step[i].can_tx_stop);
		fprintf(fp, "pattern_max_val : %f\n",
			myData->COA_Client[div].testCond.step[i].pattern_max_val);
		fprintf(fp, "pattern_file_size : %ld\n",
			myData->COA_Client[div].testCond.step[i].pattern_file_size);
		fprintf(fp, "pattern_file_checksum : %ld\n",
			myData->COA_Client[div].testCond.step[i].pattern_file_checksum);
		fprintf(fp,"\n");
	}
		
	fprintf(fp, "TestCond_ETC\n");
	fprintf(fp, "test_cond_file_size : %ld\n",
		myData->COA_Client[div].testCond.test_cond_file_size);
	fprintf(fp, "test_cond_file_checksum : %ld\n",
		myData->COA_Client[div].testCond.test_cond_file_checksum);

	fclose(fp);
	return 0;
}

int COA_Client_TestCond_write_V100D_pattern(int div)
{
#ifdef __COA_VER_100D__
	char fileName[256];
	int i, j, ch;
	FILE *fp;
	int k = 0;

	ch = 2;
	for(i=0; i < ch; i++) {
		memset(fileName, 0, sizeof(fileName));

		sprintf(fileName, "./COA%d_Client_conversion_V100D_pattern_ch%d", div+1,i+1);
		if((fp = fopen(fileName, "w")) == NULL) {
			printf("Can not open COA%d_Client_TestCond file(write)\n", div+1);
			return -1;
		}
		fprintf(fp, "COA%d_Client_Receive_Data_V100D_pattern\n", div+1);
			for(k=0; k < MAX_P1_AUX_FUNCTION; k++) {
				for(j=0; j < 10; j++) {
					fprintf(fp, "%02d, %d, %d, %d, %d, %f\n", k+1,
						myData->COA_Client[1].testCond.step[j]
						.reference[0].aux_func_div[k],
						myData->COA_Client[1].testCond.step[j]
						.reference[0].aux_compare_type[k],
						myData->COA_Client[1].testCond.step[j]
						.reference[0].aux_data_type[k],
						myData->COA_Client[1].testCond.step[j]
						.reference[0].aux_branch[k],
						myData->COA_Client[1].testCond.step[j]
						.reference[0].aux_value[k]);
				}
			}
		fprintf(fp,"\n");
		fclose(fp);
	}
#endif
	return 0;
}

#if defined __COA_VER_100B__ || defined __COA_VER_100B2__ || defined __COA_VER_100C__ || defined __COA_VER_100D__
#else //COA_VER_100F~
int COA_Client_TestCond_write_TimeSch(int div)
{
//kjh_160418s
	char fileName[256];
	int i = 0;
	FILE *fp;

	memset(fileName, 0, sizeof(fileName));

	sprintf(fileName, "./COA%d_Client_TestCond_TimeSch", div+1);

	if((fp = fopen(fileName, "w")) == NULL) {
		printf("Can not open COA%d_Client_TestCond_TimeSch file(write)\n", div+1);
		return -1;
	}

	fprintf(fp, "COA%d_Client_Receive_Data_V100F_TimeSch\n", div+1);
	fprintf(fp,"\n");
		
	fprintf(fp, "TestCond_Header\n");
	fprintf(fp, "totalStep : %d\n", 
		myData->COA_Client[div].testCond.header.totalStep);
	fprintf(fp, "totalPatternCount : %d\n", 
		myData->COA_Client[div].testCond.header.totalPatternCount);
	fprintf(fp, "totalTimeSchCount : %d\n", 
		myData->COA_Client[div].testCond.header.totalTimeSchCount);
	fprintf(fp, "reserved : %d\n", 
		myData->COA_Client[div].testCond.header.reserved);
	fprintf(fp, "reserverd1[2] : %ld, %ld\n", 
		myData->COA_Client[div].testCond.header.reserved1[0],
		myData->COA_Client[div].testCond.header.reserved1[1]);
	fprintf(fp, "\n");
/*		
	fprintf(fp, "Common_Safety\n");
	fprintf(fp, "faultLowerV : %ld\n", 
		myData->COA_Client[div].testCond.safety.faultLowerV);
	fprintf(fp, "faultUpperV : %ld\n", 
		myData->COA_Client[div].testCond.safety.faultUpperV);
	//fprintf(fp, "reserved1 : %ld\n", 
	//	myData->COA_Client[div].testCond.safety.reserved1);
	fprintf(fp, "faultCompAuxV : %ld\n", 
		myData->COA_Client[div].testCond.safety.faultCompAuxV);
	fprintf(fp, "faultUpperI : %ld\n", 
		myData->COA_Client[div].testCond.safety.faultUpperI);
	fprintf(fp, "faultLowerTemp : %ld\n", 
		myData->COA_Client[div].testCond.safety.faultLowerTemp);
	fprintf(fp, "faultUpperTemp : %ld\n", 
		myData->COA_Client[div].testCond.safety.faultUpperTemp);
	fprintf(fp, "faultUpper_AmpareHour : %ld\n", 
		myData->COA_Client[div].testCond.safety.faultUpper_AmpareHour);
	fprintf(fp, "faultUpperP : %ld\n", 
		myData->COA_Client[div].testCond.safety.faultUpperP);
	fprintf(fp, "faultUpper_WattHour : %ld\n", 
		myData->COA_Client[div].testCond.safety.faultUpper_WattHour);
	fprintf(fp,"\n");

	fprintf(fp, "CAN_Common_Safety\n");
	fprintf(fp, "index, Division, Compare_Type, Data_Type, Value\n");
	for(i=0; i < MAX_P1_CAN_FUNCTION; i++) {
		fprintf(fp, "%02d, %d, %d, %d, %f\n", i+1,
			myData->COA_Client[div].testCond.safety.can_func_div[i],
			myData->COA_Client[div].testCond.safety.can_compare_type[i],
			myData->COA_Client[div].testCond.safety.can_data_type[i],
			myData->COA_Client[div].testCond.safety.can_value[i]);
	}
	fprintf(fp,"\n");

	fprintf(fp, "Aux_Common_Safety\n");
	fprintf(fp, "index, Division, Compare_Type, Data_Type, Value\n");
	for(i=0; i < MAX_P1_AUX_FUNCTION; i++) {
		fprintf(fp, "%02d, %d, %d, %d, %f\n", i+1,
			myData->COA_Client[div].testCond.safety.aux_func_div[i],
			myData->COA_Client[div].testCond.safety.aux_compare_type[i],
			myData->COA_Client[div].testCond.safety.aux_data_type[i],
			myData->COA_Client[div].testCond.safety.aux_value[i]);
	}
	fprintf(fp,"\n");

	fprintf(fp, "reserved2[4] : %ld, %ld, %ld, %ld\n", 
		myData->COA_Client[div].testCond.safety.reserved2[0],
		myData->COA_Client[div].testCond.safety.reserved2[1],
		myData->COA_Client[div].testCond.safety.reserved2[2],
		myData->COA_Client[div].testCond.safety.reserved2[3]);
	fprintf(fp,"\n");
*/
	fprintf(fp, "TestCond_Step\n");
	fprintf(fp,"\n");

	for(i=0; i < myData->COA_Client[div].testCond.header.totalStep; i++) {
		fprintf(fp, "Step_Header : %d\n", i+1);
		fprintf(fp, "type : %d\n",
			myData->COA_Client[div].testCond.step[i].header.type);
		fprintf(fp, "stepNo : %d\n",
			myData->COA_Client[div].testCond_time.time_step[i].stepNo);
		fprintf(fp, "timeInit : %d\n",
			myData->COA_Client[div].testCond_time.time_step[i].timeInit);
		fprintf(fp, "reserved : %d\n",
			myData->COA_Client[div].testCond_time.time_step[i].reserved);
		fprintf(fp, "wait_day : %d\n",
			myData->COA_Client[div].testCond_time.time_step[i].wait_day);
		fprintf(fp, "wait_hour : %d\n",
			myData->COA_Client[div].testCond_time.time_step[i].wait_hour);
		fprintf(fp, "wait_min : %d\n",
			myData->COA_Client[div].testCond_time.time_step[i].wait_min);
		fprintf(fp, "wait_sec : %d\n",
			myData->COA_Client[div].testCond_time.time_step[i].wait_sec);
		fprintf(fp, "reserved2[0][1] : %ld, %ld\n",
			myData->COA_Client[div].testCond_time.time_step[i].reserved2[0],
			myData->COA_Client[div].testCond_time.time_step[i].reserved2[1]);
		fprintf(fp,"\n");
	}
/*	for(i=0; i < myData->COA_Client[div].testCond.header.totalStep; i++) {
		for(j=0;
			j < myData->COA_Client[div].testCond.header.totalTimeSchCount;
			j++) {
			if(myData->COA_Client[div].testCond_time.time_step[j].stepNo != i+1) continue;
			fprintf(fp, "Step_Header : %d\n", i+1);
			fprintf(fp, "type : %d\n",
				myData->COA_Client[div].testCond.step[i].header.type);
			fprintf(fp, "stepNo : %d\n",
				myData->COA_Client[div].testCond_time.time_step[j].stepNo);
			fprintf(fp, "timeInit : %d\n",
				myData->COA_Client[div].testCond_time.time_step[j].timeInit);
			fprintf(fp, "reserved : %d\n",
				myData->COA_Client[div].testCond_time.time_step[j].reserved);
			fprintf(fp, "wait_day : %d\n",
				myData->COA_Client[div].testCond_time.time_step[j].wait_day);
			fprintf(fp, "wait_hour : %d\n",
				myData->COA_Client[div].testCond_time.time_step[j].wait_hour);
			fprintf(fp, "wait_min : %d\n",
				myData->COA_Client[div].testCond_time.time_step[j].wait_min);
			fprintf(fp, "wait_sec : %d\n",
				myData->COA_Client[div].testCond_time.time_step[j].wait_sec);
			fprintf(fp, "reserved2[0][1] : %ld, %ld\n",
				myData->COA_Client[div].testCond_time.time_step[i].reserved2[0],
				myData->COA_Client[div].testCond_time.time_step[i].reserved2[1]);
			fprintf(fp,"\n");
		}
	}
	*/	
	fprintf(fp, "TestCond_ETC\n");
	fprintf(fp, "test_cond_file_size : %ld\n",
		myData->COA_Client[div].testCond_time.test_cond_time_file_size);
	fprintf(fp, "test_cond_file_checksum : %ld\n",
		myData->COA_Client[div].testCond_time.test_cond_time_file_checksum);

	fclose(fp);
	return 0;
//kjh_160418e
}
#endif

int write_IO_Print(int k)
{
	struct	tm *tm;
	char	*asciitime, fileName[256];
	time_t  curtime;
	int		i,j= 0;
	FILE	*fp;

	time(&curtime);
	asciitime = ctime(&curtime);
	asciitime[24] = 0;
	tm = localtime(&curtime);

	memset(fileName, 0, sizeof(fileName));

	sprintf(fileName, "./write_IO_Print.csv");

	fp = fopen(fileName, "a");
	if(fp == NULL || fp < 0) {
		printf("Can not open write_IO_Print\n");
		return -1;
	}

	fprintf(fp, "time %02d%02d %s\n", tm->tm_mon+1, tm->tm_mday, asciitime);
		
	fprintf(fp, "in_address\n");
	for(i=0; i < MAX_DIO_IN_BYTES; i++) {
		fprintf(fp, "%x ", myData->dio.function_set.in_address[i]);
	}
	fprintf(fp, "\n");

	fprintf(fp, "in bytes\n");
	for(i=0; i < 4; i++) {
		for(j=0; j < 8; j++) {
			fprintf(fp, "%03x:", myData->dio.function_set.in_address[i*8+j]);
			fprintf(fp, "%02x ", myData->dio.in.bytes[i*8+j]);
		}
		fprintf(fp, "\n");
	}
	fprintf(fp, "\n");

	fprintf(fp, "out_address\n");
	for(i=0; i < MAX_DIO_OUT_BYTES; i++) {
		fprintf(fp, "%x ", myData->dio.function_set.out_address[i]);
	}
	fprintf(fp, "\n");
	fflush(fp);
	fclose(fp);
	return 0;
}

int KeyInput(void)
{
	int i, rtn;

	rtn = 0;

	i = getc(stdin);
	switch(i) {
		case '1':
			bd = 0;
			for(i=0; i < 10; i++) {
				min[i] = 0;
				max[i] = 0;
			}
			break;
		case '2':
			bd = 1;
			for(i=0; i < 10; i++) {
				min[i] = 0;
				max[i] = 0;
			}
			break;
		//case '3':
		//	bd = 2;
		//	for(i=0; i < 10; i++) {
		//		min[i] = 0;
		//		max[i] = 0;
		//	}
		//	break;
		case '3':
			//bd = 3;
			//for(i=0; i < 10; i++) {
			//	min[i] = 0;
			//	max[i] = 0;
			//}
			myData->cData[0].signal[C_SIG_CMD_GOTO_STEP] = P1;
			myData->cData[0].misc.user_branch_stepNo = 3;
			break;
		case '4':
			//myData->mData.signal[M_SIG_BMS_EOL_PHASE] = P1;
			myData->cData[0].signal[C_SIG_CMD_GOTO_STEP] = P1;
			myData->cData[0].misc.user_branch_stepNo = 4;
			break;
		case '5':
			//myData->mData.signal[M_SIG_BMS_EOL_PHASE] = P100;
			myData->cData[0].signal[C_SIG_CMD_GOTO_STEP] = P1;
			myData->cData[0].misc.user_branch_stepNo = 5;
			break;
		case '6':
			//myData->mData.signal[M_SIG_BMS_EOL_FLASH_PHASE] = P100;
			//myData->mData.signal[M_SIG_BMS_EOL_FLASH_PHASE2] = P0;
			myData->cData[0].signal[C_SIG_CMD_GOTO_STEP] = P1;
			myData->cData[0].misc.user_branch_stepNo = 6;
			break;
		case '7':
			//myData->mData.signal[M_SIG_BMS_EOL_FLASH_PHASE] = P1;
			//memset((char *)&myData->COA_Client[0].misc.write_pack_id[0], 0, 40);
			//strcpy((char *)&myData->COA_Client[0].misc.write_pack_id[0],
			//	"KJG_test_120130");
			myData->cData[0].signal[C_SIG_CMD_GOTO_STEP] = P1;
			myData->cData[0].misc.user_branch_stepNo = 7;
			break;
		case '8':
			//myData->mData.signal[M_SIG_BMS_EOL_FLASH_PHASE] = P0;
			//myData->mData.signal[M_SIG_BMS_EOL_FLASH_PHASE2] = P0;
			myData->cData[0].signal[C_SIG_CMD_GOTO_STEP] = P1;
			myData->cData[0].misc.user_branch_stepNo = 2;
			break;
		case '9':
			//myData->mData.signal[M_SIG_BMS_EOL_FLASH_PHASE] = P0;
			//myData->mData.signal[M_SIG_BMS_EOL_FLASH_PHASE2] = P0;
			myData->cData[0].signal[C_SIG_CMD_GOTO_STEP] = P1;
			myData->cData[0].misc.user_branch_stepNo = 10;
			break;
		case 'a':
			//myData->cData[0].op.runTime += (360000 * 24 * 5); //5Day
			//myData->cData[0].op.totalRunTime += (360000 * 24 * 5); //5Day
			myData->cData[0].op.runTime += (360000); //1h
			myData->cData[0].op.totalRunTime += (360000); //1h
			break;
		case 'b': //kjg_140916_s
			myData->test_val_l[0]++;
			switch(myData->test_val_l[0]) {
				case 1:
					myData->test_val_l[1] = 0; //fault_clear 0:idle, 1:clear
					myData->test_val_l[2] = 0; //e48_stat_error 0:idle, x:fault
					myData->test_val_l[3] = 0; //e48_nm 0:idle, 1:run, 2:req
					myData->test_val_l[4] = 1; //pc_nm 0:idle, 1:run, 2:req
					myData->test_val_l[5] = 0; //rp_nm 0:idle, 1:run, 2:req
					myData->test_val_l[6] = 15000; //charge_v
					myData->test_val_l[7] = 100; //discharge_v
					myData->test_val_l[8] = 2; //set_mode 2:cc, 6:cp
					myData->test_val_l[9] = 0; //cmd_i
					myData->test_val_l[10] = 0; //cmd_p
					myData->test_val_l[11] = 0; //cmd_t
					myData->test_val_l[12] = 0;
						//set_control 0:idle, 1:run, 2:pause, 3:continue, 4:end
					myData->test_val_l[13] = 100; //receive_parsing_period
					myData->test_val_l[14] = 10; //can_transmit_period
					break;
				case 2:
					myData->test_val_l[1] = 0; //fault_clear 0:idle, 1:clear
					myData->test_val_l[2] = 0; //e48_stat_error 0:idle, x:fault
					myData->test_val_l[3] = 0; //e48_nm 0:idle, 1:run, 2:req
					myData->test_val_l[4] = 1; //pc_nm 0:idle, 1:run, 2:req
					myData->test_val_l[5] = 0; //rp_nm 0:idle, 1:run, 2:req
					myData->test_val_l[6] = 15000; //charge_v
					myData->test_val_l[7] = 100; //discharge_v
					myData->test_val_l[8] = 2; //set_mode 2:cc, 6:cp
					myData->test_val_l[9] = 0; //cmd_i
					myData->test_val_l[10] = 0; //cmd_p
					myData->test_val_l[11] = 0; //cmd_t
					myData->test_val_l[12] = 1;
						//set_control 0:idle, 1:run, 2:pause, 3:continue, 4:end
					myData->test_val_l[13] = 100; //receive_parsing_period
					myData->test_val_l[14] = 10; //can_transmit_period
					break;
				case 3:
					myData->test_val_l[1] = 0; //fault_clear 0:idle, 1:clear
					myData->test_val_l[2] = 0; //e48_stat_error 0:idle, x:fault
					myData->test_val_l[3] = 0; //e48_nm 0:idle, 1:run, 2:req
					myData->test_val_l[4] = 1; //pc_nm 0:idle, 1:run, 2:req
					myData->test_val_l[5] = 0; //rp_nm 0:idle, 1:run, 2:req
					myData->test_val_l[6] = 15000; //charge_v
					myData->test_val_l[7] = 100; //discharge_v
					myData->test_val_l[8] = 2; //set_mode 2:cc, 6:cp
					myData->test_val_l[9] = 1000; //cmd_i
					myData->test_val_l[10] = 0; //cmd_p
					myData->test_val_l[11] = 0; //cmd_t
					myData->test_val_l[12] = 1;
						//set_control 0:idle, 1:run, 2:pause, 3:continue, 4:end
					myData->test_val_l[13] = 100; //receive_parsing_period
					myData->test_val_l[14] = 10; //can_transmit_period
					break;
				case 4:
					myData->test_val_l[1] = 0; //fault_clear 0:idle, 1:clear
					myData->test_val_l[2] = 0; //e48_stat_error 0:idle, x:fault
					myData->test_val_l[3] = 0; //e48_nm 0:idle, 1:run, 2:req
					myData->test_val_l[4] = 1; //pc_nm 0:idle, 1:run, 2:req
					myData->test_val_l[5] = 0; //rp_nm 0:idle, 1:run, 2:req
					myData->test_val_l[6] = 15000; //charge_v
					myData->test_val_l[7] = 100; //discharge_v
					myData->test_val_l[8] = 2; //set_mode 2:cc, 6:cp
					myData->test_val_l[9] = 0; //cmd_i
					myData->test_val_l[10] = 0; //cmd_p
					myData->test_val_l[11] = 0; //cmd_t
					myData->test_val_l[12] = 1;
						//set_control 0:idle, 1:run, 2:pause, 3:continue, 4:end
					myData->test_val_l[13] = 100; //receive_parsing_period
					myData->test_val_l[14] = 10; //can_transmit_period
					break;
				case 5:
					myData->test_val_l[1] = 0; //fault_clear 0:idle, 1:clear
					myData->test_val_l[2] = 0; //e48_stat_error 0:idle, x:fault
					myData->test_val_l[3] = 0; //e48_nm 0:idle, 1:run, 2:req
					myData->test_val_l[4] = 1; //pc_nm 0:idle, 1:run, 2:req
					myData->test_val_l[5] = 0; //rp_nm 0:idle, 1:run, 2:req
					myData->test_val_l[6] = 15000; //charge_v
					myData->test_val_l[7] = 100; //discharge_v
					myData->test_val_l[8] = 2; //set_mode 2:cc, 6:cp
					myData->test_val_l[9] = 0; //cmd_i
					myData->test_val_l[10] = 0; //cmd_p
					myData->test_val_l[11] = 0; //cmd_t
					myData->test_val_l[12] = 4;
						//set_control 0:idle, 1:run, 2:pause, 3:continue, 4:end
					myData->test_val_l[13] = 100; //receive_parsing_period
					myData->test_val_l[14] = 10; //can_transmit_period
					break;
				case 6:
					myData->test_val_l[1] = 0; //fault_clear 0:idle, 1:clear
					myData->test_val_l[2] = 0; //e48_stat_error 0:idle, x:fault
					myData->test_val_l[3] = 0; //e48_nm 0:idle, 1:run, 2:req
					myData->test_val_l[4] = 1; //pc_nm 0:idle, 1:run, 2:req
					myData->test_val_l[5] = 0; //rp_nm 0:idle, 1:run, 2:req
					myData->test_val_l[6] = 15000; //charge_v
					myData->test_val_l[7] = 100; //discharge_v
					myData->test_val_l[8] = 2; //set_mode 2:cc, 6:cp
					myData->test_val_l[9] = 0; //cmd_i
					myData->test_val_l[10] = 0; //cmd_p
					myData->test_val_l[11] = 0; //cmd_t
					myData->test_val_l[12] = 0;
						//set_control 0:idle, 1:run, 2:pause, 3:continue, 4:end
					myData->test_val_l[13] = 100; //receive_parsing_period
					myData->test_val_l[14] = 10; //can_transmit_period
					break;
				default:
					break;
			}
			break;
		case 'c':
//			for(i=0; i < MAX_TEST_VALUE; i++) myData->test_val_l[i] = 0;
			myData->cData[1].signal[C_SIG_CMD_STOP] = P1;
			break; //kjg_140916_e
		case 'd':
			myData->cData[0].signal[C_SIG_CMD_CHAMBER_CONTINUE] = P1;
			break;
		case 'e':
			myData->cData[1].signal[C_SIG_CMD_CHAMBER_CONTINUE] = P1;
			break;
		case 'q':
			rtn = (-1);
			break;
		case 's':
			for(i=0; i < MAX_TEST_VALUE; i++) {
				myData->test_val_i[0][i] = 0;
				myData->test_val_i[1][i] = 0;
				myData->test_val_i[2][i] = 0;
				myData->test_val_i[3][i] = 0;
				myData->test_val_i[4][i] = 0;
				myData->test_val_i[5][i] = 0;
				myData->test_val_i[6][i] = 0;
				myData->test_val_i[7][i] = 0;
			}
			//myData->mData.signal[M_SIG_EXIT_PHASE] = P3;
			//myData->mData.signal[M_SIG_BMS_EOL3_PHASE] = P1;
			break;
		//case 't': myData->CAN.signal[0][CAN_SIG_ECU_ID_REQUEST] = P1; break;
		case 't': file_write = 1; break;
		case 'f': file_write = 2; break; //TestCond_write_V1009(0);
		case 'F': file_write = 3; break; //TestCond_write_V100B(0);
		case 'G': file_write = 4; break; //TestCond_write_V100C(0);
		case 'H': file_write = 5; break; //TestCond_write_V100D(0);
		case 'K': file_write = 6; break; //TestCond_write_V100D_Pattern(0);
		case 'J': file_write = 7; break; //write_IO_Print(0);
		case 'r':
			myData->cData[0].op.state = C_RUN;
			myData->cData[0].op.stepType = STEP_END;
			myData->cData[0].op.phase = P102;
			break;
		case 'n':
			//myData->test_val_l[0] = 15000;//temp
			//myData->test_val_l[1] = 100;//soc
			myData->test_val_l[1] = 1234;
			break;
		case 'N':
			//myData->test_val_l[0] = 10;//temp
			//myData->test_val_l[1] = -65;//temp
			myData->test_val_l[1] = 123;
			break;
		case 'l':
			myData->test_val_l[0]++;
			switch(myData->test_val_l[0]) {
				case 1:
					myData->test_val_l[1] = -20000;
					break;
				case 2:
					myData->test_val_l[1] = -10000;
					break;
				case 3:
					myData->test_val_l[1] = 0;
					break;
				case 4:
					myData->test_val_l[1] = 10000;
					break;
				case 5:
					myData->test_val_l[1] = 20000;
					break;
				case 6:
					myData->test_val_l[1] = 30000;
					break;
				case 7:
					myData->test_val_l[1] = 40000;
					break;
				case 8:
					myData->test_val_l[1] = 50000;
					break;
				case 9:
					myData->test_val_l[1] = 60000;
					break;
				case 10:
					myData->test_val_l[1] = -40000;
					break;
				default:
					myData->test_val_l[0] = 0;
					myData->test_val_l[1] = 0;
					break;
			}
			break;
			/*
		case 'l':
			myData->test_val_l[0]++;
			switch(myData->test_val_l[0]) {
				case 1:
					myData->test_val_l[1] = 5000000;
					break;
				case 2:
					myData->test_val_l[1] = 15000000;
					break;
				case 3:
					myData->test_val_l[1] = 25000000;
					break;
				case 4:
					myData->test_val_l[1] = 35000000;
					break;
				case 5:
					myData->test_val_l[1] = 45000000;
					break;
				case 6:
					myData->test_val_l[1] = 55000000;
					break;
				case 7:
					myData->test_val_l[1] = 65000000;
					break;
				case 8:
					myData->test_val_l[1] = 75000000;
					break;
				case 9:
					myData->test_val_l[1] = 85000000;
					break;
				case 10:
					myData->test_val_l[1] = 95000000;
					break;
				case 11:
					myData->test_val_l[1] = 105000000;
					break;
				default:
					myData->test_val_l[0] = 0;
					myData->test_val_l[1] = 0;
					break;
			}
			break;
			*/
		case 'm':
			myData->test_val_l[15]++;
			switch(myData->test_val_l[15]) {
				case 1:
					myData->test_val_l[0] = 1000; //ch1 temp
					myData->test_val_l[1] = -2000; //ch1 temp
					myData->test_val_l[2] = 3000; //ch1 temp
					myData->test_val_l[6] = 4000; //ch2 temp
					myData->test_val_l[7] = -5000; //ch2 temp
					myData->test_val_l[8] = 6000; //ch2 temp
					break;
				case 2:
					myData->test_val_l[3] = 10000000; //ch1 th
					myData->test_val_l[4] = -20000000; //ch1 th
					myData->test_val_l[5] = 30000000; //ch1 th
					myData->test_val_l[9] = 40000000; //ch2 th
					myData->test_val_l[10] = -50000000; //ch2 th
					myData->test_val_l[11] = 60000000; //ch2 th
					break;
				case 3:
					myData->test_val_l[0] = 8000;
					myData->test_val_l[6] = -9000;
					break;
				case 4:
					myData->test_val_l[1] = 7000;
					myData->test_val_l[7] = -8000;
					break;
				case 5:
					myData->test_val_l[2] = 6000;
					myData->test_val_l[8] = -7000;
					break;
				case 6:
					myData->test_val_l[3] = 5000000;
					myData->test_val_l[9] = -6000000;
					break;
				case 7:
					myData->test_val_l[4] = 6000000;
					myData->test_val_l[10] = -7000000;
					break;
				case 8:
					myData->test_val_l[5] = 7000000;
					myData->test_val_l[11] = -8000000;
					break;
				case 9:
					break;
				default:
					for(i=0; i < 16; i++) {
						myData->test_val_l[i] = 0;
					}
					break;
			}
		case 'o':
			for(i=0; i < 100; i++) {
				myData->mData.runningTime[0][i] = 0;
				myData->mData.runningTime[1][i] = 0;
				myData->mData.runningTime[2][i] = 0;
				myData->mData.runningTime[3][i] = 0;
				myData->mData.runningTime[4][i] = 0;
				myData->mData.runningTime[5][i] = 0;
			}
			break;
		case 'w':	//reset
			myData->test_val_i[1][0] = 0;	//Vsens = 50V
			myData->test_val_i[1][1] = 0;	//Isens = 100A
			break;
		case 'W':	//Charge_CC_test
			myData->test_val_i[1][0] = 1;	//Vsens = 100mV/sec ++
			myData->test_val_i[1][1] = 0;	//Isens = 100A
			break;
		case 'x':	//Charge_CV_test
			myData->test_val_i[1][0] = 3;	//Vsens = stop
			myData->test_val_i[1][1] = 2;	//Isens = 100mA/sec --
			break;
		case 'X':	//DisCharge CC_test
			myData->test_val_i[1][0] = 2;	//Vsens = 100mV/sec --
			myData->test_val_i[1][1] = 0;	//Isens = -100A
			break;
		case 'y':	//DisCharge CV_test
			myData->test_val_i[1][0] = 3;	//Vsens = stop
			myData->test_val_i[1][1] = 1;	//Isens = 100mA/sec ++
			break;
		default: break;
	}
	return rtn;
}

int main(void)
{
    int	retval, rtn, i;
    struct timeval tv;
    fd_set rfds;

	if(Open_SystemMemory(0) < 0) return -1;

	bd = 0;

	for(i=0; i < 10; i++) {
		min[i] = 0;
		max[i] = 0;
	}

    while(1) {
	    tv.tv_sec = 0;
	    tv.tv_usec = 800000;
	    //tv.tv_usec = 300000;
	    FD_ZERO(&rfds);
	    FD_SET(0, &rfds);
		retval = select(1, &rfds, NULL, NULL, &tv);
	    if(retval == 0) {
			Test_Print();
		} else {
			printf("rtn %d\n", retval);
			rtn = KeyInput();
			if(rtn < 0) break;
		}
    }

	Close_SystemMemory();
    return 0;
}
