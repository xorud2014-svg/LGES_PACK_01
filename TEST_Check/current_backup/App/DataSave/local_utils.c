#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include "../../INC/datastore.h"
#include "userlog.h"
#include "local_utils.h"

extern volatile S_SYSTEM_DATA	*myData;
extern volatile S_DATA_SAVE		*myPs;
extern char psName[PROCESS_NAME_SIZE];

void Init_SystemMemory(void)
{
	memset((char *)&psName[0], 0, PROCESS_NAME_SIZE);
	strcpy(psName, "DataSave");

	memset((char *)&myPs->signal, 0, MAX_SIGNAL);

	myPs->misc.processPointer = (int)&myData;
}

int	Read_DataSave_Config(void)
{
	char temp[32], buf[8], fileName[128];
    int tmp, i;
    FILE *fp;

	memset(fileName, 0, sizeof(fileName));
	sprintf(fileName, "/root/%s/config/parameter/DataSave_Config",
		myData->AppControl.misc.path1);
	// /root/system_data/config/parameter/DataSave_Config
    if((fp = fopen(fileName, "r")) == NULL) {
		userlog(DEBUG_LOG, psName, "DataSave_Config file read error\n");
		return -1;
	}

    tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->config.resultData_saveFlag = (unsigned char)atoi(buf);

    tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->config.monitoringData_saveFlag = (unsigned char)atoi(buf);

    tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->config.monitoringData_saveInterval = (unsigned char)atoi(buf);

    tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
	i = atoi(buf);
	if(i > MAX_RESULT_FILE_INDEX_FORMATION || i <= 0)
		i = MAX_RESULT_FILE_INDEX_FORMATION;
    myPs->config.maxSaveNo = (unsigned char)i;

    tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->config.checkData_saveFlag = (unsigned char)atoi(buf);

    fclose(fp);
	return 0;
}

int Open_ResultData_1(int p_ch)
{
	/*Simplication - shh_250417*/
	char cmd[128], path[128];
	int ch, i, j, groupNo;
	struct tm *tm;
	time_t t;
	FILE *fp;

	groupNo = 0; //kjg_w

	memset(path, 0, sizeof path);
	sprintf(path, "/root/%s/resultData/group%d/data0/ch%03d",
		myData->AppControl.misc.path1, groupNo+1, p_ch+1);
	// /root/system_data/resultData/group#/data0/ch###

	memset(cmd, 0, sizeof cmd);
	sprintf(cmd, "rm -rf %s/*", path);
	system(cmd);
	// rm -rf path/*

	myPs->resultData[p_ch].fileIndex = 1;
	myPs->resultData[p_ch].maxFileIndex = 1;
	myPs->resultData[p_ch].divisionCount = 0;
	myPs->resultData[p_ch].last_resultIndex = 0;
	time(&t);
	tm = localtime(&t);
	myPs->resultData[p_ch].open_year = (unsigned char)tm->tm_year-100;
	myPs->resultData[p_ch].open_month = (unsigned char)tm->tm_mon+1;
	myPs->resultData[p_ch].open_day = (unsigned char)tm->tm_mday;

	ch = myData->CellArray1[p_ch].number2 - 1;

	memset(cmd, 0, sizeof(cmd));
	sprintf(cmd, "%s/ch%03d_SaveEndData.csv", path, p_ch+1);
	// path/ch###_SaveEndData.csv
	fp = fopen(cmd, "w");
	if(fp == NULL || fp < 0) {
		userlog(DEBUG_LOG, psName, "Open_ResultData_1 error -1\n");
		return -1; //kjg_w
	}

	//COA_VER_100B2~
	fprintf(fp, "index, totalTime_day, totalTime, stepTime_day, stepTime, state, type, mode, select, code, grade, stepNo, Vsens, Isens, charge_AmpareHour, discharge_AmpareHour, capacitance, watt, charge_WattHour, discharge_WattHour, impedance, reservedCmd, external_comm_state, ch_output_state, ch_input_state, auxCount, canCount, totalCycle, elementCycle, accCycle1, accCycle2, accCycle3, accCycle4, accCycle5, multiCycle1, multiCycle2, multiCycle3, multiCycle4, multiCycle5, averageV, averageI, cvTime_day, cvTime, realDate, realClock, Vinput, Vpower, Vbus, out_mux_use, out_mux_backup, maxAuxT, minAuxT, diffAuxT, avgAuxT, SOC, dIsens, dMaxAuxVChNo, dMaxAuxV, dMinAuxVChNo, dMinAuxV \n");
	//shhw_230614
	fflush(fp);
	fclose(fp);

	if(myData->mData.config.installedTemp != 0) {
		memset(cmd, 0, sizeof(cmd));
		sprintf(cmd, "%s/ch%03d_SaveEndData_auxT.csv", path, p_ch+1);
		// path/ch###_SaveEndData_auxT.csv
		fp = fopen(cmd, "w");
		if(fp == NULL || fp < 0) {
			userlog(DEBUG_LOG, psName, "Open_ResultData_1 error -2\n");
			return -2;
		}
		//COA_VER_100B2~
		fprintf(fp, "index, totalTime_day, totalTime, stepTime_day, stepTime");
		//jhkw_181110s
		//for(i=0; i < myData->auxDataCount[ch][0]; i++) {
		//	fprintf(fp, ", value(%d)", (int)myData->auxSetData[i].auxChNo);
		for(i=0; i < myData->mData.config.installedTemp; i++) {
			if(p_ch+1 == myData->auxSetData[i].chNo) {
				fprintf(fp, ", value(%d)", (int)myData->auxSetData[i].auxChNo);
			}
		//jhkw_181110e
		}
		fprintf(fp, "\n");
		fflush(fp);
		fclose(fp);
	}

	//COA_VER_100F~
	//jhkw_161208
	//if((myData->mData.config.installedAuxV != 0)
	//	&& (myData->mData.config.installedTH != 0)) {
	if((myData->mData.config.installedAuxV != 0)
		|| (myData->mData.config.installedTH != 0)) {
		memset(cmd, 0, sizeof(cmd));
		sprintf(cmd, "%s/ch%03d_SaveEndData_auxV.csv", path, p_ch+1);
		// path/ch###_SaveEndData_auxV.csv
		fp = fopen(cmd, "w");
		if(fp == NULL || fp < 0) {
			userlog(DEBUG_LOG, psName, "Open_ResultData_1 error -3\n");
			return -3;
		}
		//COA_VER_100B2~
		fprintf(fp, "index, totalTime_day, totalTime, stepTime_day, stepTime");

		//COA_VER_100F~
		//j = myData->auxDataCount[ch][0] + myData->auxDataCount[ch][1]
		//	+ myData->auxDataCount[ch][2];
		j = myData->mData.config.installedTemp + myData->mData.config.installedAuxV
			+ myData->mData.config.installedTH;
		//for(i=myData->auxDataCount[ch][0]; i < j; i++) {
		//	fprintf(fp, ", value(%d)", (int)myData->auxSetData[i].auxChNo);
		for(i=myData->mData.config.installedTemp; i < j; i++) {
			if(p_ch+1 == myData->auxSetData[i].chNo) {
				fprintf(fp, ", value(%d)", (int)myData->auxSetData[i].auxChNo);
			}
		//jhkw_181110e
		}
		
		fprintf(fp, "\n");
		fflush(fp);
		fclose(fp);
	}

	if(myData->mData.config.installedCAN != 0) {
		memset(cmd, 0, sizeof(cmd));
		sprintf(cmd, "%s/ch%03d_SaveEndData_canMaster.csv", path, p_ch+1);
		// path/ch###_SaveEndData_canMaster.csv
		fp = fopen(cmd, "w");
		if(fp == NULL || fp < 0) {
			userlog(DEBUG_LOG, psName, "Open_ResultData_1 error -4\n");
			return -4;
		}
		//COA_VER_100B2~
		fprintf(fp, "index, totalTime_day, totalTime, stepTime_day, stepTime");
		for(i=0; i < myData->canReceiveDataCount[ch][0]; i++) {
			fprintf(fp, ", value(%d)", i+1);
		}
		fprintf(fp, "\n");
		fflush(fp);
		fclose(fp);

		memset(cmd, 0, sizeof(cmd));
		sprintf(cmd, "%s/ch%03d_SaveEndData_canSlave.csv", path, p_ch+1);
		// path/ch###_SaveEndData_canSlave.csv
		fp = fopen(cmd, "w");
		if(fp == NULL || fp < 0) {
			userlog(DEBUG_LOG, psName, "Open_ResultData_1 error -5\n");
			return -5;
		}
		//COA_VER_100B2~
		fprintf(fp, "index, totalTime_day, totalTime, stepTime_day, stepTime");
		j = myData->canReceiveDataCount[ch][0]
			+ myData->canReceiveDataCount[ch][1];
		for(i=myData->canReceiveDataCount[ch][0]; i < j; i++) {
			fprintf(fp, ", value(%d)", i+1);
		}
		fprintf(fp, "\n");
		fflush(fp);
		fclose(fp);
	}

	memset(cmd, 0, sizeof(cmd));
	sprintf(cmd, "%s/savingFileIndex_start.csv", path);
	// path/savingFileIndex_start.csv
	fp = fopen(cmd, "w");
	if(fp == NULL || fp < 0) {
		userlog(DEBUG_LOG, psName, "Open_ResultData_1 error -6\n");
		return -6;
	}

	fprintf(fp, "fileIndex %d, ", (int)myPs->resultData[p_ch].fileIndex);
	fprintf(fp, "resultIndex %ld, ", myData->cData[ch].op.resultIndex+1);
	fprintf(fp, "maxFileIndex %d, ", myPs->resultData[p_ch].maxFileIndex);
	fprintf(fp, "open_year %d, ", (int)myPs->resultData[p_ch].open_year);
	fprintf(fp, "open_month %d, ", (int)myPs->resultData[p_ch].open_month);
	fprintf(fp, "open_day %d\n", (int)myPs->resultData[p_ch].open_day);
	fflush(fp);
	fclose(fp);

	memset(cmd, 0, sizeof(cmd));
	sprintf(cmd, "%s/savingFileIndex_last.csv", path);
	// path/savingFileIndex_last.csv
	fp = fopen(cmd, "w");
	if(fp == NULL || fp < 0) {
		userlog(DEBUG_LOG, psName, "Open_ResultData_1 error -7\n");
		return -7;
	}

	fprintf(fp, "fileIndex %d, ", (int)myPs->resultData[p_ch].fileIndex);
	fprintf(fp, "resultIndex %ld, ", myData->cData[ch].op.resultIndex+1);
	fprintf(fp, "maxFileIndex %d, ", myPs->resultData[p_ch].maxFileIndex);
	fprintf(fp, "open_year %d, ", (int)myPs->resultData[p_ch].open_year);
	fprintf(fp, "open_month %d, ", (int)myPs->resultData[p_ch].open_month);
	fprintf(fp, "open_day %d\n", (int)myPs->resultData[p_ch].open_day);
	fflush(fp);
	fclose(fp);

	memset(cmd, 0, sizeof(cmd));
	sprintf(cmd, "%s/ch%03d_SaveData%03d.csv",
		path, p_ch+1, (int)myPs->resultData[p_ch].fileIndex);
	// path/ch###_SaveData###.csv
	fp = fopen(cmd, "w");
	if(fp == NULL || fp < 0) {
		userlog(DEBUG_LOG, psName, "Open_ResultData_1 error -10\n");
		return -10; //kjg_w
	}

	//COA_VER_100B2~
	fprintf(fp, "index, totalTime_day, totalTime, stepTime_day, stepTime, state, type, mode, select, code, grade, stepNo, Vsens, Isens, charge_AmpareHour, discharge_AmpareHour, capacitance, watt, charge_WattHour, discharge_WattHour, impedance, reservedCmd, external_comm_state, ch_output_state, ch_input_state, auxCount, canCount, totalCycle, elementCycle, accCycle1, accCycle2, accCycle3, accCycle4, accCycle5, multiCycle1, multiCycle2, multiCycle3, multiCycle4, multiCycle5, averageV, averageI, cvTime_day, cvTime, realDate, realClock, Vinput, Vpower, Vbus, out_mux_use, out_mux_backup, maxAuxT, minAuxT, diffAuxT, avgAuxT, SOC, dIsens, dMaxAuxVChNo, dMaxAuxV, dMinAuxVChNo, dMinAuxV \n");
	//shhw_230614
	fflush(fp);
	fclose(fp);

	if(myData->mData.config.installedTemp != 0) {
		memset(cmd, 0, sizeof(cmd));
		sprintf(cmd, "%s/ch%03d_SaveData%03d_auxT.csv",
			path, p_ch+1, (int)myPs->resultData[p_ch].fileIndex);
		// path/ch###_SaveData###_auxT.csv
		fp = fopen(cmd, "w");
		if(fp == NULL || fp < 0) {
			userlog(DEBUG_LOG, psName, "Open_ResultData_1 error -11\n");
			return -11; //kjg_w
		}
		//COA_VER_100B2~
		fprintf(fp, "index, totalTime_day, totalTime, stepTime_day, stepTime");
		//jhkw_181110s
		//for(i=0; i < myData->auxDataCount[ch][0]; i++) {
		//	fprintf(fp, ", value(%d)", (int)myData->auxSetData[i].auxChNo);
		for(i=0; i < myData->mData.config.installedTemp; i++) {
			if(p_ch+1 == myData->auxSetData[i].chNo) {
				fprintf(fp, ", value(%d)", (int)myData->auxSetData[i].auxChNo);
			}
		//jhkw_181110e
		}
		fprintf(fp, "\n");
		fflush(fp);
		fclose(fp);
	}

	//COA_VER_100F~
	//jhkw_161208
	//if((myData->mData.config.installedAuxV != 0)
	//	&& (myData->mData.config.installedTH != 0)) {
	if((myData->mData.config.installedAuxV != 0)
		|| (myData->mData.config.installedTH != 0)) {
		memset(cmd, 0, sizeof(cmd));
		sprintf(cmd, "%s/ch%03d_SaveData%03d_auxV.csv",
			path, p_ch+1, (int)myPs->resultData[p_ch].fileIndex);
		// path/ch###_SaveData###_auxV.csv
		fp = fopen(cmd, "w");
		if(fp == NULL || fp < 0) {
			userlog(DEBUG_LOG, psName, "Open_ResultData_1 error -12\n");
			return -12; //kjg_w
		}
		//COA_VER_100B2~
		fprintf(fp, "index, totalTime_day, totalTime, stepTime_day, stepTime");

		//COA_VER_100F~
		//j = myData->auxDataCount[ch][0] + myData->auxDataCount[ch][1]
		//	+ myData->auxDataCount[ch][2];
		j = myData->mData.config.installedTemp + myData->mData.config.installedAuxV
			+ myData->mData.config.installedTH;
		//for(i=myData->auxDataCount[ch][0]; i < j; i++) {
		//	fprintf(fp, ", value(%d)", (int)myData->auxSetData[i].auxChNo);
		for(i=myData->mData.config.installedTemp; i < j; i++) {
			if(p_ch+1 == myData->auxSetData[i].chNo) {
				fprintf(fp, ", value(%d)", (int)myData->auxSetData[i].auxChNo);
			}
		//jhkw_181110e
		}
		
		fprintf(fp, "\n");
		fflush(fp);
		fclose(fp);
	}

	if(myData->mData.config.installedCAN != 0) {
		memset(cmd, 0, sizeof(cmd));
		sprintf(cmd, "%s/ch%03d_SaveData%03d_canMaster.csv",
			path, p_ch+1, (int)myPs->resultData[p_ch].fileIndex);
		// path/ch###_SaveData###_canMaster.csv
		fp = fopen(cmd, "w");
		if(fp == NULL || fp < 0) {
			userlog(DEBUG_LOG, psName, "Open_ResultData_1 error -13\n");
			return -13; //kjg_w
		}
		//COA_VER_100B2~
		fprintf(fp, "index, totalTime_day, totalTime, stepTime_day, stepTime");
		for(i=0; i < myData->canReceiveDataCount[ch][0]; i++) {
			fprintf(fp, ", value(%d)", i+1);
		}
		fprintf(fp, "\n");
		fflush(fp);
		fclose(fp);

		memset(cmd, 0, sizeof(cmd));
		sprintf(cmd,
			"%s/ch%03d_SaveData%03d_canSlave.csv",
			path, p_ch+1, (int)myPs->resultData[p_ch].fileIndex);
		// path/ch###_SaveData###_canSlave.csv
		fp = fopen(cmd, "w");
		if(fp == NULL || fp < 0) {
			userlog(DEBUG_LOG, psName, "Open_ResultData_1 error -14\n");
			return -14; //kjg_w
		}
		//COA_VER_100B2~
		fprintf(fp, "index, totalTime_day, totalTime, stepTime_day, stepTime");
		j = myData->canReceiveDataCount[ch][0]
			+ myData->canReceiveDataCount[ch][1];
		for(i=myData->canReceiveDataCount[ch][0]; i < j; i++) {
			fprintf(fp, ", value(%d)", i+1);
		}
		fprintf(fp, "\n");
		fflush(fp);
		fclose(fp);
	}

	return 0;
}

