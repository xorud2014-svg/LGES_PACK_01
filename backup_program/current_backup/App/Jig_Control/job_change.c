#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../../INC/datastore.h"
#include "userlog.h"
#include "common_utils.h"
#include "local_utils.h"
#include "common_message.h"
#include "local_message.h"
#include "job_change.h"

extern volatile S_SYSTEM_DATA	*myData;
extern volatile S_JIG_DATA		*myPs;
extern char psName[PROCESS_NAME_SIZE];

void Check_JobChange(void)
{
	switch(myPs->signal[J_SIG_JOB_CHANGE]) {
		case JOB_CHANGE_1:
			jobChange_1();
			break;
		case JOB_CHANGE_2:
			jobChange_2();
			break;
		case JOB_CHANGE_3:
			jobChange_3();
			break;
		case JOB_CHANGE_4:
			jobChange_4();
			break;
		case JOB_CHANGE_5:
			jobChange_5();
			break;
		case JOB_CHANGE_6:
			jobChange_6();
			break;
		case JOB_CHANGE_7:
			jobChange_7();
			break;
		case JOB_CHANGE_8:
			jobChange_8();
			break;
	}
}

void jobChange_1(void)
{
	int flag1, flag2, flag3;
	S_MSG_VAL SendMsg;
	
	if(myPs->config.jigUseFlag == UNUSE) {
		return;
	}

	if(myData->dio.misc.delayTimer < myData->dio.config.dioDelay) return;
	if(myPs->jigNotMove == ON) return;
	if(myPs->jobChangeEnd != JOB_CHANGE_START) return;

	switch(myPs->jobChangePhase) {
		case P0:
			Select_OutPoint(myPs->misc.JigNo+1, 0, O_JIG_CHANGE_LOCK, ON);
			myPs->misc.jobChangeMoveTimer = myData->mData.misc.timer_1sec;
			myPs->jobChangePhase = P1;
			break;
		case P1:
			flag1 = Read_InPoint(myPs->misc.JigNo+1, 0, I_JIG_CHANGE_LOCK);
			if(flag1 == P1) {
				if((myData->mData.misc.timer_1sec - myPs->misc.jobChangeMoveTimer)
					> myPs->config.cylMoveDelay/1000) {
					myPs->jobChangePhase = P2;
				}
			} else {
				if((myData->mData.misc.timer_1sec - myPs->misc.jobChangeMoveTimer)
					> myPs->config.cylMoveTimeout/1000) {
					myPs->code = J_CD_JOB_CHANGE_ERROR;
					myPs->jigState = J_FAIL;
					myPs->microState = P0;
					Select_OutPoint(myPs->misc.JigNo+1, 0, O_JIG_CHANGE_LOCK, OFF);
					myPs->signal[J_SIG_JOB_CHANGE] = JOB_CHANGE_IDLE;
					myPs->jobChangePhase = P0;
				}
			}
			break;
		case P2:
			Select_OutPoint(myPs->misc.JigNo+1, 1, O_JIG_CHANGE_X, OFF); //push
			Select_OutPoint(myPs->misc.JigNo+1, 2, O_JIG_CHANGE_X, ON); //pull
			Select_OutPoint(myPs->misc.JigNo+1, 1, O_JIG_CHANGE_Y, OFF); //push
			Select_OutPoint(myPs->misc.JigNo+1, 2, O_JIG_CHANGE_Y, ON); //pull
			Select_OutPoint(myPs->misc.JigNo+1, 1, O_JIG_CHANGE_Z, OFF); //push
			Select_OutPoint(myPs->misc.JigNo+1, 2, O_JIG_CHANGE_Z, ON); //pull
			myPs->misc.jobChangeMoveTimer = myData->mData.misc.timer_1sec;
			myPs->jobChangePhase = P3;
			break;
		case P3:
			flag1 = Read_InPoint(myPs->misc.JigNo+1, 0, I_JIG_CHANGE_X);
			flag2 = Read_InPoint(myPs->misc.JigNo+1, 0, I_JIG_CHANGE_Y);
			flag3 = Read_InPoint(myPs->misc.JigNo+1, 0, I_JIG_CHANGE_Z);
			if(flag1 == P0 && flag2 == P0 && flag3 == P0) {

				if((myData->mData.misc.timer_1sec - myPs->misc.jobChangeMoveTimer)
					> myPs->config.cylMoveDelay/1000) {
						Select_OutPoint(myPs->misc.JigNo+1, 0, O_JIG_CHANGE_LOCK, OFF);
						myPs->jobChangePhase = P4;
				}
			} else {
				if((myData->mData.misc.timer_1sec - myPs->misc.jobChangeMoveTimer)
					> myPs->config.cylMoveTimeout/1000) {
					myPs->code = J_CD_JOB_CHANGE_ERROR;
					myPs->jigState = J_FAIL;
//					Select_OutPoint(myPs->misc.JigNo+1, 0, O_JIG_CHANGE_LOCK, OFF);
					myPs->microState = P0;
					myPs->signal[J_SIG_JOB_CHANGE] = JOB_CHANGE_IDLE;
					myPs->jobChangePhase = P0;
				}
			}
			break;
		case P4:
			flag1 = Read_InPoint(myPs->misc.JigNo+1, 0, I_JIG_CHANGE_LOCK);
			if(flag1 == P0) {
				if((myData->mData.misc.timer_1sec - myPs->misc.jobChangeMoveTimer)
					> myPs->config.cylMoveDelay/1000) {
					myPs->jobChangeEnd = JOB_CHANGE_END;
					myPs->jobChangePhase = P5;
				}
			}else{
				if((myData->mData.misc.timer_1sec - myPs->misc.jobChangeMoveTimer)
					> myPs->config.cylMoveTimeout/1000) {
					myPs->code = J_CD_JOB_CHANGE_ERROR;
					myPs->jigState = J_FAIL;
					myPs->microState = P0;
					Select_OutPoint(myPs->misc.JigNo+1, 0, O_JIG_CHANGE_LOCK, OFF);
					myPs->signal[J_SIG_JOB_CHANGE] = JOB_CHANGE_IDLE;
					myPs->jobChangePhase = P0;
				}
			}
			break;
		case P5:
			if(myPs->pitchChangeEnd == PITCH_CHANGE_END) {
				memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
				SendMsg.msg = MSG_JIGC_COB_JOB_CHANGE_END;
				SendMsg.val[0] = (int)myPs->signal[J_SIG_JOB_CHANGE];
				send_msg(JIGC_TO_COB1, (char *)&SendMsg);
				myPs->jobChangePhase = P6;
			}
			break;
		case P6:
			break;
		default: break;
	}
}

