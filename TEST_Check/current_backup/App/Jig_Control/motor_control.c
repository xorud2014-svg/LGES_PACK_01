#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../../INC/datastore.h"
#include "userlog.h"
#include "common_utils.h"
#include "local_utils.h"
#include "motor_control.h"

extern volatile S_SYSTEM_DATA	*myData;
extern volatile S_JIG_DATA		*myPs;
extern char psName[PROCESS_NAME_SIZE];

void Check_Motor_Control(void)
{
	switch(myPs->signal[J_SIG_PITCH_CHANGE]) {
		case PITCH_CHANGE_1:
			motorControl_1();
			break;
		case PITCH_CHANGE_2:
			motorControl_2();
			break;
		case PITCH_CHANGE_3:
			motorControl_3();
			break;
		case PITCH_CHANGE_4:
			motorControl_4();
			break;
		case PITCH_CHANGE_5:
			motorControl_5();
			break;
		case PITCH_CHANGE_6:
			motorControl_6();
			break;
		case PITCH_CHANGE_7:
			motorControl_7();
			break;
		case PITCH_CHANGE_8:
			motorControl_8();
			break;
	}
}

void motorControl_1(void)
{
	int flag1, flag2;
	
	if(myPs->config.jigUseFlag == UNUSE) {
		myPs->pitchChangeEnd = PITCH_CHANGE_END;
		return;
	}

	if(myData->dio.misc.delayTimer < myData->dio.config.dioDelay) return;
	if(myPs->jigNotMove == ON) return;
	if(myPs->pitchChangeEnd != PITCH_CHANGE_START) return;

	switch(myPs->pitchChangePhase) {
		case P0:
			flag1 = Read_InPoint(myPs->misc.JigNo+1, 0, I_MOTOR_ERROR);
			if(flag1 == P0) {
				myPs->misc.pitchChangeMoveTimer 
					= myData->mData.misc.timer_1sec;
				myPs->pitchChangePhase = P5;
			}else{
				Select_OutPoint(myPs->misc.JigNo+1, 0, O_MOTOR_RESET, ON);
				myPs->pitchChangePhase = P1;
			}
			break;
		case P1:
			Select_OutPoint(myPs->misc.JigNo+1, 0, O_MOTOR_RESET, OFF);
			myPs->misc.pitchChangeMoveTimer 
				= myData->mData.misc.timer_1sec;
			myPs->pitchChangePhase = P2;
			break;
		case P2:
			flag1 = Read_InPoint(myPs->misc.JigNo+1, 0, I_MOTOR_ERROR);
			flag1 = P0;
			if(flag1 == P0) {
				myPs->pitchChangePhase = P5;
			}else{
				if((myData->mData.misc.timer_1sec - myPs->misc.pitchChangeMoveTimer)
					> myPs->config.cylMoveTimeout/1000) {
					myPs->code = J_CD_PITCH_CHANGE_ERROR;
					myPs->jigState = J_FAIL;
					myPs->microState = P0;
				}
			}
			break;
		case P5:
			Select_OutPoint(myPs->misc.JigNo+1, 0, O_MOTOR_HOME, ON);
			myPs->misc.pitchChangeMoveTimer 
				= myData->mData.misc.timer_1sec;
			myPs->pitchChangePhase = P6;
			break;
		case P6:
			flag1 = Read_InPoint(myPs->misc.JigNo+1, 0, I_MOTOR_HOME);
			flag2 = Read_InPoint(myPs->misc.JigNo+1, 0, I_MOTOR_ERROR);
			if(flag1 == P0 && flag2 ==P0) {
				myPs->pitchChangePhase = P7;
			}else{
				if((myData->mData.misc.timer_1sec - myPs->misc.pitchChangeMoveTimer)
					> 30) {
//					> myPs->config.cylMoveTimeout/1000) {
					Select_OutPoint(myPs->misc.JigNo+1, 0, O_MOTOR_HOME, OFF);
					myPs->code = J_CD_PITCH_CHANGE_ERROR;
					myPs->jigState = J_FAIL;
					myPs->microState = P0;
				}
			}
			break;
		case P7:
			Select_OutPoint(myPs->misc.JigNo+1, 0, O_MOTOR_RUN, ON);
			Select_OutPoint(myPs->misc.JigNo+1, 1, O_MOTOR_STEP, OFF);
			Select_OutPoint(myPs->misc.JigNo+1, 2, O_MOTOR_STEP, OFF);
			Select_OutPoint(myPs->misc.JigNo+1, 3, O_MOTOR_STEP, OFF);
			Select_OutPoint(myPs->misc.JigNo+1, 4, O_MOTOR_STEP, OFF);
			myPs->misc.pitchChangeMoveTimer = myData->mData.misc.timer_1sec;
			myPs->pitchChangePhase = P8;
			break;
		case P8:
			flag1 = Read_InPoint(myPs->misc.JigNo+1, 0, I_MOTOR_RUN);
			flag2 = Read_InPoint(myPs->misc.JigNo+1, 0, I_MOTOR_ERROR);
			if(flag1 == P0 && flag2 == P0) {
				if((myData->mData.misc.timer_1sec - myPs->misc.jobChangeMoveTimer)
					> myPs->config.cylMoveDelay/1000) {
					Select_OutPoint(myPs->misc.JigNo+1, 0, O_MOTOR_RUN, OFF);
					myPs->pitchChangePhase = P9;
				}
			} else {
				if((myData->mData.misc.timer_1sec - myPs->misc.jobChangeMoveTimer)
					> 30) {
//					> myPs->config.cylMoveTimeout/1000) {
					Select_OutPoint(myPs->misc.JigNo+1, 0, O_MOTOR_RUN, OFF);
					myPs->code = J_CD_PITCH_CHANGE_ERROR;
					myPs->jigState = J_FAIL;
					myPs->microState = P0;
				}
			}
			break;
		case P9:
//			flag1 = Read_InPoint(myPs->misc.JigNo+1, 0, I_MOTOR_READY);
//			if(flag1 == P1 ) {
				myPs->pitchChangePhase = P0;
				myPs->pitchChangeEnd = PITCH_CHANGE_END;
//			}
			break;
	}
}

