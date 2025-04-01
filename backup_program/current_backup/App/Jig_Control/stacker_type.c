#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../../INC/datastore.h"
#include "userlog.h"
#include "common_utils.h"
#include "local_utils.h"
#include "local_message.h"
#include "stacker_type.h"

extern volatile S_SYSTEM_DATA	*myData;
extern volatile S_JIG_DATA		*myPs;
extern char psName[PROCESS_NAME_SIZE];

void Check_StackerCrane(void)
{
	if(myPs->config.jigUseFlag == UNUSE) return;

	Check_StackerCrane_Input();
	Output_StackerCrane_Signal();
}

void Check_StackerCrane_Input(void)
{
	switch(myPs->config.jigType) {
		case 1:	Check_StackerCrane_Input_1();	break;
		case 2: Check_StackerCrane_Input_2();	break;
		default:	break;
	}
}

void Output_StackerCrane_Signal(void)
{
	switch(myPs->config.jigType) {
		case 1: Output_StackerCrane_Signal_1(); break;
		case 2: Output_StackerCrane_Signal_2(); break;
		default:	break;
	}
}

void Check_StackerCrane_Input_1(void)
{	
	int loadingEnd, jigNotMove;
		
	loadingEnd = Read_InPoint(myPs->misc.JigNo, 1, I_JIG_STACKER_STATE);
	jigNotMove = Read_InPoint(myPs->misc.JigNo, 2, I_JIG_STACKER_STATE);
	
	if(loadingEnd == ON) {
		if(myPs->loadingEnd != ON){
			userlog(DEBUG_LOG, psName, "STK Load End\n");
		}
		myPs->loadingEnd = ON;
	}

	if(jigNotMove == ON) {
		if(myPs->jigNotMove != ON){
			userlog(DEBUG_LOG, psName, "STK Not Move On\n");
		}
		myPs->jigNotMove = ON;
		myPs->jigNotMoveTimer++;
	} else {
		if(myPs->jigNotMove != OFF){
			userlog(DEBUG_LOG, psName, "STK Not Move Off\n");
			myPs->loadingEnd_waitTime = myData->mData.misc.timer_1sec;
		}
		myPs->jigNotMove = OFF;
	}
}

void Output_StackerCrane_Signal_1(void)
{
	int trayOut=OFF, trayIn=OFF;

	if(myPs->trayState[0] == T_LOAD) {
		switch(myPs->jigState) {
			case J_CONTACT:	
				trayOut = OFF; 
				trayIn = OFF;
				break;
			case J_DISCONTACT:
				trayOut = ON;
				trayIn = OFF;
				break;	
			case J_FAIL:
			case J_IDLE:
			case J_TO_CONTACT:
			case J_TO_DISCONTACT:
			case J_TRAYCHECK:
				trayOut = OFF;
				trayIn = OFF;
				break;
			default: break;
		}	 
	} else if(myPs->trayState[0] == T_UNLOAD) {
		switch(myPs->jigState) {
			case J_CONTACT:	
				trayOut = OFF; 
				trayIn = OFF;
				break;
			case J_DISCONTACT:
				trayOut = OFF;
				trayIn = ON;
				break;	
			case J_FAIL:
			case J_IDLE:
			case J_TO_CONTACT:
			case J_TO_DISCONTACT:
			case J_TRAYCHECK:
				trayOut = OFF;
				trayIn = OFF;
				break;
			default: break;
		}	 
	} else if(myPs->trayState[0] == T_CHECK) {
		switch(myPs->jigState) {
			default: 
				trayOut = OFF;
				trayIn = OFF;
				break;
		}
	} else { // T_MOVING
		switch(myPs->jigState) {
			case J_CONTACT:	
				trayOut = OFF; 
				trayIn = OFF;
				break;
			case J_DISCONTACT:
				trayOut = ON;
				trayIn = ON;
				break;	
			case J_FAIL:
			case J_IDLE:
			case J_TO_CONTACT:
			case J_TO_DISCONTACT:
			case J_TRAYCHECK:
				trayOut = OFF;
				trayIn = OFF;
				break;
			default: break;
		}	 
	
	}	

	if(myPs->trayState[0] != myPs->trayStateCheck[0]) {
		myPs->trayStateCheck[0] = myPs->trayState[0];
		userlog(DEBUG_LOG, psName,
		"trayState[%x], trayOut[%x], trayIn[%x]\n"
		,myPs->trayState[0], trayOut, trayIn);
	}

	Select_OutPoint(myPs->misc.JigNo, 1, O_JIG_STACKER_SIGNAL, trayOut);
	Select_OutPoint(myPs->misc.JigNo, 2, O_JIG_STACKER_SIGNAL, trayIn);
}