void jobChange_3(void)
{
	int flag1, flag2, flag3;
	S_MSG_VAL SendMsg;
	
	if(myPs->config.jigUseFlag == UNUSE) {
		return;
	}

	if(myData->dio.misc.delayTimer < myData->dio.config.dioDelay) return;
	if(myPs->jigNotMove == ON) return;
	if(myPs->jobChangeEnd != JOB_CHANGE_START) return;

	switch(myPs->jobChangePhase) {
		case P0:
			Select_OutPoint(myPs->misc.JigNo+1, 0, O_JIG_CHANGE_LOCK, ON);
			myPs->misc.jobChangeMoveTimer = myData->mData.misc.timer_1sec;
			myPs->jobChangePhase = P1;
			break;
		case P1:
			flag1 = Read_InPoint(myPs->misc.JigNo+1, 0, I_JIG_CHANGE_LOCK);
			if(flag1 == P1) {
				if((myData->mData.misc.timer_1sec - myPs->misc.jobChangeMoveTimer)
					> myPs->config.cylMoveDelay/1000) {
					myPs->jobChangePhase = P2;
				}
			} else {
				if((myData->mData.misc.timer_1sec - myPs->misc.jobChangeMoveTimer)
					> myPs->config.cylMoveTimeout/1000) {
					myPs->code = J_CD_JOB_CHANGE_ERROR;
					myPs->jigState = J_FAIL;
					myPs->microState = P0;
					Select_OutPoint(myPs->misc.JigNo+1, 0, O_JIG_CHANGE_LOCK, OFF);
					myPs->signal[J_SIG_JOB_CHANGE] = JOB_CHANGE_IDLE;
					myPs->jobChangePhase = P0;
				}
			}
			break;
		case P2:
			Select_OutPoint(myPs->misc.JigNo+1, 1, O_JIG_CHANGE_X, ON); //push
			Select_OutPoint(myPs->misc.JigNo+1, 2, O_JIG_CHANGE_X, OFF); //pull
			Select_OutPoint(myPs->misc.JigNo+1, 1, O_JIG_CHANGE_Y, OFF); //push
			Select_OutPoint(myPs->misc.JigNo+1, 2, O_JIG_CHANGE_Y, ON); //pull
			Select_OutPoint(myPs->misc.JigNo+1, 1, O_JIG_CHANGE_Z, OFF); //push
			Select_OutPoint(myPs->misc.JigNo+1, 2, O_JIG_CHANGE_Z, ON); //pull
			myPs->misc.jobChangeMoveTimer = myData->mData.misc.timer_1sec;
			myPs->jobChangePhase = P3;
			break;
		case P3:
			flag1 = Read_InPoint(myPs->misc.JigNo+1, 0, I_JIG_CHANGE_X);
			flag2 = Read_InPoint(myPs->misc.JigNo+1, 0, I_JIG_CHANGE_Y);
			flag3 = Read_InPoint(myPs->misc.JigNo+1, 0, I_JIG_CHANGE_Z);
			if(flag1 == P1 && flag2 == P0 && flag3 == P0) {
				if((myData->mData.misc.timer_1sec - myPs->misc.jobChangeMoveTimer)
					> myPs->config.cylMoveDelay/1000) {
						Select_OutPoint(myPs->misc.JigNo+1, 0, O_JIG_CHANGE_LOCK, OFF);
						myPs->jobChangePhase = P4;
				}
			} else {
				if((myData->mData.misc.timer_1sec - myPs->misc.jobChangeMoveTimer)
					> myPs->config.cylMoveTimeout/1000) {
					myPs->code = J_CD_JOB_CHANGE_ERROR;
					myPs->jigState = J_FAIL;
					myPs->microState = P0;
//					Select_OutPoint(myPs->misc.JigNo+1, 0, O_JIG_CHANGE_LOCK, OFF);
					myPs->signal[J_SIG_JOB_CHANGE] = JOB_CHANGE_IDLE;
					myPs->jobChangePhase = P0;
				}
			}
			break;
		case P4:
			flag1 = Read_InPoint(myPs->misc.JigNo+1, 0, I_JIG_CHANGE_LOCK);
			if(flag1 == P0) {
				if((myData->mData.misc.timer_1sec - myPs->misc.jobChangeMoveTimer)
					> myPs->config.cylMoveDelay/1000) {
					myPs->jobChangeEnd = JOB_CHANGE_END;
					myPs->jobChangePhase = P5;
				}
			}else{
				if((myData->mData.misc.timer_1sec - myPs->misc.jobChangeMoveTimer)
					> myPs->config.cylMoveTimeout/1000) {
					myPs->code = J_CD_JOB_CHANGE_ERROR;
					myPs->jigState = J_FAIL;
					myPs->microState = P0;
					Select_OutPoint(myPs->misc.JigNo+1, 0, O_JIG_CHANGE_LOCK, OFF);
					myPs->signal[J_SIG_JOB_CHANGE] = JOB_CHANGE_IDLE;
					myPs->jobChangePhase = P0;
				}
			}
			break;
		case P5:
			if(myPs->pitchChangeEnd == PITCH_CHANGE_END) {
				memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
				SendMsg.msg = MSG_JIGC_COB_JOB_CHANGE_END;
				SendMsg.val[0] = (int)myPs->signal[J_SIG_JOB_CHANGE];
				send_msg(JIGC_TO_COB1, (char *)&SendMsg);
				myPs->jobChangePhase = P6;
			}
			break;
		case P6:
			break;
		default: break;
	}
}

