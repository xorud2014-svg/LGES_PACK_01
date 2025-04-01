#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../../INC/datastore.h"
#include "userlog.h"
#include "common_utils.h"
#include "local_utils.h"
#include "local_message.h"
#include "emg_type.h"

extern volatile S_SYSTEM_DATA	*myData;
extern volatile S_JIG_DATA		*myPs;
extern char psName[PROCESS_NAME_SIZE];

void Check_EMG(void)
{
	if(myPs->config.jigUseFlag == UNUSE) {
		return;
	}

	switch(myPs->config.jigType) {
		case 1:	Check_EMG_1();	break;
		case 2: Check_EMG_2();	break;
		case 6: Check_EMG_6();	break;
		case 7: Check_EMG_7();	break;
		case 8:	Check_EMG_8();	break;
		case 9: Check_EMG_9();	break;
		case 10: Check_EMG_10();	break;
		case 11: Check_EMG_11();	break;
		default: break;
	}
}

void Check_EMG_1(void)
{
	int flag1, flag2, flag3, diff, ch=0;

	switch(myPs->emgPhase){
		case P0:
			diff = myData->mData.misc.timer_1sec
				-myPs->emgCheckDelay;
			if(diff >= myData->dio.config.dioDelay / 1000){
				myPs->emgPhase = P1;
			}
			break;
		case P1:
			flag1 = Read_InPoint(myPs->misc.JigNo, 1, I_JIG_SMOKE_SENS);
			flag2 = Read_InPoint(myPs->misc.JigNo, 2, I_JIG_SMOKE_SENS);
			flag3 = Read_InPoint(myPs->misc.JigNo, ch, I_JIG_OT_SENS);
			if(flag1 == ON){
				myPs->code = J_CD_SMOKE_SENS_1;
				myPs->jigState = J_EMG;
				myPs->microState = P0;
				myPs->troublePos = 1;
				myPs->emgPhase = P2;
				break;
			}else if(flag2 == ON){
				myPs->code = J_CD_SMOKE_SENS_2;
				myPs->jigState = J_EMG;
				myPs->microState = P0;
				myPs->troublePos = 2;
				myPs->emgPhase = P2;
				break;
			}else if(flag3 == ON){
				myPs->code = J_CD_OVER_TEMP_1;
				myPs->jigState = J_EMG;
				myPs->microState = P0;
				myPs->emgPhase = P2;
				break;
			}else{
				myPs->troublePos = 0;
			}
			break;
		default: break;
	}
}

void Check_EMG_2(void)
{
	int flag1, flag2, flag3, diff, ch=0;

	switch(myPs->emgPhase){
		case P0:
			diff = myData->mData.misc.timer_1sec
				- myPs->emgCheckDelay;
			if(diff >= myData->dio.config.dioDelay / 1000){
				myPs->emgPhase = P1;
			}
			break;
		case P1:
			flag1 = Read_InPoint(myPs->misc.JigNo, 1, I_JIG_SMOKE_SENS);
			flag2 = Read_InPoint(myPs->misc.JigNo, 2, I_JIG_SMOKE_SENS);
			flag3 = Read_InPoint(myPs->misc.JigNo, ch, I_JIG_OT_SENS);
			if(flag1 == ON){
				myPs->code = J_CD_SMOKE_SENS_1;
				myPs->jigState = J_EMG;
				myPs->microState = P0;
				myPs->troublePos = 1;
				myPs->emgPhase = P2;
				break;
			}else if(flag2 == ON){
				myPs->code = J_CD_SMOKE_SENS_2;
				myPs->jigState = J_EMG;
				myPs->microState = P0;
				myPs->troublePos = 2;
				myPs->emgPhase = P2;
				break;
			}else if(flag3 == ON){
				myPs->code = J_CD_OVER_TEMP_1;
				myPs->jigState = J_EMG;
				myPs->microState = P0;
				myPs->emgPhase = P2;
				break;
			}else{
				myPs->troublePos = 0;
			}
			break;
		default: break;
	}
}