int Open_ResultData_2(int p_ch)
{
	/*Simplication - shh_250417*/
	char cmd[128], path[128];
	int ch, i, j, groupNo;
	struct tm *tm;
	time_t t;
	FILE *fp;

	groupNo = 0; //kjg_w

	memset(path, 0, sizeof path);
	sprintf(path, "/root/%s/resultData/group%d/data0/ch%03d",
		myData->AppControl.misc.path1, groupNo+1, p_ch+1);
	// /root/system_data/resultData/group#/data0/ch###

	time(&t);
	tm = localtime(&t);

	if(myPs->resultData[p_ch].open_year != (unsigned char)tm->tm_year-100
		|| myPs->resultData[p_ch].open_month != (unsigned char)tm->tm_mon+1
		|| myPs->resultData[p_ch].open_day != (unsigned char)tm->tm_mday) {
		i = 1;
	} else i = 0;

	myPs->resultData[p_ch].divisionCount++;
	if(myPs->resultData[p_ch].divisionCount > MAX_DIVISION_COUNT) {
		i = 2;
	}

	if(myData->AppControl.config.debugType == 110) {
		if(myData->cData[p_ch].op.resultIndex != 0
			&& (myData->cData[p_ch].op.resultIndex % 10) == 0) i = 3;
	}

	if(i == 0) return 0;
	myPs->resultData[p_ch].divisionCount = 0;

	myPs->resultData[p_ch].open_year = (unsigned char)tm->tm_year-100;
	myPs->resultData[p_ch].open_month = (unsigned char)tm->tm_mon+1;
	myPs->resultData[p_ch].open_day = (unsigned char)tm->tm_mday;

	myPs->resultData[p_ch].fileIndex++;
	myPs->resultData[p_ch].maxFileIndex++;
	if(myPs->resultData[p_ch].fileIndex > MAX_RESULT_FILE_INDEX) {
		myPs->resultData[p_ch].fileIndex = 1;
	}

	ch = myData->CellArray1[p_ch].number2 - 1;

	memset(cmd, 0, sizeof cmd);
	sprintf(cmd, "%s/savingFileIndex_start.csv", path);
	// path/savingFileIndex_start.csv
	fp = fopen(cmd, "a");
	if(fp == NULL || fp < 0) {
		userlog(DEBUG_LOG, psName, "Open_ResultData_2 error -6\n");
		return -6; //kjg_w
	}

	fprintf(fp, "fileIndex %d, ", (int)myPs->resultData[p_ch].fileIndex);
	fprintf(fp, "resultIndex %ld, ", myPs->resultData[p_ch].last_resultIndex+1);
	fprintf(fp, "maxFileIndex %d, ", myPs->resultData[p_ch].maxFileIndex);
	fprintf(fp, "open_year %d, ", (int)myPs->resultData[p_ch].open_year);
	fprintf(fp, "open_month %d, ", (int)myPs->resultData[p_ch].open_month);
	fprintf(fp, "open_day %d\n", (int)myPs->resultData[p_ch].open_day);
	fflush(fp);
	fclose(fp);

	memset(cmd, 0, sizeof cmd);
	sprintf(cmd, "%s/savingFileIndex_last.csv", path);
	// path/savingFileIndex_last.csv
	fp = fopen(cmd, "w");
	if(fp == NULL || fp < 0) {
		userlog(DEBUG_LOG, psName, "Open_ResultData_2 error -7\n");
		return -7; //kjg_w
	}

	fprintf(fp, "fileIndex %d, ", (int)myPs->resultData[p_ch].fileIndex);
	fprintf(fp, "resultIndex %ld, ", myPs->resultData[p_ch].last_resultIndex+1);
	fprintf(fp, "maxFileIndex %d, ", myPs->resultData[p_ch].maxFileIndex);
	fprintf(fp, "open_year %d, ", (int)myPs->resultData[p_ch].open_year);
	fprintf(fp, "open_month %d, ", (int)myPs->resultData[p_ch].open_month);
	fprintf(fp, "open_day %d\n", (int)myPs->resultData[p_ch].open_day);
	fflush(fp);
	fclose(fp);

	memset(cmd, 0, sizeof cmd);
	sprintf(cmd, "rm -rf %s/ch%03d_SaveData%03d.csv",
		path, p_ch+1, (int)myPs->resultData[p_ch].fileIndex);
	//rm -rf path/ch###_SaveData###.csv
	system(cmd);

	if(myData->mData.config.installedTemp != 0) {
		memset(cmd, 0, sizeof cmd);
		sprintf(cmd, "rm -rf %s/ch%03d_SaveData%03d_auxT.csv",
			path, p_ch+1, (int)myPs->resultData[p_ch].fileIndex);
		//rm -rf path/ch###_SaveData###_auxT.csv
		system(cmd);
	}

	//COA_VER_100F~
	//jhkw_161208
	//if((myData->mData.config.installedAuxV != 0)
	//	&& (myData->mData.config.installedTH != 0)) {
	if((myData->mData.config.installedAuxV != 0)
		|| (myData->mData.config.installedTH != 0)) {
		memset(cmd, 0, sizeof cmd);
		sprintf(cmd, "rm -rf %s/ch%03d_SaveData%03d_auxV.csv",
			path, p_ch+1, (int)myPs->resultData[p_ch].fileIndex);
		//rm -rf path/ch###_SaveData###_auxV.csv
		system(cmd);
	}

	if(myData->mData.config.installedCAN != 0) {
		memset(cmd, 0, sizeof cmd);
		sprintf(cmd, "rm -rf %s/ch%03d_SaveData%03d_canMaster.csv",
			path, p_ch+1, (int)myPs->resultData[p_ch].fileIndex);
		//rm -rf path/ch###_SaveData###_canMaster.csv
		system(cmd);

		memset(cmd, 0, sizeof cmd);
		sprintf(cmd,
		"rm -rf %s/ch%03d_SaveData%03d_canSlave.csv",
			path, p_ch+1, (int)myPs->resultData[p_ch].fileIndex);
		//rm -rf path/ch###_SaveData###_canSlave.csv
		system(cmd);
	}

	memset(cmd, 0, sizeof(cmd));
	sprintf(cmd, "%s/ch%03d_SaveData%03d.csv",
		path, p_ch+1, (int)myPs->resultData[p_ch].fileIndex);
	// path/ch###_SaveData###.csv
	fp = fopen(cmd, "w");
	if(fp == NULL || fp < 0) {
		userlog(DEBUG_LOG, psName, "Open_ResultData_2 error -10\n");
		return -10; //kjg_w
	}

	//COA_VER_100B2~
	fprintf(fp, "index, totalTime_day, totalTime, stepTime_day, stepTime, state, type, mode, select, code, grade, stepNo, Vsens, Isens, charge_AmpareHour, discharge_AmpareHour, capacitance, watt, charge_WattHour, discharge_WattHour, impedance, reservedCmd, external_comm_state, ch_output_state, ch_input_state, auxCount, canCount, totalCycle, elementCycle, accCycle1, accCycle2, accCycle3, accCycle4, accCycle5, multiCycle1, multiCycle2, multiCycle3, multiCycle4, multiCycle5, averageV, averageI, cvTime_day, cvTime, realDate, realClock, Vinput, Vpower, Vbus, out_mux_use, out_mux_backup, maxAuxT, minAuxT, diffAuxT, avgAuxT, SOC, dIsens, dMaxAuxVChNo, dMaxAuxV, dMinAuxVChNo, dMinAuxV \n");
	//shhw_230614
	fflush(fp);
	fclose(fp);

	if(myData->mData.config.installedTemp != 0) {
		memset(cmd, 0, sizeof(cmd));
		sprintf(cmd, "%s/ch%03d_SaveData%03d_auxT.csv",
			path, p_ch+1, (int)myPs->resultData[p_ch].fileIndex);
		// path/ch###_SaveData###_auxT.csv
		fp = fopen(cmd, "w");
		if(fp == NULL || fp < 0) {
			userlog(DEBUG_LOG, psName, "Open_ResultData_2 error -11\n");
			return -11; //kjg_w
		}
		//COA_VER_100B2~
		fprintf(fp, "index, totalTime_day, totalTime, stepTime_day, stepTime");
		//jhkw_181110s
		//for(i=0; i < myData->auxDataCount[ch][0]; i++) {
		//	fprintf(fp, ", value(%d)", (int)myData->auxSetData[i].auxChNo);
		for(i=0; i < myData->mData.config.installedTemp; i++) {
			if(p_ch+1 == myData->auxSetData[i].chNo) {
				fprintf(fp, ", value(%d)", (int)myData->auxSetData[i].auxChNo);
			}
		//jhkw_181110e
		}
		fprintf(fp, "\n");
		fflush(fp);
		fclose(fp);
	}

	//COA_VER_100F~
	//jhkw_161208
	//if((myData->mData.config.installedAuxV != 0)
	//	&& (myData->mData.config.installedTH != 0)) {
	if((myData->mData.config.installedAuxV != 0)
		|| (myData->mData.config.installedTH != 0)) {
		memset(cmd, 0, sizeof(cmd));
		sprintf(cmd, "%s/ch%03d_SaveData%03d_auxV.csv",
			path, p_ch+1, (int)myPs->resultData[p_ch].fileIndex);
		// path/ch###_SaveData###_auxV.csv
		fp = fopen(cmd, "w");
		if(fp == NULL || fp < 0) {
			userlog(DEBUG_LOG, psName, "Open_ResultData_2 error -12\n");
			return -12; //kjg_w
		}
		//COA_VER_100B2~
		fprintf(fp, "index, totalTime_day, totalTime, stepTime_day, stepTime");

		//COA_VER_100F~
		//j = myData->auxDataCount[ch][0] + myData->auxDataCount[ch][1]
		//	+ myData->auxDataCount[ch][2];
		j = myData->mData.config.installedTemp + myData->mData.config.installedAuxV
			+ myData->mData.config.installedTH;
		//for(i=myData->auxDataCount[ch][0]; i < j; i++) {
		//	fprintf(fp, ", value(%d)", (int)myData->auxSetData[i].auxChNo);
		for(i=myData->mData.config.installedTemp; i < j; i++) {
			if(p_ch+1 == myData->auxSetData[i].chNo) {
				fprintf(fp, ", value(%d)", (int)myData->auxSetData[i].auxChNo);
			}
		//jhkw_181110e
		}
		
		fprintf(fp, "\n");
		fflush(fp);
		fclose(fp);
	}

	if(myData->mData.config.installedCAN != 0) {
		memset(cmd, 0, sizeof(cmd));
		sprintf(cmd, "%s/ch%03d_SaveData%03d_canMaster.csv",
			path, p_ch+1, (int)myPs->resultData[p_ch].fileIndex);
		// path/ch###_SaveData###_canMaster.csv
		fp = fopen(cmd, "w");
		if(fp == NULL || fp < 0) {
			userlog(DEBUG_LOG, psName, "Open_ResultData_2 error -13\n");
			return -13; //kjg_w
		}
		//COA_VER_100B2~
		fprintf(fp, "index, totalTime_day, totalTime, stepTime_day, stepTime");
		for(i=0; i < myData->canReceiveDataCount[ch][0]; i++) {
			fprintf(fp, ", value(%d)", i+1);
		}
		fprintf(fp, "\n");
		fflush(fp);
		fclose(fp);

		memset(cmd, 0, sizeof(cmd));
		sprintf(cmd, "%s/ch%03d_SaveData%03d_canSlave.csv",
			path, p_ch+1, (int)myPs->resultData[p_ch].fileIndex);
		// path/ch###_SaveData###_canSlave.csv
		fp = fopen(cmd, "w");
		if(fp == NULL || fp < 0) {
			userlog(DEBUG_LOG, psName, "Open_ResultData_2 error -14\n");
			return -14; //kjg_w
		}
		//COA_VER_100B2~
		fprintf(fp, "index, totalTime_day, totalTime, stepTime_day, stepTime");
		j = myData->canReceiveDataCount[ch][0]
			+ myData->canReceiveDataCount[ch][1];
		for(i=myData->canReceiveDataCount[ch][0]; i < j; i++) {
			fprintf(fp, ", value(%d)", i+1);
		}
		fprintf(fp, "\n");
		fflush(fp);
		fclose(fp);
	}

	return 0;
}