void jobChange_5(void)
{
	int flag1, flag2, flag3;
	S_MSG_VAL SendMsg;
	
	if(myPs->config.jigUseFlag == UNUSE) {
		return;
	}

	if(myData->dio.misc.delayTimer < myData->dio.config.dioDelay) return;
	if(myPs->jigNotMove == ON) return;
	if(myPs->jobChangeEnd != JOB_CHANGE_START) return;

	switch(myPs->jobChangePhase) {
		case P0:
			Select_OutPoint(myPs->misc.JigNo+1, 0, O_JIG_CHANGE_LOCK, ON);
			myPs->misc.jobChangeMoveTimer = myData->mData.misc.timer_1sec;
			myPs->jobChangePhase = P1;
			break;
		case P1:
			flag1 = Read_InPoint(myPs->misc.JigNo+1, 0, I_JIG_CHANGE_LOCK);
			if(flag1 == P1) {
				if((myData->mData.misc.timer_1sec - myPs->misc.jobChangeMoveTimer)
					> myPs->config.cylMoveDelay/1000) {
					myPs->jobChangePhase = P2;
				}
			} else {
				if((myData->mData.misc.timer_1sec - myPs->misc.jobChangeMoveTimer)
					> myPs->config.cylMoveTimeout/1000) {
					myPs->code = J_CD_JOB_CHANGE_ERROR;
					myPs->jigState = J_FAIL;
					myPs->microState = P0;
					Select_OutPoint(myPs->misc.JigNo+1, 0, O_JIG_CHANGE_LOCK, OFF);
					myPs->signal[J_SIG_JOB_CHANGE] = JOB_CHANGE_IDLE;
					myPs->jobChangePhase = P0;
				}
			}
			break;
		case P2:
			Select_OutPoint(myPs->misc.JigNo+1, 1, O_JIG_CHANGE_X, ON); //push
			Select_OutPoint(myPs->misc.JigNo+1, 2, O_JIG_CHANGE_X, OFF); //pull
			Select_OutPoint(myPs->misc.JigNo+1, 1, O_JIG_CHANGE_Y, ON); //push
			Select_OutPoint(myPs->misc.JigNo+1, 2, O_JIG_CHANGE_Y, OFF); //pull
			Select_OutPoint(myPs->misc.JigNo+1, 1, O_JIG_CHANGE_Z, OFF); //push
			Select_OutPoint(myPs->misc.JigNo+1, 2, O_JIG_CHANGE_Z, ON); //pull
			myPs->misc.jobChangeMoveTimer = myData->mData.misc.timer_1sec;
			myPs->jobChangePhase = P3;
			break;
		case P3:
			flag1 = Read_InPoint(myPs->misc.JigNo+1, 0, I_JIG_CHANGE_X);
			flag2 = Read_InPoint(myPs->misc.JigNo+1, 0, I_JIG_CHANGE_Y);
			flag3 = Read_InPoint(myPs->misc.JigNo+1, 0, I_JIG_CHANGE_Z);
			if(flag1 == P1 && flag2 == P1 && flag3 == P0) {
				if((myData->mData.misc.timer_1sec - myPs->misc.jobChangeMoveTimer)
					> myPs->config.cylMoveDelay/1000) {
						Select_OutPoint(myPs->misc.JigNo+1, 0, O_JIG_CHANGE_LOCK, OFF);
						myPs->jobChangePhase = P4;
				}
			} else {
				if((myData->mData.misc.timer_1sec - myPs->misc.jobChangeMoveTimer)
					> myPs->config.cylMoveTimeout/1000) {
					myPs->code = J_CD_JOB_CHANGE_ERROR;
					myPs->jigState = J_FAIL;
					myPs->microState = P0;
//					Select_OutPoint(myPs->misc.JigNo+1, 0, O_JIG_CHANGE_LOCK, OFF);
					myPs->signal[J_SIG_JOB_CHANGE] = JOB_CHANGE_IDLE;
					myPs->jobChangePhase = P0;
				}
			}
			break;
		case P4:
			flag1 = Read_InPoint(myPs->misc.JigNo+1, 0, I_JIG_CHANGE_LOCK);
			if(flag1 == P0) {
				if((myData->mData.misc.timer_1sec - myPs->misc.jobChangeMoveTimer)
					> myPs->config.cylMoveDelay/1000) {
					myPs->jobChangeEnd = JOB_CHANGE_END;
					myPs->jobChangePhase = P5;
				}
			}else{
				if((myData->mData.misc.timer_1sec - myPs->misc.jobChangeMoveTimer)
					> myPs->config.cylMoveTimeout/1000) {
					myPs->code = J_CD_JOB_CHANGE_ERROR;
					myPs->jigState = J_FAIL;
					myPs->microState = P0;
					Select_OutPoint(myPs->misc.JigNo+1, 0, O_JIG_CHANGE_LOCK, OFF);
					myPs->signal[J_SIG_JOB_CHANGE] = JOB_CHANGE_IDLE;
					myPs->jobChangePhase = P0;
				}
			}
			break;
		case P5:
			if(myPs->pitchChangeEnd == PITCH_CHANGE_END) {
				memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
				SendMsg.msg = MSG_JIGC_COB_JOB_CHANGE_END;
				SendMsg.val[0] = (int)myPs->signal[J_SIG_JOB_CHANGE];
				send_msg(JIGC_TO_COB1, (char *)&SendMsg);
				myPs->jobChangePhase = P6;
			}
			break;
		case P6:
			break;
		default: break;
	}
}

