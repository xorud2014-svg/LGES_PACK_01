#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../../INC/datastore.h"
#include "userlog.h"
#include "common_utils.h"
#include "local_utils.h"
#include "local_message.h"
#include "common_message.h"
#include "tray_type.h"

extern volatile S_SYSTEM_DATA	*myData;
extern volatile S_JIG_DATA		*myPs;
extern char psName[PROCESS_NAME_SIZE];

void Check_Tray(void)
{
	int i;
	if(myPs->config.jigUseFlag == UNUSE) {
		for(i=0; i < MAX_TRAY_PER_GROUP; i++) myPs->trayState[i] = T_LOAD;
		return;
	}

	switch(myPs->config.jigType) {
		case 1: Check_Tray_1(); break;
		case 2: Check_Tray_2(); break;
		case 3: Check_Tray_3(); break;
		case 4: Check_Tray_4(); break;
		case 5: Check_Tray_5(); break;
		case 6: Check_Tray_6(); break;
		case 7: Check_Tray_7(); break;
		case 8: Check_Tray_8(); break;
		case 9: Check_Tray_9(); break;
		case 10: Check_Tray_10(); break;
		case 11: Check_Tray_11(); break;
		default:	break;
	}
}

void Check_Tray_1(void)
{
	int flag1, flag2, diff;

	flag1 = Read_InPoint(myPs->misc.JigNo, 1, I_JIG_TRAY_STATE);
	flag2 = Read_InPoint(myPs->misc.JigNo, 2, I_JIG_TRAY_STATE);

	if(myPs->curTrayFlag 
		!= myPs->preTrayFlag){
		myPs->preTrayFlag = myPs->preTrayFlag;
		myPs->traySensingDelay 
			= myData->mData.misc.timer_1sec;
	}

	if(flag1 == ON && flag2 == OFF) {
		myPs->curTrayFlag = T_LOAD;
		if(myPs->tmpTrayState[0] != T_LOAD) {
			diff = myData->mData.misc.timer_1sec
					-myPs->traySensingDelay;
			if(diff >= 4){//4sec
				myPs->tmpTrayState[0] = T_LOAD;
				myPs->trayState[0] = T_LOAD;
				myPs->signal[J_SIG_TRY_TRAY_CHECK_COMPLETE] = P0;
			}
		} else {
			myPs->traySensingDelay 
				= myData->mData.misc.timer_1sec;
		}
	} else if(flag1 == OFF && flag2 == OFF) {
		myPs->curTrayFlag = T_UNLOAD;
		if(myPs->tmpTrayState[0] != T_UNLOAD) {
			diff = myData->mData.misc.timer_1sec
					-myPs->traySensingDelay;
			if(diff >= 4){//4sec
				myPs->tmpTrayState[0] = T_UNLOAD;
				myPs->trayState[0] = T_UNLOAD;
				myPs->signal[J_SIG_TRY_TRAY_CHECK_COMPLETE] = P0;
			}
		} else {
			myPs->traySensingDelay 
				= myData->mData.misc.timer_1sec;
		}
	} else { 
		if(myPs->signal[J_SIG_TRY_TRAY_CHECK_COMPLETE] == P0){
			myPs->curTrayFlag = T_MOVING;
			if(myPs->tmpTrayState[0] != T_MOVING
				&& myPs->tmpTrayState[0] != T_CHECK) {
				diff = myData->mData.misc.timer_1sec
						-myPs->traySensingDelay;
				if(diff >= 15){
					if(myPs->tmpTrayState[0] != T_CHECK){
						myPs->curTrayFlag = T_CHECK;
						myPs->tmpTrayState[0] = T_CHECK;
						myPs->trayState[0] = T_CHECK;
						myPs->signal[J_SIG_TRY_TRAY_CHECK] = P1;
					}
				}
			}else{
				myPs->traySensingDelay 
					= myData->mData.misc.timer_1sec;
			}
		}else if(myPs->signal[J_SIG_TRY_TRAY_CHECK_COMPLETE] == P1){
			diff = myData->mData.misc.timer_1sec
					-myPs->traySensingDelay;
			if(diff >= 5){//5sec
				myPs->code = J_CD_TRAY_DIR_ERROR;
				myPs->jigState = J_FAIL;
				myPs->microState = P0;
				myPs->tmpTrayState[0] = T_MOVING;
				myPs->trayState[0] = T_MOVING;
				myPs->signal[J_SIG_TRY_TRAY_CHECK_COMPLETE] = P0;
			}
		}
	}
}

