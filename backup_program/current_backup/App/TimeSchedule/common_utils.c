#ifdef __KERNEL__
#include <rtl.h>
#else
#include <stdlib.h>
#include <stdio.h>
#include "userlog.h"
#endif

#include <mbuff.h>
#include "../../INC/datastore.h"
#include "common_utils.h"

extern volatile S_SYSTEM_DATA *myData;

void Unload_Module(void)
{
#ifdef __KERNEL__
#else
	system("./Unload_mControl");
#endif
}

int Open_SystemMemory(int init)
{
#ifdef __KERNEL__
#else
	if(init == 1) {
		system("insmod -f ../../Module/mbuff/mbuff.o");
	}
#endif

	myData = (volatile S_SYSTEM_DATA *)
		mbuff_alloc("myData", sizeof(S_SYSTEM_DATA));
	if(myData == NULL) {
#ifdef __KERNEL__
		rtl_printf("mbuff_alloc failed %d\n", sizeof(S_SYSTEM_DATA));
#else
		printf("mbuff_alloc failed %d\n", sizeof(S_SYSTEM_DATA));
#endif
		return -1;
	}

    if(myData == ((S_SYSTEM_DATA *)-1)) {
#ifdef __KERNEL__
		rtl_printf("Can not create system memory\n");
#else
		printf("Can not create system memory\n");
#endif
		return -2;
    }

	if(init == 1) {
		memset((S_SYSTEM_DATA *)myData, 0, sizeof(S_SYSTEM_DATA));
#ifdef __KERNEL__
		rtl_printf("Sizeof S_SYSTEM_DATA %d bytes\n", sizeof(S_SYSTEM_DATA));
#else
		printf("Sizeof S_SYSTEM_DATA %d bytes\n", sizeof(S_SYSTEM_DATA));
#endif
	}

	return 0;
}

void Close_SystemMemory(void)
{
	mbuff_free("myData", (void*)myData);
}

void Close_mbuff(int pointer)
{
	char *sysData;

	sysData = (char *)pointer;
	mbuff_free("myData", (void *)sysData);
}
/*kjg_w
int my_system(const char *) {
	int pid, status;

	if(command == 0) return 1;

	pid = fork();
	if(pid == -1) {
		return -1;
	} else if(pid == 0) {
		char *argv[4];

		argv[0] = "sh";
		argv[1] = "-c";
		argv[2] = command;
		argv[3] = 0;
		execve("/bin/sh", argv, environ);
		exit(127);
	}

	do {
		if(waitpid(pid, &status, 0) == -1) {
			if(errno != EINTR) return -1;
		} else {
			return status;
		}
	} while(1);
}*/

int bdNo_to_groupNo(int bd)
{
	int group;

	group = (int)myData->mData.config.bdInGroup[bd];
	if(group <= 0) group = -1;
	else if(group > MAX_GROUP_PER_MODULE) group = MAX_GROUP_PER_MODULE;

	return group;
}

int groupNo_to_jigNo(int group)
{
	int jig;

	jig = group + 1;

	return jig;
}

int jigNo_to_groupNo(int jig)
{
	int group;

	group = jig + 1;

	return group;
}

void make_com_chCode_cond(S_TEST_CONDITION *testCond, int code, int count, int type, int idx1)
{
	code -= C_CD_COM_START;
/*kjg_170810	myData->testCond[ch].common_chCode[code].compCount = (long)count;
	myData->testCond[ch].common_chCode[code].compType = (unsigned char)type;
	myData->testCond[ch].common_chCode[code].compIndex1 = (short int)idx1;*/
	testCond->common_chCode[code].compCount = (long)count;
	testCond->common_chCode[code].compType = (unsigned char)type;
	testCond->common_chCode[code].compIndex1 = (short int)idx1;
}

void make_loc_chCode_cond(S_TEST_CONDITION *testCond, int step2, int code, int count, int type, int idx1)
{
	code -= C_CD_END_START;
/*kjg_170810	myData->testCond[ch].local_chCode[step2][code].compCount = (long)count;
	myData->testCond[ch].local_chCode[step2][code].compType
		= (unsigned char)type;
	myData->testCond[ch].local_chCode[step2][code].compIndex1
		= (short int)idx1;*/
	testCond->local_chCode[step2][code].compCount = (long)count;
	testCond->local_chCode[step2][code].compType = (unsigned char)type;
	testCond->local_chCode[step2][code].compIndex1 = (short int)idx1;
}

void make_loc_chCode_cond2(S_TEST_CONDITION *testCond, int step2, int code, int count, int type, int idx1, int idx2)
{
	code -= C_CD_END_START;
/*kjg_170810	myData->testCond[ch].local_chCode[step2][code].compCount = (long)count;
	myData->testCond[ch].local_chCode[step2][code].compType
		= (unsigned char)type;
	myData->testCond[ch].local_chCode[step2][code].compIndex1
		= (short int)idx1;
	myData->testCond[ch].local_chCode[step2][code].compIndex2
		= (short int)idx2;*/
	testCond->local_chCode[step2][code].compCount = (long)count;
	testCond->local_chCode[step2][code].compType = (unsigned char)type;
	testCond->local_chCode[step2][code].compIndex1 = (short int)idx1;
	testCond->local_chCode[step2][code].compIndex2 = (short int)idx2;
}

void make_aux_chCode_cond(S_TEST_CONDITION *testCond, int code, int count, int type)
{
	code -= C_CD_AUX_START;
/*kjg_170810	myData->testCond[ch].aux_chCode[code].compCount = (long)count;
	myData->testCond[ch].aux_chCode[code].compType = (unsigned char)type;*/
	testCond->aux_chCode[code].compCount = (long)count;
	testCond->aux_chCode[code].compType = (unsigned char)type;
}

void make_can_chCode_cond(S_TEST_CONDITION *testCond, int code, int count, int type)
{
	code -= C_CD_CAN_START;
/*kjg_170810	myData->testCond[ch].can_chCode[code].compCount = (long)count;
	myData->testCond[ch].can_chCode[code].compType = (unsigned char)type;*/
	testCond->can_chCode[code].compCount = (long)count;
	testCond->can_chCode[code].compType = (unsigned char)type;
}

//jhkw_220103s
void make_sub_code(int ch, int code, short int val1, short int val2, short int val3, short int val4, long val5, long val6)
{
	//myData->cData[ch].op.code = code;
	myData->cData[ch].op.subCode.end_div_no = val1;
	myData->cData[ch].op.subCode.func_div = val2;
	myData->cData[ch].op.subCode.ch_index = val3;
	myData->cData[ch].op.subCode.reserved1 = val4;
	myData->cData[ch].op.subCode.reserved2[0] = val5;
	myData->cData[ch].op.subCode.reserved2[1] = val6;
}

int convert_subcode_division(int code)
{
	int rtn_div=0;
	
	switch(code) {
		case P1_C_CD_AUX_END_UPPER_TEMP:
		case P1_C_CD_AUX_END_LOWER_TEMP:
		case P1_C_CD_AUX_END_UPPER_V:
		case P1_C_CD_AUX_END_LOWER_V:
		case P1_C_CD_END_AUX_FUNC_DIV:
		case P1_C_CD_AUX_END_UPPER_HUMIDITY:
		case P1_C_CD_AUX_END_LOWER_HUMIDITY:
		case P1_C_CD_AUX_END_UPPER_GAS:	//sec_220926
		case P1_C_CD_AUX_END_LOWER_GAS:	//sec_220926
		case P1_C_CD_FAULT_AUX_COM_FUNC_DIV:
		case P1_C_CD_AUX_FAULT_UPPER_TEMP:
		case P1_C_CD_AUX_FAULT_LOWER_TEMP:
		case P1_C_CD_AUX_FAULT_UPPER_V:
		case P1_C_CD_AUX_FAULT_LOWER_V:
		case P1_C_CD_FAULT_AUX_FUNC_DIV:
		case P1_C_CD_AUX_FAULT_VENT_UPPER:
		case P1_C_CD_AUX_FAULT_VENT_LOWER:
		case P1_C_CD_AUX_FAULT_UPPER_HUMIDITY:
		case P1_C_CD_AUX_FAULT_LOWER_HUMIDITY:
		case P1_C_CD_AUX_FAULT_UPPER_GAS:	//sec_220926
		case P1_C_CD_AUX_FAULT_LOWER_GAS:	//sec_220926
		case P1_C_CD_FAULT_AUX_V_SVDF:
		case P1_C_CD_AUX_FAULT_DROP_VOLTAGE:
		case P1_C_CD_CAN_END_UPPER:
		case P1_C_CD_CAN_END_LOWER:
		case P1_C_CD_END_CAN_FUNC_DIV:
		case P1_C_CD_END_CAN_BMS_ERROR:
		case P1_C_CD_FAULT_CAN_COM_FUNC_DIV:
		case P1_C_CD_CAN_FAULT_UPPER:
		case P1_C_CD_CAN_FAULT_LOWER:
		case P1_C_CD_FAULT_CAN_FUNC_DIV:
		case P1_C_CD_FAULT_CAN_BMS_PAUSE:
		case P1_C_CD_CAN_FAULT_V_SVDF:
		case P1_C_CD_CAN_FAULT_DROP_VOLTAGE:
		case P1_C_CD_CAN_FAULT_COMP_V:
		case P1_C_CD_FAULT_UPPER_TEMP:
		case P1_C_CD_FAULT_COMP_GROUP_AUX_V1:	//ktg_220220s
		case P1_C_CD_FAULT_COMP_GROUP_AUX_V2:
		case P1_C_CD_FAULT_COMP_GROUP_AUX_V3:
		case P1_C_CD_FAULT_DELTA_AUX_V:			//ktg_220220e
		case P1_C_CD_AUX_FAULT_SOFTVENTING_GAS:	//ktg_220331s
		case P1_C_CD_AUX_FAULT_HARDVENTING_GAS:
		case P1_C_CD_CAN_FAULT_SOFTVENTING_GAS:
		case P1_C_CD_CAN_FAULT_HARDVENTING_GAS:	//ktg_220331e
		case P1_C_CD_CAN_FAULT_CAN_FREEZE:	//ktg_230728
		case P1_C_CD_FAULT_COMP_CAN_V:		//jhj_250514
		case P1_C_CD_FAULT_COMP_CAN_V2:
		case P1_C_CD_FAULT_COMP_CAN_V3:
			rtn_div = 1;
			break;
		default :
			rtn_div = 0;
			break;
	}
	return rtn_div;
}
//jhkw_220103e

long convert_group_state(int select, long state)
{
	long rtn_state=0;

	if(select == CONVERT_P1_TO_ORG) {
	} else if(select == CONVERT_ORG_TO_P1) {
	} else if(select == CONVERT_P2_TO_ORG) {
	} else if(select == CONVERT_ORG_TO_P2) {
#if defined __COB__
		switch(state) {
			case G_IDLE:	rtn_state = P2_G_IDLE;		break;
			case G_STANDBY:	rtn_state = P2_G_STANDBY;	break;
			case G_RUN:		rtn_state = P2_G_RUN;		break;
			case G_PAUSE:	rtn_state = P2_G_PAUSE;		break;
			case G_CALI:	rtn_state = P2_G_MAINTENANCE;	break;
			default:		rtn_state = P2_G_IDLE;		break;
		}
#endif
	} else if(select == CONVERT_P3_TO_ORG) {
	} else if(select == CONVERT_ORG_TO_P3) {
	} else {
		rtn_state = 0;
	}

	return rtn_state;
}

long convert_step_type(int select, long type)
{
	long rtn_type=0;

	if(select == CONVERT_P1_TO_ORG) {
		switch(type) {
			case P1_STEP_IDLE:		rtn_type = STEP_IDLE;		break;
			case P1_STEP_CHARGE:	rtn_type = STEP_CHARGE;		break;
			case P1_STEP_DISCHARGE:	rtn_type = STEP_DISCHARGE;	break;
			case P1_STEP_REST:		rtn_type = STEP_REST;		break;
			case P1_STEP_OCV:		rtn_type = STEP_OCV;		break;
			case P1_STEP_Z:			rtn_type = STEP_Z;			break;
			case P1_STEP_END:		rtn_type = STEP_END;		break;
			case P1_STEP_CYCLE:		rtn_type = STEP_CYCLE;		break;
			case P1_STEP_LOOP:		rtn_type = STEP_LOOP;		break;
			case P1_STEP_PATTERN:	rtn_type = STEP_PATTERN;	break;
			case P1_STEP_LONG_TIME_REST:
									rtn_type = STEP_LONG_TIME_REST;	break;
			case P1_STEP_EXTERNAL_CAN:
									rtn_type = STEP_EXTERNAL_CAN;	break;
			//kjhw_140828
			case P1_STEP_USERMAP:	rtn_type = STEP_USERMAP;	break;
			case P1_STEP_EXTERNAL_CAN_2: //kjg_140916
									rtn_type = STEP_EXTERNAL_CAN_2;	break;
			default:				rtn_type = STEP_IDLE;		break;
		}
	} else if(select == CONVERT_ORG_TO_P1) {
		switch(type) {
			case STEP_IDLE:			rtn_type = P1_STEP_IDLE;		break;
			case STEP_CHARGE:		rtn_type = P1_STEP_CHARGE;		break;
			case STEP_DISCHARGE:	rtn_type = P1_STEP_DISCHARGE;	break;
			case STEP_REST:			rtn_type = P1_STEP_REST;		break;
			case STEP_OCV:			rtn_type = P1_STEP_OCV;			break;
			case STEP_Z:			rtn_type = P1_STEP_Z;			break;
			case STEP_END:			rtn_type = P1_STEP_END;			break;
			case STEP_CYCLE:		rtn_type = P1_STEP_CYCLE;		break;
			case STEP_LOOP:			rtn_type = P1_STEP_LOOP;		break;
			case STEP_PATTERN:		rtn_type = P1_STEP_PATTERN;		break;
			case STEP_LONG_TIME_REST:
									rtn_type = P1_STEP_LONG_TIME_REST;	break;
			case STEP_EXTERNAL_CAN:	rtn_type = P1_STEP_EXTERNAL_CAN;	break;
			//kjhw_140828
			case STEP_USERMAP:		rtn_type = P1_STEP_USERMAP;		break;
			case STEP_EXTERNAL_CAN_2: //kjg_140916
									rtn_type = P1_STEP_EXTERNAL_CAN_2;	break;
			default:				rtn_type = P1_STEP_IDLE;		break;
		}
	} else if(select == CONVERT_P2_TO_ORG) {
#if defined __COB__
		switch(type) {
			case P2_STEP_IDLE:		rtn_type = STEP_IDLE;		break;
			case P2_STEP_CHARGE:	rtn_type = STEP_CHARGE;		break;
			case P2_STEP_DISCHARGE:	rtn_type = STEP_DISCHARGE;	break;
			case P2_STEP_REST:		rtn_type = STEP_REST;		break;
			case P2_STEP_OCV:		rtn_type = STEP_OCV;		break;
			case P2_STEP_Z:			rtn_type = STEP_Z;			break;
			case P2_STEP_END:		rtn_type = STEP_END;		break;
			case P2_STEP_CYCLE:		rtn_type = STEP_CYCLE;		break;
			case P2_STEP_LOOP:		rtn_type = STEP_LOOP;		break;
			case P2_STEP_PATTERN:	rtn_type = STEP_PATTERN;	break;
			case P2_STEP_LONG_TIME_REST:
				rtn_type = STEP_LONG_TIME_REST;
				break;
			default:				rtn_type = STEP_IDLE;		break;
		}
#endif
	} else if(select == CONVERT_ORG_TO_P2) {
#if defined __COB__
		switch(type) {
			case STEP_IDLE:			rtn_type = P2_STEP_IDLE;		break;
			case STEP_CHARGE:		rtn_type = P2_STEP_CHARGE;		break;
			case STEP_DISCHARGE:	rtn_type = P2_STEP_DISCHARGE;	break;
			case STEP_REST:			rtn_type = P2_STEP_REST;		break;
			case STEP_OCV:			rtn_type = P2_STEP_OCV;			break;
			case STEP_Z:			rtn_type = P2_STEP_Z;			break;
			case STEP_END:			rtn_type = P2_STEP_END;			break;
			case STEP_CYCLE:		rtn_type = P2_STEP_CYCLE;		break;
			case STEP_LOOP:			rtn_type = P2_STEP_LOOP;		break;
			case STEP_PATTERN:		rtn_type = P2_STEP_PATTERN;		break;
			case STEP_LONG_TIME_REST:
				rtn_type = P2_STEP_LONG_TIME_REST;
				break;
			default:				rtn_type = P2_STEP_IDLE;		break;
		}
#endif
	} else if(select == CONVERT_P3_TO_ORG) {
#if defined __COC__
		switch(type) {
			case P3_STEP_IDLE:		rtn_type = STEP_IDLE;		break;
			case P3_STEP_CHARGE:	rtn_type = STEP_CHARGE;		break;
			case P3_STEP_DISCHARGE:	rtn_type = STEP_DISCHARGE;	break;
			case P3_STEP_REST:		rtn_type = STEP_REST;		break;
			case P3_STEP_OCV:		rtn_type = STEP_OCV;		break;
			case P3_STEP_Z:			rtn_type = STEP_Z;			break;
			case P3_STEP_END:		rtn_type = STEP_END;		break;
			case P3_STEP_CYCLE:		rtn_type = STEP_CYCLE;		break;
			case P3_STEP_LOOP:		rtn_type = STEP_LOOP;		break;
			case P3_STEP_PATTERN:	rtn_type = STEP_PATTERN;	break;
			default:				rtn_type = STEP_IDLE;		break;
		}
#endif
	} else if(select == CONVERT_ORG_TO_P3) {
#if defined __COC__
		switch(type) {
			case STEP_IDLE:			rtn_type = P3_STEP_IDLE;		break;
			case STEP_CHARGE:		rtn_type = P3_STEP_CHARGE;		break;
			case STEP_DISCHARGE:	rtn_type = P3_STEP_DISCHARGE;	break;
			case STEP_REST:			rtn_type = P3_STEP_REST;		break;
			case STEP_OCV:			rtn_type = P3_STEP_OCV;			break;
			case STEP_Z:			rtn_type = P3_STEP_Z;			break;
			case STEP_END:			rtn_type = P3_STEP_END;			break;
			case STEP_CYCLE:		rtn_type = P3_STEP_CYCLE;		break;
			case STEP_LOOP:			rtn_type = P3_STEP_LOOP;		break;
			case STEP_PATTERN:		rtn_type = P3_STEP_PATTERN;		break;
			default:				rtn_type = P3_STEP_IDLE;		break;
		}
#endif
	} else {
		rtn_type = 0;
	}

	return rtn_type;
}

long convert_step_mode(int select, long mode)
{
	long rtn_mode=0;

	if(select == CONVERT_P1_TO_ORG) {
		switch(mode) {
			case P1_MODE_IDLE:	rtn_mode = MODE_IDLE;		break;
			case P1_MODE_CC_CV:	rtn_mode = MODE_CC_CV;		break;
			case P1_MODE_CC:	rtn_mode = MODE_CC;			break;
			case P1_MODE_CV:	rtn_mode = MODE_CV;			break;
			case P1_MODE_DC:	rtn_mode = MODE_DC;			break;
			case P1_MODE_AC:	rtn_mode = MODE_AC;			break;
			case P1_MODE_CP:	rtn_mode = MODE_CP;			break;
			case P1_MODE_CCP:	rtn_mode = MODE_CCP;		break;
			case P1_MODE_CR:	rtn_mode = MODE_CR;			break;
			case P1_MODE_CP_CC:	rtn_mode = MODE_CP_CC;		break;
			case P1_MODE_CC_CP:	rtn_mode = MODE_CC_CP;		break; //kjh_160623
			default:			rtn_mode = MODE_IDLE;		break;
		}
	} else if(select == CONVERT_ORG_TO_P1) {
		switch(mode) {
			case MODE_IDLE:		rtn_mode = P1_MODE_IDLE;	break;
			case MODE_CC_CV:	rtn_mode = P1_MODE_CC_CV;	break;
			case MODE_CC:		rtn_mode = P1_MODE_CC;		break;
			case MODE_CV:		rtn_mode = P1_MODE_CV;		break;
			case MODE_DC:		rtn_mode = P1_MODE_DC;		break;
			case MODE_AC:		rtn_mode = P1_MODE_AC;		break;
			case MODE_CP:		rtn_mode = P1_MODE_CP;		break;
			case MODE_CCP:		rtn_mode = P1_MODE_CCP;		break;
			case MODE_CR:		rtn_mode = P1_MODE_CR;		break;
			case MODE_CP_CC:	rtn_mode = P1_MODE_CP_CC;	break;
			case MODE_CC_CP:	rtn_mode = P1_MODE_CC_CP;	break; //kjh_160623
			default:			rtn_mode = P1_MODE_IDLE;	break;
		}
	} else if(select == CONVERT_P2_TO_ORG) {
#if defined __COB__
		switch(mode) {
			case P2_MODE_IDLE:	rtn_mode = MODE_IDLE;		break;
			case P2_MODE_CC_CV:	rtn_mode = MODE_CC_CV;		break;
			case P2_MODE_CC:	rtn_mode = MODE_CC;			break;
			case P2_MODE_CV:	rtn_mode = MODE_CV;			break;
			case P2_MODE_DC:	rtn_mode = MODE_DC;			break;
			case P2_MODE_AC:	rtn_mode = MODE_AC;			break;
			case P2_MODE_CP:	rtn_mode = MODE_CP;			break;
			case P2_MODE_CCP:	rtn_mode = MODE_CCP;		break;
			case P2_MODE_CR:	rtn_mode = MODE_CR;			break;
			case P2_MODE_CP_CC:	rtn_mode = MODE_CP_CC;		break;
			default:			rtn_mode = MODE_IDLE;		break;
		}
#endif
	} else if(select == CONVERT_ORG_TO_P2) {
#if defined __COB__
		switch(mode) {
			case MODE_IDLE:		rtn_mode = P2_MODE_IDLE;	break;
			case MODE_CC_CV:	rtn_mode = P2_MODE_CC_CV;	break;
			case MODE_CC:		rtn_mode = P2_MODE_CC;		break;
			case MODE_CV:		rtn_mode = P2_MODE_CV;		break;
			case MODE_DC:		rtn_mode = P2_MODE_DC;		break;
			case MODE_AC:		rtn_mode = P2_MODE_AC;		break;
			case MODE_CP:		rtn_mode = P2_MODE_CP;		break;
			case MODE_CCP:		rtn_mode = P2_MODE_CCP;		break;
			case MODE_CR:		rtn_mode = P2_MODE_CR;		break;
			case MODE_CP_CC:	rtn_mode = P2_MODE_CP_CC;	break;
			default:			rtn_mode = P2_MODE_IDLE;	break;
		}
#endif
	} else if(select == CONVERT_P3_TO_ORG) {
#if defined __COC__
		switch(mode) {
			case P3_MODE_IDLE:	rtn_mode = MODE_IDLE;		break;
			case P3_MODE_CC_CV:	rtn_mode = MODE_CC_CV;		break;
			case P3_MODE_CC:	rtn_mode = MODE_CC;			break;
			case P3_MODE_CV:	rtn_mode = MODE_CV;			break;
			case P3_MODE_DC:	rtn_mode = MODE_DC;			break;
			case P3_MODE_AC:	rtn_mode = MODE_AC;			break;
			case P3_MODE_CP:	rtn_mode = MODE_CP;			break;
			case P3_MODE_CCP:	rtn_mode = MODE_CCP;		break;
			case P3_MODE_CR:	rtn_mode = MODE_CR;			break;
			case P3_MODE_CP_CC:	rtn_mode = MODE_CP_CC;		break;
			default:			rtn_mode = MODE_IDLE;		break;
		}
#endif
	} else if(select == CONVERT_ORG_TO_P3) {
#if defined __COC__
		switch(mode) {
			case MODE_IDLE:		rtn_mode = P3_MODE_IDLE;	break;
			case MODE_CC_CV:	rtn_mode = P3_MODE_CC_CV;	break;
			case MODE_CC:		rtn_mode = P3_MODE_CC;		break;
			case MODE_CV:		rtn_mode = P3_MODE_CV;		break;
			case MODE_DC:		rtn_mode = P3_MODE_DC;		break;
			case MODE_AC:		rtn_mode = P3_MODE_AC;		break;
			case MODE_CP:		rtn_mode = P3_MODE_CP;		break;
			case MODE_CCP:		rtn_mode = P3_MODE_CCP;		break;
			case MODE_CR:		rtn_mode = P3_MODE_CR;		break;
			case MODE_CP_CC:	rtn_mode = P3_MODE_CP_CC;	break;
			default:			rtn_mode = P3_MODE_IDLE;	break;
		}
#endif
	} else {
		rtn_mode = 0;
	}

	return rtn_mode;
}

