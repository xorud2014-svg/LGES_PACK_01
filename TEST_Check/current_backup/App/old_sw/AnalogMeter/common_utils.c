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
/* kjgw
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

	jig = (int)myData->mData.config.groupInJig[group];
	if(jig <= 0) jig = -1;
	else if(jig > MAX_JIG_PER_MODULE) jig = MAX_JIG_PER_MODULE;

	return jig;
}

int jigNo_to_groupNo(int jig, int index)
{
	int group;

	group = (int)myData->mData.config.jigInGroup[jig][index];
	if(group <= 0) group = -1;
	else if(group > MAX_GROUP_PER_MODULE) group = MAX_GROUP_PER_MODULE;

	return group;
}

void make_com_chCode_cond(int ch, int code, int count, char type[4], int idx1)
{
	char compType[4];
	
	code -= C_CD_COM_START;
	myData->testCond[ch].common_chCode[code].compCount = (unsigned char)count;

	memset(compType, 0, sizeof compType);
	strcpy(compType, type);
	if(strcmp(compType, "N") == 0) {
		myData->testCond[ch].common_chCode[code].compType = COMP_NONE;
	} else if(strcmp(compType, "<") == 0) {
		myData->testCond[ch].common_chCode[code].compType = COMP_LESS_THAN;
	} else if(strcmp(compType, "<=") == 0) {
		myData->testCond[ch].common_chCode[code].compType
			= COMP_LESS_THAN_OR_EQUAL_TO;
	} else if(strcmp(compType, ">") == 0) {
		myData->testCond[ch].common_chCode[code].compType = COMP_GREATER_THAN;
	} else if(strcmp(compType, ">=") == 0) {
		myData->testCond[ch].common_chCode[code].compType
			= COMP_GREATER_THAN_OR_EQUAL_TO;
	} else if(strcmp(compType, "=") == 0) {
		myData->testCond[ch].common_chCode[code].compType = COMP_EQUAL_TO;
	} else if(strcmp(compType, "==") == 0) {
		myData->testCond[ch].common_chCode[code].compType = COMP_EQUAL_TO;
	} else if(strcmp(compType, "!=") == 0) {
		myData->testCond[ch].common_chCode[code].compType = COMP_NOT_EQUAL_TO;
	}

	myData->testCond[ch].common_chCode[code].compIndex1 = (unsigned char)idx1;
}

void make_loc_chCode_cond(int ch, int step2, int code, int count, char type[4], int idx1)
{
	char compType[4];
	
	code -= C_CD_END_START;
	myData->testCond[ch].local_chCode[step2][code].compCount
		= (unsigned char)count;

	memset(compType, 0, sizeof compType);
	strcpy(compType, type);
	if(strcmp(compType, "N") == 0) {
		myData->testCond[ch].local_chCode[step2][code].compType = COMP_NONE;
	} else if(strcmp(compType, "<") == 0) {
		myData->testCond[ch].local_chCode[step2][code].compType
			= COMP_LESS_THAN;
	} else if(strcmp(compType, "<=") == 0) {
		myData->testCond[ch].local_chCode[step2][code].compType
			= COMP_LESS_THAN_OR_EQUAL_TO;
	} else if(strcmp(compType, ">") == 0) {
		myData->testCond[ch].local_chCode[step2][code].compType
			= COMP_GREATER_THAN;
	} else if(strcmp(compType, ">=") == 0) {
		myData->testCond[ch].local_chCode[step2][code].compType
			= COMP_GREATER_THAN_OR_EQUAL_TO;
	} else if(strcmp(compType, "=") == 0) {
		myData->testCond[ch].local_chCode[step2][code].compType = COMP_EQUAL_TO;
	} else if(strcmp(compType, "==") == 0) {
		myData->testCond[ch].local_chCode[step2][code].compType = COMP_EQUAL_TO;
	} else if(strcmp(compType, "!=") == 0) {
		myData->testCond[ch].local_chCode[step2][code].compType
			= COMP_NOT_EQUAL_TO;
	}

	myData->testCond[ch].local_chCode[step2][code].compIndex1
		= (unsigned char)idx1;
}

void make_loc_chCode_cond2(int ch, int step2, int code, int count, char type[4], int idx1, int idx2)
{
	char compType[4];
	
	myData->testCond[ch].local_chCode[step2][code].compCount
		= (unsigned char)count;

	memset(compType, 0, sizeof compType);
	strcpy(compType, type);
	if(strcmp(compType, "N") == 0) {
		myData->testCond[ch].local_chCode[step2][code].compType = COMP_NONE;
	} else if(strcmp(compType, "<") == 0) {
		myData->testCond[ch].local_chCode[step2][code].compType
			= COMP_LESS_THAN;
	} else if(strcmp(compType, "<=") == 0) {
		myData->testCond[ch].local_chCode[step2][code].compType
			= COMP_LESS_THAN_OR_EQUAL_TO;
	} else if(strcmp(compType, ">") == 0) {
		myData->testCond[ch].local_chCode[step2][code].compType
			= COMP_GREATER_THAN;
	} else if(strcmp(compType, ">=") == 0) {
		myData->testCond[ch].local_chCode[step2][code].compType
			= COMP_GREATER_THAN_OR_EQUAL_TO;
	} else if(strcmp(compType, "=") == 0) {
		myData->testCond[ch].local_chCode[step2][code].compType = COMP_EQUAL_TO;
	} else if(strcmp(compType, "==") == 0) {
		myData->testCond[ch].local_chCode[step2][code].compType = COMP_EQUAL_TO;
	} else if(strcmp(compType, "!=") == 0) {
		myData->testCond[ch].local_chCode[step2][code].compType
			= COMP_NOT_EQUAL_TO;
	}

	myData->testCond[ch].local_chCode[step2][code].compIndex1
		= (unsigned char)idx1;
	myData->testCond[ch].local_chCode[step2][code].compIndex2
		= (unsigned char)idx2;
}

void make_aux_chCode_cond(int ch, int code, int count, char type[4])
{
	char compType[4];
	
	code -= C_CD_AUX_START;
	myData->testCond[ch].aux_chCode[code].compCount = (unsigned char)count;

	memset(compType, 0, sizeof compType);
	strcpy(compType, type);
	if(strcmp(compType, "N") == 0) {
		myData->testCond[ch].aux_chCode[code].compType = COMP_NONE;
	} else if(strcmp(compType, "<") == 0) {
		myData->testCond[ch].aux_chCode[code].compType = COMP_LESS_THAN;
	} else if(strcmp(compType, "<=") == 0) {
		myData->testCond[ch].aux_chCode[code].compType
			= COMP_LESS_THAN_OR_EQUAL_TO;
	} else if(strcmp(compType, ">") == 0) {
		myData->testCond[ch].aux_chCode[code].compType = COMP_GREATER_THAN;
	} else if(strcmp(compType, ">=") == 0) {
		myData->testCond[ch].aux_chCode[code].compType
			= COMP_GREATER_THAN_OR_EQUAL_TO;
	} else if(strcmp(compType, "=") == 0) {
		myData->testCond[ch].aux_chCode[code].compType = COMP_EQUAL_TO;
	} else if(strcmp(compType, "==") == 0) {
		myData->testCond[ch].aux_chCode[code].compType = COMP_EQUAL_TO;
	} else if(strcmp(compType, "!=") == 0) {
		myData->testCond[ch].aux_chCode[code].compType = COMP_NOT_EQUAL_TO;
	}
}

void make_can_chCode_cond(int ch, int code, int count, char type[4])
{
	char compType[4];
	
	code -= C_CD_CAN_START;
	myData->testCond[ch].can_chCode[code].compCount = (unsigned char)count;

	memset(compType, 0, sizeof compType);
	strcpy(compType, type);
	if(strcmp(compType, "N") == 0) {
		myData->testCond[ch].can_chCode[code].compType = COMP_NONE;
	} else if(strcmp(compType, "<") == 0) {
		myData->testCond[ch].can_chCode[code].compType = COMP_LESS_THAN;
	} else if(strcmp(compType, "<=") == 0) {
		myData->testCond[ch].can_chCode[code].compType
			= COMP_LESS_THAN_OR_EQUAL_TO;
	} else if(strcmp(compType, ">") == 0) {
		myData->testCond[ch].can_chCode[code].compType = COMP_GREATER_THAN;
	} else if(strcmp(compType, ">=") == 0) {
		myData->testCond[ch].can_chCode[code].compType
			= COMP_GREATER_THAN_OR_EQUAL_TO;
	} else if(strcmp(compType, "=") == 0) {
		myData->testCond[ch].can_chCode[code].compType = COMP_EQUAL_TO;
	} else if(strcmp(compType, "==") == 0) {
		myData->testCond[ch].can_chCode[code].compType = COMP_EQUAL_TO;
	} else if(strcmp(compType, "!=") == 0) {
		myData->testCond[ch].can_chCode[code].compType = COMP_NOT_EQUAL_TO;
	}
}

void make2_com_chCode_cond(int ch, int code, int count, int type, int idx1)
{
	code -= C_CD_COM_START;
	myData->testCond[ch].common_chCode[code].compCount = (unsigned char)count;
	myData->testCond[ch].common_chCode[code].compType = (unsigned char)type;
	myData->testCond[ch].common_chCode[code].compIndex1 = (unsigned char)idx1;
}

void make2_loc_chCode_cond(int ch, int step2, int code, int count, int type, int idx1)
{
	code -= C_CD_END_START;
	myData->testCond[ch].local_chCode[step2][code].compCount
		= (unsigned char)count;
	myData->testCond[ch].local_chCode[step2][code].compType
		= (unsigned char)type;
	myData->testCond[ch].local_chCode[step2][code].compIndex1
		= (unsigned char)idx1;
}

void make2_loc_chCode_cond2(int ch, int step2, int code, int count, int type, int idx1, int idx2)
{
	code -= C_CD_END_START;
	myData->testCond[ch].local_chCode[step2][code].compCount
		= (unsigned char)count;
	myData->testCond[ch].local_chCode[step2][code].compType
		= (unsigned char)type;
	myData->testCond[ch].local_chCode[step2][code].compIndex1
		= (unsigned char)idx1;
	myData->testCond[ch].local_chCode[step2][code].compIndex2
		= (unsigned char)idx2;
}

void make2_aux_chCode_cond(int ch, int code, int count, int type)
{
	code -= C_CD_AUX_START;
	myData->testCond[ch].aux_chCode[code].compCount = (unsigned char)count;
	myData->testCond[ch].aux_chCode[code].compType = (unsigned char)type;
}

void make2_can_chCode_cond(int ch, int code, int count, int type)
{
	code -= C_CD_CAN_START;
	myData->testCond[ch].can_chCode[code].compCount = (unsigned char)count;
	myData->testCond[ch].can_chCode[code].compType = (unsigned char)type;
}

long convert_group_state(int select, long state)
{
	long rtn_state=0;

	if(select == CONVERT_P1_TO_ORG) {
	} else if(select == CONVERT_ORG_TO_P1) {
	} else if(select == CONVERT_P2_TO_ORG) {
	} else if(select == CONVERT_ORG_TO_P2) {
		switch(state) {
			case G_IDLE:	rtn_state = P2_G_IDLE;		break;
			case G_STANDBY:	rtn_state = P2_G_STANDBY;	break;
			case G_RUN:		rtn_state = P2_G_RUN;		break;
			case G_PAUSE:	rtn_state = P2_G_PAUSE;		break;
			case G_CALI:	rtn_state = P2_G_MAINTENANCE;	break;
			default:		rtn_state = P2_G_IDLE;		break;
		}
	} else {
		rtn_state = 0;
	}

	return rtn_state;
}

long convert_step_type(int select, long type)
{
	long rtn_type;
	
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
			default:				rtn_type = P1_STEP_IDLE;		break;
		}
	} else if(select == CONVERT_P2_TO_ORG) {
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
	} else if(select == CONVERT_ORG_TO_P2) {
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
	} else {
		rtn_type = 0;
	}

	return rtn_type;
}

long convert_step_mode(int select, long mode)
{
	long rtn_mode;
	
	if(select == CONVERT_P1_TO_ORG) {
		switch(mode) {
			case P1_MODE_IDLE:	rtn_mode = MODE_IDLE;		break;
			case P1_MODE_CCCV:	rtn_mode = MODE_CCCV;		break;
			case P1_MODE_CC:	rtn_mode = MODE_CC;			break;
			case P1_MODE_CV:	rtn_mode = MODE_CV;			break;
			case P1_MODE_DC:	rtn_mode = MODE_DC;			break;
			case P1_MODE_AC:	rtn_mode = MODE_AC;			break;
			case P1_MODE_CP:	rtn_mode = MODE_CP;			break;
			case P1_MODE_CCP:	rtn_mode = MODE_CCP;		break;
			case P1_MODE_CR:	rtn_mode = MODE_CR;			break;
			default:			rtn_mode = MODE_IDLE;		break;
		}
	} else if(select == CONVERT_ORG_TO_P1) {
		switch(mode) {
			case MODE_IDLE:		rtn_mode = P1_MODE_IDLE;	break;
			case MODE_CCCV:		rtn_mode = P1_MODE_CCCV;	break;
			case MODE_CC:		rtn_mode = P1_MODE_CC;		break;
			case MODE_CV:		rtn_mode = P1_MODE_CV;		break;
			case MODE_DC:		rtn_mode = P1_MODE_DC;		break;
			case MODE_AC:		rtn_mode = P1_MODE_AC;		break;
			case MODE_CP:		rtn_mode = P1_MODE_CP;		break;
			case MODE_CCP:		rtn_mode = P1_MODE_CCP;		break;
			case MODE_CR:		rtn_mode = P1_MODE_CR;		break;
			default:			rtn_mode = P1_MODE_IDLE;	break;
		}
	} else if(select == CONVERT_P2_TO_ORG) {
		switch(mode) {
			case P2_MODE_IDLE:	rtn_mode = MODE_IDLE;		break;
			case P2_MODE_CCCV:	rtn_mode = MODE_CCCV;		break;
			case P2_MODE_CC:	rtn_mode = MODE_CC;			break;
			case P2_MODE_CV:	rtn_mode = MODE_CV;			break;
			case P2_MODE_DC:	rtn_mode = MODE_DC;			break;
			case P2_MODE_AC:	rtn_mode = MODE_AC;			break;
			case P2_MODE_CP:	rtn_mode = MODE_CP;			break;
			case P2_MODE_CCP:	rtn_mode = MODE_CCP;		break;
			case P2_MODE_CR:	rtn_mode = MODE_CR;			break;
			default:			rtn_mode = MODE_IDLE;		break;
		}
	} else if(select == CONVERT_ORG_TO_P2) {
		switch(mode) {
			case MODE_IDLE:		rtn_mode = P2_MODE_IDLE;	break;
			case MODE_CCCV:		rtn_mode = P2_MODE_CCCV;	break;
			case MODE_CC:		rtn_mode = P2_MODE_CC;		break;
			case MODE_CV:		rtn_mode = P2_MODE_CV;		break;
			case MODE_DC:		rtn_mode = P2_MODE_DC;		break;
			case MODE_AC:		rtn_mode = P2_MODE_AC;		break;
			case MODE_CP:		rtn_mode = P2_MODE_CP;		break;
			case MODE_CCP:		rtn_mode = P2_MODE_CCP;		break;
			case MODE_CR:		rtn_mode = P2_MODE_CR;		break;
			default:			rtn_mode = P2_MODE_IDLE;	break;
		}
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
	} else {
		rtn_state = 0;
	}

	return rtn_state;
}

long convert_ch_state_org_to_p2(long state, long type, long attribute, long phase)
{
	long rtn_state=0;

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
	} else {
		rtn_code = 0;
	}

	return rtn_code;
}

long convert_ch_code_org_to_p1(long code)
{
	long rtn_code;

	switch(code) {
		case C_CD_NONE:
			rtn_code = P1_C_CD_NONE;						break;

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
		case C_CD_COM_FAULT_UPPER_TEMP:
			rtn_code = P1_C_CD_FAULT_UPPER_TEMP;			break;
		case C_CD_COM_FAULT_LOWER_TEMP:
			rtn_code = P1_C_CD_FAULT_LOWER_TEMP;			break;
		case C_CD_COM_FAULT_UPPER_CAPACITANCE:
			rtn_code = P1_C_CD_FAULT_UPPER_CAPACITANCE;		break;
		case C_CD_COM_FAULT_LOWER_CAPACITANCE:
			rtn_code = P1_C_CD_FAULT_LOWER_CAPACITANCE;		break;

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

		//save code //kjgw

		//hard fault code
		case C_CD_FAULT_HW_UPPER_V:
		case C_CD_FAULT_HW_LOWER_V:
			rtn_code = P1_C_CD_FAULT_CH_V_FAIL;				break;
		case C_CD_FAULT_HW_UPPER_I:
		case C_CD_FAULT_HW_LOWER_I:
			rtn_code = P1_C_CD_FAULT_CH_I_FAIL;				break;
		case C_CD_FAULT_HW_OT:
			rtn_code = P1_C_CD_FAULT_OT;					break;
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
		case C_CD_FAULT_STOP_WATT_HOUR:
			rtn_code = P1_C_CD_STOP_WATT_HOUR;				break;
		case C_CD_FAULT_STOP_CMD:
			rtn_code = P1_C_CD_FAULT_STOP_CMD;				break;
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

		//aux code
		case C_CD_AUX_END_UPPER_TEMP:
			rtn_code = P1_C_CD_AUX_END_UPPER_TEMP;			break;
		case C_CD_AUX_END_LOWER_TEMP:
			rtn_code = P1_C_CD_AUX_END_LOWER_TEMP;			break;
		case C_CD_AUX_END_UPPER_V:
			rtn_code = P1_C_CD_AUX_END_UPPER_V;				break;
		case C_CD_AUX_END_LOWER_V:
			rtn_code = P1_C_CD_AUX_END_LOWER_V;				break;
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

		default:
			rtn_code = P1_C_CD_FAULT_WORK_ERROR;			break;
	}

	return rtn_code;
}

long convert_ch_code_org_to_p2(long code)
{
	long rtn_code;

	switch(code) {
		case C_CD_NONE:
			rtn_code = P2_C_CD_NONE;						break;

		//common code
		case C_CD_COM_FAULT_UPPER_V:
		case C_CD_COM_FAULT_HW_UPPER_V:
			rtn_code = P2_C_CD_FAULT_UPPER_V;				break;
		case C_CD_COM_FAULT_LOWER_V:
		case C_CD_COM_FAULT_HW_LOWER_V:
			rtn_code = P2_C_CD_FAULT_LOWER_V;				break;
		case C_CD_COM_FAULT_UPPER_I:
			rtn_code = P2_C_CD_FAULT_UPPER_I;				break;
		case C_CD_COM_FAULT_LOWER_I:
			rtn_code = P2_C_CD_FAULT_LOWER_I;				break;

		//end code
		case C_CD_END_TIME:
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
		case C_CD_END_STOP_CMD:
			rtn_code = P2_C_CD_END_STOP_CMD;				break;
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
		case C_CD_END_DELTA_I: //kjgw
			rtn_code = P2_C_CD_END_DELTA_V;					break;
		case C_CD_END_VALUE_RATE_AMPARE_HOUR:
			rtn_code = P2_C_CD_END_SOC;						break;

/*		case C_CD_END_PAUSE_CMD:
			rtn_code = P2_C_CD_END_PAUSE_CMD;				break;
*/
		//check fault code
		case C_CD_FAULT_CHECK_NG:
			rtn_code = P2_C_CD_FAULT_CHECK_CONTACT_BAD;		break;
		case C_CD_FAULT_CHECK_UPPER_OCV:
			rtn_code = P2_C_CD_FAULT_CHECK_UPPER_OCV;		break;
		case C_CD_FAULT_CHECK_LOWER_OCV:
			rtn_code = P2_C_CD_FAULT_CHECK_LOWER_OCV;		break;
			//rtn_code = P2_C_CD_NONCELL;						break;
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
		case C_CD_FAULT_LOWER_TEMP:
		case C_CD_FAULT_DELTA_TEMP:
			rtn_code = P2_C_CD_FAULT_OT;					break;
		case C_CD_FAULT_UPPER_COMP_I2:
			rtn_code = P2_C_CD_FAULT_UPPER_COMP_I2;			break;
		case C_CD_FAULT_LOWER_COMP_I2:
			rtn_code = P2_C_CD_FAULT_LOWER_COMP_I2;			break;
		case C_CD_FAULT_UPPER_COMP_I3:
			rtn_code = P2_C_CD_FAULT_UPPER_COMP_I3;			break;
		case C_CD_FAULT_LOWER_COMP_I3:
			rtn_code = P2_C_CD_FAULT_LOWER_COMP_I3;			break;

		case C_CD_FAULT_STOP_CMD:
			rtn_code = P2_C_CD_FAULT_STOP_CMD;				break;
		case C_CD_FAULT_PAUSE_CMD:
			rtn_code = P2_C_CD_FAULT_PAUSE_CMD;				break;
		case C_CD_COM_FAULT_UPPER_TEMP:
			rtn_code = P2_C_CD_FAULT_UPPER_TEMP;			break;
		case C_CD_COM_FAULT_LOWER_TEMP:
			rtn_code = P2_C_CD_FAULT_LOWER_TEMP;			break;

