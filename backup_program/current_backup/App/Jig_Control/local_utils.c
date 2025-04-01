#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "../../INC/datastore.h"
#include "userlog.h"
#include "common_utils.h"
#include "local_utils.h"
#include "local_message.h"

extern volatile S_SYSTEM_DATA	*myData;
extern volatile S_JIG_DATA		*myPs;
extern char psName[PROCESS_NAME_SIZE];

int Initialize(int argc, char *argv[])
{
	int JigNo, rtn;
			
	if(argc != 1) {
		printf("Jig_Control start fail %d, %s\n", argc, argv[0]);
		return -1;
	}
	
	if(Open_SystemMemory(0) < 0) return -2;

	JigNo = argv[0][5] - 0x31; //Jig?_Control
	myPs = &(myData->jData[JigNo]);
	
	Init_SystemMemory(JigNo);
	
	rtn = Read_JigControl_Config();
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "Read_Jig_Control_Config fail %d\n", rtn);
		return -10;
	}

	myData->AppControl.signal[myPs->misc.psSignal] = P1;
	return 0;
}

void Init_SystemMemory(int JigNo)
{
	int i;

	memset((char *)&psName, 0, PROCESS_NAME_SIZE);
	sprintf(psName, "Jig%d", JigNo+1);
	myPs->misc.psSignal = APP_SIG_JIG1_CONTROL_PROCESS + JigNo;

	memset((char *)&myPs->signal, 0, MAX_SIGNAL);

	myPs->jigState = J_IDLE;
	for(i=0; i < MAX_TRAY_16; i++) myPs->trayPreLoadState[i] = P0;
	myPs->microState = P0;
	myPs->emgPhase = P0;
	myPs->jobChangePhase = P0;
	myPs->jobChangeEnd = JOB_CHANGE_IDLE;
	
	myPs->traySensingDelay = myData->mData.misc.timer_1sec;
	myPs->emgCheckDelay = myData->mData.misc.timer_1sec;
	
	myPs->misc.processPointer = (int)&myData;
	myPs->misc.JigNo = JigNo;
}

int	Read_JigControl_Config(void)
{
	char temp[32], buf[32], fileName[256];
    int tmp;
    FILE *fp;

	memset(fileName, 0, sizeof fileName);
	strcpy(fileName, "/root/");
	strcat(fileName, (char *)&myData->AppControl.misc.path1);
	strcat(fileName, "/config/parameter/jig/");
	strcat(fileName, psName);
	strcat(fileName, "/Control_Config");
	// /root/system_data/config/parameter/jig/Jig?/Control_Config
    if((fp = fopen(fileName, "r")) == NULL) {
		userlog(DEBUG_LOG, psName,
			"%s Control_Config file read error\n", psName);
    	fclose(fp);
		return -1;
	}

	memset(temp, 0, sizeof temp);
    tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "jigUseFlag") != 0) {
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
	//0:UNUSE, 1:USE
    myPs->config.jigUseFlag = (unsigned char)atoi(buf);

	memset(temp, 0, sizeof temp);
    tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "jigType") != 0) {
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
	//1:LG1, 2:LG2, 3:SEBANG1, 4:LS, 5:LGC R&D, 6:VKEIG, 7:SKE, 8:SDI(chunan),
	//9:SDI2(kihung), 10:SDI3(chunan), 11:SDI4(ulsan)
    myPs->config.jigType = (unsigned char)atoi(buf);

	memset(temp, 0, sizeof temp);
    tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "trayType") != 0) {
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
	//1:LG1, 2:LG2, 3:SEBANG1, 4:LS, 5:LGC R&D, 6:VKEIG, 7:SKE, 8:SDI(chunan),
	//9:SDI2(kihung), 10:SDI3(chunan), 11:SDI4(ulsan)
    myPs->config.trayType = (unsigned char)atoi(buf);

	memset(temp, 0, sizeof temp);
    tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "totalTrayNo") != 0) {
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
    myPs->config.totalTrayNo = (unsigned char)atoi(buf);

	memset(temp, 0, sizeof temp);
    tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "localRemote") != 0) {
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
	//0:REMOTE, 1:LOCAL
    myPs->config.localRemote = (unsigned char)atoi(buf);

	memset(temp, 0, sizeof temp);
    tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "cylinderMoveTimeout") != 0) {
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
    myPs->config.cylMoveTimeout = atol(buf);

	memset(temp, 0, sizeof temp);
    tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "cylinderMoveDelay") != 0) {
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
    myPs->config.cylMoveDelay = atol(buf);

    fclose(fp);
	return 0;
}