long convert_ch_state(int select, long state)
{
	long rtn_state=0;

	if(select == CONVERT_P1_TO_ORG) {
	} else if(select == CONVERT_ORG_TO_P1) {
		switch(state) {
			case C_IDLE:	rtn_state = P1_C_IDLE;		break;
			case C_STANDBY:	rtn_state = P1_C_STANDBY;	break;
			case C_PAUSE:	rtn_state = P1_C_PAUSE;		break;
			case C_CALI:	rtn_state = P1_C_CALI;		break;
			case C_FAULT:	rtn_state = P1_C_FAULT;		break;
			case C_RUN:		rtn_state = P1_C_RUN;		break;
			default: 		rtn_state = P1_C_IDLE;		break;
		}
	} else if(select == CONVERT_P3_TO_ORG) {
	} else if(select == CONVERT_ORG_TO_P3) {
#if defined __COC__
		switch(state) {
			case C_IDLE:	rtn_state = P3_C_IDLE;		break;
			case C_STANDBY:	rtn_state = P3_C_STANDBY;	break;
			case C_PAUSE:	rtn_state = P3_C_PAUSE;		break;
			case C_CALI:	rtn_state = P3_C_CALI;		break;
			case C_FAULT:	rtn_state = P3_C_FAULT;		break;
			case C_RUN:		rtn_state = P3_C_RUN;		break;
			default: 		rtn_state = P3_C_IDLE;		break;
		}
#endif
	} else {
		rtn_state = 0;
	}

	return rtn_state;
}

long convert_ch_state_org_to_p2(long state, long type, long attribute, long phase)
{
	long rtn_state=0;

#if defined __COB__
	switch(state) {
		case C_IDLE:
			rtn_state = P2_C_IDLE;
			break;
		case C_STANDBY:
			rtn_state = P2_C_STANDBY;
			break;
		case C_PAUSE:
			if(phase < P100) {
				rtn_state = P2_C_PAUSE;
				break;
			}

			switch(type) {
				case STEP_IDLE:
					rtn_state = P2_C_STOP;
					break;
				case STEP_OCV:
					rtn_state = P2_C_OCV;
					break;
				case STEP_CYCLE:
				case STEP_LOOP:
					if((attribute >= ATTR_CHECK1_START
						&& attribute <= ATTR_CHECK1_END)
						|| (attribute >= ATTR_CHECK2_START
						&& attribute <= ATTR_CHECK2_END)
						|| (attribute >= ATTR_CHECK3_START
						&& attribute <= ATTR_CHECK3_END)) {
						rtn_state = P2_C_CHECK;
					} else {
						rtn_state = P2_C_STOP;
					}
					break;
				case STEP_END:
					rtn_state = P2_C_END;
					break;
				case STEP_CHARGE:
					if(attribute >= ATTR_DC1_START
						&& attribute <= ATTR_DC1_END) {
						rtn_state = P2_C_Z;
					} else if(attribute >= ATTR_DC2_START
						&& attribute <= ATTR_DC2_END) {
						rtn_state = P2_C_Z;
					} else if(attribute >= ATTR_DC3_START
						&& attribute <= ATTR_DC3_END) {
						rtn_state = P2_C_Z;
					} else {
						rtn_state = P2_C_CHARGE;
					}
					break;
				case STEP_DISCHARGE:
					if(attribute >= ATTR_DC1_START
						&& attribute <= ATTR_DC1_END) {
						rtn_state = P2_C_Z;
					} else if(attribute >= ATTR_DC2_START
						&& attribute <= ATTR_DC2_END) {
						rtn_state = P2_C_Z;
					} else if(attribute >= ATTR_DC3_START
						&& attribute <= ATTR_DC3_END) {
						rtn_state = P2_C_Z;
					} else {
						rtn_state = P2_C_DISCHARGE;
					}
					break;
				case STEP_REST:
					if(attribute >= ATTR_DC1_START
						&& attribute <= ATTR_DC1_END) {
						rtn_state = P2_C_Z;
					} else if(attribute >= ATTR_DC2_START
						&& attribute <= ATTR_DC2_END) {
						rtn_state = P2_C_Z;
					} else if(attribute >= ATTR_DC3_START
						&& attribute <= ATTR_DC3_END) {
						rtn_state = P2_C_Z;
					} else {
						rtn_state = P2_C_REST;
					}
					break;
				case STEP_LONG_TIME_REST:
					if(attribute >= ATTR_DC1_START
						&& attribute <= ATTR_DC1_END) {
						rtn_state = P2_C_Z;
					} else if(attribute >= ATTR_DC2_START
						&& attribute <= ATTR_DC2_END) {
						rtn_state = P2_C_Z;
					} else if(attribute >= ATTR_DC3_START
						&& attribute <= ATTR_DC3_END) {
						rtn_state = P2_C_Z;
					} else {
						rtn_state = P2_C_LONG_TIME_REST;
					}
					break;
				default:
					rtn_state = P2_C_STOP;
					break;
			}
			break;
		case C_CALI:
			rtn_state = P2_C_MAINTERNANCE;
			break;
		case C_FAULT:
			rtn_state = P2_C_FAULT;
			break;
		case C_RUN:
			switch(type) {
				case STEP_IDLE:
					rtn_state = P2_C_STOP;
					break;
				case STEP_OCV:
					if((attribute >= ATTR_CHECK1_START
						&& attribute <= ATTR_CHECK1_END)
						|| (attribute >= ATTR_CHECK2_START
						&& attribute <= ATTR_CHECK2_END)
						|| (attribute >= ATTR_CHECK3_START
						&& attribute <= ATTR_CHECK3_END)) {
						rtn_state = P2_C_CHECK;
					} else {
						rtn_state = P2_C_OCV;
					}
					break;
				case STEP_CYCLE:
				case STEP_LOOP:
					if((attribute >= ATTR_CHECK1_START
						&& attribute <= ATTR_CHECK1_END)
						|| (attribute >= ATTR_CHECK2_START
						&& attribute <= ATTR_CHECK2_END)
						|| (attribute >= ATTR_CHECK3_START
						&& attribute <= ATTR_CHECK3_END)) {
						rtn_state = P2_C_CHECK;
					} else {
						rtn_state = P2_C_STOP;
					}
					break;
				case STEP_END:
					rtn_state = P2_C_END;
					break;
				case STEP_CHARGE:
					if((attribute >= ATTR_CHECK1_START
						&& attribute <= ATTR_CHECK1_END)
						|| (attribute >= ATTR_CHECK2_START
						&& attribute <= ATTR_CHECK2_END)
						|| (attribute >= ATTR_CHECK3_START
						&& attribute <= ATTR_CHECK3_END)) {
						rtn_state = P2_C_CHECK;
					} else if(attribute >= ATTR_DC1_START
						&& attribute <= ATTR_DC1_END) {
						rtn_state = P2_C_Z;
					} else if(attribute >= ATTR_DC2_START
						&& attribute <= ATTR_DC2_END) {
						rtn_state = P2_C_Z;
					} else if(attribute >= ATTR_DC3_START
						&& attribute <= ATTR_DC3_END) {
						rtn_state = P2_C_Z;
					} else {
						rtn_state = P2_C_CHARGE;
					}
					break;
				case STEP_DISCHARGE:
					if((attribute >= ATTR_CHECK1_START
						&& attribute <= ATTR_CHECK1_END)
						|| (attribute >= ATTR_CHECK2_START
						&& attribute <= ATTR_CHECK2_END)
						|| (attribute >= ATTR_CHECK3_START
						&& attribute <= ATTR_CHECK3_END)) {
						rtn_state = P2_C_CHECK;
					} else if(attribute >= ATTR_DC1_START
						&& attribute <= ATTR_DC1_END) {
						rtn_state = P2_C_Z;
					} else if(attribute >= ATTR_DC2_START
						&& attribute <= ATTR_DC2_END) {
						rtn_state = P2_C_Z;
					} else if(attribute >= ATTR_DC3_START
						&& attribute <= ATTR_DC3_END) {
						rtn_state = P2_C_Z;
					} else {
						rtn_state = P2_C_DISCHARGE;
					}
					break;
				case STEP_REST:
					if((attribute >= ATTR_CHECK1_START
						&& attribute <= ATTR_CHECK1_END)
						|| (attribute >= ATTR_CHECK2_START
						&& attribute <= ATTR_CHECK2_END)
						|| (attribute >= ATTR_CHECK3_START
						&& attribute <= ATTR_CHECK3_END)) {
						rtn_state = P2_C_CHECK;
					} else if(attribute >= ATTR_DC1_START
						&& attribute <= ATTR_DC1_END) {
						rtn_state = P2_C_Z;
					} else if(attribute >= ATTR_DC2_START
						&& attribute <= ATTR_DC2_END) {
						rtn_state = P2_C_Z;
					} else if(attribute >= ATTR_DC3_START
						&& attribute <= ATTR_DC3_END) {
						rtn_state = P2_C_Z;
					} else {
						rtn_state = P2_C_REST;
					}
					break;
				case STEP_LONG_TIME_REST:
					if((attribute >= ATTR_CHECK1_START
						&& attribute <= ATTR_CHECK1_END)
						|| (attribute >= ATTR_CHECK2_START
						&& attribute <= ATTR_CHECK2_END)
						|| (attribute >= ATTR_CHECK3_START
						&& attribute <= ATTR_CHECK3_END)) {
						rtn_state = P2_C_CHECK;
					} else if(attribute >= ATTR_DC1_START
						&& attribute <= ATTR_DC1_END) {
						rtn_state = P2_C_Z;
					} else if(attribute >= ATTR_DC2_START
						&& attribute <= ATTR_DC2_END) {
						rtn_state = P2_C_Z;
					} else if(attribute >= ATTR_DC3_START
						&& attribute <= ATTR_DC3_END) {
						rtn_state = P2_C_Z;
					} else {
						rtn_state = P2_C_LONG_TIME_REST;
					}
					break;
				default:
					rtn_state = P2_C_STOP;
					break;
			}
			break;
		default:
			rtn_state = P2_C_IDLE;
			break;
	}
#endif
	return rtn_state;
}

long convert_ch_code(int select, long code)
{
	long rtn_code=0;

	if(select == CONVERT_P1_TO_ORG) {
	} else if(select == CONVERT_ORG_TO_P1) {
		rtn_code = convert_ch_code_org_to_p1(code);
	} else if(select == CONVERT_P2_TO_ORG) {
	} else if(select == CONVERT_ORG_TO_P2) {
		rtn_code = convert_ch_code_org_to_p2(code);
	} else if(select == CONVERT_P3_TO_ORG) {
	} else if(select == CONVERT_ORG_TO_P3) {
		rtn_code = convert_ch_code_org_to_p3(code);
	} else {
		rtn_code = 0;
	}

	return rtn_code;
}

