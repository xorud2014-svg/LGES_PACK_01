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
#include "jig_type.h"

extern volatile S_SYSTEM_DATA	*myData;
extern volatile S_JIG_DATA		*myPs;
extern char psName[PROCESS_NAME_SIZE];

void jIdle(void)
{
	switch(myPs->config.jigType) {
		default:
			jIdle_1();	
			break;
	}
}

void jDisContact(void)
{
	switch(myPs->config.jigType) {
		case 1:	jDisContact_1();	break;
		case 2: jDisContact_2();	break;
		case 3: jDisContact_3();	break;
		case 4: jDisContact_4();	break;
		case 5: jDisContact_5();	break;
		case 6: jDisContact_6();	break;
		case 7: jDisContact_7();	break;
		case 8: jDisContact_8();	break;
		case 9: jDisContact_9();	break; //F_SDI_5V_400A_200A_100A_10A
		case 10: jDisContact_10();	break; //F_SDI_5V_400A_200A_100A_10A_2
		case 11: jDisContact_11();	break; //F_SDI_5V_450A_200A_100A_10A
		default:	break;
	}
}

void jTo_Contact(void)
{
	switch(myPs->config.jigType) {
		case 1:	jTo_Contact_1();	break;
		case 2: jTo_Contact_2();	break;
		case 3: jTo_Contact_3();	break;
		case 4: jTo_Contact_4();	break;
		case 5: jTo_Contact_5();	break;
		case 6: jTo_Contact_6();	break;
		case 7: jTo_Contact_7();	break;
		case 8: jTo_Contact_8();	break;
		case 9: jTo_Contact_9();	break;
		case 10: jTo_Contact_10();	break;
		case 11: jTo_Contact_11();	break;
		default:	break;
	}
}

void jContact(void)
{
	switch(myPs->config.jigType) {
		case 1:	jContact_1();	break;
		case 2: jContact_2();	break;
		case 3: jContact_3();	break;
		case 4: jContact_4();	break;
		case 5: jContact_5();	break;
		case 6: jContact_6();	break;
		case 7: jContact_7();	break;
		case 8: jContact_8();	break;
		case 9: jContact_9();	break;
		case 10: jContact_10();	break;
		case 11: jContact_11();	break;
		default:	break;
	}
}

void jTo_DisContact(void)
{
	switch(myPs->config.jigType) {
		case 1:	jTo_DisContact_1();	break;
		case 2: jTo_DisContact_2();	break;
		case 3: jTo_DisContact_3();	break;
		case 4: jTo_DisContact_4();	break;
		case 5: jTo_DisContact_5();	break;
		case 6: jTo_DisContact_6();	break;
		case 7: jTo_DisContact_7();	break;
		case 8: jTo_DisContact_8();	break;
		case 9: jTo_DisContact_9();	break;
		case 10: jTo_DisContact_10();	break;
		case 11: jTo_DisContact_11();	break;
		default:	break;
	}
}

void jFail(void)
{
	switch(myPs->config.jigType) {
		case 1:	jFail_1();	break;
		case 2: jFail_2();	break;
		case 3: jFail_3();	break;
		case 4: jFail_4();	break;
		case 5: jFail_5();	break;
		case 6: jFail_6();	break;
		case 7: jFail_7();	break;
		case 8: jFail_8();	break;
		case 9: jFail_9();	break;
		case 10: jFail_10();	break;
		case 11: jFail_11();	break;
		default:	break;
	}
}

void jEMG(void)
{
	switch(myPs->config.jigType) {
		case 1:	jEMG_1();	break;
		case 2: jEMG_2();	break;
		case 6: jEMG_6();	break;
		case 7: jEMG_7();	break;
		case 8: jEMG_8();	break;
		case 9: jEMG_9();	break;
		case 10: jEMG_10();	break;
		case 11: jEMG_11();	break;
		default:	break;
	}
}

void jTrayCheck(void)
{
	switch(myPs->config.jigType) {
		case 1: jTrayCheck_1(); break;
		case 2: jTrayCheck_2(); break;
		default:	break;
	}
}

void jIdle_1(void)
{
	if(myData->mData.misc.timer_1sec
		>= (myData->dio.config.dioDelay / 1000) + 2) {
		myPs->jigState = J_TO_DISCONTACT;
		myPs->signal[J_SIG_TRY_TRAY_CHECK_COMPLETE] = P0;
		myPs->microState = P0;
		myPs->code = J_CD_NONE;
	}
}

void jDisContact_1(void)
{
	int ch, jigNo;
	S_MSG_VAL SendMsg;

	ch = 0;
	jigNo = myPs->misc.JigNo - 1;

	switch(myPs->microState) {
		case P0:
			if(myPs->config.jigUseFlag == USE) {
				Select_OutPoint(myPs->misc.JigNo, ch, O_JIG_FAN_RELAY, OFF);
			}

			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
			SendMsg.msg = MSG_JIGC_JIGM_DISCONTACT_COMPLETE;
			send_msg(JIGC1_TO_JIGM + jigNo, (char *)&SendMsg);
			myPs->microState = P1;
			break;
		default: break;
	}
}

void jTo_Contact_1(void)
{
	int flag1, flag2, ch;
	
	if(myPs->trayState[0] != T_LOAD) {
		myPs->code = J_CD_TRAY_UNLOAD;
		myPs->jigState = J_FAIL;
		myPs->microState = P0;
		return;
	}

	if(myPs->config.jigUseFlag == UNUSE) {
		myPs->jigState = J_CONTACT;
		myPs->microState = P0;
		return;
	}

	if(myPs->jigNotMove == ON) return;
	
	ch = 0;
					
	switch(myPs->microState) {
		case P0:
			Select_OutPoint(myPs->misc.JigNo, ch,
				O_JIG_LATCH_CYLINDER_ON_OFF, ON);
			myPs->misc.cylMoveTimer = myData->mData.misc.timer_1sec;
			myPs->microState = P1;
			break;
		case P1:
			flag1 = Read_InPoint(myPs->misc.JigNo, ch,
				I_JIG_LATCH_CYLINDER_R_ON);
			flag2 = Read_InPoint(myPs->misc.JigNo, ch,
				I_JIG_LATCH_CYLINDER_L_ON);
			if(flag1 == P1 && flag2 == P1) {
				Select_OutPoint(myPs->misc.JigNo, ch,
					O_JIG_MAIN_CYLINDER_ON_OFF, ON);
				myPs->misc.cylMoveTimer = myData->mData.misc.timer_1sec;
				myPs->microState = P2;
			} else {
				if((myData->mData.misc.timer_1sec - myPs->misc.cylMoveTimer)
					> myPs->config.cylMoveTimeout/1000) {
					if(flag1 == P0 && flag2 == P0) {
						myPs->code = J_CD_LATCH_CYL_BOTH_CLOSE_TIMEOUT;
					} else if(flag1 == P0 && flag2 == P1) {
						myPs->code = J_CD_LATCH_CYL_R_CLOSE_TIMEOUT;
					} else if(flag1 == P1 && flag2 == P0) {
						myPs->code = J_CD_LATCH_CYL_L_CLOSE_TIMEOUT;
					}
					myPs->jigState = J_FAIL;
					myPs->microState = P0;
				}
			}
			break;
		case P2:
			flag1 = Read_InPoint(myPs->misc.JigNo, ch,
				I_JIG_MAIN_CYLINDER_L_ON);
			if(flag1 == P1) {
				myPs->jigState = J_CONTACT;
				myPs->microState = P0;
			} else {
				if((myData->mData.misc.timer_1sec - myPs->misc.cylMoveTimer)
					> myPs->config.cylMoveTimeout/1000) {
					myPs->code = J_CD_MAIN_CYL_DOWN_TIMEOUT;
					myPs->jigState = J_FAIL;
					myPs->microState = P0;
				}
			}
			break;
		default: break;
	}
}

void jContact_1(void)
{
	int ch=0, jigNo;
	S_MSG_VAL SendMsg;

	jigNo = myPs->misc.JigNo - 1;

	switch(myPs->microState) {
		case P0:
			if(myPs->config.jigUseFlag == USE) {
				Select_OutPoint(myPs->misc.JigNo, ch, O_JIG_FAN_RELAY, ON);
			}

			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
			SendMsg.msg = MSG_JIGC_JIGM_CONTACT_COMPLETE;
			send_msg(JIGC1_TO_JIGM + jigNo, (char *)&SendMsg);
			myPs->microState = P1;
			break;
		default: break;
	}
}

void jTo_DisContact_1(void)
{
	int flag1, flag2, ch=0;
	
	if(myPs->config.jigUseFlag == UNUSE) {
		myPs->jigState = J_DISCONTACT;
		myPs->microState = P0;
		return;
	}

	if(myPs->jigNotMove == ON) return;

	switch(myPs->microState) {
		case P0:
			Select_OutPoint(myPs->misc.JigNo, ch,
				O_JIG_MAIN_CYLINDER_ON_OFF, OFF);
			myPs->misc.cylMoveTimer = myData->mData.misc.timer_1sec;
			myPs->signal[J_SIG_TRY_TRAY_CHECK_COMPLETE] = P0;
			myPs->microState = P1;
			break;
		case P1:
			flag1 = Read_InPoint(myPs->misc.JigNo, ch,
				I_JIG_MAIN_CYLINDER_L_OFF);
			if(flag1 == P1) {
				Select_OutPoint(myPs->misc.JigNo, ch,
					O_JIG_LATCH_CYLINDER_ON_OFF, OFF);
				myPs->misc.cylMoveTimer = myData->mData.misc.timer_1sec;
				myPs->microState = P2;
			} else {
				if((myData->mData.misc.timer_1sec - myPs->misc.cylMoveTimer)
					> myPs->config.cylMoveTimeout/1000) {
					myPs->code = J_CD_MAIN_CYL_UP_TIMEOUT;
					myPs->jigState = J_FAIL;
					myPs->microState = P0;
				}
			}
			myPs->microState = P2;
			break;
		case P2:
			flag1 = Read_InPoint(myPs->misc.JigNo, ch,
				I_JIG_LATCH_CYLINDER_L_OFF);
			flag2 = Read_InPoint(myPs->misc.JigNo, ch,
				I_JIG_LATCH_CYLINDER_R_OFF);
			if(flag1 == P1 && flag2 == P1) {
				myPs->jigState = J_DISCONTACT;
				myPs->microState = P0;
			} else {
				if((myData->mData.misc.timer_1sec - myPs->misc.cylMoveTimer)
					> myPs->config.cylMoveTimeout/1000) {
					if(flag1 == P0 && flag2 == P0) {
						myPs->code = J_CD_LATCH_CYL_BOTH_OPEN_TIMEOUT;
					} else if(flag1 == P0 && flag2 == P1) {
						myPs->code = J_CD_LATCH_CYL_R_OPEN_TIMEOUT;
					} else if(flag1 == P1 && flag2 == P0) {
						myPs->code = J_CD_LATCH_CYL_L_OPEN_TIMEOUT;
					}
					myPs->jigState = J_FAIL;
					myPs->microState = P0;
				}
			}
			break;
		default: break;
	}
}

void jFail_1(void)
{
	int flag1, flag2, code, ch=0, jigNo;
	S_MSG_VAL SendMsg;
	
	if(myPs->jigNotMove == ON) return;

	jigNo = myPs->misc.JigNo - 1;

	switch(myPs->microState) {
		case P0:
			code = convert_jig_code(CONVERT_JIG_TO_ORG, (int)myPs->code);

			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
			SendMsg.msg = MSG_JIGC_JIGM_FAIL;
			SendMsg.val[0] = code;
			send_msg(JIGC1_TO_JIGM + jigNo, (char *)&SendMsg);
			myPs->microState = P1;
			break;
		case P1:
			myPs->code = J_CD_NONE;
			if(myPs->config.jigUseFlag == USE) {
				Select_OutPoint(myPs->misc.JigNo, ch,
					O_JIG_MAIN_CYLINDER_ON_OFF, OFF);
			}
			myPs->microState = P2;
			break;
		case P2:
			if(myPs->config.jigUseFlag == USE) {
				flag1 = Read_InPoint(myPs->misc.JigNo, ch,
					I_JIG_MAIN_CYLINDER_L_OFF);
				if(flag1 == P1) {
					Select_OutPoint(myPs->misc.JigNo, ch,
						O_JIG_LATCH_CYLINDER_ON_OFF, OFF);
				}
				myPs->microState = P3;
			}
			break;
		case P3:
			if(myPs->config.jigUseFlag == USE) {
				flag1 = Read_InPoint(myPs->misc.JigNo, ch,
					I_JIG_LATCH_CYLINDER_L_OFF);
				flag2 = Read_InPoint(myPs->misc.JigNo, ch,
					I_JIG_LATCH_CYLINDER_R_OFF);
				if(flag1 == P1 && flag2 == P1) {
					myPs->jigState = J_DISCONTACT;
					myPs->microState = P0;
				} 
			} else {
				myPs->jigState = J_DISCONTACT;
				myPs->microState = P0;
			}
			break;
		default: break;
	}
}

void jEMG_1(void)
{
	int code, jigNo;
	S_MSG_VAL SendMsg;
		
	jigNo = myPs->misc.JigNo - 1;

	switch(myPs->microState) {
		case P0:
			code = convert_jig_code(CONVERT_JIG_TO_ORG, (int)myPs->code);

			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
			SendMsg.msg = MSG_JIGC_IO_EMG;
			SendMsg.val[0] = code;
			SendMsg.val[1] = jigNo;
			send_msg(JIGC1_TO_IO + jigNo, (char *)&SendMsg);
			userlog(DEBUG_LOG, psName,
				"Jig Emg Code[%d] Position[%d]\n", code, myPs->troublePos);
			myPs->microState = P1;
			break;
		case P1:
			myPs->code = J_CD_NONE;
			myPs->microState = P2;
			break;
	}
}

void jTrayCheck_1(void)
{
	int flag1, flag2, ch=0;

	if(myPs->jigNotMove == ON) return;
	
	switch(myPs->microState) {
		case P0:
			myPs->microState = P1;
			break;
		case P1:
			if(myPs->config.jigUseFlag == USE) {
				Select_OutPoint(myPs->misc.JigNo, ch,
					O_JIG_LATCH_CYLINDER_ON_OFF, ON);
				myPs->misc.cylMoveTimer = myData->mData.misc.timer_1sec;
			}
			myPs->microState = P2;
			break;
		case P2:
			if(myPs->config.jigUseFlag == USE) {
				flag1 = Read_InPoint(myPs->misc.JigNo, ch,
					I_JIG_LATCH_CYLINDER_L_ON);
				flag2 = Read_InPoint(myPs->misc.JigNo, ch,
					I_JIG_LATCH_CYLINDER_R_ON);
				if(flag1 == P1 && flag2 == P1) {
					myPs->microState = P3;
				} else {
					if((myData->mData.misc.timer_1sec - myPs->misc.cylMoveTimer)
						> myPs->config.cylMoveTimeout/1000) {
						myPs->microState = P3; //jig fail ignore
					}							   //because group state is not run
				}
			} else {
				myPs->microState = P3;
			}
			break;
		case P3: // latch close delay1
			myPs->microState = P4;
			break;
		case P4: // latch close delay2 (total 2 sec)
			if(myPs->config.jigUseFlag == USE) {
				Select_OutPoint(myPs->misc.JigNo, ch,
					O_JIG_LATCH_CYLINDER_ON_OFF, OFF);
				myPs->misc.cylMoveTimer = myData->mData.misc.timer_1sec;
				myPs->microState = P5;
			} else {
				myPs->microState = P5;
			}	
			break;	
		case P5:
			if(myPs->config.jigUseFlag == USE) {
				flag1 = Read_InPoint(myPs->misc.JigNo, ch,
					I_JIG_LATCH_CYLINDER_L_OFF);
				flag2 = Read_InPoint(myPs->misc.JigNo, ch,
					I_JIG_LATCH_CYLINDER_R_OFF);
				if(flag1 == P1 && flag2 == P1) {
					myPs->jigState = J_DISCONTACT;
					myPs->microState = P0;
					myPs->signal[J_SIG_TRY_TRAY_CHECK_COMPLETE] = P1;
				} else {
					if((myData->mData.misc.timer_1sec - myPs->misc.cylMoveTimer)
						> myPs->config.cylMoveTimeout/1000) {
						myPs->jigState = J_DISCONTACT; //jig fail ignore
						myPs->microState = P0;
					}
				}
			} else {
				myPs->jigState = J_DISCONTACT;
				myPs->microState = P0;
				myPs->signal[J_SIG_TRY_TRAY_CHECK_COMPLETE] = P1;
			}
			break;
		default: break;
	}
}

