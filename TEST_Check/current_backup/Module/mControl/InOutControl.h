#ifndef __INOUTCONTROL_H__
#define __INOUTCONTROL_H__

void	InOutControl(int);
void	DIO_Signal(void);
void	DIO_Signal_RemotePs_General1(void);
void	DIO_Signal_RemotePs_General2(void);
void	DIO_Signal_RemotePs_Pack1(void);
void	DIO_Signal_RemotePs_Pack2(void);
void	DIO_Signal_RemotePs_Pack3(void);
void	DIO_Signal_RemotePs_Pack_Dsp1_block1(void);
void	DIO_Signal_RemotePs_Pack_Dsp1_block2(void);
void	DIO_Signal_RemotePs_Pack1_Block2(void);
void	DIO_Signal_RemotePs_Pack_Dsp2_block2(void);	//jhkw_181114
void	DIO_Signal_RemotePs_Pack_Dsp2_block2_2(void);	//phb_230224
void	DIO_Signal_RemotePs_Pack_Dsp2_block2_TR1_FUSE(void); 	//Only use for 23PPSCSA029, shhw_231013
void	DIO_Signal_RemotePs_Pack_Dsp2_block4(void);	//jhkw_170917
void	DIO_Signal_RemotePs_Pack_Dsp2_block4_2(void);	//jhkw_180810
void	DIO_Signal_RemotePs_Pack_Dsp3_block6(void);	//jhkw_210319	//phb_230324

void	DIO_RippleSwitch(void);
void	DIO_JigSwitch(void);

unsigned char	InCheck(int, int, unsigned char, int);

void	I_InScan(void);
void	I_In_FlagCheck(void);
int		I_In_FlagCheck_PowerSwitch(void);
int		I_In_FlagCheck_Main_EmgSwitch(void);
int		I_In_FlagCheck_Sub_EmgSwitch(void);
int		I_In_FlagCheck_PowerFail(void);
int		I_In_FlagCheck_PowerFail_1(void);
int		I_In_FlagCheck_PowerFail_2(void);
int		I_In_FlagCheck_PowerFail_3(void);
int		I_In_FlagCheck_PowerFail_4(void);
int		I_In_FlagCheck_PSFail(void);
int		I_In_FlagCheck_Chamber(void);
int		I_In_FlagCheck_Chamber_1(void);
int		I_In_FlagCheck_Chamber_2(void);
int		I_In_FlagCheck_Chamber_3(void);
int		I_In_FlagCheck_Chamber_4(int);
int		I_In_FlagCheck_Chamber_5(void);	//jhkw_130711
int		I_In_FlagCheck_Chamber_8(int);	//jhkw_191216
int		I_In_FlagCheck_Fan(void);
void	I_OutScan(void);

void	M_InScan(void);
int		M_InCheck(int, int, int, int);
void	M_In_FlagCheck(void);
void	M_In_FlagCheck_Pack(int);
void	M_In_FlagCheck_Pack_TR1_FUSE(int); //Only use for 23PPSCSA029, shhw_231013
void	M_OutScan(void);

void	C_InScan(void);
int		C_InCheck(int, int, int, int);
void	C_In_FlagCheck(void);
void	C_In_Flag_RippleSwitch(void);
void	C_OutScan(void);

void	E_InScan(void);
int		E_InCheck(int, int, int, int);
void	E_In_FlagCheck(void);
void	E_OutScan(void);

void	CheckWDT(void);
void	EnableWDT(void);
void	DisableWDT(void);
void	RefreshWDT(void);

void	Clear_OutPort(void);

void	PNE_AC_Power_Fail_Detect(void);
void	PNE_AC_Power_Fail_Detect_1(void);
void	PNE_AC_Power_Fail_Detect_2(void);
void	PNE_AC_Power_Fail_Detect_3(void);
void	PNE_AC_Power_Fail_Detect_4(void);
void	PNE_AC_Power_Fail_Detect_5(void);
void 	PNE_AC_Power_Fail_Detect_6(void);
void 	PNE_AC_Power_Fail_Detect_7(void);
void 	PNE_AC_Power_Fail_Detect_8(void);	//jhkw_170917
#endif
