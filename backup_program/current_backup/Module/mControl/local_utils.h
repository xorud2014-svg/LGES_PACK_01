#ifndef __LOCAL_UTILS_H__
#define __LOCAL_UTILS_H__

#include "common_utils.h"

//void	Initialize(void);
int		Initialize(void);		//csk_240301
void	Init_SystemMemory_1(void);

void	Init_DMA_DAQ(void);
void	Close_DMA_DAQ(void);
void	Init_RT_CAN(void);
void	Close_RT_CAN(void);
void	Init_RT_COM(void);
void	Close_RT_COM(void);

void	timer_1sec_increment(void);
void	sub_sens_v_count_increment(void);
int		ch_sens_count_increment(int);
int		bd_sens_count_increment(void);
int		ch_sens_count_increment_2(int); //kjg_logic_type_140325
void	bd_sens_count_increment_2(void); //kjg_logic_type_140325
//void	module_runningTime(int, long, long);
void	module_runningTime(int);		//csk_240301
void	module_runningTime1(int, long, long, long, long, long, long, long); //kjhw_140520
void	module_runningTime2(int);		//csk_240301
void	module_runningTime3(void);		//csk_240301

void	Sync_RTC(void);
void 	Sync_RTC2(void); //shh_250122 SWEGPROD-1488
void 	getTime_RTC(void); //shh_250122 SWEGPROD-1488
void 	getTime_RTC2(void); //shh_250122 SWEGPROD-1488
void	slot_tic_timer(void);	//KHK 20191121
void	slot_tic_timer2(void);	//shh_250122 SWEGPROD-1488

void	sens_count_increment_group(void);
void	sens_count_increment_bd(int);

void	time_cal_increment(unsigned long, unsigned long, unsigned long);
void	time_cal_decrement(unsigned long, unsigned long, unsigned long);
int		time_cal_compare(int, unsigned long, unsigned long, unsigned long, unsigned long);

void	Aux_Voltage_Temp_Thermistor_compare(int); //kjhw_170820
void	CAN_function_compare(int);	//ktg_2106

#endif