void jobChange_6(void)
{
	int flag1, flag2, flag3;
	S_MSG_VAL SendMsg;
	
	if(myPs->config.jigUseFlag == UNUSE) {
		return;
	}

	if(myData->dio.misc.delayTimer < myData->dio.config.dioDelay) return;
	if(myPs->jigNotMove == ON) return;
	if(myPs->jobChangeEnd != JOB_CHANGE_START) return;

	switch(myPs->jobChangePhase) {
		case P0:
			Select_OutPoint(myPs->misc.JigNo+1, 0, O_JIG_CHANGE_LOCK, ON);
			myPs->misc.jobChangeMoveTimer = myData->mData.misc.timer_1sec;
			myPs->jobChangePhase = P1;
			break;
		case P1:
			flag1 = Read_InPoint(myPs->misc.JigNo+1, 0, I_JIG_CHANGE_LOCK);
			if(flag1 == P1) {
				if((myData->mData.misc.timer_1sec - myPs->misc.jobChangeMoveTimer)
					> myPs->config.cylMoveDelay/1000) {
					myPs->jobChangePhase = P2;
				}
			} else {
				if((myData->mData.misc.timer_1sec - myPs->misc.jobChangeMoveTimer)
					> myPs->config.cylMoveTimeout/1000) {
					myPs->code = J_CD_JOB_CHANGE_ERROR;
					myPs->jigState = J_FAIL;
					myPs->microState = P0;
					Select_OutPoint(myPs->misc.JigNo+1, 0, O_JIG_CHANGE_LOCK, OFF);
					myPs->signal[J_SIG_JOB_CHANGE] = JOB_CHANGE_IDLE;
					myPs->jobChangePhase = P0;
				}
			}
			break;
		case P2:
			Select_OutPoint(myPs->misc.JigNo+1, 1, O_JIG_CHANGE_X, ON); //push
			Select_OutPoint(myPs->misc.JigNo+1, 2, O_JIG_CHANGE_X, OFF); //pull
			Select_OutPoint(myPs->misc.JigNo+1, 1, O_JIG_CHANGE_Y, ON); //push
			Select_OutPoint(myPs->misc.JigNo+1, 2, O_JIG_CHANGE_Y, OFF); //pull
			Select_OutPoint(myPs->misc.JigNo+1, 1, O_JIG_CHANGE_Z, ON); //push
			Select_OutPoint(myPs->misc.JigNo+1, 2, O_JIG_CHANGE_Z, OFF); //pull
			myPs->misc.jobChangeMoveTimer = myData->mData.misc.timer_1sec;
			myPs->jobChangePhase = P3;
			break;
		case P3:
			flag1 = Read_InPoint(myPs->misc.JigNo+1, 0, I_JIG_CHANGE_X);
			flag2 = Read_InPoint(myPs->misc.JigNo+1, 0, I_JIG_CHANGE_Y);
			flag3 = Read_InPoint(myPs->misc.JigNo+1, 0, I_JIG_CHANGE_Z);
			if(flag1 == P1 && flag2 == P1 && flag3 == P1) {
				if((myData->mData.misc.timer_1sec - myPs->misc.jobChangeMoveTimer)
					> myPs->config.cylMoveDelay/1000) {
						Select_OutPoint(myPs->misc.JigNo+1, 0, O_JIG_CHANGE_LOCK, OFF);
						myPs->jobChangePhase = P4;
				}
			} else {
				if((myData->mData.misc.timer_1sec - myPs->misc.jobChangeMoveTimer)
					> myPs->config.cylMoveTimeout/1000) {
					myPs->code = J_CD_JOB_CHANGE_ERROR;
					myPs->jigState = J_FAIL;
					myPs->microState = P0;
//					Select_OutPoint(myPs->misc.JigNo+1, 0, O_JIG_CHANGE_LOCK, OFF);
					myPs->signal[J_SIG_JOB_CHANGE] = JOB_CHANGE_IDLE;
					myPs->jobChangePhase = P0;
				}
			}
			break;
		case P4:
			flag1 = Read_InPoint(myPs->misc.JigNo+1, 0, I_JIG_CHANGE_LOCK);
			if(flag1 == P0) {
				if((myData->mData.misc.timer_1sec - myPs->misc.jobChangeMoveTimer)
					> myPs->config.cylMoveDelay/1000) {
					myPs->jobChangeEnd = JOB_CHANGE_END;
					myPs->jobChangePhase = P5;
				}
			}else{
				if((myData->mData.misc.timer_1sec - myPs->misc.jobChangeMoveTimer)
					> myPs->config.cylMoveTimeout/1000) {
					myPs->code = J_CD_JOB_CHANGE_ERROR;
					myPs->jigState = J_FAIL;
					Select_OutPoint(myPs->misc.JigNo+1, 0, O_JIG_CHANGE_LOCK, OFF);
					myPs->microState = P0;
					myPs->signal[J_SIG_JOB_CHANGE] = JOB_CHANGE_IDLE;
					myPs->jobChangePhase = P0;
				}
			}
			break;
		case P5:
			if(myPs->pitchChangeEnd == PITCH_CHANGE_END) {
				memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
				SendMsg.msg = MSG_JIGC_COB_JOB_CHANGE_END;
				SendMsg.val[0] = (int)myPs->signal[J_SIG_JOB_CHANGE];
				send_msg(JIGC_TO_COB1, (char *)&SendMsg);
				myPs->jobChangePhase = P6;
			}
			break;
		case P6:
			break;
		default: break;
	}
}