void Save_ResultData_1(int type)
{ //kjg_110921
	int msg, send_count, rcv_count, rtn, p_ch;

	msg = 1;
	rcv_count = 100;
	if(type == 1) { //software close routine
		if(myData->pulse_msg_1[msg].total_count > rcv_count) {
			rcv_count = myData->pulse_msg_1[msg].total_count;
		}
	}
	for(p_ch=0; p_ch < myData->mData.config.installedCh; p_ch++) {
		for(send_count=0; send_count < rcv_count; send_count++) {
			rtn = Save_PulseData(p_ch);
			if(rtn <= 0) break;
		}
	}

	if(myData->save_msg[msg].write_idx == myData->save_msg[msg].read_idx) {
		return;
	}

	rcv_count = 200;
	if(type == 1) { //software close routine
		if(myData->save_msg[msg].total_count > rcv_count) {
			rcv_count = myData->save_msg[msg].total_count;
		}
	}
	for(send_count=0; send_count < rcv_count; send_count++) {
		rtn = Save_ResultData_2();
		if(rtn <= 0) break;
	}

	if(type == 1) { //software close routine
		myData->pulse_msg_1[msg].total_count = 0;
		for(p_ch=0; p_ch < myData->mData.config.installedCh; p_ch++) {
			myData->pulse_msg_1[msg].count[p_ch] = 0;
		}

		myData->save_msg[msg].total_count = 0;
		for(p_ch=0; p_ch < myData->mData.config.installedCh; p_ch++) {
			myData->save_msg[msg].count[p_ch] = 0;
		}
	}
}

