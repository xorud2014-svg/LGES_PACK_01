#ifndef __DATASTORE_COA_H__
#define __DATASTORE_COA_H__

#include "SysDefine.h"
#include "Message_str.h"
#include "AppControl_str.h"
#include "COA_Client_str.h"
//#include "COB_Client_str.h"
//#include "COC_Client_str.h"
#include "DataSave_str.h"
#include "ModuleControl_str.h"
#include "InOutControl_str.h"
//#include "JigControl_str.h"
#include "SubSensV_str.h"
#include "CAN_str.h" 
#include "COM_str.h"
#include "PLC_def.h"
//#include "FCH_Control_str.h"
#include "PWM3_Control_str.h"
#include "TimeSchedule_str.h" //kjh_160418
#include "datastore_common.h"
#include "Temp_Client_str.h" //20190905 KHK
#include "Update_Process_str.h"	//ktg_231031

typedef struct s_system_data_tag {
	S_MSG				msg[MAX_MSG_RING];
	S_SAVE_MSG			save_msg[MAX_SAVE_MSG_RING];

//	S_SAVE_MSG_1		save_msg_1[MAX_SAVE_MSG_RING];
//	S_RECORD_MSG_1		record_msg_1[MAX_RECORD_MSG_RING];
	S_PULSE_MSG_1		pulse_msg_1[MAX_PULSE_MSG_RING];

//	S_F_SAVE_MSG		f_save_msg[MAX_F_SAVE_MSG_RING]; //debug_size_cob
//	S_F_DCIR_SAVE_MSG	save_dcir_msg[MAX_GROUP_2];
//	S_F_SAVE_MSG		save_real_data_msg[MAX_GROUP_2];

	S_LOGFILE			log[MAX_LOG];

	S_CALIBRATION		cali[MAX_CH_8];

//	//kjg_110712_s
//	S_F_CALIBRATION		f_cali; //debug_size_cob
//	S_CALI_SET_DATA		cali_set_data;
//	S_MSG_CH_DATA_FLAG	cali_ch_flag;
//	S_MSG_CH_DATA_FLAG	measure_ch_flag;
//	S_CH_TEMP			TempArray1[MAX_CH_256];
//	S_CH_TEMP			TempArray2[MAX_CH_256]; //kjg_110712_e

	S_TEST_CONDITION	testCond[MAX_TEST_COND_COUNT];
	S_TEST_CONDITION	testCond_update; //kjg_170810

	S_TIMESCH_CONTROL	TimeSch; //kjh_160418

	S_CH_NUMBER			CellArray1[MAX_CH_256]; //index : monitor_no
	S_CH_NUMBER			CellArray2[MAX_CH_256]; //index : hw_no
	
	S_APP_CONTROL		AppControl;
	S_COA_CLIENT		COA_Client[MAX_GROUP_1]; //debug_size_coa
//	S_COB_CLIENT		COB_Client[MAX_GROUP_2]; //debug_size_cob
//	S_COC_CLIENT		COC_Client[MAX_GROUP_1]; //debug_size_coc
//	S_COD_CLIENT		COD_Client[MAX_GROUP_1]; //kjg_w
	S_TEMP_CLIENT		Temp_Client[2]; //20190905 KHK
	S_DATA_SAVE			DataSave;
	S_UPDATE_PROCESS	UpdateProcess; //ktg_231031
		
	S_MODULE_DATA		mData;
	S_GROUP_DATA		gData[MAX_GROUP_2];
	S_BD_DATA			bData[MAX_BD_16];
	S_CH_DATA			cData[MAX_CH_256];

	unsigned short int	local_codeCount_aux[MAX_CH_8][MAX_AUX_DATA][MAX_AUX_FUNCTION]; //kjg_171223
	unsigned long		local_codeCount_can_day[MAX_CH_8][MAX_CAN_DATA][MAX_CAN_FUNCTION];
	unsigned long		local_codeCount_can_time[MAX_CH_8][MAX_CAN_DATA][MAX_CAN_FUNCTION];
	//unsigned char		can_check[MAX_CAN_DATA][MAX_LOC_CODE];		//khj_210308	//ktg_210328
	unsigned char		can_check[MAX_CH_8][MAX_CAN_DATA][MAX_LOC_CODE];		//ktg_210809
	//khj_201120
	
//	S_FCH_DATA			fch;
	S_PWM3_DATA			pwm3_ch;

	S_LINE_CHECK		lineCheck[MAX_GROUP_2][MAX_CH_256];

//	int					COB_save_step[MAX_GROUP_2][MAX_CH_256];
//	S_CH_OP_DATA		COB_opSave[MAX_GROUP_2][MAX_CH_256][MAX_P2_STEP]; //debug_size_cob
//	long				dcr_check[MAX_GROUP_2][MAX_CH_256];

	S_ADDR_MAP			addr_map;
	S_DIO				dio;
//	S_JIG_DATA			jData[MAX_JIG_8];

	S_CH_ATTRIBUTE		ChAttribute[MAX_CH_256];

	S_SUB_SENS_V		SubSensV;
	//int					auxDataCount[MAX_CH_256][MAX_AUX_TYPE];
	//int					auxDataCount[MAX_CH_512][MAX_AUX_TYPE]; //kjg_180914
	int					auxDataCount[MAX_CH_4][MAX_AUX_TYPE];	//kjh_191115
	S_AUX_SET_DATA		auxSetData[MAX_AUX_DATA];

	S_CAN				CAN;
	int					canReceiveCellCount[MAX_CH_8][MAX_CAN_TYPE];	//ktg_210706
	int					canReceiveDataCount[MAX_CH_8][MAX_CAN_TYPE];
	S_CAN_RECEIVE_SET_DATA	canReceiveSetData;
	int					canTransmitDataCount[MAX_CH_8][MAX_CAN_TYPE];
	S_CAN_TRANSMIT_SET_DATA	canTransmitSetData;
	long				canTransmitTime[MAX_CH_8][MAX_CAN_DATA];
	S_CAN_TRANSMIT_CHANGE	canTransmitChange;
	U_CAN_VAL			CanData[MAX_CH_8][MAX_CAN_DATA];
	U_CAN_VAL			tmpCanData[MAX_CH_8][MAX_CAN_DATA][12]; //kjg_141114_s
	U_CAN_VAL			tmpCanData_sort[MAX_CH_8][MAX_CAN_DATA][12];
	unsigned char		tmpCanData_sort_flag[MAX_CH_8][MAX_CAN_DATA][12];
						//kjg_171224 9->12
	unsigned char		tmpCanData_sort_count[MAX_CH_8];
	unsigned char		tmpCanData_index[MAX_CH_8][MAX_CAN_DATA];
	unsigned char		tmpCanData_count[MAX_CH_8][MAX_CAN_DATA]; //kjg_141114_e
	long				test_can_val_l[16]; //kjg_240204

	S_COM				COM;
	S_CAN_CELL			CANCell[MAX_CH_8][MAX_CAN_DATA];		//ktg_210706

	S_MEASURE_CALI		measure_cali[2][MAX_SUB_SENS_V_DATA];
	S_TH_TABLE			th_table[MAX_TH_TABLE];
	S_MEASURE_TEMP		measure_cali_temp[2][MAX_SUB_SENS_V_DATA];	//khj_210802
	S_AUX_CH_NUM		aux_ch_num[MAX_SUB_SENS_V_DATA];	//jhkw_201117
	S_HUMIDITY_TABLE	humidity_table[MAX_HUMIDITY_TABLE];		//khj_191205
	S_CAN_TH_TABLE		can_th_table[MAX_CAN_TH_TABLE];	//ktg_220614
	S_PAUSE_CODE_FLAG	pause_code[MAX_CH_4];

//kjg_d
//	float				kjg_table_1[300][4];
//	float				kjg_table_2[1240810][4];

	unsigned char		test_val_uc[8][MAX_TEST_VALUE];
	int			 		test_val_i[8][MAX_TEST_VALUE];
	long				test_val_l1[MAX_TEST_VALUE];		//csk_240301s
	long				test_val_l2[MAX_TEST_VALUE];
	long long			test_val_ll1[MAX_TEST_VALUE];
	long long			test_val_ll2[8][MAX_TEST_VALUE];	//csk_240301e
	float				test_val_f[MAX_TEST_VALUE];
	double				test_val_d[MAX_TEST_VALUE];	//kjh_211021ttt

	int					data_10ms_count[2];
	long				data_10ms[2][4][2000];
	long				data_10ms_tmp[2][4][2000];

	long				delayCount;
	long				delayCount_2;
	long				compV[4];
} S_SYSTEM_DATA;
#endif