void jobChange_7(void)
{
	int flag1, flag2, flag3;
	S_MSG_VAL SendMsg;
	
	if(myPs->config.jigUseFlag == UNUSE) {
		return;
	}

	if(myData->dio.misc.delayTimer < myData->dio.config.dioDelay) return;
	if(myPs->jigNotMove == ON) return;
	if(myPs->jobChangeEnd != JOB_CHANGE_START) return;

	switch(myPs->jobChangePhase) {
		case P0:
			Select_OutPoint(myPs->misc.JigNo+1, 0, O_JIG_CHANGE_LOCK, ON);
			myPs->misc.jobChangeMoveTimer = myData->mData.misc.timer_1sec;
			myPs->jobChangePhase = P1;
			break;
		case P1:
			flag1 = Read_InPoint(myPs->misc.JigNo+1, 0, I_JIG_CHANGE_LOCK);
			if(flag1 == P1) {
				if((myData->mData.misc.timer_1sec - myPs->misc.jobChangeMoveTimer)
					> myPs->config.cylMoveDelay/1000) {
					myPs->jobChangePhase = P2;
				}
			} else {
				if((myData->mData.misc.timer_1sec - myPs->misc.jobChangeMoveTimer)
					> myPs->config.cylMoveTimeout/1000) {
					myPs->code = J_CD_JOB_CHANGE_ERROR;
					myPs->jigState = J_FAIL;
					myPs->microState = P0;
					Select_OutPoint(myPs->misc.JigNo+1, 0, O_JIG_CHANGE_LOCK, OFF);
					myPs->signal[J_SIG_JOB_CHANGE] = JOB_CHANGE_IDLE;
					myPs->jobChangePhase = P0;
				}
			}
			break;
		case P2:
			Select_OutPoint(myPs->misc.JigNo+1, 1, O_JIG_CHANGE_X, OFF); //push
			Select_OutPoint(myPs->misc.JigNo+1, 2, O_JIG_CHANGE_X, ON); //pull
			Select_OutPoint(myPs->misc.JigNo+1, 1, O_JIG_CHANGE_Y, ON); //push
			Select_OutPoint(myPs->misc.JigNo+1, 2, O_JIG_CHANGE_Y, OFF); //pull
			Select_OutPoint(myPs->misc.JigNo+1, 1, O_JIG_CHANGE_Z, OFF); //push
			Select_OutPoint(myPs->misc.JigNo+1, 2, O_JIG_CHANGE_Z, ON); //pull

			myPs->misc.jobChangeMoveTimer = myData->mData.misc.timer_1sec;
			myPs->jobChangePhase = P3;
			break;
		case P3:
			flag1 = Read_InPoint(myPs->misc.JigNo+1, 0, I_JIG_CHANGE_X);
			flag2 = Read_InPoint(myPs->misc.JigNo+1, 0, I_JIG_CHANGE_Y);
			flag3 = Read_InPoint(myPs->misc.JigNo+1, 0, I_JIG_CHANGE_Z);
			if(flag1 == P0 && flag2 == P1 && flag3 == P0) {
				if((myData->mData.misc.timer_1sec - myPs->misc.jobChangeMoveTimer)
					> myPs->config.cylMoveDelay/1000) {
						Select_OutPoint(myPs->misc.JigNo+1, 0, O_JIG_CHANGE_LOCK, OFF);
						myPs->jobChangePhase = P4;
				}
			} else {
				if((myData->mData.misc.timer_1sec - myPs->misc.jobChangeMoveTimer)
					> myPs->config.cylMoveTimeout/1000) {
					myPs->code = J_CD_JOB_CHANGE_ERROR;
					myPs->jigState = J_FAIL;
//					Select_OutPoint(myPs->misc.JigNo+1, 0, O_JIG_CHANGE_LOCK, OFF);
					myPs->microState = P0;
					myPs->signal[J_SIG_JOB_CHANGE] = JOB_CHANGE_IDLE;
					myPs->jobChangePhase = P0;
				}
			}
			break;
		case P4:
			flag1 = Read_InPoint(myPs->misc.JigNo+1, 0, I_JIG_CHANGE_LOCK);
			if(flag1 == P0) {
				if((myData->mData.misc.timer_1sec - myPs->misc.jobChangeMoveTimer)
					> myPs->config.cylMoveDelay/1000) {
					myPs->jobChangeEnd = JOB_CHANGE_END;
					myPs->jobChangePhase = P5;
				}
			}else{
				if((myData->mData.misc.timer_1sec - myPs->misc.jobChangeMoveTimer)
					> myPs->config.cylMoveTimeout/1000) {
					myPs->code = J_CD_JOB_CHANGE_ERROR;
					myPs->jigState = J_FAIL;
					myPs->microState = P0;
					Select_OutPoint(myPs->misc.JigNo+1, 0, O_JIG_CHANGE_LOCK, OFF);
					myPs->signal[J_SIG_JOB_CHANGE] = JOB_CHANGE_IDLE;
					myPs->jobChangePhase = P0;
				}
			}
			break;
		case P5:
			if(myPs->pitchChangeEnd == PITCH_CHANGE_END) {
				memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
				SendMsg.msg = MSG_JIGC_COB_JOB_CHANGE_END;
				SendMsg.val[0] = (int)myPs->signal[J_SIG_JOB_CHANGE];
				send_msg(JIGC_TO_COB1, (char *)&SendMsg);
				myPs->jobChangePhase = P6;
			}
			break;
		case P6:
			break;
		default: break;
	}
}