int Save_ResultData_2(void)
{
	/*Simplication - shh_250417*/
	char cmd[128], path[128];
	int ch, msg, idx, i, j, groupNo, count;
	FILE *fp;

	msg = 1;

	if(myData->save_msg[msg].write_idx == myData->save_msg[msg].read_idx) {
		return 0;
	}

	myData->save_msg[msg].read_idx++;
	if(myData->save_msg[msg].read_idx >= MAX_SAVE_MSG) {
		myData->save_msg[msg].read_idx = 0;
	}
	idx = myData->save_msg[msg].read_idx;

	if(myData->save_msg[msg].total_count > 0)
		myData->save_msg[msg].total_count--;

	ch = (int)myData->save_msg[msg].val[idx].chData.ch - 1;
	if(myData->save_msg[msg].count[ch] > 0)
		myData->save_msg[msg].count[ch]--;
	count = myData->save_msg[msg].count[ch];

	if(Open_ResultData_2(ch) < 0) {
		return -1;
	}

	groupNo = 0; //kjg_w

	memset(path, 0, sizeof path);
	sprintf(path, "/root/%s/resultData/group%d/data0/ch%03d",
		myData->AppControl.misc.path1, groupNo+1, ch+1);
	// /root/system_data/resultData/group#/data0/ch###

	memset(cmd, 0, sizeof cmd);
	sprintf(cmd, "%s/ch%03d_SaveData%03d.csv",
		path, ch+1, (int)myPs->resultData[ch].fileIndex);
	// path/ch###_SaveData###.csv
	fp = fopen(cmd, "a");
	if(fp == NULL || fp < 0) {
		userlog(DEBUG_LOG, psName, "Save_ResultData error -1 (ch:%d)\n", ch+1);
		return -2;
	}
	//COA_VER_100B2~
	fprintf(fp,
		"%ld, %ld, %ld, %ld, %ld, %d, %d, %d, %d, %d, %d, %d, %ld, %ld, %ld, %ld, %ld, %ld, %ld, %ld, %ld, %d, %d, %d, %d, %d, %d, %ld, %ld, %ld, %ld, %ld, %ld, %ld, %ld, %ld, %ld, %ld, %ld, %ld, %ld, %ld, %ld, %ld, %ld, %ld, %ld, %ld, %d, %d, %ld, %ld, %ld, %ld, %ld, %ld, %d, %ld, %d, %ld\n",
		myData->save_msg[msg].val[idx].chData.resultIndex,
		myData->save_msg[msg].val[idx].chData.totalRunTime_day,
		myData->save_msg[msg].val[idx].chData.totalRunTime,
		myData->save_msg[msg].val[idx].chData.runTime_day,
		myData->save_msg[msg].val[idx].chData.runTime,
		(int)myData->save_msg[msg].val[idx].chData.state,
		(int)myData->save_msg[msg].val[idx].chData.stepType,
		(int)myData->save_msg[msg].val[idx].chData.stepMode,
		(int)myData->save_msg[msg].val[idx].chData.select,
		(int)myData->save_msg[msg].val[idx].chData.code,
		(int)myData->save_msg[msg].val[idx].chData.grade,
		(int)myData->save_msg[msg].val[idx].chData.stepNo,
		myData->save_msg[msg].val[idx].chData.Vsens,
		myData->save_msg[msg].val[idx].chData.Isens,
		myData->save_msg[msg].val[idx].chData.charge_AmpareHour,
		myData->save_msg[msg].val[idx].chData.discharge_AmpareHour,
		myData->save_msg[msg].val[idx].chData.capacitance,
		myData->save_msg[msg].val[idx].chData.watt,
		myData->save_msg[msg].val[idx].chData.charge_WattHour,
		myData->save_msg[msg].val[idx].chData.discharge_WattHour,
		myData->save_msg[msg].val[idx].chData.z,
		(int)myData->save_msg[msg].val[idx].chData.reservedCmd,
		(int)myData->save_msg[msg].val[idx].chData.external_comm_state,
		(int)myData->save_msg[msg].val[idx].chData.ch_output_state,
		(int)myData->save_msg[msg].val[idx].chData.ch_input_state,
		(int)myData->save_msg[msg].val[idx].chData.auxDataCount,
		(int)myData->save_msg[msg].val[idx].chData.canReceiveDataCount,
		myData->save_msg[msg].val[idx].chData.totalCycle,
		myData->save_msg[msg].val[idx].chData.elementCycle,
		myData->save_msg[msg].val[idx].chData.accCycle[0],
		myData->save_msg[msg].val[idx].chData.accCycle[1],
		myData->save_msg[msg].val[idx].chData.accCycle[2],
		myData->save_msg[msg].val[idx].chData.accCycle[3],
		myData->save_msg[msg].val[idx].chData.accCycle[4],
		myData->save_msg[msg].val[idx].chData.multiCycle[0],
		myData->save_msg[msg].val[idx].chData.multiCycle[1],
		myData->save_msg[msg].val[idx].chData.multiCycle[2],
		myData->save_msg[msg].val[idx].chData.multiCycle[3],
		myData->save_msg[msg].val[idx].chData.multiCycle[4],
		myData->save_msg[msg].val[idx].chData.avgV,
		myData->save_msg[msg].val[idx].chData.avgI,
		myData->save_msg[msg].val[idx].chData.cvTime_day,
		myData->save_msg[msg].val[idx].chData.cvTime,
		//myData->save_msg[msg].val[idx].chData.ccTime_day,
		//myData->save_msg[msg].val[idx].chData.ccTime,
		myData->save_msg[msg].val[idx].chData.realDate,
		myData->save_msg[msg].val[idx].chData.realClock,
		myData->save_msg[msg].val[idx].chData.Vinput,
		myData->save_msg[msg].val[idx].chData.Vpower,
		myData->save_msg[msg].val[idx].chData.Vbus,
		//myData->save_msg[msg].val[idx].chData.integral_AmpareHour,
		//myData->save_msg[msg].val[idx].chData.integral_WattHour,
		//myData->save_msg[msg].val[idx].chData.reserved1[0],
		//myData->save_msg[msg].val[idx].chData.reserved1[1],
		myData->save_msg[msg].val[idx].chData.out_mux_use, //kjhw_151021
		myData->save_msg[msg].val[idx].chData.out_mux_backup, //kjhw_151021
		myData->save_msg[msg].val[idx].chData.maxAuxT, //kjhw_181223
		myData->save_msg[msg].val[idx].chData.minAuxT, //kjhw_181223
		myData->save_msg[msg].val[idx].chData.diffAuxT, //kjhw_181223
		myData->save_msg[msg].val[idx].chData.avgAuxT, //kjhw_181223
		myData->save_msg[msg].val[idx].chData.SOC, //20181219 KHK
		//myData->save_msg[msg].val[idx].chData.reserved2,
		myData->save_msg[msg].val[idx].chData.dIsens,		//shhw_230614s
		(int)myData->save_msg[msg].val[idx].chData.dMaxAuxVChNo,
		myData->save_msg[msg].val[idx].chData.dMaxAuxV,
		(int)myData->save_msg[msg].val[idx].chData.dMinAuxVChNo,
		myData->save_msg[msg].val[idx].chData.dMinAuxV);	//shhw_230614e		
	fflush(fp);
	fclose(fp);

	if(myData->mData.config.installedTemp != 0) {
		memset(cmd, 0, sizeof cmd);
		sprintf(cmd, "%s/ch%03d_SaveData%03d_auxT.csv",
			path, ch+1, (int)myPs->resultData[ch].fileIndex);
		// path/ch###_SaveData###_auxT.csv
		fp = fopen(cmd, "a");
		if(fp == NULL || fp < 0) {
			userlog(DEBUG_LOG, psName,
				"Save_ResultData error -2 (ch:%d)\n", ch+1);
			return -3;
		}

		//COA_VER_100B2~
		fprintf(fp, "%ld, %ld, %ld, %ld, %ld",
			myData->save_msg[msg].val[idx].chData.resultIndex,
			myData->save_msg[msg].val[idx].chData.totalRunTime_day,
			myData->save_msg[msg].val[idx].chData.totalRunTime,
			myData->save_msg[msg].val[idx].chData.runTime_day,
			myData->save_msg[msg].val[idx].chData.runTime);

		for(i=0; i < myData->auxDataCount[ch][0]; i++) {
			fprintf(fp, ", %ld",
				myData->save_msg[msg].val[idx].auxData[i].val);
		}
		
		fprintf(fp, "\n");
		fflush(fp);
		fclose(fp);
	}

	//COA_VER_100F~
	//jhkw_161208
	//if((myData->mData.config.installedAuxV != 0)
	//	&& (myData->mData.config.installedTH != 0)) {
	if((myData->mData.config.installedAuxV != 0)
		|| (myData->mData.config.installedTH != 0)) {
		memset(cmd, 0, sizeof cmd);
		sprintf(cmd, "%s/ch%03d_SaveData%03d_auxV.csv",
			path, ch+1, (int)myPs->resultData[ch].fileIndex);
		// path/ch###_SaveData###_auxV.csv
		fp = fopen(cmd, "a");
		if(fp == NULL || fp < 0) {
			userlog(DEBUG_LOG, psName,
				"Save_ResultData error -3 (ch:%d)\n", ch+1);
			return -4;
		}

		//COA_VER_100B2~
		fprintf(fp, "%ld, %ld, %ld, %ld, %ld",
			myData->save_msg[msg].val[idx].chData.resultIndex,
			myData->save_msg[msg].val[idx].chData.totalRunTime_day,
			myData->save_msg[msg].val[idx].chData.totalRunTime,
			myData->save_msg[msg].val[idx].chData.runTime_day,
			myData->save_msg[msg].val[idx].chData.runTime);

		//COA_VER_100F~
		j = myData->auxDataCount[ch][0] + myData->auxDataCount[ch][1]
			+ myData->auxDataCount[ch][2];
		for(i=myData->auxDataCount[ch][0]; i < j; i++) {
			fprintf(fp, ", %ld",myData->save_msg[msg].val[idx].auxData[i].val);
		}
		
		fprintf(fp, "\n");
		fflush(fp);
		fclose(fp);
	}

	if(myData->mData.config.installedCAN != 0) {
		memset(cmd, 0, sizeof cmd);
		sprintf(cmd, "%s/ch%03d_SaveData%03d_canMaster.csv",
			path, ch+1, (int)myPs->resultData[ch].fileIndex);
		// path/ch###_SaveData###_canMaster.csv
		fp = fopen(cmd, "a");
		if(fp == NULL || fp < 0) {
			userlog(DEBUG_LOG, psName,
				"Save_ResultData error -4 (ch:%d)\n", ch+1);
			return -5;
		}

		//COA_VER_100B2~
		fprintf(fp, "%ld, %ld, %ld, %ld, %ld",
			myData->save_msg[msg].val[idx].chData.resultIndex,
			myData->save_msg[msg].val[idx].chData.totalRunTime_day,
			myData->save_msg[msg].val[idx].chData.totalRunTime,
			myData->save_msg[msg].val[idx].chData.runTime_day,
			myData->save_msg[msg].val[idx].chData.runTime);

		for(i=0; i < myData->canReceiveDataCount[ch][0]; i++) {
			switch(myData->save_msg[msg].val[idx].canData[i].data_type) {
				case 0: //unsigned
				case 1: //signed
				case 2: //float
					fprintf(fp, ", %f", myData->save_msg[msg].val[idx]
						.canData[i].val.f_val[0]);
					break;
				/*default: //string
					fprintf(fp, ", %s", myData->save_msg[msg].val[idx]
						.canData[i].val.c_val);
					break;
					*/
					//kjhw_140811s
				case 3: //string
					fprintf(fp, ", %s", myData->save_msg[msg].val[idx]
						.canData[i].val.c_val);
					break;
				case 4: //hex
					fprintf(fp, ", %x", (unsigned int)myData->save_msg[msg]
						.val[idx].canData[i].val.f_val[0]);
					break;
				default:
					fprintf(fp, ", %s", myData->save_msg[msg].val[idx]
						.canData[i].val.c_val);
					break;
					//kjhw_140811e
			}
		}
		fprintf(fp, "\n");
		fflush(fp);
		fclose(fp);

		memset(cmd, 0, sizeof cmd);
		sprintf(cmd, "%s/ch%03d_SaveData%03d_canSlave.csv",
			path, ch+1, (int)myPs->resultData[ch].fileIndex);
		// path/ch###_SaveData###_canSlave.csv
		fp = fopen(cmd, "a");
		if(fp == NULL || fp < 0) {
			userlog(DEBUG_LOG, psName,
				"Save_ResultData error -5 (ch:%d)\n", ch+1);
			return -6;
		}

		//COA_VER_100B2~
		fprintf(fp, "%ld, %ld, %ld, %ld, %ld",
			myData->save_msg[msg].val[idx].chData.resultIndex,
			myData->save_msg[msg].val[idx].chData.totalRunTime_day,
			myData->save_msg[msg].val[idx].chData.totalRunTime,
			myData->save_msg[msg].val[idx].chData.runTime_day,
			myData->save_msg[msg].val[idx].chData.runTime);

		j = myData->canReceiveDataCount[ch][0]
			+ myData->canReceiveDataCount[ch][1];
		for(i=myData->canReceiveDataCount[ch][0]; i < j; i++) {
			switch(myData->save_msg[msg].val[idx].canData[i].data_type) {
				case 0: //unsigned
				case 1: //signed
				case 2: //float
					fprintf(fp, ", %f", myData->save_msg[msg].val[idx]
						.canData[i].val.f_val[0]);
					break;
				/*default: //string
					fprintf(fp, ", %s", myData->save_msg[msg].val[idx]
						.canData[i].val.c_val);
					break;
					*/
					//kjhw_140811s
				case 3: //string
					fprintf(fp, ", %s", myData->save_msg[msg].val[idx]
						.canData[i].val.c_val);
					break;
				case 4: //hex
					fprintf(fp, ", %x", (unsigned int)myData->save_msg[msg]
						.val[idx].canData[i].val.f_val[0]);
					break;
				default:
					fprintf(fp, ", %s", myData->save_msg[msg].val[idx]
						.canData[i].val.c_val);
					break;
					//kjhw_140811e
			}
		}
		fprintf(fp, "\n");
		fflush(fp);
		fclose(fp);
	}

	if(myData->save_msg[msg].val[idx].chData.select == SAVE_FLAG_SAVING_END) {
		memset(cmd, 0, sizeof cmd);
		sprintf(cmd, "%s/ch%03d_SaveEndData.csv", path, ch+1);
		// path/ch###_SaveEndData.csv
		fp = fopen(cmd, "a");
		if(fp == NULL || fp < 0) {
			userlog(DEBUG_LOG, psName, "Save_ResultData error -10 (ch:%d)\n",
				ch+1);
			return -10;
		}

		//COA_VER_100B2~
		fprintf(fp,
			"%ld, %ld, %ld, %ld, %ld, %d, %d, %d, %d, %d, %d, %d, %ld, %ld, %ld, %ld, %ld, %ld, %ld, %ld, %ld, %d, %d, %d, %d, %d, %d, %ld, %ld, %ld, %ld, %ld, %ld, %ld, %ld, %ld, %ld, %ld, %ld, %ld, %ld, %ld, %ld, %ld, %ld, %ld, %ld, %ld, %d, %d, %ld, %ld, %ld, %ld, %ld, %ld, %d, %ld, %d, %ld\n",
			myData->save_msg[msg].val[idx].chData.resultIndex,
			myData->save_msg[msg].val[idx].chData.totalRunTime_day,
			myData->save_msg[msg].val[idx].chData.totalRunTime,
			myData->save_msg[msg].val[idx].chData.runTime_day,
			myData->save_msg[msg].val[idx].chData.runTime,
			(int)myData->save_msg[msg].val[idx].chData.state,
			(int)myData->save_msg[msg].val[idx].chData.stepType,
			(int)myData->save_msg[msg].val[idx].chData.stepMode,
			(int)myData->save_msg[msg].val[idx].chData.select,
			(int)myData->save_msg[msg].val[idx].chData.code,
			(int)myData->save_msg[msg].val[idx].chData.grade,
			(int)myData->save_msg[msg].val[idx].chData.stepNo,
			myData->save_msg[msg].val[idx].chData.Vsens,
			myData->save_msg[msg].val[idx].chData.Isens,
			myData->save_msg[msg].val[idx].chData.charge_AmpareHour,
			myData->save_msg[msg].val[idx].chData.discharge_AmpareHour,
			myData->save_msg[msg].val[idx].chData.capacitance,
			myData->save_msg[msg].val[idx].chData.watt,
			myData->save_msg[msg].val[idx].chData.charge_WattHour,
			myData->save_msg[msg].val[idx].chData.discharge_WattHour,
			myData->save_msg[msg].val[idx].chData.z,
			(int)myData->save_msg[msg].val[idx].chData.reservedCmd,
			(int)myData->save_msg[msg].val[idx].chData.external_comm_state,
			(int)myData->save_msg[msg].val[idx].chData.ch_output_state,
			(int)myData->save_msg[msg].val[idx].chData.ch_input_state,
			(int)myData->save_msg[msg].val[idx].chData.auxDataCount,
			(int)myData->save_msg[msg].val[idx].chData.canReceiveDataCount,
			myData->save_msg[msg].val[idx].chData.totalCycle,
			myData->save_msg[msg].val[idx].chData.elementCycle,
			myData->save_msg[msg].val[idx].chData.accCycle[0],
			myData->save_msg[msg].val[idx].chData.accCycle[1],
			myData->save_msg[msg].val[idx].chData.accCycle[2],
			myData->save_msg[msg].val[idx].chData.accCycle[3],
			myData->save_msg[msg].val[idx].chData.accCycle[4],
			myData->save_msg[msg].val[idx].chData.multiCycle[0],
			myData->save_msg[msg].val[idx].chData.multiCycle[1],
			myData->save_msg[msg].val[idx].chData.multiCycle[2],
			myData->save_msg[msg].val[idx].chData.multiCycle[3],
			myData->save_msg[msg].val[idx].chData.multiCycle[4],
			myData->save_msg[msg].val[idx].chData.avgV,
			myData->save_msg[msg].val[idx].chData.avgI,
			myData->save_msg[msg].val[idx].chData.cvTime_day,
			myData->save_msg[msg].val[idx].chData.cvTime,
			//myData->save_msg[msg].val[idx].chData.ccTime_day,
			//myData->save_msg[msg].val[idx].chData.ccTime,
			myData->save_msg[msg].val[idx].chData.realDate,
			myData->save_msg[msg].val[idx].chData.realClock,
			myData->save_msg[msg].val[idx].chData.Vinput,
			myData->save_msg[msg].val[idx].chData.Vpower,
			myData->save_msg[msg].val[idx].chData.Vbus,
			//myData->save_msg[msg].val[idx].chData.integral_AmpareHour,
			//myData->save_msg[msg].val[idx].chData.integral_WattHour,
			//myData->save_msg[msg].val[idx].chData.reserved1[0],
			//myData->save_msg[msg].val[idx].chData.reserved1[1],
			myData->save_msg[msg].val[idx].chData.out_mux_use, //kjhw_151021
			myData->save_msg[msg].val[idx].chData.out_mux_backup, //kjhw_151021
			myData->save_msg[msg].val[idx].chData.maxAuxT, //kjhw_181223
			myData->save_msg[msg].val[idx].chData.minAuxT, //kjhw_181223
			myData->save_msg[msg].val[idx].chData.diffAuxT, //kjhw_181223
			myData->save_msg[msg].val[idx].chData.avgAuxT, //kjhw_181223
			myData->save_msg[msg].val[idx].chData.SOC, //20181219 KHK
			//myData->save_msg[msg].val[idx].chData.reserved2,
			myData->save_msg[msg].val[idx].chData.dIsens,		//shhw_230614s
			(int)myData->save_msg[msg].val[idx].chData.dMaxAuxVChNo,
			myData->save_msg[msg].val[idx].chData.dMaxAuxV,
			(int)myData->save_msg[msg].val[idx].chData.dMinAuxVChNo,
			myData->save_msg[msg].val[idx].chData.dMinAuxV);	//shhw_230614e		
		fflush(fp);
		fclose(fp);

		if(myData->mData.config.installedTemp != 0) {
			memset(cmd, 0, sizeof cmd);
			sprintf(cmd, "%s/ch%03d_SaveEndData_auxT.csv", path, ch+1);
			// path/ch###_SaveEndData_auxT.csv
			fp = fopen(cmd, "a");
			if(fp == NULL || fp < 0) {
				userlog(DEBUG_LOG, psName,
					"Save_ResultData error -11 (ch:%d)\n", ch+1);
				return -11;
			}

			//COA_VER_100B2~
			fprintf(fp, "%ld, %ld, %ld, %ld, %ld",
				myData->save_msg[msg].val[idx].chData.resultIndex,
				myData->save_msg[msg].val[idx].chData.totalRunTime_day,
				myData->save_msg[msg].val[idx].chData.totalRunTime,
				myData->save_msg[msg].val[idx].chData.runTime_day,
				myData->save_msg[msg].val[idx].chData.runTime);

			for(i=0; i < myData->auxDataCount[ch][0]; i++) {
				fprintf(fp, ", %ld",
					myData->save_msg[msg].val[idx].auxData[i].val);
			}
			
			fprintf(fp, "\n");
			fflush(fp);
			fclose(fp);
		}

		//COA_VER_100F~
		//jhkw_161208
		//if((myData->mData.config.installedAuxV != 0)
		//	&& (myData->mData.config.installedTH != 0)) {
		if((myData->mData.config.installedAuxV != 0)
			|| (myData->mData.config.installedTH != 0)) {
			memset(cmd, 0, sizeof cmd);
			sprintf(cmd, "%s/ch%03d_SaveEndData_auxV.csv", path, ch+1);
			// path/ch###_SaveEndData_auxV.csv
			fp = fopen(cmd, "a");
			if(fp == NULL || fp < 0) {
				userlog(DEBUG_LOG, psName,
					"Save_ResultData error -12 (ch:%d)\n", ch+1);
				return -12;
			}

			//COA_VER_100B2~
			fprintf(fp, "%ld, %ld, %ld, %ld, %ld",
				myData->save_msg[msg].val[idx].chData.resultIndex,
				myData->save_msg[msg].val[idx].chData.totalRunTime_day,
				myData->save_msg[msg].val[idx].chData.totalRunTime,
				myData->save_msg[msg].val[idx].chData.runTime_day,
				myData->save_msg[msg].val[idx].chData.runTime);

			//COA_VER_100F~
			j = myData->auxDataCount[ch][0] + myData->auxDataCount[ch][1]
				+ myData->auxDataCount[ch][2];
			for(i=myData->auxDataCount[ch][0]; i < j; i++) {
				fprintf(fp, ", %ld",
					myData->save_msg[msg].val[idx].auxData[i].val);
			}
			
			fprintf(fp, "\n");
			fflush(fp);
			fclose(fp);
		}

		if(myData->mData.config.installedCAN != 0) {
			memset(cmd, 0, sizeof cmd);
			sprintf(cmd, "%s/ch%03d_SaveEndData_canMaster.csv", path, ch+1);
			// path/ch###_SaveEndData_canMaster.csv
			fp = fopen(cmd, "a");
			if(fp == NULL || fp < 0) {
				userlog(DEBUG_LOG, psName,
					"Save_ResultData error -13 (ch:%d)\n", ch+1);
				return -13;
			}

			//COA_VER_100B2~
			fprintf(fp, "%ld, %ld, %ld, %ld, %ld",
				myData->save_msg[msg].val[idx].chData.resultIndex,
				myData->save_msg[msg].val[idx].chData.totalRunTime_day,
				myData->save_msg[msg].val[idx].chData.totalRunTime,
				myData->save_msg[msg].val[idx].chData.runTime_day,
				myData->save_msg[msg].val[idx].chData.runTime);

			for(i=0; i < myData->canReceiveDataCount[ch][0]; i++) {
				switch(myData->save_msg[msg].val[idx].canData[i].data_type) {
					case 0: //unsigned
					case 1: //signed
					case 2: //float
						fprintf(fp, ", %f", myData->save_msg[msg].val[idx]
							.canData[i].val.f_val[0]);
						break;
					/*default: //string
						fprintf(fp, ", %s", myData->save_msg[msg].val[idx]
							.canData[i].val.c_val);
						break;
						*/
						//kjhw_140811s
					case 3: //string
						fprintf(fp, ", %s", myData->save_msg[msg].val[idx]
							.canData[i].val.c_val);
						break;
					case 4: //hex
						fprintf(fp, ", %x", (unsigned int)myData->save_msg[msg]
							.val[idx].canData[i].val.f_val[0]);
						break;
					default:
						fprintf(fp, ", %s", myData->save_msg[msg].val[idx]
							.canData[i].val.c_val);
						break;
						//kjhw_140811e
				}
			}
			fprintf(fp, "\n");
			fflush(fp);
			fclose(fp);

			memset(cmd, 0, sizeof cmd);
			sprintf(cmd, "%s/ch%03d_SaveEndData_canSlave.csv", path, ch+1);
			// path/ch###_SaveEndData_canSlave.csv
			fp = fopen(cmd, "a");
			if(fp == NULL || fp < 0) {
				userlog(DEBUG_LOG, psName,
					"Save_ResultData error -14 (ch:%d)\n", ch+1);
				return -14;
			}

			//COA_VER_100B2~
			fprintf(fp, "%ld, %ld, %ld, %ld, %ld",
				myData->save_msg[msg].val[idx].chData.resultIndex,
				myData->save_msg[msg].val[idx].chData.totalRunTime_day,
				myData->save_msg[msg].val[idx].chData.totalRunTime,
				myData->save_msg[msg].val[idx].chData.runTime_day,
				myData->save_msg[msg].val[idx].chData.runTime);
			j = myData->canReceiveDataCount[ch][0]
				+ myData->canReceiveDataCount[ch][1];
			for(i=myData->canReceiveDataCount[ch][0]; i < j; i++) {
				switch(myData->save_msg[msg].val[idx].canData[i].data_type) {
					case 0: //unsigned
					case 1: //signed
					case 2: //float
						fprintf(fp, ", %f", myData->save_msg[msg].val[idx]
							.canData[i].val.f_val[0]);
						break;
					/*default: //string
						fprintf(fp, ", %s", myData->save_msg[msg].val[idx]
							.canData[i].val.c_val);
						break;
						*/
						//kjhw_140811s
					case 3: //string
						fprintf(fp, ", %s", myData->save_msg[msg].val[idx]
							.canData[i].val.c_val);
						break;
					case 4: //hex
						fprintf(fp, ", %x", (unsigned int)myData->save_msg[msg]
							.val[idx].canData[i].val.f_val[0]);
						break;
					default:
						fprintf(fp, ", %s", myData->save_msg[msg].val[idx]
							.canData[i].val.c_val);
						break;
						//kjhw_140811e
				}
			}
			fprintf(fp, "\n");
			fflush(fp);
			fclose(fp);
		}
	}

	memset(cmd, 0, sizeof cmd);
	sprintf(cmd, "%s/savingFileIndex_last.csv", path);
	// path/savingFileIndex_last.csv
	fp = fopen(cmd, "w");
	if(fp == NULL || fp < 0) {
		userlog(DEBUG_LOG, psName, "Save_ResultData error -20 (ch:%d)\n", ch+1);
		return -20;
	}

	myPs->resultData[ch].last_resultIndex
		= myData->save_msg[msg].val[idx].chData.resultIndex;

	fprintf(fp, "fileIndex %d, ", (int)myPs->resultData[ch].fileIndex);
	fprintf(fp, "resultIndex %ld, ",
		myData->save_msg[msg].val[idx].chData.resultIndex);
	fprintf(fp, "maxFileIndex %d, ", myPs->resultData[ch].maxFileIndex);
	fprintf(fp, "open_year %d, ", (int)myPs->resultData[ch].open_year);
	fprintf(fp, "open_month %d, ", (int)myPs->resultData[ch].open_month);
	fprintf(fp, "open_day %d\n", (int)myPs->resultData[ch].open_day);
	fflush(fp);
	fclose(fp);

	return count;
}