//kjgw_f		case C_CD_FAULT_NEXTSTEP_CMD:
//			rtn_code = P2_C_CD_FAULT_NEXTSTEP_CMD;			break;

		case C_CD_FAULT_HW_UPPER_V:
		case C_CD_FAULT_HW_LOWER_V:
			rtn_code = P2_C_CD_FAULT_CH_V_FAIL;				break;
		case C_CD_FAULT_HW_UPPER_I:
		case C_CD_FAULT_HW_LOWER_I:
			rtn_code = P2_C_CD_FAULT_CH_I_FAIL;				break;
		case C_CD_FAULT_HW_OT:
			rtn_code = P2_C_CD_FAULT_OT;					break;
		case C_CD_FAULT_M_CPU_WATCHDOG:
			rtn_code = P2_C_CD_FAULT_UNKNOWN;				break;
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
//kjgw_f		case C_CD_FAULT_B_TEMP_UPPER:
//			rtn_code = P2_C_CD_FAULT_OT;					break;
		case C_CD_FAULT_B_ADC:
			rtn_code = P2_C_CD_FAULT_UNKNOWN;				break;

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
	} else {
		rtn_code = 0;
	}

	return rtn_code;
}

int convert_group_trouble_code_org_to_p1(int code)
{
	int rtn_code;

	switch(code) {
		case M_CD_FAULT_AC_POWER_SHORT:
			rtn_code = P1_G_CD_FAULT_AC_POWER_SHORT;		break;
		case M_CD_FAULT_AC_POWER_LONG:
			rtn_code = P1_G_CD_FAULT_AC_POWER_LONG;			break;
		case M_CD_FAULT_UPS_BATTERY:
			rtn_code = P1_G_CD_FAULT_UPS_BATTERY;			break;
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
		case M_CD_FAULT_UPPER_VOLTAGE:
			rtn_code = P1_G_CD_FAULT_MAIN_UPPER_VOLTAGE;	break;
		case M_CD_FAULT_LOWER_VOLTAGE:
			rtn_code = P1_G_CD_FAULT_MAIN_LOWER_VOLTAGE;	break;
		case M_CD_FAULT_MAIN_PS:
			rtn_code = P1_G_CD_FAULT_MAIN_PS;				break;
		case M_CD_FAULT_FAN:
			rtn_code = P1_G_CD_FAULT_FAN;					break;

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
/*kjgw
		case G_CD_FAULT_JIG_ACTIVE_ERROR:
			rtn_code = P1_FAIL_JIG_ACTIVE_ERROR;			break;
		case G_CD_FAULT_JIG_TRAY_ERROR:
			rtn_code = P1_FAIL_JIG_TRAY_ERROR;				break;
		case G_CD_FAULT_JIG_SMOKE_ERROR:
			rtn_code = P1_FAIL_JIG_SMOKE_ERROR;				break;
		case G_CD_FAULT_JIG_TEMP_ERROR:
			rtn_code = P1_FAIL_JIG_TEMP_ERROR;				break;
		case G_CD_FAULT_JIG_GAS_ERROR:
			rtn_code = P1_FAIL_JIG_GAS_ERROR;				break;
		case G_CD_FAULT_JIG_DOOR_ERROR:
			rtn_code = P1_FAIL_JIG_DOOR_ERROR;				break;
		case G_CD_FAULT_JIG_AIR_PRESS_ERROR:
			rtn_code = P1_FAIL_JIG_AIR_PRESS_ERROR;			break;
		case G_CD_FAULT_JIG_STACKER_ERROR:
			rtn_code = P1_FAIL_JIG_STACKER_ERROR;			break;
*/
		case B_CD_FAULT_OT:
			rtn_code = P1_G_CD_FAULT_BD_OT;					break;
		case B_CD_FAULT_ADC:
			rtn_code = P1_G_CD_FAULT_BD_ADC;				break;

		default:	
			rtn_code = P1_G_CD_FAULT_HOLD;					break;
	}

	return rtn_code;
}