void jobChange_8(void)
{
	int flag1, flag2, flag3;
	S_MSG_VAL SendMsg;
	
	if(myPs->config.jigUseFlag == UNUSE) {
		return;
	}

	if(myData->dio.misc.delayTimer < myData->dio.config.dioDelay) return;
	if(myPs->jigNotMove == ON) return;
	if(myPs->jobChangeEnd != JOB_CHANGE_START) return;

	switch(myPs->jobChangePhase) {
		case P0:
			Select_OutPoint(myPs->misc.JigNo+1, 0, O_JIG_CHANGE_LOCK, ON);
			myPs->misc.jobChangeMoveTimer = myData->mData.misc.timer_1sec;
			myPs->jobChangePhase = P1;
			break;
		case P1:
			flag1 = Read_InPoint(myPs->misc.JigNo+1, 0, I_JIG_CHANGE_LOCK);
			if(flag1 == P1) {
				if((myData->mData.misc.timer_1sec - myPs->misc.jobChangeMoveTimer)
					> myPs->config.cylMoveDelay/1000) {
					myPs->jobChangePhase = P2;
				}
			} else {
				if((myData->mData.misc.timer_1sec - myPs->misc.jobChangeMoveTimer)
					> myPs->config.cylMoveTimeout/1000) {
					myPs->code = J_CD_JOB_CHANGE_ERROR;
					myPs->jigState = J_FAIL;
					Select_OutPoint(myPs->misc.JigNo+1, 0, O_JIG_CHANGE_LOCK, OFF);
					myPs->microState = P0;
					myPs->signal[J_SIG_JOB_CHANGE] = JOB_CHANGE_IDLE;
					myPs->jobChangePhase = P0;
				}
			}
			break;
		case P2:
			Select_OutPoint(myPs->misc.JigNo+1, 1, O_JIG_CHANGE_X, OFF); //push
			Select_OutPoint(myPs->misc.JigNo+1, 2, O_JIG_CHANGE_X, ON); //pull
			Select_OutPoint(myPs->misc.JigNo+1, 1, O_JIG_CHANGE_Y, ON); //push
			Select_OutPoint(myPs->misc.JigNo+1, 2, O_JIG_CHANGE_Y, OFF); //pull
			Select_OutPoint(myPs->misc.JigNo+1, 1, O_JIG_CHANGE_Z, ON); //push
			Select_OutPoint(myPs->misc.JigNo+1, 2, O_JIG_CHANGE_Z, OFF); //pull

			myPs->misc.jobChangeMoveTimer = myData->mData.misc.timer_1sec;
			myPs->jobChangePhase = P3;
			break;
		case P3:
			flag1 = Read_InPoint(myPs->misc.JigNo+1, 0, I_JIG_CHANGE_X);
			flag2 = Read_InPoint(myPs->misc.JigNo+1, 0, I_JIG_CHANGE_Y);
			flag3 = Read_InPoint(myPs->misc.JigNo+1, 0, I_JIG_CHANGE_Z);
			if(flag1 == P0 && flag2 == P1 && flag3 == P1) {
				if((myData->mData.misc.timer_1sec - myPs->misc.jobChangeMoveTimer)
					> myPs->config.cylMoveDelay/1000) {
						Select_OutPoint(myPs->misc.JigNo+1, 0, O_JIG_CHANGE_LOCK, OFF);
						myPs->jobChangePhase = P4;
				}
			} else {
				if((myData->mData.misc.timer_1sec - myPs->misc.jobChangeMoveTimer)
					> myPs->config.cylMoveTimeout/1000) {
					myPs->code = J_CD_JOB_CHANGE_ERROR;
					myPs->jigState = J_FAIL;
					myPs->microState = P0;
//					Select_OutPoint(myPs->misc.JigNo+1, 0, O_JIG_CHANGE_LOCK, OFF);
					myPs->signal[J_SIG_JOB_CHANGE] = JOB_CHANGE_IDLE;
					myPs->jobChangePhase = P0;
				}
			}
			break;
		case P4:
			flag1 = Read_InPoint(myPs->misc.JigNo+1, 0, I_JIG_CHANGE_LOCK);
			if(flag1 == P0) {
				if((myData->mData.misc.timer_1sec - myPs->misc.jobChangeMoveTimer)
					> myPs->config.cylMoveDelay/1000) {
					myPs->jobChangeEnd = JOB_CHANGE_END;
					myPs->jobChangePhase = P5;
				}
			}else{
				if((myData->mData.misc.timer_1sec - myPs->misc.jobChangeMoveTimer)
					> myPs->config.cylMoveTimeout/1000) {
					myPs->code = J_CD_JOB_CHANGE_ERROR;
					myPs->jigState = J_FAIL;
					myPs->microState = P0;
					Select_OutPoint(myPs->misc.JigNo+1, 0, O_JIG_CHANGE_LOCK, OFF);
					myPs->signal[J_SIG_JOB_CHANGE] = JOB_CHANGE_IDLE;
					myPs->jobChangePhase = P0;
				}
			}
			break;
		case P5:
			if(myPs->pitchChangeEnd == PITCH_CHANGE_END) {
				memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
				SendMsg.msg = MSG_JIGC_COB_JOB_CHANGE_END;
				SendMsg.val[0] = (int)myPs->signal[J_SIG_JOB_CHANGE];
				send_msg(JIGC_TO_COB1, (char *)&SendMsg);
				myPs->jobChangePhase = P6;
			}
			break;
		case P6:
			break;
		default: break;
	}
}

