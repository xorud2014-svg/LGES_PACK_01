#ifndef __APPCONTROL_STR_H__
#define __APPCONTROL_STR_H__

#include "SysDefine.h"
#include "AppControl_def.h"

typedef struct s_app_misc_tag {
	long				quitDelayTime;
	long				processCheckTime;
	int					processPointer;
	char				path1[256];

	unsigned char		Load_Process[MAX_PROCESS_GROUP][MAX_GROUP_8];
	unsigned char		Load_Process_DAQ[12];
	unsigned char		Load_Process_CAN[12];
	unsigned char		Load_Process_COM[12];
} S_APP_MISC;

typedef struct s_app_config_tag {
	char				modelName[128];
	int					moduleNo;
	short int			totalGroup;
	short int			systemType;
	int					systemModel;

	unsigned char		bootOnStart;
	unsigned char		sbcType;
	unsigned char		osVersion;
	unsigned char		debugType;
	int					versionNo;
} S_APP_CONFIG;

typedef struct s_app_control_tag {
	S_APP_MISC			misc;
	S_APP_CONFIG		config;

	unsigned char		signal[MAX_SIGNAL];
} S_APP_CONTROL;

#endif