void Check_EMG_6(void)
{
	int flag1, flag2, flag3, flag4, diff;

	switch(myPs->emgPhase){
		case P0:
			diff = myData->mData.misc.timer_1sec
				-myPs->emgCheckDelay;
			if(diff >= myData->dio.config.dioDelay / 1000){
				myPs->emgPhase = P1;
			}
			break;
		case P1:
			flag1 = Read_InPoint(myPs->misc.JigNo, 1, I_JIG_SMOKE_SENS);
			flag2 = Read_InPoint(myPs->misc.JigNo, 2, I_JIG_SMOKE_SENS);
			flag3 = Read_InPoint(myPs->misc.JigNo, 1, I_JIG_OT_SENS);
			flag4 = Read_InPoint(myPs->misc.JigNo, 2, I_JIG_OT_SENS);
			if(flag1 == ON){
				myPs->code = J_CD_SMOKE_SENS_1;
				myPs->jigState = J_EMG;
				myPs->microState = P0;
				myPs->troublePos = 1;
				myPs->emgPhase = P2;
				break;
			}else if(flag2 == ON){
				myPs->code = J_CD_SMOKE_SENS_2;
				myPs->jigState = J_EMG;
				myPs->microState = P0;
				myPs->troublePos = 2;
				myPs->emgPhase = P2;
				break;
			}else if(flag3 == ON){
				myPs->code = J_CD_OVER_TEMP_1;
				myPs->jigState = J_EMG;
				myPs->microState = P0;
				myPs->troublePos = 1;
				myPs->emgPhase = P2;
				break;
			}else if(flag4 == ON){
				myPs->code = J_CD_OVER_TEMP_2;
				myPs->jigState = J_EMG;
				myPs->microState = P0;
				myPs->troublePos = 2;
				myPs->emgPhase = P2;
				break;
			}else{
				myPs->troublePos = 0;
			}
			break;
		default: break;
	}
}

void Check_EMG_7(void)
{
	int flag1, flag2, diff;

	switch(myPs->emgPhase){
		case P0:
			diff = myData->mData.misc.timer_1sec
				- myPs->emgCheckDelay;
			if(diff >= myData->dio.config.dioDelay / 1000){
				myPs->emgPhase = P1;
			}
			break;
		case P1:
			flag1 = Read_InPoint(myPs->misc.JigNo, 1, I_JIG_SMOKE_SENS);
			flag2 = Read_InPoint(myPs->misc.JigNo, 2, I_JIG_SMOKE_SENS);
			if(flag1 == ON){
				myPs->code = J_CD_SMOKE_SENS_1;
				myPs->jigState = J_EMG;
				myPs->microState = P0;
				myPs->troublePos = 1;
				myPs->emgPhase = P2;
				userlog(DEBUG_LOG, psName,
					"%s Smoke Sens Detect[%d]", psName, myPs->troublePos);
				break;
			}else if(flag2 == ON){
				myPs->code = J_CD_SMOKE_SENS_2;
				myPs->jigState = J_EMG;
				myPs->microState = P0;
				myPs->troublePos = 2;
				myPs->emgPhase = P2;
				userlog(DEBUG_LOG, psName,
					"%s Smoke Sens Detect[%d]", psName, myPs->troublePos);
				break;
			}else{
				myPs->troublePos = 0;
			}
			break;
		default: break;
	}
}

void Check_EMG_8(void)
{
	int flag1, flag2, diff;

	switch(myPs->emgPhase){
		case P0:
			diff = myData->mData.misc.timer_1sec
				- myPs->emgCheckDelay;
			if(diff >= myData->dio.config.dioDelay / 1000){
				myPs->emgPhase = P1;
			}
			break;
		case P1:
			flag1 = Read_InPoint(myPs->misc.JigNo, 1, I_JIG_SMOKE_SENS);
			flag2 = Read_InPoint(myPs->misc.JigNo, 2, I_JIG_SMOKE_SENS);
			if(flag1 == ON){
				myPs->code = J_CD_SMOKE_SENS_1;
				myPs->jigState = J_EMG;
				myPs->microState = P0;
				myPs->troublePos = 1;
				myPs->emgPhase = P2;
				userlog(DEBUG_LOG, psName,
					"%s Smoke Sens Detect[%d]", psName, myPs->troublePos);
				break;
			}else if(flag2 == ON){
				myPs->code = J_CD_SMOKE_SENS_2;
				myPs->jigState = J_EMG;
				myPs->microState = P0;
				myPs->troublePos = 2;
				myPs->emgPhase = P2;
				userlog(DEBUG_LOG, psName,
					"%s Smoke Sens Detect[%d]", psName, myPs->troublePos);
				break;
			}else{
				myPs->troublePos = 0;
			}
			break;
		default: break;
	}
}

