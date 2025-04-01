#ifndef __ANALOG_H__
#define __ANALOG_H__

void	Analog_Value_Input(int);
void	Analog_Value_Input_1(int);
void	Analog_Value_Input_2(void);

void	Get_AD_Value(int, int, int);
void	Get_AD_Value_1(int, int, int);
void	Read_AD_Data_1(int, int, int, int, int);

void	Get_AD_Value_2(int, int, int);
void 	Read_AD_Data_2(int, int, int, int, int);

void	Get_AD_Value_3(int, int, int);
void 	Read_AD_Data_3(int, int, int, int, int);

void	Get_AD_Value_4(int, int, int);
void	Read_AD_Data_4(int, int, int, int, int);

void	Get_AD_Value_5(int, int, int);
void	Read_AD_Data_5(int, int, int, int, int);

void	Get_AD_Value_6(int, int, int);
void	Read_AD_Data_6(int, int, int, int, int);

void	Get_AD_Value_7(int);
void	Read_AD_Data_7(int, int, int, int);

void	AD_Data_Calculate(int, int, int, int);
void	AD_Data_Calculate_1(int, int, int, int);
void	AD_Data_Calculate_2(int, int, int);
void	AD_Data_Calculate_V(int, int, int, float, int);
void	AD_Data_Calculate_I(int, int, int, float, int);
void	AD_Data_Calculate_Vp(int, int, int, float);
void	AD_Data_Calculate_Vb(int, int, int, float);

void	Set_Mux(int, int, int);
void	Set_Mux_1(int, int, int);
void	Set_Mux_2(int, int, int);
void	Set_Mux_3(int, int, int);
void	Set_Mux_4(int, int, int);
void	Set_Mux_5(int, int, int);
void	Set_Mux_6(int, int, int);
void	Set_Mux_7(int);

void	CalChAverage(int, int);
void	CalSourceAverage(int, int);
void	CalSourceAverage2(int);
void	CalibratorSource(int);

double	Ref_V_Ratio(int);
double	Ref_I_Ratio(int);
double	Ref_V_Value(void);
double	Ref_I_Value(void);

void	AnalogValue_Operate(int, int);

void	aGetAD_Reference(int);
void	aGetAD_Channel(int);
void	aGetAD_Temp(int);

void	aSetMux_Ground(void);
void	aSetMux_Reference(int);
void	aSetMux_Channel(int);
void	aSetMux_Temp(int);

void	aSetAuxDA(int);
void	aSetAuxDA_V(int, int, int);
void	aSetAuxDA_I(int, int, int);

void	aCalChAverage(int);
void	aCalCh_Voltage(int, int, int, int);
void	aCalCh_Current(int, int, int, int);
void	aCalCh_Capacity(void);

void	aCalReferenceAverage1(int);
void	aCalReferenceAverage2(int);
void	aCalibratorReference(void);

void	aSetMainDA_V(int, long);
void	aSetMainDA_I(int, long);

int		aFindADCaliPoint(int, int, long, int, int);
int		aFindADCaliPoint_1(int, int, long, int, int);
int		aFindADCaliPoint_2(int, int, long, int, int);
int		aFindDACaliPoint(int, int, long, int, int);
int		aFindDACaliPoint_1(int, int, long, int, int);
int		aFindDACaliPoint_2(int, int, long, int, int);

//kjg_logic_type_140325_s
void	Analog_Value_Input_3(int);
void	Analog_Value_Input_3a(int);
void	Analog_Value_Input_3b(int);
void	Source_Value_Average(int);
void	Ch_Value_Average(int);
//kjg_logic_type_140325_e
#endif