void Check_Tray_2(void)
{
	int flag1, flag2, diff;

	flag1 = Read_InPoint(myPs->misc.JigNo, 1, I_JIG_TRAY_STATE);
	flag2 = Read_InPoint(myPs->misc.JigNo, 2, I_JIG_TRAY_STATE);

	if(myPs->curTrayFlag 
		!= myPs->preTrayFlag){
		myPs->preTrayFlag = myPs->curTrayFlag;
		myPs->traySensingDelay 
			= myData->mData.misc.timer_1sec;
	}

	if(flag1 == ON && flag2 == OFF) {
		myPs->curTrayFlag = T_LOAD;
		if(myPs->tmpTrayState[0] != T_LOAD) {
			diff = myData->mData.misc.timer_1sec
					-myPs->traySensingDelay;
			if(diff >= 4){//4sec
				myPs->tmpTrayState[0] = T_LOAD;
				myPs->trayState[0] = T_LOAD;
				myPs->signal[J_SIG_TRY_TRAY_CHECK_COMPLETE] = P0;
			}
		} else {
			myPs->traySensingDelay 
				= myData->mData.misc.timer_1sec;
		}
	} else if(flag1 == OFF && flag2 == OFF) {
		myPs->curTrayFlag = T_UNLOAD;
		if(myPs->tmpTrayState[0] != T_UNLOAD) {
			diff = myData->mData.misc.timer_1sec
					-myPs->traySensingDelay;
			if(diff >= 4){//4sec
				myPs->tmpTrayState[0] = T_UNLOAD;
				myPs->trayState[0] = T_UNLOAD;
				myPs->signal[J_SIG_TRY_TRAY_CHECK_COMPLETE] = P0;
			}
		} else {
			myPs->traySensingDelay 
				= myData->mData.misc.timer_1sec;
		}
	} else { 
		if(myPs->signal[J_SIG_TRY_TRAY_CHECK_COMPLETE] == P0){
			myPs->curTrayFlag = T_MOVING;
			if(myPs->tmpTrayState[0] != T_MOVING
				&& myPs->tmpTrayState[0] != T_CHECK) {
				diff = myData->mData.misc.timer_1sec
						-myPs->traySensingDelay;
				if(diff >= 15){
					if(myPs->tmpTrayState[0] != T_CHECK){
						myPs->curTrayFlag = T_CHECK;
						myPs->tmpTrayState[0] = T_CHECK;
						myPs->trayState[0] = T_CHECK;
						myPs->signal[J_SIG_TRY_TRAY_CHECK] = P1;
						myPs->trayCheckDelay 
							= myData->mData.misc.timer_1sec;
					}
				}
			}else{
				myPs->traySensingDelay 
					= myData->mData.misc.timer_1sec;
			}
		}else if(myPs->signal[J_SIG_TRY_TRAY_CHECK_COMPLETE] == P1){
			diff = myData->mData.misc.timer_1sec
					-myPs->traySensingDelay;
			if(diff >= 5){//5sec
				myPs->code = J_CD_TRAY_DIR_ERROR;
				myPs->jigState = J_FAIL;
				myPs->microState = P0;
				myPs->tmpTrayState[0] = T_MOVING;
				myPs->trayState[0] = T_MOVING;
				myPs->signal[J_SIG_TRY_TRAY_CHECK_COMPLETE] = P0;
			}
		}
	}
}

void Check_Tray_3(void)
{
	int flag1, flag2, flag3, flag4;

	flag1 = Read_InPoint(myPs->misc.JigNo, 1, I_JIG_TRAY_STATE);
	flag2 = Read_InPoint(myPs->misc.JigNo, 2, I_JIG_TRAY_STATE);
	flag3 = Read_InPoint(myPs->misc.JigNo, 3, I_JIG_TRAY_STATE);
	flag4 = Read_InPoint(myPs->misc.JigNo, 4, I_JIG_TRAY_STATE);

	if(flag1 == P1 || flag2 == P1 || flag3 == P1 || flag4 == P1) {
		myPs->trayState[0] = T_LOAD;
	} else {
		myPs->trayState[0] = T_UNLOAD;
	}
}