void Check_EMG_9(void)
{
	int flag1, flag2, diff;

	if(myData->dio.misc.delayTimer < myData->dio.config.dioDelay) return;

	switch(myPs->emgPhase) {
		case P0:
			diff = myData->mData.misc.timer_1sec - myPs->emgCheckDelay;
			if(diff < 0) myPs->emgCheckDelay = myData->mData.misc.timer_1sec;
			if(diff >= myData->dio.config.dioDelay / 1000) {
				myPs->emgPhase = P1;
			}
			break;
		case P1:
			flag1 = Read_InPoint(myPs->misc.JigNo+1, 0, I_JIG_SMOKE_SENS);
			flag2 = Read_InPoint(myPs->misc.JigNo+1, 0, I_JIG_FIRE_SENS);
			if(flag1 == ON) {
				myPs->code = J_CD_SMOKE_SENS_1; //smoke
				myPs->jigState = J_EMG;
				myPs->microState = P0;
				myPs->troublePos = 1;
				myPs->emgPhase = P2;
				userlog(DEBUG_LOG, psName,
					"Smoke Sens Detect[%d]\n", myPs->troublePos);
				break;
			} else if(flag2 == ON) {
				myPs->code = J_CD_SMOKE_SENS_2; //fire
				myPs->jigState = J_EMG;
				myPs->microState = P0;
				myPs->troublePos = 2;
				myPs->emgPhase = P2;
				userlog(DEBUG_LOG, psName,
					"Smoke Sens Detect[%d]\n", myPs->troublePos);
				break;
			} else {
				myPs->troublePos = 0;
			}
			break;
		default: break;
	}
}

void Check_EMG_10(void)
{
	int flag1, flag2, diff;

	if(myData->dio.misc.delayTimer < myData->dio.config.dioDelay) return;

	switch(myPs->emgPhase) {
		case P0:
			diff = myData->mData.misc.timer_1sec - myPs->emgCheckDelay;
			if(diff < 0) myPs->emgCheckDelay = myData->mData.misc.timer_1sec;
			if(diff >= myData->dio.config.dioDelay / 1000) {
				myPs->emgPhase = P1;
			}
			break;
		case P1:
			flag1 = Read_InPoint(myPs->misc.JigNo+1, 0, I_JIG_SMOKE_SENS);
			flag2 = Read_InPoint(0, 0, I_JIG_AIR_PRESS);
			if(flag1 == ON) {
				myPs->code = J_CD_SMOKE_SENS_1; //smoke
				myPs->jigState = J_EMG;
				myPs->microState = P0;
				myPs->troublePos = 1;
				myPs->emgPhase = P2;
				userlog(DEBUG_LOG, psName,
					"Smoke Sens Detect[%d]\n", myPs->troublePos);
			} else if(flag2 == ON) {
				myPs->code = J_CD_AIR_PRESS_ERROR;
				myPs->jigState = J_EMG;
				myPs->microState = P0;
				myPs->troublePos = 1;
				myPs->emgPhase = P2;
				userlog(DEBUG_LOG, psName,
					"Air Press Detect[%d]\n", myPs->troublePos);
			} else {
				myPs->troublePos = 0;
			}
			break;
		default: break;
	}
}

