#ifndef __MAIN_H__
#define __MAIN_H__

void	Test_Print(void);
void	TimeSlot_Print(void);
void	TimeSlot_Print2(void);
void	DataStructureSize_Print(void);
void	Debug_Print(void);

void	AppControl_Print(void);
void	COA_Client_Print(void);
void	COB_Client_Print(int);
void	COC_Client_Print(void);

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
void	Humidity_Print(void);
void	Gas_Print(void);	//sec_220926
void	AuxTemp_Print(void);
void	Message_Print(void);
void	COM_Print(void);
void	CAN_Print(void);

void	Chamber_Print(void);
void	jhk_test(void);

int		data_10ms_write(int, int);

int		COA_Client_TestCond_write_V1009(int);
int		COA_Client_TestCond_write_V100B(int);
int		COA_Client_TestCond_write_V100C(int);
int		COA_Client_TestCond_write_V100D(int);
int		COA_Client_TestCond_write_V100D_pattern(int);
int		COA_Client_TestCond_write_TimeSch(int); //kjh_160418
int		write_IO_Print(int);
 
int		KeyInput(void);
int		main(void);

#endif