void motorControl_2(void)
{
	int flag1, flag2;
	
	if(myPs->config.jigUseFlag == UNUSE) {
		myPs->pitchChangeEnd = PITCH_CHANGE_END;
		return;
	}

	if(myData->dio.misc.delayTimer < myData->dio.config.dioDelay) return;
	if(myPs->jigNotMove == ON) return;
	if(myPs->pitchChangeEnd != PITCH_CHANGE_START) return;

	switch(myPs->pitchChangePhase) {
		case P0:
			flag1 = Read_InPoint(myPs->misc.JigNo+1, 0, I_MOTOR_ERROR);
			if(flag1 == P0) {
				myPs->misc.pitchChangeMoveTimer 
					= myData->mData.misc.timer_1sec;
				myPs->pitchChangePhase = P5;
			}else{
				Select_OutPoint(myPs->misc.JigNo+1, 0, O_MOTOR_RESET, ON);
				myPs->pitchChangePhase = P1;
			}
			break;
		case P1:
			Select_OutPoint(myPs->misc.JigNo+1, 0, O_MOTOR_RESET, OFF);
			myPs->misc.pitchChangeMoveTimer 
				= myData->mData.misc.timer_1sec;
			myPs->pitchChangePhase = P2;
			break;
		case P2:
			flag1 = Read_InPoint(myPs->misc.JigNo+1, 0, I_MOTOR_ERROR);
			if(flag1 == P0) {
				myPs->pitchChangePhase = P5;
			}else{
				if((myData->mData.misc.timer_1sec - myPs->misc.pitchChangeMoveTimer)
					> myPs->config.cylMoveTimeout/1000) {
					myPs->code = J_CD_PITCH_CHANGE_ERROR;
					myPs->jigState = J_FAIL;
					myPs->microState = P0;
				}
			}
			break;
		case P5:
			Select_OutPoint(myPs->misc.JigNo+1, 0, O_MOTOR_HOME, ON);
			myPs->misc.pitchChangeMoveTimer = myData->mData.misc.timer_1sec;
			myPs->pitchChangePhase = P6;
			break;
		case P6:
			flag1 = Read_InPoint(myPs->misc.JigNo+1, 0, I_MOTOR_HOME);
			flag2 = Read_InPoint(myPs->misc.JigNo+1, 0, I_MOTOR_ERROR);
			if(flag1 == P0 && flag2 ==P0) {
				myPs->pitchChangePhase = P7;
			}else{
				if((myData->mData.misc.timer_1sec - myPs->misc.pitchChangeMoveTimer)
					> 30) {
//					> myPs->config.cylMoveTimeout/1000) {
					Select_OutPoint(myPs->misc.JigNo+1, 0, O_MOTOR_HOME, OFF);
					myPs->code = J_CD_PITCH_CHANGE_ERROR;
					myPs->jigState = J_FAIL;
					myPs->microState = P0;
				}
			}
			break;
		case P7:
			Select_OutPoint(myPs->misc.JigNo+1, 0, O_MOTOR_RUN, ON);
			Select_OutPoint(myPs->misc.JigNo+1, 1, O_MOTOR_STEP, ON);
			Select_OutPoint(myPs->misc.JigNo+1, 2, O_MOTOR_STEP, OFF);
			Select_OutPoint(myPs->misc.JigNo+1, 3, O_MOTOR_STEP, OFF);
			Select_OutPoint(myPs->misc.JigNo+1, 4, O_MOTOR_STEP, OFF);
			myPs->misc.pitchChangeMoveTimer = myData->mData.misc.timer_1sec;
			myPs->pitchChangePhase = P8;
			break;
		case P8:
			flag1 = Read_InPoint(myPs->misc.JigNo+1, 0, I_MOTOR_RUN);
			flag2 = Read_InPoint(myPs->misc.JigNo+1, 0, I_MOTOR_ERROR);
			if(flag1 == P0 && flag2 == P0) {
				if((myData->mData.misc.timer_1sec - myPs->misc.jobChangeMoveTimer)
					> myPs->config.cylMoveDelay/1000) {
					Select_OutPoint(myPs->misc.JigNo+1, 0, O_MOTOR_RUN, OFF);
					myPs->pitchChangePhase = P9;
				}
			} else {
				if((myData->mData.misc.timer_1sec - myPs->misc.jobChangeMoveTimer)
					> 30) {
//					> myPs->config.cylMoveTimeout/1000) {
					Select_OutPoint(myPs->misc.JigNo+1, 0, O_MOTOR_RUN, OFF);
					myPs->code = J_CD_PITCH_CHANGE_ERROR;
					myPs->jigState = J_FAIL;
					myPs->microState = P0;
				}
			}
			break;
		case P9:
//			flag1 = Read_InPoint(myPs->misc.JigNo+1, 0, I_MOTOR_READY);
//			if(flag1 == P1 ) {
				myPs->pitchChangePhase = P0;
				myPs->pitchChangeEnd = PITCH_CHANGE_END;
//			}
			break;
	}
}