long convert_ch_code_org_to_p1(long code)
{
	long rtn_code=0;

	switch(code) {
		case C_CD_NONE:
			rtn_code = P1_C_CD_NONE;						break;

		//common code(hard)
		case C_CD_COM_FAULT_HW_UPPER_V:
			rtn_code = P1_C_CD_FAULT_CH_V_FAIL;				break;
		case C_CD_COM_FAULT_HW_LOWER_V:
			rtn_code = P1_C_CD_FAULT_CH_V_FAIL;				break;
		case C_CD_COM_FAULT_HW_UPPER_I:
			rtn_code = P1_C_CD_FAULT_CH_I_FAIL;				break;
		case C_CD_COM_FAULT_HW_LOWER_I:
			rtn_code = P1_C_CD_FAULT_CH_I_FAIL;				break;
		case C_CD_COM_FAULT_HW_UPPER_TEMP:
			rtn_code = P1_C_CD_FAULT_OT;					break;
		case C_CD_COM_FAULT_HW_LOWER_TEMP:
			rtn_code = P1_C_CD_FAULT_OT;					break;

		//common code
		case C_CD_COM_FAULT_UPPER_V:
			rtn_code = P1_C_CD_FAULT_UPPER_V;				break;
		case C_CD_COM_FAULT_LOWER_V:
			rtn_code = P1_C_CD_FAULT_LOWER_V;				break;
		case C_CD_COM_FAULT_UPPER_I:
			rtn_code = P1_C_CD_FAULT_UPPER_I;				break;
		case C_CD_COM_FAULT_LOWER_I:
			rtn_code = P1_C_CD_FAULT_LOWER_I;				break;
		case C_CD_COM_FAULT_UPPER_AMPARE_HOUR:
			rtn_code = P1_C_CD_FAULT_UPPER_AMPARE_HOUR;		break;
		case C_CD_COM_FAULT_LOWER_AMPARE_HOUR:
			rtn_code = P1_C_CD_FAULT_LOWER_AMPARE_HOUR;		break;
		case C_CD_COM_FAULT_UPPER_WATT_HOUR:
			rtn_code = P1_C_CD_FAULT_UPPER_WATT_HOUR;		break;
		case C_CD_COM_FAULT_LOWER_WATT_HOUR:
			rtn_code = P1_C_CD_FAULT_LOWER_WATT_HOUR;		break;
		case C_CD_COM_FAULT_UPPER_P:
			rtn_code = P1_C_CD_FAULT_UPPER_P;				break;
		case C_CD_COM_FAULT_LOWER_P:
			rtn_code = P1_C_CD_FAULT_LOWER_P;				break;
		case C_CD_COM_FAULT_UPPER_TEMP:
			rtn_code = P1_C_CD_FAULT_UPPER_TEMP;			break;
		case C_CD_COM_FAULT_LOWER_TEMP:
			rtn_code = P1_C_CD_FAULT_LOWER_TEMP;			break;
		case C_CD_COM_FAULT_UPPER_CAPACITANCE:
			rtn_code = P1_C_CD_FAULT_UPPER_CAPACITANCE;		break;
		case C_CD_COM_FAULT_LOWER_CAPACITANCE:
			rtn_code = P1_C_CD_FAULT_LOWER_CAPACITANCE;		break;
		case C_CD_COM_FAULT_COMP_AUX_V:	//kjhw_150730
			rtn_code = P1_C_CD_FAULT_COMP_AUX_V;			break;
		case C_CD_COM_FAULT_COMP_AUX_V2:	//csk_190808
			rtn_code = P1_C_CD_FAULT_COMP_AUX_V2;			break;
		case C_CD_COM_FAULT_COMP_AUX_V3:	//csk_190808
			rtn_code = P1_C_CD_FAULT_COMP_AUX_V3;			break;
		case C_CD_COM_FAULT_GROUP_COMP_AUX_V1: //ktg_220220s
			rtn_code = P1_C_CD_FAULT_COMP_GROUP_AUX_V1;			break;
		case C_CD_COM_FAULT_GROUP_COMP_AUX_V2:
			rtn_code = P1_C_CD_FAULT_COMP_GROUP_AUX_V2;			break;
		case C_CD_COM_FAULT_GROUP_COMP_AUX_V3:
			rtn_code = P1_C_CD_FAULT_COMP_GROUP_AUX_V3;			break;	//ktg_220220e
		case C_CD_COM_FAULT_COMP_CAN_V:									//ktg_241008
			rtn_code = P1_C_CD_FAULT_COMP_CAN_V;			break;
		case C_CD_COM_FAULT_COMP_CAN_V2:
			rtn_code = P1_C_CD_FAULT_COMP_CAN_V2;			break;
		case C_CD_COM_FAULT_COMP_CAN_V3:
			rtn_code = P1_C_CD_FAULT_COMP_CAN_V3;			break;
		case C_CD_COM_FAULT_TEMP_PAUSE:	//jhkw_160123
			rtn_code = P1_C_CD_FAULT_TEMP_PAUSE;			break;
		case C_CD_COM_FAULT_TEMP_CONTINUE:	//jhkw_160123
			rtn_code = P1_C_CD_FAULT_TEMP_CONTINUE;			break;
		case C_CD_COM_FAULT_DELTA_V:	//jhkw_160828
			rtn_code = P1_C_CD_FAULT_DELTA_V;				break;
		case C_CD_COM_FAULT_AUX_UPPER_V:	//kjhw_181111
			rtn_code = P1_C_CD_FAULT_AUX_UPPER_V;			break;
		case C_CD_COM_FAULT_AUX_LOWER_V:	//kjhw_181111
			rtn_code = P1_C_CD_FAULT_AUX_LOWER_V;			break;
		case C_CD_COM_CAN_FAULT_FUNC_DIV_1:
		case C_CD_COM_CAN_FAULT_FUNC_DIV_2:
		case C_CD_COM_CAN_FAULT_FUNC_DIV_3:
		case C_CD_COM_CAN_FAULT_FUNC_DIV_4:
		case C_CD_COM_CAN_FAULT_FUNC_DIV_5:
		case C_CD_COM_CAN_FAULT_FUNC_DIV_6:
		case C_CD_COM_CAN_FAULT_FUNC_DIV_7:
		case C_CD_COM_CAN_FAULT_FUNC_DIV_8:
		case C_CD_COM_CAN_FAULT_FUNC_DIV_9:
		case C_CD_COM_CAN_FAULT_FUNC_DIV_10:
			rtn_code = P1_C_CD_FAULT_CAN_COM_FUNC_DIV;		break;
		case C_CD_COM_AUX_FAULT_FUNC_DIV_1:
		case C_CD_COM_AUX_FAULT_FUNC_DIV_2:
		case C_CD_COM_AUX_FAULT_FUNC_DIV_3:
		case C_CD_COM_AUX_FAULT_FUNC_DIV_4:
		case C_CD_COM_AUX_FAULT_FUNC_DIV_5:
		case C_CD_COM_AUX_FAULT_FUNC_DIV_6:
		case C_CD_COM_AUX_FAULT_FUNC_DIV_7:
		case C_CD_COM_AUX_FAULT_FUNC_DIV_8:
		case C_CD_COM_AUX_FAULT_FUNC_DIV_9:
		case C_CD_COM_AUX_FAULT_FUNC_DIV_10:
			rtn_code = P1_C_CD_FAULT_AUX_COM_FUNC_DIV;		break;

		//end code
		case C_CD_END_TIME:
			rtn_code = P1_C_CD_END_TIME;					break;
		case C_CD_END_V_UPPER:
			rtn_code = P1_C_CD_END_V_UPPER;					break;
		case C_CD_END_I:
			rtn_code = P1_C_CD_END_I;						break;
		case C_CD_END_AMPARE_HOUR:
			rtn_code = P1_C_CD_END_AMPARE_HOUR;				break;
		case C_CD_END_OCV:
			rtn_code = P1_C_CD_END_OCV;						break;
		case C_CD_END_STEP:
			rtn_code = P1_C_CD_END_STEP;					break;
		case C_CD_END_CHECK:
			rtn_code = P1_C_CD_END_CHECK;					break;
		case C_CD_END_NEXT_STEP_CMD:
			rtn_code = P1_C_CD_END_NEXT_STEP_CMD;			break;
		case C_CD_END_CYCLE:
			rtn_code = P1_C_CD_END_CYCLE;					break;
		case C_CD_END_LOOP:
			rtn_code = P1_C_CD_END_LOOP;					break;
		case C_CD_END_Z:
			rtn_code = P1_C_CD_END_Z;						break;
		case C_CD_END_DELTA_V:
			rtn_code = P1_C_CD_END_DELTA_V;					break;
		case C_CD_END_DELTA_I:
			rtn_code = P1_C_CD_END_DELTA_I;					break;
		case C_CD_END_VALUE_RATE_AMPARE_HOUR:
			rtn_code = P1_C_CD_END_VALUE_RATE_AMPARE_HOUR;	break;
		case C_CD_END_P:
			rtn_code = P1_C_CD_END_P;						break;
		case C_CD_END_WATT_HOUR:
			rtn_code = P1_C_CD_END_WATT_HOUR;				break;
		case C_CD_END_V_LOWER:
			rtn_code = P1_C_CD_END_V_LOWER;					break;
		case C_CD_END_CV_TIME:
			rtn_code = P1_C_CD_END_CV_TIME;					break;
		case C_CD_END_GOTO_STEP_CMD:
			rtn_code = P1_C_CD_END_GOTO_STEP_CMD;			break;
		case C_CD_END_STOP_CMD:
			rtn_code = P1_C_CD_END_STOP_CMD;				break;
		case C_CD_END_VALUE_RATE_WATT_HOUR:
			rtn_code = P1_C_CD_END_VALUE_RATE_WATT_HOUR;	break;
		case C_CD_END_SUM_AMPARE_HOUR:
			rtn_code = P1_C_CD_END_SUM_AMPARE_HOUR;			break;
		case C_CD_END_SUM_WATT_HOUR:
			rtn_code = P1_C_CD_END_SUM_WATT_HOUR;			break;
		case C_CD_END_SUM_TIME:
			rtn_code = P1_C_CD_END_SUM_TIME;				break;
		case C_CD_END_TEMP_UPPER:
		case C_CD_END_TEMP_LOWER:
			rtn_code = P1_C_CD_END_TEMP;					break;
		case C_CD_CHAMBER_CONTROL_WAIT:
			rtn_code = P1_C_CD_CHAMBER_CONTROL_WAIT;		break;
		case C_CD_CYCLE_CONTROL_WAIT:
			rtn_code = P1_C_CD_CYCLE_CONTROL_WAIT;			break;
		case C_CD_END_INTERNAL_SKIP:
			rtn_code = P1_C_CD_END_INTERNAL_SKIP;			break;
		case C_CD_END_TIMESCH_TIME:	//kjh_160418
			rtn_code = P1_C_CD_END_TIMESCH_TIME;			break;
		case C_CD_END_TEST_COND_UPDATE:	//kjg_170810
			rtn_code = P1_C_CD_END_TEST_COND_UPDATE;		break;
		case C_CD_END_VALUE_RATE_CAN_AMPARE_HOUR:	//ktg_220829
			rtn_code = P1_C_CD_END_CAN_RATE_AMPARE_HOUR;	break;
		case C_CD_END_VALUE_RATE_CAN_WATT_HOUR:
			rtn_code = P1_C_CD_END_CAN_RATE_WATT_HOUR;		break;

		//save code
		case C_CD_SAVE_TEST_COND_UPDATE:	//kjg_170810
			rtn_code = P1_C_CD_SAVE_TEST_COND_UPDATE;		break;

		//hard fault code
		case C_CD_FAULT_HW_UPPER_V:
		case C_CD_FAULT_HW_LOWER_V:
			rtn_code = P1_C_CD_FAULT_CH_V_FAIL;				break;
		case C_CD_FAULT_HW_UPPER_I:
		case C_CD_FAULT_HW_LOWER_I:
			rtn_code = P1_C_CD_FAULT_CH_I_FAIL;				break;
		case C_CD_FAULT_HW_OT:
			rtn_code = P1_C_CD_FAULT_OT;					break;
		case C_CD_FAULT_RANGE:	//jhkw_130416
			rtn_code = P1_C_CD_FAULT_RANGE;					break;
		case C_CD_FAULT_CHANNEL_PS:
			rtn_code = P1_C_CD_FAULT_CHANNEL_PS;			break;
		case C_CD_FAULT_PS_OV:
			rtn_code = P1_C_CD_FAULT_PS_OV;					break;
		case C_CD_FAULT_PS_OC:
			rtn_code = P1_C_CD_FAULT_PS_OC;					break;
		case C_CD_FAULT_PS_DCOV:
			rtn_code = P1_C_CD_FAULT_PS_DCOV;				break;
		case C_CD_FAULT_IN_FUSE:
			rtn_code = P1_C_CD_FAULT_IN_FUSE;				break;
		case C_CD_FAULT_OUT_FUSE:
			rtn_code = P1_C_CD_FAULT_OUT_FUSE;				break;
		case C_CD_FAULT_OUT_REACTOR_OT:
			rtn_code = P1_C_CD_FAULT_OUT_REACTOR_OT;		break;
		case C_CD_FAULT_POWER_TRANS_OT:
			rtn_code = P1_C_CD_FAULT_POWER_TRANS_OT;		break;
		case C_CD_FAULT_POWER_REACTOR_OT:
			rtn_code = P1_C_CD_FAULT_POWER_REACTOR_OT;		break;
		case C_CD_FAULT_OC:	//jhkw_131011
			rtn_code = P1_C_CD_FAULT_OC;					break;
		case C_CD_FAULT_ETC:	//jhkw_131011
			rtn_code = P1_C_CD_FAULT_ETC;					break;
		case C_CD_FAULT_READ_PATTERN_FILE:	//kjg_170810
			rtn_code = P1_C_CD_FAULT_PATTERN_READ;			break;
		case C_CD_FAULT_READ_USERMAP_FILE:	//kjg_170810
			rtn_code = P1_C_CD_FAULT_USERMAP_READ;			break;
		case C_CD_FAULT_LEAKAGE_CURRENT:	//ktg_211104
			rtn_code = P1_C_CD_FAULT_LEAKAGE_CURRENT;		break;

		//external fault code
		case C_CD_FAULT_M_CPU_WATCHDOG:
			rtn_code = P1_C_CD_FAULT_CPU_WATCHDOG;			break;
		case C_CD_FAULT_M_AC_POWER:
			rtn_code = P1_C_CD_FAULT_AC_POWER;				break;
		case C_CD_FAULT_M_UPS_BATTERY:
			rtn_code = P1_C_CD_FAULT_UPS_BATTERY;			break;
		case C_CD_FAULT_M_EMG_SWITCH:
			rtn_code = P1_C_CD_FAULT_MAIN_EMG;				break;
		case C_CD_FAULT_M_FORCE_POWER_SWITCH:
			rtn_code = P1_C_CD_FAULT_FORCE_POWER_SWITCH;	break;
		case C_CD_FAULT_M_FORCE_TERMINAL_HALT:
			rtn_code = P1_C_CD_FAULT_FORCE_TERMINAL_HALT;	break;
		case C_CD_FAULT_M_FORCE_TERMINAL_QUIT:
			rtn_code = P1_C_CD_FAULT_FORCE_TERMINAL_QUIT;	break;
		case C_CD_FAULT_G_NETWORK_COMM:
			rtn_code = P1_C_CD_FAULT_NETWORK_COMM;			break;
		case C_CD_FAULT_B_UPPER_TEMP:
			rtn_code = P1_C_CD_FAULT_OT;					break;
		case C_CD_FAULT_B_ADC:
			rtn_code = P1_C_CD_FAULT_ADC;					break;
		case C_CD_FAULT_CONTROL_PS:
			rtn_code = P1_C_CD_FAULT_CONTROL_PS;			break;
		case C_CD_FAULT_CHAMBER_FAULT:
			rtn_code = P1_C_CD_FAULT_CHAMBER_ERROR;			break;

		//check fault code
		case C_CD_FAULT_CHECK_NG:
			rtn_code = P1_C_CD_FAULT_CHECK_CONTACT_BAD;		break;
		case C_CD_FAULT_CHECK_UPPER_OCV:
			rtn_code = P1_C_CD_FAULT_CHECK_UPPER_OCV;		break;
		case C_CD_FAULT_CHECK_LOWER_OCV:
			rtn_code = P1_C_CD_FAULT_CHECK_LOWER_OCV;		break;
		case C_CD_FAULT_CHECK_UPPER_V:
			rtn_code = P1_C_CD_FAULT_CHECK_UPPER_V;			break;
		case C_CD_FAULT_CHECK_LOWER_V:
			rtn_code = P1_C_CD_FAULT_CHECK_LOWER_V;			break;
		case C_CD_FAULT_CHECK_UPPER_I:
			rtn_code = P1_C_CD_FAULT_CHECK_UPPER_I;			break;
		case C_CD_FAULT_CHECK_LOWER_I:
			rtn_code = P1_C_CD_FAULT_CHECK_LOWER_I;			break;
		case C_CD_FAULT_CHECK_ERROR_NO:
			rtn_code = P1_C_CD_FAULT_CHECK_ERROR_NO;		break;
		case C_CD_FAULT_CHECK_ERROR_YES:
			rtn_code = P1_C_CD_FAULT_CHECK_ERROR_YES;		break;

		//soft fault code
		case C_CD_FAULT_UPPER_V:
			rtn_code = P1_C_CD_FAULT_UPPER_V;				break;
		case C_CD_FAULT_LOWER_V:
			rtn_code = P1_C_CD_FAULT_LOWER_V;				break;
		case C_CD_FAULT_UPPER_DELTA_V:
			rtn_code = P1_C_CD_FAULT_UPPER_DELTA_V;			break;
		case C_CD_FAULT_LOWER_DELTA_V:
			rtn_code = P1_C_CD_FAULT_LOWER_DELTA_V;			break;
		case C_CD_FAULT_UPPER_COMP_V1:
			rtn_code = P1_C_CD_FAULT_UPPER_COMP_V1;			break;
		case C_CD_FAULT_LOWER_COMP_V1:
			rtn_code = P1_C_CD_FAULT_LOWER_COMP_V1;			break;
		case C_CD_FAULT_UPPER_COMP_V2:
			rtn_code = P1_C_CD_FAULT_UPPER_COMP_V2;			break;
		case C_CD_FAULT_LOWER_COMP_V2:
			rtn_code = P1_C_CD_FAULT_LOWER_COMP_V2;			break;
		case C_CD_FAULT_UPPER_COMP_V3:
			rtn_code = P1_C_CD_FAULT_UPPER_COMP_V3;			break;
		case C_CD_FAULT_LOWER_COMP_V3:
			rtn_code = P1_C_CD_FAULT_LOWER_COMP_V3;			break;
		case C_CD_FAULT_UPPER_OCV:
			rtn_code = P1_C_CD_FAULT_UPPER_OCV;				break;
		case C_CD_FAULT_LOWER_OCV:
			rtn_code = P1_C_CD_FAULT_LOWER_OCV;				break;
		case C_CD_FAULT_UPPER_I:
			rtn_code = P1_C_CD_FAULT_UPPER_I;				break;
		case C_CD_FAULT_LOWER_I:
			rtn_code = P1_C_CD_FAULT_LOWER_I;				break;
		case C_CD_FAULT_UPPER_DELTA_I:
			rtn_code = P1_C_CD_FAULT_UPPER_DELTA_I;			break;
		case C_CD_FAULT_LOWER_DELTA_I:
			rtn_code = P1_C_CD_FAULT_LOWER_DELTA_I;			break;
		case C_CD_FAULT_UPPER_COMP_I1:
			rtn_code = P1_C_CD_FAULT_UPPER_COMP_I1;			break;
		case C_CD_FAULT_LOWER_COMP_I1:
			rtn_code = P1_C_CD_FAULT_LOWER_COMP_I1;			break;
		case C_CD_FAULT_UPPER_T:
			rtn_code = P1_C_CD_FAULT_UPPER_T;				break;
		case C_CD_FAULT_LOWER_T:
			rtn_code = P1_C_CD_FAULT_LOWER_T;				break;
		case C_CD_FAULT_UPPER_AMPARE_HOUR:
			rtn_code = P1_C_CD_FAULT_UPPER_AMPARE_HOUR;		break;
		case C_CD_FAULT_LOWER_AMPARE_HOUR:
			rtn_code = P1_C_CD_FAULT_LOWER_AMPARE_HOUR;		break;
		case C_CD_FAULT_UPPER_DELTA_C:
			rtn_code = P1_C_CD_FAULT_UPPER_DELTA_C;			break;
		case C_CD_FAULT_LOWER_DELTA_C:
			rtn_code = P1_C_CD_FAULT_LOWER_DELTA_C;			break;
		case C_CD_FAULT_UPPER_COMP_C:
			rtn_code = P1_C_CD_FAULT_UPPER_COMP_C;			break;
		case C_CD_FAULT_LOWER_COMP_C:
			rtn_code = P1_C_CD_FAULT_LOWER_COMP_C;			break;
		case C_CD_FAULT_UPPER_Z:
			rtn_code = P1_C_CD_FAULT_UPPER_Z;				break;
		case C_CD_FAULT_LOWER_Z:
			rtn_code = P1_C_CD_FAULT_LOWER_Z;				break;
		case C_CD_FAULT_UPPER_TEMP:
			rtn_code = P1_C_CD_FAULT_UPPER_TEMP;			break;
		case C_CD_FAULT_LOWER_TEMP:
			rtn_code = P1_C_CD_FAULT_LOWER_TEMP;			break;
		case C_CD_FAULT_DELTA_TEMP:
			rtn_code = P1_C_CD_FAULT_DELTA_TEMP;			break;
		case C_CD_FAULT_UPPER_COMP_I2:
			rtn_code = P1_C_CD_FAULT_UPPER_COMP_I2;			break;
		case C_CD_FAULT_LOWER_COMP_I2:
			rtn_code = P1_C_CD_FAULT_LOWER_COMP_I2;			break;
		case C_CD_FAULT_UPPER_COMP_I3:
			rtn_code = P1_C_CD_FAULT_UPPER_COMP_I3;			break;
		case C_CD_FAULT_LOWER_COMP_I3:
			rtn_code = P1_C_CD_FAULT_LOWER_COMP_I3;			break;

		case C_CD_FAULT_STOP_V:
			rtn_code = P1_C_CD_STOP_V;						break;
		case C_CD_FAULT_STOP_I:
			rtn_code = P1_C_CD_STOP_I;						break;
		case C_CD_FAULT_STOP_T:
			rtn_code = P1_C_CD_STOP_T;						break;
		case C_CD_FAULT_STOP_C:
			rtn_code = P1_C_CD_STOP_C;						break;
		case C_CD_FAULT_STOP_P:
			rtn_code = P1_C_CD_STOP_P;						break;
		case C_CD_FAULT_STOP_V_LIMITED:	//jhkw_131023
			rtn_code = P1_C_CD_FAULT_STOP_V_LIMITED;		break;
		case C_CD_FAULT_STOP_WATT_HOUR:
			rtn_code = P1_C_CD_STOP_WATT_HOUR;				break;
		case C_CD_FAULT_STOP_CMD:
			rtn_code = P1_C_CD_FAULT_STOP_CMD;				break;
		case C_CD_FAULT_STOP_MUX_SELECT_ERR:	//kjhw_151021
			rtn_code = P1_C_CD_FAULT_STOP_MUX_SELECT_ERR;	break;
		case C_CD_FAULT_PAUSE_CMD:
			rtn_code = P1_C_CD_FAULT_PAUSE_CMD;				break;
		case C_CD_FAULT_ACC_CYCLE_COUNT:
			rtn_code = P1_C_CD_FAULT_ACC_CYCLE_COUNT;		break;
		case C_CD_FAULT_UPPER_CAPACITANCE:
			rtn_code = P1_C_CD_FAULT_UPPER_CAPACITANCE;		break;
		case C_CD_FAULT_LOWER_CAPACITANCE:
			rtn_code = P1_C_CD_FAULT_LOWER_CAPACITANCE;		break;
		case C_CD_FAULT_UPPER_WATT_HOUR:
			rtn_code = P1_C_CD_FAULT_UPPER_WATT_HOUR;		break;
		case C_CD_FAULT_LOWER_WATT_HOUR:
			rtn_code = P1_C_CD_FAULT_LOWER_WATT_HOUR;		break;
		case C_CD_FAULT_OUT_PRECHARGING:
			rtn_code = P1_C_CD_FAULT_OUT_PRECHARGING;		break;
		case C_CD_FAULT_COMP_AUX_V:	//csk_190527
			rtn_code = P1_C_CD_FAULT_COMP_AUX_V;			break;
		case C_CD_FAULT_COMP_AUX_V_STEP:	//csk_190527
			rtn_code = P1_C_CD_FAULT_COMP_AUX_V_STEP;		break;
		case C_CD_FAULT_COMP_AUX_TEMP:	//csk_190808
			rtn_code = P1_C_CD_FAULT_COMP_AUX_TEMP;			break;
		case C_CD_FAULT_COMP_AUX_TH:	//csk_190808
			rtn_code = P1_C_CD_FAULT_COMP_AUX_TH;			break;
		case C_CD_FAULT_COMP_AUX_T:	//csk_190808
			rtn_code = P1_C_CD_FAULT_COMP_AUX_T;			break;
		case C_CD_FAULT_DELTA_AUX_V:	//csk_190808
			rtn_code = P1_C_CD_FAULT_DELTA_AUX_V;			break;
		case C_CD_FAULT_VENT_OPEN_TEST:	//csk_200122
			rtn_code = P1_C_CD_FAULT_VENT_OPEN_TEST;		break;
		case C_CD_FAULT_AUX_V_SVDF:	//ktg_210105
			rtn_code = P1_C_CD_FAULT_AUX_V_SVDF;			break;
		case C_CD_FAULT_CV_INTERLOCK_I:	//ktg_210315
			rtn_code = P1_C_CD_FAULT_CV_INTERLOCK_I;		break;
		case C_CD_AUX_FAULT_DROP_VOLTAGE:	//ktg_210408
			rtn_code = P1_C_CD_AUX_FAULT_DROP_VOLTAGE;		break;
		case C_CD_FAULT_PAUSE_CMD_ETC:	//ktg_210511
			rtn_code = P1_C_CD_FAULT_PAUSE_CMD_ETC;			break;
		case C_CD_CAN_FAULT_SVDF:	//ktg_210706
			rtn_code = P1_C_CD_CAN_FAULT_V_SVDF;			break;
		case C_CD_CAN_FAULT_DROP_VOLTAGE:	//ktg_210706
			rtn_code = P1_C_CD_CAN_FAULT_DROP_VOLTAGE;		break;
		case C_CD_CAN_FAULT_COMP_V:	//ktg_210706
			rtn_code = P1_C_CD_CAN_FAULT_COMP_V;			break;
		case C_CD_AUX_FAULT_SOFTVENTING_GAS:	//ktg_220331s
			rtn_code = P1_C_CD_AUX_FAULT_SOFTVENTING_GAS;	break;
		case C_CD_AUX_FAULT_HARDVENTING_GAS:
			rtn_code = P1_C_CD_AUX_FAULT_HARDVENTING_GAS;	break;
		case C_CD_CAN_FAULT_SOFTVENTING_GAS:
			rtn_code = P1_C_CD_CAN_FAULT_SOFTVENTING_GAS;	break;
		case C_CD_CAN_FAULT_HARDVENTING_GAS:
			rtn_code = P1_C_CD_CAN_FAULT_HARDVENTING_GAS;	break;	//ktg_220331e
		case C_CD_FAULT_PAUSE_CMD_DLL:
			rtn_code = P1_C_CD_FAULT_PAUSE_CMD_DLL;			break;
		case C_CD_FAULT_V_RISING:
			rtn_code = P1_C_CD_FAULT_V_RISING;				break;
		case C_CD_FAULT_CC_DELTA_V_ERROR:
			rtn_code = P1_C_CD_FAULT_CC_DELTA_V_ERROR;		break;
		case C_CD_FAULT_BREAKDOWN_VOLTAGE:
			rtn_code = P1_C_CD_FAULT_BREAKDOWN_VOLTAGE;		break;	//ktg_240930

		//aux code
		case C_CD_AUX_END_UPPER_TEMP:
			rtn_code = P1_C_CD_AUX_END_UPPER_TEMP;			break;
		case C_CD_AUX_END_LOWER_TEMP:
			rtn_code = P1_C_CD_AUX_END_LOWER_TEMP;			break;
		case C_CD_AUX_END_UPPER_V:
			rtn_code = P1_C_CD_AUX_END_UPPER_V;				break;
		case C_CD_AUX_END_LOWER_V:
			rtn_code = P1_C_CD_AUX_END_LOWER_V;				break;
		case C_CD_AUX_END_UPPER_HUMIDITY:	//csk_200211
			rtn_code = P1_C_CD_AUX_END_UPPER_HUMIDITY;		break;
		case C_CD_AUX_END_LOWER_HUMIDITY:	//csk_200211
			rtn_code = P1_C_CD_AUX_END_LOWER_HUMIDITY;		break;
		case C_CD_AUX_END_UPPER_GAS:		//sec_220926
			rtn_code = P1_C_CD_AUX_END_UPPER_GAS;			break;
		case C_CD_AUX_END_LOWER_GAS:		//sec_220926
			rtn_code = P1_C_CD_AUX_END_LOWER_GAS;			break;
		case C_CD_AUX_END_UPPER_V_BRANCH:
			rtn_code = P1_C_CD_AUX_END_V_UPPER_BRANCH;		break;
		case C_CD_AUX_END_LOWER_V_BRANCH:
			rtn_code = P1_C_CD_AUX_END_V_LOWER_BRANCH;		break;
		case C_CD_AUX_END_UPPER_TEMP_BRANCH:
			rtn_code = P1_C_CD_AUX_END_TEMP_UPPER_BRANCH;	break;
		case C_CD_AUX_END_LOWER_TEMP_BRANCH:
			rtn_code = P1_C_CD_AUX_END_TEMP_LOWER_BRANCH;	break;
		case C_CD_AUX_FAULT_UPPER_TEMP:
			rtn_code = P1_C_CD_AUX_FAULT_UPPER_TEMP;		break;
		case C_CD_AUX_FAULT_LOWER_TEMP:
			rtn_code = P1_C_CD_AUX_FAULT_LOWER_TEMP;		break;
		case C_CD_AUX_FAULT_UPPER_V:
			rtn_code = P1_C_CD_AUX_FAULT_UPPER_V;			break;
		case C_CD_AUX_FAULT_LOWER_V:
			rtn_code = P1_C_CD_AUX_FAULT_LOWER_V;			break;
		case C_CD_AUX_FAULT_UPPER_HUMIDITY:	//csk_200211
			rtn_code = P1_C_CD_AUX_FAULT_UPPER_HUMIDITY;	break;
		case C_CD_AUX_FAULT_LOWER_HUMIDITY:	//csk_200211
			rtn_code = P1_C_CD_AUX_FAULT_LOWER_HUMIDITY;	break;
		case C_CD_AUX_FAULT_UPPER_GAS:		//sec_220926
			rtn_code = P1_C_CD_AUX_FAULT_UPPER_GAS;			break;
		case C_CD_AUX_FAULT_LOWER_GAS:	//sec_220926
			rtn_code = P1_C_CD_AUX_FAULT_LOWER_GAS;			break;
		case C_CD_AUX_END_FUNC_DIV_1:
		case C_CD_AUX_END_FUNC_DIV_2:
		case C_CD_AUX_END_FUNC_DIV_3:
		case C_CD_AUX_END_FUNC_DIV_4:
		case C_CD_AUX_END_FUNC_DIV_5:
		case C_CD_AUX_END_FUNC_DIV_6:
		case C_CD_AUX_END_FUNC_DIV_7:
		case C_CD_AUX_END_FUNC_DIV_8:
		case C_CD_AUX_END_FUNC_DIV_9:
		case C_CD_AUX_END_FUNC_DIV_10:
			rtn_code = P1_C_CD_END_AUX_FUNC_DIV;			break;
		case C_CD_AUX_FAULT_FUNC_DIV_1:
		case C_CD_AUX_FAULT_FUNC_DIV_2:
		case C_CD_AUX_FAULT_FUNC_DIV_3:
		case C_CD_AUX_FAULT_FUNC_DIV_4:
		case C_CD_AUX_FAULT_FUNC_DIV_5:
		case C_CD_AUX_FAULT_FUNC_DIV_6:
		case C_CD_AUX_FAULT_FUNC_DIV_7:
		case C_CD_AUX_FAULT_FUNC_DIV_8:
		case C_CD_AUX_FAULT_FUNC_DIV_9:
		case C_CD_AUX_FAULT_FUNC_DIV_10:
			rtn_code = P1_C_CD_FAULT_AUX_FUNC_DIV;			break;
		case C_CD_AUX_FAULT_VENT_UPPER:	//csk_200113
			rtn_code = P1_C_CD_AUX_FAULT_VENT_UPPER;		break;
		case C_CD_AUX_FAULT_VENT_LOWER:	//csk_200113
			rtn_code = P1_C_CD_AUX_FAULT_VENT_LOWER;		break;
		case C_CD_FAULT_ISOLATION_STATE:
			rtn_code = P1_C_CD_FAULT_ISOLATION_STATE;		break;	//ktg_210908	//ktg_220701

		//can code
		case C_CD_CAN_END_UPPER:
			rtn_code = P1_C_CD_CAN_END_UPPER;				break;
		case C_CD_CAN_END_LOWER:
			rtn_code = P1_C_CD_CAN_END_LOWER;				break;
		case C_CD_CAN_END_UPPER_V_BRANCH:
			rtn_code = P1_C_CD_CAN_END_V_UPPER_BRANCH;		break;
		case C_CD_CAN_END_LOWER_V_BRANCH:
			rtn_code = P1_C_CD_CAN_END_V_LOWER_BRANCH;		break;
		case C_CD_CAN_END_UPPER_TEMP_BRANCH:
			rtn_code = P1_C_CD_CAN_END_TEMP_UPPER_BRANCH;	break;
		case C_CD_CAN_END_LOWER_TEMP_BRANCH:
			rtn_code = P1_C_CD_CAN_END_TEMP_LOWER_BRANCH;	break;
		case C_CD_CAN_END_FAULT_BRANCH:
			rtn_code = P1_C_CD_CAN_END_FAULT_BRANCH;		break;
		case C_CD_CAN_FAULT_UPPER:
			rtn_code = P1_C_CD_CAN_FAULT_UPPER;				break;
		case C_CD_CAN_FAULT_LOWER:
			rtn_code = P1_C_CD_CAN_FAULT_LOWER;				break;
		case C_CD_CAN_END_FUNC_DIV_1:
		case C_CD_CAN_END_FUNC_DIV_2:
		case C_CD_CAN_END_FUNC_DIV_3:
		case C_CD_CAN_END_FUNC_DIV_4:
		case C_CD_CAN_END_FUNC_DIV_5:
		case C_CD_CAN_END_FUNC_DIV_6:
		case C_CD_CAN_END_FUNC_DIV_7:
		case C_CD_CAN_END_FUNC_DIV_8:
		case C_CD_CAN_END_FUNC_DIV_9:
		case C_CD_CAN_END_FUNC_DIV_10:
			rtn_code = P1_C_CD_END_CAN_FUNC_DIV;			break;
		case C_CD_CAN_END_BMS_ERROR:
			rtn_code = P1_C_CD_END_CAN_BMS_ERROR;			break;
		case C_CD_CAN_FAULT_FUNC_DIV_1:
		case C_CD_CAN_FAULT_FUNC_DIV_2:
		case C_CD_CAN_FAULT_FUNC_DIV_3:
		case C_CD_CAN_FAULT_FUNC_DIV_4:
		case C_CD_CAN_FAULT_FUNC_DIV_5:
		case C_CD_CAN_FAULT_FUNC_DIV_6:
		case C_CD_CAN_FAULT_FUNC_DIV_7:
		case C_CD_CAN_FAULT_FUNC_DIV_8:
		case C_CD_CAN_FAULT_FUNC_DIV_9:
		case C_CD_CAN_FAULT_FUNC_DIV_10:
			rtn_code = P1_C_CD_FAULT_CAN_FUNC_DIV;			break;
		case C_CD_CAN_FAULT_BMS_PAUSE:
			rtn_code = P1_C_CD_FAULT_CAN_BMS_PAUSE;			break;
		case C_CD_CAN_CABLE_FAULT:	//kjhw_131129
			rtn_code = P1_C_CD_FAULT_CAN_CABLE_FAULT;		break;
		case C_CD_CAN_FAULT_EXT_DEVICE:	//kjg_140916
			rtn_code = P1_C_CD_FAULT_CAN_EXT_DEVICE;		break;
		case C_CD_CAN_FAULT_EXT_CMD_RANGE:	//kjg_140916
			rtn_code = P1_C_CD_FAULT_CAN_EXT_CMD_RANGE;		break;
		case C_CD_CAN_FAULT_EXT_CONTROLLER_STATE:	//kjg_140916
			rtn_code = P1_C_CD_FAULT_CAN_EXT_CONTROLLER_STATE;	break;
		case C_CD_CAN_FAULT_EXT_NM_PERIOD:	//kjg_140916
			rtn_code = P1_C_CD_FAULT_CAN_EXT_NM_PERIOD;		break;
		case C_CD_CAN_FAULT_CAN_FREEZE:	//ktg_230728
			rtn_code = P1_C_CD_CAN_FAULT_CAN_FREEZE;		break;
		default:
			rtn_code = P1_C_CD_FAULT_WORK_ERROR;			break;
	}
	return rtn_code;
}

