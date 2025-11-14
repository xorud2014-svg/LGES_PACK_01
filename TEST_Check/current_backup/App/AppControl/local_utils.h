#ifndef __LOCAL_UTILS_H__
#define __LOCAL_UTILS_H__

#include "common_utils.h"

int		SystemLoader(int, char *[]);
int		Initialize(int);

int		Init_SystemData(void);
void    Delete_SystemMemory(void); //shhw_250409 
int		Read_SystemMemory(char *);
void	Init_SystemMemory_1(void);
void	Init_SystemMemory_2(void);
void	Save_SystemMemory(void);

int		Load_Process_All(void);
int		Load_Process(char *, char *, int);
void	Close_Process_All(void);
void	Close_Process(char *, int, int);
void	Close_Process_2(char *, char *, int, int);

void	Kill_Process(char *);
void	Kill_Process_2(char *, char *);
void 	Check_Process(void);
int		DieCheck_Process(char *);
int		DieCheck_Process_2(char *, char *);

int		Read_ConfigFiles(int);
int		Read_Log_Config(void);
int 	Read_AppControl_Config(int);
int 	Read_mControl_Config(void);
int		Read_Mux_Scan(void);
int		Read_Load_Process_Config(void);

int		Read_Addr_Map_AIO(void);
int		Read_DIO_Config(void);
int		Read_II_Function_Set(void);
int		Read_IO_Function_Set(void);
int		Read_MI_Function_Set(void);
int		Read_MO_Function_Set(void);
int		Read_CI_Function_Set(void);
int		Read_CO_Function_Set(void);
int		Read_EI_Function_Set(void);
int		Read_EO_Function_Set(void);

int		Read_Calibration_Data(void);
int		Read_Bd_CaliData(int);
int		Read_Cali_Set_Main(void); //kjg_110713_s
int		Read_Cali_Set_Ch(void);
int		Read_Main_CaliData(int, int);
int		Write_Main_CaliData(int, int);
int		Read_Ch_CaliData(int, int);
int		Write_Ch_CaliData(int, int);
int		Read_Ch_Check_CaliData(int, int);
int		Write_Ch_Check_CaliData(int, int);
int		Read_MainAD_V_Offset(void); //kjg_110713_e

int		Read_CellArray_A(void);
int 	Read_Daq_Map(void); //jhkw_201117

int 	Create_BdCaliData_Org(int);

int		Read_ChAttribute(void);
int		Write_ChAttribute(void);
int		Write_ChAttribute_1(void); //kjhw_180219

int		Read_AuxSetData(void);
int		Write_AuxSetData(void);

int		Read_CanConfig(void);
int		Read_CanReceiveSetData(void);
int		Write_CanReceiveSetData(void);
int		Read_CanTransmitSetData(void);
int		Write_CanTransmitSetData(void);
void	Copy_Th_Table(int);	//jhkw_191108
void	Copy_Measure_cali(int);	//jhkw_200317

void	Sync_Time(void);

int		Read_COM_Config(void);
int		Read_Measure_Cali_1(void);
int		Read_Measure_Cali_2(int);	//jhkw_200317

int		Read_Th_Table(void);
int		Read_Th_Table_Type(int, int);

int		Read_CanFlashFile(void);
int		Write_VBF_File(char *, char *);

int		Read_DAQ_Config(void); //20190902 KHK
//kjg_d
//int		Read_KJG_Test_1(void);
//int		Write_KJG_Test_1(void);
//int		Read_KJG_Test_2(void);
//int		Write_KJG_Test_2(void);

/*kjg_170810 int		read_test_cond_step_file_coa(int);
int		read_test_cond_time_step_file_coa(int); //kjh_160418

int		convert_test_cond_coa(int);
int		convert_test_cond_time_coa(int);	//kjh_160418
void	convert_test_cond_common_aux_coa(int);
void	convert_test_cond_common_can_coa(int);

void	convert_test_cond_cycle_coa(int, int, long, long);
void	convert_test_cond_loop_coa(int, int, long, long);
int		convert_test_cond_step_default_coa(int, int, long, long);

void	convert_test_cond_step_aux_coa(int, int);
void	convert_test_cond_step_can_coa(int, int);

void	convert_test_cond_ocv_rest_coa(int, int, long, long);
void	convert_test_cond_charge_discharge_z_coa(int, int, long, long);
int		convert_test_cond_pattern_coa(int, int, long, long);
void	convert_test_cond_external_can_coa(int, int, long, long);
int		convert_test_cond_usermap_coa(int, int, long, long); //kjhw_140828

void	convert_test_cond_cable_check_coa(int);*/
#endif