void jobChange_2(void)
{
	int flag1, flag2, flag3;
	S_MSG_VAL SendMsg;
	
	if(myPs->config.jigUseFlag == UNUSE) {
		return;
	}

	if(myData->dio.misc.delayTimer < myData->dio.config.dioDelay) return;
	if(myPs->jigNotMove == ON) return;
	if(myPs->jobChangeEnd != JOB_CHANGE_START) return;

	switch(myPs->jobChangePhase) {
		case P0:
			Select_OutPoint(myPs->misc.JigNo+1, 0, O_JIG_CHANGE_LOCK, ON);
			myPs->misc.jobChangeMoveTimer = myData->mData.misc.timer_1sec;
			myPs->jobChangePhase = P1;
			break;
		case P1:
			flag1 = Read_InPoint(myPs->misc.JigNo+1, 0, I_JIG_CHANGE_LOCK);
			if(flag1 == P1) {
				if((myData->mData.misc.timer_1sec - myPs->misc.jobChangeMoveTimer)
					> myPs->config.cylMoveDelay/1000) {
					myPs->jobChangePhase = P2;
				}
			} else {
				if((myData->mData.misc.timer_1sec - myPs->misc.jobChangeMoveTimer)
					> myPs->config.cylMoveTimeout/1000) {
					myPs->code = J_CD_JOB_CHANGE_ERROR;
					myPs->jigState = J_FAIL;
					myPs->microState = P0;
					Select_OutPoint(myPs->misc.JigNo+1, 0, O_JIG_CHANGE_LOCK, OFF);
					myPs->signal[J_SIG_JOB_CHANGE] = JOB_CHANGE_IDLE;
					myPs->jobChangePhase = P0;
				}
			}
			break;
		case P2:
			Select_OutPoint(myPs->misc.JigNo+1, 1, O_JIG_CHANGE_X, OFF); //push
			Select_OutPoint(myPs->misc.JigNo+1, 2, O_JIG_CHANGE_X, ON); //pull
			Select_OutPoint(myPs->misc.JigNo+1, 1, O_JIG_CHANGE_Y, OFF); //push
			Select_OutPoint(myPs->misc.JigNo+1, 2, O_JIG_CHANGE_Y, ON); //pull
			Select_OutPoint(myPs->misc.JigNo+1, 1, O_JIG_CHANGE_Z, ON); //push
			Select_OutPoint(myPs->misc.JigNo+1, 2, O_JIG_CHANGE_Z, OFF); //pull

			myPs->misc.jobChangeMoveTimer = myData->mData.misc.timer_1sec;
			myPs->jobChangePhase = P3;
			break;
		case P3:
			flag1 = Read_InPoint(myPs->misc.JigNo+1, 0, I_JIG_CHANGE_X);
			flag2 = Read_InPoint(myPs->misc.JigNo+1, 0, I_JIG_CHANGE_Y);
			flag3 = Read_InPoint(myPs->misc.JigNo+1, 0, I_JIG_CHANGE_Z);
			if(flag1 == P0 && flag2 == P0 && flag3 == P1) {
				if((myData->mData.misc.timer_1sec - myPs->misc.jobChangeMoveTimer)
					> myPs->config.cylMoveDelay/1000) {
						Select_OutPoint(myPs->misc.JigNo+1, 0, O_JIG_CHANGE_LOCK, OFF);
						myPs->jobChangePhase = P4;
				}
			} else {
				if((myData->mData.misc.timer_1sec - myPs->misc.jobChangeMoveTimer)
					> myPs->config.cylMoveTimeout/1000) {
					myPs->code = J_CD_JOB_CHANGE_ERROR;
					myPs->jigState = J_FAIL;
					myPs->microState = P0;
//					Select_OutPoint(myPs->misc.JigNo+1, 0, O_JIG_CHANGE_LOCK, OFF);
					myPs->signal[J_SIG_JOB_CHANGE] = JOB_CHANGE_IDLE;
					myPs->jobChangePhase = P0;
				}
			}
			break;
		case P4:
			flag1 = Read_InPoint(myPs->misc.JigNo+1, 0, I_JIG_CHANGE_LOCK);
			if(flag1 == P0) {
				if((myData->mData.misc.timer_1sec - myPs->misc.jobChangeMoveTimer)
					> myPs->config.cylMoveDelay/1000) {
					myPs->jobChangeEnd = JOB_CHANGE_END;
					myPs->jobChangePhase = P5;
				}
			}else{
				if((myData->mData.misc.timer_1sec - myPs->misc.jobChangeMoveTimer)
					> myPs->config.cylMoveTimeout/1000) {
					myPs->code = J_CD_JOB_CHANGE_ERROR;
					myPs->jigState = J_FAIL;
					myPs->microState = P0;
					Select_OutPoint(myPs->misc.JigNo+1, 0, O_JIG_CHANGE_LOCK, OFF);
					myPs->signal[J_SIG_JOB_CHANGE] = JOB_CHANGE_IDLE;
					myPs->jobChangePhase = P0;
				}
			}
			break;
		case P5:
			if(myPs->pitchChangeEnd == PITCH_CHANGE_END) {
				memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
				SendMsg.msg = MSG_JIGC_COB_JOB_CHANGE_END;
				SendMsg.val[0] = (int)myPs->signal[J_SIG_JOB_CHANGE];
				send_msg(JIGC_TO_COB1, (char *)&SendMsg);
				myPs->jobChangePhase = P6;
			}
			break;
		case P6:
			break;
		default: break;
	}
}