void motorControl_3(void)
{
	int flag1, flag2;
	
	if(myPs->config.jigUseFlag == UNUSE) {
		myPs->pitchChangeEnd = PITCH_CHANGE_END;
		return;
	}

	if(myData->dio.misc.delayTimer < myData->dio.config.dioDelay) return;
	if(myPs->jigNotMove == ON) return;
	if(myPs->pitchChangeEnd != PITCH_CHANGE_START) return;

	switch(myPs->pitchChangePhase) {
		case P0:
			flag1 = Read_InPoint(myPs->misc.JigNo+1, 0, I_MOTOR_ERROR);
			if(flag1 == P0) {
				myPs->misc.pitchChangeMoveTimer 
					= myData->mData.misc.timer_1sec;
				myPs->pitchChangePhase = P5;
			}else{
				Select_OutPoint(myPs->misc.JigNo+1, 0, O_MOTOR_RESET, ON);
				myPs->pitchChangePhase = P1;
			}
			break;
		case P1:
			Select_OutPoint(myPs->misc.JigNo+1, 0, O_MOTOR_RESET, OFF);
			myPs->misc.pitchChangeMoveTimer 
				= myData->mData.misc.timer_1sec;
			myPs->pitchChangePhase = P2;
			break;
		case P2:
			flag1 = Read_InPoint(myPs->misc.JigNo+1, 0, I_MOTOR_ERROR);
			if(flag1 == P0) {
				myPs->pitchChangePhase = P5;
			}else{
				if((myData->mData.misc.timer_1sec - myPs->misc.pitchChangeMoveTimer)
					> myPs->config.cylMoveTimeout/1000) {
					myPs->code = J_CD_PITCH_CHANGE_ERROR;
					myPs->jigState = J_FAIL;
					myPs->microState = P0;
				}
			}
			break;
		case P5:
			Select_OutPoint(myPs->misc.JigNo+1, 0, O_MOTOR_HOME, ON);
			myPs->misc.pitchChangeMoveTimer = myData->mData.misc.timer_1sec;
			myPs->pitchChangePhase = P6;
			break;
		case P6:
			flag1 = Read_InPoint(myPs->misc.JigNo+1, 0, I_MOTOR_HOME);
			flag2 = Read_InPoint(myPs->misc.JigNo+1, 0, I_MOTOR_ERROR);
			if(flag1 == P0 && flag2 ==P0) {
				myPs->pitchChangePhase = P7;
			}else{
				if((myData->mData.misc.timer_1sec - myPs->misc.pitchChangeMoveTimer)
					> 30) {
//					> myPs->config.cylMoveTimeout/1000) {
					Select_OutPoint(myPs->misc.JigNo+1, 0, O_MOTOR_HOME, OFF);
					myPs->code = J_CD_PITCH_CHANGE_ERROR;
					myPs->jigState = J_FAIL;
					myPs->microState = P0;
				}
			}
			break;
		case P7:
			Select_OutPoint(myPs->misc.JigNo+1, 0, O_MOTOR_RUN, ON);
			Select_OutPoint(myPs->misc.JigNo+1, 1, O_MOTOR_STEP, OFF);
			Select_OutPoint(myPs->misc.JigNo+1, 2, O_MOTOR_STEP, ON);
			Select_OutPoint(myPs->misc.JigNo+1, 3, O_MOTOR_STEP, OFF);
			Select_OutPoint(myPs->misc.JigNo+1, 4, O_MOTOR_STEP, OFF);
			myPs->misc.pitchChangeMoveTimer = myData->mData.misc.timer_1sec;
			myPs->pitchChangePhase = P8;
			break;
		case P8:
			flag1 = Read_InPoint(myPs->misc.JigNo+1, 0, I_MOTOR_RUN);
			flag2 = Read_InPoint(myPs->misc.JigNo+1, 0, I_MOTOR_ERROR);
			if(flag1 == P0 && flag2 == P0) {
				if((myData->mData.misc.timer_1sec - myPs->misc.jobChangeMoveTimer)
					> myPs->config.cylMoveDelay/1000) {
					Select_OutPoint(myPs->misc.JigNo+1, 0, O_MOTOR_RUN, OFF);
					myPs->pitchChangePhase = P9;
				}
			} else {
				if((myData->mData.misc.timer_1sec - myPs->misc.jobChangeMoveTimer)
					> 30) {
//					> myPs->config.cylMoveTimeout/1000) {
					Select_OutPoint(myPs->misc.JigNo+1, 0, O_MOTOR_RUN, OFF);
					myPs->code = J_CD_PITCH_CHANGE_ERROR;
					myPs->jigState = J_FAIL;
					myPs->microState = P0;
				}
			}
			break;
		case P9:
//			flag1 = Read_InPoint(myPs->misc.JigNo+1, 0, I_MOTOR_READY);
//			if(flag1 == P1 ) {
				myPs->pitchChangePhase = P0;
				myPs->pitchChangeEnd = PITCH_CHANGE_END;
//			}
			break;
	}
}

