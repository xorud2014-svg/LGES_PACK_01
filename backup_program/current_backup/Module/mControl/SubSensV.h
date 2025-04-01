#ifndef __SUBSENSV_H__
#define __SUBSENSV_H__

void	SubSensV_adc_start(void);
void	SubSensV_Control1(int);
void	SubSensV_No_DMA_Read(int);
void	SubSensV_Control2(int);
void  	Cal_SubSensV_Source1(int);
void  	Cal_SubSensV_Source2(int);
void  	Cal_SubSensV_Ch(int);

float	Convert_Vth_to_Temp(float, int);
float	Convert_Vth_to_Ohm(float, int);
float	Convert_Vwet_to_Humidity(float, int);	//csk_191128

void	SubSensV_process(void);
void	SubSensV_process_CAN_DAQ(void);	//kjg_180914	//ktg_200728
#endif