long convert_ch_code_org_to_p2(long code)
{
	long rtn_code=0;
#if defined __COB__
	switch(code) {
		case C_CD_NONE:
			rtn_code = P2_C_CD_NONE;						break;

		//common code(hard)
		case C_CD_COM_FAULT_HW_UPPER_V:
			rtn_code = P2_C_CD_FAULT_CH_V_FAIL;				break;
		case C_CD_COM_FAULT_HW_LOWER_V:
			rtn_code = P2_C_CD_FAULT_CH_V_FAIL;				break;
		case C_CD_COM_FAULT_HW_UPPER_I:
			rtn_code = P2_C_CD_FAULT_CH_I_FAIL;				break;
		case C_CD_COM_FAULT_HW_LOWER_I:
			rtn_code = P2_C_CD_FAULT_CH_I_FAIL;				break;
		case C_CD_COM_FAULT_HW_UPPER_TEMP:
			rtn_code = P2_C_CD_FAULT_OT;					break;
		case C_CD_COM_FAULT_HW_LOWER_TEMP:
			rtn_code = P2_C_CD_FAULT_OT;					break;

		//common code(soft)
		case C_CD_COM_FAULT_UPPER_V:
			rtn_code = P2_C_CD_FAULT_UPPER_V;				break;
		case C_CD_COM_FAULT_LOWER_V:
			rtn_code = P2_C_CD_FAULT_LOWER_V;				break;
		case C_CD_COM_FAULT_UPPER_I:
			rtn_code = P2_C_CD_FAULT_UPPER_I;				break;
		case C_CD_COM_FAULT_LOWER_I:
			rtn_code = P2_C_CD_FAULT_LOWER_I;				break;
		case C_CD_COM_FAULT_UPPER_AMPARE_HOUR:
			rtn_code = P2_C_CD_FAULT_UPPER_C;				break;
		case C_CD_COM_FAULT_LOWER_AMPARE_HOUR:
			rtn_code = P2_C_CD_FAULT_LOWER_C;				break;
		case C_CD_COM_FAULT_UPPER_WATT_HOUR:
			rtn_code = P2_C_CD_FAULT_UPPER_WATT_HOUR;		break;
		case C_CD_COM_FAULT_LOWER_WATT_HOUR:
			rtn_code = P2_C_CD_FAULT_LOWER_WATT_HOUR;		break;
		case C_CD_COM_FAULT_UPPER_P:
			rtn_code = P2_C_CD_FAULT_UPPER_P;				break;
		case C_CD_COM_FAULT_LOWER_P:
			rtn_code = P2_C_CD_FAULT_LOWER_P;				break;
		case C_CD_COM_FAULT_UPPER_TEMP:
			rtn_code = P2_C_CD_FAULT_UPPER_TEMP;			break;
		case C_CD_COM_FAULT_LOWER_TEMP:
			rtn_code = P2_C_CD_FAULT_LOWER_TEMP;			break;
		case C_CD_COM_FAULT_UPPER_CAPACITANCE:
			rtn_code = P2_C_CD_FAULT_UPPER_C;				break;
		case C_CD_COM_FAULT_LOWER_CAPACITANCE:
			rtn_code = P2_C_CD_FAULT_LOWER_C;				break;

		//end code
		case C_CD_END_TIME:
		case C_CD_END_CV_TIME:
			rtn_code = P2_C_CD_END_T;						break;
		case C_CD_END_V_UPPER:
		case C_CD_END_V_LOWER:
			rtn_code = P2_C_CD_END_V;						break;
		case C_CD_END_I:
			rtn_code = P2_C_CD_END_I;						break;
		case C_CD_END_AMPARE_HOUR:
			rtn_code = P2_C_CD_END_C;						break;
		case C_CD_END_OCV:
			rtn_code = P2_C_CD_END_OCV;						break;
		case C_CD_END_STEP:
			rtn_code = P2_C_CD_END_STEP;					break;
		case C_CD_END_CHECK:
			rtn_code = P2_C_CD_END_CHECK;					break;
		case C_CD_END_NEXT_STEP_CMD:
			rtn_code = P2_C_CD_END_NEXTSTEP_CMD;			break;
		case C_CD_END_CYCLE:
			rtn_code = P2_C_CD_END_CYCLE_STEP;				break;
		case C_CD_END_LOOP:
			rtn_code = P2_C_CD_END_LOOP_STEP;				break;
		case C_CD_END_Z:
			rtn_code = P2_C_CD_END_Z;						break;
		case C_CD_END_DELTA_V:
			rtn_code = P2_C_CD_END_DELTA_V;					break;
		case C_CD_END_DELTA_I:
			rtn_code = P2_C_CD_END_DELTA_I;					break;
		case C_CD_END_VALUE_RATE_AMPARE_HOUR:
			rtn_code = P2_C_CD_END_SOC;						break;
		case C_CD_END_P:
			rtn_code = P2_C_CD_END_P;						break;
		case C_CD_END_WATT_HOUR:
			rtn_code = P2_C_CD_END_WATT_HOUR;				break;
		case C_CD_END_GOTO_STEP_CMD:
			rtn_code = P2_C_CD_END_GOTO_STEP_CMD;			break;
		case C_CD_END_STOP_CMD:
			rtn_code = P2_C_CD_END_STOP_CMD;				break;
		case C_CD_END_VALUE_RATE_WATT_HOUR:
			rtn_code = P2_C_CD_END_VALUE_RATE_WATT_HOUR;	break;
		case C_CD_END_SUM_AMPARE_HOUR:
			rtn_code = P2_C_CD_END_SUM_AMPARE_HOUR;			break;
		case C_CD_END_SUM_WATT_HOUR:
			rtn_code = P2_C_CD_END_SUM_WATT_HOUR;			break;
		case C_CD_END_SUM_TIME:
			rtn_code = P2_C_CD_END_SUM_TIME;				break;

		//check fault code
		case C_CD_FAULT_CHECK_NG:
			rtn_code = P2_C_CD_FAULT_CHECK_CONTACT_BAD;		break;
		case C_CD_FAULT_CHECK_UPPER_OCV:
			rtn_code = P2_C_CD_FAULT_CHECK_UPPER_OCV;		break;
		case C_CD_FAULT_CHECK_LOWER_OCV:
			rtn_code = P2_C_CD_FAULT_CHECK_LOWER_OCV;		break;
		case C_CD_FAULT_CHECK_UPPER_V:
			rtn_code = P2_C_CD_FAULT_CHECK_UPPER_V;			break;
		case C_CD_FAULT_CHECK_LOWER_V:
			rtn_code = P2_C_CD_FAULT_CHECK_LOWER_V;			break;
		case C_CD_FAULT_CHECK_UPPER_I:
			rtn_code = P2_C_CD_FAULT_CHECK_UPPER_I;			break;
		case C_CD_FAULT_CHECK_LOWER_I:
			rtn_code = P2_C_CD_FAULT_CHECK_LOWER_I;			break;
		case C_CD_FAULT_CHECK_ERROR_NO:
			rtn_code = P2_C_CD_FAULT_CHECK_ERROR_NO;		break;
		case C_CD_FAULT_CHECK_ERROR_YES:
			rtn_code = P2_C_CD_FAULT_CHECK_ERROR_YES;		break;
		case C_CD_FAULT_CHECK_DETECT_V:
			rtn_code = P2_C_CD_FAULT_CHECK_DETECT_V;		break;
		case C_CD_FAULT_CHECK_REVERSE_V:
			rtn_code = P2_C_CD_FAULT_CHECK_REVERSE_V;		break;
		case C_CD_FAULT_CHECK_I_JUDGE_RATIO:
			rtn_code = P2_C_CD_FAULT_CHECK_I_JUDGE_RATIO;	break;
		case C_CD_FAULT_CHECK_UPPER_DELTA_V:
			rtn_code = P2_C_CD_FAULT_CHECK_UPPER_DELTA_V;	break;
		case C_CD_FAULT_CHECK_LOWER_DELTA_V:
			rtn_code = P2_C_CD_FAULT_CHECK_LOWER_DELTA_V;	break;
		case C_CD_FAULT_CHECK_CONTACT_BAD1:
		case C_CD_FAULT_CHECK_CONTACT_BAD2:
		case C_CD_FAULT_CHECK_CONTACT_BAD3:
		case C_CD_FAULT_CHECK_CONTACT_BAD4:
		case C_CD_FAULT_CHECK_CONTACT_BAD5:
		case C_CD_FAULT_CHECK_CONTACT_BAD6:
		case C_CD_FAULT_CHECK_CONTACT_BAD7:
		case C_CD_FAULT_CHECK_CONTACT_BAD8:
		case C_CD_FAULT_CHECK_CONTACT_BAD9:
			rtn_code = P2_C_CD_FAULT_CHECK_CONTACT_BAD2;	break;

		//soft fault code
		case C_CD_FAULT_UPPER_V:
			rtn_code = P2_C_CD_FAULT_UPPER_V;				break;
		case C_CD_FAULT_LOWER_V:
			rtn_code = P2_C_CD_FAULT_LOWER_V;				break;
		case C_CD_FAULT_UPPER_DELTA_V:
			rtn_code = P2_C_CD_FAULT_UPPER_DELTA_V;			break;
		case C_CD_FAULT_LOWER_DELTA_V:
			rtn_code = P2_C_CD_FAULT_LOWER_DELTA_V;			break;
		case C_CD_FAULT_UPPER_COMP_V1:
			rtn_code = P2_C_CD_FAULT_UPPER_COMP_V;			break;
		case C_CD_FAULT_LOWER_COMP_V1:
			rtn_code = P2_C_CD_FAULT_LOWER_COMP_V;			break;
		case C_CD_FAULT_UPPER_COMP_V2:
			rtn_code = P2_C_CD_FAULT_UPPER_COMP_V2;			break;
		case C_CD_FAULT_LOWER_COMP_V2:
			rtn_code = P2_C_CD_FAULT_LOWER_COMP_V2;			break;
		case C_CD_FAULT_UPPER_COMP_V3:
			rtn_code = P2_C_CD_FAULT_UPPER_COMP_V3;			break;
		case C_CD_FAULT_LOWER_COMP_V3:
			rtn_code = P2_C_CD_FAULT_LOWER_COMP_V3;			break;
		case C_CD_FAULT_UPPER_OCV:
			rtn_code = P2_C_CD_FAULT_UPPER_OCV;				break;
		case C_CD_FAULT_LOWER_OCV:
			rtn_code = P2_C_CD_FAULT_LOWER_OCV;				break;
		case C_CD_FAULT_UPPER_I:
			rtn_code = P2_C_CD_FAULT_UPPER_I;				break;
		case C_CD_FAULT_LOWER_I:
			rtn_code = P2_C_CD_FAULT_LOWER_I;				break;
		case C_CD_FAULT_UPPER_DELTA_I:
			rtn_code = P2_C_CD_FAULT_UPPER_DELTA_I;			break;
		case C_CD_FAULT_LOWER_DELTA_I:
			rtn_code = P2_C_CD_FAULT_LOWER_DELTA_I;			break;
		case C_CD_FAULT_UPPER_COMP_I1:
			rtn_code = P2_C_CD_FAULT_UPPER_COMP_I;			break;
		case C_CD_FAULT_LOWER_COMP_I1:
			rtn_code = P2_C_CD_FAULT_LOWER_COMP_I;			break;
		case C_CD_FAULT_UPPER_T:
			rtn_code = P2_C_CD_FAULT_UPPER_T;				break;
		case C_CD_FAULT_LOWER_T:
			rtn_code = P2_C_CD_FAULT_LOWER_T;				break;
		case C_CD_FAULT_UPPER_AMPARE_HOUR:
			rtn_code = P2_C_CD_FAULT_UPPER_C;				break;
		case C_CD_FAULT_LOWER_AMPARE_HOUR:
			rtn_code = P2_C_CD_FAULT_LOWER_C;				break;
		case C_CD_FAULT_UPPER_DELTA_C:
			rtn_code = P2_C_CD_FAULT_UPPER_DELTA_C;			break;
		case C_CD_FAULT_LOWER_DELTA_C:
			rtn_code = P2_C_CD_FAULT_LOWER_DELTA_C;			break;
		case C_CD_FAULT_UPPER_COMP_C:
			rtn_code = P2_C_CD_FAULT_UPPER_COMP_C;			break;
		case C_CD_FAULT_LOWER_COMP_C:
			rtn_code = P2_C_CD_FAULT_LOWER_COMP_C;			break;
		case C_CD_FAULT_UPPER_Z:
			rtn_code = P2_C_CD_FAULT_UPPER_Z;				break;
		case C_CD_FAULT_LOWER_Z:
			rtn_code = P2_C_CD_FAULT_LOWER_Z;				break;
		case C_CD_FAULT_UPPER_TEMP:
		case C_CD_FAULT_DELTA_TEMP:
			rtn_code = P2_C_CD_FAULT_UPPER_TEMP;			break;
		case C_CD_FAULT_LOWER_TEMP:
			rtn_code = P2_C_CD_FAULT_LOWER_TEMP;			break;
		case C_CD_FAULT_UPPER_COMP_I2:
			rtn_code = P2_C_CD_FAULT_UPPER_COMP_I2;			break;
		case C_CD_FAULT_LOWER_COMP_I2:
			rtn_code = P2_C_CD_FAULT_LOWER_COMP_I2;			break;
		case C_CD_FAULT_UPPER_COMP_I3:
			rtn_code = P2_C_CD_FAULT_UPPER_COMP_I3;			break;
		case C_CD_FAULT_LOWER_COMP_I3:
			rtn_code = P2_C_CD_FAULT_LOWER_COMP_I3;			break;

		case C_CD_FAULT_STOP_V:
			rtn_code = P2_C_CD_STOP_V;						break;
		case C_CD_FAULT_STOP_I:
			rtn_code = P2_C_CD_STOP_I;						break;
		case C_CD_FAULT_STOP_T:
			rtn_code = P2_C_CD_STOP_T;						break;
		case C_CD_FAULT_STOP_C:
			rtn_code = P2_C_CD_STOP_C;						break;
		case C_CD_FAULT_STOP_P:
			rtn_code = P2_C_CD_STOP_POWER;					break;
		case C_CD_FAULT_STOP_WATT_HOUR:
			rtn_code = P2_C_CD_STOP_WATTHOUR;				break;
		case C_CD_FAULT_STOP_CMD:
			rtn_code = P2_C_CD_FAULT_STOP_CMD;				break;
		case C_CD_FAULT_PAUSE_CMD:
			rtn_code = P2_C_CD_FAULT_PAUSE_CMD;				break;
		case C_CD_FAULT_ACC_CYCLE_COUNT:
			rtn_code = P2_C_CD_FAULT_ACC_CYCLE_COUNT;		break;
		case C_CD_FAULT_UPPER_CAPACITANCE:
			rtn_code = P2_C_CD_FAULT_UPPER_CAPACITANCE;		break;
		case C_CD_FAULT_LOWER_CAPACITANCE:
			rtn_code = P2_C_CD_FAULT_LOWER_CAPACITANCE;		break;
		case C_CD_FAULT_UPPER_WATT_HOUR:
			rtn_code = P2_C_CD_FAULT_UPPER_WATT_HOUR;		break;
		case C_CD_FAULT_LOWER_WATT_HOUR:
			rtn_code = P2_C_CD_FAULT_LOWER_WATT_HOUR;		break;
		case C_CD_FAULT_MULTI_CYCLE_COUNT:
			rtn_code = P2_C_CD_FAULT_MULTI_CYCLE_COUNT;		break;
		case C_CD_FAULT_RUN_HOLD:
			rtn_code = P2_C_CD_FAULT_RUN_HOLD;				break;
		case C_CD_FAULT_BAD_CELL:
			rtn_code = P2_C_CD_FAULT_BAD_CELL;				break;

		//hard fault code
		case C_CD_FAULT_HW_UPPER_V:
		case C_CD_FAULT_HW_LOWER_V:
		case C_CD_FAULT_PS_OV:
		case C_CD_FAULT_DELTA_V_FAIL:
			rtn_code = P2_C_CD_FAULT_CH_V_FAIL;				break;
		case C_CD_FAULT_HW_UPPER_I:
		case C_CD_FAULT_HW_LOWER_I:
		case C_CD_FAULT_PS_OC:
			rtn_code = P2_C_CD_FAULT_CH_I_FAIL;				break;
		case C_CD_FAULT_HW_OT:
			rtn_code = P2_C_CD_FAULT_OT;					break;
		case C_CD_FAULT_CHANNEL_PS:
		case C_CD_FAULT_PS_DCOV:
		case C_CD_FAULT_IN_FUSE:
		case C_CD_FAULT_OUT_FUSE:
			rtn_code = P2_C_CD_FAULT_SMPS;					break;
		case C_CD_FAULT_M_CPU_WATCHDOG:
			rtn_code = P2_C_CD_FAULT_WATCHDOG;				break;
		case C_CD_FAULT_M_AC_POWER:
			rtn_code = P2_C_CD_FAULT_AC_POWER;				break;
		case C_CD_FAULT_M_UPS_BATTERY:
			rtn_code = P2_C_CD_FAULT_UPS_BATTERY;			break;
		case C_CD_FAULT_M_EMG_SWITCH:
			rtn_code = P2_C_CD_FAULT_MAIN_EMG;				break;
		case C_CD_FAULT_M_FORCE_POWER_SWITCH:
		case C_CD_FAULT_M_FORCE_TERMINAL_HALT:
		case C_CD_FAULT_M_FORCE_TERMINAL_QUIT:
			rtn_code = P2_C_CD_FAULT_FORCE_POWER;			break;
		case C_CD_FAULT_G_NETWORK_COMM:
			rtn_code = P2_C_CD_FAULT_NETWORK_COMM;			break;
		case C_CD_FAULT_B_UPPER_TEMP:
			rtn_code = P2_C_CD_FAULT_OT;					break;
		case C_CD_FAULT_B_ADC:
			rtn_code = P2_C_CD_FAULT_ADC;					break;
		case C_CD_FAULT_CONTROL_PS:
		case C_CD_FAULT_MAIN_PS:
			rtn_code = P2_C_CD_FAULT_SMPS;					break;
		case C_CD_FAULT_READ_PATTERN_FILE:
			rtn_code = P2_C_CD_FAULT_FILE_ERROR;			break;

		//for sbl fault code
		case C_CD_COM_FAULT_CHARGE_UPPER_V:
			rtn_code = P2_C_CD_FAULT_CHARGE_UPPER_V;		break;
		case C_CD_COM_FAULT_CHARGE_UPPER_AMPARE_HOUR:
			rtn_code = P2_C_CD_FAULT_CHARGE_UPPER_AMPARE_HOUR;	break;
		case C_CD_COM_FAULT_CHARGE_END_UPPER_V:
			rtn_code = P2_C_CD_FAULT_CHARGE_END_UPPER_V;	break;
		case C_CD_COM_FAULT_CHARGE_LOWER_I:
			rtn_code = P2_C_CD_FAULT_CHARGE_LOWER_I;		break;
		case C_CD_COM_FAULT_DISCHARGE_LOWER_V:
			rtn_code = P2_C_CD_FAULT_DISCHARGE_LOWER_V;		break;
		case C_CD_COM_FAULT_DISCHARGE_RUN_TIME:
			rtn_code = P2_C_CD_FAULT_DISCHARGE_RUN_TIME;	break;
		case C_CD_COM_FAULT_DISCHARGE_LOWER_AMPARE_HOUR:
			rtn_code = P2_C_CD_FAULT_DISCHARGE_LOWER_AMPARE_HOUR;	break;
		case C_CD_COM_FAULT_OVER_CURRENT_LIMIT:
			rtn_code = P2_C_CD_FAULT_OVER_CURRENT_LIMIT;	break;
		case C_CD_COM_FAULT_COMP_UPPER_V1:
		case C_CD_COM_FAULT_COMP_LOWER_V1:
			rtn_code = P2_C_CD_FAULT_COMP_CHECK_V;			break;
		case C_CD_COM_FAULT_COMP_UPPER_V2:
		case C_CD_COM_FAULT_COMP_LOWER_V2:
			rtn_code = P2_C_CD_FAULT_COMP_V1;				break;
		case C_CD_COM_FAULT_COMP_UPPER_V3:
		case C_CD_COM_FAULT_COMP_LOWER_V3:
			rtn_code = P2_C_CD_FAULT_COMP_V2;				break;
		case C_CD_COM_FAULT_COMP_UPPER_I1:
		case C_CD_COM_FAULT_COMP_LOWER_I1:
		case C_CD_COM_FAULT_COMP_UPPER_I2:
		case C_CD_COM_FAULT_COMP_LOWER_I2:
		case C_CD_COM_FAULT_COMP_UPPER_I3:
		case C_CD_COM_FAULT_COMP_LOWER_I3:
			rtn_code = P2_C_CD_FAULT_COMP_I1;				break;
		case C_CD_COM_FAULT_COMP_LOWER_CV_TIME:
			rtn_code = P2_C_CD_FAULT_COMP_LOWER_CV_TIME;	break;
		case C_CD_COM_FAULT_COMP_UPPER_CV_TIME:
			rtn_code = P2_C_CD_FAULT_COMP_UPPER_CV_TIME;		break;

		//for sbl ng cond
		case C_CD_FAULT_VOLTAGE_DATA_MIN:
			rtn_code = P2_C_CD_FAULT_VOLTAGE_DATA_MIN;		break;
		case C_CD_FAULT_VOLTAGE_DATA_MAX:
			rtn_code = P2_C_CD_FAULT_VOLTAGE_DATA_MAX;		break;
		case C_CD_FAULT_VOLTAGE_AVG_MIN:
			rtn_code = P2_C_CD_FAULT_VOLTAGE_AVG_MIN;		break;
		case C_CD_FAULT_VOLTAGE_AVG_MAX:
			rtn_code = P2_C_CD_FAULT_VOLTAGE_AVG_MAX;		break;
		case C_CD_FAULT_AMPARE_HOUR_DATA_MIN:
			rtn_code = P2_C_CD_FAULT_AMPARE_HOUR_DATA_MIN;	break;
		case C_CD_FAULT_AMPARE_HOUR_DATA_MAX:
			rtn_code = P2_C_CD_FAULT_AMPARE_HOUR_DATA_MAX;	break;
		case C_CD_FAULT_AMPARE_HOUR_AVG_MIN:
			rtn_code = P2_C_CD_FAULT_AMPARE_HOUR_AVG_MIN;	break;
		case C_CD_FAULT_AMPARE_HOUR_AVG_MAX:
			rtn_code = P2_C_CD_FAULT_AMPARE_HOUR_AVG_MAX;	break;
		case C_CD_FAULT_POWER_DATA_MIN:
			rtn_code = P2_C_CD_FAULT_POWER_DATA_MIN;		break;
		case C_CD_FAULT_POWER_DATA_MAX:
			rtn_code = P2_C_CD_FAULT_POWER_DATA_MAX;		break;
		case C_CD_FAULT_POWER_AVG_MIN:
			rtn_code = P2_C_CD_FAULT_POWER_AVG_MIN;			break;
		case C_CD_FAULT_POWER_AVG_MAX:
			rtn_code = P2_C_CD_FAULT_POWER_AVG_MAX;			break;

		default:	rtn_code = P2_C_CD_FAULT_UNKNOWN; 		break;
	}
#endif
	return rtn_code;
}

