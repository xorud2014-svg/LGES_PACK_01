#ifndef __MAIN_H__
#define __MAIN_H__

#define VOLTAGE 	0
#define CURRENT 	1

#define DMM_IN_PC	0
#define DMM_IN_SBC	1

#define COUNT		5
#define SOURCE_COUNT	5

int		main(int, char *[]);
int		DoRead(int);
int		ReadCommand(int, int *, int *);
int		cal_std_value(double, int);
int		CharToInt(char);
long	Conversion(void);
int		RealData(void);
int		swapBit(int);
void	SetAuxDA(int, unsigned char);
void	RelayOnOff(int, int);
void	SetMux(int, int);
#endif