void jDisContact_2(void)
{
	int flag1, flag2, ch=0, jigNo;
	S_MSG_VAL SendMsg;

	jigNo = myPs->misc.JigNo - 1;

	switch(myPs->microState) {
		case P0:
			if(myPs->config.jigUseFlag == USE) {
				Select_OutPoint(myPs->misc.JigNo, ch, O_JIG_FAN_RELAY, OFF);
			}

			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
			SendMsg.msg = MSG_JIGC_JIGM_DISCONTACT_COMPLETE;
			send_msg(JIGC1_TO_JIGM + jigNo, (char *)&SendMsg);
			myPs->microState = P1;
			break;
		case P1:
			if(myPs->config.jigUseFlag == USE) {
				flag1 = Read_InPoint(myPs->misc.JigNo, ch,
					I_JIG_GRIP_CYLINDER_L_OFF);
				flag2 = Read_InPoint(myPs->misc.JigNo, ch,
					I_JIG_MAIN_CYLINDER_L_OFF);
				if(flag1 != P1) {
					myPs->code = J_CD_GRIP_CYL_OPEN_ERROR;
					myPs->jigState = J_FAIL;
					myPs->microState = P0;
					break;
				} else if(flag2 != P1) {
					myPs->code = J_CD_MAIN_CYL_UP_ERROR;
					myPs->jigState = J_FAIL;
					myPs->microState = P0;
					break;
				}

				flag1 = Read_InPoint(myPs->misc.JigNo, ch,
					I_JIG_LATCH_CYLINDER_L_OFF);
				flag2 = Read_InPoint(myPs->misc.JigNo, ch,
					I_JIG_LATCH_CYLINDER_R_OFF);
				if(flag1 != P1) {
					myPs->code = J_CD_LATCH_CYL_R_OPEN_ERROR;
					myPs->jigState = J_FAIL;
					myPs->microState = P0;
					break;
				} else if(flag2 != P1) {
					myPs->code = J_CD_LATCH_CYL_L_OPEN_ERROR;
					myPs->jigState = J_FAIL;
					myPs->microState = P0;
					break;
				}
			}
			break;
		default: break;
	}
}

void jTo_Contact_2(void)
{
	int flag1, flag2, ch=0;
	
	if(myPs->trayState[0] != T_LOAD) {
		myPs->code = J_CD_TRAY_UNLOAD;
		myPs->jigState = J_FAIL;
		myPs->microState = P0;
		return;
	}

	if(myPs->config.jigUseFlag == UNUSE) {
		myPs->jigState = J_CONTACT;
		myPs->microState = P0;
		return;
	}

	if(myPs->jigNotMove == ON) return;

	switch(myPs->microState) {
		case P0:
			Select_OutPoint(myPs->misc.JigNo, ch,
				O_JIG_LATCH_CYLINDER_ON_OFF, ON);
			myPs->misc.cylMoveTimer = myData->mData.misc.timer_1sec;
			myPs->microState = P1;
			break;
		case P1:
			flag1 = Read_InPoint(myPs->misc.JigNo, ch,
				I_JIG_LATCH_CYLINDER_L_ON);
			flag2 = Read_InPoint(myPs->misc.JigNo, ch,
				I_JIG_LATCH_CYLINDER_R_ON);
			if(flag1 == P1 && flag2 == P1) {
				Select_OutPoint(myPs->misc.JigNo, ch,
					O_JIG_MAIN_CYLINDER_ON_OFF, ON);
				myPs->misc.cylMoveTimer = myData->mData.misc.timer_1sec;
				myPs->microState = P2;
			} else {
				if((myData->mData.misc.timer_1sec - myPs->misc.cylMoveTimer)
					> myPs->config.cylMoveTimeout/1000) {
					if(flag1 == P0 && flag2 == P0) {
						myPs->code = J_CD_LATCH_CYL_BOTH_CLOSE_TIMEOUT;
					} else if(flag1 == P0 && flag2 == P1) {
						myPs->code = J_CD_LATCH_CYL_R_CLOSE_TIMEOUT;
					} else if(flag1 == P1 && flag2 == P0) {
						myPs->code = J_CD_LATCH_CYL_L_CLOSE_TIMEOUT;
					}
					myPs->jigState = J_FAIL;
					myPs->microState = P0;
				}
			}
			break;
		case P2:
			flag1 = Read_InPoint(myPs->misc.JigNo, ch,
				I_JIG_MAIN_CYLINDER_L_ON);
			if(flag1 == P1) {
				myPs->misc.cylMoveTimer = myData->mData.misc.timer_1sec;
				myPs->microState = P3;
			} else {
				if((myData->mData.misc.timer_1sec - myPs->misc.cylMoveTimer)
					> myPs->config.cylMoveTimeout/1000) {
					myPs->code = J_CD_MAIN_CYL_DOWN_TIMEOUT;
					myPs->jigState = J_FAIL;
					myPs->microState = P0;
				}
			}
			break;
		case P3: //jig down delay1
			if((myData->mData.misc.timer_1sec - myPs->misc.cylMoveTimer)
					> myPs->config.cylMoveDelay/1000) {
				myPs->microState = P4;
			}
			break;
		case P4: //jig down delay2 (total 2sec)
			Select_OutPoint(myPs->misc.JigNo, ch,
				O_JIG_GRIP_CYLINDER_ON_OFF, ON);
			myPs->misc.cylMoveTimer = myData->mData.misc.timer_1sec;
			myPs->microState = P5;
			break;
		case P5:
			flag1 = Read_InPoint(myPs->misc.JigNo, ch,
				I_JIG_GRIP_CYLINDER_L_ON);
			if(flag1 == P1) {
				if((myData->mData.misc.timer_1sec - myPs->misc.cylMoveTimer)
						> myPs->config.cylMoveDelay/1000) {
					myPs->jigState = J_CONTACT;
					myPs->microState = P0;
				}
			} else {
				if((myData->mData.misc.timer_1sec - myPs->misc.cylMoveTimer)
					> myPs->config.cylMoveTimeout/1000) {
					myPs->code = J_CD_GRIP_CYL_CLOSE_TIMEOUT;
					myPs->jigState = J_FAIL;
					myPs->microState = P0;
				}
			}
			break;
		default: break;
	}
}

void jContact_2(void)
{
	int flag1, flag2, ch=0, jigNo;
	S_MSG_VAL SendMsg;

	jigNo = myPs->misc.JigNo - 1;

	switch(myPs->microState) {
		case P0:
			if(myPs->config.jigUseFlag == USE) {
				Select_OutPoint(myPs->misc.JigNo, ch, O_JIG_FAN_RELAY, ON);
			}

			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
			SendMsg.msg = MSG_JIGC_JIGM_CONTACT_COMPLETE;
			send_msg(JIGC1_TO_JIGM + jigNo, (char *)&SendMsg);
			myPs->microState = P1;
			break;
		case P1:
			if(myPs->config.jigUseFlag == USE) {
				flag1 = Read_InPoint(myPs->misc.JigNo, ch,
					I_JIG_LATCH_CYLINDER_L_ON);
				flag2 = Read_InPoint(myPs->misc.JigNo, ch,
					I_JIG_LATCH_CYLINDER_R_ON);
				if(flag1 != P1) {
					myPs->code = J_CD_LATCH_CYL_R_CLOSE_ERROR;
					myPs->jigState = J_FAIL;
					myPs->microState = P0;
					break;
				} else if(flag2 != P1) {
					myPs->code = J_CD_LATCH_CYL_L_CLOSE_ERROR;
					myPs->jigState = J_FAIL;
					myPs->microState = P0;
					break;
				}

				flag1 = Read_InPoint(myPs->misc.JigNo, ch,
					I_JIG_MAIN_CYLINDER_L_ON);
				flag2 = Read_InPoint(myPs->misc.JigNo, ch,
					I_JIG_GRIP_CYLINDER_L_ON);
				if(flag1 != P1) {
					myPs->code = J_CD_MAIN_CYL_DOWN_ERROR;
					myPs->jigState = J_FAIL;
					myPs->microState = P0;
					break;
				} else if(flag2 != P1) {
					myPs->code = J_CD_GRIP_CYL_CLOSE_ERROR;
					myPs->jigState = J_FAIL;
					myPs->microState = P0;
					break;
				}
			}
			break;
		default: break;
	}
}

void jTo_DisContact_2(void)
{
	int flag1, flag2, ch=0;
	
	if(myPs->config.jigUseFlag == UNUSE) {
		myPs->jigState = J_DISCONTACT;
		myPs->microState = P0;
		return;
	}

	if(myPs->jigNotMove == ON) return;

	switch(myPs->microState) {
		case P0:
			Select_OutPoint(myPs->misc.JigNo, ch,
				O_JIG_GRIP_CYLINDER_ON_OFF, OFF);
			myPs->misc.cylMoveTimer = myData->mData.misc.timer_1sec;
			myPs->microState = P1;
			break;
		case P1:
			flag1 = Read_InPoint(myPs->misc.JigNo, ch,
				I_JIG_GRIP_CYLINDER_L_OFF);
			if(flag1 == P1) {
				myPs->microState = P2;
			} else {
				if((myData->mData.misc.timer_1sec - myPs->misc.cylMoveTimer)
					> myPs->config.cylMoveTimeout/1000) {
					myPs->code = J_CD_GRIP_CYL_OPEN_TIMEOUT;
					myPs->jigState = J_FAIL;
					myPs->microState = P0;
				}
			}
			break;
		case P2: // clamp(grip cyl) open delay1
			myPs->microState = P3;
			break;
		case P3: // clamp(grip cyl) open delay2
			Select_OutPoint(myPs->misc.JigNo, ch,
				O_JIG_MAIN_CYLINDER_ON_OFF, OFF);
			myPs->misc.cylMoveTimer = myData->mData.misc.timer_1sec;
			myPs->microState = P4;
			break;
		case P4:
			flag1 = Read_InPoint(myPs->misc.JigNo, ch,
				I_JIG_MAIN_CYLINDER_L_OFF);
			if(flag1 == P1) {
				Select_OutPoint(myPs->misc.JigNo, ch,
					O_JIG_LATCH_CYLINDER_ON_OFF, OFF);
				myPs->misc.cylMoveTimer = myData->mData.misc.timer_1sec;
				myPs->microState = P5;
			} else {
				if((myData->mData.misc.timer_1sec - myPs->misc.cylMoveTimer)
					> myPs->config.cylMoveTimeout/1000) {
					myPs->code = J_CD_MAIN_CYL_UP_TIMEOUT;
					myPs->jigState = J_FAIL;
					myPs->microState = P0;
				}
			}
			break;
		case P5:
			flag1 = Read_InPoint(myPs->misc.JigNo, ch,
				I_JIG_LATCH_CYLINDER_L_OFF);
			flag2 = Read_InPoint(myPs->misc.JigNo, ch,
				I_JIG_LATCH_CYLINDER_R_OFF);
			if(flag1 == P1 && flag2 == P1) {
				myPs->jigState = J_DISCONTACT;
				myPs->microState = P0;
			} else {
				if((myData->mData.misc.timer_1sec - myPs->misc.cylMoveTimer)
					> myPs->config.cylMoveTimeout/1000) {
					if(flag1 == P0 && flag2 == P0) {
						myPs->code = J_CD_LATCH_CYL_BOTH_OPEN_TIMEOUT;
					} else if(flag1 == P0 && flag2 == P1) {
						myPs->code = J_CD_LATCH_CYL_R_OPEN_TIMEOUT;
					} else if(flag1 == P1 && flag2 == P0) {
						myPs->code = J_CD_LATCH_CYL_L_OPEN_TIMEOUT;
					}
					myPs->jigState = J_FAIL;
					myPs->microState = P0;
				}
			}
			break;
		default: break;
	}
}

void jFail_2(void)
{
	int flag1, flag2, code, ch=0, jigNo;
	S_MSG_VAL SendMsg;
	
	if(myPs->jigNotMove == ON) return;

	jigNo = myPs->misc.JigNo - 1;

	switch(myPs->microState) {
		case P0:
			code = convert_jig_code(CONVERT_JIG_TO_ORG, (int)myPs->code);

			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
			SendMsg.msg = MSG_JIGC_JIGM_FAIL;
			SendMsg.val[0] = code;
			send_msg(JIGC1_TO_JIGM + jigNo, (char *)&SendMsg);
			myPs->microState = P1;
			break;
		case P1:
			myPs->code = J_CD_NONE;
			if(myPs->config.jigUseFlag == USE) {
				Select_OutPoint(myPs->misc.JigNo, ch,
					O_JIG_GRIP_CYLINDER_ON_OFF, OFF);
			}
			myPs->microState = P2;
			break;
		case P2:
			if(myPs->config.jigUseFlag == USE) {
				flag1 = Read_InPoint(myPs->misc.JigNo, ch,
					I_JIG_GRIP_CYLINDER_L_OFF);
				if(flag1 == P1) 
					myPs->microState = P3;
			} else 
				myPs->microState = P3;
			break;
		case P3: // clamp(grip cyl) open delay1
			myPs->microState = P4;
			break;
		case P4: // clamp(grip cyl) open delay2
			if(myPs->config.jigUseFlag == USE) {
				Select_OutPoint(myPs->misc.JigNo, ch,
					O_JIG_MAIN_CYLINDER_ON_OFF, OFF);
				myPs->microState = P5;
			} else 
				myPs->microState = P5;
			break;
		case P5:
			if(myPs->config.jigUseFlag == USE) {
				flag1 = Read_InPoint(myPs->misc.JigNo, ch,
					I_JIG_MAIN_CYLINDER_L_OFF);
				if(flag1 == P1) {
					Select_OutPoint(myPs->misc.JigNo, ch,
						O_JIG_LATCH_CYLINDER_ON_OFF, OFF);
					myPs->misc.cylMoveTimer = myData->mData.misc.timer_1sec;
					myPs->microState = P6;
				}
			} else 
				myPs->microState = P6;
			break;
		case P6:
			if(myPs->config.jigUseFlag == USE) {
				flag1 = Read_InPoint(myPs->misc.JigNo, ch,
					I_JIG_LATCH_CYLINDER_L_OFF);
				flag2 = Read_InPoint(myPs->misc.JigNo, ch,
					I_JIG_LATCH_CYLINDER_R_OFF);
				if(flag1 == P1 && flag2 == P1) {
					myPs->jigState = J_DISCONTACT;
					myPs->microState = P0;
				}
			} else {
				myPs->jigState = J_DISCONTACT;
				myPs->microState = P0;
			}
			break;
		default: break;
	}
}

void jEMG_2(void)
{
	int code, jigNo;
	S_MSG_VAL SendMsg;
		
	jigNo = myPs->misc.JigNo -1;
	switch(myPs->microState) {
		case P0:
			code = convert_jig_code(CONVERT_JIG_TO_ORG, (int)myPs->code);

			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
			SendMsg.msg = MSG_JIGC_IO_EMG;
			SendMsg.val[0] = code;
			SendMsg.val[1] = jigNo;
			send_msg(JIGC1_TO_IO + jigNo, (char *)&SendMsg);
			userlog(DEBUG_LOG, psName,
				"Jig Emg Code[%d] Position[%d]\n", code, myPs->troublePos);
			myPs->microState = P1;
			break;
		case P1:
			myPs->code = J_CD_NONE;
			myPs->microState = P2;
			break;
	}
}