void motorControl_4(void)
{
	int flag1, flag2;
	
	if(myPs->config.jigUseFlag == UNUSE) {
		myPs->pitchChangeEnd = PITCH_CHANGE_END;
		return;
	}

	if(myData->dio.misc.delayTimer < myData->dio.config.dioDelay) return;
	if(myPs->jigNotMove == ON) return;
	if(myPs->pitchChangeEnd != PITCH_CHANGE_START) return;

	switch(myPs->pitchChangePhase) {
		case P0:
			flag1 = Read_InPoint(myPs->misc.JigNo+1, 0, I_MOTOR_ERROR);
			if(flag1 == P0) {
				myPs->misc.pitchChangeMoveTimer 
					= myData->mData.misc.timer_1sec;
				myPs->pitchChangePhase = P5;
			}else{
				Select_OutPoint(myPs->misc.JigNo+1, 0, O_MOTOR_RESET, ON);
				myPs->pitchChangePhase = P1;
			}
			break;
		case P1:
			Select_OutPoint(myPs->misc.JigNo+1, 0, O_MOTOR_RESET, OFF);
			myPs->misc.pitchChangeMoveTimer 
				= myData->mData.misc.timer_1sec;
			myPs->pitchChangePhase = P2;
			break;
		case P2:
			flag1 = Read_InPoint(myPs->misc.JigNo+1, 0, I_MOTOR_ERROR);
			if(flag1 == P0) {
				myPs->pitchChangePhase = P5;
			}else{
				if((myData->mData.misc.timer_1sec - myPs->misc.pitchChangeMoveTimer)
					> myPs->config.cylMoveTimeout/1000) {
					myPs->code = J_CD_PITCH_CHANGE_ERROR;
					myPs->jigState = J_FAIL;
					myPs->microState = P0;
				}
			}
			break;
		case P5:
			Select_OutPoint(myPs->misc.JigNo+1, 0, O_MOTOR_HOME, ON);
			myPs->misc.pitchChangeMoveTimer = myData->mData.misc.timer_1sec;
			myPs->pitchChangePhase = P6;
			break;
		case P6:
			flag1 = Read_InPoint(myPs->misc.JigNo+1, 0, I_MOTOR_HOME);
			flag2 = Read_InPoint(myPs->misc.JigNo+1, 0, I_MOTOR_ERROR);
			if(flag1 == P0 && flag2 ==P0) {
				myPs->pitchChangePhase = P7;
			}else{
				if((myData->mData.misc.timer_1sec - myPs->misc.pitchChangeMoveTimer)
					> 30) {
//					> myPs->config.cylMoveTimeout/1000) {
					Select_OutPoint(myPs->misc.JigNo+1, 0, O_MOTOR_HOME, OFF);
					myPs->code = J_CD_PITCH_CHANGE_ERROR;
					myPs->jigState = J_FAIL;
					myPs->microState = P0;
				}
			}
			break;
		case P7:
			Select_OutPoint(myPs->misc.JigNo+1, 0, O_MOTOR_RUN, ON);
			Select_OutPoint(myPs->misc.JigNo+1, 1, O_MOTOR_STEP, ON);
			Select_OutPoint(myPs->misc.JigNo+1, 2, O_MOTOR_STEP, ON);
			Select_OutPoint(myPs->misc.JigNo+1, 3, O_MOTOR_STEP, OFF);
			Select_OutPoint(myPs->misc.JigNo+1, 4, O_MOTOR_STEP, OFF);
			myPs->misc.pitchChangeMoveTimer = myData->mData.misc.timer_1sec;
			myPs->pitchChangePhase = P8;
			break;
		case P8:
			flag1 = Read_InPoint(myPs->misc.JigNo+1, 0, I_MOTOR_RUN);
			flag2 = Read_InPoint(myPs->misc.JigNo+1, 0, I_MOTOR_ERROR);
			if(flag1 == P0 && flag2 == P0) {
				if((myData->mData.misc.timer_1sec - myPs->misc.jobChangeMoveTimer)
					> myPs->config.cylMoveDelay/1000) {
					Select_OutPoint(myPs->misc.JigNo+1, 0, O_MOTOR_RUN, OFF);
					myPs->pitchChangePhase = P9;
				}
			} else {
				if((myData->mData.misc.timer_1sec - myPs->misc.jobChangeMoveTimer)
					> 30) {
//					> myPs->config.cylMoveTimeout/1000) {
					Select_OutPoint(myPs->misc.JigNo+1, 0, O_MOTOR_RUN, OFF);
					myPs->code = J_CD_PITCH_CHANGE_ERROR;
					myPs->jigState = J_FAIL;
					myPs->microState = P0;
				}
			}
			break;
		case P9:
//			flag1 = Read_InPoint(myPs->misc.JigNo+1, 0, I_MOTOR_READY);
//			if(flag1 == P1 ) {
				myPs->pitchChangePhase = P0;
				myPs->pitchChangeEnd = PITCH_CHANGE_END;
//			}
			break;
	}
}