long convert_ch_code_org_to_p3(long code)
{
	long rtn_code=0;
#if defined __COC__
	switch(code) {
		case C_CD_NONE:
			rtn_code = P3_C_CD_NONE;						break;

		//common code(hard)
		case C_CD_COM_FAULT_HW_UPPER_V:
			rtn_code = P3_C_CD_FAULT_CH_V_FAIL;				break;
		case C_CD_COM_FAULT_HW_LOWER_V:
			rtn_code = P3_C_CD_FAULT_CH_V_FAIL;				break;
		case C_CD_COM_FAULT_HW_UPPER_I:
			rtn_code = P3_C_CD_FAULT_CH_I_FAIL;				break;
		case C_CD_COM_FAULT_HW_LOWER_I:
			rtn_code = P3_C_CD_FAULT_CH_I_FAIL;				break;
		case C_CD_COM_FAULT_HW_UPPER_TEMP:
			rtn_code = P3_C_CD_FAULT_OT;					break;
		case C_CD_COM_FAULT_HW_LOWER_TEMP:
			rtn_code = P3_C_CD_FAULT_OT;					break;

		//common code
		case C_CD_COM_FAULT_UPPER_V:
			rtn_code = P3_C_CD_FAULT_UPPER_V;				break;
		case C_CD_COM_FAULT_LOWER_V:
			rtn_code = P3_C_CD_FAULT_LOWER_V;				break;
		case C_CD_COM_FAULT_UPPER_I:
			rtn_code = P3_C_CD_FAULT_UPPER_I;				break;
		case C_CD_COM_FAULT_LOWER_I:
			rtn_code = P3_C_CD_FAULT_LOWER_I;				break;
		case C_CD_COM_FAULT_UPPER_AMPARE_HOUR:
			rtn_code = P3_C_CD_FAULT_UPPER_AMPARE_HOUR;		break;
		case C_CD_COM_FAULT_LOWER_AMPARE_HOUR:
			rtn_code = P3_C_CD_FAULT_LOWER_AMPARE_HOUR;		break;
		case C_CD_COM_FAULT_UPPER_WATT_HOUR:
			rtn_code = P3_C_CD_FAULT_UPPER_WATT_HOUR;		break;
		case C_CD_COM_FAULT_LOWER_WATT_HOUR:
			rtn_code = P3_C_CD_FAULT_LOWER_WATT_HOUR;		break;
		case C_CD_COM_FAULT_UPPER_P:
			rtn_code = P3_C_CD_FAULT_UPPER_P;				break;
		case C_CD_COM_FAULT_LOWER_P:
			rtn_code = P3_C_CD_FAULT_LOWER_P;				break;
		case C_CD_COM_FAULT_UPPER_TEMP:
			rtn_code = P3_C_CD_FAULT_UPPER_TEMP;			break;
		case C_CD_COM_FAULT_LOWER_TEMP:
			rtn_code = P3_C_CD_FAULT_LOWER_TEMP;			break;
		case C_CD_COM_FAULT_UPPER_CAPACITANCE:
			rtn_code = P3_C_CD_FAULT_UPPER_CAPACITANCE;		break;
		case C_CD_COM_FAULT_LOWER_CAPACITANCE:
			rtn_code = P3_C_CD_FAULT_LOWER_CAPACITANCE;		break;

		//end code
		case C_CD_END_TIME:
			rtn_code = P3_C_CD_END_TIME;					break;
		case C_CD_END_V_UPPER:
			rtn_code = P3_C_CD_END_V_UPPER;					break;
		case C_CD_END_I:
			rtn_code = P3_C_CD_END_I;						break;
		case C_CD_END_AMPARE_HOUR:
			rtn_code = P3_C_CD_END_AMPARE_HOUR;				break;
		case C_CD_END_OCV:
			rtn_code = P3_C_CD_END_OCV;						break;
		case C_CD_END_STEP:
			rtn_code = P3_C_CD_END_STEP;					break;
		case C_CD_END_CHECK:
			rtn_code = P3_C_CD_END_CHECK;					break;
		case C_CD_END_NEXT_STEP_CMD:
			rtn_code = P3_C_CD_END_NEXT_STEP_CMD;			break;
		case C_CD_END_CYCLE:
			rtn_code = P3_C_CD_END_CYCLE;					break;
		case C_CD_END_LOOP:
			rtn_code = P3_C_CD_END_LOOP;					break;
		case C_CD_END_Z:
			rtn_code = P3_C_CD_END_Z;						break;
		case C_CD_END_DELTA_V:
			rtn_code = P3_C_CD_END_DELTA_V;					break;
		case C_CD_END_DELTA_I:
			rtn_code = P3_C_CD_END_DELTA_I;					break;
		case C_CD_END_VALUE_RATE_AMPARE_HOUR:
			rtn_code = P3_C_CD_END_VALUE_RATE_AMPARE_HOUR;	break;
		case C_CD_END_P:
			rtn_code = P3_C_CD_END_P;						break;
		case C_CD_END_WATT_HOUR:
			rtn_code = P3_C_CD_END_WATT_HOUR;				break;
		case C_CD_END_V_LOWER:
			rtn_code = P3_C_CD_END_V_LOWER;					break;
		case C_CD_END_CV_TIME:
			rtn_code = P3_C_CD_END_CV_TIME;					break;
		case C_CD_END_GOTO_STEP_CMD:
			rtn_code = P3_C_CD_END_GOTO_STEP_CMD;			break;
		case C_CD_END_STOP_CMD:
			rtn_code = P3_C_CD_END_STOP_CMD;				break;
		case C_CD_END_VALUE_RATE_WATT_HOUR:
			rtn_code = P3_C_CD_END_VALUE_RATE_WATT_HOUR;	break;
		case C_CD_END_SUM_AMPARE_HOUR:
			rtn_code = P3_C_CD_END_SUM_AMPARE_HOUR;			break;
		case C_CD_END_SUM_WATT_HOUR:
			rtn_code = P3_C_CD_END_SUM_WATT_HOUR;			break;
		case C_CD_END_SUM_TIME:
			rtn_code = P3_C_CD_END_SUM_TIME;				break;
		case C_CD_END_TEMP_UPPER:
		case C_CD_END_TEMP_LOWER:
			rtn_code = P3_C_CD_END_TEMP;					break;
		case C_CD_CHAMBER_CONTROL_WAIT:
			rtn_code = P3_C_CD_CHAMBER_CONTROL_WAIT;		break;
		case C_CD_CYCLE_CONTROL_WAIT:
			rtn_code = P3_C_CD_CYCLE_CONTROL_WAIT;			break;

		//save code //kjg_w

		//hard fault code
		case C_CD_FAULT_HW_UPPER_V:
		case C_CD_FAULT_HW_LOWER_V:
			rtn_code = P3_C_CD_FAULT_CH_V_FAIL;				break;
		case C_CD_FAULT_HW_UPPER_I:
		case C_CD_FAULT_HW_LOWER_I:
			rtn_code = P3_C_CD_FAULT_CH_I_FAIL;				break;
		case C_CD_FAULT_HW_OT:
			rtn_code = P3_C_CD_FAULT_OT;					break;
		case C_CD_FAULT_CHANNEL_PS:
			rtn_code = P3_C_CD_FAULT_CHANNEL_PS;			break;
		case C_CD_FAULT_PS_OV:
			rtn_code = P3_C_CD_FAULT_PS_OV;					break;
		case C_CD_FAULT_PS_OC:
			rtn_code = P3_C_CD_FAULT_PS_OC;					break;
		case C_CD_FAULT_PS_DCOV:
			rtn_code = P3_C_CD_FAULT_PS_DCOV;				break;
		case C_CD_FAULT_IN_FUSE:
			rtn_code = P3_C_CD_FAULT_IN_FUSE;				break;
		case C_CD_FAULT_OUT_FUSE:
			rtn_code = P3_C_CD_FAULT_OUT_FUSE;				break;

		//external fault code
		case C_CD_FAULT_M_CPU_WATCHDOG:
			rtn_code = P3_C_CD_FAULT_CPU_WATCHDOG;			break;
		case C_CD_FAULT_M_AC_POWER:
			rtn_code = P3_C_CD_FAULT_AC_POWER;				break;
		case C_CD_FAULT_M_UPS_BATTERY:
			rtn_code = P3_C_CD_FAULT_UPS_BATTERY;			break;
		case C_CD_FAULT_M_EMG_SWITCH:
			rtn_code = P3_C_CD_FAULT_MAIN_EMG;				break;
		case C_CD_FAULT_M_FORCE_POWER_SWITCH:
			rtn_code = P3_C_CD_FAULT_FORCE_POWER_SWITCH;	break;
		case C_CD_FAULT_M_FORCE_TERMINAL_HALT:
			rtn_code = P3_C_CD_FAULT_FORCE_TERMINAL_HALT;	break;
		case C_CD_FAULT_M_FORCE_TERMINAL_QUIT:
			rtn_code = P3_C_CD_FAULT_FORCE_TERMINAL_QUIT;	break;
		case C_CD_FAULT_G_NETWORK_COMM:
			rtn_code = P3_C_CD_FAULT_NETWORK_COMM;			break;
		case C_CD_FAULT_B_UPPER_TEMP:
			rtn_code = P3_C_CD_FAULT_OT;					break;
		case C_CD_FAULT_B_ADC:
			rtn_code = P3_C_CD_FAULT_ADC;					break;
		case C_CD_FAULT_CONTROL_PS:
			rtn_code = P3_C_CD_FAULT_CONTROL_PS;			break;
		case C_CD_FAULT_CHAMBER_FAULT:
			rtn_code = P3_C_CD_FAULT_CHAMBER_ERROR;			break;

		//check fault code
		case C_CD_FAULT_CHECK_NG:
			rtn_code = P3_C_CD_FAULT_CHECK_CONTACT_BAD;		break;
		case C_CD_FAULT_CHECK_UPPER_OCV:
			rtn_code = P3_C_CD_FAULT_CHECK_UPPER_OCV;		break;
		case C_CD_FAULT_CHECK_LOWER_OCV:
			rtn_code = P3_C_CD_FAULT_CHECK_LOWER_OCV;		break;
		case C_CD_FAULT_CHECK_UPPER_V:
			rtn_code = P3_C_CD_FAULT_CHECK_UPPER_V;			break;
		case C_CD_FAULT_CHECK_LOWER_V:
			rtn_code = P3_C_CD_FAULT_CHECK_LOWER_V;			break;
		case C_CD_FAULT_CHECK_UPPER_I:
			rtn_code = P3_C_CD_FAULT_CHECK_UPPER_I;			break;
		case C_CD_FAULT_CHECK_LOWER_I:
			rtn_code = P3_C_CD_FAULT_CHECK_LOWER_I;			break;
		case C_CD_FAULT_CHECK_ERROR_NO:
			rtn_code = P3_C_CD_FAULT_CHECK_ERROR_NO;		break;
		case C_CD_FAULT_CHECK_ERROR_YES:
			rtn_code = P3_C_CD_FAULT_CHECK_ERROR_YES;		break;

		//soft fault code
		case C_CD_FAULT_UPPER_V:
			rtn_code = P3_C_CD_FAULT_UPPER_V;				break;
		case C_CD_FAULT_LOWER_V:
			rtn_code = P3_C_CD_FAULT_LOWER_V;				break;
		case C_CD_FAULT_UPPER_DELTA_V:
			rtn_code = P3_C_CD_FAULT_UPPER_DELTA_V;			break;
		case C_CD_FAULT_LOWER_DELTA_V:
			rtn_code = P3_C_CD_FAULT_LOWER_DELTA_V;			break;
		case C_CD_FAULT_UPPER_COMP_V1:
			rtn_code = P3_C_CD_FAULT_UPPER_COMP_V1;			break;
		case C_CD_FAULT_LOWER_COMP_V1:
			rtn_code = P3_C_CD_FAULT_LOWER_COMP_V1;			break;
		case C_CD_FAULT_UPPER_COMP_V2:
			rtn_code = P3_C_CD_FAULT_UPPER_COMP_V2;			break;
		case C_CD_FAULT_LOWER_COMP_V2:
			rtn_code = P3_C_CD_FAULT_LOWER_COMP_V2;			break;
		case C_CD_FAULT_UPPER_COMP_V3:
			rtn_code = P3_C_CD_FAULT_UPPER_COMP_V3;			break;
		case C_CD_FAULT_LOWER_COMP_V3:
			rtn_code = P3_C_CD_FAULT_LOWER_COMP_V3;			break;
		case C_CD_FAULT_UPPER_OCV:
			rtn_code = P3_C_CD_FAULT_UPPER_OCV;				break;
		case C_CD_FAULT_LOWER_OCV:
			rtn_code = P3_C_CD_FAULT_LOWER_OCV;				break;
		case C_CD_FAULT_UPPER_I:
			rtn_code = P3_C_CD_FAULT_UPPER_I;				break;
		case C_CD_FAULT_LOWER_I:
			rtn_code = P3_C_CD_FAULT_LOWER_I;				break;
		case C_CD_FAULT_UPPER_DELTA_I:
			rtn_code = P3_C_CD_FAULT_UPPER_DELTA_I;			break;
		case C_CD_FAULT_LOWER_DELTA_I:
			rtn_code = P3_C_CD_FAULT_LOWER_DELTA_I;			break;
		case C_CD_FAULT_UPPER_COMP_I1:
			rtn_code = P3_C_CD_FAULT_UPPER_COMP_I1;			break;
		case C_CD_FAULT_LOWER_COMP_I1:
			rtn_code = P3_C_CD_FAULT_LOWER_COMP_I1;			break;
		case C_CD_FAULT_UPPER_T:
			rtn_code = P3_C_CD_FAULT_UPPER_T;				break;
		case C_CD_FAULT_LOWER_T:
			rtn_code = P3_C_CD_FAULT_LOWER_T;				break;
		case C_CD_FAULT_UPPER_AMPARE_HOUR:
			rtn_code = P3_C_CD_FAULT_UPPER_AMPARE_HOUR;		break;
		case C_CD_FAULT_LOWER_AMPARE_HOUR:
			rtn_code = P3_C_CD_FAULT_LOWER_AMPARE_HOUR;		break;
		case C_CD_FAULT_UPPER_DELTA_C:
			rtn_code = P3_C_CD_FAULT_UPPER_DELTA_C;			break;
		case C_CD_FAULT_LOWER_DELTA_C:
			rtn_code = P3_C_CD_FAULT_LOWER_DELTA_C;			break;
		case C_CD_FAULT_UPPER_COMP_C:
			rtn_code = P3_C_CD_FAULT_UPPER_COMP_C;			break;
		case C_CD_FAULT_LOWER_COMP_C:
			rtn_code = P3_C_CD_FAULT_LOWER_COMP_C;			break;
		case C_CD_FAULT_UPPER_Z:
			rtn_code = P3_C_CD_FAULT_UPPER_Z;				break;
		case C_CD_FAULT_LOWER_Z:
			rtn_code = P3_C_CD_FAULT_LOWER_Z;				break;
		case C_CD_FAULT_UPPER_TEMP:
			rtn_code = P3_C_CD_FAULT_UPPER_TEMP;			break;
		case C_CD_FAULT_LOWER_TEMP:
			rtn_code = P3_C_CD_FAULT_LOWER_TEMP;			break;
		case C_CD_FAULT_DELTA_TEMP:
			rtn_code = P3_C_CD_FAULT_DELTA_TEMP;			break;
		case C_CD_FAULT_UPPER_COMP_I2:
			rtn_code = P3_C_CD_FAULT_UPPER_COMP_I2;			break;
		case C_CD_FAULT_LOWER_COMP_I2:
			rtn_code = P3_C_CD_FAULT_LOWER_COMP_I2;			break;
		case C_CD_FAULT_UPPER_COMP_I3:
			rtn_code = P3_C_CD_FAULT_UPPER_COMP_I3;			break;
		case C_CD_FAULT_LOWER_COMP_I3:
			rtn_code = P3_C_CD_FAULT_LOWER_COMP_I3;			break;

		case C_CD_FAULT_STOP_V:
			rtn_code = P3_C_CD_STOP_V;						break;
		case C_CD_FAULT_STOP_I:
			rtn_code = P3_C_CD_STOP_I;						break;
		case C_CD_FAULT_STOP_T:
			rtn_code = P3_C_CD_STOP_T;						break;
		case C_CD_FAULT_STOP_C:
			rtn_code = P3_C_CD_STOP_C;						break;
		case C_CD_FAULT_STOP_P:
			rtn_code = P3_C_CD_STOP_P;						break;
		case C_CD_FAULT_STOP_WATT_HOUR:
			rtn_code = P3_C_CD_STOP_WATT_HOUR;				break;
		case C_CD_FAULT_STOP_CMD:
			rtn_code = P3_C_CD_FAULT_STOP_CMD;				break;
		case C_CD_FAULT_PAUSE_CMD:
			rtn_code = P3_C_CD_FAULT_PAUSE_CMD;				break;
		case C_CD_FAULT_ACC_CYCLE_COUNT:
			rtn_code = P3_C_CD_FAULT_ACC_CYCLE_COUNT;		break;
		case C_CD_FAULT_UPPER_CAPACITANCE:
			rtn_code = P3_C_CD_FAULT_UPPER_CAPACITANCE;		break;
		case C_CD_FAULT_LOWER_CAPACITANCE:
			rtn_code = P3_C_CD_FAULT_LOWER_CAPACITANCE;		break;
		case C_CD_FAULT_UPPER_WATT_HOUR:
			rtn_code = P3_C_CD_FAULT_UPPER_WATT_HOUR;		break;
		case C_CD_FAULT_LOWER_WATT_HOUR:
			rtn_code = P3_C_CD_FAULT_LOWER_WATT_HOUR;		break;

		//aux code
		case C_CD_AUX_END_UPPER_TEMP:
			rtn_code = P3_C_CD_AUX_END_UPPER_TEMP;			break;
		case C_CD_AUX_END_LOWER_TEMP:
			rtn_code = P3_C_CD_AUX_END_LOWER_TEMP;			break;
		case C_CD_AUX_END_UPPER_V:
			rtn_code = P3_C_CD_AUX_END_UPPER_V;				break;
		case C_CD_AUX_END_LOWER_V:
			rtn_code = P3_C_CD_AUX_END_LOWER_V;				break;
		case C_CD_AUX_END_UPPER_V_BRANCH:
			rtn_code = P3_C_CD_AUX_END_V_UPPER_BRANCH;		break;
		case C_CD_AUX_END_LOWER_V_BRANCH:
			rtn_code = P3_C_CD_AUX_END_V_LOWER_BRANCH;		break;
		case C_CD_AUX_END_UPPER_TEMP_BRANCH:
			rtn_code = P3_C_CD_AUX_END_TEMP_UPPER_BRANCH;	break;
		case C_CD_AUX_END_LOWER_TEMP_BRANCH:
			rtn_code = P3_C_CD_AUX_END_TEMP_LOWER_BRANCH;	break;
		case C_CD_AUX_FAULT_UPPER_TEMP:
			rtn_code = P3_C_CD_AUX_FAULT_UPPER_TEMP;		break;
		case C_CD_AUX_FAULT_LOWER_TEMP:
			rtn_code = P3_C_CD_AUX_FAULT_LOWER_TEMP;		break;
		case C_CD_AUX_FAULT_UPPER_V:
			rtn_code = P3_C_CD_AUX_FAULT_UPPER_V;			break;
		case C_CD_AUX_FAULT_LOWER_V:
			rtn_code = P3_C_CD_AUX_FAULT_LOWER_V;			break;

		//can code
		case C_CD_CAN_END_UPPER:
			rtn_code = P3_C_CD_CAN_END_UPPER;				break;
		case C_CD_CAN_END_LOWER:
			rtn_code = P3_C_CD_CAN_END_LOWER;				break;
		case C_CD_CAN_END_UPPER_V_BRANCH:
			rtn_code = P3_C_CD_CAN_END_V_UPPER_BRANCH;		break;
		case C_CD_CAN_END_LOWER_V_BRANCH:
			rtn_code = P3_C_CD_CAN_END_V_LOWER_BRANCH;		break;
		case C_CD_CAN_END_UPPER_TEMP_BRANCH:
			rtn_code = P3_C_CD_CAN_END_TEMP_UPPER_BRANCH;	break;
		case C_CD_CAN_END_LOWER_TEMP_BRANCH:
			rtn_code = P3_C_CD_CAN_END_TEMP_LOWER_BRANCH;	break;
		case C_CD_CAN_END_FAULT_BRANCH:
			rtn_code = P3_C_CD_CAN_END_FAULT_BRANCH;		break;
		case C_CD_CAN_FAULT_UPPER:
			rtn_code = P3_C_CD_CAN_FAULT_UPPER;				break;
		case C_CD_CAN_FAULT_LOWER:
			rtn_code = P3_C_CD_CAN_FAULT_LOWER;				break;

		default:
			rtn_code = P3_C_CD_FAULT_WORK_ERROR;			break;
	}
#endif
	return rtn_code;
}