int convert_group_trouble_code_org_to_p2(int code)
{
	int rtn_code;

	switch(code) {
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
		//kjgw BCR_COMM_ERROR
		case M_CD_FAULT_CALIBRATOR_COMM_ERROR:
			rtn_code = P2_FAIL_CALIBRATOR_COMM_ERROR;		break;

		case M_CD_FAULT_FUSE:
		case M_CD_FAULT_UPPER_VOLTAGE:
		case M_CD_FAULT_LOWER_VOLTAGE:
		case M_CD_FAULT_MAIN_PS:
			rtn_code = P2_FAIL_SMPS;						break;
		case M_CD_FAULT_FAN:
			rtn_code = P2_FAIL_FAN;							break;

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
		case M_CD_FAULT_CHAMBER_FIRE:
			rtn_code = P2_FAIL_JIG_SMOKE_ERROR;				break;
		//case M_CD_FAULT_CHAMBER_FIRE:
		//	rtn_code = P2_FAIL_CHAMBER_FIRE;				break;
		case G_CD_FAULT_JIG_TEMP_ERROR:
			rtn_code = P2_FAIL_JIG_TEMP_ERROR;				break;
		case G_CD_FAULT_JIG_GAS_ERROR:
			rtn_code = P2_FAIL_JIG_GAS_ERROR;				break;
		case G_CD_FAULT_JIG_DOOR_ERROR:
		//case M_CD_FAULT_CHAMBER_DOOR_OPEN:
		case M_CD_FAULT_CHAMBER:
			rtn_code = P2_FAIL_JIG_DOOR_ERROR;				break;
		case G_CD_FAULT_JIG_AIR_PRESS_ERROR:
			rtn_code = P2_FAIL_JIG_AIR_PRESS_ERROR;			break;
		case G_CD_FAULT_JIG_STACKER_ERROR:
			rtn_code = P2_FAIL_JIG_STACKER_ERROR;			break;
		default:	
			rtn_code = P2_FAIL_HOLD;						break;
	}

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

	for(i=0; i < MAX_DIO_OUT_FUNCTION; i++) {
		if((int)myData->dio.function_set.out_set[i].group != group) continue;
		if((int)myData->dio.function_set.out_set[i].ch != ch) continue;
		if(myData->dio.function_set.out_set[i].function != function) continue;

		if(myData->dio.function_set.out_set[i].use == USE) 
			myData->dio.out.function[i] = val;
		break;
	}
}