void motorControl_5(void)
{
	int flag1, flag2;
	
	if(myPs->config.jigUseFlag == UNUSE) {
		myPs->pitchChangeEnd = PITCH_CHANGE_END;
		return;
	}

	if(myData->dio.misc.delayTimer < myData->dio.config.dioDelay) return;
	if(myPs->jigNotMove == ON) return;
	if(myPs->pitchChangeEnd != PITCH_CHANGE_START) return;

	switch(myPs->pitchChangePhase) {
		case P0:
			flag1 = Read_InPoint(myPs->misc.JigNo+1, 0, I_MOTOR_ERROR);
			if(flag1 == P0) {
				myPs->misc.pitchChangeMoveTimer 
					= myData->mData.misc.timer_1sec;
				myPs->pitchChangePhase = P5;
			}else{
				Select_OutPoint(myPs->misc.JigNo+1, 0, O_MOTOR_RESET, ON);
				myPs->pitchChangePhase = P1;
			}
			break;
		case P1:
			Select_OutPoint(myPs->misc.JigNo+1, 0, O_MOTOR_RESET, OFF);
			myPs->misc.pitchChangeMoveTimer 
				= myData->mData.misc.timer_1sec;
			myPs->pitchChangePhase = P2;
			break;
		case P2:
			flag1 = Read_InPoint(myPs->misc.JigNo+1, 0, I_MOTOR_ERROR);
			if(flag1 == P0) {
				myPs->pitchChangePhase = P5;
			}else{
				if((myData->mData.misc.timer_1sec - myPs->misc.pitchChangeMoveTimer)
					> myPs->config.cylMoveTimeout/1000) {
					myPs->code = J_CD_PITCH_CHANGE_ERROR;
					myPs->jigState = J_FAIL;
					myPs->microState = P0;
				}
			}
			break;
		case P5:
			Select_OutPoint(myPs->misc.JigNo+1, 0, O_MOTOR_HOME, ON);
			myPs->misc.pitchChangeMoveTimer = myData->mData.misc.timer_1sec;
			myPs->pitchChangePhase = P6;
			break;
		case P6:
			flag1 = Read_InPoint(myPs->misc.JigNo+1, 0, I_MOTOR_HOME);
			flag2 = Read_InPoint(myPs->misc.JigNo+1, 0, I_MOTOR_ERROR);
			if(flag1 == P0 && flag2 ==P0) {
				myPs->pitchChangePhase = P7;
			}else{
				if((myData->mData.misc.timer_1sec - myPs->misc.pitchChangeMoveTimer)
					> 30) {
//					> myPs->config.cylMoveTimeout/1000) {
					Select_OutPoint(myPs->misc.JigNo+1, 0, O_MOTOR_HOME, OFF);
					myPs->code = J_CD_PITCH_CHANGE_ERROR;
					myPs->jigState = J_FAIL;
					myPs->microState = P0;
				}
			}
			break;
		case P7:
			Select_OutPoint(myPs->misc.JigNo+1, 0, O_MOTOR_RUN, ON);
			Select_OutPoint(myPs->misc.JigNo+1, 1, O_MOTOR_STEP, OFF);
			Select_OutPoint(myPs->misc.JigNo+1, 2, O_MOTOR_STEP, OFF);
			Select_OutPoint(myPs->misc.JigNo+1, 3, O_MOTOR_STEP, ON);
			Select_OutPoint(myPs->misc.JigNo+1, 4, O_MOTOR_STEP, OFF);
			myPs->misc.pitchChangeMoveTimer = myData->mData.misc.timer_1sec;
			myPs->pitchChangePhase = P8;
			break;
		case P8:
			flag1 = Read_InPoint(myPs->misc.JigNo+1, 0, I_MOTOR_RUN);
			flag2 = Read_InPoint(myPs->misc.JigNo+1, 0, I_MOTOR_ERROR);
			if(flag1 == P0 && flag2 == P0) {
				if((myData->mData.misc.timer_1sec - myPs->misc.jobChangeMoveTimer)
					> myPs->config.cylMoveDelay/1000) {
					Select_OutPoint(myPs->misc.JigNo+1, 0, O_MOTOR_RUN, OFF);
					myPs->pitchChangePhase = P9;
				}
			} else {
				if((myData->mData.misc.timer_1sec - myPs->misc.jobChangeMoveTimer)
					> 30) {
//					> myPs->config.cylMoveTimeout/1000) {
					Select_OutPoint(myPs->misc.JigNo+1, 0, O_MOTOR_RUN, OFF);
					myPs->code = J_CD_PITCH_CHANGE_ERROR;
					myPs->jigState = J_FAIL;
					myPs->microState = P0;
				}
			}
			break;
		case P9:
//			flag1 = Read_InPoint(myPs->misc.JigNo+1, 0, I_MOTOR_READY);
//			if(flag1 == P1 ) {
				myPs->pitchChangePhase = P0;
				myPs->pitchChangeEnd = PITCH_CHANGE_END;
//			}
			break;
	}
}

