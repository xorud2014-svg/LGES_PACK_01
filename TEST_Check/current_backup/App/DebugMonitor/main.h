#ifndef __MAIN_H__
#define __MAIN_H__

void	Test_Print(void);
void	TimeSlot_Print(void);
void	DataStructureSize_Print(void);
void	Debug_Print(void);

void 	Sequence_Charge_Print(int);
void	SOC_Tracking_Print(int);
void 	Charging_Count_Print(int);
void 	Charging_RPT_SOC_Print(int);

void	AppControl_Print(void);
void	COA_Client_Print(void);
void	COB_Client_Print(int);
void	COC_Client_Print(void);
void	Temp_Client_Print(int);
void	Can_Client_Print(int);

void	DataSave_Print(void);
void	Jig_Control_Print(int);
void	Job_Change_Print(int);
void	Pitch_Change_Print(int);

void	ModuleState_Print(void);
void	GroupState_Print(void);
void	BoardState_Print(void);
void	ChannelState_Print(void);
void	FCH_State_Print(void);
void	COB_Channel_Print(int);
void	LineCheck_Print(int);

void	IO_Print(void);
void	TestCond_Print(void);
void	CaliData_Print(void);
void	SubSensV_Print(void);
void	AuxV_Print(void);
void	AuxTemp_Print(void);
void	Message_Print(void);
void	COM_Print(void);
void	CAN_Print(void);

void	Chamber_Print(void);
void	Shin_Print(void);	//shh_test

int		data_10ms_write(int, int);

int		COA_Client_TestCond_write_V1009(int);
int		COA_Client_TestCond_write_V100B(int);
int		COA_Client_TestCond_write_V100C(int);
int		COA_Client_TestCond_write_V100D(int);
int		COA_Client_TestCond_write_V100D_pattern(int);
int		COA_Client_TestCond_write_TimeSch(int); //kjh_160418
int		write_IO_Print(int);
 
int		KeyInput(void);

void	TimeSlot_Print2(void);  //shhw_240611
long	makePerTime(long);		//shhw_240611

int		main(void);

#endif
