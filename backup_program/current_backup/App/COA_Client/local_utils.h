#ifndef __LOCAL_UTILS_H__
#define __LOCAL_UTILS_H__

#include "common_utils.h"

int		Initialize(int, char *[]);
void	Init_SystemMemory(int);

int		Read_COA_Client_Config(void);
int		Write_COA_Client_Config(void);
int		Read_COA_Client_testConfig(void);

void	CaliUpdateCh(int);
void	CaliUpdateCh1(int); //kjhw_121031 Vref x 2
int		Write_BdCaliData(int);

int		read_test_cond_step_file(int, int);
int		read_test_cond_time_step_file(int, int);

int		Write_test_cond_step_file(S_P1_FILE_TEST_COND *, int, int);	//csk_190213

int		convert_test_cond(int);
int		convert_test_cond_time(int);
void	convert_test_cond_common_aux(S_P1_TEST_COND_SAFETY *, int);
void	convert_test_cond_common_can(S_P1_TEST_COND_SAFETY *, int);

void	convert_test_cond_cycle(S_P1_TEST_COND_STEP *, int, int, long, long);
void	convert_test_cond_loop(S_P1_TEST_COND_STEP *, int, int, long, long);
int		convert_test_cond_step_default(S_P1_TEST_COND_STEP *, int, int, long, long, int);

void	convert_test_cond_step_aux(S_P1_TEST_COND_STEP *, int, int);
void	convert_test_cond_step_can(S_P1_TEST_COND_STEP *, int, int);

void	convert_test_cond_ocv_rest(S_P1_TEST_COND_STEP *, int, int, long, long); //kjhw_121031 Vref x 2
//void	convert_test_cond_charge_discharge_z(S_P1_TEST_COND_STEP *, int, int, long, long);
int		convert_test_cond_charge_discharge_z(S_P1_TEST_COND_STEP *, int, int, long, long);	//ktg_210310
int		convert_test_cond_pattern(S_P1_TEST_COND_STEP *, int, int, long, long, int);
void	convert_test_cond_external_can(S_P1_TEST_COND_STEP *, int, int, long, long);
int		convert_test_cond_usermap(S_P1_TEST_COND_STEP *, int, int, long, long, int);

void	convert_test_cond_cable_check(int);

int		convert_test_cond_step_update(int);
int		convert_test_cond_common_safety_update(int);
void	convert_test_cond_module_coa_common_safety(int);
void	convert_test_cond_module_coa_step(int, int);

void	StateChange_Pause(int);

void	Update_RealTime(char *);
#endif