void motorControl_6(void)
{
	int flag1, flag2;
	
	if(myPs->config.jigUseFlag == UNUSE) {
		myPs->pitchChangeEnd = PITCH_CHANGE_END;
		return;
	}

	if(myData->dio.misc.delayTimer < myData->dio.config.dioDelay) return;
	if(myPs->jigNotMove == ON) return;
	if(myPs->pitchChangeEnd != PITCH_CHANGE_START) return;

	switch(myPs->pitchChangePhase) {
		case P0:
			flag1 = Read_InPoint(myPs->misc.JigNo+1, 0, I_MOTOR_ERROR);
			if(flag1 == P0) {
				myPs->misc.pitchChangeMoveTimer 
					= myData->mData.misc.timer_1sec;
				myPs->pitchChangePhase = P5;
			}else{
				Select_OutPoint(myPs->misc.JigNo+1, 0, O_MOTOR_RESET, ON);
				myPs->pitchChangePhase = P1;
			}
			break;
		case P1:
			Select_OutPoint(myPs->misc.JigNo+1, 0, O_MOTOR_RESET, OFF);
			myPs->misc.pitchChangeMoveTimer 
				= myData->mData.misc.timer_1sec;
			myPs->pitchChangePhase = P2;
			break;
		case P2:
			flag1 = Read_InPoint(myPs->misc.JigNo+1, 0, I_MOTOR_ERROR);
			if(flag1 == P0) {
				myPs->pitchChangePhase = P5;
			}else{
				if((myData->mData.misc.timer_1sec - myPs->misc.pitchChangeMoveTimer)
					> myPs->config.cylMoveTimeout/1000) {
					myPs->code = J_CD_PITCH_CHANGE_ERROR;
					myPs->jigState = J_FAIL;
					myPs->microState = P0;
				}
			}
			break;
		case P5:
			Select_OutPoint(myPs->misc.JigNo+1, 0, O_MOTOR_HOME, ON);
			myPs->misc.pitchChangeMoveTimer = myData->mData.misc.timer_1sec;
			myPs->pitchChangePhase = P6;
			break;
		case P6:
			flag1 = Read_InPoint(myPs->misc.JigNo+1, 0, I_MOTOR_HOME);
			flag2 = Read_InPoint(myPs->misc.JigNo+1, 0, I_MOTOR_ERROR);
			if(flag1 == P0 && flag2 ==P0) {
				myPs->pitchChangePhase = P7;
			}else{
				if((myData->mData.misc.timer_1sec - myPs->misc.pitchChangeMoveTimer)
					> 30) {
//					> myPs->config.cylMoveTimeout/1000) {
					Select_OutPoint(myPs->misc.JigNo+1, 0, O_MOTOR_HOME, OFF);
					myPs->code = J_CD_PITCH_CHANGE_ERROR;
					myPs->jigState = J_FAIL;
					myPs->microState = P0;
				}
			}
			break;
		case P7:
			Select_OutPoint(myPs->misc.JigNo+1, 0, O_MOTOR_RUN, ON);
			Select_OutPoint(myPs->misc.JigNo+1, 1, O_MOTOR_STEP, ON);
			Select_OutPoint(myPs->misc.JigNo+1, 2, O_MOTOR_STEP, OFF);
			Select_OutPoint(myPs->misc.JigNo+1, 3, O_MOTOR_STEP, ON);
			Select_OutPoint(myPs->misc.JigNo+1, 4, O_MOTOR_STEP, OFF);
			myPs->misc.pitchChangeMoveTimer = myData->mData.misc.timer_1sec;
			myPs->pitchChangePhase = P8;
			break;
		case P8:
			flag1 = Read_InPoint(myPs->misc.JigNo+1, 0, I_MOTOR_RUN);
			flag2 = Read_InPoint(myPs->misc.JigNo+1, 0, I_MOTOR_ERROR);
			if(flag1 == P0 && flag2 == P0) {
				if((myData->mData.misc.timer_1sec - myPs->misc.jobChangeMoveTimer)
					> myPs->config.cylMoveDelay/1000) {
					Select_OutPoint(myPs->misc.JigNo+1, 0, O_MOTOR_RUN, OFF);
					myPs->pitchChangePhase = P9;
				}
			} else {
				if((myData->mData.misc.timer_1sec - myPs->misc.jobChangeMoveTimer)
					> 30) {
//					> myPs->config.cylMoveTimeout/1000) {
					Select_OutPoint(myPs->misc.JigNo+1, 0, O_MOTOR_RUN, OFF);
					myPs->code = J_CD_PITCH_CHANGE_ERROR;
					myPs->jigState = J_FAIL;
					myPs->microState = P0;
				}
			}
			break;
		case P9:
//			flag1 = Read_InPoint(myPs->misc.JigNo+1, 0, I_MOTOR_READY);
//			if(flag1 == P1 ) {
				myPs->pitchChangePhase = P0;
				myPs->pitchChangeEnd = PITCH_CHANGE_END;
//			}
			break;
	}
}