void Check_EMG_11(void)
{
	int flag1, diff;

	if(myData->dio.misc.delayTimer < myData->dio.config.dioDelay) return;

	switch(myPs->emgPhase) {
		case P0:
			diff = myData->mData.misc.timer_1sec - myPs->emgCheckDelay;
			if(diff < 0) myPs->emgCheckDelay = myData->mData.misc.timer_1sec;
			if(diff >= myData->dio.config.dioDelay / 1000) {
				myPs->emgPhase = P1;
			}
			break;
		case P1:
			if(myPs->signal[J_SIG_PLC_ALARM_AREA_SENSOR] == P1) {
				myPs->code = J_CD_CHAMBER_AREA_SENSOR;
				myPs->jigState = J_EMG;
				myPs->microState = P0;
				myPs->troublePos = 1;
				myPs->emgPhase = P2;
				userlog(DEBUG_LOG, psName,
					"Chamber Area Sensor[%d]\n", myPs->troublePos);
				break;
			}

			if(myPs->signal[J_SIG_PLC_ALARM_MANUAL_DOOR_OPEN] == P1) {
				myPs->code = J_CD_CHAMBER_MANUAL_DOOR_OPEN;
				myPs->jigState = J_EMG;
				myPs->microState = P0;
				myPs->troublePos = 1;
				myPs->emgPhase = P2;
				userlog(DEBUG_LOG, psName,
					"Chamber Manual Door Open[%d]\n", myPs->troublePos);
				break;
			}

			if(myPs->signal[J_SIG_PLC_ALARM_AIR] == P1) {
				myPs->code = J_CD_CHAMBER_AIR;
				myPs->jigState = J_EMG;
				myPs->microState = P0;
				myPs->troublePos = 1;
				myPs->emgPhase = P2;
				userlog(DEBUG_LOG, psName,
					"Chamber Air[%d]\n", myPs->troublePos);
				break;
			}

			if(myPs->signal[J_SIG_PLC_ALARM_APR_S] == P1) {
				myPs->code = J_CD_CHAMBER_APR_S;
				myPs->jigState = J_EMG;
				myPs->microState = P0;
				myPs->troublePos = 1;
				myPs->emgPhase = P2;
				userlog(DEBUG_LOG, psName,
					"Chamber APR_S[%d]\n", myPs->troublePos);
				break;
			}

			if(myPs->signal[J_SIG_PLC_ALARM_REF] == P1) {
				myPs->code = J_CD_CHAMBER_REF;
				myPs->jigState = J_EMG;
				myPs->microState = P0;
				myPs->troublePos = 1;
				myPs->emgPhase = P2;
				userlog(DEBUG_LOG, psName,
					"Chamber REF[%d]\n", myPs->troublePos);
				break;
			}

			if(myPs->signal[J_SIG_PLC_ALARM_COIL_TRIP] == P1) {
				myPs->code = J_CD_CHAMBER_COIL_TRIP;
				myPs->jigState = J_EMG;
				myPs->microState = P0;
				myPs->troublePos = 1;
				myPs->emgPhase = P2;
				userlog(DEBUG_LOG, psName,
					"Chamber Coil Trip[%d]\n", myPs->troublePos);
				break;
			}

			if(myPs->signal[J_SIG_PLC_ALARM_TEMP] == P1) {
				myPs->code = J_CD_OVER_TEMP_1;
				myPs->jigState = J_EMG;
				myPs->microState = P0;
				myPs->troublePos = 1;
				myPs->emgPhase = P2;
				userlog(DEBUG_LOG, psName,
					"Chamber Over Temp[%d]\n", myPs->troublePos);
				break;
			}

			if(myPs->signal[J_SIG_PLC_ALARM_SMOKE] == P1) {
				myPs->code = J_CD_SMOKE_SENS_1;
				myPs->jigState = J_EMG;
				myPs->microState = P0;
				myPs->troublePos = 1;
				myPs->emgPhase = P2;
				userlog(DEBUG_LOG, psName,
					"Chamber Smoke[%d]\n", myPs->troublePos);
				break;
			}

			if(myPs->signal[J_SIG_PLC_ALARM_FIRE] == P1) {
				myPs->code = J_CD_CHAMBER_FIRE;
				myPs->jigState = J_EMG;
				myPs->microState = P0;
				myPs->troublePos = 1;
				myPs->emgPhase = P2;
				userlog(DEBUG_LOG, psName,
					"Chamber Fire[%d]\n", myPs->troublePos);
				break;
			}

			if(myPs->signal[J_SIG_PLC_ALARM_EMG] == P1) {
				myPs->code = J_CD_EMG_SWITCH;
				myPs->jigState = J_EMG;
				myPs->microState = P0;
				myPs->troublePos = 1;
				myPs->emgPhase = P2;
				userlog(DEBUG_LOG, psName,
					"Chamber Emg. Switch[%d]\n", myPs->troublePos);
				break;
			}

			flag1 = Read_InPoint(myPs->misc.JigNo+1, 0, I_CHAMBER_FAULT);
			if(flag1 == ON) {
				myPs->code = J_CD_CHAMBER_FAULT;
				myPs->jigState = J_EMG;
				myPs->microState = P0;
				myPs->troublePos = 1;
				myPs->emgPhase = P2;
				userlog(DEBUG_LOG, psName,
					"Chamber Fault Detect[%d]\n", myPs->troublePos);
				break;
			}

			myPs->troublePos = 0;
			break;
		default: break;
	}
}