void jobChange_4(void)
{
	int flag1, flag2, flag3;
	S_MSG_VAL SendMsg;
	
	if(myPs->config.jigUseFlag == UNUSE) {
		return;
	}

	if(myData->dio.misc.delayTimer < myData->dio.config.dioDelay) return;
	if(myPs->jigNotMove == ON) return;
	if(myPs->jobChangeEnd != JOB_CHANGE_START) return;

	switch(myPs->jobChangePhase) {
		case P0:
			Select_OutPoint(myPs->misc.JigNo+1, 0, O_JIG_CHANGE_LOCK, ON);
			myPs->misc.jobChangeMoveTimer = myData->mData.misc.timer_1sec;
			myPs->jobChangePhase = P1;
			break;
		case P1:
			flag1 = Read_InPoint(myPs->misc.JigNo+1, 0, I_JIG_CHANGE_LOCK);
			if(flag1 == P1) {
				if((myData->mData.misc.timer_1sec - myPs->misc.jobChangeMoveTimer)
					> myPs->config.cylMoveDelay/1000) {
					myPs->jobChangePhase = P2;
				}
			} else {
				if((myData->mData.misc.timer_1sec - myPs->misc.jobChangeMoveTimer)
					> myPs->config.cylMoveTimeout/1000) {
					myPs->code = J_CD_JOB_CHANGE_ERROR;
					myPs->jigState = J_FAIL;
					myPs->microState = P0;
					Select_OutPoint(myPs->misc.JigNo+1, 0, O_JIG_CHANGE_LOCK, OFF);
					myPs->signal[J_SIG_JOB_CHANGE] = JOB_CHANGE_IDLE;
					myPs->jobChangePhase = P0;
				}
			}
			break;
		case P2:
			Select_OutPoint(myPs->misc.JigNo+1, 1, O_JIG_CHANGE_X, ON); //push
			Select_OutPoint(myPs->misc.JigNo+1, 2, O_JIG_CHANGE_X, OFF); //pull
			Select_OutPoint(myPs->misc.JigNo+1, 1, O_JIG_CHANGE_Y, OFF); //push
			Select_OutPoint(myPs->misc.JigNo+1, 2, O_JIG_CHANGE_Y, ON); //pull
			Select_OutPoint(myPs->misc.JigNo+1, 1, O_JIG_CHANGE_Z, ON); //push
			Select_OutPoint(myPs->misc.JigNo+1, 2, O_JIG_CHANGE_Z, OFF); //pull

			myPs->misc.jobChangeMoveTimer = myData->mData.misc.timer_1sec;
			myPs->jobChangePhase = P3;
			break;
		case P3:
			flag1 = Read_InPoint(myPs->misc.JigNo+1, 0, I_JIG_CHANGE_X);
			flag2 = Read_InPoint(myPs->misc.JigNo+1, 0, I_JIG_CHANGE_Y);
			flag3 = Read_InPoint(myPs->misc.JigNo+1, 0, I_JIG_CHANGE_Z);
			if(flag1 == P1 && flag2 == P0 && flag3 == P1) {
				if((myData->mData.misc.timer_1sec - myPs->misc.jobChangeMoveTimer)
					> myPs->config.cylMoveDelay/1000) {
					Select_OutPoint(myPs->misc.JigNo+1, 0, O_JIG_CHANGE_LOCK, OFF);
				}
				myPs->jobChangePhase = P4;
			} else {
				if((myData->mData.misc.timer_1sec - myPs->misc.jobChangeMoveTimer)
					> myPs->config.cylMoveTimeout/1000) {
					myPs->code = J_CD_JOB_CHANGE_ERROR;
					myPs->jigState = J_FAIL;
					myPs->microState = P0;
//					Select_OutPoint(myPs->misc.JigNo+1, 0, O_JIG_CHANGE_LOCK, OFF);
					myPs->signal[J_SIG_JOB_CHANGE] = JOB_CHANGE_IDLE;
					myPs->jobChangePhase = P0;
				}
			}
			break;
		case P4:
			flag1 = Read_InPoint(myPs->misc.JigNo+1, 0, I_JIG_CHANGE_LOCK);
			if(flag1 == P0) {
				if((myData->mData.misc.timer_1sec - myPs->misc.jobChangeMoveTimer)
					> myPs->config.cylMoveDelay/1000) {
					myPs->jobChangeEnd = JOB_CHANGE_END;
					myPs->jobChangePhase = P5;
				}
			}else{
				if((myData->mData.misc.timer_1sec - myPs->misc.jobChangeMoveTimer)
					> myPs->config.cylMoveTimeout/1000) {
					myPs->code = J_CD_JOB_CHANGE_ERROR;
					myPs->jigState = J_FAIL;
					myPs->microState = P0;
					Select_OutPoint(myPs->misc.JigNo+1, 0, O_JIG_CHANGE_LOCK, OFF);
					myPs->signal[J_SIG_JOB_CHANGE] = JOB_CHANGE_IDLE;
					myPs->jobChangePhase = P0;
				}
			}
			break;
		case P5:
			if(myPs->pitchChangeEnd == PITCH_CHANGE_END) {
				memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
				SendMsg.msg = MSG_JIGC_COB_JOB_CHANGE_END;
				SendMsg.val[0] = (int)myPs->signal[J_SIG_JOB_CHANGE];
				send_msg(JIGC_TO_COB1, (char *)&SendMsg);
				myPs->jobChangePhase = P6;
			}
			break;
		case P6:
			break;
		default: break;
	}
}