void motorControl_7(void)
{
	int flag1, flag2;
	
	if(myPs->config.jigUseFlag == UNUSE) {
		myPs->pitchChangeEnd = PITCH_CHANGE_END;
		return;
	}

	if(myData->dio.misc.delayTimer < myData->dio.config.dioDelay) return;
	if(myPs->jigNotMove == ON) return;
	if(myPs->pitchChangeEnd != PITCH_CHANGE_START) return;

	switch(myPs->pitchChangePhase) {
		case P0:
			flag1 = Read_InPoint(myPs->misc.JigNo+1, 0, I_MOTOR_ERROR);
			if(flag1 == P0) {
				myPs->misc.pitchChangeMoveTimer 
					= myData->mData.misc.timer_1sec;
				myPs->pitchChangePhase = P5;
			}else{
				Select_OutPoint(myPs->misc.JigNo+1, 0, O_MOTOR_RESET, ON);
				myPs->pitchChangePhase = P1;
			}
			break;
		case P1:
			Select_OutPoint(myPs->misc.JigNo+1, 0, O_MOTOR_RESET, OFF);
			myPs->misc.pitchChangeMoveTimer 
				= myData->mData.misc.timer_1sec;
			myPs->pitchChangePhase = P2;
			break;
		case P2:
			flag1 = Read_InPoint(myPs->misc.JigNo+1, 0, I_MOTOR_ERROR);
			if(flag1 == P0) {
				myPs->pitchChangePhase = P5;
			}else{
				if((myData->mData.misc.timer_1sec - myPs->misc.pitchChangeMoveTimer)
					> myPs->config.cylMoveTimeout/1000) {
					myPs->code = J_CD_PITCH_CHANGE_ERROR;
					myPs->jigState = J_FAIL;
					myPs->microState = P0;
				}
			}
			break;
		case P5:
			Select_OutPoint(myPs->misc.JigNo+1, 0, O_MOTOR_HOME, ON);
			myPs->misc.pitchChangeMoveTimer = myData->mData.misc.timer_1sec;
			myPs->pitchChangePhase = P6;
			break;
		case P6:
			flag1 = Read_InPoint(myPs->misc.JigNo+1, 0, I_MOTOR_HOME);
			flag2 = Read_InPoint(myPs->misc.JigNo+1, 0, I_MOTOR_ERROR);
			if(flag1 == P0 && flag2 ==P0) {
				myPs->pitchChangePhase = P7;
			}else{
				if((myData->mData.misc.timer_1sec - myPs->misc.pitchChangeMoveTimer)
					> 30) {
//					> myPs->config.cylMoveTimeout/1000) {
					Select_OutPoint(myPs->misc.JigNo+1, 0, O_MOTOR_HOME, OFF);
					myPs->code = J_CD_PITCH_CHANGE_ERROR;
					myPs->jigState = J_FAIL;
					myPs->microState = P0;
				}
			}
			break;
		case P7:
			Select_OutPoint(myPs->misc.JigNo+1, 0, O_MOTOR_RUN, ON);
			Select_OutPoint(myPs->misc.JigNo+1, 1, O_MOTOR_STEP, OFF);
			Select_OutPoint(myPs->misc.JigNo+1, 2, O_MOTOR_STEP, ON);
			Select_OutPoint(myPs->misc.JigNo+1, 3, O_MOTOR_STEP, ON);
			Select_OutPoint(myPs->misc.JigNo+1, 4, O_MOTOR_STEP, OFF);
			myPs->misc.pitchChangeMoveTimer = myData->mData.misc.timer_1sec;
			myPs->pitchChangePhase = P8;
			break;
		case P8:
			flag1 = Read_InPoint(myPs->misc.JigNo+1, 0, I_MOTOR_RUN);
			flag2 = Read_InPoint(myPs->misc.JigNo+1, 0, I_MOTOR_ERROR);
			if(flag1 == P0 && flag2 == P0) {
				if((myData->mData.misc.timer_1sec - myPs->misc.jobChangeMoveTimer)
					> myPs->config.cylMoveDelay/1000) {
					Select_OutPoint(myPs->misc.JigNo+1, 0, O_MOTOR_RUN, OFF);
					myPs->pitchChangePhase = P9;
				}
			} else {
				if((myData->mData.misc.timer_1sec - myPs->misc.jobChangeMoveTimer)
					> 30) {
//					> myPs->config.cylMoveTimeout/1000) {
					Select_OutPoint(myPs->misc.JigNo+1, 0, O_MOTOR_RUN, OFF);
					myPs->code = J_CD_PITCH_CHANGE_ERROR;
					myPs->jigState = J_FAIL;
					myPs->microState = P0;
				}
			}
			break;
		case P9:
//			flag1 = Read_InPoint(myPs->misc.JigNo+1, 0, I_MOTOR_READY);
//			if(flag1 == P1 ) {
				myPs->pitchChangePhase = P0;
				myPs->pitchChangeEnd = PITCH_CHANGE_END;
//			}
			break;
	}
}