void jTrayCheck_2(void)
{
	int flag1, flag2, diff, ch=0;

	if(myPs->jigNotMove == ON) return;

	diff = myData->mData.misc.timer_1sec - myPs->trayCheckDelay;
	if(diff < 5) return;
	
	switch(myPs->microState) {
		case P0:
			myPs->microState = P1;
			break;
		case P1:
			if(myPs->config.jigUseFlag == USE) {
				Select_OutPoint(myPs->misc.JigNo, ch,
					O_JIG_LATCH_CYLINDER_ON_OFF, ON);
				myPs->misc.cylMoveTimer = myData->mData.misc.timer_1sec;
			}
			myPs->microState = P2;
			break;
		case P2:
			if(myPs->config.jigUseFlag == USE) {
				flag1 = Read_InPoint(myPs->misc.JigNo, ch,
					I_JIG_LATCH_CYLINDER_L_OFF);
				flag2 = Read_InPoint(myPs->misc.JigNo, ch,
					I_JIG_LATCH_CYLINDER_R_OFF);
				if(flag1 == P1 && flag2 == P1) {
					myPs->microState = P3;
				} else { 
					if((myData->mData.misc.timer_1sec - myPs->misc.cylMoveTimer)
						> myPs->config.cylMoveTimeout/1000) {
						myPs->microState = P3; //jig fail ignore
					}
				}
			} else {
				myPs->microState = P3;
			}
			break;
		case P3: // latch close delay1
			myPs->microState = P4;
			break;
		case P4: // latch close delay2 (total 2 sec)
			if(myPs->config.jigUseFlag == USE) {
				Select_OutPoint(myPs->misc.JigNo, ch,
					O_JIG_LATCH_CYLINDER_ON_OFF, OFF);
				myPs->misc.cylMoveTimer = myData->mData.misc.timer_1sec;
				myPs->microState = P5;
			} else {
				myPs->microState = P5;
			}	
			break;	
		case P5:
			if(myPs->config.jigUseFlag == USE) {
				flag1 = Read_InPoint(myPs->misc.JigNo, ch,
					I_JIG_LATCH_CYLINDER_L_OFF);
				flag2 = Read_InPoint(myPs->misc.JigNo, ch,
					I_JIG_LATCH_CYLINDER_R_OFF);
				if(flag1 == P1 && flag2 == P1) {
					myPs->jigState = J_DISCONTACT;
					myPs->microState = P0;
					myPs->signal[J_SIG_TRY_TRAY_CHECK_COMPLETE] = P1;
				} else {
					if((myData->mData.misc.timer_1sec - myPs->misc.cylMoveTimer)
						> myPs->config.cylMoveTimeout/1000) {
						myPs->jigState = J_DISCONTACT; //jig fail ignore
						myPs->microState = P0;
					}
				}
			} else {
				myPs->jigState = J_DISCONTACT;
				myPs->microState = P0;
				myPs->signal[J_SIG_TRY_TRAY_CHECK_COMPLETE] = P1;
			}
			break;
		default: break;
	}
}

void jDisContact_3(void)
{
	int flag1, ch=0;

	switch(myPs->microState) {
		case P0:
/*			send_msg(JIGC_TO_JIGM, MSG_JIGC_JIGM_DISCONTACT_COMPLETE, 0, 0);
			myPs->microState = P1;
			break;
		case P1:*/
			if(myPs->config.jigUseFlag == USE) {
				flag1 = Read_InPoint(myPs->misc.JigNo, ch, I_JIG_AIR_PRESS);
				if(flag1 == P0) {
					myPs->code = J_CD_AIR_PRESS_ERROR;
					myPs->jigState = J_FAIL;
					myPs->microState = P10;
					break;
				}
				flag1 = Read_InPoint(myPs->misc.JigNo, ch, I_JIG_SW_DOWN);
				if(flag1 == P1) {
					myPs->jigState = J_TO_CONTACT;
					myPs->microState = P0;
					myPs->code = J_CD_NONE;
				}
			}
			break;
		default: break;
	}
}

void jTo_Contact_3(void)
{
	int flag1, flag2, flag3, flag4, ch=0;
	
	if(myPs->config.jigUseFlag == UNUSE) {
		myPs->jigState = J_CONTACT;
		myPs->microState = P0;
		return;
	}

/*	if(myPs->trayState[0] != T_LOAD) {
		myPs->code = J_CD_TRAY_UNLOAD;
		myPs->jigState = J_FAIL;
		myPs->microState = P0;
		return;
	}*/

	switch(myPs->microState) {
		case P0:
			Select_OutPoint(myPs->misc.JigNo, ch, O_JIG_MAIN_CYLINDER_OFF, OFF);
			Select_OutPoint(myPs->misc.JigNo, ch, O_JIG_MAIN_CYLINDER_ON, ON);
			myPs->misc.cylMoveTimer = myData->mData.misc.timer_1sec;
			if(myPs->trayState[0] == T_LOAD) {
				myPs->microState = P1;
			} else {
				myPs->jigState = J_CONTACT;
				myPs->microState = P0;
			}
			flag1 = Read_InPoint(myPs->misc.JigNo, ch, I_JIG_SW_UP);
			if(flag1 == P1) {
				myPs->jigState = J_TO_DISCONTACT;
				myPs->microState = P0;
				myPs->code = J_CD_NONE;
			}
			break;
		case P1:
			flag1 = Read_InPoint(myPs->misc.JigNo, ch, I_JIG_AIR_PRESS);
			if(flag1 == P0) {
				myPs->code = J_CD_AIR_PRESS_ERROR;
				myPs->jigState = J_FAIL;
				myPs->microState = P10;
				break;
			}
			flag1 = Read_InPoint(myPs->misc.JigNo, 1, I_JIG_MAIN_CYLINDER_L_ON);
			flag2 = Read_InPoint(myPs->misc.JigNo, 2, I_JIG_MAIN_CYLINDER_L_ON);
			flag3 = Read_InPoint(myPs->misc.JigNo, 3, I_JIG_MAIN_CYLINDER_L_ON);
			flag4 = Read_InPoint(myPs->misc.JigNo, 4, I_JIG_MAIN_CYLINDER_L_ON);
			if(flag1 == P1 || flag2 == P1 || flag3 == P1 || flag4 == P1) {
				myPs->jigState = J_CONTACT;
				myPs->microState = P0;
			} else {
				if((myData->mData.misc.timer_1sec - myPs->misc.cylMoveTimer)
					> myPs->config.cylMoveTimeout/1000) {
					myPs->code = J_CD_MAIN_CYL_DOWN_TIMEOUT;
					myPs->jigState = J_FAIL;
					myPs->microState = P0;
				}
			}
			flag1 = Read_InPoint(myPs->misc.JigNo, ch, I_JIG_SW_UP);
			if(flag1 == P1) {
				myPs->jigState = J_TO_DISCONTACT;
				myPs->microState = P0;
				myPs->code = J_CD_NONE;
			}
			break;
		default: break;
	}
}

void jContact_3(void)
{
	int flag1, flag2, flag3, flag4, flag5, ch=0;

	switch(myPs->microState) {
		case P0:
			/*send_msg(JIGC_TO_JIGM, MSG_JIGC_JIGM_CONTACT_COMPLETE, 0, 0);
			myPs->microState = P1;
			if(myPs->config.jigUseFlag == USE) {
				flag1 = Read_InPoint(J3_IN_SW_UP);
				if(flag1 == P1) {
					myPs->jigState = J_TO_DISCONTACT;
					myPs->microState = P0;
					myPs->code = J_CD_NONE;
				}
			}
			break;
		case P1:*/
			if(myPs->config.jigUseFlag == USE) {
				flag1 = Read_InPoint(myPs->misc.JigNo, 1, I_JIG_STOPPER_STATE); 
				flag2 = Read_InPoint(myPs->misc.JigNo, 2, I_JIG_STOPPER_STATE); 
				flag3 = Read_InPoint(myPs->misc.JigNo, 3, I_JIG_STOPPER_STATE); 
				flag4 = Read_InPoint(myPs->misc.JigNo, 4, I_JIG_STOPPER_STATE); 
				flag5 = Read_InPoint(myPs->misc.JigNo, ch, I_JIG_AIR_PRESS); 
				if(flag1 == ON || flag2 == ON || flag3 == ON || flag4 == ON) {
					myPs->code = J_CD_STOPPER_ERROR;
					myPs->jigState = J_FAIL;
					myPs->microState = P0;
					break;
				} else if(flag5 == P0) {
					myPs->code = J_CD_AIR_PRESS_ERROR;
					myPs->jigState = J_FAIL;
					myPs->microState = P0;
					break;
				}
				flag1 = Read_InPoint(myPs->misc.JigNo, ch, I_JIG_SW_UP);
				if(flag1 == P1) {
					myPs->jigState = J_TO_DISCONTACT;
					myPs->microState = P0;
					myPs->code = J_CD_NONE;
				}
			}
			break;
		default: break;
	}
}

void jTo_DisContact_3(void)
{
	int flag1, flag2, ch=0;
	
	if(myPs->config.jigUseFlag == UNUSE) {
		myPs->jigState = J_DISCONTACT;
		myPs->microState = P0;
		return;
	}

	switch(myPs->microState) {
		case P0:
			Select_OutPoint(myPs->misc.JigNo, ch,
				O_JIG_MAIN_CYLINDER_ON, OFF);
			Select_OutPoint(myPs->misc.JigNo, ch,
				O_JIG_MAIN_CYLINDER_OFF, ON);
			myPs->misc.cylMoveTimer = myData->mData.misc.timer_1sec;
			myPs->microState = P1;
			break;
		case P1:
			flag1 = Read_InPoint(myPs->misc.JigNo, ch,
				I_JIG_MAIN_CYLINDER_L_OFF);
			flag2 = Read_InPoint(myPs->misc.JigNo, ch,
				I_JIG_MAIN_CYLINDER_R_OFF);
			if(flag1 == P1 || flag2 == P1) {
				myPs->jigState = J_DISCONTACT;
				myPs->microState = P0;
			} else {
				if((myData->mData.misc.timer_1sec - myPs->misc.cylMoveTimer)
					> myPs->config.cylMoveTimeout/1000) {
					myPs->code = J_CD_MAIN_CYL_UP_TIMEOUT;
					myPs->jigState = J_FAIL;
					myPs->microState = P0;
				}
			}
			break;
		default: break;
	}
}

void jFail_3(void)
{
	int flag1, flag2, code, ch=0, jigNo;
	S_MSG_VAL SendMsg;

	jigNo = myPs->misc.JigNo - 1;
	
	switch(myPs->microState) {
		case P0:
			code = convert_jig_code(CONVERT_JIG_TO_ORG, (int)myPs->code);

			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
			SendMsg.msg = MSG_JIGC_JIGM_FAIL;
			SendMsg.val[0] = code;
			send_msg(JIGC1_TO_JIGM + jigNo, (char *)&SendMsg);
			myPs->microState = P1;
			break;
		case P1:
			myPs->code = J_CD_NONE;
			myPs->misc.cylMoveTimer = myData->mData.misc.timer_1sec;
			if(myPs->config.jigUseFlag == USE) {
				Select_OutPoint(myPs->misc.JigNo, ch,
					O_JIG_MAIN_CYLINDER_ON, OFF);
				Select_OutPoint(myPs->misc.JigNo, ch,
					O_JIG_MAIN_CYLINDER_OFF, ON);
			}
			myPs->microState = P2;
			break;
		case P2:
			if(myPs->config.jigUseFlag == USE) {
				flag1 = Read_InPoint(myPs->misc.JigNo, ch,
					I_JIG_MAIN_CYLINDER_L_OFF);
				flag2 = Read_InPoint(myPs->misc.JigNo, ch,
					I_JIG_MAIN_CYLINDER_R_OFF);
				if(flag1 == P1 || flag2 == P1) {
					//myPs->jigState = J_DISCONTACT;
					myPs->microState = P10;
				} 
			} else {
				myPs->jigState = J_DISCONTACT;
				myPs->microState = P0;
			}
			break;
		case P10:
			flag1 = Read_InPoint(myPs->misc.JigNo, ch, I_JIG_AIR_PRESS);
			if(flag1 == P1) {
				myPs->code = J_CD_NONE;
				myPs->jigState = J_DISCONTACT;
				myPs->microState = P0;
				myPs->misc.cylMoveTimer = myData->mData.misc.timer_1sec;
			}
			break;
		default: break;
	}
}

void jDisContact_4(void)
{
	int flag1, ch=0, jigNo;
	S_MSG_VAL SendMsg;

	jigNo = myPs->misc.JigNo - 1;

	switch(myPs->microState) {
		case P0:
			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
			SendMsg.msg = MSG_JIGC_JIGM_DISCONTACT_COMPLETE;
			send_msg(JIGC1_TO_JIGM + jigNo, (char *)&SendMsg);
			myPs->microState = P1;
			break;
		case P1:
			if(myPs->config.jigUseFlag == USE) {
				flag1 = Read_InPoint(myPs->misc.JigNo, ch, I_JIG_AIR_PRESS);
				if(flag1 == P0) {
					myPs->code = J_CD_AIR_PRESS_ERROR;
					myPs->jigState = J_FAIL;
					myPs->microState = P10;
					break;
				}
			}
			break;
		default: break;
	}
}

void jTo_Contact_4(void)
{
	int flag1, totalTrayNo, tray, i, ch=0;

	totalTrayNo = myPs->config.totalTrayNo;
	tray = 0;
	for(i = 0; i < totalTrayNo; i++)
		if(myPs->trayState[i] == T_LOAD) tray++;
	
	if(tray == 0) {
		myPs->code = J_CD_TRAY_UNLOAD;
		myPs->jigState = J_FAIL;
		myPs->microState = P0;
		return;
	}

	if(myPs->config.jigUseFlag == UNUSE) {
		myPs->jigState = J_CONTACT;
		myPs->microState = P0;
		return;
	}

	switch(myPs->microState) {
		case P0:
			Select_OutPoint(myPs->misc.JigNo, ch, O_JIG_MAIN_CYLINDER_OFF, OFF);
			Select_OutPoint(myPs->misc.JigNo, ch, O_JIG_MAIN_CYLINDER_ON, ON);
			myPs->misc.cylMoveTimer = myData->mData.misc.timer_1sec;
			myPs->misc.contactTime = 0;
			myPs->microState = P1;
			break;
		case P1:
			flag1 = Read_InPoint(myPs->misc.JigNo, ch,
				I_JIG_MAIN_CYLINDER_L_ON);
			if(flag1 == P1) {
				myPs->jigState = J_CONTACT;
				myPs->microState = P0;
				Select_OutPoint(myPs->misc.JigNo, ch,
					O_JIG_MAIN_CYLINDER_ON, OFF);
			} else {
				if((myData->mData.misc.timer_1sec - myPs->misc.cylMoveTimer)
					> myPs->config.cylMoveTimeout/1000) {
					myPs->code = J_CD_MAIN_CYL_DOWN_TIMEOUT;
					myPs->jigState = J_FAIL;
					myPs->microState = P0;
				}
			}
			break;
		default: break;
	}
}

