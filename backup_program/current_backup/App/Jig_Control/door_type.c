#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../../INC/datastore.h"
#include "userlog.h"
#include "common_utils.h"
#include "local_utils.h"
#include "door_type.h"

extern volatile S_SYSTEM_DATA	*myData;
extern volatile S_JIG_DATA		*myPs;
extern char psName[PROCESS_NAME_SIZE];

void Check_Door(void)
{
	int i;

	if(myPs->config.jigUseFlag == UNUSE) {
		for(i=0; i < MAX_TRAY_PER_GROUP; i++) {
			myPs->doorState[i] = D_CLOSE;
		}
		return;
	}

	switch(myPs->config.jigType) {
		case 6:
		case 8:
			Check_Door_6_8();
			break;
		case 9:
			Check_Door_9();
			break;
		case 11:
			Check_Door_11();
			break;
		default:
			Check_Door_1(); break;
	}
}

void Check_Door_1(void)
{
	myPs->doorState[0] = D_CLOSE;
}

void Check_Door_6_8(void)
{
	myPs->doorState[0] = D_CLOSE;
	myPs->doorState[1] = D_CLOSE;
}

void Check_Door_9(void)
{
	int flag1;

	if(myData->dio.misc.delayTimer < myData->dio.config.dioDelay) return;

	flag1 = Read_InPoint(myPs->misc.JigNo+1, 0, I_CHAMBER_FAULT);
	if(flag1 == ON) {
		myPs->doorState[0] = D_OPEN;
	} else {
		myPs->doorState[0] = D_CLOSE;
	}
}

void Check_Door_11(void)
{
	int flag1;

	if(myData->dio.misc.delayTimer < myData->dio.config.dioDelay) return;

	//flag1 = Read_InPoint(myPs->misc.JigNo+1, 0, I_CHAMBER_FAULT);
	flag1 = myPs->signal[J_SIG_DOOR_OPEN];
	if(flag1 == ON) {
		myPs->doorState[0] = D_OPEN;
	} else {
		myPs->doorState[0] = D_CLOSE;
	}
}