void motorControl_8(void)
{
	int flag1, flag2;
	
	if(myPs->config.jigUseFlag == UNUSE) {
		myPs->pitchChangeEnd = PITCH_CHANGE_END;
		return;
	}

	if(myData->dio.misc.delayTimer < myData->dio.config.dioDelay) return;
	if(myPs->jigNotMove == ON) return;
	if(myPs->pitchChangeEnd != PITCH_CHANGE_START) return;

	switch(myPs->pitchChangePhase) {
		case P0:
			flag1 = Read_InPoint(myPs->misc.JigNo+1, 0, I_MOTOR_ERROR);
			if(flag1 == P0) {
				myPs->misc.pitchChangeMoveTimer 
					= myData->mData.misc.timer_1sec;
				myPs->pitchChangePhase = P5;
			}else{
				Select_OutPoint(myPs->misc.JigNo+1, 0, O_MOTOR_RESET, ON);
				myPs->pitchChangePhase = P1;
			}
			break;
		case P1:
			Select_OutPoint(myPs->misc.JigNo+1, 0, O_MOTOR_RESET, OFF);
			myPs->misc.pitchChangeMoveTimer 
				= myData->mData.misc.timer_1sec;
			myPs->pitchChangePhase = P2;
			break;
		case P2:
			flag1 = Read_InPoint(myPs->misc.JigNo+1, 0, I_MOTOR_ERROR);
			if(flag1 == P0) {
				myPs->pitchChangePhase = P5;
			}else{
				if((myData->mData.misc.timer_1sec - myPs->misc.pitchChangeMoveTimer)
					> myPs->config.cylMoveTimeout/1000) {
					myPs->code = J_CD_PITCH_CHANGE_ERROR;
					myPs->jigState = J_FAIL;
					myPs->microState = P0;
				}
			}
			break;
		case P5:
			Select_OutPoint(myPs->misc.JigNo+1, 0, O_MOTOR_HOME, ON);
			myPs->misc.pitchChangeMoveTimer = myData->mData.misc.timer_1sec;
			myPs->pitchChangePhase = P6;
			break;
		case P6:
			flag1 = Read_InPoint(myPs->misc.JigNo+1, 0, I_MOTOR_HOME);
			flag2 = Read_InPoint(myPs->misc.JigNo+1, 0, I_MOTOR_ERROR);
			if(flag1 == P0 && flag2 ==P0) {
				myPs->pitchChangePhase = P7;
			}else{
				if((myData->mData.misc.timer_1sec - myPs->misc.pitchChangeMoveTimer)
					> 30) {
//					> myPs->config.cylMoveTimeout/1000) {
					Select_OutPoint(myPs->misc.JigNo+1, 0, O_MOTOR_HOME, OFF);
					myPs->code = J_CD_PITCH_CHANGE_ERROR;
					myPs->jigState = J_FAIL;
					myPs->microState = P0;
				}
			}
			break;
		case P7:
			Select_OutPoint(myPs->misc.JigNo+1, 0, O_MOTOR_RUN, ON);
			Select_OutPoint(myPs->misc.JigNo+1, 1, O_MOTOR_STEP, ON);
			Select_OutPoint(myPs->misc.JigNo+1, 2, O_MOTOR_STEP, ON);
			Select_OutPoint(myPs->misc.JigNo+1, 3, O_MOTOR_STEP, ON);
			Select_OutPoint(myPs->misc.JigNo+1, 4, O_MOTOR_STEP, OFF);
			myPs->misc.pitchChangeMoveTimer = myData->mData.misc.timer_1sec;
			myPs->pitchChangePhase = P8;
			break;
		case P8:
			flag1 = Read_InPoint(myPs->misc.JigNo+1, 0, I_MOTOR_RUN);
			flag2 = Read_InPoint(myPs->misc.JigNo+1, 0, I_MOTOR_ERROR);
			if(flag1 == P0 && flag2 == P0) {
				if((myData->mData.misc.timer_1sec - myPs->misc.jobChangeMoveTimer)
					> myPs->config.cylMoveDelay/1000) {
					Select_OutPoint(myPs->misc.JigNo+1, 0, O_MOTOR_RUN, OFF);
					myPs->pitchChangePhase = P9;
				}
			} else {
				if((myData->mData.misc.timer_1sec - myPs->misc.jobChangeMoveTimer)
					> 30) {
//					> myPs->config.cylMoveTimeout/1000) {
					Select_OutPoint(myPs->misc.JigNo+1, 0, O_MOTOR_RUN, OFF);
					myPs->code = J_CD_PITCH_CHANGE_ERROR;
					myPs->jigState = J_FAIL;
					myPs->microState = P0;
				}
			}
			break;
		case P9:
//			flag1 = Read_InPoint(myPs->misc.JigNo+1, 0, I_MOTOR_READY);
//			if(flag1 == P1 ) {
				myPs->pitchChangePhase = P0;
				myPs->pitchChangeEnd = PITCH_CHANGE_END;
//			}
			break;
	}
}