void jContact_4(void)
{
	int flag1, ch=0, jigNo;
	S_MSG_VAL SendMsg;

	jigNo = myPs->misc.JigNo - 1;

	switch(myPs->microState) {
		case P0:
			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
			SendMsg.msg = MSG_JIGC_JIGM_CONTACT_COMPLETE;
			send_msg(JIGC1_TO_JIGM + jigNo, (char *)&SendMsg);
			myPs->microState = P1;
			break;
		case P1:
			if(myPs->config.jigUseFlag == USE) {
				flag1 = Read_InPoint(myPs->misc.JigNo, ch, I_JIG_AIR_PRESS);
//				flag2 = Read_InPoint(J4_IN_STOPPER_SENS);
				if(flag1 == P0) {
					myPs->code = J_CD_AIR_PRESS_ERROR;
					myPs->jigState = J_FAIL;
					myPs->microState = P0;
					break;
				}/* else if(flag2 == P1) {
					myPs->code = J_CD_STOPPER_ERROR;
					myPs->jigState = J_FAIL;
					myPs->microState = P0;
					break;
				}*/
			}
			break;
		default: break;
	}
}

void jTo_DisContact_4(void)
{
	int flag1, ch=0;
	
	if(myPs->config.jigUseFlag == UNUSE) {
		myPs->jigState = J_DISCONTACT;
		myPs->microState = P0;
		return;
	}

	switch(myPs->microState) {
		case P0:
			Select_OutPoint(myPs->misc.JigNo, ch, O_JIG_MAIN_CYLINDER_ON, OFF);
			Select_OutPoint(myPs->misc.JigNo, ch, O_JIG_MAIN_CYLINDER_OFF, ON);
			myPs->misc.cylMoveTimer = myData->mData.misc.timer_1sec;
			myPs->microState = P1;
			break;
		case P1:
			flag1 = Read_InPoint(myPs->misc.JigNo, ch,
				I_JIG_MAIN_CYLINDER_L_ON);
			if(flag1 == P0) {
				myPs->jigState = J_DISCONTACT;
				myPs->microState = P0;
				Select_OutPoint(myPs->misc.JigNo, ch,
					O_JIG_MAIN_CYLINDER_OFF, OFF);
			} else {
				if((myData->mData.misc.timer_1sec - myPs->misc.cylMoveTimer)
					> myPs->config.cylMoveTimeout/1000) {
					myPs->code = J_CD_MAIN_CYL_UP_TIMEOUT;
					myPs->jigState = J_FAIL;
					myPs->microState = P0;
				}
			}
			break;
		default: break;
	}
}

void jFail_4(void)
{
	int flag1, code, ch=0, jigNo;
	S_MSG_VAL SendMsg;

	jigNo = myPs->misc.JigNo - 1;
	
	switch(myPs->microState) {
		case P0:
			code = convert_jig_code(CONVERT_JIG_TO_ORG, (int)myPs->code);

			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
			SendMsg.msg = MSG_JIGC_JIGM_FAIL;
			SendMsg.val[0] = code;
			send_msg(JIGC1_TO_JIGM + jigNo, (char *)&SendMsg);
			myPs->microState = P1;
			break;
		case P1:
			myPs->code = J_CD_NONE;
			if(myPs->config.jigUseFlag == USE) {
				Select_OutPoint(myPs->misc.JigNo, ch,
					O_JIG_MAIN_CYLINDER_ON, OFF);
				Select_OutPoint(myPs->misc.JigNo, ch,
					O_JIG_MAIN_CYLINDER_OFF, ON);
			}
			myPs->microState = P2;
			break;
		case P2:
			if(myPs->config.jigUseFlag == USE) {
				flag1 = Read_InPoint(myPs->misc.JigNo, ch,
					I_JIG_MAIN_CYLINDER_L_ON);
				if(flag1 == P0) {
					myPs->jigState = J_DISCONTACT;
					myPs->microState = P0;
				} 
			} else {
				myPs->jigState = J_DISCONTACT;
				myPs->microState = P0;
			}
			break;
		case P10:
			flag1 = Read_InPoint(myPs->misc.JigNo, ch, I_JIG_AIR_PRESS);
			if(flag1 == P0) {
				myPs->code = J_CD_NONE;
				myPs->jigState = J_DISCONTACT;
				myPs->microState = P1;
			}
			break;
		default: break;
	}
}

void jDisContact_5(void)
{
	int jigNo;
	S_MSG_VAL SendMsg;

	jigNo = myPs->misc.JigNo - 1;

	switch(myPs->microState) {
		case P0:
			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
			SendMsg.msg = MSG_JIGC_JIGM_DISCONTACT_COMPLETE;
			send_msg(JIGC1_TO_JIGM + jigNo, (char *)&SendMsg);
			myPs->microState = P1;
			break;
		case P1:
/*			if(myPs->config.jigUseFlag == USE) {
				flag1 = Read_InPoint(J4_IN_AIR_SW);
				if(flag1 == P0) {
					myPs->code = J_CD_AIR_PRESS_ERROR;
					myPs->jigState = J_FAIL;
					myPs->microState = P10;
					break;
				}
			}
*/			break;
		default: break;
	}
}

void jTo_Contact_5(void)
{
	int flag1, flag2, totalTrayNo, tray, i, ch=0;

	totalTrayNo = myPs->config.totalTrayNo;
	tray = 0;
	for(i = 0; i < totalTrayNo; i++)
		if(myPs->trayState[i] == T_LOAD) tray++;
	
	if(tray == 0) {
		myPs->code = J_CD_TRAY_UNLOAD;
		myPs->jigState = J_FAIL;
		myPs->microState = P0;
		return;
	}

	if(myPs->config.jigUseFlag == UNUSE) {
		myPs->jigState = J_CONTACT;
		myPs->microState = P0;
		return;
	}

	if(myPs->jigNotMove == ON) return;
	
	switch(myPs->microState) {
		case P0:
			Select_OutPoint(myPs->misc.JigNo, ch, O_JIG_MAIN_CYLINDER_OFF, OFF);
			Select_OutPoint(myPs->misc.JigNo, ch, O_JIG_MAIN_CYLINDER_ON, ON);
			myPs->misc.cylMoveTimer = myData->mData.misc.timer_1sec;
			myPs->misc.contactTime = 0;
			myPs->microState = P1;
			break;
		case P1:
			flag1 = Read_InPoint(myPs->misc.JigNo, ch,
				I_JIG_MAIN_CYLINDER_L_ON);
			flag2 = Read_InPoint(myPs->misc.JigNo, ch,
				I_JIG_MAIN_CYLINDER_R_ON);
			if(flag1 == P1 && flag2 == P1) {
				myPs->jigState = J_CONTACT;
				myPs->microState = P0;
				Select_OutPoint(myPs->misc.JigNo, ch, O_JIG_FAN_RELAY, ON);
			} else {
				if((myData->mData.misc.timer_1sec - myPs->misc.cylMoveTimer)
					> myPs->config.cylMoveTimeout/1000) {
					myPs->code = J_CD_GRIP_CYL_CLOSE_TIMEOUT;
					myPs->jigState = J_FAIL;
					myPs->microState = P0;
				}
			}
			break;
		default: break;
	}
}

void jContact_5(void)
{
	int jigNo;
	S_MSG_VAL SendMsg;

	jigNo = myPs->misc.JigNo - 1;

	switch(myPs->microState) {
		case P0:
			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
			SendMsg.msg = MSG_JIGC_JIGM_CONTACT_COMPLETE;
			send_msg(JIGC1_TO_JIGM + jigNo, (char *)&SendMsg);
			myPs->microState = P1;
			break;
		case P1:
/*			if(myPs->config.jigUseFlag == USE) {
				flag1 = Read_InPoint(J5_IN_AIR_SW);
				if(flag1 == P0) {
					myPs->code = J_CD_AIR_PRESS_ERROR;
					myPs->jigState = J_FAIL;
					myPs->microState = P0;
					break;
				}
			}
*/			break;
		default: break;
	}
}

void jTo_DisContact_5(void)
{
	int flag1, flag2, ch=0;
	
	if(myPs->config.jigUseFlag == UNUSE) {
		myPs->jigState = J_DISCONTACT;
		myPs->microState = P0;
		return;
	}

	if(myPs->jigNotMove == ON) return;

	switch(myPs->microState) {
		case P0:
			Select_OutPoint(myPs->misc.JigNo, ch, O_JIG_MAIN_CYLINDER_ON, OFF);
			Select_OutPoint(myPs->misc.JigNo, ch, O_JIG_MAIN_CYLINDER_OFF, ON);
			myPs->misc.cylMoveTimer = myData->mData.misc.timer_1sec;
			myPs->microState = P1;
			break;
		case P1:
			flag1 = Read_InPoint(myPs->misc.JigNo, ch,
				I_JIG_MAIN_CYLINDER_L_OFF);
			flag2 = Read_InPoint(myPs->misc.JigNo, ch,
				I_JIG_MAIN_CYLINDER_R_OFF);
			if(flag1 == P1 && flag2 == P1) {
				myPs->jigState = J_DISCONTACT;
				myPs->microState = P0;
				Select_OutPoint(myPs->misc.JigNo, ch,
					O_JIG_MAIN_CYLINDER_OFF, OFF);
				Select_OutPoint(myPs->misc.JigNo, ch, O_JIG_FAN_RELAY, OFF);
			} else {
				if((myData->mData.misc.timer_1sec - myPs->misc.cylMoveTimer)
					> myPs->config.cylMoveTimeout/1000) {
					myPs->code = J_CD_GRIP_CYL_OPEN_TIMEOUT;
					myPs->jigState = J_FAIL;
					myPs->microState = P0;
				}
			}
			break;
		default: break;
	}
}

void jFail_5(void)
{
	int flag1, flag2, code, ch=0, jigNo;
	S_MSG_VAL SendMsg;
	
	if(myPs->jigNotMove == ON) return;

	jigNo = myPs->misc.JigNo - 1;

	switch(myPs->microState) {
		case P0:
			code = convert_jig_code(CONVERT_JIG_TO_ORG, (int)myPs->code);

			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
			SendMsg.msg = MSG_JIGC_JIGM_FAIL;
			SendMsg.val[0] = code;
			send_msg(JIGC1_TO_JIGM + jigNo, (char *)&SendMsg);
			myPs->microState = P1;
			break;
		case P1:
			myPs->code = J_CD_NONE;
			if(myPs->config.jigUseFlag == USE) {
				Select_OutPoint(myPs->misc.JigNo, ch,
					O_JIG_MAIN_CYLINDER_ON, OFF);
				Select_OutPoint(myPs->misc.JigNo, ch,
					O_JIG_MAIN_CYLINDER_OFF, ON);
			}
			myPs->microState = P2;
			break;

			if(myPs->config.jigUseFlag == USE) {
				flag1 = Read_InPoint(myPs->misc.JigNo, ch,
					I_JIG_MAIN_CYLINDER_L_OFF);
				flag2 = Read_InPoint(myPs->misc.JigNo, ch,
					I_JIG_MAIN_CYLINDER_R_OFF);
				if(flag1 == P1 && flag2 == P1) {
					myPs->jigState = J_DISCONTACT;
					myPs->microState = P0;
				} 
			} else {
				myPs->jigState = J_DISCONTACT;
				myPs->microState = P0;
			}
			break;
		case P10:
/*			flag1 = Read_InPoint(J5_IN_AIR_SW);
			if(flag1 == P0) {
				myPs->code = J_CD_NONE;
				myPs->jigState = J_DISCONTACT;
				myPs->microState = P1;
			}
*/			
			break;
		default: break;
	}
}

void jDisContact_6(void)
{
	int flag1, flag2, ch=0, jigNo;
	S_MSG_VAL SendMsg;

	jigNo = myPs->misc.JigNo - 1;

	switch(myPs->microState) {
		case P0:
			if(myPs->config.jigUseFlag == USE) {
				Select_OutPoint(myPs->misc.JigNo, 1, O_JIG_FAN_RELAY, OFF);
				Select_OutPoint(myPs->misc.JigNo, 2, O_JIG_FAN_RELAY, OFF);
			}

			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
			SendMsg.msg = MSG_JIGC_JIGM_DISCONTACT_COMPLETE;
			send_msg(JIGC1_TO_JIGM + jigNo, (char *)&SendMsg);
			myPs->microState = P1;
			break;
		case P1:
			if(myPs->config.jigUseFlag == USE) {
				flag1 = Read_InPoint(myPs->misc.JigNo, ch,
					I_JIG_MAIN_CYLINDER_L_OFF);
				flag2 = Read_InPoint(myPs->misc.JigNo, ch,
					I_JIG_MAIN_CYLINDER_R_OFF);
				if(flag1 != P1 || flag2 != P1) {
					myPs->code = J_CD_GRIP_CYL_OPEN_ERROR;
					myPs->jigState = J_FAIL;
					myPs->microState = P0;
					break;
				}
			}
			break;
		default: break;
	}
}

void jTo_Contact_6(void)
{
	int flag1, flag2, ch=0;
	
	if(myPs->config.jigUseFlag == UNUSE) {
		myPs->jigState = J_CONTACT;
		myPs->microState = P0;
		return;
	}

	switch(myPs->microState) {
		case P0:
			Select_OutPoint(myPs->misc.JigNo, ch, O_JIG_GRIP_CYLINDER_OFF, OFF);
			Select_OutPoint(myPs->misc.JigNo, ch, O_JIG_GRIP_CYLINDER_ON, ON);
			myPs->misc.cylMoveTimer = myData->mData.misc.timer_1sec;
			myPs->microState = P1;
			break;
		case P1:
			flag1 = Read_InPoint(myPs->misc.JigNo, ch,
				I_JIG_MAIN_CYLINDER_L_ON);
			flag2 = Read_InPoint(myPs->misc.JigNo, ch,
				I_JIG_MAIN_CYLINDER_R_ON);
			if(flag1 == P1 && flag2 == P1) {
				myPs->misc.cylMoveTimer = myData->mData.misc.timer_1sec;
				myPs->microState = P2;
			} else {
				if((myData->mData.misc.timer_1sec - myPs->misc.cylMoveTimer)
					> myPs->config.cylMoveTimeout/1000) {
					myPs->code = J_CD_GRIP_CYL_CLOSE_TIMEOUT;
					myPs->jigState = J_FAIL;
					myPs->microState = P0;
				}
			}
			break;
		case P2:
			if((myData->mData.misc.timer_1sec - myPs->misc.cylMoveTimer)
					> myPs->config.cylMoveDelay/1000) {
				myPs->jigState = J_CONTACT;
				myPs->microState = P0;
			}
			break;
		default: break;
	}
}

void jContact_6(void)
{
	int flag1, flag2, ch=0, jigNo;
	S_MSG_VAL SendMsg;

	jigNo = myPs->misc.JigNo - 1;

	switch(myPs->microState) {
		case P0:
			if(myPs->config.jigUseFlag == USE) {
				Select_OutPoint(myPs->misc.JigNo, 1, O_JIG_FAN_RELAY, ON);
				Select_OutPoint(myPs->misc.JigNo, 2, O_JIG_FAN_RELAY, ON);
			}

			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
			SendMsg.msg = MSG_JIGC_JIGM_CONTACT_COMPLETE;
			send_msg(JIGC1_TO_JIGM + jigNo, (char *)&SendMsg);
			myPs->microState = P1;
			break;
		case P1:
			if(myPs->config.jigUseFlag == USE) {
				flag1 = Read_InPoint(myPs->misc.JigNo, ch,
					I_JIG_MAIN_CYLINDER_L_ON);
				flag2 = Read_InPoint(myPs->misc.JigNo, ch,
					I_JIG_MAIN_CYLINDER_R_ON);
				if(flag1 != P1 || flag2 != P1) {
					myPs->code = J_CD_GRIP_CYL_CLOSE_ERROR;
					myPs->jigState = J_FAIL;
					myPs->microState = P0;
					break;
				}
			}
			break;
		default: break;
	}
}