int convert_group_trouble_code(int select, int code)
{
	int rtn_code=0;

	if(select == CONVERT_P1_TO_ORG) {
	} else if(select == CONVERT_ORG_TO_P1) {
		rtn_code = convert_group_trouble_code_org_to_p1(code);
	} else if(select == CONVERT_P2_TO_ORG) {
	} else if(select == CONVERT_ORG_TO_P2) {
		rtn_code = convert_group_trouble_code_org_to_p2(code);
	} else if(select == CONVERT_P3_TO_ORG) {
	} else if(select == CONVERT_ORG_TO_P3) {
		rtn_code = convert_group_trouble_code_org_to_p3(code);
	} else {
		rtn_code = 0;
	}

	return rtn_code;
}

int convert_group_trouble_code_org_to_p1(int code)
{
	int rtn_code=0;

	switch(code) {
		case M_CD_NONE:	//kjg_141016
			rtn_code = P1_G_CD_NONE;						break;
		case M_CD_FAULT_AC_POWER_SHORT:
			rtn_code = P1_G_CD_FAULT_AC_POWER_SHORT;		break;
		case M_CD_FAULT_AC_POWER_LONG:
			rtn_code = P1_G_CD_FAULT_AC_POWER_LONG;			break;
		case M_CD_FAULT_UPS_BATTERY:
			rtn_code = P1_G_CD_FAULT_UPS_BATTERY;			break;
		case M_CD_FAULT_GUI_SHUTDOWN:
			rtn_code = P1_G_CD_FAULT_GUI_SHUTDOWN;			break;	//ktg_250410	//jhj_250410
		case M_CD_FAULT_MAIN_EMG_SWITCH:
			rtn_code = P1_G_CD_FAULT_MAIN_EMG_SWITCH;		break;
		case M_CD_FAULT_SUB_EMG_SWITCH:
			rtn_code = P1_G_CD_FAULT_SUB_EMG_SWITCH;		break;
		case M_CD_FAULT_CONTROL_PS:
			rtn_code = P1_G_CD_FAULT_CONTROL_PS;			break;
		case M_CD_FAULT_OT:
			rtn_code = P1_G_CD_FAULT_MODULE_OT;				break;
		case M_CD_FAULT_WARNING_POWER_OFF:
			rtn_code = P1_G_CD_FAULT_WARNING_POWER_OFF;		break;
		case M_CD_FAULT_NORMAL_POWER_OFF:
			rtn_code = P1_G_CD_NONE;						break;
		case M_CD_FAULT_FORCE_POWER_OFF:
			rtn_code = P1_G_CD_FAULT_FORCE_POWER_OFF;		break;
		case M_CD_FAULT_NORMAL_TERMINAL_QUIT:
			rtn_code = P1_G_CD_NONE;						break;
		case M_CD_FAULT_FORCE_TERMINAL_QUIT:
			rtn_code = P1_G_CD_FAULT_FORCE_TERMINAL_QUIT;	break;
		case M_CD_FAULT_NORMAL_TERMINAL_HALT:
			rtn_code = P1_G_CD_NONE;						break;
		case M_CD_FAULT_FORCE_TERMINAL_HALT:
			rtn_code = P1_G_CD_FAULT_FORCE_TERMINAL_HALT;	break;
		case M_CD_FAULT_CPU_WATCHDOG:
			rtn_code = P1_G_CD_FAULT_CPU_WATCHDOG;			break;
		case M_CD_FAULT_CALI_METER_COMM_ERROR:
			rtn_code = P1_G_CD_FAULT_CALI_METER_COMM_ERROR;	break;
		case M_CD_FAULT_CALIBRATOR_COMM_ERROR:
			rtn_code = P1_G_CD_FAULT_CALIBRATOR_COMM_ERROR;	break;
		case M_CD_FAULT_FUSE:
			rtn_code = P1_G_CD_FAULT_FUSE;					break;
		case M_CD_FAULT_MCCB:	//lki_111010
			rtn_code = P1_G_CD_FAULT_MCCB;					break;
		case M_CD_FAULT_UPPER_VOLTAGE:
			rtn_code = P1_G_CD_FAULT_MAIN_UPPER_VOLTAGE;	break;
		case M_CD_FAULT_LOWER_VOLTAGE:
			rtn_code = P1_G_CD_FAULT_MAIN_LOWER_VOLTAGE;	break;
		case M_CD_FAULT_MAIN_PS:
			rtn_code = P1_G_CD_FAULT_MAIN_PS;				break;
		case M_CD_FAULT_FAN:
			rtn_code = P1_G_CD_FAULT_FAN;					break;
		//kjg_w BCR_COMM_ERROR

		case M_CD_FAULT_POWER_TRANS_OT:
			rtn_code = P1_G_CD_FAULT_POWER_TRANS_OT;		break;
		case M_CD_FAULT_POWER_REACTOR_OT:
			rtn_code = P1_G_CD_FAULT_POWER_REACTOR_OT;		break;
		case M_CD_FAULT_OC:	//jhkw_131011
			rtn_code = P1_G_CD_FAULT_OC;					break;
		case M_CD_FAULT_ETC:	//jhkw_131011
			rtn_code = P1_G_CD_FAULT_ETC;					break;
		case M_CD_FAULT_LOAD_LINE:
			rtn_code = P1_G_CD_FAULT_LOAD_LINE;				break;
		case M_CD_FAULT_LOAD_LINE2:
			rtn_code = P1_G_CD_FAULT_LOAD_LINE2;			break;
		case M_CD_FAULT_CAN_WARNNING:
			rtn_code = P1_G_CD_FAULT_CAN_WARNNING;			break;
		case M_CD_FAULT_CAN_ERROR:
			rtn_code = P1_G_CD_FAULT_CAN_ERROR;				break;

		case M_CD_FAULT_CHAMBER_DOOR_OPEN:
			rtn_code = P1_G_CD_FAULT_CHAMBER_DOOR_OPEN;		break;
		case M_CD_FAULT_CHAMBER_OVER_TEMP:
			rtn_code = P1_G_CD_FAULT_CHAMBER_HEATER_OVER_TEMP;	break;
		case M_CD_FAULT_CHAMBER_AIR:
			rtn_code = P1_G_CD_FAULT_CHAMBER_AIR;			break;
		case M_CD_FAULT_CHAMBER_COIL_TRIP:
			rtn_code = P1_G_CD_FAULT_CHAMBER_EMG;			break;
		case M_CD_FAULT_CHAMBER_REF:
			rtn_code = P1_G_CD_FAULT_CHAMBER_EOCR;			break;
		case M_CD_FAULT_CHAMBER_FAN:
			rtn_code = P1_G_CD_FAULT_CHAMBER_FAN;			break;
		case M_CD_FAULT_CHAMBER_FIRE:
			rtn_code = P1_G_CD_FAULT_CHAMBER_FIRE;			break;
		case M_CD_FAULT_CHAMBER_SMOKE:
			rtn_code = P1_G_CD_FAULT_CHAMBER_SMOKE;			break;
		case M_CD_FAULT_CHAMBER_POWER:
			rtn_code = P1_G_CD_FAULT_CHAMBER_POWER;			break;
		case M_CD_FAULT_CHAMBER_REF_ERROR:
			rtn_code = P1_G_CD_FAULT_CHAMBER_REF_ERROR;		break;
		case M_CD_FAULT_CHAMBER_LEAK:	//lki_111010
			rtn_code = P1_G_CD_FAULT_CHAMBER_LEAK;			break;
		case M_CD_FAULT_CHAMBER_CP_TRIP:	//lki_111010
			rtn_code = P1_G_CD_FAULT_CHAMBER_CP_TRIP;		break;
		case M_CD_FAULT_TEMP_LINE:
			rtn_code = P1_G_CD_FAULT_TEMP_CABLE_ERROR;		break;
		case M_CD_FAULT_LEAKAGE_CURRENT:	//ktg_211104
			rtn_code = P1_G_CD_FAULT_LEAKAGE_CURRENT;		break;
		case M_CD_FAULT_DC_LINK_FUSE:	//phb_230710
			rtn_code = P1_G_CD_FAULT_DC_LINK_FUSE;   		break;

		case G_CD_FAULT_UPPER_VOLTAGE:
			rtn_code = P1_G_CD_FAULT_UPPER_VOLTAGE;			break;
		case G_CD_FAULT_UPPER_CURRENT:
			rtn_code = P1_G_CD_FAULT_UPPER_CURRENT;			break;
		case G_CD_FAULT_RUN_TIME_OVER:
			rtn_code = P1_G_CD_FAULT_RUN_TIME_OVER;			break;
		case G_CD_FAULT_DATASAVE_PROCESS_ERROR:
			rtn_code = P1_G_CD_FAULT_DATASAVE_PROCESS_ERROR;	break;
		case G_CD_FAULT_ADC:
			rtn_code = P1_G_CD_FAULT_GROUP_ADC;				break;
		case G_CD_FAULT_OT:
			rtn_code = P1_G_CD_FAULT_GROUP_OT;				break;
		case B_CD_FAULT_OT:
			rtn_code = P1_G_CD_FAULT_BD_OT;					break;
		case B_CD_FAULT_ADC:
			rtn_code = P1_G_CD_FAULT_BD_ADC;				break;

		default:
			//kjg_101218 rtn_code = P1_G_CD_FAULT_HOLD;					break;
			rtn_code = code + 1000;							break;
	}
	return rtn_code;
}

int convert_group_trouble_code_org_to_p2(int code)
{
	int rtn_code=0;
#if defined __COB__
	switch(code) {
		case M_CD_NONE: //kjg_141016
			rtn_code = P2_FAIL_NONE;						break;
		case M_CD_FAULT_AC_POWER_SHORT:
			rtn_code = P2_FAIL_AC_POWER_SHORT;				break;
		case M_CD_FAULT_AC_POWER_LONG:
			rtn_code = P2_FAIL_AC_POWER_LONG;				break;
		case M_CD_FAULT_UPS_BATTERY:
			rtn_code = P2_FAIL_UPS_BATTERY;					break;
		case M_CD_FAULT_MAIN_EMG_SWITCH:
			rtn_code = P2_FAIL_MAIN_EMG;					break;
		case M_CD_FAULT_SUB_EMG_SWITCH:
			rtn_code = P2_FAIL_SUB_EMG;						break;
		case M_CD_FAULT_CONTROL_PS:
			rtn_code = P2_FAIL_SMPS;						break;
		case M_CD_FAULT_OT:
			rtn_code = P2_FAIL_OT;							break;
		case M_CD_FAULT_WARNING_POWER_OFF:
			rtn_code = 0;									break;
		case M_CD_FAULT_NORMAL_POWER_OFF:
			rtn_code = 0;									break;
		case M_CD_FAULT_FORCE_POWER_OFF:
			rtn_code = P2_FAIL_FORCE_POWER;					break;
		case M_CD_FAULT_NORMAL_TERMINAL_QUIT:
			rtn_code = 0;									break;
		case M_CD_FAULT_FORCE_TERMINAL_QUIT:
			rtn_code = P2_FAIL_FORCE_POWER;					break;
		case M_CD_FAULT_NORMAL_TERMINAL_HALT:
			rtn_code = 0;									break;
		case M_CD_FAULT_FORCE_TERMINAL_HALT:
			rtn_code = P2_FAIL_FORCE_POWER;					break;
		case M_CD_FAULT_CPU_WATCHDOG:
			rtn_code = P2_FAIL_CPU_WATCHDOG;				break;
		case M_CD_FAULT_CALI_METER_COMM_ERROR:
			rtn_code = P2_FAIL_CALI_METER_COMM_ERROR;		break;
		//kjg_w BCR_COMM_ERROR
		case M_CD_FAULT_CALIBRATOR_COMM_ERROR:
			rtn_code = P2_FAIL_CALIBRATOR_COMM_ERROR;		break;

		case M_CD_FAULT_FUSE:
		case M_CD_FAULT_UPPER_VOLTAGE:
		case M_CD_FAULT_LOWER_VOLTAGE:
		case M_CD_FAULT_MAIN_PS:
			rtn_code = P2_FAIL_SMPS;						break;
		case M_CD_FAULT_FAN:
			rtn_code = P2_FAIL_FAN;							break;
		case M_CD_FAULT_LOAD_LINE:
			rtn_code = P2_FAIL_LOAD_LINE;					break;

		case M_CD_FAULT_CHAMBER:
			rtn_code = P2_FAIL_CHAMBER;						break;
		case M_CD_FAULT_CHAMBER_FIRE:
			rtn_code = P2_FAIL_CHAMBER_FIRE;				break;
		case M_CD_FAULT_CHAMBER_DOOR_OPEN:
			rtn_code = P2_FAIL_CHAMBER_DOOR_OPEN;			break;
		case M_CD_FAULT_JOB_CHANGE:
			rtn_code = P2_FAIL_JOB_CHANGE;					break;
		case M_CD_FAULT_PITCH_CHANGE:
			rtn_code = P2_FAIL_PITCH_CHANGE;				break;
		case M_CD_FAULT_CHAMBER_AREA_SENSOR:
			rtn_code = P2_FAIL_CHAMBER_AREA_SENSOR;			break;
		case M_CD_FAULT_CHAMBER_MANUAL_DOOR:
			rtn_code = P2_FAIL_CHAMBER_MANUAL_DOOR;			break;
		case M_CD_FAULT_CHAMBER_AIR:
			rtn_code = P2_FAIL_CHAMBER_AIR;					break;
		case M_CD_FAULT_CHAMBER_APR_S:
			rtn_code = P2_FAIL_CHAMBER_APR_S;				break;
		case M_CD_FAULT_CHAMBER_REF:
			rtn_code = P2_FAIL_CHAMBER_REF;					break;
		case M_CD_FAULT_CHAMBER_COIL_TRIP:
			rtn_code = P2_FAIL_CHAMBER_COIL_TRIP;			break;

		case G_CD_FAULT_UPPER_VOLTAGE:
			rtn_code = P2_FAIL_UPPER_VOLTAGE;			break;
		case G_CD_FAULT_UPPER_CURRENT:
			rtn_code = P2_FAIL_UPPER_CURRENT;			break;
		case G_CD_FAULT_RUN_TIME_OVER:
			rtn_code = P2_FAIL_RUN_TIME_OVER;			break;

//		case G_CD_FAULT_DATASAVE_PROCESS_ERROR:
//			rtn_code = P2_G_CD_FAULT_DATASAVE_PROCESS_ERROR;	break;
		case G_CD_FAULT_ADC:
			rtn_code = P2_FAIL_AD_PART;						break;
		case G_CD_FAULT_OT:
			rtn_code = P2_FAIL_OT;							break;

		case B_CD_FAULT_OT:
			rtn_code = P2_FAIL_OT;							break;
		case B_CD_FAULT_ADC:
			rtn_code = P2_FAIL_AD_PART;						break;

		case G_CD_FAULT_JIG_ACTIVE_ERROR:
		case G_CD_FAULT_J_MAIN_CYL_UP_TIMEOUT:
		case G_CD_FAULT_J_MAIN_CYL_DOWN_TIMEOUT:
		case G_CD_FAULT_J_LATCH_CYL_OPEN_TIMEOUT:
		case G_CD_FAULT_J_LATCH_CYL_CLOSE_TIMEOUT:
		case G_CD_FAULT_J_GRIP_CYL_OPEN_TIMEOUT:
		case G_CD_FAULT_J_GRIP_CYL_CLOSE_TIMEOUT:
		case G_CD_FAULT_J_MAIN_CYL_UP_SENS:
		case G_CD_FAULT_J_MAIN_CYL_DOWN_SENS:
		case G_CD_FAULT_J_LATCH_CYL_OPEN_SENS:
		case G_CD_FAULT_J_LATCH_CYL_CLOSE_SENS:
		case G_CD_FAULT_J_GRIP_CYL_OPEN_SENS:
		case G_CD_FAULT_J_GRIP_CYL_CLOSE_SENS:
			rtn_code = P2_FAIL_JIG_ACTIVE_ERROR;			break;

		case G_CD_FAULT_JIG_TRAY_ERROR:
			rtn_code = P2_FAIL_JIG_TRAY_ERROR;				break;
		case G_CD_FAULT_JIG_SMOKE_ERROR:
			rtn_code = P2_FAIL_JIG_SMOKE_ERROR;				break;
		//case M_CD_FAULT_CHAMBER_FIRE:
		//	rtn_code = P2_FAIL_CHAMBER_FIRE;				break;
		case G_CD_FAULT_JIG_TEMP_ERROR:
			rtn_code = P2_FAIL_JIG_TEMP_ERROR;				break;
		case G_CD_FAULT_JIG_GAS_ERROR:
			rtn_code = P2_FAIL_JIG_GAS_ERROR;				break;
		case G_CD_FAULT_JIG_DOOR_ERROR:
			rtn_code = P2_FAIL_JIG_DOOR_ERROR;				break;
		case G_CD_FAULT_JIG_AIR_PRESS_ERROR:
			rtn_code = P2_FAIL_JIG_AIR_PRESS_ERROR;			break;
		case G_CD_FAULT_JIG_STACKER_ERROR:
			rtn_code = P2_FAIL_JIG_STACKER_ERROR;			break;
		default:
			//kjg_101218 rtn_code = P2_FAIL_HOLD;						break;
			rtn_code = code + 1000;							break;
	}
#endif
	return rtn_code;
}

int convert_group_trouble_code_org_to_p3(int code)
{
	int rtn_code=0;
#if defined __COC__
	switch(code) {
		case M_CD_NONE: //kjg_141016
			rtn_code = P3_G_CD_NONE;						break;
		case M_CD_FAULT_AC_POWER_SHORT:
			rtn_code = P3_G_CD_FAULT_AC_POWER_SHORT;		break;
		case M_CD_FAULT_AC_POWER_LONG:
			rtn_code = P3_G_CD_FAULT_AC_POWER_LONG;			break;
		case M_CD_FAULT_UPS_BATTERY:
			rtn_code = P3_G_CD_FAULT_UPS_BATTERY;			break;
		case M_CD_FAULT_MAIN_EMG_SWITCH:
			rtn_code = P3_G_CD_FAULT_MAIN_EMG_SWITCH;		break;
		case M_CD_FAULT_SUB_EMG_SWITCH:
			rtn_code = P3_G_CD_FAULT_SUB_EMG_SWITCH;		break;
		case M_CD_FAULT_CONTROL_PS:
			rtn_code = P3_G_CD_FAULT_CONTROL_PS;			break;
		case M_CD_FAULT_OT:
			rtn_code = P3_G_CD_FAULT_MODULE_OT;				break;
		case M_CD_FAULT_WARNING_POWER_OFF:
			rtn_code = P3_G_CD_FAULT_WARNING_POWER_OFF;		break;
		case M_CD_FAULT_NORMAL_POWER_OFF:
			rtn_code = P3_G_CD_NONE;						break;
		case M_CD_FAULT_FORCE_POWER_OFF:
			rtn_code = P3_G_CD_FAULT_FORCE_POWER_OFF;		break;
		case M_CD_FAULT_NORMAL_TERMINAL_QUIT:
			rtn_code = P3_G_CD_NONE;						break;
		case M_CD_FAULT_FORCE_TERMINAL_QUIT:
			rtn_code = P3_G_CD_FAULT_FORCE_TERMINAL_QUIT;	break;
		case M_CD_FAULT_NORMAL_TERMINAL_HALT:
			rtn_code = P3_G_CD_NONE;						break;
		case M_CD_FAULT_FORCE_TERMINAL_HALT:
			rtn_code = P3_G_CD_FAULT_FORCE_TERMINAL_HALT;	break;
		case M_CD_FAULT_CPU_WATCHDOG:
			rtn_code = P3_G_CD_FAULT_CPU_WATCHDOG;			break;
		case M_CD_FAULT_CALI_METER_COMM_ERROR:
			rtn_code = P3_G_CD_FAULT_CALI_METER_COMM_ERROR;	break;
		case M_CD_FAULT_CALIBRATOR_COMM_ERROR:
			rtn_code = P3_G_CD_FAULT_CALIBRATOR_COMM_ERROR;	break;
		case M_CD_FAULT_FUSE:
			rtn_code = P3_G_CD_FAULT_FUSE;					break;
		case M_CD_FAULT_UPPER_VOLTAGE:
			rtn_code = P3_G_CD_FAULT_MAIN_UPPER_VOLTAGE;	break;
		case M_CD_FAULT_LOWER_VOLTAGE:
			rtn_code = P3_G_CD_FAULT_MAIN_LOWER_VOLTAGE;	break;
		case M_CD_FAULT_MAIN_PS:
			rtn_code = P3_G_CD_FAULT_MAIN_PS;				break;
		case M_CD_FAULT_FAN:
			rtn_code = P3_G_CD_FAULT_FAN;					break;

		case M_CD_FAULT_CHAMBER_DOOR_OPEN:
			rtn_code = P3_G_CD_FAULT_CHAMBER_DOOR_OPEN;		break;
		case M_CD_FAULT_CHAMBER_OVER_TEMP:
			rtn_code = P3_G_CD_FAULT_CHAMBER_HEATER_OVER_TEMP;	break;
		case M_CD_FAULT_CHAMBER_AIR:
			rtn_code = P3_G_CD_FAULT_CHAMBER_AIR;			break;
		case M_CD_FAULT_CHAMBER_COIL_TRIP:
			rtn_code = P3_G_CD_FAULT_CHAMBER_EMG;			break;
		case M_CD_FAULT_CHAMBER_REF:
			rtn_code = P3_G_CD_FAULT_CHAMBER_EOCR;			break;
		case M_CD_FAULT_CHAMBER_FAN:
			rtn_code = P3_G_CD_FAULT_CHAMBER_FAN;			break;
		case M_CD_FAULT_CHAMBER_FIRE:
			rtn_code = P3_G_CD_FAULT_CHAMBER_FIRE;			break;
		case M_CD_FAULT_CHAMBER_SMOKE:
			rtn_code = P3_G_CD_FAULT_CHAMBER_SMOKE;			break;
		case M_CD_FAULT_CHAMBER_POWER:
			rtn_code = P3_G_CD_FAULT_CHAMBER_POWER;			break;
		case M_CD_FAULT_CHAMBER_LEAK: //lki_111010
			rtn_code = P3_G_CD_FAULT_CHAMBER_LEAK;			break;
		case M_CD_FAULT_CHAMBER_CP_TRIP: //lki_111010
			rtn_code = P3_G_CD_FAULT_CHAMBER_CP_TRIP;		break;

		case M_CD_FAULT_LOAD_LINE:
			rtn_code = P3_G_CD_FAULT_LOAD_CABLE_ERROR;		break;
		case M_CD_FAULT_LOAD_LINE2:
			rtn_code = P3_G_CD_FAULT_LOAD_CABLE_ERROR2;		break;

		case G_CD_FAULT_UPPER_VOLTAGE:
			rtn_code = P3_G_CD_FAULT_UPPER_VOLTAGE;			break;
		case G_CD_FAULT_UPPER_CURRENT:
			rtn_code = P3_G_CD_FAULT_UPPER_CURRENT;			break;
		case G_CD_FAULT_RUN_TIME_OVER:
			rtn_code = P3_G_CD_FAULT_RUN_TIME_OVER;			break;
		case G_CD_FAULT_DATASAVE_PROCESS_ERROR:
			rtn_code = P3_G_CD_FAULT_DATASAVE_PROCESS_ERROR;	break;
		case G_CD_FAULT_ADC:
			rtn_code = P3_G_CD_FAULT_AD_PART_ERROR;			break;
		case G_CD_FAULT_OT:
			rtn_code = P3_G_CD_FAULT_GROUP_OT;				break;

		case B_CD_FAULT_OT:
			rtn_code = P3_G_CD_FAULT_BD_OT;					break;
		case B_CD_FAULT_ADC:
			rtn_code = P3_G_CD_FAULT_BD_ADC;				break;

		default:
			//kjg_101218 rtn_code = P3_G_CD_FAULT_HOLD;					break;
			rtn_code = code + 1000;							break;
	}
#endif
	return rtn_code;
}

