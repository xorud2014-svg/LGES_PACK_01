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
	unsigned char		saveTimeFlag;		//ktg_210411
		 									//0 : saveInterval,  1 : 1hour 100ms after saveInterval
	
	//unsigned char		maxSaveNo;			//csk_190830_c PACK_MERGE
	//unsigned char		reserved1[3];		//csk_190830_c PACK_MERGE
	int					maxSaveNo; //kjg_130923 unsigned char -> int
} S_DATA_SAVE_CONFIG;

typedef struct s_data_save_result_data_tag {
	unsigned char		open_year;
	unsigned char		open_month;
	unsigned char		open_day;
	unsigned char		reserved1;

	//int					fileIndex;		//csk_190830_c PACK_MERGE
	//int					maxFileIndex;	//csk_190830_c PACK_MERGE
	int					file_index;
	int					max_file_index;
	int					divisionCount;

	long				last_resultIndex;
} S_DATA_SAVE_RESULT_DATA;

typedef struct s_data_save_result_data_formation_tag {
	//unsigned char		test_index;			//csk_190830_c PACK_MERGE
	int					test_index; //kjg_130923 unsigned char -> int

	unsigned char		make_header;		//csk_190830_c PACK_MERGE
	unsigned char		open_month;
	unsigned short		open_year;

	unsigned char		open_day;
	unsigned char		open_hour;
	unsigned char		open_min;
	unsigned char		open_sec;

	//unsigned char		file_index;			//csk_190830_c PACK_MERGE
	//unsigned char		make_header;		//csk_190830_c PACK_MERGE
	//unsigned char		reserved1[2];		//csk_190830_c PACK_MERGE
	int					file_index; //kjg_130923 unsigned char -> int

	int					save_count;
} S_DATA_SAVE_RESULT_DATA_FORMATION;

typedef struct s_data_save_monitor_data_formation_tag {
	//unsigned char		test_index;			//csk_190830_c PACK_MERGE
	int					test_index; //kjg_130923 unsigned char -> int

	unsigned char		open_month;
	unsigned char		reserved1;			//csk_190830 PACK_MERGE
	unsigned short		open_year;

	unsigned char		open_day;
	unsigned char		open_hour;
	unsigned char		open_min;
	unsigned char		open_sec;

	//unsigned char		file_index[MAX_CH_256];	//csk_190830_c PACK_MERGE
	int					file_index[MAX_CH_256]; //kjg_130923 unsigned char -> int
	unsigned char		make_header[MAX_CH_256];

	//int					save_count[MAX_CH_256];	//csk_190830_c PACK_MERGE
	int					divisionCount[MAX_CH_256];
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
