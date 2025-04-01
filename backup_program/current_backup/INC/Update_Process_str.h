#ifndef __UPDATE_PROCESS_STR_H__
#define __UPDATEP_ROCESS_STR_H__

#include "SysDefine.h"
#include "Update_Process_def.h"

typedef struct s_update_process_misc_tag {
	long					quitDelayTime;
	long					processCheckTime;
	int						processPointer;
	char					path1[256];

} S_UPDATE_PROCESS_MISC;

typedef struct s_update_process_config_tag {
	char					modelName[128];
	int						moduleNo;
	short int				totalGroup;
	short int				systemType;
	int						systemModel;

	unsigned char			useFlag;
	unsigned char			reserved1[3];
} S_UPDATE_PROCESS_CONFIG;


typedef struct s_update_process_tag {
	S_UPDATE_PROCESS_MISC			misc;
	S_UPDATE_PROCESS_CONFIG			config;

	unsigned char				signal[MAX_SIGNAL];
} S_UPDATE_PROCESS;

#endif
