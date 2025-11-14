#ifndef __SUBSENSV_H__
#define __SUBSENSV_H__

#define SWAP(x, y, z)((z)=(x), (x)=(y), (y)=(z))
void	SubSensV_adc_start(void);
void	SubSensV_adc_start_Default(void); //20190902 KHK
void	SubSensV_adc_start_12(void); //20190902 KHK
void	SubSensV_Control(int);
void	SubSensV_No_DMA_Read(int);
void	SubSensV_Control2(int);
void  	Cal_SubSensV_Source1(int);
void  	Cal_SubSensV_Source1_Default(int);//20190902 KHK
void 	Cal_SubSensV_Source1_12_Filter1(int, int); //20190902 KHK
void 	Cal_SubSensV_Source1_12(int, int); //20190902 KHK
void  	Cal_SubSensV_Source2(int);
void  	Cal_SubSensV_Source2_Default(int); //20190902 KHK
void  	Cal_SubSensV_Source2_12(int, int); //20190902 KHK
void  	Cal_SubSensV_Ch(int);
void  	Cal_SubSensV_Ch_Default(int); //20190902 KHK
void  	Cal_SubSensV_Ch_12(int, int); //20190902 KHK

float	Convert_Vth_to_Temp(float, int);
float	Convert_Vth_to_Ohm(float, int);

void	SubSensV_process(void);
void 	SubSensV_process_7(int); //20190902 KHK
void 	SubSensV_process_12(int); //20190902 KHK
//void	SubSensV_process_CAN_DAQ(void);	//kjg_180914
#endif
