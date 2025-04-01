#ifndef __LOCAL_UTILS_H__
#define __LOCAL_UTILS_H__

#include "common_utils.h"

int		Initialize(int, char *[]);
void	Init_SystemMemory(int);

int		Read_COC_Client_Config(void);
int		Write_COC_Client_Config(void);
int		Read_COC_Client_testConfig(void);

void	CaliUpdateCh(int);
int		Write_BdCaliData(int);

void	convert_test_cond(int);
void	convert_test_cond_cycle(int, int, long, long);
void	convert_test_cond_loop(int, int, long, long);
void	convert_test_cond_default(int, int, long, long);
void	convert_test_cond_charge_discharge_z(int, int, long, long);
void	convert_test_cond_pattern(int, int, long, long);
void	convert_test_cond_cable_check(int);

void	StateChange_Pause(int);

void	Update_RealTime(char *);
#endif