void Check_Tray_4(void)
{
	int flag, tray, io_no;

	for(tray = 0; tray < myPs->config.totalTrayNo; tray++){
		flag = Read_InPoint(myPs->misc.JigNo, tray+1, I_JIG_TRAY_STATE);

		io_no = J4_OUT_BCR_LAMP_1 + tray;	
		if(flag == P1) {
			myPs->trayState[tray] = T_LOAD;
			Tray_Load_Lamp(flag, tray, io_no, ON);
		} else {
			myPs->trayState[tray] = T_UNLOAD;
			Tray_Load_Lamp(flag, tray, io_no, OFF);
		}
	}
}

void Check_Tray_5(void)
{
	int flag1, flag2, tray;

	for(tray = 0; tray < myPs->config.totalTrayNo; tray++){
		flag1 = Read_InPoint(myPs->misc.JigNo, 1, I_JIG_TRAY_STATE);
		flag2 = Read_InPoint(myPs->misc.JigNo, 2, I_JIG_TRAY_STATE);

		if((flag1 == P1 && flag2 == P0) 
			||(flag1 == P0 && flag2 == P1)){
			myPs->trayState[tray] = T_LOAD;
		} else {
			myPs->trayState[tray] = T_UNLOAD;
		}
	}
}

void Check_Tray_6(void)
{
	int flag, tray, io_no;

	for(tray = 0; tray < myPs->config.totalTrayNo; tray++){
		flag = Read_InPoint(myPs->misc.JigNo, tray+1, I_JIG_TRAY_STATE);

		io_no = J6_OUT_TRAY_LAMP_1 + tray;	
		if(flag == P1){
			myPs->trayState[tray] = T_LOAD;
			Tray_Load_Lamp(flag, tray, io_no, ON);
		}else{
		   	myPs->trayState[tray] = T_UNLOAD;
			Tray_Load_Lamp(flag, tray, io_no, OFF);
		}
	}
}

void Check_Tray_7(void)
{
	int flag, tray;

	for(tray = 0; tray < myPs->config.totalTrayNo; tray++){
		flag = Read_InPoint(myPs->misc.JigNo, tray+1, I_JIG_TRAY_STATE);

		if(flag == P1){
			myPs->trayState[tray] = T_LOAD;
		}else{
		   	myPs->trayState[tray] = T_UNLOAD;
		}
	}
}

void Check_Tray_8(void)
{
	int flag1, flag2, tray;

	for(tray = 0; tray < myPs->config.totalTrayNo; tray++){
		flag1 = Read_InPoint(myPs->misc.JigNo, tray*2+1, I_JIG_TRAY_STATE);
		flag2 = Read_InPoint(myPs->misc.JigNo, tray*2+2, I_JIG_TRAY_STATE);

		if(flag1 == P0 && flag2 == P0) {
			myPs->trayState[tray] = T_UNLOAD;
		} else {
			myPs->trayState[tray] = T_LOAD;
		}
	}
}