int GradeCodeCheck(int ch, unsigned char idxStepNo, long val)
{
	unsigned char count;
	int i, code;
	long lower, upper;

	count = myData->testCond[ch].grade[idxStepNo].totalGrade;

	code = 0;
	for(i=0; i < count; i++) {
		lower = myData->testCond[ch].grade[idxStepNo].value1[i];
		upper = myData->testCond[ch].grade[idxStepNo].value2[i];
		if(lower <= val && upper > val) {
			code = (int)myData->testCond[ch].grade[idxStepNo].code[i];
			return code;
		}
	}

	return code;
}

unsigned char Read_InPoint(int group, int ch, short int function)
{
	unsigned char rtn;
	int i;

	rtn = OFF;

	for(i=0; i < MAX_DIO_IN_FUNCTION; i++) {
		if((int)myData->dio.function_set.in_set[i].group != group) continue;
		if((int)myData->dio.function_set.in_set[i].ch != ch) continue;
		if(myData->dio.function_set.in_set[i].function != function) continue;

		if(myData->dio.function_set.in_set[i].use == USE)
			rtn = myData->dio.in.function[i];
		break;
	}

	return rtn;
}

void Select_OutPoint(int group, int ch, short int function, unsigned char val)
{
	int i;

	if(myData->dio.config.dio_Control_Flag == P0) return;

	for(i=0; i < MAX_DIO_OUT_FUNCTION; i++) {
		if((int)myData->dio.function_set.out_set[i].group != group) continue;
		if((int)myData->dio.function_set.out_set[i].ch != ch) continue;
		if(myData->dio.function_set.out_set[i].function != function) continue;

		if(myData->dio.function_set.out_set[i].use == USE)
			myData->dio.out.function[i] = val;
		break;
	}
}
void cFind_Temp_Cali_Data(long temp, int index)
{	//khj_210802
	unsigned char i=0, index_max=0;
	unsigned char j=0;

	for(i=0; i < MAX_CALI_TEMP_POINT; i++) {
		if(temp < myData->measure_cali_temp[0][index].temp[i]) break;
		if(i > 0) {
			j++;
		}
	}

	if(myData->measure_cali_temp[0][index].temp[MAX_CALI_TEMP_POINT - 1] == 0) { 	
		for(i=0; i < MAX_CALI_TEMP_POINT; i++) {
			if(myData->measure_cali_temp[0][index].temp[i] == 0) {
				break;
			}
			if(i > 0) {
				index_max++;
			}
		}
	} else {
		index_max = MAX_CALI_TEMP_POINT - 1;
	}

	if(temp <= myData->measure_cali_temp[0][index].temp[0]) {
		myData->measure_cali[0][index].offset
			= myData->measure_cali_temp[0][index].offset[0];
		myData->measure_cali[0][index].factor
			= myData->measure_cali_temp[0][index].factor[0];
	} else if(temp >= myData->measure_cali_temp[0][index].temp[index_max]) {
		myData->measure_cali[0][index].offset
			= myData->measure_cali_temp[0][index].offset[index_max-1];
		myData->measure_cali[0][index].factor
			= myData->measure_cali_temp[0][index].factor[index_max-1];
	} else {
		myData->measure_cali[0][index].offset
			= myData->measure_cali_temp[0][index].offset[j];
		myData->measure_cali[0][index].factor
			= myData->measure_cali_temp[0][index].factor[j];
	}
}

#ifndef __KERNEL__
long string_to_long(char *buf, int mode, int div)
{
	char tmp[16], tmpbuf[40];
	int i, j, k;
	long val, t_hour, t_min, t_sec;

	memset(tmpbuf, 0, sizeof tmpbuf);
	strcpy((char *)&tmpbuf[0], buf);

	if(div == 1) {
		if(mode == MODE_CC) {
			//val = (long)(atof(tmpbuf) * 1000000.0);
			val = (long)(atof(tmpbuf) * myData->mData.patt_ratioI);	//kjh_220321
		} else if(mode == MODE_CP) {
			//val = (long)(atof(tmpbuf) * 1000.0);
			val = (long)(atof(tmpbuf) * myData->mData.patt_ratioP);	//kjh_220321
		} else if(mode == MODE_CV) { //kjhw_150210
			//val = (long)(atof(tmpbuf) * 1000000.0);
			val = (long)(atof(tmpbuf) * myData->mData.patt_ratioV);	//kjh_220321
		} else {
			val = 0;
		}

		return val;
	}

	if(div == 2) { //kjhw_140828 usemap
		if(mode == MODE_CC) {
			//val = (long)(atof(tmpbuf) * 1000000.0);
			val = (long)(atof(tmpbuf) * myData->mData.patt_ratioI);	//kjh_220321
		} else if(mode == MODE_CP) {
			//val = (long)(atof(tmpbuf) * 1000.0);
			val = (long)(atof(tmpbuf) * myData->mData.patt_ratioP);	//kjh_220321
		} else {
			val = 0;
		}

		return val;
	}

	if(div == 3) { //kjhw_140828 usemap
		val = (long)(atof(tmpbuf) * 1000.0);
		return val;
	}

	val = 0;
	for(i=0; i < strlen(tmpbuf); i++) {
		if(tmpbuf[i] == ':') {
			val = 1;
			break;
		}
	}

	if(val == 0) {
		//jhkw_161026s
		if(div == 4) { //temperature
			val = (long)(atof(tmpbuf) * 1000.0);
		} else {
			val = (long)(atof(tmpbuf) * 100.0 + 0.1);
		}
		//val = (long)(atof(tmpbuf) * 100.0 + 0.1);
		//jhkw_161026e
		//val = (long)(atof(tmpbuf) * 100.0 + 0.01);
	} else {
		t_hour = 0; t_min = 0; t_sec = 0;

		j = 0; k = 0;
		memset(tmp, 0, sizeof tmp);
		for(i=0; i < strlen(tmpbuf); i++) {
			if(tmpbuf[i] == ':') {
				j = 0;
				switch(k) {
					case 0:
						t_hour = atol(tmp);
						k++;
						memset(tmp, 0, sizeof tmp);
						break;
					case 1:
						t_min = atol(tmp);
						k++;
						memset(tmp, 0, sizeof tmp);
						break;
				}
			} else {
				tmp[j] = tmpbuf[i];
				j++;
			}
		}
		t_sec = (long)(atof(tmp) * 100.0 + 0.1);
		//t_sec = (long)(atof(tmp) * 100.0 + 0.01);

		if(k == 1) {
			val = (t_hour * 60) * 100;
		} else {
			val = ((t_hour * 3600) + (t_min * 60)) * 100;
		}
		val += t_sec;
	}

	return val;
}

int Read_Pattern_File_1(char *psName1, int ch, int pattern_index)
{
	char temp[32], buf[32], fileName[128], *in_delimiter = " ,\t\n\r", *token;
	int tmp, i, mode, groupNo;
	FILE *fp;

	groupNo = 0; //kjg_w

	userlog(DEBUG_LOG, psName1, "ch%d pattern file read %d\n",
		ch+1, pattern_index+1);

	memset(fileName, 0, sizeof(fileName));
	sprintf(fileName,
		"/root/%s/config/pattern/group%d/ch%03d/pattern_data_%02d.csv",
		(char *)&myData->AppControl.misc.path1,
		groupNo+1, ch+1, pattern_index+1);
	// /root/system_data/config/pattern/group#/ch###/pattern_data_##.csv
    if((fp = fopen(fileName, "r")) == NULL) {
		userlog(DEBUG_LOG, psName1, "ch%d pattern file read error %d\n",
			ch+1, pattern_index+1);
		return -1;
	}

	memset((char *)&myData->testCond[ch].pattern[0], 0,
		sizeof(S_TEST_COND_PATTERN_DATA) * MAX_PATTERN_DATA);

	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	token = strtok(temp, in_delimiter);
	if(token == NULL) {
	}

	memset(buf, 0, sizeof buf);
	strcpy(buf, token);
	if((strncmp(buf, "t1", 2) == 0) || (strncmp(buf, "T1", 2) == 0)) {
		//accumulation time
		myData->testCond[ch].pattern[0].t_val = 0;
	} else if((strncmp(buf, "t2", 2) == 0) || (strncmp(buf, "T2", 2) == 0)) {
		//displacement time
		myData->testCond[ch].pattern[0].t_val = 1;
	} else {
	}
	//printf("kjg_d 1:%s\n", buf);

	mode = MODE_IDLE;
	//memset(temp, 0, sizeof temp);
	//tmp = fscanf(fp, "%s", temp);
	//token = strtok(temp, in_delimiter);
	token = strtok(NULL, in_delimiter);
	if(token == NULL) {
	}
	memset(buf, 0, sizeof buf);
	strcpy(buf, token);
	if((strncmp(buf, "i", 1) == 0) || (strncmp(buf, "I", 1) == 0)) {
		mode = MODE_CC;
		myData->testCond[ch].pattern[0].cmd_val[0] = 0;
		myData->testCond[ch].pattern[0].cmd_val[1]
			= myData->testCond[ch].pattern[0].cmd_val[0];
	} else if((strncmp(buf, "p", 1) == 0) || (strncmp(buf, "P", 1) == 0)) {
		mode = MODE_CP;
		myData->testCond[ch].pattern[0].cmd_val[0] = 1;
		myData->testCond[ch].pattern[0].cmd_val[1]
			= myData->testCond[ch].pattern[0].cmd_val[0];
	} else {
	}
	//printf("kjg_d 2:%s\n", buf);

	for(i=1; i < MAX_P1_PATTERN_DATA; i++) {
		memset(temp, 0, sizeof temp);
		tmp = fscanf(fp, "%s", temp);
		token = strtok(temp, in_delimiter);
		if(token == NULL) {
			break;
		}
		memset(buf, 0, sizeof buf);
		strcpy(buf, token);
		if((strncmp(buf, "EOF", 3) == 0) || (strncmp(buf, "eof", 3) == 0)) {
		} else {
			myData->testCond[ch].pattern[i].t_val
				= string_to_long(buf, mode, 0);
		}
		//printf("kjg_d 3:%s\n", buf);

		//memset(temp, 0, sizeof temp);
		//tmp = fscanf(fp, "%s", temp);
		//token = strtok(temp, in_delimiter);
		token = strtok(NULL, in_delimiter);
		if(token == NULL) {
			break;
		}
		memset(buf, 0, sizeof buf);
		strcpy(buf, token);
		if((strncmp(buf, "EOF", 3) == 0) || (strncmp(buf, "eof", 3) == 0)) {
		} else {
			myData->testCond[ch].pattern[i].cmd_val[0]
				= string_to_long(buf, mode, 1);
			myData->testCond[ch].pattern[i].cmd_val[1]
				= myData->testCond[ch].pattern[i].cmd_val[0];
		}
		//printf("kjg_d 4:%s, %ld\n", buf,
		//	myData->testCond[ch].pattern[i].cmd_val[0]);

		myData->testCond[ch].pattern[i].waveform_type = 0;
			//0:rectangle, 1:triangle
	}

	myData->testCond[ch].pattern[i].t_val = (-1);

	fclose(fp);
	return 0;
}

int Write_Pattern_File_1(char *psName1, int ch, int pattern_index, char *pattern)
{
	char fileName[128];
	int groupNo;
	FILE *fp;

	groupNo = 0; //kjg_w

	memset(fileName, 0, sizeof(fileName));
	sprintf(fileName,
		"/root/%s/config/pattern/group%d/ch%03d/pattern_data_%02d.csv",
		(char *)&myData->AppControl.misc.path1,
		groupNo+1, ch+1, pattern_index+1);
	// /root/system_data/config/pattern/group#/ch###/pattern_data_##.csv
	if((fp = fopen(fileName, "w")) == NULL) {
		userlog(DEBUG_LOG, psName1, "ch%d pattern file write error %d\n",
			ch+1, pattern_index+1);
		return -1;
	}

	fprintf(fp, "%s", pattern);

	fclose(fp);
	return 0;
}

int Read_Pattern_File_2(char *psName1, int ch, int pattern_index)
{
#if defined __COC__
	//kjg_100208_w P1_ P3_
	char temp[32], buf[32], fileName[128], *in_delimiter = " ,\t\n\r", *token;
	int tmp, i, mode, groupNo;
	FILE *fp;

	groupNo = 0; //kjg_w

	userlog(DEBUG_LOG, psName1, "ch%d pattern file read %d\n",
		ch+1, pattern_index+1);

	memset(fileName, 0, sizeof(fileName));
	sprintf(fileName,
		"/root/%s/config/pattern/group%d/ch%03d/pattern_data_%02d.csv",
		(char *)&myData->AppControl.misc.path1,
		groupNo+1, ch+1, pattern_index+1);
	// /root/system_data/config/pattern/group#/ch###/pattern_data_##.csv
	if((fp = fopen(fileName, "r")) == NULL) {
		userlog(DEBUG_LOG, psName1, "ch%d pattern file read error %d\n",
			ch+1, pattern_index+1);
		return -1;
	}

	memset((char *)&myData->testCond[ch].pattern[0], 0,
		sizeof(S_TEST_COND_PATTERN_DATA) * MAX_PATTERN_DATA);

	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	token = strtok(temp, in_delimiter);
	if(token == NULL) {
	}
	memset(buf, 0, sizeof buf);
	strcpy(buf, token);
	if((strncmp(buf, "t1", 2) == 0) || (strncmp(buf, "T1", 2) == 0)) {
		//accumulation time
		myData->testCond[ch].pattern[0].t_val = 0;
	} else if((strncmp(buf, "t2", 2) == 0) || (strncmp(buf, "T2", 2) == 0)) {
		//displacement time
		myData->testCond[ch].pattern[0].t_val = 1;
	} else {
	}
	//printf("kjg_d 1:%s\n", buf);

	mode = MODE_IDLE;
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	token = strtok(temp, in_delimiter);
	if(token == NULL) {
	}
	memset(buf, 0, sizeof buf);
	strcpy(buf, token);
	if((strncmp(buf, "i", 1) == 0) || (strncmp(buf, "I", 1) == 0)) {
		mode = MODE_CC;
		myData->testCond[ch].pattern[0].cmd_val[0] = 0;
		myData->testCond[ch].pattern[0].cmd_val[1]
			= myData->testCond[ch].pattern[0].cmd_val[0];
	} else if((strncmp(buf, "p", 1) == 0) || (strncmp(buf, "P", 1) == 0)) {
		mode = MODE_CP;
		myData->testCond[ch].pattern[0].cmd_val[0] = 1;
		myData->testCond[ch].pattern[0].cmd_val[1]
			= myData->testCond[ch].pattern[0].cmd_val[0];
	} else {
	}
	//printf("kjg_d 2:%s\n", buf);

	for(i=1; i < MAX_P3_PATTERN_DATA; i++) {
		memset(temp, 0, sizeof temp);
		tmp = fscanf(fp, "%s", temp);
		token = strtok(temp, in_delimiter);
		if(token == NULL) {
			break;
		}
		memset(buf, 0, sizeof buf);
		strcpy(buf, token);
		if((strncmp(buf, "EOF", 3) == 0) || (strncmp(buf, "eof", 3) == 0)) {
		} else {
			myData->testCond[ch].pattern[i].t_val
				= string_to_long(buf, mode, 0);
		}
		//printf("kjg_d 3:%s\n", buf);

		memset(temp, 0, sizeof temp);
		tmp = fscanf(fp, "%s", temp);
		token = strtok(temp, in_delimiter);
		if(token == NULL) {
			break;
		}
		memset(buf, 0, sizeof buf);
		strcpy(buf, token);
		if((strncmp(buf, "EOF", 3) == 0) || (strncmp(buf, "eof", 3) == 0)) {
		} else {
			myData->testCond[ch].pattern[i].cmd_val[0]
				= string_to_long(buf, mode, 1);
		}
		//printf("kjg_d 4:%s, %ld\n", buf,
		//	myData->testCond[ch].pattern[i].cmd_val[0]);

		memset(temp, 0, sizeof temp);
		tmp = fscanf(fp, "%s", temp);
		token = strtok(temp, in_delimiter);
		if(token == NULL) {
			break;
		}
		memset(buf, 0, sizeof buf);
		strcpy(buf, token);
		if((strncmp(buf, "EOF", 3) == 0) || (strncmp(buf, "eof", 3) == 0)) {
		} else {
			myData->testCond[ch].pattern[i].cmd_val[1]
				= string_to_long(buf, mode, 1);
		}
		//printf("kjg_d 5:%s, %ld\n", buf,
		//	myData->testCond[ch].pattern[i].cmd_val[1]);

		memset(temp, 0, sizeof temp);
		tmp = fscanf(fp, "%s", temp);
		token = strtok(temp, in_delimiter);
		if(token == NULL) {
			break;
		}
		memset(buf, 0, sizeof buf);
		strcpy(buf, token);
		if((strncmp(buf, "EOF", 3) == 0) || (strncmp(buf, "eof", 3) == 0)) {
		} else {
			myData->testCond[ch].pattern[i].waveform_type
				= (unsigned char)atoi(buf);
		}
		//printf("kjg_d 6:%s, %d\n", buf,
		//	(int)myData->testCond[ch].pattern[i].waveform_type);
	}

	myData->testCond[ch].pattern[i].t_val = (-1);

	fclose(fp);
#endif
	return 0;
}

int Write_Pattern_File_2(char *psName1, int ch, int pattern_index, char *rcv_cmd)
{
#if defined __COC__
	char fileName[128];
	int groupNo, point, data_size, i;
	FILE *fp;
	S_P3_RCV_CMD_TESTCOND_PATTERN cmd;
	S_TEST_COND_PATTERN_DATA pattern;

	groupNo = 0; //kjg_w

	memset(fileName, 0, sizeof(fileName));
	sprintf(fileName,
		"/root/%s/config/pattern/group%d/ch%03d/pattern_data_%02d.csv",
		(char *)&myData->AppControl.misc.path1,
		groupNo+1, ch+1, pattern_index+1);
	// /root/system_data/config/pattern/group#/ch###/pattern_data_##.csv
	if((fp = fopen(fileName, "w")) == NULL) {
		userlog(DEBUG_LOG, psName1, "ch%d pattern file write error %d\n",
			ch+1, pattern_index+1);
		return -1;
	}

	memcpy((char *)&cmd, rcv_cmd, sizeof(S_P3_RCV_CMD_TESTCOND_PATTERN));

	if(cmd.testCondPattern.type == 0x02) {
		fprintf(fp, "t1, i\n");

		point = sizeof(S_P3_CMD_HEADER) + sizeof(S_P3_TEST_COND_PATTERN);
		data_size = sizeof(S_TEST_COND_PATTERN_DATA);
		for(i=0; i < cmd.testCondPattern.length; i++) {
			memcpy((char *)&pattern, rcv_cmd + point + data_size * i,
				data_size);
			fprintf(fp, "%f, %f, %f, %d\n", (float)pattern.t_val/100.0,
				(float)pattern.cmd_val[0]/1000000.0,
				(float)pattern.cmd_val[1]/1000000.0,
				(int)pattern.waveform_type);
		}
	} else { //0x0A
		fprintf(fp, "t1, p\n");

		point = sizeof(S_P3_CMD_HEADER) + sizeof(S_P3_TEST_COND_PATTERN);
		data_size = sizeof(S_TEST_COND_PATTERN_DATA);
		for(i=0; i < cmd.testCondPattern.length; i++) {
			memcpy((char *)&pattern, rcv_cmd + point + data_size * i,
				data_size);
			fprintf(fp, "%f, %f, %f, %d\n", (float)pattern.t_val/100.0,
				(float)pattern.cmd_val[0]/1000.0,
				(float)pattern.cmd_val[1]/1000.0,
				(int)pattern.waveform_type);
		}
	}

	fprintf(fp, "EOF\n");

	fclose(fp);
#endif
	return 0;
}