int Save_PulseData(int ch)
{
	char cmd[128], path[128];
	int msg, idx, groupNo, count=0, totalCycle, stepNo;
	FILE *fp;

	msg = 1;

	if(myData->pulse_msg_1[msg].write_idx[ch]
		== myData->pulse_msg_1[msg].read_idx[ch]) {
		return 0;
	}

	myData->pulse_msg_1[msg].read_idx[ch]++;
	if(myData->pulse_msg_1[msg].read_idx[ch] >= MAX_PULSE_MSG) {
		myData->pulse_msg_1[msg].read_idx[ch] = 0;
	}
	idx = myData->pulse_msg_1[msg].read_idx[ch];

	if(myData->pulse_msg_1[msg].total_count > 0)
		myData->pulse_msg_1[msg].total_count--;

	if(myData->pulse_msg_1[msg].count[ch] > 0)
		myData->pulse_msg_1[msg].count[ch]--;
	count = myData->pulse_msg_1[msg].count[ch];

	groupNo = 0; //kjg_w

	memset(path, 0, sizeof path);
	sprintf(path, "/root/%s/resultData/group%d/data0/ch%03d",
		myData->AppControl.misc.path1, groupNo+1, ch+1);
	// /root/system_data/resultData/group#/data0/ch###

	totalCycle = (int)myData->pulse_msg_1[msg].val[idx][ch].totalCycle;
	stepNo = (int)myData->pulse_msg_1[msg].val[idx][ch].stepNo;

	memset(cmd, 0, sizeof cmd);
	sprintf(cmd, "%s/ch%03d_PulseData_%06d_%03d.csv", path, ch+1,
		totalCycle, stepNo);
		// path/ch###_PulseData_######_###.csv
	fp = fopen(cmd, "a");
	if(fp == NULL || fp < 0) {
		userlog(DEBUG_LOG, psName,
			"Save_PulseData error -10 (ch:%d, totalCycle:%d, stepNo:%d)\n",
			ch+1, totalCycle, stepNo);
		return -3; //kjg_w
	}

	fprintf(fp,
		"%ld, %ld, %ld, %ld, %ld\n",
		(long)myData->pulse_msg_1[msg].val[idx][ch].runTime,
		myData->pulse_msg_1[msg].val[idx][ch].Vsens,
		myData->pulse_msg_1[msg].val[idx][ch].Isens,
		myData->pulse_msg_1[msg].val[idx][ch].capacity,
		myData->pulse_msg_1[msg].val[idx][ch].wattHour);
	fflush(fp);
	fclose(fp);

	return count;
}

int Open_ResultData_General_1(int p_ch)
{
#ifdef __COC__
	char cmd[128], path[128];
	int ch, groupNo, file_format; //i, j;
	struct tm *tm;
	time_t t;
	FILE *fp;

	groupNo = 0; //kjg_w
	file_format = 1;

	memset(path, 0, sizeof path);
	sprintf(path, "/root/%s/resultData/group%d/data0/ch%03d",
		myData->AppControl.misc.path1, groupNo+1, p_ch+1);
	// /root/system_data/resultData/group#/data0/ch###

	memset(cmd, 0, sizeof cmd);
	sprintf(cmd, "rm -rf %s/*", path);
	system(cmd);
	// rm -rf path/*

	myPs->resultData[p_ch].fileIndex = 1;
	myPs->resultData[p_ch].maxFileIndex = 1;
	myPs->resultData[p_ch].divisionCount = 0;
	myPs->resultData[p_ch].last_resultIndex = 0;
	time(&t);
	tm = localtime(&t);
	myPs->resultData[p_ch].open_year = (unsigned char)tm->tm_year-100;
	myPs->resultData[p_ch].open_month = (unsigned char)tm->tm_mon+1;
	myPs->resultData[p_ch].open_day = (unsigned char)tm->tm_mday;

	ch = myData->CellArray1[p_ch].number2 - 1;

	memset(cmd, 0, sizeof(cmd));
	sprintf(cmd, "%s/ch%02d_SaveEndData.csv", path, p_ch+1);
	// path/ch##_SaveEndData.csv
	fp = fopen(cmd, "w");
	if(fp == NULL || fp < 0) {
		userlog(DEBUG_LOG, psName, "Open_ResultData_General_1 error -1\n");
		return -1; //kjg_w
	}
	//fprintf(fp, "index, totalTime, stepTime, state, type, mode, select, code, grade, stepNo, Vsens, Isens, charge_AmpareHour, discharge_AmpareHour, capacitance, watt, charge_WattHour, discharge_WattHour, impedance, reservedCmd, auxCount, canCount, totalCycle, elementCycle, accCycle1, accCycle2, accCycle3, accCycle4, accCycle5, multiCycle1, multiCycle2, multiCycle3, multiCycle4, multiCycle5, averageV, averageI, cvTime, realDate, realClock\n");
	fflush(fp);
	fclose(fp);

	memset(cmd, 0, sizeof(cmd));
	sprintf(cmd, "%s/ch%02d_RecordData.csv", path, p_ch+1);
	// path/ch##_RecordData.csv
	fp = fopen(cmd, "w");
	if(fp == NULL || fp < 0) {
		userlog(DEBUG_LOG, psName, "Open_RecordData_General_1 error -1\n");
		return -1; //kjg_w
	}
	fflush(fp);
	fclose(fp);

	memset(cmd, 0, sizeof(cmd));
	sprintf(cmd, "%s/savingFileIndex_start.csv", path);
	// path/savingFileIndex_start.csv
	fp = fopen(cmd, "w");
	if(fp == NULL || fp < 0) {
		userlog(DEBUG_LOG, psName, "Open_ResultData_General_1 error -6\n");
		return -6; //kjg_w
	}

	fprintf(fp, "fileIndex %d, ", myPs->resultData[p_ch].fileIndex);
	fprintf(fp, "resultIndex %ld, ", myData->cData[ch].op.resultIndex+1);
	if(file_format == 1) {
		fprintf(fp, "maxFileIndex %d, ", myPs->resultData[p_ch].maxFileIndex);
	}
	fprintf(fp, "open_year %d, ", (int)myPs->resultData[p_ch].open_year);
	fprintf(fp, "open_month %d, ", (int)myPs->resultData[p_ch].open_month);
	fprintf(fp, "open_day %d\n", (int)myPs->resultData[p_ch].open_day);
	fflush(fp);
	fclose(fp);

	memset(cmd, 0, sizeof(cmd));
	sprintf(cmd, "%s/savingFileIndex_last.csv", path);
	// path/savingFileIndex_last.csv
	fp = fopen(cmd, "w");
	if(fp == NULL || fp < 0) {
		userlog(DEBUG_LOG, psName, "Open_ResultData_General_1 error -7\n");
		return -7; //kjg_w
	}

	fprintf(fp, "fileIndex %d, ", myPs->resultData[p_ch].fileIndex);
	fprintf(fp, "resultIndex %ld, ", myData->cData[ch].op.resultIndex+1);
	if(file_format == 1) {
		fprintf(fp, "maxFileIndex %d, ", myPs->resultData[p_ch].maxFileIndex);
	}
	fprintf(fp, "open_year %d, ", (int)myPs->resultData[p_ch].open_year);
	fprintf(fp, "open_month %d, ", (int)myPs->resultData[p_ch].open_month);
	fprintf(fp, "open_day %d\n", (int)myPs->resultData[p_ch].open_day);
	fflush(fp);
	fclose(fp);

	memset(cmd, 0, sizeof(cmd));
	if(file_format == 0) {
		sprintf(cmd, "%s/ch%02d_SaveData%02d.csv",
			path, p_ch+1, (int)myPs->resultData[p_ch].fileIndex);
		// path/ch##_SaveData##.csv
	} else {
		sprintf(cmd, "%s/ch%02d_SaveData%03d.csv",
			path, p_ch+1, (int)myPs->resultData[p_ch].fileIndex);
		// path/ch##_SaveData###.csv
	}
	fp = fopen(cmd, "w");
	if(fp == NULL || fp < 0) {
		userlog(DEBUG_LOG, psName, "Open_ResultData_General_1 error -10\n");
		return -10; //kjg_w
	}
	//fprintf(fp, "index, totalTime, stepTime, state, type, mode, select, code, grade, stepNo, Vsens, Isens, charge_AmpareHour, discharge_AmpareHour, capacitance, watt, charge_WattHour, discharge_WattHour, impedance, reservedCmd, auxCount, canCount, totalCycle, elementCycle, accCycle1, accCycle2, accCycle3, accCycle4, accCycle5, multiCycle1, multiCycle2, multiCycle3, multiCycle4, multiCycle5, averageV, averageI, cvTime, realDate, realClock\n");
	fflush(fp);
	fclose(fp);
#endif
	return 0;
}

int Open_ResultData_General_2(int p_ch)
{
#ifdef __COC__
	char cmd[128], path[128];
	int ch, groupNo, i, file_format; //, j;
	struct tm *tm;
	time_t t;
	FILE *fp;

	groupNo = 0; //kjg_w
	file_format = 1;

	memset(path, 0, sizeof path);
	sprintf(path, "/root/%s/resultData/group%d/data0/ch%03d",
		myData->AppControl.misc.path1, groupNo+1, p_ch+1);
	// /root/system_data/resultData/group#/data0/ch###

	time(&t);
	tm = localtime(&t);

	if(myPs->resultData[p_ch].open_year != (unsigned char)tm->tm_year-100
		|| myPs->resultData[p_ch].open_month != (unsigned char)tm->tm_mon+1
		|| myPs->resultData[p_ch].open_day != (unsigned char)tm->tm_mday) {
		i = 1;
	} else i = 0;

	myPs->resultData[p_ch].divisionCount++;
	if(myPs->resultData[p_ch].divisionCount > MAX_DIVISION_COUNT) {
		i = 2;
	}

	if(i == 0) return 0;
	myPs->resultData[p_ch].divisionCount = 0;

	myPs->resultData[p_ch].open_year = (unsigned char)tm->tm_year-100;
	myPs->resultData[p_ch].open_month = (unsigned char)tm->tm_mon+1;
	myPs->resultData[p_ch].open_day = (unsigned char)tm->tm_mday;

	myPs->resultData[p_ch].fileIndex++;
	myPs->resultData[p_ch].maxFileIndex++;
	if(myPs->resultData[p_ch].fileIndex > MAX_RESULT_FILE_INDEX_GENERAL) {
		myPs->resultData[p_ch].fileIndex = 1;
//		myPs->resultData[p_ch].maxFileIndex = MAX_RESULT_FILE_INDEX_GENERAL;
	}

	ch = myData->CellArray1[p_ch].number2 - 1;

	memset(cmd, 0, sizeof cmd);
	sprintf(cmd, "%s/savingFileIndex_start.csv", path);
	// path/savingFileIndex_start.csv
	fp = fopen(cmd, "a");
	if(fp == NULL || fp < 0) {
		userlog(DEBUG_LOG, psName, "Open_ResultData_General_2 error -6\n");
		return -6; //kjg_w
	}

	fprintf(fp, "fileIndex %d, ", myPs->resultData[p_ch].fileIndex);
	//fprintf(fp, "resultIndex %ld, ", myData->cData[ch].op.resultIndex);
	fprintf(fp, "resultIndex %ld, ", myPs->resultData[p_ch].last_resultIndex+1);
	if(file_format == 1) {
		fprintf(fp, "maxFileIndex %d, ", myPs->resultData[p_ch].maxFileIndex);
	}
	fprintf(fp, "open_year %d, ", (int)myPs->resultData[p_ch].open_year);
	fprintf(fp, "open_month %d, ", (int)myPs->resultData[p_ch].open_month);
	fprintf(fp, "open_day %d\n", (int)myPs->resultData[p_ch].open_day);
	fflush(fp);
	fclose(fp);

	memset(cmd, 0, sizeof cmd);
	sprintf(cmd, "%s/savingFileIndex_last.csv", path);
	// path/savingFileIndex_last.csv
	fp = fopen(cmd, "w");
	if(fp == NULL || fp < 0) {
		userlog(DEBUG_LOG, psName, "Open_ResultData_General_2 error -7\n");
		return -7; //kjg_w
	}

	fprintf(fp, "fileIndex %d, ", myPs->resultData[p_ch].fileIndex);
	//fprintf(fp, "resultIndex %ld, ", myData->cData[ch].op.resultIndex);
	fprintf(fp, "resultIndex %ld, ", myPs->resultData[p_ch].last_resultIndex+1);
	if(file_format == 1) {
		fprintf(fp, "maxFileIndex %d, ", myPs->resultData[p_ch].maxFileIndex);
	}
	fprintf(fp, "open_year %d, ", (int)myPs->resultData[p_ch].open_year);
	fprintf(fp, "open_month %d, ", (int)myPs->resultData[p_ch].open_month);
	fprintf(fp, "open_day %d\n", (int)myPs->resultData[p_ch].open_day);
	fflush(fp);
	fclose(fp);

	memset(cmd, 0, sizeof cmd);
	if(file_format == 0) {
		sprintf(cmd, "rm -rf %s/ch%02d_SaveData%02d.csv",
			path, p_ch+1, (int)myPs->resultData[p_ch].fileIndex);
		//rm -rf path/ch##_SaveData##.csv
	} else {
		sprintf(cmd, "rm -rf %s/ch%02d_SaveData%03d.csv",
			path, p_ch+1, (int)myPs->resultData[p_ch].fileIndex);
		//rm -rf path/ch##_SaveData###.csv
	}
	system(cmd);
#endif
	return 0;
}

void Save_ResultData_General_1(int type)
{ //kjg_110921
#ifdef __COC__
	int msg, send_count, rcv_count, p_ch, rtn;

	msg = 1;
	rcv_count = 200;
	if(type == 1) { //software close routine
		if(myData->save_msg_1[msg].total_count > rcv_count) {
			rcv_count = myData->save_msg_1[msg].total_count;
		}
	}
	for(p_ch=0; p_ch < myData->mData.config.installedCh; p_ch++) {
		for(send_count=0; send_count < rcv_count; send_count++) {
			rtn = Save_ResultData_General_2(p_ch);
			if(rtn <= 0) break;
		}
	}

	rcv_count = 200;
	if(type == 1) { //software close routine
		if(myData->record_msg_1[msg].total_count > rcv_count) {
			rcv_count = myData->record_msg_1[msg].total_count;
		}
	}
	for(p_ch=0; p_ch < myData->mData.config.installedCh; p_ch++) {
		for(send_count=0; send_count < rcv_count; send_count++) {
			rtn = Save_RecordData_General(p_ch);
			if(rtn <= 0) break;
		}
	}

	if(type == 1) { //software close routine
		myData->save_msg_1[msg].total_count = 0;
		for(p_ch=0; p_ch < myData->mData.config.installedCh; p_ch++) {
			myData->save_msg_1[msg].count[p_ch] = 0;
		}

		myData->record_msg_1[msg].total_count = 0;
		for(p_ch=0; p_ch < myData->mData.config.installedCh; p_ch++) {
			myData->record_msg_1[msg].count[p_ch] = 0;
		}
	}
#endif
}

