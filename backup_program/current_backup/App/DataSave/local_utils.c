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
	//myPs->config.maxSaveNo = (unsigned char)i;
	myPs->config.maxSaveNo = i; //kjg_130923 unsigned char -> int

	tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	myPs->config.checkData_saveFlag = (unsigned char)atoi(buf);

	tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);	//ktg_210411s
	memset(buf, 0, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	myPs->config.saveTimeFlag = (unsigned char)atoi(buf);			//ktg_210411e

	fclose(fp);
	return 0;
}

int Open_ResultData_1(int p_ch)
{
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

	myPs->resultData[p_ch].file_index = 1;
	myPs->resultData[p_ch].max_file_index = 1;
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

	fprintf(fp, "index, totalTime_day, totalTime, stepTime_day, stepTime, state, type, mode, select, code, grade, stepNo, Vsens, Isens, charge_AmpareHour, discharge_AmpareHour, capacitance, watt, charge_WattHour, discharge_WattHour, impedance, reservedCmd, external_comm_state, ch_output_state, ch_input_state, auxCount, canCount, totalCycle, elementCycle, accCycle1, accCycle2, accCycle3, accCycle4, accCycle5, multiCycle1, multiCycle2, multiCycle3, multiCycle4, multiCycle5, averageV, averageI, cvTime_day, cvTime, realDate, realClock, Vinput, Vpower, Vbus, out_mux_use, out_mux_backup, minAuxV, minAuxV_auxChNo, maxAuxV, maxAuxV_auxChNo, minAuxTemp, minAuxTemp_auxChNo, maxAuxTemp, maxAuxTemp_auxChNo, minAuxTH, minAuxTH_auxChNo, maxAuxTH, maxAuxTH_auxChNo, sub_code1, sub_code2, sub_code3, sub_code4, freeze_flag, cv_flag, freeze_time, freeze_ChNo, CycleTime_day, CycleTime\n");

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

		fprintf(fp, "index, totalTime_day, totalTime, stepTime_day, stepTime, CycleTime_day, CycleTime");

		for(i=0; i < myData->mData.config.installedTemp; i++) {
			if(p_ch+1 == myData->auxSetData[i].chNo) {
				fprintf(fp, ", value(%d)", (int)myData->auxSetData[i].auxChNo);
			}
		}
		fprintf(fp, "\n");
		fflush(fp);
		fclose(fp);
	}

	if((myData->mData.config.installedAuxV != 0)	//ktg_210706s
		|| (myData->mData.config.installedTH != 0)
		|| (myData->mData.config.installedHumidity != 0)//) {	//ktg_210706e
		|| (myData->mData.config.installedGas != 0)) {	//sec_220926
		memset(cmd, 0, sizeof(cmd));
		sprintf(cmd, "%s/ch%03d_SaveEndData_auxV.csv", path, p_ch+1);
		// path/ch###_SaveEndData_auxV.csv
		fp = fopen(cmd, "w");
		if(fp == NULL || fp < 0) {
			userlog(DEBUG_LOG, psName, "Open_ResultData_1 error -3\n");
			return -3;
		}

		fprintf(fp, "index, totalTime_day, totalTime, stepTime_day, stepTime, CycleTime_day, CycleTime");

		j = myData->mData.config.installedTemp + myData->mData.config.installedAuxV
			+ myData->mData.config.installedTH + myData->mData.config.installedHumidity
			+ myData->mData.config.installedGas;	//ktg_210706	//sec_220926

		for(i=myData->mData.config.installedTemp; i < j; i++) {
			if(p_ch+1 == myData->auxSetData[i].chNo) {
				fprintf(fp, ", value(%d)", (int)myData->auxSetData[i].auxChNo);
			}
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

		fprintf(fp, "index, totalTime_day, totalTime, stepTime_day, stepTime, CycleTime_day, CycleTime");

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

		fprintf(fp, "index, totalTime_day, totalTime, stepTime_day, stepTime, CycleTime_day, CycleTime");

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

	fprintf(fp, "fileIndex %d, ", (int)myPs->resultData[p_ch].file_index);
	fprintf(fp, "resultIndex %ld, ", myData->cData[ch].op.resultIndex+1);
	fprintf(fp, "maxFileIndex %d, ", myPs->resultData[p_ch].max_file_index);
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

	fprintf(fp, "fileIndex %d, ", (int)myPs->resultData[p_ch].file_index);
	fprintf(fp, "resultIndex %ld, ", myData->cData[ch].op.resultIndex+1);
	fprintf(fp, "maxFileIndex %d, ", myPs->resultData[p_ch].max_file_index);
	fprintf(fp, "open_year %d, ", (int)myPs->resultData[p_ch].open_year);
	fprintf(fp, "open_month %d, ", (int)myPs->resultData[p_ch].open_month);
	fprintf(fp, "open_day %d\n", (int)myPs->resultData[p_ch].open_day);
	fflush(fp);
	fclose(fp);

	memset(cmd, 0, sizeof(cmd));
	sprintf(cmd, "%s/ch%03d_SaveData%04d.csv",
		path, p_ch+1, (int)myPs->resultData[p_ch].file_index);
	// path/ch###_SaveData####.csv
	fp = fopen(cmd, "w");
	if(fp == NULL || fp < 0) {
		userlog(DEBUG_LOG, psName, "Open_ResultData_1 error -10\n");
		return -10; //kjg_w
	}

	fprintf(fp, "index, totalTime_day, totalTime, stepTime_day, stepTime, state, type, mode, select, code, grade, stepNo, Vsens, Isens, charge_AmpareHour, discharge_AmpareHour, capacitance, watt, charge_WattHour, discharge_WattHour, impedance, reservedCmd, external_comm_state, ch_output_state, ch_input_state, auxCount, canCount, totalCycle, elementCycle, accCycle1, accCycle2, accCycle3, accCycle4, accCycle5, multiCycle1, multiCycle2, multiCycle3, multiCycle4, multiCycle5, averageV, averageI, cvTime_day, cvTime, realDate, realClock, Vinput, Vpower, Vbus, out_mux_use, out_mux_backup, minAuxV, minAuxV_auxChNo, maxAuxV, maxAuxV_auxChNo, minAuxTemp, minAuxTemp_auxChNo, maxAuxTemp, maxAuxTemp_auxChNo, minAuxTH, minAuxTH_auxChNo, maxAuxTH, maxAuxTH_auxChNo, sub_code1, sub_code2, sub_code3, sub_code4, freeze_flag, cv_flag, freeze_time, freeze_ChNo, CycleTime_day, CycleTime\n");

	fflush(fp);
	fclose(fp);

	if(myData->mData.config.installedTemp != 0) {
		memset(cmd, 0, sizeof(cmd));
		sprintf(cmd, "%s/ch%03d_SaveData%04d_auxT.csv",
			path, p_ch+1, (int)myPs->resultData[p_ch].file_index);
		// path/ch###_SaveData####_auxT.csv
		fp = fopen(cmd, "w");
		if(fp == NULL || fp < 0) {
			userlog(DEBUG_LOG, psName, "Open_ResultData_1 error -11\n");
			return -11; //kjg_w
		}

		fprintf(fp, "index, totalTime_day, totalTime, stepTime_day, stepTime, CycleTime_day, CycleTime");

		for(i=0; i < myData->mData.config.installedTemp; i++) {
			if(p_ch+1 == myData->auxSetData[i].chNo) {
				fprintf(fp, ", value(%d)", (int)myData->auxSetData[i].auxChNo);
			}
		}
		fprintf(fp, "\n");
		fflush(fp);
		fclose(fp);
	}

	if((myData->mData.config.installedAuxV != 0)	//ktg_210706s
		|| (myData->mData.config.installedTH != 0)
		|| (myData->mData.config.installedHumidity != 0)//) {	//ktg_210706e
		|| (myData->mData.config.installedGas != 0)) {	//sec_220926
		memset(cmd, 0, sizeof(cmd));
		sprintf(cmd, "%s/ch%03d_SaveData%04d_auxV.csv",
			path, p_ch+1, (int)myPs->resultData[p_ch].file_index);
		// path/ch###_SaveData####_auxV.csv
		fp = fopen(cmd, "w");
		if(fp == NULL || fp < 0) {
			userlog(DEBUG_LOG, psName, "Open_ResultData_1 error -12\n");
			return -12; //kjg_w
		}

		fprintf(fp, "index, totalTime_day, totalTime, stepTime_day, stepTime, CycleTime_day, CycleTime");

		j = myData->mData.config.installedTemp + myData->mData.config.installedAuxV
			+ myData->mData.config.installedTH + myData->mData.config.installedHumidity
			+ myData->mData.config.installedGas;	//sec_220926

		for(i=myData->mData.config.installedTemp; i < j; i++) {
			if(p_ch+1 == myData->auxSetData[i].chNo) {
				fprintf(fp, ", value(%d)", (int)myData->auxSetData[i].auxChNo);
			}
		}
		fprintf(fp, "\n");
		fflush(fp);
		fclose(fp);
	}

	if(myData->mData.config.installedCAN != 0) {
		memset(cmd, 0, sizeof(cmd));
		sprintf(cmd, "%s/ch%03d_SaveData%04d_canMaster.csv",
			path, p_ch+1, (int)myPs->resultData[p_ch].file_index);
		// path/ch###_SaveData####_canMaster.csv
		fp = fopen(cmd, "w");
		if(fp == NULL || fp < 0) {
			userlog(DEBUG_LOG, psName, "Open_ResultData_1 error -13\n");
			return -13; //kjg_w
		}

		fprintf(fp, "index, totalTime_day, totalTime, stepTime_day, stepTime, CycleTime_day, CycleTime");

		for(i=0; i < myData->canReceiveDataCount[ch][0]; i++) {
			fprintf(fp, ", value(%d)", i+1);
		}
		fprintf(fp, "\n");
		fflush(fp);
		fclose(fp);

		memset(cmd, 0, sizeof(cmd));
		sprintf(cmd,
			"%s/ch%03d_SaveData%04d_canSlave.csv",
			path, p_ch+1, (int)myPs->resultData[p_ch].file_index);
		// path/ch###_SaveData####_canSlave.csv
		fp = fopen(cmd, "w");
		if(fp == NULL || fp < 0) {
			userlog(DEBUG_LOG, psName, "Open_ResultData_1 error -14\n");
			return -14; //kjg_w
		}

		fprintf(fp, "index, totalTime_day, totalTime, stepTime_day, stepTime, CycleTime_day, CycleTime");

		j = myData->canReceiveDataCount[ch][0]
			+ myData->canReceiveDataCount[ch][1];
		for(i=myData->canReceiveDataCount[ch][0]; i < j; i++) {
			fprintf(fp, ", value(%d)", i+1);
		}
		fprintf(fp, "\n");
		fflush(fp);
		fclose(fp);

		memset(cmd, 0, sizeof(cmd)); //ktg_231219s
		sprintf(cmd,
			"%s/ch%03d_SaveData%04d_ref.csv",
			path, p_ch+1, (int)myPs->resultData[p_ch].file_index);
		// path/ch###_SaveData####_ref.csv
		fp = fopen(cmd, "w");
		if(fp == NULL || fp < 0) {
			userlog(DEBUG_LOG, psName, "Open_ResultData_1 error -15\n");
			return -15; //kjg_w
		}

		fprintf(fp, "index, totalTime_day, totalTime, stepTime_day, stepTime");

		fprintf(fp, ", refV1, refI");
		fprintf(fp, "\n");
		fflush(fp);
		fclose(fp);	//ktg_231219e

	}

	return 0;
}

int Open_ResultData_2(int p_ch)
{
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

	myPs->resultData[p_ch].file_index++;
	myPs->resultData[p_ch].max_file_index++;
	//if(myPs->resultData[p_ch].file_index > MAX_RESULT_FILE_INDEX) {	//ktg_210419s
	//	myPs->resultData[p_ch].file_index = 1;
	//}
	if(myPs->resultData[p_ch].file_index > MAX_RESULT_FILE_COUNT) {
		myPs->resultData[p_ch].file_index = 1;
	}	//ktg_210419e

	ch = myData->CellArray1[p_ch].number2 - 1;

	memset(cmd, 0, sizeof cmd);
	sprintf(cmd, "%s/savingFileIndex_start.csv", path);
	// path/savingFileIndex_start.csv
	fp = fopen(cmd, "a");
	if(fp == NULL || fp < 0) {
		userlog(DEBUG_LOG, psName, "Open_ResultData_2 error -6\n");
		return -6; //kjg_w
	}

	fprintf(fp, "fileIndex %d, ", (int)myPs->resultData[p_ch].file_index);
	fprintf(fp, "resultIndex %ld, ", myPs->resultData[p_ch].last_resultIndex+1);
	fprintf(fp, "maxFileIndex %d, ", myPs->resultData[p_ch].max_file_index);
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

	fprintf(fp, "fileIndex %d, ", (int)myPs->resultData[p_ch].file_index);
	fprintf(fp, "resultIndex %ld, ", myPs->resultData[p_ch].last_resultIndex+1);
	fprintf(fp, "maxFileIndex %d, ", myPs->resultData[p_ch].max_file_index);
	fprintf(fp, "open_year %d, ", (int)myPs->resultData[p_ch].open_year);
	fprintf(fp, "open_month %d, ", (int)myPs->resultData[p_ch].open_month);
	fprintf(fp, "open_day %d\n", (int)myPs->resultData[p_ch].open_day);
	fflush(fp);
	fclose(fp);

	memset(cmd, 0, sizeof cmd);
	sprintf(cmd, "rm -rf %s/ch%03d_SaveData%04d.csv",
		path, p_ch+1, (int)myPs->resultData[p_ch].file_index);
	//rm -rf path/ch###_SaveData####.csv
	system(cmd);

	if(myPs->resultData[p_ch].file_index > MAX_RESULT_FILE_INDEX) {	//ktg_210419s
		sprintf(cmd, "rm -rf %s/ch%03d_SaveData%04d.csv",
			path, p_ch+1, (int)myPs->resultData[p_ch].file_index - MAX_RESULT_FILE_INDEX );
		//rm -rf path/ch###_SaveData####.csv
		system(cmd);
	} else if(myPs->resultData[p_ch].max_file_index > myPs->resultData[p_ch].file_index) {
		sprintf(cmd, "rm -rf %s/ch%03d_SaveData%04d.csv",path, p_ch+1,
		MAX_RESULT_FILE_COUNT - (MAX_RESULT_FILE_INDEX - (int)myPs->resultData[p_ch].file_index));
		//rm -rf path/ch###_SaveData####.csv
		system(cmd);
	}	//ktg_210419e

	if(myData->mData.config.installedTemp != 0) {
		memset(cmd, 0, sizeof cmd);
		sprintf(cmd, "rm -rf %s/ch%03d_SaveData%04d_auxT.csv",
			path, p_ch+1, (int)myPs->resultData[p_ch].file_index);
		//rm -rf path/ch###_SaveData####_auxT.csv
		system(cmd);
	}

	if(myPs->resultData[p_ch].file_index > MAX_RESULT_FILE_INDEX) {	//ktg_210419s
		if(myData->mData.config.installedTemp != 0) {
			memset(cmd, 0, sizeof cmd);
			sprintf(cmd, "rm -rf %s/ch%03d_SaveData%04d_auxT.csv",
				path, p_ch+1, (int)myPs->resultData[p_ch].file_index - MAX_RESULT_FILE_INDEX);
			//rm -rf path/ch###_SaveData####_auxT.csv
			system(cmd);
		}
	} else if(myPs->resultData[p_ch].max_file_index > myPs->resultData[p_ch].file_index) {
		if(myData->mData.config.installedTemp != 0) {
			memset(cmd, 0, sizeof cmd);
			sprintf(cmd, "rm -rf %s/ch%03d_SaveData%04d_auxT.csv",path, p_ch+1,
				MAX_RESULT_FILE_COUNT - (MAX_RESULT_FILE_INDEX - (int)myPs->resultData[p_ch].file_index));
			//rm -rf path/ch###_SaveData####_auxT.csv
			system(cmd);
		}
	}	//ktg_210419e

	if((myData->mData.config.installedAuxV != 0)	//ktg_210706s
		|| (myData->mData.config.installedTH != 0)
		|| (myData->mData.config.installedHumidity != 0)//) {	//ktg_210706e
		|| (myData->mData.config.installedGas != 0)) {	//sec_220926
		memset(cmd, 0, sizeof cmd);
		sprintf(cmd, "rm -rf %s/ch%03d_SaveData%04d_auxV.csv",
			path, p_ch+1, (int)myPs->resultData[p_ch].file_index);
		//rm -rf path/ch###_SaveData####_auxV.csv
		system(cmd);
	}

	if(myPs->resultData[p_ch].file_index > MAX_RESULT_FILE_INDEX) {	//ktg_210419s
		if((myData->mData.config.installedAuxV != 0)	//ktg_210706s
			|| (myData->mData.config.installedTH != 0)
			|| (myData->mData.config.installedHumidity != 0)//) {	//ktg_210706e
			|| (myData->mData.config.installedGas != 0)) {	//sec_220926
			memset(cmd, 0, sizeof cmd);
			sprintf(cmd, "rm -rf %s/ch%03d_SaveData%04d_auxV.csv",
				path, p_ch+1, (int)myPs->resultData[p_ch].file_index - MAX_RESULT_FILE_INDEX);
			//rm -rf path/ch###_SaveData####_auxV.csv
			system(cmd);
		}
 	} else if(myPs->resultData[p_ch].max_file_index > myPs->resultData[p_ch].file_index) {
		if((myData->mData.config.installedAuxV != 0)	//ktg_210706s
			|| (myData->mData.config.installedTH != 0)
			|| (myData->mData.config.installedHumidity != 0)//) {	//ktg_210706e
			|| (myData->mData.config.installedGas != 0)) {	//sec_220926
			memset(cmd, 0, sizeof cmd);
			sprintf(cmd, "rm -rf %s/ch%03d_SaveData%04d_auxV.csv", path, p_ch+1,
			MAX_RESULT_FILE_COUNT - (MAX_RESULT_FILE_INDEX - (int)myPs->resultData[p_ch].file_index));
			//rm -rf path/ch###_SaveData####_auxV.csv
			system(cmd);
		}
	}	//ktg_210419e

	if(myData->mData.config.installedCAN != 0) {
		memset(cmd, 0, sizeof cmd);
		sprintf(cmd, "rm -rf %s/ch%03d_SaveData%04d_canMaster.csv",
			path, p_ch+1, (int)myPs->resultData[p_ch].file_index);
		//rm -rf path/ch###_SaveData####_canMaster.csv
		system(cmd);

		memset(cmd, 0, sizeof cmd);
		sprintf(cmd,
		"rm -rf %s/ch%03d_SaveData%04d_canSlave.csv",
			path, p_ch+1, (int)myPs->resultData[p_ch].file_index);
		//rm -rf path/ch###_SaveData####_canSlave.csv
		system(cmd);
	}

	if(myPs->resultData[p_ch].file_index > MAX_RESULT_FILE_INDEX) {	//ktg_210419s
		if(myData->mData.config.installedCAN != 0) {
			memset(cmd, 0, sizeof cmd);
			sprintf(cmd, "rm -rf %s/ch%03d_SaveData%04d_canMaster.csv",
				path, p_ch+1, (int)myPs->resultData[p_ch].file_index - MAX_RESULT_FILE_INDEX);
			//rm -rf path/ch###_SaveData####_canMaster.csv
			system(cmd);
	
			memset(cmd, 0, sizeof cmd);
			sprintf(cmd,
			"rm -rf %s/ch%03d_SaveData%04d_canSlave.csv",
				path, p_ch+1, (int)myPs->resultData[p_ch].file_index - MAX_RESULT_FILE_INDEX);
			//rm -rf path/ch###_SaveData####_canSlave.csv
			system(cmd);
		}
	} else if(myPs->resultData[p_ch].max_file_index > myPs->resultData[p_ch].file_index) {
		if(myData->mData.config.installedCAN != 0) {
			memset(cmd, 0, sizeof cmd);
			sprintf(cmd, "rm -rf %s/ch%03d_SaveData%04d_canMaster.csv",path, p_ch+1,
				MAX_RESULT_FILE_COUNT - (MAX_RESULT_FILE_INDEX - (int)myPs->resultData[p_ch].file_index));
			//rm -rf path/ch###_SaveData####_canMaster.csv
			system(cmd);
	
			memset(cmd, 0, sizeof cmd);
			sprintf(cmd,
			"rm -rf %s/ch%03d_SaveData%04d_canSlave.csv",path, p_ch+1,
				MAX_RESULT_FILE_COUNT - (MAX_RESULT_FILE_INDEX - (int)myPs->resultData[p_ch].file_index));
			//rm -rf path/ch###_SaveData####_canSlave.csv
			system(cmd);
		}
	}	//ktg_210419e

	if(myPs->resultData[p_ch].file_index > MAX_RESULT_FILE_INDEX) {	//ktg_231219s
			memset(cmd, 0, sizeof cmd);
			sprintf(cmd,
			"rm -rf %s/ch%03d_SaveData%04d_ref.csv",
				path, p_ch+1, (int)myPs->resultData[p_ch].file_index - MAX_RESULT_FILE_INDEX);
			//rm -rf path/ch###_SaveData####_ref.csv
			system(cmd);
	} else if(myPs->resultData[p_ch].max_file_index > myPs->resultData[p_ch].file_index) {
			memset(cmd, 0, sizeof cmd);
			sprintf(cmd,
			"rm -rf %s/ch%03d_SaveData%04d_ref.csv",path, p_ch+1,
				MAX_RESULT_FILE_COUNT - (MAX_RESULT_FILE_INDEX - (int)myPs->resultData[p_ch].file_index));
			//rm -rf path/ch###_SaveData####_ref.csv
			system(cmd);
	}	//ktg_231219e
	
	memset(cmd, 0, sizeof(cmd));
	sprintf(cmd, "%s/ch%03d_SaveData%04d.csv",
		path, p_ch+1, (int)myPs->resultData[p_ch].file_index);
	// path/ch###_SaveData####.csv
	fp = fopen(cmd, "w");
	if(fp == NULL || fp < 0) {
		userlog(DEBUG_LOG, psName, "Open_ResultData_2 error -10\n");
		return -10; //kjg_w
	}

	fprintf(fp, "index, totalTime_day, totalTime, stepTime_day, stepTime, state, type, mode, select, code, grade, stepNo, Vsens, Isens, charge_AmpareHour, discharge_AmpareHour, capacitance, watt, charge_WattHour, discharge_WattHour, impedance, reservedCmd, external_comm_state, ch_output_state, ch_input_state, auxCount, canCount, totalCycle, elementCycle, accCycle1, accCycle2, accCycle3, accCycle4, accCycle5, multiCycle1, multiCycle2, multiCycle3, multiCycle4, multiCycle5, averageV, averageI, cvTime_day, cvTime, realDate, realClock, Vinput, Vpower, Vbus, out_mux_use, out_mux_backup, minAuxV, minAuxV_auxChNo, maxAuxV, maxAuxV_auxChNo, minAuxTemp, minAuxTemp_auxChNo, maxAuxTemp, maxAuxTemp_auxChNo, minAuxTH, minAuxTH_auxChNo, maxAuxTH, maxAuxTH_auxChNo, sub_code1, sub_code2, sub_code3, sub_code4, freeze_flag, cv_flag, freeze_time, freeze_ChNo, CycleTime_day, CycleTime\n");

	fflush(fp);
	fclose(fp);

	if(myData->mData.config.installedTemp != 0) {
		memset(cmd, 0, sizeof(cmd));
		sprintf(cmd, "%s/ch%03d_SaveData%04d_auxT.csv",
			path, p_ch+1, (int)myPs->resultData[p_ch].file_index);
		// path/ch###_SaveData####_auxT.csv
		fp = fopen(cmd, "w");
		if(fp == NULL || fp < 0) {
			userlog(DEBUG_LOG, psName, "Open_ResultData_2 error -11\n");
			return -11; //kjg_w
		}

		fprintf(fp, "index, totalTime_day, totalTime, stepTime_day, stepTime, CycleTime_day, CycleTime");

		for(i=0; i < myData->mData.config.installedTemp; i++) {
			if(p_ch+1 == myData->auxSetData[i].chNo) {
				fprintf(fp, ", value(%d)", (int)myData->auxSetData[i].auxChNo);
			}
		}
		fprintf(fp, "\n");
		fflush(fp);
		fclose(fp);
	}

	if((myData->mData.config.installedAuxV != 0)	//ktg_210706s
		|| (myData->mData.config.installedTH != 0)
		|| (myData->mData.config.installedHumidity != 0)//) {	//ktg_210706e
		|| (myData->mData.config.installedGas != 0)) {	//sec_220926

		memset(cmd, 0, sizeof(cmd));
		sprintf(cmd, "%s/ch%03d_SaveData%04d_auxV.csv",
			path, p_ch+1, (int)myPs->resultData[p_ch].file_index);
		// path/ch###_SaveData####_auxV.csv
		fp = fopen(cmd, "w");
		if(fp == NULL || fp < 0) {
			userlog(DEBUG_LOG, psName, "Open_ResultData_2 error -12\n");
			return -12; //kjg_w
		}

		fprintf(fp, "index, totalTime_day, totalTime, stepTime_day, stepTime, CycleTime_day, CycleTime");

		j = myData->mData.config.installedTemp + myData->mData.config.installedAuxV
			+ myData->mData.config.installedTH + myData->mData.config.installedHumidity
			+ myData->mData.config.installedGas;		//ktg_210706	//sec_220926

		for(i=myData->mData.config.installedTemp; i < j; i++) {
			if(p_ch+1 == myData->auxSetData[i].chNo) {
				fprintf(fp, ", value(%d)", (int)myData->auxSetData[i].auxChNo);
			}
		}
		fprintf(fp, "\n");
		fflush(fp);
		fclose(fp);
	}

	if(myData->mData.config.installedCAN != 0) {
		memset(cmd, 0, sizeof(cmd));
		sprintf(cmd, "%s/ch%03d_SaveData%04d_canMaster.csv",
			path, p_ch+1, (int)myPs->resultData[p_ch].file_index);
		// path/ch###_SaveData####_canMaster.csv
		fp = fopen(cmd, "w");
		if(fp == NULL || fp < 0) {
			userlog(DEBUG_LOG, psName, "Open_ResultData_2 error -13\n");
			return -13; //kjg_w
		}
		fprintf(fp, "index, totalTime_day, totalTime, stepTime_day, stepTime, CycleTime_day, CycleTime");

		for(i=0; i < myData->canReceiveDataCount[ch][0]; i++) {
			fprintf(fp, ", value(%d)", i+1);
		}
		fprintf(fp, "\n");
		fflush(fp);
		fclose(fp);

		memset(cmd, 0, sizeof(cmd));
		sprintf(cmd, "%s/ch%03d_SaveData%04d_canSlave.csv",
			path, p_ch+1, (int)myPs->resultData[p_ch].file_index);
		// path/ch###_SaveData####_canSlave.csv
		fp = fopen(cmd, "w");
		if(fp == NULL || fp < 0) {
			userlog(DEBUG_LOG, psName, "Open_ResultData_2 error -14\n");
			return -14; //kjg_w
		}

		fprintf(fp, "index, totalTime_day, totalTime, stepTime_day, stepTime, CycleTime_day, CycleTime");

		j = myData->canReceiveDataCount[ch][0]
			+ myData->canReceiveDataCount[ch][1];
		for(i=myData->canReceiveDataCount[ch][0]; i < j; i++) {
			fprintf(fp, ", value(%d)", i+1);
		}
		fprintf(fp, "\n");
		fflush(fp);
		fclose(fp);
	}

	memset(cmd, 0, sizeof(cmd));	//ktg_231219s
	sprintf(cmd, "%s/ch%03d_SaveData%04d_ref.csv",
		path, p_ch+1, (int)myPs->resultData[p_ch].file_index);
	// path/ch###_SaveData####_ref.csv
	fp = fopen(cmd, "w");
	if(fp == NULL || fp < 0) {
		userlog(DEBUG_LOG, psName, "Open_ResultData_2 error -14\n");
		return -14; //kjg_w
	}

	fprintf(fp, "index, totalTime_day, totalTime, stepTime_day, stepTime");

	fprintf(fp, ", refV1, refI");
	fprintf(fp, "\n");
	fflush(fp);
	fclose(fp);		//ktg_231219e

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
	sprintf(cmd, "%s/ch%03d_SaveData%04d.csv",
		path, ch+1, (int)myPs->resultData[ch].file_index);
	// path/ch###_SaveData####.csv
	fp = fopen(cmd, "a");
	if(fp == NULL || fp < 0) {
		userlog(DEBUG_LOG, psName, "Save_ResultData error -1 (ch:%d)\n", ch+1);
		return -2;
	}

	fprintf(fp,
		"%ld, %ld, %ld, %ld, %ld, %d, %d, %d, %d, %d, %d, %d, %ld, %ld, %ld, %ld, %ld, %ld, %ld, %ld, %ld, %d, %d, %d, %d, %d, %d, %ld, %ld, %ld, %ld, %ld, %ld, %ld, %ld, %ld, %ld, %ld, %ld, %ld, %ld, %ld, %ld, %ld, %ld, %ld, %ld, %ld, %d, %d, %ld, %d, %ld, %d, %ld, %d, %ld, %d, %ld, %d, %ld, %d, %d, %d, %d, %d, %d, %d, %d, %d, %ld, %ld\n",
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
		/*myData->save_msg[msg].val[idx].chData.reserved2[0], //jhkw_211014s
		myData->save_msg[msg].val[idx].chData.reserved2[1],
		myData->save_msg[msg].val[idx].chData.reserved2[2],
		myData->save_msg[msg].val[idx].chData.reserved2[3],
		myData->save_msg[msg].val[idx].chData.reserved2[4],
		myData->save_msg[msg].val[idx].chData.reserved2[5]);*/
		myData->save_msg[msg].val[idx].chData.minAuxV,
		myData->save_msg[msg].val[idx].chData.minAuxV_auxChNo,
		myData->save_msg[msg].val[idx].chData.maxAuxV,
		myData->save_msg[msg].val[idx].chData.maxAuxV_auxChNo,
		myData->save_msg[msg].val[idx].chData.minAuxTemp,
		myData->save_msg[msg].val[idx].chData.minAuxTemp_auxChNo,
		myData->save_msg[msg].val[idx].chData.maxAuxTemp,
		myData->save_msg[msg].val[idx].chData.maxAuxTemp_auxChNo,
		myData->save_msg[msg].val[idx].chData.minAuxTH,
		myData->save_msg[msg].val[idx].chData.minAuxTH_auxChNo,
		myData->save_msg[msg].val[idx].chData.maxAuxTH,
		myData->save_msg[msg].val[idx].chData.maxAuxTH_auxChNo,
		myData->save_msg[msg].val[idx].chData.sub_code[0].si_val[0], //jhkw_220103
		myData->save_msg[msg].val[idx].chData.sub_code[1].si_val[0],
		myData->save_msg[msg].val[idx].chData.sub_code[2].si_val[0],
		myData->save_msg[msg].val[idx].chData.sub_code[3].si_val[0], //jhkw_211014e
		myData->save_msg[msg].val[idx].chData.freeze_flag,
		myData->save_msg[msg].val[idx].chData.cv_flag,			//ktg_230728
		myData->save_msg[msg].val[idx].chData.maxFreeze_CANTime,			//ktg_230822
		myData->save_msg[msg].val[idx].chData.maxFreeze_CANChNo,			//ktg_230822
		myData->save_msg[msg].val[idx].chData.CycleTime_day,
		myData->save_msg[msg].val[idx].chData.CycleTime);	//ktg_231115
	fflush(fp);
	fclose(fp);

	if(myData->mData.config.installedTemp != 0) {
		memset(cmd, 0, sizeof cmd);
		sprintf(cmd, "%s/ch%03d_SaveData%04d_auxT.csv",
			path, ch+1, (int)myPs->resultData[ch].file_index);
		// path/ch###_SaveData####_auxT.csv
		fp = fopen(cmd, "a");
		if(fp == NULL || fp < 0) {
			userlog(DEBUG_LOG, psName,
				"Save_ResultData error -2 (ch:%d)\n", ch+1);
			return -3;
		}

		fprintf(fp, "%ld, %ld, %ld, %ld, %ld, %ld, %ld",
			myData->save_msg[msg].val[idx].chData.resultIndex,
			myData->save_msg[msg].val[idx].chData.totalRunTime_day,
			myData->save_msg[msg].val[idx].chData.totalRunTime,
			myData->save_msg[msg].val[idx].chData.runTime_day,
			myData->save_msg[msg].val[idx].chData.runTime,
			myData->save_msg[msg].val[idx].chData.CycleTime_day,
			myData->save_msg[msg].val[idx].chData.CycleTime);	//ktg_231115

		for(i=0; i < myData->auxDataCount[ch][0]; i++) {
			fprintf(fp, ", %ld",
				myData->save_msg[msg].val[idx].auxData[i].val);
		}
		fprintf(fp, "\n");
		fflush(fp);
		fclose(fp);
	}

	if((myData->mData.config.installedAuxV != 0)	//ktg_210706s
		|| (myData->mData.config.installedTH != 0)
		|| (myData->mData.config.installedHumidity != 0)//) {	//ktg_210706e
		|| (myData->mData.config.installedGas != 0)) {	//sec_220926
		memset(cmd, 0, sizeof cmd);
		sprintf(cmd, "%s/ch%03d_SaveData%04d_auxV.csv",
			path, ch+1, (int)myPs->resultData[ch].file_index);
		// path/ch###_SaveData####_auxV.csv
		fp = fopen(cmd, "a");
		if(fp == NULL || fp < 0) {
			userlog(DEBUG_LOG, psName,
				"Save_ResultData error -3 (ch:%d)\n", ch+1);
			return -4;
		}

		fprintf(fp, "%ld, %ld, %ld, %ld, %ld, %ld, %ld",
			myData->save_msg[msg].val[idx].chData.resultIndex,
			myData->save_msg[msg].val[idx].chData.totalRunTime_day,
			myData->save_msg[msg].val[idx].chData.totalRunTime,
			myData->save_msg[msg].val[idx].chData.runTime_day,
			myData->save_msg[msg].val[idx].chData.runTime,
			myData->save_msg[msg].val[idx].chData.CycleTime_day,
			myData->save_msg[msg].val[idx].chData.CycleTime);	//ktg_231115

		j = myData->auxDataCount[ch][0] + myData->auxDataCount[ch][1]
			+ myData->auxDataCount[ch][2] + myData->auxDataCount[ch][3]
			+ myData->auxDataCount[ch][4];	//khj_191205	//sec_220926

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
		sprintf(cmd, "%s/ch%03d_SaveData%04d_canMaster.csv",
			path, ch+1, (int)myPs->resultData[ch].file_index);
		// path/ch###_SaveData####_canMaster.csv
		fp = fopen(cmd, "a");
		if(fp == NULL || fp < 0) {
			userlog(DEBUG_LOG, psName,
				"Save_ResultData error -4 (ch:%d)\n", ch+1);
			return -5;
		}

		fprintf(fp, "%ld, %ld, %ld, %ld, %ld, %ld, %ld",
			myData->save_msg[msg].val[idx].chData.resultIndex,
			myData->save_msg[msg].val[idx].chData.totalRunTime_day,
			myData->save_msg[msg].val[idx].chData.totalRunTime,
			myData->save_msg[msg].val[idx].chData.runTime_day,
			myData->save_msg[msg].val[idx].chData.runTime,
			myData->save_msg[msg].val[idx].chData.CycleTime_day,
			myData->save_msg[msg].val[idx].chData.CycleTime);	//ktg_231115

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
		sprintf(cmd, "%s/ch%03d_SaveData%04d_canSlave.csv",
			path, ch+1, (int)myPs->resultData[ch].file_index);
		// path/ch###_SaveData####_canSlave.csv
		fp = fopen(cmd, "a");
		if(fp == NULL || fp < 0) {
			userlog(DEBUG_LOG, psName,
				"Save_ResultData error -5 (ch:%d)\n", ch+1);
			return -6;
		}

		fprintf(fp, "%ld, %ld, %ld, %ld, %ld, %ld, %ld",
			myData->save_msg[msg].val[idx].chData.resultIndex,
			myData->save_msg[msg].val[idx].chData.totalRunTime_day,
			myData->save_msg[msg].val[idx].chData.totalRunTime,
			myData->save_msg[msg].val[idx].chData.runTime_day,
			myData->save_msg[msg].val[idx].chData.runTime,
			myData->save_msg[msg].val[idx].chData.CycleTime_day,
			myData->save_msg[msg].val[idx].chData.CycleTime);	//ktg_231115

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

		memset(cmd, 0, sizeof cmd);	//ktg_231219s
		sprintf(cmd, "%s/ch%03d_SaveData%04d_ref.csv",
			path, ch+1, (int)myPs->resultData[ch].file_index);
		// path/ch###_SaveData####_ref.csv
		fp = fopen(cmd, "a");
		if(fp == NULL || fp < 0) {
			userlog(DEBUG_LOG, psName,
				"Save_ResultData error -7 (ch:%d)\n", ch+1);
			return -7;
		}

		fprintf(fp, "%ld, %ld, %ld, %ld, %ld",
			myData->save_msg[msg].val[idx].chData.resultIndex,
			myData->save_msg[msg].val[idx].chData.totalRunTime_day,
			myData->save_msg[msg].val[idx].chData.totalRunTime,
			myData->save_msg[msg].val[idx].chData.runTime_day,
			myData->save_msg[msg].val[idx].chData.runTime);

		fprintf(fp, ", %ld, %ld", 
			//myData->test_val_l[ch],
			//myData->test_val_l[ch+4]);
			myData->test_val_l1[ch],
			myData->test_val_l1[ch+4]);
		fprintf(fp, "\n");
		fflush(fp);
		fclose(fp);	//ktg_231219e


	if(myData->save_msg[msg].val[idx].chData.select == SAVE_FLAG_SAVING_ETC) {
		userlog(DEBUG_LOG, psName,
			"CH%d PAUSE resultIndex(%ld), runTime(%ld), stepType(%d), stepNo(%d), code(%d), Vsens(%ld), real_time(%ld day : %ld)\n",
			ch+1, myData->save_msg[msg].val[idx].chData.resultIndex,
			myData->save_msg[msg].val[idx].chData.runTime,
			(int)myData->save_msg[msg].val[idx].chData.stepType,
			(int)myData->save_msg[msg].val[idx].chData.stepNo,
			(int)myData->save_msg[msg].val[idx].chData.code,
			myData->save_msg[msg].val[idx].chData.Vsens,
			myData->save_msg[msg].val[idx].chData.realDate,
			myData->save_msg[msg].val[idx].chData.realClock
		);
	}	//csk_200529
	//shhw_220919s
	 if(myData->save_msg[msg].val[idx].chData.select == SAVE_FLAG_SAVING_ETC) {
		if(myData->save_msg[msg].val[idx].chData.code == 267) {
		   userlog(DEBUG_LOG, psName,"CH%d time_init(%ld) code(%d) preV(%ld)\n",ch+1,
		    myData->cData[ch].misc.time_init1, (int)myData->save_msg[msg].val[idx].chData.code,
			myData->cData[ch].misc.preV1);
		}
		if(myData->save_msg[msg].val[idx].chData.code == 268) {
		   userlog(DEBUG_LOG, psName,"CH%d time_init(%ld) code(%d) preV(%ld)\n",ch+1,
		    myData->cData[ch].misc.time_init_ori, (int)myData->save_msg[msg].val[idx].chData.code,
			myData->cData[ch].misc.preV2);
		}
	}
	//shhw_220919e
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

		fprintf(fp,
			"%ld, %ld, %ld, %ld, %ld, %d, %d, %d, %d, %d, %d, %d, %ld, %ld, %ld, %ld, %ld, %ld, %ld, %ld, %ld, %d, %d, %d, %d, %d, %d, %ld, %ld, %ld, %ld, %ld, %ld, %ld, %ld, %ld, %ld, %ld, %ld, %ld, %ld, %ld, %ld, %ld, %ld, %ld, %ld, %ld, %d, %d, %ld, %d, %ld, %d, %ld, %d, %ld, %d, %ld, %d, %ld, %d, %d, %d, %d, %d, %d, %d, %d, %d, %ld, %ld\n",
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
			/*myData->save_msg[msg].val[idx].chData.reserved2[0], //jhkw_211014s
			myData->save_msg[msg].val[idx].chData.reserved2[1],
			myData->save_msg[msg].val[idx].chData.reserved2[2],
			myData->save_msg[msg].val[idx].chData.reserved2[3],
			myData->save_msg[msg].val[idx].chData.reserved2[4],
			myData->save_msg[msg].val[idx].chData.reserved2[5]); */
			myData->save_msg[msg].val[idx].chData.minAuxV,
			myData->save_msg[msg].val[idx].chData.minAuxV_auxChNo,
			myData->save_msg[msg].val[idx].chData.maxAuxV,
			myData->save_msg[msg].val[idx].chData.maxAuxV_auxChNo,
			myData->save_msg[msg].val[idx].chData.minAuxTemp,
			myData->save_msg[msg].val[idx].chData.minAuxTemp_auxChNo,
			myData->save_msg[msg].val[idx].chData.maxAuxTemp,
			myData->save_msg[msg].val[idx].chData.maxAuxTemp_auxChNo,
			myData->save_msg[msg].val[idx].chData.minAuxTH,
			myData->save_msg[msg].val[idx].chData.minAuxTH_auxChNo,
			myData->save_msg[msg].val[idx].chData.maxAuxTH,
			myData->save_msg[msg].val[idx].chData.maxAuxTH_auxChNo,
			myData->save_msg[msg].val[idx].chData.sub_code[0].si_val[0], //jhkw_220103
			myData->save_msg[msg].val[idx].chData.sub_code[1].si_val[0],
			myData->save_msg[msg].val[idx].chData.sub_code[2].si_val[0],
			myData->save_msg[msg].val[idx].chData.sub_code[3].si_val[0], //jhkw_211014e
			myData->save_msg[msg].val[idx].chData.freeze_flag,
			myData->save_msg[msg].val[idx].chData.cv_flag,			//ktg_230728
			myData->save_msg[msg].val[idx].chData.maxFreeze_CANTime,			//ktg_230822
			myData->save_msg[msg].val[idx].chData.maxFreeze_CANChNo,			//ktg_230822
			myData->save_msg[msg].val[idx].chData.CycleTime_day,
			myData->save_msg[msg].val[idx].chData.CycleTime);	//ktg_231115
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

			fprintf(fp, "%ld, %ld, %ld, %ld, %ld, %ld, %ld",
				myData->save_msg[msg].val[idx].chData.resultIndex,
				myData->save_msg[msg].val[idx].chData.totalRunTime_day,
				myData->save_msg[msg].val[idx].chData.totalRunTime,
				myData->save_msg[msg].val[idx].chData.runTime_day,
				myData->save_msg[msg].val[idx].chData.runTime,
				myData->save_msg[msg].val[idx].chData.CycleTime_day,
				myData->save_msg[msg].val[idx].chData.CycleTime);	//ktg_231115

			for(i=0; i < myData->auxDataCount[ch][0]; i++) {
				fprintf(fp, ", %ld",
					myData->save_msg[msg].val[idx].auxData[i].val);
			}
			fprintf(fp, "\n");
			fflush(fp);
			fclose(fp);
		}

		if((myData->mData.config.installedAuxV != 0)	//ktg_210706s
			|| (myData->mData.config.installedTH != 0)
			|| (myData->mData.config.installedHumidity != 0)//) {	//ktg_210706e
			|| (myData->mData.config.installedGas != 0)) {	//sec_220926
			memset(cmd, 0, sizeof cmd);
			sprintf(cmd, "%s/ch%03d_SaveEndData_auxV.csv", path, ch+1);
			// path/ch###_SaveEndData_auxV.csv
			fp = fopen(cmd, "a");
			if(fp == NULL || fp < 0) {
				userlog(DEBUG_LOG, psName,
					"Save_ResultData error -12 (ch:%d)\n", ch+1);
				return -12;
			}

			fprintf(fp, "%ld, %ld, %ld, %ld, %ld, %ld, %ld",
				myData->save_msg[msg].val[idx].chData.resultIndex,
				myData->save_msg[msg].val[idx].chData.totalRunTime_day,
				myData->save_msg[msg].val[idx].chData.totalRunTime,
				myData->save_msg[msg].val[idx].chData.runTime_day,
				myData->save_msg[msg].val[idx].chData.runTime,
				myData->save_msg[msg].val[idx].chData.CycleTime_day,
				myData->save_msg[msg].val[idx].chData.CycleTime);	//ktg_231115

			j = myData->auxDataCount[ch][0] + myData->auxDataCount[ch][1]
				+ myData->auxDataCount[ch][2] + myData->auxDataCount[ch][3]
				+ myData->auxDataCount[ch][4];	//ktg_210726	//sec_220926

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

			fprintf(fp, "%ld, %ld, %ld, %ld, %ld, %ld, %ld",
				myData->save_msg[msg].val[idx].chData.resultIndex,
				myData->save_msg[msg].val[idx].chData.totalRunTime_day,
				myData->save_msg[msg].val[idx].chData.totalRunTime,
				myData->save_msg[msg].val[idx].chData.runTime_day,
				myData->save_msg[msg].val[idx].chData.runTime,
				myData->save_msg[msg].val[idx].chData.CycleTime_day,
				myData->save_msg[msg].val[idx].chData.CycleTime);	//ktg_231115

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

			fprintf(fp, "%ld, %ld, %ld, %ld, %ld, %ld, %ld",
				myData->save_msg[msg].val[idx].chData.resultIndex,
				myData->save_msg[msg].val[idx].chData.totalRunTime_day,
				myData->save_msg[msg].val[idx].chData.totalRunTime,
				myData->save_msg[msg].val[idx].chData.runTime_day,
				myData->save_msg[msg].val[idx].chData.runTime,
				myData->save_msg[msg].val[idx].chData.CycleTime_day,
				myData->save_msg[msg].val[idx].chData.CycleTime);	//ktg_231115

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

	fprintf(fp, "fileIndex %d, ", (int)myPs->resultData[ch].file_index);
	fprintf(fp, "resultIndex %ld, ",
		myData->save_msg[msg].val[idx].chData.resultIndex);
	fprintf(fp, "maxFileIndex %d, ", myPs->resultData[ch].max_file_index);
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

#endif
	return 0;
}

int Open_ResultData_General_2(int p_ch)
{
#ifdef __COC__

#endif
	return 0;
}

void Save_ResultData_General_1(int type)
{ //kjg_110921
#ifdef __COC__

#endif
}

int Save_ResultData_General_2(int ch)
{
	int count=0;
#ifdef __COC__

#endif
	return count;
}

int Save_RecordData_General(int ch)
{
	int count=0;
#ifdef __COC__

#endif
	return count;
}

int Open_ResultData_Formation_1(int group)
{ //debug_size_cob

	return 0;
}

int Save_ResultData_Formation(int group)
{ //debug_size_cob

	return 0;
}

int Save_ResultData_Formation_1(int group)
{ //debug_size_cob

	return 0;
}

int Save_ResultData_Formation_2(int group)
{ //debug_size_cob

	return 0;
}

int Open_MonitoringData_Formation_1(int group)
{ //debug_size_cob

	return 0;
}

int Make_Header_MonitoringData_Formation(int group)
{ //debug_size_cob

	return 0;
}

void Open_MonitoringData_Formation_2(int group, int fromPs, int idx)
{
#ifdef __COB__

#endif
}

int Save_MonitoringData_Formation(int group)
{ //debug_size_cob

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
