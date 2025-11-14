#ifndef __LOCAL_UTILS_H__
#define __LOCAL_UTILS_H__

#include "common_utils.h"

void	Init_SystemMemory(void);
int		Read_DataSave_Config(void);

int		Open_ResultData_1(int);
int		Open_ResultData_2(int);
void	Save_ResultData_1(int);
int		Save_ResultData_2(void);
int		Save_PulseData(int);

int		Open_ResultData_General_1(int);
int		Open_ResultData_General_2(int);
void	Save_ResultData_General_1(int);
int		Save_ResultData_General_2(int);
int		Save_RecordData_General(int);

int		Open_ResultData_Formation_1(int);
int		Save_ResultData_Formation(int);
int		Save_ResultData_Formation_1(int);
int		Save_ResultData_Formation_2(int);

int		Open_MonitoringData_Formation_1(int);
void	Open_MonitoringData_Formation_2(int, int, int);
int		Save_MonitoringData_Formation(int);
int		Make_Header_MonitoringData_Formation(int);
int		Write_DataSaveNo(int);

/*kjg_120723_w
//int		get_bd_index(int, int);
//int		set_ch_index(int, int); */

int		convert_datasave_code(int, int);

int		create_run_file(int);
int		delete_run_file(int);
#endif
