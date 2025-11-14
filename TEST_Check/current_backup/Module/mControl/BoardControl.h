#ifndef __BOARDCONTROL_H__
#define __BOARDCONTROL_H__

void	BoardControl(int);

void	bIdle(int, int);
void	bStandby(int, int);
void	bRun(int, int);
void	bPause(int, int);
void	bCali(int, int);

void	bCalculateAverage(int);

void	bReadOTFault(int);
void	bSemiSwitch(int, int);
void	bSendCmd_to_Ch(int, int, int);
void	bState(S_BD_DATA *, unsigned char, unsigned char);
void	bSendSignal_to_Ch(int, int, int);

void	bVICmd_Setting(int, long, int, int, int);

void	bVICmd_Output(int);

int		bVCmd_Calculate(int);
int		bICmd_Calculate(int);

void	bDA_Output(int, int, int);

void	bCDSelect_Output(int);
void	bRangeRelay_Output(int);
void	bRunRelay_Output(int);
void	bParallelRelay_Output(int);
#endif