#ifdef __KERNEL__
#else
long string_to_long(char *buf, int mode, int div)
{
	char tmp[16], tmpbuf[40];
	int i, j, k;
	long val, t_hour, t_min, t_sec;

	memset(tmpbuf, 0, sizeof tmpbuf);
	strcpy((char *)&tmpbuf[0], buf);

	if(div == 1) {
		if(mode == P1_MODE_CC
			|| mode == P2_MODE_CC) {
			val = (long)(atof(tmpbuf) * 1000000.0);
		} else if(mode == P1_MODE_CP
			|| mode == P2_MODE_CP) {
			val = (long)(atof(tmpbuf) * 1000.0);
		} else {
			val = 0;
		}

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
		val = (long)(atof(tmpbuf) * 100.0 + 0.1);
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
		t_sec = (long)(atof(tmp) * 100.0);

		if(k == 1) {
			val = (t_hour * 60) * 100;
		} else {
			val = ((t_hour * 3600) + (t_min * 60)) * 100;
		}
		val += t_sec;
	}

	return val;
}

int Read_Pattern_File(char *psName1, int ch, int pattern_index)
{
	char temp[32], buf[32], fileName[128], *in_delimiter = " ,\t\n\r", *token;
    int tmp, i, mode, groupNo;
    FILE *fp;

	groupNo = 0; //kjgw

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
		sizeof(S_TEST_COND_PATTERN_DATA));

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

	mode = P1_MODE_IDLE;
	token = strtok(NULL, in_delimiter);
	if(token == NULL) {
	}
	memset(buf, 0, sizeof buf);
	strcpy(buf, token);
	if((strncmp(buf, "i", 1) == 0) || (strncmp(buf, "I", 1) == 0)) {
		mode = P1_MODE_CC;
		myData->testCond[ch].pattern[0].cmd_val = 0;
	} else if((strncmp(buf, "p", 1) == 0) || (strncmp(buf, "P", 1) == 0)) {
		mode = P1_MODE_CP;
		myData->testCond[ch].pattern[0].cmd_val = 1;
	} else {
	}

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

		token = strtok(NULL, in_delimiter);
		if(token == NULL) {
			break;
		}
		memset(buf, 0, sizeof buf);
		strcpy(buf, token);
		if((strncmp(buf, "EOF", 3) == 0) || (strncmp(buf, "eof", 3) == 0)) {
		} else {
			myData->testCond[ch].pattern[i].cmd_val
				= string_to_long(buf, mode, 1);
		}
	}

	myData->testCond[ch].pattern[i].t_val = (-1);

    fclose(fp);
	return 0;
}

int Write_Pattern_File(char *psName1, int ch, int pattern_index, char *pattern)
{
	char fileName[128];
	int groupNo;
    FILE *fp;

	groupNo = 0; //kjgw

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
#endif