int Save_ResultData_General_2(int ch)
{
	int count=0;
#ifdef __COC__
	char cmd[128], path[128];
	int msg, idx, groupNo, file_format; //i, j;
	FILE *fp;

	msg = 1;
	file_format = 1;

	if(myData->save_msg_1[msg].write_idx[ch]
		== myData->save_msg_1[msg].read_idx[ch]) {
		return 0;
	}

	myData->save_msg_1[msg].read_idx[ch]++;
	if(myData->save_msg_1[msg].read_idx[ch] >= MAX_SAVE_MSG) {
		myData->save_msg_1[msg].read_idx[ch] = 0;
	}
	idx = myData->save_msg_1[msg].read_idx[ch];

	if(myData->save_msg_1[msg].total_count > 0)
		myData->save_msg_1[msg].total_count--;

	if(myData->save_msg_1[msg].count[ch] > 0)
		myData->save_msg_1[msg].count[ch]--;
	count = myData->save_msg_1[msg].count[ch];

	if(Open_ResultData_General_2(ch) < 0) {
		return -1; //kjg_w
	}

	groupNo = 0; //kjg_w

	memset(path, 0, sizeof path);
	sprintf(path, "/root/%s/resultData/group%d/data0/ch%03d",
		myData->AppControl.misc.path1, groupNo+1, ch+1);
	// /root/system_data/resultData/group#/data0/ch###

	memset(cmd, 0, sizeof cmd);
	if(file_format == 0) {
		sprintf(cmd, "%s/ch%02d_SaveData%02d.csv",
			path, ch+1, (int)myPs->resultData[ch].fileIndex);
		// path/ch##_SaveData##.csv
	} else {
		sprintf(cmd, "%s/ch%02d_SaveData%03d.csv",
			path, ch+1, (int)myPs->resultData[ch].fileIndex);
		// path/ch##_SaveData###.csv
	}
	fp = fopen(cmd, "a");
	if(fp == NULL || fp < 0) {
		userlog(DEBUG_LOG, psName,
			"Save_ResultData_General error -1 (ch:%d)\n", ch+1);
		return -2; //kjg_w
	}

	fprintf(fp,
		"%ld, %d, %d, %d, %d, %d, %d, %d, %ld, %ld, %ld, %ld, %ld, %ld, %ld, %ld, %ld, %ld, %ld, %d, %ld, %ld, %ld, %ld\n",
		myData->save_msg_1[msg].val[idx][ch].resultIndex,
		(int)myData->save_msg_1[msg].val[idx][ch].state,
		(int)myData->save_msg_1[msg].val[idx][ch].type,
		(int)myData->save_msg_1[msg].val[idx][ch].mode,
		(int)myData->save_msg_1[msg].val[idx][ch].select,
		(int)myData->save_msg_1[msg].val[idx][ch].code,
		(int)myData->save_msg_1[msg].val[idx][ch].grade,
		(int)myData->save_msg_1[msg].val[idx][ch].stepNo,
		myData->save_msg_1[msg].val[idx][ch].Vsens,
		myData->save_msg_1[msg].val[idx][ch].Isens,
		myData->save_msg_1[msg].val[idx][ch].capacity,
		myData->save_msg_1[msg].val[idx][ch].watt,
		myData->save_msg_1[msg].val[idx][ch].wattHour,
		myData->save_msg_1[msg].val[idx][ch].runTime,
		myData->save_msg_1[msg].val[idx][ch].totalRunTime,
		myData->save_msg_1[msg].val[idx][ch].z,
		myData->save_msg_1[msg].val[idx][ch].temp[0],
		myData->save_msg_1[msg].val[idx][ch].temp[1],
		myData->save_msg_1[msg].val[idx][ch].temp[2],
		(int)myData->save_msg_1[msg].val[idx][ch].reservedCmd,
		myData->save_msg_1[msg].val[idx][ch].totalCycle,
		myData->save_msg_1[msg].val[idx][ch].currentCycle,
		myData->save_msg_1[msg].val[idx][ch].avgV,
		myData->save_msg_1[msg].val[idx][ch].avgI);
	fflush(fp);
	fclose(fp);

	if(myData->save_msg_1[msg].val[idx][ch].select == SAVE_FLAG_SAVING_END) {
		memset(cmd, 0, sizeof cmd);
		sprintf(cmd, "%s/ch%02d_SaveEndData.csv", path, ch+1);
		// path/ch##_SaveEndData.csv
		fp = fopen(cmd, "a");
		if(fp == NULL || fp < 0) {
			userlog(DEBUG_LOG, psName,
				"Save_ResultData_General error -10 (ch:%d)\n", ch+1);
			return -3; //kjg_w
		}

		fprintf(fp,
			"%ld, %d, %d, %d, %d, %d, %d, %d, %ld, %ld, %ld, %ld, %ld, %ld, %ld, %ld, %ld, %ld, %ld, %d, %ld, %ld, %ld, %ld\n",
			myData->save_msg_1[msg].val[idx][ch].resultIndex,
			(int)myData->save_msg_1[msg].val[idx][ch].state,
			(int)myData->save_msg_1[msg].val[idx][ch].type,
			(int)myData->save_msg_1[msg].val[idx][ch].mode,
			(int)myData->save_msg_1[msg].val[idx][ch].select,
			(int)myData->save_msg_1[msg].val[idx][ch].code,
			(int)myData->save_msg_1[msg].val[idx][ch].grade,
			(int)myData->save_msg_1[msg].val[idx][ch].stepNo,
			myData->save_msg_1[msg].val[idx][ch].Vsens,
			myData->save_msg_1[msg].val[idx][ch].Isens,
			myData->save_msg_1[msg].val[idx][ch].capacity,
			myData->save_msg_1[msg].val[idx][ch].watt,
			myData->save_msg_1[msg].val[idx][ch].wattHour,
			myData->save_msg_1[msg].val[idx][ch].runTime,
			myData->save_msg_1[msg].val[idx][ch].totalRunTime,
			myData->save_msg_1[msg].val[idx][ch].z,
			myData->save_msg_1[msg].val[idx][ch].temp[0],
			myData->save_msg_1[msg].val[idx][ch].temp[1],
			myData->save_msg_1[msg].val[idx][ch].temp[2],
			(int)myData->save_msg_1[msg].val[idx][ch].reservedCmd,
			myData->save_msg_1[msg].val[idx][ch].totalCycle,
			myData->save_msg_1[msg].val[idx][ch].currentCycle,
			myData->save_msg_1[msg].val[idx][ch].avgV,
			myData->save_msg_1[msg].val[idx][ch].avgI);
		fflush(fp);
		fclose(fp);
	}

	memset(cmd, 0, sizeof cmd);
	sprintf(cmd, "%s/savingFileIndex_last.csv", path);
	// path/savingFileIndex_last.csv
	fp = fopen(cmd, "w");
	if(fp == NULL || fp < 0) {
		userlog(DEBUG_LOG, psName,
			"Save_ResultData_General error -7 (ch:%d)\n", ch+1);
		return -4; //kjg_w
	}

	myPs->resultData[ch].last_resultIndex
		= myData->save_msg_1[msg].val[idx][ch].resultIndex;

	fprintf(fp, "fileIndex %d, ", myPs->resultData[ch].fileIndex);
	fprintf(fp, "resultIndex %ld, ",
		myData->save_msg_1[msg].val[idx][ch].resultIndex);
	if(file_format == 1) {
		fprintf(fp, "maxFileIndex %d, ", myPs->resultData[ch].maxFileIndex);
	}
	fprintf(fp, "open_year %d, ", (int)myPs->resultData[ch].open_year);
	fprintf(fp, "open_month %d, ", (int)myPs->resultData[ch].open_month);
	fprintf(fp, "open_day %d\n", (int)myPs->resultData[ch].open_day);
	fflush(fp);
	fclose(fp);

#endif
	return count;
}

int Save_RecordData_General(int ch)
{
	int count=0;
#ifdef __COC__
	char cmd[128], path[128];
	int msg, idx, groupNo, file_format;
	FILE *fp;

	msg = 1;
	file_format = 1;

	if(myData->record_msg_1[msg].write_idx[ch]
		== myData->record_msg_1[msg].read_idx[ch]) {
		return 0;
	}

	myData->record_msg_1[msg].read_idx[ch]++;
	if(myData->record_msg_1[msg].read_idx[ch] >= MAX_SAVE_MSG) {
		myData->record_msg_1[msg].read_idx[ch] = 0;
	}
	idx = myData->record_msg_1[msg].read_idx[ch];

	if(myData->record_msg_1[msg].total_count > 0)
		myData->record_msg_1[msg].total_count--;

	if(myData->record_msg_1[msg].count[ch] > 0)
		myData->record_msg_1[msg].count[ch]--;
	count = myData->record_msg_1[msg].count[ch];

	groupNo = 0; //kjg_w

	memset(path, 0, sizeof path);
	sprintf(path, "/root/%s/resultData/group%d/data0/ch%03d",
		myData->AppControl.misc.path1, groupNo+1, ch+1);
	// /root/system_data/resultData/group#/data0/ch###

	memset(cmd, 0, sizeof cmd);
	sprintf(cmd, "%s/ch%02d_RecordData.csv", path, ch+1);
		// path/ch##_RecordData.csv
	fp = fopen(cmd, "a");
	if(fp == NULL || fp < 0) {
		userlog(DEBUG_LOG, psName,
			"Save_RecordData_General error -10 (ch:%d)\n", ch+1);
		return -3; //kjg_w
	}

	fprintf(fp,
		"%ld, %d, %d, %d, %d, %d, %d, %d, %ld, %ld, %ld, %ld, %ld, %ld, %ld, %ld, %ld, %ld, %ld, %d, %ld, %ld, %ld, %ld, %d\n",
		myData->record_msg_1[msg].val[idx][ch].resultIndex,
		(int)myData->record_msg_1[msg].val[idx][ch].state,
		(int)myData->record_msg_1[msg].val[idx][ch].type,
		(int)myData->record_msg_1[msg].val[idx][ch].mode,
		(int)myData->record_msg_1[msg].val[idx][ch].select,
		(int)myData->record_msg_1[msg].val[idx][ch].code,
		(int)myData->record_msg_1[msg].val[idx][ch].grade,
		(int)myData->record_msg_1[msg].val[idx][ch].stepNo,
		myData->record_msg_1[msg].val[idx][ch].Vsens,
		myData->record_msg_1[msg].val[idx][ch].Isens,
		myData->record_msg_1[msg].val[idx][ch].capacity,
		myData->record_msg_1[msg].val[idx][ch].watt,
		myData->record_msg_1[msg].val[idx][ch].wattHour,
		myData->record_msg_1[msg].val[idx][ch].runTime,
		myData->record_msg_1[msg].val[idx][ch].totalRunTime,
		myData->record_msg_1[msg].val[idx][ch].z,
		myData->record_msg_1[msg].val[idx][ch].temp[0],
		myData->record_msg_1[msg].val[idx][ch].temp[1],
		myData->record_msg_1[msg].val[idx][ch].temp[2],
		(int)myData->record_msg_1[msg].val[idx][ch].reservedCmd,
		myData->record_msg_1[msg].val[idx][ch].totalCycle,
		myData->record_msg_1[msg].val[idx][ch].currentCycle,
		myData->record_msg_1[msg].val[idx][ch].avgV,
		myData->record_msg_1[msg].val[idx][ch].avgI,
		(int)myData->record_msg_1[msg].val[idx][ch].record_index);
	fflush(fp);
	fclose(fp);
#endif
	return count;
}

int Open_ResultData_Formation_1(int group)
{ //debug_size_cob
/*	char cmd[128], buf[MAX_RESULT_FILE_INDEX_FORMATION][128];
	unsigned char test_index;
	int	i, tmp;
	struct tm *tm;
	time_t t;
	FILE *fp;

	test_index = myPs->resultData_formation[group].test_index + 1;
	if(test_index > myPs->config.maxSaveNo) test_index = 1;
	myPs->resultData_formation[group].test_index = test_index;
	
	myPs->resultData_formation[group].save_count = 0;

	memset(cmd, 0, sizeof cmd);
	sprintf(cmd, "rm -rf /root/system_data/resultData/group%d/data%d",
		group+1, test_index);
	system(cmd);

	memset(cmd, 0, sizeof cmd);
	sprintf(cmd, "mkdir -p /root/system_data/resultData/group%d/data%d",
		group+1, test_index);
	system(cmd);

	time(&t);
	tm = localtime(&t);
	myPs->resultData_formation[group].open_year
		= (unsigned short)tm->tm_year+1900;
	myPs->resultData_formation[group].open_month
		= (unsigned char)tm->tm_mon+1;
	myPs->resultData_formation[group].open_day
		= (unsigned char)tm->tm_mday;
	myPs->resultData_formation[group].open_hour
		= (unsigned char)tm->tm_hour;
	myPs->resultData_formation[group].open_min
		= (unsigned char)tm->tm_min;
	myPs->resultData_formation[group].open_sec
		= (unsigned char)tm->tm_sec;

	memset(cmd, 0, sizeof cmd);
	sprintf(cmd, "%s/group%d/savingFileIndex_start.csv",
		"/root/system_data/resultData", group+1);

	test_index = i = 0;
	memset((char *)&buf[0][0], 0,
		sizeof(char)*MAX_RESULT_FILE_INDEX_FORMATION*128);

	fp = fopen(cmd, "r");
	if(fp == NULL) {
		userlog(DEBUG_LOG, psName, "Open_ResultData_1 NULL\n");
	} else if(fp >= 0) {
		while(1) {
			tmp = fscanf(fp, "%c", (char *)&buf[test_index][i]);
			if(buf[test_index][i] == 0) break;

			if(buf[test_index][i] == '\n') {
				test_index++;
				i = 0;
			} else {
				i++;
			}
			if(i >= 128) {
				test_index++;
				i = 0;
			}

			if(test_index >= myPs->config.maxSaveNo) break;
		}
		fclose(fp);
	} else {
		userlog(DEBUG_LOG, psName, "Open_ResultData_1 error -1\n");
		return -1;
	}

	fp = fopen(cmd, "w+");
	if(fp == NULL || fp < 0) {
		userlog(DEBUG_LOG, psName, "Open_ResultData_1 error -2\n");
		return -2;
	}

	if(test_index == myPs->config.maxSaveNo) tmp = 1;
	else tmp = 0;

	for(test_index=tmp; test_index < myPs->config.maxSaveNo; test_index++) {
		if(buf[test_index][i] == 0) break;
		for(i=0; i < 128; i++) {
			if(buf[test_index][i] == 0) {
				i = 0;
				break;
			}
			fprintf(fp, "%c", buf[test_index][i]);
		}
	}

	fprintf(fp, "index %d, ", myPs->resultData_formation[group].test_index);

	fprintf(fp, "open_day %4d/%02d/%02d, ",
		(int)myPs->resultData_formation[group].open_year,
		(int)myPs->resultData_formation[group].open_month,
		(int)myPs->resultData_formation[group].open_day);

	fprintf(fp, "open_time %02d:%02d:%02d, ",
		(int)myPs->resultData_formation[group].open_hour,
		(int)myPs->resultData_formation[group].open_min,
		(int)myPs->resultData_formation[group].open_sec);

	fprintf(fp, "test_serial_no %s, ",
		(char *)&myData->COB_Client[group].misc.test_serial_no);
	fprintf(fp, "tray_id %s\n", (char *)&myData->gData[group].tray_id);

	fflush(fp);
	fclose(fp);

	memset(cmd, 0, sizeof cmd);
	sprintf(cmd,
		"rm -rf /root/system_data/resultData/group%d/last_test_index_*",
		group+1);
	system(cmd);

	memset(cmd, 0, sizeof cmd);
	sprintf(cmd, "%s/group%d/last_test_index_%d",
		"/root/system_data/resultData", group+1,
		myPs->resultData_formation[group].test_index);

	fp = fopen(cmd, "w+");
	if(fp == NULL || fp < 0) {
		userlog(DEBUG_LOG, psName, "Open_ResultData_1 error -3\n");
		return -3;
	}
	fclose(fp);
*/
	return 0;
}