void jTo_DisContact_6(void)
{
	int flag1, flag2, ch=0;
	
	if(myPs->config.jigUseFlag == UNUSE) {
		myPs->jigState = J_DISCONTACT;
		myPs->microState = P0;
		return;
	}

	switch(myPs->microState) {
		case P0:
			Select_OutPoint(myPs->misc.JigNo, ch, O_JIG_GRIP_CYLINDER_ON, OFF);
			Select_OutPoint(myPs->misc.JigNo, ch, O_JIG_GRIP_CYLINDER_OFF, ON);
			myPs->misc.cylMoveTimer = myData->mData.misc.timer_1sec;
			myPs->microState = P1;
			break;
		case P1:
			flag1 = Read_InPoint(myPs->misc.JigNo, ch,
				I_JIG_MAIN_CYLINDER_L_OFF);
			flag2 = Read_InPoint(myPs->misc.JigNo, ch,
				I_JIG_MAIN_CYLINDER_R_OFF);
			if(flag1 == P1 && flag2 == P1) {
				myPs->jigState = J_DISCONTACT;
				myPs->microState = P0;
			} else {
				if((myData->mData.misc.timer_1sec - myPs->misc.cylMoveTimer)
					> myPs->config.cylMoveTimeout/1000) {
					myPs->code = J_CD_GRIP_CYL_OPEN_TIMEOUT;
					myPs->jigState = J_FAIL;
					myPs->microState = P0;
				}
			}
			break;
		default: break;
	}
}

void jFail_6(void)
{
	int flag1, flag2, code, ch=0, jigNo;
	S_MSG_VAL SendMsg;

	jigNo = myPs->misc.JigNo - 1;
	
	switch(myPs->microState) {
		case P0:
			code = convert_jig_code(CONVERT_JIG_TO_ORG, (int)myPs->code);

			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
			SendMsg.msg = MSG_JIGC_JIGM_FAIL;
			SendMsg.val[0] = code;
			send_msg(JIGC1_TO_JIGM + jigNo, (char *)&SendMsg);
			myPs->microState = P1;
			break;
		case P1:
			myPs->code = J_CD_NONE;
			if(myPs->config.jigUseFlag == USE) {
				Select_OutPoint(myPs->misc.JigNo, ch,
					O_JIG_GRIP_CYLINDER_OFF, OFF);
				Select_OutPoint(myPs->misc.JigNo, ch,
					O_JIG_GRIP_CYLINDER_ON, ON);
			}
			myPs->microState = P2;
			break;
		case P2:
			if(myPs->config.jigUseFlag == USE) {
				flag1 = Read_InPoint(myPs->misc.JigNo, ch,
					I_JIG_MAIN_CYLINDER_L_OFF);
				flag2 = Read_InPoint(myPs->misc.JigNo, ch,
					I_JIG_MAIN_CYLINDER_R_OFF);
				if(flag1 == P1 && flag2 == P1) 
					myPs->microState = P3;
			} else {
				myPs->microState = P3;
			}
			break;
		case P3: // clamp open delay
			myPs->jigState = J_DISCONTACT;
			myPs->microState = P0;
			break;
		default: break;
	}
}

void jEMG_6(void)
{
	int code, jigNo;
	S_MSG_VAL SendMsg;
		
	jigNo = myPs->misc.JigNo - 1;

	switch(myPs->microState) {
		case P0:
			code = convert_jig_code(CONVERT_JIG_TO_ORG, (int)myPs->code);

			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
			SendMsg.msg = MSG_JIGC_IO_EMG;
			SendMsg.val[0] = code;
			SendMsg.val[1] = jigNo;
			send_msg(JIGC1_TO_IO + jigNo, (char *)&SendMsg);
			userlog(DEBUG_LOG, psName,
				"Jig Emg Code[%d] Position[%d]\n", code, myPs->troublePos);
			myPs->microState = P1;
			break;
		case P1:
			myPs->code = J_CD_NONE;
			myPs->microState = P2;
			break;
	}
}

void jDisContact_7(void)
{
	int flag1, flag2, jigNo, ch=0;
	S_MSG_VAL SendMsg;

	jigNo = myPs->misc.JigNo - 1;

	switch(myPs->microState) {
		case P0:
			if(myPs->config.jigUseFlag == USE) {
				memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
				SendMsg.msg = MSG_JIG_FAN_1;
				SendMsg.val[0] = OFF;
				SendMsg.val[1] = jigNo;
				//kjg_w send_msg(JIGC1_TO_IO_COMM + jigNo, (char *)&SendMsg);

				SendMsg.msg = MSG_JIG_FAN_2;
				//kjg_w send_msg(JIGC1_TO_IO_COMM + jigNo, (char *)&SendMsg);
			}

			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
			SendMsg.msg = MSG_JIGC_JIGM_DISCONTACT_COMPLETE;
			send_msg(JIGC1_TO_JIGM + jigNo, (char *)&SendMsg);
			myPs->misc.retryCnt=0;
			myPs->microState = P1;
			break;
		case P1:
			if(myPs->config.jigUseFlag == USE) {
				flag1 = Read_InPoint(myPs->misc.JigNo, ch,
					I_JIG_GRIP_CYLINDER_L_OFF);
				flag2 = Read_InPoint(myPs->misc.JigNo, ch,
					I_JIG_MAIN_CYLINDER_L_OFF);
				if(flag1 != P1) {
					myPs->code = J_CD_GRIP_CYL_OPEN_ERROR;
//					myPs->jigState = J_FAIL;
//					myPs->microState = P0;
//					break;
				} else if(flag2 != P1) {
					myPs->code = J_CD_MAIN_CYL_UP_ERROR;
//					myPs->jigState = J_FAIL;
//					myPs->microState = P0;
//					break;
				}

				flag1 = Read_InPoint(myPs->misc.JigNo, ch,
					I_JIG_LATCH_CYLINDER_L_OFF);
				flag2 = Read_InPoint(myPs->misc.JigNo, ch,
					I_JIG_LATCH_CYLINDER_R_OFF);
				if(flag1 != P1) {
					myPs->code = J_CD_LATCH_CYL_R_OPEN_ERROR;
//					myPs->jigState = J_FAIL;
//					myPs->microState = P0;
//					break;
				} else if(flag2 != P1) {
					myPs->code = J_CD_LATCH_CYL_L_OPEN_ERROR;
//					myPs->jigState = J_FAIL;
//					myPs->microState = P0;
//					break;
				}
				if(myPs->code != J_CD_NONE){
					myPs->misc.retryCnt++;
					if(myPs->misc.retryCnt > 3){
						myPs->misc.retryCnt=0;
						myPs->jigState = J_FAIL;
						myPs->microState = P0;
						break;
					}else{
						myPs->code = J_CD_NONE;
					}
				}

				flag1 = Read_InPoint(myPs->misc.JigNo, ch, I_JIG_SW_START);
				if(flag1 == P1){
					if(myPs->startButtonFlag == P0){
						myPs->startButtonFlag = P1;
						if(myPs->trayState[0] == T_LOAD){
							memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
							SendMsg.msg = MSG_JIGC_JIGM_START_BUTTON;
							send_msg(JIGC1_TO_JIGM + jigNo, (char *)&SendMsg);
						}
					}
				}else{
					myPs->startButtonFlag = P0;

				}
			}
			break;
		default: break;
	}
}

void jTo_Contact_7(void)
{
	int flag1, flag2, jigNo, ch=0;
	S_MSG_VAL SendMsg;
	
	if(myPs->trayState[0] != T_LOAD) {
		myPs->code = J_CD_TRAY_UNLOAD;
		myPs->jigState = J_FAIL;
		myPs->microState = P0;
		return;
	}

	if(myPs->config.jigUseFlag == UNUSE) {
		myPs->jigState = J_CONTACT;
		myPs->microState = P0;
		return;
	}

	if(myPs->jigNotMove == ON) return;

	jigNo = myPs->misc.JigNo - 1;
					
	switch(myPs->microState) {
		case P0:
			myPs->misc.retryCnt = 0;
			myPs->microState = P1;
			break;
		case P1:
			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
			SendMsg.msg = MSG_JIG_LATCH_CYL;
			SendMsg.val[0] = ON;
			SendMsg.val[1] = jigNo;
			//kjg_w send_msg(JIGC1_TO_IO_COMM + jigNo, (char *)&SendMsg);
			myPs->misc.cylMoveTimer = myData->mData.misc.timer_1sec;
			myPs->microState = P2;
			break;
		case P2:
			flag1 = Read_InPoint(myPs->misc.JigNo, ch,
				I_JIG_LATCH_CYLINDER_L_ON);
			flag2 = Read_InPoint(myPs->misc.JigNo, ch,
				I_JIG_LATCH_CYLINDER_R_ON);
			if(flag1 == P1 && flag2 == P1) {
				myPs->microState = P3;
				myPs->misc.retryCnt = 0;
			} else {
				if((myData->mData.misc.timer_1sec - myPs->misc.cylMoveTimer)
					> myPs->config.cylMoveTimeout/1000) {
					myPs->misc.retryCnt++;
					if(myPs->misc.retryCnt < 3){
						myPs->microState = P1;
					}else{
						myPs->misc.retryCnt = 0;
						if(flag1 == P0 && flag2 == P0) {
							myPs->code = J_CD_LATCH_CYL_BOTH_CLOSE_TIMEOUT;
						} else if(flag1 == P0 && flag2 == P1) {
							myPs->code = J_CD_LATCH_CYL_R_CLOSE_TIMEOUT;
						} else if(flag1 == P1 && flag2 == P0) {
							myPs->code = J_CD_LATCH_CYL_L_CLOSE_TIMEOUT;
						}
						myPs->jigState = J_FAIL;
						myPs->microState = P0;
					}
				}
			}
			break;
		case P3:
			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
			SendMsg.msg = MSG_JIG_MAIN_CYL;
			SendMsg.val[0] = ON;
			SendMsg.val[1] = jigNo;
			//kjg_w send_msg(JIGC1_TO_IO_COMM + jigNo, (char *)&SendMsg);
			myPs->misc.cylMoveTimer = myData->mData.misc.timer_1sec;
			myPs->microState = P4;
			break;
		case P4:
			flag1 = Read_InPoint(myPs->misc.JigNo, ch,
				I_JIG_MAIN_CYLINDER_L_ON);
			if(flag1 == P1) {
				myPs->misc.cylMoveTimer = myData->mData.misc.timer_1sec;
				myPs->microState = P5;
				myPs->misc.retryCnt = 0;
			} else {
				if((myData->mData.misc.timer_1sec - myPs->misc.cylMoveTimer)
					> myPs->config.cylMoveTimeout/1000) {
					myPs->misc.retryCnt++;
					if(myPs->misc.retryCnt < 3){
						myPs->microState = P3;
					}else{
						myPs->misc.retryCnt = 0;
						myPs->code = J_CD_MAIN_CYL_DOWN_TIMEOUT;
						myPs->jigState = J_FAIL;
						myPs->microState = P0;
					}
				}	
			}
			break;
		case P5: //jig down delay1
			if((myData->mData.misc.timer_1sec - myPs->misc.cylMoveTimer)
					> myPs->config.cylMoveDelay/1000) {
				myPs->microState = P6;
			}
			break;
		case P6: //jig down delay2 (total 2sec)
			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
			SendMsg.msg = MSG_JIG_GRIP_CYL;
			SendMsg.val[0] = ON;
			SendMsg.val[1] = jigNo;
			//kjg_w send_msg(JIGC1_TO_IO_COMM + jigNo, (char *)&SendMsg);
			myPs->misc.cylMoveTimer = myData->mData.misc.timer_1sec;
			myPs->microState = P7;
			break;
		case P7:
			flag1 = Read_InPoint(myPs->misc.JigNo, ch,
				I_JIG_GRIP_CYLINDER_L_ON);
			if(flag1 == P1) {
				if((myData->mData.misc.timer_1sec - myPs->misc.cylMoveTimer)
						> myPs->config.cylMoveDelay/1000) {
					myPs->jigState = J_CONTACT;
					myPs->microState = P0;
				}
			} else {
				if((myData->mData.misc.timer_1sec - myPs->misc.cylMoveTimer)
					> myPs->config.cylMoveTimeout/1000) {
					myPs->misc.retryCnt++;
					if(myPs->misc.retryCnt < 3){
						myPs->microState = P6;
					}else{
						myPs->misc.retryCnt=0;
						myPs->code = J_CD_GRIP_CYL_CLOSE_TIMEOUT;
						myPs->jigState = J_FAIL;
						myPs->microState = P0;
					}
				}
			}
			break;
		default: break;
	}
}

void jContact_7(void)
{
	int flag1, flag2, jigNo, ch=0;
	S_MSG_VAL SendMsg;

	jigNo = myPs->misc.JigNo - 1;

	switch(myPs->microState) {
		case P0:
			if(myPs->config.jigUseFlag == USE) {
				memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
				SendMsg.msg = MSG_JIG_FAN_1;
				SendMsg.val[0] = ON;
				SendMsg.val[1] = jigNo;
				//kjg_w send_msg(JIGC1_TO_IO_COMM + jigNo, (char *)&SendMsg);
				SendMsg.msg = MSG_JIG_FAN_2;
				//kjg_w send_msg(JIGC1_TO_IO_COMM + jigNo, (char *)&SendMsg);
			}

			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
			SendMsg.msg = MSG_JIGC_JIGM_CONTACT_COMPLETE;
			send_msg(JIGC1_TO_JIGM + jigNo, (char *)&SendMsg);
			myPs->misc.retryCnt=0;
			myPs->microState = P1;
			break;
		case P1:
			if(myPs->config.jigUseFlag == USE) {
				flag1 = Read_InPoint(myPs->misc.JigNo, ch,
					I_JIG_LATCH_CYLINDER_L_ON);
				flag2 = Read_InPoint(myPs->misc.JigNo, ch,
					I_JIG_LATCH_CYLINDER_R_ON);
				if(flag1 != P1) {
					myPs->code = J_CD_LATCH_CYL_R_CLOSE_ERROR;
//					myPs->jigState = J_FAIL;
//					myPs->microState = P0;
//					break;
				} else if(flag2 != P1) {
					myPs->code = J_CD_LATCH_CYL_L_CLOSE_ERROR;
//					myPs->jigState = J_FAIL;
//					myPs->microState = P0;
//					break;
				}

				flag1 = Read_InPoint(myPs->misc.JigNo, ch,
					I_JIG_MAIN_CYLINDER_L_ON);
				flag2 = Read_InPoint(myPs->misc.JigNo, ch,
					I_JIG_MAIN_CYLINDER_R_ON);
				if(flag1 != P1) {
					myPs->code = J_CD_MAIN_CYL_DOWN_ERROR;
//					myPs->jigState = J_FAIL;
//					myPs->microState = P0;
//					break;
				} else if(flag2 != P1) {
					myPs->code = J_CD_GRIP_CYL_CLOSE_ERROR;
//					myPs->jigState = J_FAIL;
//					myPs->microState = P0;
//					break;
				}

				if(myPs->code != J_CD_NONE){
					myPs->misc.retryCnt++;
					if(myPs->misc.retryCnt > 3){
						myPs->misc.retryCnt=0;
						myPs->jigState = J_FAIL;
						myPs->microState = P0;
						break;
					}else{
						myPs->code = J_CD_NONE;
					}
				}

				flag1 = Read_InPoint(myPs->misc.JigNo, ch, I_JIG_SW_EMG);
				if(flag1 == ON){
					if(myPs->emgSwitchFlag == P0){
						myPs->emgSwitchFlag = P1;
						myPs->code = J_CD_EMG_SWITCH;
						myPs->jigState = J_FAIL;
						myPs->microState = P0;
					}
				}else{
					myPs->emgSwitchFlag = P0;
				}
			}
			break;
		default: break;
	}
}