int convert_jig_code(int select, int code)
{
	int rtn_code=0;

	switch(code) {
		case J_CD_MAIN_CYL_UP_TIMEOUT:	
			rtn_code = G_CD_FAULT_J_MAIN_CYL_UP_TIMEOUT;
			break;
		case J_CD_MAIN_CYL_DOWN_TIMEOUT:	
			rtn_code = G_CD_FAULT_J_MAIN_CYL_DOWN_TIMEOUT;
			break;
		case J_CD_LATCH_CYL_BOTH_OPEN_TIMEOUT:
		case J_CD_LATCH_CYL_R_OPEN_TIMEOUT:	
		case J_CD_LATCH_CYL_L_OPEN_TIMEOUT:	
			rtn_code = G_CD_FAULT_J_LATCH_CYL_OPEN_TIMEOUT;
			break;
		case J_CD_LATCH_CYL_BOTH_CLOSE_TIMEOUT:	
		case J_CD_LATCH_CYL_R_CLOSE_TIMEOUT:	
		case J_CD_LATCH_CYL_L_CLOSE_TIMEOUT:	
			rtn_code = G_CD_FAULT_J_LATCH_CYL_CLOSE_TIMEOUT;
			break;
		case J_CD_GRIP_CYL_OPEN_TIMEOUT:
			rtn_code = G_CD_FAULT_J_GRIP_CYL_OPEN_TIMEOUT;
			break;
		case J_CD_GRIP_CYL_CLOSE_TIMEOUT:
			rtn_code = G_CD_FAULT_J_GRIP_CYL_CLOSE_TIMEOUT;
			break;
		case J_CD_MAIN_CYL_UP_ERROR:
			rtn_code = G_CD_FAULT_J_MAIN_CYL_UP_SENS;
			break;
		case J_CD_MAIN_CYL_DOWN_ERROR:
			rtn_code = G_CD_FAULT_J_MAIN_CYL_DOWN_SENS;
			break;
		case J_CD_LATCH_CYL_R_OPEN_ERROR:
		case J_CD_LATCH_CYL_L_OPEN_ERROR:
			rtn_code = G_CD_FAULT_J_LATCH_CYL_OPEN_SENS;
			break;
		case J_CD_LATCH_CYL_R_CLOSE_ERROR:
		case J_CD_LATCH_CYL_L_CLOSE_ERROR:
			rtn_code = G_CD_FAULT_J_LATCH_CYL_CLOSE_SENS;
			break;
		case J_CD_GRIP_CYL_OPEN_ERROR:
			rtn_code = G_CD_FAULT_J_GRIP_CYL_OPEN_SENS;
			break;
		case J_CD_GRIP_CYL_CLOSE_ERROR:
			rtn_code = G_CD_FAULT_J_GRIP_CYL_CLOSE_SENS;
			break;
		case J_CD_STOPPER_ERROR:
			rtn_code = G_CD_FAULT_JIG_ACTIVE_ERROR;
			break;
		case J_CD_SMOKE_SENS_1:
		case J_CD_SMOKE_SENS_2:
			rtn_code = G_CD_FAULT_JIG_SMOKE_ERROR;
			break;
		case J_CD_OVER_TEMP_1:
		case J_CD_OVER_TEMP_2:
			rtn_code = G_CD_FAULT_JIG_TEMP_ERROR;
			break;
		case J_CD_TRAY_UNLOAD:
		case J_CD_TRAY_DIR_ERROR:
			rtn_code = G_CD_FAULT_JIG_TRAY_ERROR;
			break;
		case J_CD_AIR_PRESS_ERROR:
			rtn_code = G_CD_FAULT_JIG_AIR_PRESS_ERROR;
			break;
		case J_CD_EMG_SWITCH:
			rtn_code = M_CD_FAULT_SUB_EMG_SWITCH;
			break;
		case J_CD_CHAMBER_FAULT:
			rtn_code = M_CD_FAULT_CHAMBER;
			break;
		case J_CD_CHAMBER_DOOR_OPEN_ERROR:
			rtn_code = M_CD_FAULT_CHAMBER_DOOR_OPEN;
			//rtn_code = M_CD_FAULT_CHAMBER;
			break;
		case J_CD_JOB_CHANGE_ERROR:
			rtn_code = M_CD_FAULT_JOB_CHANGE;
			break;
		case J_CD_PITCH_CHANGE_ERROR:
			rtn_code = M_CD_FAULT_PITCH_CHANGE;
			break;
		case J_CD_CHAMBER_AREA_SENSOR:
			rtn_code = M_CD_FAULT_CHAMBER_AREA_SENSOR;
			break;
		case J_CD_CHAMBER_MANUAL_DOOR_OPEN:
			rtn_code = M_CD_FAULT_CHAMBER_MANUAL_DOOR;
			break;
		case J_CD_CHAMBER_AIR:
			rtn_code = M_CD_FAULT_CHAMBER_AIR;
			break;
		case J_CD_CHAMBER_APR_S:
			rtn_code = M_CD_FAULT_CHAMBER_APR_S;
			break;
		case J_CD_CHAMBER_REF:
			rtn_code = M_CD_FAULT_CHAMBER_REF;
			break;
		case J_CD_CHAMBER_COIL_TRIP:
			rtn_code = M_CD_FAULT_CHAMBER_COIL_TRIP;
			break;
		case J_CD_CHAMBER_FIRE:
			rtn_code = M_CD_FAULT_CHAMBER_FIRE;
			break;
		default: //kjg_w_f
			rtn_code = 255;
			break;
	}
	return rtn_code;
}
