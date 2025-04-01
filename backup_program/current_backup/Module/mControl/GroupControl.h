#ifndef __GROUPCONTROL_H__
#define __GROUPCONTROL_H__

void	GroupControl(int);

void	gIdle(int);
void	gStandby(int);
void	gRun(int);
void	gPause(int);
void	gCali(int);

void	gStandby_C(int);
void	gRun_C(int);
void	gPause_C(int);

void	gStandby_F(int);
void	gRun_F1(int);
void	gRun_F2(int);
void	gRun_F3(int);
void	gRun_F4(int);
void	gRun_F_SDI_5V_400A_200A_100A_10A(int);
void	gRun_F_SDI_5V_400A_200A_100A_10A_2(int);
void	gRun_F_SDI_5V_450A_200A_100A_10A(int);
void	gPause_F1(int);
void	gPause_F2(int);
void	gPause_F3(int);
void	gPause_F4(int);

void	gSignalCheck(int);
void	gSignalCheck_C(int);
void	gSignalCheck_F(int);

void	gSendSignal_to_Bd(int, int, int);

void	gCompare_Ch_Average_1(int);
int		gCompare_Ch_Average_2(int);
int		gCompare_Ch_Average_3(int);

void	gCellCheck_Compare(int);

int		gLoadLineCheck(int, int);

void	gCali_MainDAC_Default(void);
void	gCali_MainDAC_1(void);
int		bFindDACaliPoint(int, long, int, int);
void	gCalCmdV(int, long, int, int);
void	gCalCmdI(int, long, int, int);

void	gCali_bd(void);
void	gCali_bd_initialize(void);
void	gCalculate_bd_CaliData_1(int);
void	gCalculate_bd_CaliData_2(int);
void	gCali_bd_select(void);
void	gCali_bd_cd_select(void);
void	gCali_bd_cmd_output(void);
void	gCali_bd_next_step_check(void);
void	gCali_send_cali_data(int, int);
void	gCali_bd_schedule(void);
void	gCali_bd_next_div_check(void);
#endif