void jTo_DisContact_7(void)
{
	int flag1, flag2, jigNo, ch=0;
	S_MSG_VAL SendMsg;
	
	if(myPs->config.jigUseFlag == UNUSE) {
		myPs->jigState = J_DISCONTACT;
		myPs->microState = P0;
		return;
	}

	if(myPs->jigNotMove == ON) return;

	jigNo = myPs->misc.JigNo - 1;

	switch(myPs->microState) {
		case P0:
			myPs->misc.retryCnt =0;
			myPs->microState = P1;
			break;
		case P1:
			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
			SendMsg.msg = MSG_JIG_GRIP_CYL;
			SendMsg.val[0] = OFF;
			SendMsg.val[1] = jigNo;
			//kjg_w send_msg(JIGC1_TO_IO_COMM + jigNo, (char *)&SendMsg);
			myPs->misc.cylMoveTimer = myData->mData.misc.timer_1sec;
			myPs->microState = P2;
			break;
		case P2:
			flag1 = Read_InPoint(myPs->misc.JigNo, ch,
				I_JIG_GRIP_CYLINDER_L_OFF);
			if(flag1 == P1) {
				myPs->misc.retryCnt = 0;
				myPs->microState = P3;
			} else {
				if((myData->mData.misc.timer_1sec - myPs->misc.cylMoveTimer)
					> myPs->config.cylMoveTimeout/1000) {
					myPs->misc.retryCnt++;
					if(myPs->misc.retryCnt < 3){
						myPs->microState = P1;
					}else{
						myPs->code = J_CD_GRIP_CYL_OPEN_TIMEOUT;
						myPs->jigState = J_FAIL;
						myPs->microState = P0;
						myPs->misc.retryCnt = 0;
					}
				}
			}
			break;
		case P3: // clamp(grip cyl) open delay1
			myPs->microState = P4;
			break;
		case P4:
			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
			SendMsg.msg = MSG_JIG_MAIN_CYL;
			SendMsg.val[0] = OFF;
			SendMsg.val[1] = jigNo;
			//kjg_w send_msg(JIGC1_TO_IO_COMM + jigNo, (char *)&SendMsg);
			myPs->misc.cylMoveTimer = myData->mData.misc.timer_1sec;
			myPs->microState = P5;
			break;
		case P5:
			flag1 = Read_InPoint(myPs->misc.JigNo, ch,
				I_JIG_MAIN_CYLINDER_L_OFF);
			if(flag1 == P1) {
				myPs->misc.cylMoveTimer = myData->mData.misc.timer_1sec;
				myPs->misc.retryCnt = 0;
				myPs->microState = P6;
			} else {
				if((myData->mData.misc.timer_1sec - myPs->misc.cylMoveTimer)
					> myPs->config.cylMoveTimeout/1000) {
					myPs->misc.retryCnt++;
					if(myPs->misc.retryCnt < 3){
						myPs->microState = P4;
					}else{
						myPs->code = J_CD_MAIN_CYL_UP_TIMEOUT;
						myPs->jigState = J_FAIL;
						myPs->microState = P0;
						myPs->misc.retryCnt = 0;
					}
				}
			}
			break;
		case P6:
			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
			SendMsg.msg = MSG_JIG_LATCH_CYL;
			SendMsg.val[0] = OFF;
			SendMsg.val[1] = jigNo;
			//kjg_w send_msg(JIGC1_TO_IO_COMM + jigNo, (char *)&SendMsg);
			myPs->microState = P7;
			break;
		case P7:
			flag1 = Read_InPoint(myPs->misc.JigNo, ch,
				I_JIG_LATCH_CYLINDER_L_OFF);
			flag2 = Read_InPoint(myPs->misc.JigNo, ch,
				I_JIG_LATCH_CYLINDER_R_OFF);
			if(flag1 == P1 && flag2 == P1) {
				myPs->jigState = J_DISCONTACT;
				myPs->microState = P0;
			} else {
				if((myData->mData.misc.timer_1sec - myPs->misc.cylMoveTimer)
					> myPs->config.cylMoveTimeout/1000) {
					myPs->misc.retryCnt++;
					if(myPs->misc.retryCnt < 3){
						myPs->microState = P6;
					}else{
						myPs->misc.retryCnt=0;
						if(flag1 == P0 && flag2 == P0) {
							myPs->code = J_CD_LATCH_CYL_BOTH_OPEN_TIMEOUT;
						} else if(flag1 == P0 && flag2 == P1) {
							myPs->code = J_CD_LATCH_CYL_R_OPEN_TIMEOUT;
						} else if(flag1 == P1 && flag2 == P0) {
							myPs->code = J_CD_LATCH_CYL_L_OPEN_TIMEOUT;
						}
						myPs->jigState = J_FAIL;
						myPs->microState = P0;
					}
				}
			}
			break;
		default: break;
	}
}

void jFail_7(void)
{
	int flag1, flag2, code, jigNo, ch=0;
	S_MSG_VAL SendMsg;
	
	if(myPs->jigNotMove == ON) return;

	jigNo = myPs->misc.JigNo - 1;

	switch(myPs->microState) {
		case P0:
			code = convert_jig_code(CONVERT_JIG_TO_ORG, (int)myPs->code);

			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
			SendMsg.msg = MSG_JIGC_JIGM_FAIL;
			SendMsg.val[0] = code;
			send_msg(JIGC1_TO_JIGM + jigNo, (char *)&SendMsg);
			myPs->microState = P1;
			break;
		case P1:
			myPs->code = J_CD_NONE;
			if(myPs->config.jigUseFlag == USE) {
				memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
				SendMsg.msg = MSG_JIG_GRIP_CYL;
				SendMsg.val[0] = OFF;
				SendMsg.val[1] = jigNo;
				//kjg_w send_msg(JIGC1_TO_IO_COMM + jigNo, (char *)&SendMsg);
			}
			myPs->microState = P2;
			break;
		case P2:
			if(myPs->config.jigUseFlag == USE) {
				flag1 = Read_InPoint(myPs->misc.JigNo, ch,
					I_JIG_GRIP_CYLINDER_L_OFF);
				if(flag1 == P1) 
					myPs->microState = P3;
			} else 	myPs->microState = P3;
			break;
		case P3: // clamp(grip cyl) open delay1
			myPs->microState = P4;
			break;
		case P4: // clamp(grip cyl) open delay2
			if(myPs->config.jigUseFlag == USE) {
				memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
				SendMsg.msg = MSG_JIG_MAIN_CYL;
				SendMsg.val[0] = OFF;
				SendMsg.val[1] = jigNo;
				//kjg_w send_msg(JIGC1_TO_IO_COMM + jigNo, (char *)&SendMsg);
				SendMsg.msg = MSG_JIG_LATCH_CYL;
				//kjg_w send_msg(JIGC1_TO_IO_COMM + jigNo, (char *)&SendMsg);
				myPs->microState = P5;
			} else 	myPs->microState = P5;
			break;
		case P5:
			if(myPs->config.jigUseFlag == USE) {
				flag1 = Read_InPoint(myPs->misc.JigNo, ch,
					I_JIG_MAIN_CYLINDER_L_OFF);
				if(flag1 == P1) {
					memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
					SendMsg.msg = MSG_JIG_LATCH_CYL;
					SendMsg.val[0] = OFF;
					SendMsg.val[1] = jigNo;
					//kjg_w send_msg(JIGC1_TO_IO_COMM, (char *)&SendMsg);
					myPs->misc.cylMoveTimer = myData->mData.misc.timer_1sec;
					myPs->microState = P6;
				}
			} else 
				myPs->microState = P6;
			break;
		case P6:
			if(myPs->config.jigUseFlag == USE) {
				flag1 = Read_InPoint(myPs->misc.JigNo, ch,
					I_JIG_LATCH_CYLINDER_L_OFF);
				flag2 = Read_InPoint(myPs->misc.JigNo, ch,
					I_JIG_LATCH_CYLINDER_R_OFF);
				if(flag1 == P1 && flag2 == P1) {
					myPs->jigState = J_DISCONTACT;
					myPs->microState = P0;
				}
			} else {
				myPs->jigState = J_DISCONTACT;
				myPs->microState = P0;
			}
			break;
		default: break;
	}
}

void jEMG_7(void)
{
	int code, jigNo;
	S_MSG_VAL SendMsg;
		
	jigNo = myPs->misc.JigNo - 1;
	switch(myPs->microState) {
		case P0:
			code = convert_jig_code(CONVERT_JIG_TO_ORG, (int)myPs->code);

			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
			SendMsg.msg = MSG_JIGC_IO_EMG;
			SendMsg.val[0] = code;
			SendMsg.val[1] = jigNo;
			send_msg(JIGC1_TO_IO + jigNo, (char *)&SendMsg);
			userlog(DEBUG_LOG, psName,
				"Jig Emg Code[%d] Position[%d]\n", code, myPs->troublePos);
			myPs->microState = P1;
			break;
		case P1:
			myPs->code = J_CD_NONE;
			myPs->microState = P2;
			break;
	}
}

void jDisContact_8(void)
{
	int flag1, ch=0, jigNo;
	S_MSG_VAL SendMsg;

	jigNo = myPs->misc.JigNo - 1;

	switch(myPs->microState) {
		case P0:
			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
			SendMsg.msg = MSG_JIGC_JIGM_DISCONTACT_COMPLETE;
			send_msg(JIGC1_TO_JIGM + jigNo, (char *)&SendMsg);
			myPs->microState = P1;
			break;
		case P1:
			if(myPs->config.jigUseFlag == USE) {
				flag1 = Read_InPoint(myPs->misc.JigNo, ch,
					I_JIG_GRIP_CYLINDER_L_OFF);
				if(flag1 != P1) {
					myPs->code = J_CD_GRIP_CYL_OPEN_ERROR;
					myPs->jigState = J_FAIL;
					myPs->microState = P0;
					break;
				}
			}
			break;
		default: break;
	}
}

void jTo_Contact_8(void)
{
	int flag1, ch=0;
	
	if(myPs->config.jigUseFlag == UNUSE) {
		myPs->jigState = J_CONTACT;
		myPs->microState = P0;
		return;
	}

	switch(myPs->microState) {
		case P0:
			Select_OutPoint(myPs->misc.JigNo, ch, O_JIG_GRIP_CYLINDER_OFF, OFF);
			Select_OutPoint(myPs->misc.JigNo, ch, O_JIG_GRIP_CYLINDER_ON, ON);
			myPs->misc.cylMoveTimer = myData->mData.misc.timer_1sec;
			myPs->microState = P1;
			break;
		case P1:
			flag1 = Read_InPoint(myPs->misc.JigNo, ch,
				I_JIG_GRIP_CYLINDER_L_OFF);
			if(flag1 == P0) {
				myPs->misc.cylMoveTimer = myData->mData.misc.timer_1sec;
				myPs->microState = P2;
			} else {
				if((myData->mData.misc.timer_1sec - myPs->misc.cylMoveTimer)
					> myPs->config.cylMoveTimeout/1000) {
					myPs->code = J_CD_GRIP_CYL_CLOSE_TIMEOUT;
					myPs->jigState = J_FAIL;
					myPs->microState = P0;
				}
			}
			break;
		case P2:
			if((myData->mData.misc.timer_1sec - myPs->misc.cylMoveTimer)
					> myPs->config.cylMoveDelay/1000) {
				myPs->jigState = J_CONTACT;
				myPs->microState = P0;
			}
			break;
		default: break;
	}
}

void jContact_8(void)
{
	int flag1, ch=0, jigNo;
	S_MSG_VAL SendMsg;

	jigNo = myPs->misc.JigNo - 1;

	switch(myPs->microState) {
		case P0:
			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
			SendMsg.msg = MSG_JIGC_JIGM_CONTACT_COMPLETE;
			send_msg(JIGC1_TO_JIGM + jigNo, (char *)&SendMsg);
			myPs->microState = P1;
			break;
		case P1:
			if(myPs->config.jigUseFlag == USE) {
				flag1 = Read_InPoint(myPs->misc.JigNo, ch,
					I_JIG_GRIP_CYLINDER_L_OFF);
				if(flag1 == P1) {
					myPs->code = J_CD_GRIP_CYL_CLOSE_ERROR;
					myPs->jigState = J_FAIL;
					myPs->microState = P0;
					break;
				}
			}
			break;
		default: break;
	}
}

void jTo_DisContact_8(void)
{
	int flag1, ch=0;
	
	if(myPs->config.jigUseFlag == UNUSE) {
		myPs->jigState = J_DISCONTACT;
		myPs->microState = P0;
		return;
	}

	switch(myPs->microState) {
		case P0:
			Select_OutPoint(myPs->misc.JigNo, ch, O_JIG_GRIP_CYLINDER_ON, OFF);
			Select_OutPoint(myPs->misc.JigNo, ch, O_JIG_GRIP_CYLINDER_OFF, ON);
			myPs->misc.cylMoveTimer = myData->mData.misc.timer_1sec;
			myPs->microState = P1;
			break;
		case P1:
			flag1 = Read_InPoint(myPs->misc.JigNo, ch,
				I_JIG_GRIP_CYLINDER_L_OFF);
			if(flag1 == P1) {
				myPs->jigState = J_DISCONTACT;
				myPs->microState = P0;
			} else {
				if((myData->mData.misc.timer_1sec - myPs->misc.cylMoveTimer)
					> myPs->config.cylMoveTimeout/1000) {
					myPs->code = J_CD_GRIP_CYL_OPEN_TIMEOUT;
					myPs->jigState = J_FAIL;
					myPs->microState = P0;
				}
			}
			break;
		default: break;
	}
}

void jFail_8(void)
{
	int flag1, code, ch=0, jigNo;
	S_MSG_VAL SendMsg;

	jigNo = myPs->misc.JigNo - 1;
	
	switch(myPs->microState) {
		case P0:
			code = convert_jig_code(CONVERT_JIG_TO_ORG, (int)myPs->code);

			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
			SendMsg.msg = MSG_JIGC_JIGM_FAIL;
			SendMsg.val[0] = code;
			send_msg(JIGC1_TO_JIGM + jigNo, (char *)&SendMsg);
			myPs->microState = P1;
			break;
		case P1:
			myPs->code = J_CD_NONE;
			if(myPs->config.jigUseFlag == USE) {
				Select_OutPoint(myPs->misc.JigNo, ch,
					O_JIG_GRIP_CYLINDER_ON, OFF);
				Select_OutPoint(myPs->misc.JigNo, ch,
					O_JIG_GRIP_CYLINDER_OFF, ON);
			}
			myPs->microState = P2;
			break;
		case P2:
			if(myPs->config.jigUseFlag == USE) {
				flag1 = Read_InPoint(myPs->misc.JigNo, ch,
					I_JIG_GRIP_CYLINDER_L_OFF);
				if(flag1 == P1) myPs->microState = P3;
			} else {
				myPs->microState = P3;
			}
			break;
		case P3: // clamp open delay
			myPs->jigState = J_DISCONTACT;
			myPs->microState = P0;
			break;
		default: break;
	}
}

void jEMG_8(void)
{
	int code, jigNo;
	S_MSG_VAL SendMsg;
		
	jigNo = myPs->misc.JigNo -1;
	switch(myPs->microState) {
		case P0:
			code = convert_jig_code(CONVERT_JIG_TO_ORG, (int)myPs->code);

			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
			SendMsg.msg = MSG_JIGC_IO_EMG;
			SendMsg.val[0] = code;
			SendMsg.val[1] = jigNo;
			send_msg(JIGC1_TO_IO + jigNo, (char *)&SendMsg);
			userlog(DEBUG_LOG, psName,
				"Jig Emg Code[%d] Position[%d]\n", code, myPs->troublePos);
			myPs->microState = P1;
			break;
		case P1:
			myPs->code = J_CD_NONE;
			myPs->microState = P2;
			break;
	}
}

