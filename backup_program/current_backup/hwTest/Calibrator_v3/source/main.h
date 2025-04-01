#ifndef _MAIN_H_
#define _MAIN_H_

#define CALI_NORMAL			0
#define CALI_VOLTAGE 		1
#define CALI_CURRENT 		2
#define CALI_LIMIT			3

#define SCAN_VOLTAGE		0
#define SCAN_CURRENT		1

#define OFF 				0
#define ON 					1
#define CH_READ_COUNT1 		10
#define CH_READ_COUNT2 		100
#define REF_READ_COUNT 		10
#define CMD_READ_COUNT 		10
#define METER_PC_CTRL		0
#define METER_SBC_CTRL		1
#define CAL_BOX_NONUSE		0
#define CAL_BOX_SBC_CTRL	1
#define CAL_BOX_PC_CTRL		2

#define LG_5V10A    		0
#define VKEIG_5V10A			1
#define LGCN_PB2			2
#define LGCE_NJ_5V2A		3
#define SW_DEMO_5V3A		4

int CharToInt(char);
long Convertion(void);
long RealData(void);

void SetAuxDA_V(int, unsigned short);
void SetAuxDA_V1(int, unsigned char);
void SetAuxDA_V2(int, unsigned short);

void SetAuxDA_I(int, unsigned short);
void SetAuxDA_I1(int, unsigned char);
void SetAuxDA_I2(int, unsigned short);

void RelayAllOn(void);
void RelayAllOff(void);
void RelayOnOff(int, int);

void SetMux(int, int);
void SetMux1(int, int);
void SetMux2(int, int);
void SetMux3(int, int);

void VoltDACReset(void);
void CurrDACReset(void);

void CreatADFile(int, int);
int AD_stdev(float, int);

void Ref_V_Read(void);
void Ref_I_Read(void);
void MainADC_Calibration(void);
void SetMainDA_V(int);
void SetMainDA_I(int);
void SetMainDA_L(int);
int MainDA_V_Read(void);
int MainDA_I_Read(void);
void MainDAC_Calibration(void);
void MainDAC_Calibration2(int);
void MainDAC_CaliData_Write(int);
void MainDAC_CaliData_Read(int);

int ReadCommand(int, int *, int *);
int DoRead(int);
int main(int, char *[]);
#endif