void Check_StackerCrane_Input_2(void)
{
	int loadingEnd, jigNotMove;
	
	loadingEnd = Read_InPoint(myPs->misc.JigNo, 1, I_JIG_STACKER_STATE);
	jigNotMove = Read_InPoint(myPs->misc.JigNo, 2, I_JIG_STACKER_STATE);

	if(loadingEnd == ON) {
		if(myPs->loadingEnd != ON){
			userlog(DEBUG_LOG, psName, "STK Load End\n");
		}
		myPs->loadingEnd = ON;
	}

	if(jigNotMove == ON) {
		if(myPs->jigNotMove != ON){
			userlog(DEBUG_LOG, psName, "STK Not Move On\n");
		}
		myPs->jigNotMove = ON;
		myPs->jigNotMoveTimer++;
	} else {
		if(myPs->jigNotMove != OFF){
			userlog(DEBUG_LOG, psName, "STK Not Move Off\n");
			myPs->loadingEnd_waitTime = myData->mData.misc.timer_1sec;
		}
		myPs->jigNotMove = OFF;
	}
}

void Output_StackerCrane_Signal_2(void)
{
	int trayOut=OFF, trayIn=OFF;

	if(myPs->trayState[0] == T_LOAD) {
		switch(myPs->jigState) {
			case J_CONTACT:	
				trayOut = OFF;
				trayIn = OFF;
				break;
			case J_DISCONTACT:
				trayOut = ON;
				trayIn = OFF;
				break;	
			case J_FAIL:
			case J_IDLE:
			case J_TO_CONTACT:
			case J_TO_DISCONTACT:
			case J_TRAYCHECK:
				trayOut = OFF;
				trayIn = OFF;
				break;
			default: break;
		}	 
	} else if(myPs->trayState[0] == T_UNLOAD) {
		switch(myPs->jigState) {
			case J_CONTACT:	
				trayOut = OFF; 
				trayIn = OFF;
				break;
			case J_DISCONTACT:
				trayOut = OFF;
				trayIn = ON;
				break;	
			case J_FAIL:
			case J_IDLE:
			case J_TO_CONTACT:
			case J_TO_DISCONTACT:
			case J_TRAYCHECK:
				trayOut = OFF;
				trayIn = OFF;
				break;
			default: break;
		}	 
	} else if(myPs->trayState[0] == T_CHECK) {
		switch(myPs->jigState){
			default: 
				trayOut = OFF;
				trayIn = OFF;
				break;
		}
	} else { // T_MOVING
		switch(myPs->jigState) {
			case J_CONTACT:	
				trayOut = OFF; 
				trayIn = OFF;
				break;
			case J_DISCONTACT:
				trayOut = ON;
				trayIn = ON;
				break;
			case J_FAIL:
			case J_IDLE:
			case J_TO_CONTACT:
			case J_TO_DISCONTACT:
			case J_TRAYCHECK:
				trayOut = OFF;
				trayIn = OFF;
				break;
			default: break;
		}	 
	
	}	
	if(myPs->trayState[0] != myPs->trayStateCheck[0]) {
		myPs->trayStateCheck[0] = myPs->trayState[0];
		userlog(DEBUG_LOG, psName,
		"trayState[%x], trayOut[%x], trayIn[%x]\n"
		,myPs->trayState[0], trayOut, trayIn);
	}

	Select_OutPoint(myPs->misc.JigNo, 1, O_JIG_STACKER_SIGNAL, trayOut);
	Select_OutPoint(myPs->misc.JigNo, 2, O_JIG_STACKER_SIGNAL, trayIn);
}