void jDisContact_9(void)
{
	int ch, jigNo;
	S_MSG_VAL SendMsg;

	if(myPs->config.jigUseFlag == USE) {
		if(myData->dio.misc.delayTimer < myData->dio.config.dioDelay) return;
	}

	ch = 0;
	jigNo = myPs->misc.JigNo;

	switch(myPs->microState) {
		case P0:
			if(myPs->config.jigUseFlag == USE) {
				Select_OutPoint(myPs->misc.JigNo+1, ch, O_JIG_FAN_RELAY, OFF);
			}

			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
			SendMsg.msg = MSG_JIGC_JIGM_DISCONTACT_COMPLETE;
			send_msg(JIGC1_TO_JIGM + jigNo, (char *)&SendMsg);
			myPs->microState = P1;
			break;
		default: break;
	}
}

void jTo_Contact_9(void)
{
	int flag1, flag2, ch;
	
	if(myPs->config.jigUseFlag == UNUSE) {
		myPs->jigState = J_CONTACT;
		myPs->microState = P0;
		return;
	}

	if(myData->dio.misc.delayTimer < myData->dio.config.dioDelay) return;

	if(myPs->trayState[0] != T_LOAD) {
		myPs->code = J_CD_TRAY_UNLOAD;
		myPs->jigState = J_FAIL;
		myPs->microState = P0;
		return;
	}

	if(myPs->jigNotMove == ON) return;
	
	ch = 0;
					
	switch(myPs->microState) {
		case P0:
			Select_OutPoint(myPs->misc.JigNo+1, ch,
				O_JIG_LATCH_CYLINDER_ON_OFF, ON);
			myPs->misc.cylMoveTimer = myData->mData.misc.timer_1sec;
			myPs->microState = P1;
			break;
		case P1:
			flag1 = Read_InPoint(myPs->misc.JigNo+1, ch,
				I_JIG_LATCH_CYLINDER_R_ON);
			flag2 = Read_InPoint(myPs->misc.JigNo+1, ch,
				I_JIG_LATCH_CYLINDER_L_ON);
			if(flag1 == P1 && flag2 == P1) {
				Select_OutPoint(myPs->misc.JigNo+1, ch,
					O_JIG_MAIN_CYLINDER_ON_OFF, ON);
				myPs->misc.cylMoveTimer = myData->mData.misc.timer_1sec;
				myPs->microState = P2;
			} else {
				if((myData->mData.misc.timer_1sec - myPs->misc.cylMoveTimer)
					> myPs->config.cylMoveTimeout/1000) {
					if(flag1 == P0 && flag2 == P0) {
						myPs->code = J_CD_LATCH_CYL_BOTH_CLOSE_TIMEOUT;
					} else if(flag1 == P0 && flag2 == P1) {
						myPs->code = J_CD_LATCH_CYL_R_CLOSE_TIMEOUT;
					} else if(flag1 == P1 && flag2 == P0) {
						myPs->code = J_CD_LATCH_CYL_L_CLOSE_TIMEOUT;
					}
					myPs->jigState = J_FAIL;
					myPs->microState = P0;
				}
			}
			break;
		case P2:
			flag1 = Read_InPoint(myPs->misc.JigNo+1, ch,
				I_JIG_MAIN_CYLINDER_L_ON);
			if(flag1 == P1) {
				//myPs->jigState = J_CONTACT;
				//myPs->microState = P0;
				myPs->misc.cylMoveTimer = myData->mData.misc.timer_1sec;
				myPs->microState = P3;
			} else {
				if((myData->mData.misc.timer_1sec - myPs->misc.cylMoveTimer)
					> myPs->config.cylMoveTimeout / 1000) {
					myPs->code = J_CD_MAIN_CYL_DOWN_TIMEOUT;
					myPs->jigState = J_FAIL;
					myPs->microState = P0;
				}
			}
			break;
		case P3:
			if((myData->mData.misc.timer_1sec - myPs->misc.cylMoveTimer)
				> myPs->config.cylMoveDelay / 1000) {
				myPs->jigState = J_CONTACT;
				myPs->microState = P0;
			}
			break;
		default: break;
	}
}

void jContact_9(void)
{
	int ch=0, jigNo;
	S_MSG_VAL SendMsg;

	jigNo = myPs->misc.JigNo;

	if(myPs->config.jigUseFlag == USE) {
		if(myData->dio.misc.delayTimer < myData->dio.config.dioDelay) return;
	}

	switch(myPs->microState) {
		case P0:
			if(myPs->config.jigUseFlag == USE) {
				Select_OutPoint(myPs->misc.JigNo+1, ch, O_JIG_FAN_RELAY, ON);
			}
			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
			SendMsg.msg = MSG_JIGC_JIGM_CONTACT_COMPLETE;
			send_msg(JIGC1_TO_JIGM + jigNo, (char *)&SendMsg);
			myPs->microState = P1;
			break;
		default: break;
	}
}

void jTo_DisContact_9(void)
{
	int flag1, flag2, ch=0;
	
	if(myPs->config.jigUseFlag == UNUSE) {
		myPs->jigState = J_DISCONTACT;
		myPs->microState = P0;
		return;
	}

	if(myData->dio.misc.delayTimer < myData->dio.config.dioDelay) return;
	if(myPs->jigNotMove == ON) return;

	switch(myPs->microState) {
		case P0:
			Select_OutPoint(myPs->misc.JigNo+1, ch,
				O_JIG_MAIN_CYLINDER_ON_OFF, OFF);
			myPs->misc.cylMoveTimer = myData->mData.misc.timer_1sec;
			myPs->signal[J_SIG_TRY_TRAY_CHECK_COMPLETE] = P0;
			myPs->microState = P1;
			break;
		case P1:
			flag1 = Read_InPoint(myPs->misc.JigNo+1, ch,
				I_JIG_MAIN_CYLINDER_L_OFF);
			if(flag1 == P1) {
				Select_OutPoint(myPs->misc.JigNo+1, ch,
					O_JIG_LATCH_CYLINDER_ON_OFF, OFF);
				myPs->misc.cylMoveTimer = myData->mData.misc.timer_1sec;
				myPs->microState = P2;
			} else {
				if((myData->mData.misc.timer_1sec - myPs->misc.cylMoveTimer)
					> myPs->config.cylMoveTimeout/1000) {
					myPs->code = J_CD_MAIN_CYL_UP_TIMEOUT;
					myPs->jigState = J_FAIL;
					myPs->microState = P0;
				}
			}
			break;
		case P2:
			flag1 = Read_InPoint(myPs->misc.JigNo+1, ch,
				I_JIG_LATCH_CYLINDER_L_OFF);
			flag2 = Read_InPoint(myPs->misc.JigNo+1, ch,
				I_JIG_LATCH_CYLINDER_R_OFF);
			if(flag1 == P1 && flag2 == P1) {
				myPs->jigState = J_DISCONTACT;
				myPs->microState = P0;
			} else {
				if((myData->mData.misc.timer_1sec - myPs->misc.cylMoveTimer)
					> myPs->config.cylMoveTimeout/1000) {
					if(flag1 == P0 && flag2 == P0) {
						myPs->code = J_CD_LATCH_CYL_BOTH_OPEN_TIMEOUT;
					} else if(flag1 == P0 && flag2 == P1) {
						myPs->code = J_CD_LATCH_CYL_R_OPEN_TIMEOUT;
					} else if(flag1 == P1 && flag2 == P0) {
						myPs->code = J_CD_LATCH_CYL_L_OPEN_TIMEOUT;
					}
					myPs->jigState = J_FAIL;
					myPs->microState = P0;
				}
			}
			break;
		default: break;
	}
}

void jFail_9(void)
{
	int flag1, flag2, code, ch=0, jigNo;
	S_MSG_VAL SendMsg;
	
	if(myData->dio.misc.delayTimer < myData->dio.config.dioDelay) return;
	if(myPs->jigNotMove == ON) return;

	jigNo = myPs->misc.JigNo;

	switch(myPs->microState) {
		case P0:
			code = convert_jig_code(CONVERT_JIG_TO_ORG, (int)myPs->code);

			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
			SendMsg.msg = MSG_JIGC_JIGM_FAIL;
			SendMsg.val[0] = code;
			send_msg(JIGC1_TO_JIGM + jigNo, (char *)&SendMsg);

			if(myPs->config.jigUseFlag == USE) {
				Select_OutPoint(myPs->misc.JigNo+1, ch,
					O_JIG_MAIN_CYLINDER_ON_OFF, OFF);
			}
			myPs->microState = P1;
			break;
		case P1:
			//kjg_w_f myPs->code = J_CD_NONE;
			if(myPs->config.jigUseFlag == USE) {
				Select_OutPoint(myPs->misc.JigNo+1, ch,
					O_JIG_MAIN_CYLINDER_ON_OFF, OFF);
			}
			myPs->microState = P2;
			break;
		case P2:
			if(myPs->config.jigUseFlag == USE) {
				flag1 = Read_InPoint(myPs->misc.JigNo+1, ch,
					I_JIG_LATCH_CYLINDER_L_OFF);
				flag2 = Read_InPoint(myPs->misc.JigNo+1, ch,
					I_JIG_LATCH_CYLINDER_R_OFF);
				if(flag1 == P1 && flag2 == P1) {
					myPs->jigState = J_DISCONTACT;
					myPs->microState = P0;
				} 
			} else {
				myPs->jigState = J_DISCONTACT;
				myPs->microState = P0;
			}
			break;
		default: break;
	}
}

void jEMG_9(void)
{
	int code, jigNo;
	S_MSG_VAL SendMsg;
		
	if(myData->dio.misc.delayTimer < myData->dio.config.dioDelay) return;

	jigNo = myPs->misc.JigNo;

	switch(myPs->microState) {
		case P0:
			code = convert_jig_code(CONVERT_JIG_TO_ORG, (int)myPs->code);

			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
			SendMsg.msg = MSG_JIGC_IO_EMG;
			SendMsg.val[0] = code;
			SendMsg.val[1] = jigNo;
			send_msg(JIGC1_TO_IO + jigNo, (char *)&SendMsg);
			userlog(DEBUG_LOG, psName, "Jig Emg Code[%d] Position[%d]\n",
				code, myPs->troublePos);
			myPs->microState = P1;
			break;
		case P1:
			myPs->code = J_CD_NONE;
			myPs->microState = P2;
			break;
	}
}

void jDisContact_10(void)
{
	int ch, jigNo;
	S_MSG_VAL SendMsg;

	if(myPs->config.jigUseFlag == USE) {
		if(myData->dio.misc.delayTimer < myData->dio.config.dioDelay) return;
	}

	ch = 0;
	jigNo = myPs->misc.JigNo;

	switch(myPs->microState) {
		case P0:
			if(myPs->config.jigUseFlag == USE) {
				Select_OutPoint(myPs->misc.JigNo+1, ch, O_JIG_FAN_RELAY, OFF);
			}

			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
			SendMsg.msg = MSG_JIGC_JIGM_DISCONTACT_COMPLETE;
			send_msg(JIGC1_TO_JIGM + jigNo, (char *)&SendMsg);
			myPs->microState = P1;
			break;
		default: break;
	}
}

void jTo_Contact_10(void)
{
	int flag1, flag2, ch;
	
	if(myPs->config.jigUseFlag == UNUSE) {
		myPs->jigState = J_CONTACT;
		myPs->microState = P0;
		return;
	}

	if(myData->dio.misc.delayTimer < myData->dio.config.dioDelay) return;

	if(myPs->trayState[0] != T_LOAD) {
		myPs->code = J_CD_TRAY_UNLOAD;
		myPs->jigState = J_FAIL;
		myPs->microState = P0;
		return;
	}

	if(myPs->jigNotMove == ON) return;
	
	ch = 0;
					
	switch(myPs->microState) {
		case P0:
			Select_OutPoint(myPs->misc.JigNo+1, ch,
				O_JIG_MAIN_CYLINDER_ON_OFF, ON);
			myPs->misc.cylMoveTimer = myData->mData.misc.timer_1sec;
			myPs->microState = P1;
			break;
		case P1:
			flag1 = Read_InPoint(myPs->misc.JigNo+1, ch,
				I_JIG_MAIN_CYLINDER_L_OFF);
			flag2 = Read_InPoint(myPs->misc.JigNo+1, ch,
				I_JIG_MAIN_CYLINDER_L_ON);
			if(flag1 == P0 && flag2 == P1) {
				myPs->misc.cylMoveTimer = myData->mData.misc.timer_1sec;
				myPs->microState = P2;
			} else {
				if((myData->mData.misc.timer_1sec - myPs->misc.cylMoveTimer)
					> myPs->config.cylMoveTimeout/1000) {
					myPs->code = J_CD_MAIN_CYL_DOWN_TIMEOUT;
					myPs->jigState = J_FAIL;
					myPs->microState = P0;
				}
			}
			break;
		case P2:
			if((myData->mData.misc.timer_1sec - myPs->misc.cylMoveTimer)
				> myPs->config.cylMoveDelay / 1000) {
				myPs->jigState = J_CONTACT;
				myPs->microState = P0;
			}
			break;
		default: break;
	}
}

void jContact_10(void)
{
	int ch=0, jigNo;
	S_MSG_VAL SendMsg;

	jigNo = myPs->misc.JigNo;

	if(myPs->config.jigUseFlag == USE) {
		if(myData->dio.misc.delayTimer < myData->dio.config.dioDelay) return;
	}

	switch(myPs->microState) {
		case P0:
			if(myPs->config.jigUseFlag == USE) {
				Select_OutPoint(myPs->misc.JigNo+1, ch, O_JIG_FAN_RELAY, ON);
			}
			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
			SendMsg.msg = MSG_JIGC_JIGM_CONTACT_COMPLETE;
			send_msg(JIGC1_TO_JIGM + jigNo, (char *)&SendMsg);
			myPs->microState = P1;
			break;
		default: break;
	}
}

void jTo_DisContact_10(void)
{
	int flag1, flag2, ch=0;
	
	if(myPs->config.jigUseFlag == UNUSE) {
		myPs->jigState = J_DISCONTACT;
		myPs->microState = P0;
		return;
	}

	if(myData->dio.misc.delayTimer < myData->dio.config.dioDelay) return;
	if(myPs->jigNotMove == ON) return;

	switch(myPs->microState) {
		case P0:
			Select_OutPoint(myPs->misc.JigNo+1, ch,
				O_JIG_MAIN_CYLINDER_ON_OFF, OFF);
			myPs->misc.cylMoveTimer = myData->mData.misc.timer_1sec;
			myPs->signal[J_SIG_TRY_TRAY_CHECK_COMPLETE] = P0;
			myPs->microState = P1;
			break;
		case P1:
			flag1 = Read_InPoint(myPs->misc.JigNo+1, ch,
				I_JIG_MAIN_CYLINDER_L_OFF);
			flag2 = Read_InPoint(myPs->misc.JigNo+1, ch,
				I_JIG_MAIN_CYLINDER_L_ON);
			//if(flag1 == P1 && flag2 == P0) {
			if(flag2 == P0) {
				myPs->jigState = J_DISCONTACT;
				myPs->microState = P0;
			} else {
				if((myData->mData.misc.timer_1sec - myPs->misc.cylMoveTimer)
					> myPs->config.cylMoveTimeout/1000) {
					myPs->code = J_CD_MAIN_CYL_UP_TIMEOUT;
					myPs->jigState = J_FAIL;
					myPs->microState = P0;
				}
			}
			break;
		default: break;
	}
}