int Save_ResultData_Formation(int group)
{ //debug_size_cob
/*	int rtn;

	if(myData->COB_Client[group].config.protocol_version
		== P2_PROTOCOL_VERSION) {
		rtn = Save_ResultData_Formation_1(group);
	} else {
		rtn = Save_ResultData_Formation_2(group);
	}

	return rtn;*/
	return 0;
}

int Save_ResultData_Formation_1(int group)
{ //debug_size_cob
/*	char cmd[128];
	int fromPs, idx, count, chIdx, ch, i, chInGroup;
	FILE *fp;

	fromPs = SAVE_GROUP1_TO_DATASAVE + group;

	if(myData->f_save_msg[fromPs].write_idx
		== myData->f_save_msg[fromPs].read_idx) return 0;

	myData->f_save_msg[fromPs].read_idx++;
	if(myData->f_save_msg[fromPs].read_idx >= MAX_F_SAVE_MSG)
		myData->f_save_msg[fromPs].read_idx = 0;
	idx = myData->f_save_msg[fromPs].read_idx;

	myData->f_save_msg[fromPs].count--;
	count = myData->f_save_msg[fromPs].count;

	if(myPs->config.resultData_saveFlag == P0) return count;

//	userlog(DEBUG_LOG, psName, "data_save %d %d\n",
//		myData->f_save_msg[fromPs].val[idx].type,
//		myData->f_save_msg[fromPs].val[idx].attribute);

	i = 0; //0:return, 1:check_return, 2:save
	switch(myData->f_save_msg[fromPs].val[idx].type) {
		case P2_STEP_CYCLE:
			i = 0;
			break;
		case P2_STEP_LOOP:
			switch(myData->f_save_msg[fromPs].val[idx].attribute) {
				case ATTR_CHECK1_END:
				case ATTR_CHECK2_END:
					i = 1;
					break;
				default:
					i = 0;
					break;
			}
			break;
		case P2_STEP_CHARGE:
		case P2_STEP_DISCHARGE:
			switch(myData->f_save_msg[fromPs].val[idx].attribute) {
				case ATTR_IDLE:
				case ATTR_DC1_1:
				case ATTR_DC2_3:
				case ATTR_DC3_3:
					i = 2;
					break;
				default:
					i = 0;
					break;
			}
			break;
		case P2_STEP_OCV:
		case P2_STEP_REST:
		case P2_STEP_LONG_TIME_REST:
			switch(myData->f_save_msg[fromPs].val[idx].attribute) {
				case ATTR_IDLE:
					i = 2;
					break;
				default:
					i = 0;
					break;
			}
			break;
		case P2_STEP_END:
			i = 2;
			break;
		default:
			i = 0;
			break;
	}

	if(i == 0) {
		return count;
	} else if(i == 1) { //ckeck_step
		return count;
	}

	myPs->resultData_formation[group].save_count++;

	chInGroup = myData->mData.config.chInGroup[group];
	for(i=0; i < chInGroup; i++) {
		switch(myData->AppControl.config.systemModel) {
			case F_SDI_5V_400A_200A_100A_10A:
			case F_SDI_5V_400A_200A_100A_10A_2:
			case F_SDI_5V_450A_200A_100A_10A: //kjg_w
				chIdx = i;
				ch = i;
				break;
			default:
				chIdx = (int)myData->COB_Client[group].config.ChArray2[i];
				if(chIdx < 0) continue;
				ch = group * chInGroup + i;
				break;
		}

		memset(cmd, 0, sizeof cmd);
		sprintf(cmd, "%s%d/data%d/ch%03d_SaveData.csv",
			"/root/system_data/resultData/group", group+1,
			(int)myPs->resultData_formation[group].test_index, chIdx+1);
		fp = fopen(cmd, "a+");
		if(fp == NULL || fp < 0) { //kjg_w
			userlog(DEBUG_LOG, psName,
				"Save_ResultData %d error -1 (ch:%d)\n",
				(int)myPs->resultData_formation[group].test_index, chIdx+1);
			continue;
		}

		fprintf(fp,
			"%d, %ld, %ld, %d, %d, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f\n",
			(int)myPs->resultData_formation[group].save_count,
			(long)myData->f_save_msg[fromPs].val[idx].chData[ch].state,
			(long)myData->f_save_msg[fromPs].val[idx].chData[ch].code,
			(int)myData->f_save_msg[fromPs].val[idx].chData[ch].grade,
			(int)myData->f_save_msg[fromPs].val[idx].chData[ch].stepNo,
			(float)myData->f_save_msg[fromPs].val[idx].chData[ch]
				.totalRunTime / 100.0,
			(float)myData->f_save_msg[fromPs].val[idx].chData[ch]
				.runTime / 100.0,
			(float)myData->f_save_msg[fromPs].val[idx].chData[ch]
				.Vsens / 1000.0,
			(float)myData->f_save_msg[fromPs].val[idx].chData[ch]
				.Isens / 1000.0,
			(float)myData->f_save_msg[fromPs].val[idx].chData[ch].watt,
			(float)(myData->f_save_msg[fromPs].val[idx].chData[ch]
				.charge_WattHour
				+ myData->f_save_msg[fromPs].val[idx].chData[ch]
				.discharge_WattHour),
			(float)(myData->f_save_msg[fromPs].val[idx].chData[ch]
			 	.charge_AmpareHour
				+ myData->f_save_msg[fromPs].val[idx].chData[ch]
				.discharge_AmpareHour) / 1000.0,
			(float)myData->f_save_msg[fromPs].val[idx].chData[ch].z / 1000.0);
		fflush(fp);
		fclose(fp);
	}

	return count;*/
	return 0;
}

int Save_ResultData_Formation_2(int group)
{ //debug_size_cob
/*	char cmd[128];
	int fromPs, idx, count, chIdx, ch, i, chInGroup;
	FILE *fp;

	fromPs = SAVE_GROUP1_TO_DATASAVE + group;

	if(myData->f_save_msg[fromPs].write_idx
		== myData->f_save_msg[fromPs].read_idx) return 0;

	myData->f_save_msg[fromPs].read_idx++;
	if(myData->f_save_msg[fromPs].read_idx >= MAX_F_SAVE_MSG)
		myData->f_save_msg[fromPs].read_idx = 0;
	idx = myData->f_save_msg[fromPs].read_idx;

	myData->f_save_msg[fromPs].count--;
	count = myData->f_save_msg[fromPs].count;

	if(myPs->config.resultData_saveFlag == P0) return count;

	i = 0; //0:return, 1:check_return, 2:save
	switch(myData->f_save_msg[fromPs].val[idx].type) {
		case P2_STEP_CYCLE:
			i = 0;
			break;
		case P2_STEP_LOOP:
			switch(myData->f_save_msg[fromPs].val[idx].attribute) {
				case ATTR_CHECK1_END:
				case ATTR_CHECK2_END:
				case ATTR_CHECK3_END:
					i = 1;
					break;
				default:
					i = 0;
					break;
			}
			break;
		case P2_STEP_CHARGE:
		case P2_STEP_DISCHARGE:
			switch(myData->f_save_msg[fromPs].val[idx].attribute) {
				case ATTR_IDLE:
				case ATTR_DC1_1:
				case ATTR_DC2_3:
				case ATTR_DC3_3:
					i = 2;
					break;
				default:
					i = 0;
					break;
			}
			break;
		case P2_STEP_OCV:
		case P2_STEP_REST:
		case P2_STEP_LONG_TIME_REST:
			switch(myData->f_save_msg[fromPs].val[idx].attribute) {
				case ATTR_IDLE:
					i = 2;
					break;
				default:
					i = 0;
					break;
			}
			break;
		case P2_STEP_END:
			i = 2;
			break;
		default:
			i = 0;
			break;
	}

	if(i == 0) {
		return count;
	} else if(i == 1) { //check_step
		return count;
	}

	myPs->resultData_formation[group].save_count++;

	chInGroup = myData->mData.config.chInGroup[group];
	for(i=0; i < chInGroup; i++) {
		switch(myData->AppControl.config.systemModel) {
			case F_SDI_5V_400A_200A_100A_10A:
			case F_SDI_5V_400A_200A_100A_10A_2:
			case F_SDI_5V_450A_200A_100A_10A:
				chIdx = i;
				ch = i;
				break;
			case F_PNE_5V_15A_30AP_SW:
				if(myData->ChAttribute[0].opType == 0) {
					chIdx = (int)myData->COB_Client[group].config.ChArray2[i];
					if(chIdx < 0) continue;
					ch = group * chInGroup + i;
				} else {
					if((i % 2) != 0) continue;

					chIdx = (int)myData->COB_Client[group].config.ChArray2[i];
					chIdx /= 2;
					if(chIdx < 0) continue;
					ch = group * chInGroup + i;
				}
				break;
			default:
				chIdx = (int)myData->COB_Client[group].config.ChArray2[i];
				if(chIdx < 0) continue;
				ch = group * chInGroup + i;
				break;
		}
				
		memset(cmd, 0, sizeof cmd);
		sprintf(cmd, "%s%d/data%d/ch%03d_SaveData.csv",
			"/root/system_data/resultData/group", group+1,
			(int)myPs->resultData_formation[group].test_index, chIdx+1);
		fp = fopen(cmd, "a+");
		if(fp == NULL || fp < 0) { //kjg_w
			userlog(DEBUG_LOG, psName,
				"Save_ResultData %d error -1 (ch:%d)\n",
				(int)myPs->resultData_formation[group].test_index, chIdx+1);
			continue;
		}

		fprintf(fp,
			"%d, %ld, %ld, %d, %d, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f\n",
			(int)myPs->resultData_formation[group].save_count,
			(long)myData->f_save_msg[fromPs].val[idx].chData[ch].state,
			(long)myData->f_save_msg[fromPs].val[idx].chData[ch].code,
			(int)myData->f_save_msg[fromPs].val[idx].chData[ch].grade,
			(int)myData->f_save_msg[fromPs].val[idx].chData[ch].stepNo,
			(float)myData->f_save_msg[fromPs].val[idx].chData[ch]
				.totalRunTime / 100.0,
			(float)myData->f_save_msg[fromPs].val[idx].chData[ch]
				.runTime / 100.0,
			(float)myData->f_save_msg[fromPs].val[idx].chData[ch]
				.Vsens / 1000.0,
			(float)myData->f_save_msg[fromPs].val[idx].chData[ch]
				.Isens / 1000.0,
			(float)myData->f_save_msg[fromPs].val[idx].chData[ch].watt,
			(float)(myData->f_save_msg[fromPs].val[idx].chData[ch]
				.charge_WattHour
				+ myData->f_save_msg[fromPs].val[idx].chData[ch]
				.discharge_WattHour),
			(float)(myData->f_save_msg[fromPs].val[idx].chData[ch]
			 	.charge_AmpareHour
				+ myData->f_save_msg[fromPs].val[idx].chData[ch]
				.discharge_AmpareHour) / 1000.0,
			(float)myData->f_save_msg[fromPs].val[idx].chData[ch].z / 1000.0,
			(float)myData->f_save_msg[fromPs].val[idx].chData[ch].ccTime / 100.0);
		fflush(fp);
		fclose(fp);
	}

	return count;*/
	return 0;
}

