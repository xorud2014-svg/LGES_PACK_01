#ifndef __CH_UTILS_H__
#define __CH_UTILS_H__

void	Output_Ch_Switch(int);
void	Output_Ch_Switch_General(int, int);
void	Output_Ch_Switch_Pack(int, int);
void	Output_Ch_Switch_PWM2(int, int);

void	cOutSwitch_Select_rangeI(int, int);
void	cOutSwitch_Select_rangeV(int, int); //kjhw_120424
void	cOutSwitch_Select_ON_1(int, int, int);
void	cOutSwitch_Select_ON_2(int); //kjhw_150120
void	cOutSwitch_Select_OFF_1(int, int, int);
void	cOutSwitch_Select_OFF_2(int); //kjhw_150120

void	cOutSwitch_P1_General1(int, int);
void	cOutSwitch_P11_General1(int, int);
void	cOutSwitch_P21_General1(int, int);
void	cOutSwitch_P51_General1(int, int);
void	cOutSwitch_P61_General1(int, int);
//void	cOutSwitch_P71_General1(int, int);
//void	cOutSwitch_P71_General2(int, int);
void	cOutSwitch_P71_General_N1(int, int);
void	cOutSwitch_P81_General1(int, int);
void	cOutSwitch_P81_General2(int, int);
void	cOutSwitch_P81_General3(int, int);
//void	cOutSwitch_P85_General1(int, int);
//void	cOutSwitch_P85_General2(int, int);
void	cOutSwitch_P85_General_N1(int, int);
void	cOutSwitch_P91_General1(int, int);
void	cOutSwitch_P95_General1(int, int);

void	cOutSwitch_P1_Pack1(int, int);
void	cOutSwitch_P11_Pack1(int, int);
void	cOutSwitch_P21_Pack1(int, int);
void	cOutSwitch_P51_Pack1(int, int);
void	cOutSwitch_P53_Pack1(int, int); //kjhw_120503
void	cOutSwitch_P61_Pack1(int, int);
void	cOutSwitch_P63_Pack1(int, int); //kjhw_120503
void	cOutSwitch_P65_Pack1(int, int);
void	cOutSwitch_P71_Pack1(int, int);
void	cOutSwitch_P73_Pack1(int, int); //kjhw_120430
void	cOutSwitch_P75_Pack1(int, int);
void	cOutSwitch_P77_Pack1(int, int); //kjhw_121211
void	cOutSwitch_P81_Pack1(int, int);
void	cOutSwitch_P83_Pack1(int, int); //kjhw_120514
void	cOutSwitch_P85_Pack1(int, int);
void	cOutSwitch_P87_Pack1(int, int); //kjhw_120514
void	cOutSwitch_P91_Pack1(int, int);
void	cOutSwitch_P93_Pack1(int, int); //kjhw_121212
void	cOutSwitch_P95_Pack1(int, int);
void	cOutSwitch_P97_Pack1(int, int); //kjhw_121212
void	cOutSwitch_P101_Pack1(int, int); //kjhw_140828
void	cOutSwitch_P103_Pack1(int, int); //kjhw_140828
void	cOutSwitch_P105_Pack1(int, int); //kjhw_140828
void	cOutSwitch_P107_Pack1(int, int); //kjhw_140828

void	cOutSwitch_P1_PWM2(int, int);
void	cOutSwitch_P11_PWM2(int, int);
void	cOutSwitch_P21_PWM2(int, int);
void	cOutSwitch_P71_PWM2(int, int);

int		cFindADCaliPoint(int, double, int, int, int);
int		cFindDACaliPoint(int, long, int, int);
void    cCalCmdV(int, long, int, int);
void	cCalCmdV_external(int, long, int, int);
void    cCalCmdI(int, long, int, int);
double	cCmd_V_Ratio(int, int);
double	cCmd_I_Ratio(int, int);

int		set_testCond_point(int);
long	get_softCharge_current(long);
long	cal_internal_dv(int, long, long, long);
long	cal_internal_dv2(int, long, long, long);
void	calculate_DCR_2(int, int);

int		find_master_ch(int);

int		cCheck_Reference_Condition(int);
#endif