void jFail_10(void)
{
	int flag1, flag2, code, ch=0, jigNo;
	S_MSG_VAL SendMsg;
	
	if(myData->dio.misc.delayTimer < myData->dio.config.dioDelay) return;
	if(myPs->jigNotMove == ON) return;

	jigNo = myPs->misc.JigNo;

	switch(myPs->microState) {
		case P0:
			code = convert_jig_code(CONVERT_JIG_TO_ORG, (int)myPs->code);

			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
			SendMsg.msg = MSG_JIGC_JIGM_FAIL;
			SendMsg.val[0] = code;
			send_msg(JIGC1_TO_JIGM + jigNo, (char *)&SendMsg);

			if(myPs->config.jigUseFlag == USE) {
				Select_OutPoint(myPs->misc.JigNo+1, ch,
					O_JIG_MAIN_CYLINDER_ON_OFF, OFF);
			}
			myPs->microState = P1;
			break;
		case P1:
			//kjg_w_f myPs->code = J_CD_NONE;
			if(myPs->config.jigUseFlag == USE) {
				Select_OutPoint(myPs->misc.JigNo+1, ch,
					O_JIG_MAIN_CYLINDER_ON_OFF, OFF);
			}
			myPs->microState = P2;
			break;
		case P2:
			if(myPs->config.jigUseFlag == USE) {
				flag1 = Read_InPoint(myPs->misc.JigNo+1, ch,
					I_JIG_MAIN_CYLINDER_L_OFF);
				flag2 = Read_InPoint(myPs->misc.JigNo+1, ch,
					I_JIG_MAIN_CYLINDER_L_ON);
				//if(flag1 == P1 && flag2 == P0) {
				if(flag2 == P0) {
					myPs->jigState = J_DISCONTACT;
					myPs->microState = P0;
				} 
			} else {
				myPs->jigState = J_DISCONTACT;
				myPs->microState = P0;
			}
			break;
		default: break;
	}
}

void jEMG_10(void)
{
	int code, jigNo;
	S_MSG_VAL SendMsg;
		
	if(myData->dio.misc.delayTimer < myData->dio.config.dioDelay) return;

	jigNo = myPs->misc.JigNo;

	switch(myPs->microState) {
		case P0:
			code = convert_jig_code(CONVERT_JIG_TO_ORG, (int)myPs->code);

			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
			SendMsg.msg = MSG_JIGC_IO_EMG;
			SendMsg.val[0] = code;
			SendMsg.val[1] = jigNo;
			send_msg(JIGC1_TO_IO + jigNo, (char *)&SendMsg);
			userlog(DEBUG_LOG, psName, "Jig Emg Code[%d] Position[%d]\n",
				code, myPs->troublePos);
			myPs->microState = P1;
			break;
		case P1:
			myPs->code = J_CD_NONE;
			myPs->microState = P2;
			break;
	}
}

void jDisContact_11(void)
{
	int ch, jigNo;
	S_MSG_VAL SendMsg;

	if(myPs->config.jigUseFlag == USE) {
		if(myData->dio.misc.delayTimer < myData->dio.config.dioDelay) return;
	}

	ch = 0;
	jigNo = myPs->misc.JigNo;

	switch(myPs->microState) {
		case P0:
			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
			SendMsg.msg = MSG_JIGC_JIGM_DISCONTACT_COMPLETE;
			send_msg(JIGC1_TO_JIGM + jigNo, (char *)&SendMsg);
			myPs->microState = P1;
			break;
		default: break;
	}
}

void jTo_Contact_11(void)
{
	int flag1, flag2, flag3, flag4, ch;
	
	if(myPs->config.jigUseFlag == UNUSE) {
		myPs->jigState = J_CONTACT;
		myPs->microState = P0;
		return;
	}

	if(myData->dio.misc.delayTimer < myData->dio.config.dioDelay) return;

	if(myPs->trayState[0] != T_LOAD) {
		myPs->code = J_CD_TRAY_UNLOAD;
		myPs->jigState = J_FAIL;
		myPs->microState = P0;
		return;
	}

	if(myPs->jigNotMove == ON) return;
//	if(myPs->jobChangeEnd != JOB_CHANGE_END) return;
	
	ch = 0;
					
	switch(myPs->microState) {
		case P0:
			Select_OutPoint(myPs->misc.JigNo+1, 1,
				O_JIG_LATCH_CYLINDER_ON_OFF, ON);
			Select_OutPoint(myPs->misc.JigNo+1, 2,
				O_JIG_LATCH_CYLINDER_ON_OFF, ON);
			myPs->misc.cylMoveTimer = myData->mData.misc.timer_1sec;
			myPs->microState = P1;
			break;
		case P1:
			flag1 = Read_InPoint(myPs->misc.JigNo+1, 1,
				I_JIG_LATCH_CYLINDER_L_ON);
			flag2 = Read_InPoint(myPs->misc.JigNo+1, 2,
				I_JIG_LATCH_CYLINDER_L_ON);
			flag3 = Read_InPoint(myPs->misc.JigNo+1, 1,
				I_JIG_LATCH_CYLINDER_R_ON);
			flag4 = Read_InPoint(myPs->misc.JigNo+1, 2,
				I_JIG_LATCH_CYLINDER_R_ON);
			if(flag1 == P1 && flag2 == P1 && flag3 == P1 && flag4 == P1) {
				if((myData->mData.misc.timer_1sec - myPs->misc.cylMoveTimer)
					> myPs->config.cylMoveDelay/1000) {
					myPs->microState = P2;
				}
			} else {
				if((myData->mData.misc.timer_1sec - myPs->misc.cylMoveTimer)
					> myPs->config.cylMoveTimeout/1000) {
					myPs->code = J_CD_MAIN_CYL_DOWN_TIMEOUT;
					myPs->jigState = J_FAIL;
					myPs->microState = P0;
				}
			}
			break;
		case P2:
			Select_OutPoint(myPs->misc.JigNo+1, 1,
				O_JIG_LATCH_CYLINDER_ON_OFF, OFF);
			Select_OutPoint(myPs->misc.JigNo+1, 2,
				O_JIG_LATCH_CYLINDER_ON_OFF, OFF);
			myPs->misc.cylMoveTimer = myData->mData.misc.timer_1sec;
			myPs->microState = P3;
			break;
		case P3:
			flag1 = Read_InPoint(myPs->misc.JigNo+1, 1,
				I_JIG_LATCH_CYLINDER_L_ON);
			flag2 = Read_InPoint(myPs->misc.JigNo+1, 2,
				I_JIG_LATCH_CYLINDER_L_ON);
			flag3 = Read_InPoint(myPs->misc.JigNo+1, 1,
				I_JIG_LATCH_CYLINDER_R_ON);
			flag4 = Read_InPoint(myPs->misc.JigNo+1, 2,
				I_JIG_LATCH_CYLINDER_R_ON);
			if(flag1 == P0 && flag2 == P0 && flag3 == P0 && flag4 == P0) {
				if((myData->mData.misc.timer_1sec - myPs->misc.cylMoveTimer)
					> myPs->config.cylMoveDelay/1000) {
					myPs->microState = P4;
				}
			} else {
				if((myData->mData.misc.timer_1sec - myPs->misc.cylMoveTimer)
					> myPs->config.cylMoveTimeout/1000) {
					myPs->code = J_CD_MAIN_CYL_DOWN_TIMEOUT;
					myPs->jigState = J_FAIL;
					myPs->microState = P0;
				}
			}
			break;
		case P4:
			Select_OutPoint(myPs->misc.JigNo+1, 0,
				O_JIG_MAIN_CYLINDER_ON_OFF, ON);
			myPs->misc.cylMoveTimer = myData->mData.misc.timer_1sec;
			myPs->microState = P5;
			break;
		case P5:
			flag1 = Read_InPoint(myPs->misc.JigNo+1, 0,
				I_JIG_MAIN_CYLINDER_L_ON_OFF);
			if(flag1 == P1) {
				myPs->microState = P6;
				myPs->misc.cylMoveTimer = myData->mData.misc.timer_1sec;
			} else {
				if((myData->mData.misc.timer_1sec - myPs->misc.cylMoveTimer)
					> myPs->config.cylMoveTimeout/1000) {
					myPs->code = J_CD_MAIN_CYL_DOWN_TIMEOUT;
					myPs->jigState = J_FAIL;
					myPs->microState = P0;
				}
			}
			break;
		case P6:
			if((myData->mData.misc.timer_1sec - myPs->misc.cylMoveTimer)
				> myPs->config.cylMoveDelay / 1000) {
				myPs->jigState = J_CONTACT;
				myPs->microState = P0;
			}
			break;
		default: break;
	}
}

void jContact_11(void)
{
	int jigNo;
	S_MSG_VAL SendMsg;

	jigNo = myPs->misc.JigNo;

	if(myPs->config.jigUseFlag == USE) {
		if(myData->dio.misc.delayTimer < myData->dio.config.dioDelay) return;
	}

	switch(myPs->microState) {
		case P0:
			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
			SendMsg.msg = MSG_JIGC_JIGM_CONTACT_COMPLETE;
			send_msg(JIGC1_TO_JIGM + jigNo, (char *)&SendMsg);
			myPs->microState = P1;
			break;
		default: break;
	}
}

void jTo_DisContact_11(void)
{
	int flag1, flag2, flag3, flag4, ch=0;
	
	if(myPs->config.jigUseFlag == UNUSE) {
		myPs->jigState = J_DISCONTACT;
		myPs->microState = P0;
		return;
	}

	if(myData->dio.misc.delayTimer < myData->dio.config.dioDelay) return;
	if(myPs->jigNotMove == ON) return;
//	if(myPs->jobChangeEnd != JOB_CHANGE_END) return;

	switch(myPs->microState) {
		case P0:
			Select_OutPoint(myPs->misc.JigNo+1, 1,
				O_JIG_LATCH_CYLINDER_ON_OFF, OFF);
			Select_OutPoint(myPs->misc.JigNo+1, 2,
				O_JIG_LATCH_CYLINDER_ON_OFF, OFF);
			myPs->misc.cylMoveTimer = myData->mData.misc.timer_1sec;
			myPs->microState = P1;
			break;
		case P1:
			flag1 = Read_InPoint(myPs->misc.JigNo+1, 1,
				I_JIG_LATCH_CYLINDER_L_ON);
			flag2 = Read_InPoint(myPs->misc.JigNo+1, 2,
				I_JIG_LATCH_CYLINDER_L_ON);
			flag3 = Read_InPoint(myPs->misc.JigNo+1, 1,
				I_JIG_LATCH_CYLINDER_R_ON);
			flag4 = Read_InPoint(myPs->misc.JigNo+1, 2,
				I_JIG_LATCH_CYLINDER_R_ON);
			if(flag1 == P0 && flag2 == P0 && flag3 == P0 && flag4 == P0) {
				if((myData->mData.misc.timer_1sec - myPs->misc.cylMoveTimer)
					> myPs->config.cylMoveDelay/1000) {
					myPs->misc.cylMoveTimer = myData->mData.misc.timer_1sec;
					myPs->microState = P2;
				}
			} else {
				if((myData->mData.misc.timer_1sec - myPs->misc.cylMoveTimer)
					> myPs->config.cylMoveTimeout/1000) {
					myPs->code = J_CD_MAIN_CYL_DOWN_TIMEOUT;
					myPs->jigState = J_FAIL;
					myPs->microState = P0;
				}
			}
			break;
		case P2:
			if((myData->mData.misc.timer_1sec - myPs->misc.cylMoveTimer)
				> myPs->config.cylMoveDelay / 1000) {
				Select_OutPoint(myPs->misc.JigNo+1, ch,
					O_JIG_MAIN_CYLINDER_ON_OFF, OFF);
				myPs->misc.cylMoveTimer = myData->mData.misc.timer_1sec;
				myPs->signal[J_SIG_TRY_TRAY_CHECK_COMPLETE] = P0;
				myPs->microState = P3;
			}
			break;
		case P3:
			flag1 = Read_InPoint(myPs->misc.JigNo+1, ch,
				I_JIG_MAIN_CYLINDER_L_ON_OFF);
			if(flag1 == P0) {
				myPs->misc.cylMoveTimer = myData->mData.misc.timer_1sec;
				myPs->microState = P4;
			} else {
				if((myData->mData.misc.timer_1sec - myPs->misc.cylMoveTimer)
					> myPs->config.cylMoveTimeout/1000) {
					myPs->code = J_CD_MAIN_CYL_DOWN_TIMEOUT;
					myPs->jigState = J_FAIL;
					myPs->microState = P0;
				}
			}
			break;
		case P4:
			if((myData->mData.misc.timer_1sec - myPs->misc.cylMoveTimer)
				> myPs->config.cylMoveDelay / 1000) {
				myPs->jigState = J_DISCONTACT;
				myPs->microState = P0;
			}
			break;
		default: break;
	}
}

void jFail_11(void)
{
	int flag1, flag2, flag3, flag4, code, ch=0, jigNo;
	S_MSG_VAL SendMsg;
	
	if(myData->dio.misc.delayTimer < myData->dio.config.dioDelay) return;
	if(myPs->jigNotMove == ON) return;

	jigNo = myPs->misc.JigNo;

	switch(myPs->microState) {
		case P0:
			code = convert_jig_code(CONVERT_JIG_TO_ORG, (int)myPs->code);

			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
			SendMsg.msg = MSG_JIGC_JIGM_FAIL;
			SendMsg.val[0] = code;
			send_msg(JIGC1_TO_JIGM + jigNo, (char *)&SendMsg);

			if(myPs->config.jigUseFlag == USE) {
				Select_OutPoint(myPs->misc.JigNo+1, 1,
					O_JIG_LATCH_CYLINDER_ON_OFF, OFF);
				Select_OutPoint(myPs->misc.JigNo+1, 2,
					O_JIG_LATCH_CYLINDER_ON_OFF, OFF);
			}
			myPs->misc.cylMoveTimer = myData->mData.misc.timer_1sec;
			myPs->microState = P1;
			break;
		case P1:
			flag1 = Read_InPoint(myPs->misc.JigNo+1, 1,
				I_JIG_LATCH_CYLINDER_L_ON);
			flag2 = Read_InPoint(myPs->misc.JigNo+1, 2,
				I_JIG_LATCH_CYLINDER_L_ON);
			flag3 = Read_InPoint(myPs->misc.JigNo+1, 1,
				I_JIG_LATCH_CYLINDER_R_ON);
			flag4 = Read_InPoint(myPs->misc.JigNo+1, 2,
				I_JIG_LATCH_CYLINDER_R_ON);
			if(myPs->config.jigUseFlag == USE) {
				if(flag1 == P0 && flag2 == P0 && flag3 == P0 && flag4 == P0) {
					Select_OutPoint(myPs->misc.JigNo+1, ch,
						O_JIG_MAIN_CYLINDER_ON_OFF, OFF);
					myPs->misc.cylMoveTimer = myData->mData.misc.timer_1sec;
					myPs->microState = P2;
				}
			} else {
				myPs->misc.cylMoveTimer = myData->mData.misc.timer_1sec;
				myPs->microState = P2;
			}
			break;
		case P2:
			if(myPs->config.jigUseFlag == USE) {
				flag1 = Read_InPoint(myPs->misc.JigNo+1, ch,
					I_JIG_MAIN_CYLINDER_L_ON_OFF);
				if(flag1 == P0) {
					myPs->jigState = J_DISCONTACT;
					myPs->microState = P0;
				} 
			} else {
				myPs->jigState = J_DISCONTACT;
				myPs->microState = P0;
			}
			break;
		default: break;
	}
}

void jEMG_11(void)
{
	int code, jigNo;
	S_MSG_VAL SendMsg;
		
	if(myData->dio.misc.delayTimer < myData->dio.config.dioDelay) return;

	jigNo = myPs->misc.JigNo;

	switch(myPs->microState) {
		case P0:
			code = convert_jig_code(CONVERT_JIG_TO_ORG, (int)myPs->code);

			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
			SendMsg.msg = MSG_JIGC_IO_EMG;
			SendMsg.val[0] = code;
			SendMsg.val[1] = jigNo;
			send_msg(JIGC1_TO_IO + jigNo, (char *)&SendMsg);
			userlog(DEBUG_LOG, psName, "Jig Emg Code[%d] Position[%d]\n",
				code, myPs->troublePos);
			myPs->microState = P1;
			break;
		case P1:
			myPs->code = J_CD_NONE;
			myPs->microState = P2;
			break;
	}
}

