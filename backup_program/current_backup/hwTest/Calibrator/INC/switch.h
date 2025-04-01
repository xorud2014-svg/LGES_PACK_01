#ifndef _SWITCH_H_
#define _SWITCH_H_

#define STX		0x02
#define ETX		0x03
#define SWITCH_ADDR	0x20
#define CMD_ONE_ON	0x01
#define CMD_ONE_OFF	0x02
#define CMD_ALL_OFF	0x03
#define CMD_TEST	0x04
#define CMD_ACK		0xF1
#define CMD_NACK	0xF2
#define CMD_RESET	0xF3
#define CMD_MODE_OUT	0x10 //021011
#define CMD_MODE_IN	0x11 //021011
#define BD_PER_MODULE	4
#define CH_PER_BD 64

void TraySwitchOnOff(int,int,int);
void ScanTypeSetting(int);
void ResetTraySwitch(void);
void CellMapping(void);
void TransData(int, int, int);
int	SendSerialData(void);
void BccCreate(int);
void SendDataPrint(int);
void ReceiveDataPrint(int);
int	BccCheck(int);
int	CodeCheck(int);
int	RetryCheck(void);

#endif