void Check_Tray_9(void)
{ //kjgw_f
	int flag1, flag2;
	long diff;
	S_MSG_VAL SendMsg;

	if(myData->dio.misc.delayTimer < myData->dio.config.dioDelay) return;

	flag1 = Read_InPoint(myPs->misc.JigNo+1, 1, I_JIG_TRAY_STATE);
	flag2 = Read_InPoint(myPs->misc.JigNo+1, 2, I_JIG_TRAY_STATE);

//	if(myPs->curTrayFlag != myPs->preTrayFlag) {
//		myPs->preTrayFlag = myPs->preTrayFlag;
//		myPs->traySensingDelay = myData->mData.misc.timer_1sec;
//	}

	if(flag1 == ON && flag2 == OFF) {
		myPs->curTrayFlag = T_LOAD;
		if(myPs->tmpTrayState[0] != T_LOAD) {
			diff = myData->mData.misc.timer_1sec - myPs->traySensingDelay;
			if(diff >= 2) { //2sec
				myPs->tmpTrayState[0] = T_LOAD;
				myPs->trayState[0] = T_LOAD;
				myPs->signal[J_SIG_TRY_TRAY_CHECK_COMPLETE] = P0;

				memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
				SendMsg.msg = MSG_JIGC_JIGM_READ_BCR;
				SendMsg.val[0] = jigNo_to_groupNo(myPs->misc.JigNo);
				send_msg(JIGC1_TO_JIGM + myPs->misc.JigNo, (char *)&SendMsg);
				//userlog(DEBUG_LOG, psName, "Read BCR!!!\n");
			}
		} else {
			myPs->traySensingDelay = myData->mData.misc.timer_1sec;
		}
	} else if(flag1 == OFF && flag2 == OFF) {
		myPs->curTrayFlag = T_UNLOAD;
		if(myPs->tmpTrayState[0] != T_UNLOAD) {
			diff = myData->mData.misc.timer_1sec - myPs->traySensingDelay;
			if(diff >= 2){//2sec
				myPs->tmpTrayState[0] = T_UNLOAD;
				myPs->trayState[0] = T_UNLOAD;
				myPs->signal[J_SIG_TRY_TRAY_CHECK_COMPLETE] = P0;
			}
		} else {
			myPs->traySensingDelay = myData->mData.misc.timer_1sec;
		}
	} else { 
		if(myPs->signal[J_SIG_TRY_TRAY_CHECK_COMPLETE] == P0){
			myPs->curTrayFlag = T_MOVING;
			if(myPs->tmpTrayState[0] != T_MOVING
				&& myPs->tmpTrayState[0] != T_CHECK) {
				diff = myData->mData.misc.timer_1sec - myPs->traySensingDelay;
				if(diff >= 15){
					if(myPs->tmpTrayState[0] != T_CHECK){
						myPs->curTrayFlag = T_CHECK;
						myPs->tmpTrayState[0] = T_CHECK;
						myPs->trayState[0] = T_CHECK;
						myPs->signal[J_SIG_TRY_TRAY_CHECK] = P1;
					}
				}
			}else{
				myPs->traySensingDelay = myData->mData.misc.timer_1sec;
			}
		}else if(myPs->signal[J_SIG_TRY_TRAY_CHECK_COMPLETE] == P1){
			diff = myData->mData.misc.timer_1sec - myPs->traySensingDelay;
			if(diff >= 3){//3sec
				myPs->code = J_CD_TRAY_DIR_ERROR;
				myPs->jigState = J_FAIL;
				myPs->microState = P0;
				myPs->tmpTrayState[0] = T_MOVING;
				myPs->trayState[0] = T_MOVING;
				myPs->signal[J_SIG_TRY_TRAY_CHECK_COMPLETE] = P0;
			}
		}
	}
}

void Check_Tray_10(void)
{
	int flag1;
	long diff;

	if(myData->dio.misc.delayTimer < myData->dio.config.dioDelay) return;

	flag1 = Read_InPoint(myPs->misc.JigNo+1, 0, I_JIG_TRAY_STATE);

	if(flag1 == ON) {
		myPs->curTrayFlag = T_LOAD;
		if(myPs->tmpTrayState[0] != T_LOAD) {
			diff = myData->mData.misc.timer_1sec - myPs->traySensingDelay;
			if(diff >= 2) { //2sec
				myPs->tmpTrayState[0] = T_LOAD;
				myPs->trayState[0] = T_LOAD;
				myPs->signal[J_SIG_TRY_TRAY_CHECK_COMPLETE] = P0;
			}
		} else {
			myPs->traySensingDelay = myData->mData.misc.timer_1sec;
		}
	} else if(flag1 == OFF) {
		myPs->curTrayFlag = T_UNLOAD;
		if(myPs->tmpTrayState[0] != T_UNLOAD) {
			diff = myData->mData.misc.timer_1sec - myPs->traySensingDelay;
			if(diff >= 2){//2sec
				myPs->tmpTrayState[0] = T_UNLOAD;
				myPs->trayState[0] = T_UNLOAD;
				myPs->signal[J_SIG_TRY_TRAY_CHECK_COMPLETE] = P0;
			}
		} else {
			myPs->traySensingDelay = myData->mData.misc.timer_1sec;
		}
	}
}