int Open_MonitoringData_Formation_1(int group)
{ //debug_size_cob
/*	char cmd[128], buf[MAX_RESULT_FILE_INDEX_FORMATION][128];
	unsigned char test_index;
	int i, tmp, ch;
	struct tm *tm;
	time_t t;
	FILE *fp;

	test_index = myPs->monitorData_formation[group].test_index + 1;
	if(test_index > myPs->config.maxSaveNo) test_index = 1;
	myPs->monitorData_formation[group].test_index = test_index;

	for(ch=0; ch < myData->mData.config.chInGroup[group]; ch++) {
		myPs->monitorData_formation[group].file_index[ch] = 1;
		myPs->monitorData_formation[group].make_header[ch] = P1;
		myPs->monitorData_formation[group].save_count[ch] = 1;
	}

	memset(cmd, 0, sizeof cmd);
	sprintf(cmd, "rm -rf /root/system_data/monitoringData/group%d/data%d",
		group+1, test_index);
	system(cmd);

	memset(cmd, 0, sizeof cmd);
	sprintf(cmd, "mkdir -p /root/system_data/monitoringData/group%d/data%d",
		group+1, test_index);
	system(cmd);

	time(&t);
	tm = localtime(&t);
	myPs->monitorData_formation[group].open_year
		= (unsigned short)tm->tm_year+1900;
	myPs->monitorData_formation[group].open_month = (unsigned char)tm->tm_mon+1;
	myPs->monitorData_formation[group].open_day = (unsigned char)tm->tm_mday;
	myPs->monitorData_formation[group].open_hour = (unsigned char)tm->tm_hour;
	myPs->monitorData_formation[group].open_min = (unsigned char)tm->tm_min;
	myPs->monitorData_formation[group].open_sec = (unsigned char)tm->tm_sec;

	memset(cmd, 0, sizeof cmd);
	sprintf(cmd, "%s/group%d/savingFileIndex_start.csv",
		"/root/system_data/monitoringData", group+1);

	test_index = i = 0;
	memset((char *)&buf[0][0], 0,
		sizeof(char)*MAX_RESULT_FILE_INDEX_FORMATION*128);

	fp = fopen(cmd, "r");
	if(fp == NULL) {
		userlog(DEBUG_LOG, psName, "Open_MonitorData_1 NULL\n");
	} else if(fp >= 0) {
		while(1) {
			tmp = fscanf(fp, "%c", (char *)&buf[test_index][i]);
			if(buf[test_index][i] == 0) break;

			if(buf[test_index][i] == '\n') {
				test_index++;
				i = 0;
			} else {
				i++;
			}
			if(i >= 128) {
				test_index++;
				i = 0;
			}

			if(test_index >= myPs->config.maxSaveNo) break;
		}
		fclose(fp);
	} else {
		userlog(DEBUG_LOG, psName, "Open_MonitorData_1 error -1\n");
		return -1; //kjg_w
	}

	fp = fopen(cmd, "w+");
	if(fp == NULL || fp < 0) {
		userlog(DEBUG_LOG, psName, "Open_MonitorData_1 error -2\n");
		return -2;
	}

	if(test_index == myPs->config.maxSaveNo) tmp = 1;
	else tmp = 0;

	for(test_index=tmp; test_index < myPs->config.maxSaveNo; test_index++) {
		if(buf[test_index][i] == 0) break;
		for(i=0; i < 128; i++) {
			if(buf[test_index][i] == 0) {
				i = 0;
				break;
			}
			fprintf(fp, "%c", buf[test_index][i]);
		}
	}

	fprintf(fp, "index %d, ", myPs->monitorData_formation[group].test_index);

	fprintf(fp, "open_day %4d/%02d/%02d, ",
		(int)myPs->monitorData_formation[group].open_year,
		(int)myPs->monitorData_formation[group].open_month,
		(int)myPs->monitorData_formation[group].open_day);

	fprintf(fp, "open_time %02d:%02d:%02d, ",
		(int)myPs->monitorData_formation[group].open_hour,
		(int)myPs->monitorData_formation[group].open_min,
		(int)myPs->monitorData_formation[group].open_sec);

	fprintf(fp, "test_serial_no %s, ",
		(char *)&myData->COB_Client[group].misc.test_serial_no);
	fprintf(fp, "tray_id %s\n", (char *)&myData->gData[group].tray_id);

	fflush(fp);
	fclose(fp);

	Write_DataSaveNo(group);

	memset(cmd, 0, sizeof cmd);
	sprintf(cmd,
		"rm -rf /root/system_data/monitoringData/group%d/last_test_index_*",
		group+1);
	system(cmd);

	memset(cmd, 0, sizeof cmd);
	sprintf(cmd, "%s/group%d/last_test_index_%d",
		"/root/system_data/monitoringData", group+1,
		myPs->monitorData_formation[group].test_index);

	fp = fopen(cmd, "w+");
	if(fp == NULL || fp < 0) {
		userlog(DEBUG_LOG, psName, "Open_MonitorData_1 error -3\n");
		return -3;
	}
	fclose(fp);
*/
	return 0;
}

int Make_Header_MonitoringData_Formation(int group)
{ //debug_size_cob
/*	char cmd[128];
	int chIdx, ch;
	FILE *fp;

	for(ch=0; ch < myData->mData.config.chInGroup[group]; ch++) {
		if(myPs->monitorData_formation[group].make_header[ch] == P0) continue;
		myPs->monitorData_formation[group].make_header[ch] = P0;

		if(myData->ChAttribute[0].opType == 0) {
			chIdx = (int)myData->COB_Client[group].config.ChArray2[ch];
			if(chIdx < 0) continue;
		} else {
			if((ch % 2) != 0) continue;

			chIdx = (int)myData->COB_Client[group].config.ChArray2[ch];
			chIdx /= 2;
			if(chIdx < 0) continue;
		}
				
		memset(cmd, 0, sizeof cmd);
		sprintf(cmd, "%s%d/data%d/ch%03d_MonitoringData%02d.csv",
			"/root/system_data/monitoringData/group",
			group+1, (int)myPs->monitorData_formation[group].test_index,
			chIdx+1, (int)myPs->monitorData_formation[group].file_index[ch]);

		fp = fopen(cmd, "a+");
		if(fp == NULL || fp < 0) {
			userlog(DEBUG_LOG, psName,
				"Save_MonitoringData %d error (ch:%d)\n",
				(int)myPs->monitorData_formation[group].test_index, chIdx+1);
			continue;
		}

		fprintf(fp,
			"stepNo,State,Tot_t(Sec),Step_t(Sec),V(mV),I(mA),C(mAh),Wh(mWh),Temp\n");
		fflush(fp);
		fclose(fp);
	}
*/
	return 0;
}

void Open_MonitoringData_Formation_2(int group, int fromPs, int idx)
{
#ifdef __COB__
	int ch;

	for(ch=0; ch < myData->mData.config.chInGroup[group]; ch++) {
		if(myData->ChAttribute[0].opType == 0) {
		} else {
			if((ch % 2) != 0) continue;
		}

		if(myData->save_real_data_msg[fromPs].val[idx].chData[ch].save_flag
			== 0) continue;

		//line number = header + line => 65535 : Excel 2Byte
		myPs->monitorData_formation[group].save_count[ch]++;
		//if(myPs->monitorData_formation[group].save_count[ch] < 65535) continue;

		//myPs->monitorData_formation[group].file_index[ch]++;
		//myPs->monitorData_formation[group].make_header[ch] = P1;
		//myPs->monitorData_formation[group].save_count[ch] = 1;
	}
#endif
}

int Save_MonitoringData_Formation(int group)
{ //debug_size_cob
/*	char cmd[128];
	int fromPs, idx, count, chIdx, ch, chInGroup;
	FILE *fp;

	fromPs = SAVE_GROUP1_TO_REAL_DATASAVE + group;

	if(myData->save_real_data_msg[fromPs].write_idx 
		== myData->save_real_data_msg[fromPs].read_idx) return 0;
	if(myData->save_real_data_msg[fromPs].count <= 0) return 0;

	myData->save_real_data_msg[fromPs].read_idx++;
	if(myData->save_real_data_msg[fromPs].read_idx >= MAX_F_SAVE_MSG)
		myData->save_real_data_msg[fromPs].read_idx = 0;
	idx = myData->save_real_data_msg[fromPs].read_idx;

	myData->save_real_data_msg[fromPs].count--;
	count = myData->save_real_data_msg[fromPs].count;

	if(myPs->config.monitoringData_saveFlag == P0) return count;

	//Open_MonitoringData_Formation_2(group, fromPs, idx);

	Make_Header_MonitoringData_Formation(group);

	chInGroup = myData->mData.config.chInGroup[group];

	for(ch=0; ch < chInGroup; ch++) {
		switch(myData->AppControl.config.systemModel) {
			case F_SDI_5V_400A_200A_100A_10A:
			case F_SDI_5V_400A_200A_100A_10A_2:
			case F_SDI_5V_450A_200A_100A_10A:
				chIdx = ch;
				break;
			case F_PNE_5V_15A_30AP_SW:
				if(myData->ChAttribute[0].opType == 0) {
					chIdx = (int)myData->COB_Client[group].config.ChArray2[ch];
					if(chIdx < 0) continue;
				} else {
					if((ch % 2) != 0) continue;

					chIdx = (int)myData->COB_Client[group].config.ChArray2[ch];
					chIdx /= 2;
					if(chIdx < 0) continue;
				}
				break;
			default:
				chIdx = (int)myData->COB_Client[group].config.ChArray2[ch];
				if(chIdx < 0) continue;
				break;
		}

		if(myData->save_real_data_msg[fromPs].val[idx].chData[ch].save_flag
			== P0) continue;

		if(myData->save_real_data_msg[fromPs].val[idx].chData[ch].stepType
			== P2_STEP_CYCLE
			|| myData->save_real_data_msg[fromPs].val[idx].chData[ch].stepType
			== P2_STEP_LOOP
			|| myData->save_real_data_msg[fromPs].val[idx].chData[ch].stepType
			== P2_STEP_END
			|| myData->save_real_data_msg[fromPs].val[idx].chData[ch].stepType
			== P2_STEP_IDLE) {
			continue;
		}

		if(myData->save_real_data_msg[fromPs].val[idx].chData[ch].code
			== P2_C_CD_NONCELL
			|| myData->save_real_data_msg[fromPs].val[idx].chData[ch].code
			== P2_C_CD_CELL_CHECK_ERROR
			|| (myData->save_real_data_msg[fromPs].val[idx].chData[ch].code
			>= P2_C_CD_FAULT_CHECK_CODE_START
			&& myData->save_real_data_msg[fromPs].val[idx].chData[ch].code
			< P2_C_CD_FAULT_SOFT_CODE_START)) {
			continue;
		}
				
		memset(cmd, 0, sizeof cmd);
		sprintf(cmd, "%s%d/data%d/ch%03d_MonitoringData%02d.csv",
			"/root/system_data/monitoringData/group",
			group+1, (int)myPs->monitorData_formation[group].test_index,
			chIdx+1, (int)myPs->monitorData_formation[group].file_index[ch]);
		fp = fopen(cmd, "a+");
		if(fp == NULL || fp < 0) { //kjg_w
			userlog(DEBUG_LOG, psName,
				"Save_MonitoringData %d error (ch:%d)\n",
				(int)myPs->monitorData_formation[group].test_index, chIdx+1);
			continue;
		}

		fprintf(fp, "%d,%d,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f\n",
			(int)myData->save_real_data_msg[fromPs].val[idx].chData[ch].stepNo,
			(int)myData->save_real_data_msg[fromPs].val[idx].chData[ch].state,
			myData->save_real_data_msg[fromPs].val[idx].chData[ch]
				.totalRunTime/100.0,
			myData->save_real_data_msg[fromPs].val[idx].chData[ch]
				.runTime/100.0,
			myData->save_real_data_msg[fromPs].val[idx].chData[ch]
				.Vsens/1000.0,
			myData->save_real_data_msg[fromPs].val[idx].chData[ch]
				.Isens/1000.0,
			(myData->save_real_data_msg[fromPs].val[idx].chData[ch]
				.charge_AmpareHour
				+ myData->save_real_data_msg[fromPs].val[idx].chData[ch]
				.discharge_AmpareHour)/1000.0,
			(float)(myData->save_real_data_msg[fromPs].val[idx].chData[ch]
				.charge_WattHour
				+ myData->save_real_data_msg[fromPs].val[idx].chData[ch]
				.discharge_WattHour),
			(float)myData->save_real_data_msg[fromPs]
				.val[idx].chData[ch].temp / 1000.0);
		fflush(fp);
		fclose(fp);

		if(myData->COB_Client[group].config.send_sensor_data_interval > 0) {
			memset(cmd, 0, sizeof cmd);
			sprintf(cmd, "%s%d/data%d/sensor_data_ch%03d_%02d.csv",
				"/root/system_data/monitoringData/group",
				group+1, (int)myPs->monitorData_formation[group].test_index,
				chIdx+1,
				(int)myPs->monitorData_formation[group].file_index[ch]);

			fp = fopen(cmd, "a+");
			if(fp == NULL || fp < 0) {
				userlog(DEBUG_LOG, psName,
					"Save_SensorData %d error (ch:%d)\n", 
					(int)myPs->monitorData_formation[group].test_index,
					chIdx+1);
			}

			fprintf(fp, "0, %.3f, 0.0\n",
				(float)myData->save_real_data_msg[fromPs]
					.val[idx].chData[ch].temp / 1000.0); //time, temp, voltage
			fflush(fp);
			fclose(fp);
		}
	}

	return count;*/
	return 0;
}

int	Write_DataSaveNo(int group)
{
	char fileName[128];
    FILE *fp;

	memset(fileName, 0, sizeof(fileName));
	sprintf(fileName, 
		"/root/system_data/config/parameter/client/COB%d/%s",
		group+1, "DataSaveNo");
    if((fp = fopen(fileName, "w+")) == NULL) {
		userlog(DEBUG_LOG, psName, "%s DataSaveNo file write error\n", psName);
		return -1;
	}

    fprintf(fp, "%d\n", myData->DataSave.monitorData_formation[group].test_index);

    fclose(fp);

	return 0;
}
/* kjg_120723_w
int get_bd_index(int group, int idx)
{
	int i, bd;
	
	bd = idx / myData->mData.config.chPerBd;

	for(i=0; i < MAX_BD_PER_MODULE; i++) {
		if(myData->mData.config.bdInGroup[i] == group+1) {
			bd += i;
			break;
		}
	}
	
	return bd;
}

int get_ch_index(int group, int idx)
{
	int ch;

	ch = idx % myData->mData.config.chPerBd;

	return ch;
}
*/
int convert_datasave_code(int select, int code)
{
	int rtn_code;

	if(select == CONVERT_DATASAVE_TO_ORG) {
		switch(code) {
			case DATASAVE_CD_SAVED_FILE_DELETE_ERROR:
			case DATASAVE_CD_SEND_SAVE_MSG_ERROR:
			case DATASAVE_CD_RESULT_SAVE_FILE_ERROR:
				rtn_code = G_CD_FAULT_DATASAVE_PROCESS_ERROR;
				break;
			default:
				rtn_code = 0;
				break;
		}
	} else {
		rtn_code = 0;
	}

	return rtn_code;
}

int create_run_file(int ch)
{
	char cmd[128], path[128];

	memset(path, 0, sizeof path);
	sprintf(path, "/root/START_INFO/CH%03d", ch+1);
	// /root/START_INFO/CH###
	
	memset(cmd, 0, sizeof cmd);
	sprintf(cmd, "touch %s/run.txt", path);
	system(cmd);
	// touch path/run.txt

	userlog(DEBUG_LOG, psName, "ch %d run_file create\n", ch+1);
	return 0;
}

int delete_run_file(int ch)
{
	char cmd[128], path[128];
   	memset(path, 0, sizeof path);
	sprintf(path, "/root/START_INFO/CH%03d", ch+1);
	// /root/START_INFO/CH###

	memset(cmd, 0, sizeof cmd);
	sprintf(cmd, "rm %s/run.txt", path);
	system(cmd);
	// rm path/run.txt

	userlog(DEBUG_LOG, psName, "ch %d run_file delete\n", ch+1);

	return 0;
}
