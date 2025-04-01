#ifndef __TIMESCHEDULE_STR_H__
#define __TIMESCHEDULE_STR_H__

#include "SysDefine.h"
#include "TimeSchedule_def.h"

typedef struct s_timesch_misc_tag {
	long					quitDelayTime;
	long					processCheckTime;
	int						processPointer;
	char					path1[256];

} S_TIMESCH_MISC;

typedef struct s_timesch_config_tag {
	char					modelName[128];
	int						moduleNo;
	short int				totalGroup;
	short int				systemType;
	int						systemModel;

	unsigned char			bootOnStart;
	unsigned char			sbcType;
	unsigned char			osVersion;
	unsigned char			debugType;
	int						versionNo;

	unsigned char			useFlag;
	unsigned char			reserved1[3]; //kjg_171219
} S_TIMESCH_CONFIG;

typedef struct s_timesch_timer_tag {
	unsigned long			timer_1day;
	unsigned long			timer_1time;

} S_TIMESCH_TIMER;
/*
typedef struct s_timesch_ch_code_tag {
	long			compCount;

	unsigned char	compType;
	unsigned char	reserved1[3];

	short int		compIndex1;
	short int		compIndex2;
} S_TIMESCH_CH_CODE;
*/
typedef struct s_timesch_test_condition_tag {
	long					local_object[MAX_TEST_STEP][MAX_LOC_TEST_OBJECT];
	//S_TIMESCH_CH_CODE		local_chCode[MAX_TEST_STEP][MAX_LOC_CODE];

} S_TIMESCH_TEST_CONDITION;

typedef struct s_timesch_control_tag {
	S_TIMESCH_MISC				misc;
	S_TIMESCH_CONFIG			config;
	S_TIMESCH_TIMER				timer[MAX_TEST_COND_COUNT];
	S_TIMESCH_TEST_CONDITION	testCond[MAX_TEST_COND_COUNT];

	unsigned char				signal[MAX_SIGNAL];
} S_TIMESCH_CONTROL;

#endif