void Check_Tray_11(void)
{
	int flag1, flag2, flag3;
	long diff;

	if(myData->dio.misc.delayTimer < myData->dio.config.dioDelay) return;
	if(myPs->jigState != J_DISCONTACT) return;

	flag1 = Read_InPoint(myPs->misc.JigNo+1, 1, I_JIG_TRAY_STATE); //Hor
	flag2 = Read_InPoint(myPs->misc.JigNo+1, 2, I_JIG_TRAY_STATE); // Front
	flag3 = Read_InPoint(myPs->misc.JigNo+1, 3, I_JIG_TRAY_STATE); // Rear
/*
	if(flag1 == ON) {
		myPs->trayState[0] = T_LOAD;
		myPs->signal[J_SIG_TRY_TRAY_CHECK_COMPLETE] = P0;
	} else {
		myPs->trayState[0] = T_UNLOAD;
		myPs->signal[J_SIG_TRY_TRAY_CHECK_COMPLETE] = P0;
	}
*/
	if(flag1 == OFF && flag2 == OFF && flag3 == OFF) {
		myPs->curTrayFlag = T_UNLOAD;
		if(myPs->tmpTrayState[0] != T_UNLOAD) {
			diff = myData->mData.misc.timer_1sec - myPs->traySensingDelay;
			if(diff >= 2) {//2sec
				myPs->tmpTrayState[0] = T_UNLOAD;
				myPs->trayState[0] = T_UNLOAD;
				myPs->signal[J_SIG_TRY_TRAY_CHECK_COMPLETE] = P0;
			}
		} else {
			myPs->traySensingDelay = myData->mData.misc.timer_1sec;
		}
	} else if(flag1 == OFF && flag2 == OFF && flag3 == ON) {
		myPs->curTrayFlag = T_LOAD;
		if(myPs->tmpTrayState[0] != T_LOAD) {
			diff = myData->mData.misc.timer_1sec - myPs->traySensingDelay;
			if(diff >= 2) { //2sec
				myPs->tmpTrayState[0] = T_LOAD;
				myPs->trayState[0] = T_LOAD;
				myPs->signal[J_SIG_TRY_TRAY_CHECK_COMPLETE] = P0;
			}
		} else {
			myPs->traySensingDelay = myData->mData.misc.timer_1sec;
		}
	} else if(flag1 == ON && flag2 == OFF && flag3 == ON) {
		if(myPs->tmpTrayState[0] == T_LOAD) {
		} else {
			myPs->curTrayFlag = T_MOVING;
			if(myPs->tmpTrayState[0] != T_MOVING) {
				diff = myData->mData.misc.timer_1sec - myPs->traySensingDelay;
				if(diff >= 2) {//2sec
					myPs->tmpTrayState[0] = T_MOVING;
					myPs->trayState[0] = T_MOVING;
					myPs->signal[J_SIG_TRY_TRAY_CHECK_COMPLETE] = P0;
					myPs->traySensingDelay = myData->mData.misc.timer_1sec;
				}
			} else {
				diff = myData->mData.misc.timer_1sec - myPs->traySensingDelay;
				if(diff == 30) {//30sec
					myPs->jigState = J_FAIL;
					myPs->microState = P0;
					myPs->code = J_CD_TRAY_UNLOAD;
				}
			}
		}
	} else {
		myPs->curTrayFlag = T_MOVING;
		if(myPs->tmpTrayState[0] != T_MOVING) {
			diff = myData->mData.misc.timer_1sec - myPs->traySensingDelay;
			if(diff >= 2) {//2sec
				myPs->tmpTrayState[0] = T_MOVING;
				myPs->trayState[0] = T_MOVING;
				myPs->signal[J_SIG_TRY_TRAY_CHECK_COMPLETE] = P0;
				myPs->traySensingDelay = myData->mData.misc.timer_1sec;
			}
		} else {
			diff = myData->mData.misc.timer_1sec - myPs->traySensingDelay;
			if(diff == 30) {//30sec
				myPs->jigState = J_FAIL;
				myPs->microState = P0;
				myPs->code = J_CD_TRAY_UNLOAD;
			}
		}
	}
}


void Tray_Load_Lamp(unsigned char flag, int tray, int j_io_no, int OnOff)
{
	if(myPs->trayPreLoadState[tray] != OnOff){
		Select_OutPoint(myPs->misc.JigNo, j_io_no, O_JIG_TRAY_STATE, OnOff);
		myPs->trayPreLoadState[tray] = flag;
		printf("tray load  : %d\n\n", j_io_no);
	}
}
