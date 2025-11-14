#ifndef __DATASAVE_STR_H__
#define __DATASAVE_STR_H__

#include "SysDefine.h"
#include "DataSave_def.h"

typedef struct s_data_save_misc_tag {
	int					processPointer;
} S_DATA_SAVE_MISC;

typedef struct s_data_save_config_tag {
	unsigned char		resultData_saveFlag;
	unsigned char		monitoringData_saveFlag;
	unsigned char		monitoringData_saveInterval;
	unsigned char		checkData_saveFlag;

	unsigned char		maxSaveNo;
	unsigned char		reserved1[3];
} S_DATA_SAVE_CONFIG;

typedef struct s_data_save_result_data_tag {
	unsigned char		open_year;
	unsigned char		open_month;
	unsigned char		open_day;
	unsigned char		reserved1;

	int					fileIndex;
	int					maxFileIndex;
	int					divisionCount;

	long				last_resultIndex;
} S_DATA_SAVE_RESULT_DATA;

typedef struct s_data_save_result_data_formation_tag {
	unsigned char		test_index;
	unsigned char		open_month;
	unsigned short		open_year;

	unsigned char		open_day;
	unsigned char		open_hour;
	unsigned char		open_min;
	unsigned char		open_sec;

	unsigned char		file_index;
	unsigned char		make_header;
	unsigned char		reserved1[2];

	unsigned int		save_count;
} S_DATA_SAVE_RESULT_DATA_FORMATION;

typedef struct s_data_save_monitor_data_formation_tag {
	unsigned char		test_index;
	unsigned char		open_month;
	unsigned short		open_year;

	unsigned char		open_day;
	unsigned char		open_hour;
	unsigned char		open_min;
	unsigned char		open_sec;

	unsigned char		file_index[MAX_CH_256];
	unsigned char		make_header[MAX_CH_256];

	unsigned int		save_count[MAX_CH_256];
} S_DATA_SAVE_MONITOR_DATA_FORMATION;

typedef struct s_data_save_tag {
	unsigned char		signal[MAX_SIGNAL];

	S_DATA_SAVE_MISC	misc;
	S_DATA_SAVE_CONFIG	config;
	S_DATA_SAVE_RESULT_DATA	resultData[MAX_CH_PER_MODULE];

	S_DATA_SAVE_RESULT_DATA_FORMATION
						resultData_formation[MAX_GROUP_4];
	S_DATA_SAVE_MONITOR_DATA_FORMATION
						monitorData_formation[MAX_GROUP_4];
} S_DATA_SAVE;

#endif