int checksum_file(char *psName1, int ch, int size, int rcv_checksum, char *point)
{ //kjg_130430
	int checksum=0;
	unsigned char tmp;
	int i, j, debug;

	if(ch == 0) {
		debug = 0;
		//debug = 1;
	} else {
		debug = 0;
	}

	checksum = j = 0;

	if(debug != 0) {
		userlog(DEBUG_LOG, psName1, "checksum log");
	}

	for(i=0; i < size; i++) {
		tmp = *(point + i);
		checksum += (int)tmp;
		j += (int)tmp;

		if(debug != 0) {
			userlog2(DEBUG_LOG, psName1, " %d", tmp);

			if((i % 10) == 9) {
				userlog2(DEBUG_LOG, psName1, " : %d\n", j);
				j = 0;
				userlog(DEBUG_LOG, psName1, "checksum log");
			}
		}
	}

	if(debug != 0) {
		userlog2(DEBUG_LOG, psName1, " : %d\n", j);

		userlog2(DEBUG_LOG, psName1, "checksum end sbc:%d, gui:%d\n",
			checksum, rcv_checksum);
	}
	return checksum;
}

int read_test_cond_pattern_file_coa(char *psName1, int ch, int pattern_index, unsigned char flag)
{ //kjg_130430	//shh_220607t
	char cmd[256], *in_delimiter = " ,\t\n\r", *token;
	char test_cond[1024 * 1024 * 4], temp[256], buf[256];
	int fp1, rtn, tmp, i, mode, j, k, pattern_file_size, pattern_file_checksum;
	long div;
	FILE *fp2;

	i = IDX_LOC_OBJ_PATTERN_FILE_SIZE;
	pattern_file_size = myData->testCond[ch].local_object[pattern_index][i];
	i = IDX_LOC_OBJ_PATTERN_FILE_CHECKSUM;
	pattern_file_checksum = myData->testCond[ch].local_object[pattern_index][i];

	//file check : kjg_w

	i = IDX_LOC_OBJ_PATTERN_UPDATED; //kjg_170810
	div = myData->testCond[ch].local_object[pattern_index][i];

	memset(cmd, 0, sizeof cmd);
	if(div == 0) { //first
		sprintf(cmd, "/root/START_INFO/CH%03d/sbc_pattern_data_step_%05d.csv",
			ch+1, pattern_index+1);
		// /root/START_INFO/CH00#/sbc_pattern_data_step_#####.csv
	} else { //1 update kjg_170810
		sprintf(cmd,
			"/root/START_INFO/CH%03d/UPDATE/sbc_pattern_data_step_%05d.csv",
			ch+1, pattern_index+1);
		// /root/START_INFO/CH00#/UPDATE/sbc_pattern_data_step_#####.csv
	}

	if((fp1 = open(cmd, O_RDONLY)) < 0) {
		userlog(DEBUG_LOG, psName1, "Can not open1 %s file(load)\n", cmd);
		return -1;
	}

	i = sizeof(char) * 1024 * 1024 * 4;
	memset((char *)&test_cond, 0, i);

	rtn = read(fp1, (char *)&test_cond, i);
	if(rtn != pattern_file_size) {
		userlog(DEBUG_LOG, psName1, "error %s size sbc:%d, gui:%d\n", cmd, rtn,
			pattern_file_size);
		rtn = -2;
	} else rtn = 0;
	close(fp1);
	if(rtn < 0) return rtn;

	rtn = checksum_file(psName1, ch, pattern_file_size, pattern_file_checksum,
		(char *)&test_cond);
	if(pattern_file_checksum != rtn) {
		userlog(DEBUG_LOG, psName1, "error %s checksum sbc:%d, gui:%d\n",
			cmd, rtn, pattern_file_checksum);
		return -3;
	}

	if((fp2 = fopen(cmd, "r")) == NULL) {
		userlog(DEBUG_LOG, psName1, "Can not open2 %s file(load)\n", cmd);
		return -4;
	}
	if(flag == 0) {	//shh_220607t
		fclose(fp2);
		if(div == 0) { //first
			userlog(DEBUG_LOG, psName1,
				"read pattern_file completed ch:%d index:%d size:%d checksum:%d\n",
				ch+1, pattern_index, pattern_file_size, pattern_file_checksum);
		} else { //1 update
			userlog(DEBUG_LOG, psName1, "read pattern_file completed(update) ch:%d index:%d size:%d checksum:%d\n",
				ch+1, pattern_index, pattern_file_size, pattern_file_checksum);
		}
		return 0;
	}				//shh_220607t
	j = k = 0;
KJGGG:
	i = 0;
	//userlog(DEBUG_LOG, psName1, "kjg_d1", cmd);
	while(i == 0) {
		j++;
		if(j >= 100) {
			i = -1;
			break;
		}

		memset(temp, 0, sizeof temp);
		tmp = fscanf(fp2, "%s", temp);

	//	userlog2(DEBUG_LOG, psName1, " %02x:%02x:%02x:%02x",
	//		(unsigned char)temp[0], (unsigned char)temp[1],
	//		(unsigned char)temp[2], (unsigned char)temp[3]);

		if((temp[0] == 'S' && temp[1] == 'T' && temp[2] == 'X')
			&& ((temp[3] == ',') || (temp[3] == 0x00))) {
			i = 1;
			break;
		}
	}
	//userlog2(DEBUG_LOG, psName1, ":end\n");

	if(i <= 0) {
		userlog(DEBUG_LOG, psName1, "Can not open3 %s file(load)\n", cmd);
		fclose(fp2);
		return -5;
	} else {
		//memset(temp, 0, sizeof temp);
		//tmp = fscanf(fp2, "%s", temp);
		//userlog(DEBUG_LOG, psName1, "kjg_d2 %s\n", temp);
		//fclose(fp2);
		//return -6;
	}

	memset((char *)&myData->testCond[ch].pattern[0], 0,
		sizeof(S_TEST_COND_PATTERN_DATA) * MAX_PATTERN_DATA);

	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp2, "%s", temp);
	token = strtok(temp, in_delimiter);
	if(token == NULL) {
	}
	memset(buf, 0, sizeof buf);
	strcpy(buf, token);
	if((strncmp(buf, "t1", 2) == 0) || (strncmp(buf, "T1", 2) == 0)) {
		//accumulation time
		myData->testCond[ch].pattern[0].t_val = 0;
		//kjg_131217 return -8;
	} else if((strncmp(buf, "t2", 2) == 0) || (strncmp(buf, "T2", 2) == 0)) {
		//displacement time
		myData->testCond[ch].pattern[0].t_val = 1;
	//jhkw_161026s
	} else if((strncmp(buf, "Temp", 4) == 0) || (strncmp(buf, "TEMP", 4) == 0)) {
		//temperature
		myData->testCond[ch].pattern[0].t_val = 2;
	//jhkw_161026e
	} else {
		k++;
		if(k == 1) {
			goto KJGGG;
		} else if(k == 2) {
			userlog(DEBUG_LOG, psName1, "Can not open4 %s file(load)\n", cmd);
			fclose(fp2);
			return -7;
		}
	}
	//userlog(DEBUG_LOG, psName1, "kjg_d 1:%s\n", buf);

	mode = MODE_IDLE;
	//memset(temp, 0, sizeof temp);
	//tmp = fscanf(fp2, "%s", temp);
	//token = strtok(temp, in_delimiter);
	token = strtok(NULL, in_delimiter);
	if(token == NULL) {
	}
	memset(buf, 0, sizeof buf);
	strcpy(buf, token);
	if((strncmp(buf, "i", 1) == 0) || (strncmp(buf, "I", 1) == 0)) {
		mode = MODE_CC;
		myData->testCond[ch].pattern[0].cmd_val[0] = 0;
		myData->testCond[ch].pattern[0].cmd_val[1]
			= myData->testCond[ch].pattern[0].cmd_val[0];
	} else if((strncmp(buf, "p", 1) == 0) || (strncmp(buf, "P", 1) == 0)) {
		mode = MODE_CP;
		myData->testCond[ch].pattern[0].cmd_val[0] = 1;
		myData->testCond[ch].pattern[0].cmd_val[1]
			= myData->testCond[ch].pattern[0].cmd_val[0];
	//kjhw_150210s
	} else if((strncmp(buf, "v", 1) == 0) || (strncmp(buf, "V", 1) == 0)) {
		mode = MODE_CV;
		myData->testCond[ch].pattern[0].cmd_val[0] = 2;
		myData->testCond[ch].pattern[0].cmd_val[1]
			= myData->testCond[ch].pattern[0].cmd_val[0];
	//kjhw_150210s
	} else {
	}
	//userlog(DEBUG_LOG, psName1, "kjg_d 2:%s\n", buf);

	if(mode == MODE_CV) { //kjhw_150914s
		for(i=1; i < MAX_P1_PATTERN_DATA; i++) {
			memset(temp, 0, sizeof temp);
			tmp = fscanf(fp2, "%s", temp);
			token = strtok(temp, in_delimiter);
			if(token == NULL) {
				break;
			}
			memset(buf, 0, sizeof buf);
			strcpy(buf, token);
			if((strncmp(buf, "EOF", 3) == 0) || (strncmp(buf, "eof", 3) == 0)) {
			} else {
				myData->testCond[ch].pattern[i].t_val
					= string_to_long(buf, mode, 0);
			}
			//userlog(DEBUG_LOG, psName1, "kjg_d 3:%s, %ld\n", buf,
			//	myData->testCond[ch].pattern[i].t_val);

			//memset(temp, 0, sizeof temp);
			//tmp = fscanf(fp2, "%s", temp);
			//token = strtok(temp, in_delimiter);
			token = strtok(NULL, in_delimiter);
			if(token == NULL) {
				break;
			}
			memset(buf, 0, sizeof buf);
			strcpy(buf, token);
			if((strncmp(buf, "EOF", 3) == 0) || (strncmp(buf, "eof", 3) == 0)) {
			} else { //Vref
				myData->testCond[ch].pattern[i].cmd_val[0]
					= string_to_long(buf, mode, 1);
			}
			//userlog(DEBUG_LOG, psName1, "kjg_d 4:%s, %ld\n", buf,
			//	myData->testCond[ch].pattern[i].cmd_val[0]);
			token = strtok(NULL, in_delimiter);
			if(token == NULL) {
				break;
			}
			memset(buf, 0, sizeof buf);
			strcpy(buf, token);
			if((strncmp(buf, "EOF", 3) == 0) || (strncmp(buf, "eof", 3) == 0)) {
			} else { //+Iref
				myData->testCond[ch].pattern[i].cmd_val[1]
					= string_to_long(buf, mode, 1);
			}

			token = strtok(NULL, in_delimiter);
			if(token == NULL) {
				break;
			}
			memset(buf, 0, sizeof buf);
			strcpy(buf, token);
			if((strncmp(buf, "EOF", 3) == 0) || (strncmp(buf, "eof", 3) == 0)) {
			} else { //-Iref
				myData->testCond[ch].pattern[i].cmd_val[2]
					= string_to_long(buf, mode, 1);
			}

			myData->testCond[ch].pattern[i].waveform_type = 0;
				//0:rectangle, 1:triangle
		} //kjhw_150914e
	} else { //CC, CP
		for(i=1; i < MAX_P1_PATTERN_DATA; i++) {
			memset(temp, 0, sizeof temp);
			tmp = fscanf(fp2, "%s", temp);
			token = strtok(temp, in_delimiter);
			if(token == NULL) {
				break;
			}
			memset(buf, 0, sizeof buf);
			strcpy(buf, token);
			if((strncmp(buf, "EOF", 3) == 0) || (strncmp(buf, "eof", 3) == 0)) {
			} else {
				//jhkw_161026s
				if(myData->testCond[ch].pattern[0].t_val == 2) { //temperature
					myData->testCond[ch].pattern[i].t_val
						= string_to_long(buf, mode, 4);
					if((i > 1) && (myData->testCond[ch].pattern[i].t_val
						<= myData->testCond[ch].pattern[i-1].t_val)) {
						userlog(DEBUG_LOG, psName1, "temp error:%ld, %ld\n",
							myData->testCond[ch].pattern[i].t_val,
							myData->testCond[ch].pattern[i-1].t_val);
						return -8;
					}
				} else {
					myData->testCond[ch].pattern[i].t_val
						= string_to_long(buf, mode, 0);
				}
				//myData->testCond[ch].pattern[i].t_val
				//	= string_to_long(buf, mode, 0);
				//jhkw_161026e
			}
			//userlog(DEBUG_LOG, psName1, "kjg_d 3:%s, %ld\n", buf,
			//	myData->testCond[ch].pattern[i].t_val);

			//memset(temp, 0, sizeof temp);
			//tmp = fscanf(fp2, "%s", temp);
			//token = strtok(temp, in_delimiter);
			token = strtok(NULL, in_delimiter);
			if(token == NULL) {
				break;
			}
			memset(buf, 0, sizeof buf);
			strcpy(buf, token);
			if((strncmp(buf, "EOF", 3) == 0) || (strncmp(buf, "eof", 3) == 0)) {
			} else {
				myData->testCond[ch].pattern[i].cmd_val[0]
					= string_to_long(buf, mode, 1);
				myData->testCond[ch].pattern[i].cmd_val[1]
					= myData->testCond[ch].pattern[i].cmd_val[0];
			}
			//userlog(DEBUG_LOG, psName1, "kjg_d 4:%s, %ld\n", buf,
			//	myData->testCond[ch].pattern[i].cmd_val[0]);

			myData->testCond[ch].pattern[i].waveform_type = 0;
				//0:rectangle, 1:triangle
		}
	}

	//jhkw_161026s
	if(myData->testCond[ch].pattern[0].t_val == 2) { //temperature
	} else {
		myData->testCond[ch].pattern[i].t_val = (-1);
	}
	//myData->testCond[ch].pattern[i].t_val = (-1);
	//jhkw_161026e

	//userlog(DEBUG_LOG, psName1, "kjg_d 5:%d, %ld\n", i-1,
	//	myData->testCond[ch].pattern[i-1].t_val);
	//userlog(DEBUG_LOG, psName1, "kjg_d 6:%d, %ld\n", i,
	//	myData->testCond[ch].pattern[i].t_val);

	fclose(fp2);

	if(div == 0) { //first
		userlog(DEBUG_LOG, psName1,
			"read pattern_file completed ch:%d index:%d size:%d checksum:%d\n",
			ch+1, pattern_index, pattern_file_size, pattern_file_checksum);
	} else { //1 update
		userlog(DEBUG_LOG, psName1, "read pattern_file completed(update) ch:%d index:%d size:%d checksum:%d\n",
			ch+1, pattern_index, pattern_file_size, pattern_file_checksum);
	}
	return 0;
}

int read_test_cond_usermap_file_coa(char *psName1, int ch, int usermap_index)
{ //kjhw_140828
	char cmd[256], *in_delimiter = " ,\t\n\r", *token;
	char test_cond[1024 * 1024 * 4], temp[256], buf[256];
	int fp1, rtn, tmp, i, mode, j, k, usermap_file_size, usermap_file_checksum;
	int mmm;
	long div;
	FILE *fp2;

	i = IDX_LOC_OBJ_PATTERN_FILE_SIZE;
	usermap_file_size = myData->testCond[ch].local_object[usermap_index][i];
	i = IDX_LOC_OBJ_PATTERN_FILE_CHECKSUM;
	usermap_file_checksum = myData->testCond[ch].local_object[usermap_index][i];

	//file check : kjg_w

	i = IDX_LOC_OBJ_USERMAP_UPDATED; //kjg_170810
	div = myData->testCond[ch].local_object[usermap_index][i];

	memset(cmd, 0, sizeof cmd);
	if(div == 0) { //first
		sprintf(cmd, "/root/START_INFO/CH%03d/sbc_usermap_data_step_%05d.csv",
			ch+1, usermap_index+1);
		// /root/START_INFO/CH00#/sbc_usermap_data_step_#####.csv
	} else { //1 update kjg_170810
		sprintf(cmd,
			"/root/START_INFO/CH%03d/UPDATE/sbc_usermap_data_step_%05d.csv",
			ch+1, usermap_index+1);
		// /root/START_INFO/CH00#/UPDATE/sbc_usermap_data_step_#####.csv
	}

	if((fp1 = open(cmd, O_RDONLY)) < 0) {
		userlog(DEBUG_LOG, psName1, "Can not open1 %s file(load)\n", cmd);
		return -1;
	}

	i = sizeof(char) * 1024 * 1024 * 4;
	memset((char *)&test_cond, 0, i);

	rtn = read(fp1, (char *)&test_cond, i);
	if(rtn != usermap_file_size) {
		userlog(DEBUG_LOG, psName1, "error %s size sbc:%d, gui:%d\n", cmd, rtn,
			usermap_file_size);
		rtn = -2;
	} else rtn = 0;
	close(fp1);
	if(rtn < 0) return rtn;

	rtn = checksum_file(psName1, ch, usermap_file_size, usermap_file_checksum,
		(char *)&test_cond);
	if(usermap_file_checksum != rtn) {
		userlog(DEBUG_LOG, psName1, "error %s checksum sbc:%d, gui:%d\n",
			cmd, rtn, usermap_file_checksum);
		return -3;
	}

	if((fp2 = fopen(cmd, "r")) == NULL) {
		userlog(DEBUG_LOG, psName1, "Can not open2 %s file(load)\n", cmd);
		return -4;
	}

	j = k = 0;
KJGGG:
	i = 0;
	//userlog(DEBUG_LOG, psName1, "kjg_d1", cmd);
	while(i == 0) {
		j++;
		if(j >= 100) {
			i = -1;
			break;
		}

		memset(temp, 0, sizeof temp);
		tmp = fscanf(fp2, "%s", temp);

	//	userlog2(DEBUG_LOG, psName1, " %02x:%02x:%02x:%02x",
	//		(unsigned char)temp[0], (unsigned char)temp[1],
	//		(unsigned char)temp[2], (unsigned char)temp[3]);

		if((temp[0] == 'S' && temp[1] == 'T' && temp[2] == 'X')
			&& ((temp[3] == ',') || (temp[3] == 0x00))) {
			i = 1;
			break;
		}
	}
	//userlog2(DEBUG_LOG, psName1, ":end\n");

	if(i <= 0) {
		userlog(DEBUG_LOG, psName1, "Can not open3 %s file(load)\n", cmd);
	fclose(fp2);
		return -5;
	} else {
		//memset(temp, 0, sizeof temp);
		//tmp = fscanf(fp2, "%s", temp);
		//userlog(DEBUG_LOG, psName1, "kjg_d2 %s\n", temp);
	//fclose(fp2);
		//return -6;
	}

	memset((char *)&myData->testCond[ch].usermap, 0,
		sizeof(S_TEST_COND_USERMAP_DATA));

	mode = MODE_IDLE;

	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp2, "%s", temp);
	token = strtok(temp, in_delimiter);
	if(token == NULL) {
	}
	memset(buf, 0, sizeof buf);
	strcpy(buf, token);

	if((strncmp(buf, "TIME", 4) == 0) || (strncmp(buf, "time", 4) == 0)) {
		token = strtok(NULL, in_delimiter);
		if(token == NULL) {
		}
		memset(buf, 0, sizeof buf);
		strcpy(buf, token);

		myData->testCond[ch].usermap.t_val[1] = string_to_long(buf, mode, 0);
	} else {
		k++;
		if(k == 1) {
			goto KJGGG;
		} else if(k == 2) {
			userlog(DEBUG_LOG, psName1, "Can not open4 %s file(load)\n", cmd);
			fclose(fp2);
			return -6;
		}
	}

	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp2, "%s", temp);
	token = strtok(temp, in_delimiter);
	if(token == NULL) {
	}
	memset(buf, 0, sizeof buf);
	strcpy(buf, token);

	if((strncmp(buf, "t1", 2) == 0) || (strncmp(buf, "T1", 2) == 0)) {
		//accumulation time
		myData->testCond[ch].usermap.t_val[0] = 0;
		//kjg_131217 return -8;
	} else if((strncmp(buf, "t2", 2) == 0) || (strncmp(buf, "T2", 2) == 0)) {
		//displacement time
		myData->testCond[ch].usermap.t_val[0] = 1;
	} else {
		k++;
		if(k == 1) {
			goto KJGGG;
		} else if(k == 2) {
			userlog(DEBUG_LOG, psName1, "Can not open5 %s file(load)\n", cmd);
			fclose(fp2);
			return -7;
		}
	}
	//userlog(DEBUG_LOG, psName1, "kjg_d 1:%s\n", buf);

	//mode = MODE_IDLE;
	//memset(temp, 0, sizeof temp);
	//tmp = fscanf(fp2, "%s", temp);
	//token = strtok(temp, in_delimiter);
	token = strtok(NULL, in_delimiter);
	if(token == NULL) {
	}
	memset(buf, 0, sizeof buf);
	strcpy(buf, token);
	if((strncmp(buf, "i", 1) == 0) || (strncmp(buf, "I", 1) == 0)) {
		mode = MODE_CC;
		myData->testCond[ch].usermap.cmd_val[0] = 0;
		//myData->testCond[ch].usermap.cmd_val[1]
		//	= myData->testCond[ch].usermap.cmd_val[0];
	} else if((strncmp(buf, "p", 1) == 0) || (strncmp(buf, "P", 1) == 0)) {
		mode = MODE_CP;
		myData->testCond[ch].usermap.cmd_val[0] = 1;
		//myData->testCond[ch].usermap.cmd_val[1]
		//	= myData->testCond[ch].usermap.cmd_val[0];
	} else {
	}

	mmm = 0;
	for(i=0; i < MAX_P1_USERMAP_ROW_DATA; i++) {
		memset(temp, 0, sizeof temp);
		tmp = fscanf(fp2, "%s", temp);
		token = strtok(temp, in_delimiter);
		if(token == NULL) {
			break;
		}
		memset(buf, 0, sizeof buf);
		strcpy(buf, token);
		if((strncmp(buf, "EOF", 3) == 0) || (strncmp(buf, "eof", 3) == 0)) {
		} else {
			for(mmm=0; mmm < MAX_P1_USERMAP_COL_DATA; mmm++) {
				if(i == 0 && mmm == 0) continue;
				if(i != 0 && mmm == 0) {
				} else {
					token = strtok(NULL, in_delimiter);
					if(token == NULL) {
						break;
					}
					memset(buf, 0, sizeof buf);
					strcpy(buf, token);
				}
				if(i == 0 || mmm == 0) {
					myData->testCond[ch].usermap.table[i][mmm]
						= string_to_long(buf, mode, 3);
				} else {
					myData->testCond[ch].usermap.table[i][mmm]
						= string_to_long(buf, mode, 2);
				}
			}
		}
	}

	//userlog(DEBUG_LOG, psName1, "kjhd COL_DATA:%d, %ld\n", mmm-1,
	//	myData->testCond[ch].usermap.table[i-1][mmm-1]);
	//userlog(DEBUG_LOG, psName1, "kjhd ROW_DATA:%d, %ld\n", i-1,
	//	myData->testCond[ch].usermap.table[i-1][mmm-1]);

	fclose(fp2);

	if(div == 0) { //first
		userlog(DEBUG_LOG, psName1,
			"read usermap_file completed ch:%d index:%d size:%d checksum:%d\n",
			ch+1, usermap_index, usermap_file_size, usermap_file_checksum);
	} else { //1 update
		userlog(DEBUG_LOG, psName1, "read usermap_file completed(update) ch:%d index:%d size:%d checksum:%d\n",
			ch+1, usermap_index, usermap_file_size, usermap_file_checksum);
	}
	return 0;
}

#endif

