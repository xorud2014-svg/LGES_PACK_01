#ifndef __LOCAL_UTILS_H__
#define __LOCAL_UTILS_H__

#include "common_utils.h"

int		Initialize(int, char *[]);
void	Init_SystemMemory(int);

int		Read_COB_Client_Config(void);
int		Write_COB_Client_Config(void);
int		Read_WorkMode(void);
int		Write_WorkMode(void);
int		Read_DataSaveNo(int);
int		Read_ChArray(void);
int		Read_ChArray1(void);
int		Read_ChArray2(void);
int		Read_ChArray3(void);
int		Read_COB_ConfigData(void);
int		Write_COB_ConfigData(void);
int		Read_testConfig(void);
int		Read_sensor_limit(void);
int		Write_sensor_limit(void);
int		Read_chamber_set(void);
int		Write_chamber_set(void);
int		Read_job_change_set(void);
int		Write_job_change_set(int);

void	CaliUpdateCh(int);
int		Write_BdCaliData(int);

void	Check_GroupState(void);

void	convert_test_cond(void);
void	convert_test_cond_common_safety2(S_TEST_CONDITION *, int);
void	convert_test_cond_check(S_TEST_CONDITION *, int);
void	convert_test_cond_check2(S_TEST_CONDITION *, int);
void	convert_test_cond_check3(S_TEST_CONDITION *, int);
void	convert_test_cond_ocv(S_TEST_CONDITION *, int, int, int);
void	convert_test_cond_rest(S_TEST_CONDITION *, int, int, int);
void	convert_test_cond_long_time_rest(S_TEST_CONDITION *, int, int, int);
void	convert_test_cond_charge(S_TEST_CONDITION *, int, int, int);
void	convert_test_cond_discharge(S_TEST_CONDITION *, int, int, int);
void	convert_test_cond_z(S_TEST_CONDITION *, int, int, int);
void	convert_test_cond_pattern(S_TEST_CONDITION *, int, int, int);

void	convert_test_cond_default(int, int, long, long);

void	StateChange_Pause(int);

void	Update_RealTime(char *);

//kjg_110713
int		Write_Cali_Set_Main(void);
int		Write_Cali_Set_Ch(void);
int		Write_jig_temp_set_data(void);
int		Read_jig_temp_set_data(void);

int		get_chInGroup(int);
int		get_chNo(int, int);
void	convert_ch_attribute(int);

#endif
