#ifndef __DATASTORE_COB_H__
#define __DATASTORE_COB_H__

#include "SysDefine.h"
#include "Message_str.h"
#include "AppControl_str.h"
//#include "COA_Client_str.h"
#include "COB_Client_str.h"
//#include "COC_Client_str.h"
#include "DataSave_str.h"
#include "ModuleControl_str.h"
#include "InOutControl_str.h"
#include "JigControl_str.h"
#include "SubSensV_str.h"
#include "CAN_str.h" 
#include "COM_str.h"
#include "PLC_def.h"
//#include "FCH_Control_str.h"
//#include "PWM3_Control_str.h"
//#include "TimeSchedule_str.h" //kjh_160418
#include "datastore_common.h"
#include "DAQ_str.h"

typedef struct s_cali_board_data_tag { //kjg_110712
	unsigned char		caliFlag[MAX_RANGE];

	char				version_name[P2_CALI_VER_NAME_SIZE];
	char				main_bd_serial[P2_MAIN_BD_SERIAL_SIZE];
	char				cali_date[P2_CALI_DATE_SIZE];

	int					DA_Cali_P_PointNum[MAX_TYPE][MAX_RANGE];
	int					DA_Cali_D_PointNum[MAX_TYPE][MAX_RANGE];
	int					DA_Cali_C_PointNum[MAX_TYPE][MAX_RANGE];
	long				DA_Cali_P_Cmd[MAX_TYPE][MAX_RANGE][MAX_F_CALI_POINT];
	int					DA_Cali_N_PointNum[MAX_TYPE][MAX_RANGE];
	long				DA_Cali_N_Cmd[MAX_TYPE][MAX_RANGE][MAX_F_CALI_POINT];

	double				DA_A_P[MAX_TYPE][MAX_RANGE][MAX_F_CALI_POINT];
	double				DA_B_P[MAX_TYPE][MAX_RANGE][MAX_F_CALI_POINT];
	double				DA_A_N[MAX_TYPE][MAX_RANGE][MAX_F_CALI_POINT];
	double				DA_B_N[MAX_TYPE][MAX_RANGE][MAX_F_CALI_POINT];

	long				ref_AD[MAX_TYPE][3];
	long				ref_AD2_V[2];
	long				ref_AD2_I[2];
} S_CALI_BD_DATA;

typedef struct s_cali_channel_data_tag {
	unsigned char		caliFlag[MAX_TYPE][MAX_RANGE];

	int					DA_Cali_P_PointNum[MAX_TYPE][MAX_RANGE];
	long				DA_Cali_P_Cmd[MAX_TYPE][MAX_RANGE][MAX_F_CALI_POINT];
	int					DA_Cali_N_PointNum[MAX_TYPE][MAX_RANGE];
	long				DA_Cali_N_Cmd[MAX_TYPE][MAX_RANGE][MAX_F_CALI_POINT];
	int					DA_Cali_D_PointNum[MAX_TYPE][MAX_RANGE];
	int					DA_Cali_C_PointNum[MAX_TYPE][MAX_RANGE];

	long				meter_value_P[MAX_TYPE][MAX_RANGE][MAX_F_CALI_POINT];
	long				meter_value_N[MAX_TYPE][MAX_RANGE][MAX_F_CALI_POINT];

	long				ad_value_P[MAX_TYPE][MAX_RANGE][MAX_F_CALI_POINT];
	long				ad_value_N[MAX_TYPE][MAX_RANGE][MAX_F_CALI_POINT];

	double				AUX_DA_A_P[MAX_TYPE][MAX_RANGE][MAX_F_CALI_POINT];
	double				AUX_DA_B_P[MAX_TYPE][MAX_RANGE][MAX_F_CALI_POINT];
	double				AUX_DA_A_N[MAX_TYPE][MAX_RANGE][MAX_F_CALI_POINT];
	double				AUX_DA_B_N[MAX_TYPE][MAX_RANGE][MAX_F_CALI_POINT];

	double				AD_A_P[MAX_TYPE][MAX_RANGE][MAX_F_CALI_POINT];
	double				AD_B_P[MAX_TYPE][MAX_RANGE][MAX_F_CALI_POINT];
	double				AD_A_N[MAX_TYPE][MAX_RANGE][MAX_F_CALI_POINT];
	double				AD_B_N[MAX_TYPE][MAX_RANGE][MAX_F_CALI_POINT];
	double				DA_A_P[MAX_TYPE][MAX_RANGE][MAX_F_CALI_POINT];
	double				DA_B_P[MAX_TYPE][MAX_RANGE][MAX_F_CALI_POINT];
	double				DA_A_N[MAX_TYPE][MAX_RANGE][MAX_F_CALI_POINT];
	double				DA_B_N[MAX_TYPE][MAX_RANGE][MAX_F_CALI_POINT];
} S_CALI_CH_DATA;

typedef struct s_f_calibration_tag {
	S_CALI_BD_DATA		tmp_bData[MAX_BD_16];
	S_CALI_CH_DATA		tmp_cData[MAX_BD_16][MAX_CH_PER_BD_64];

	S_CALI_BD_DATA		bData[MAX_BD_16];
	S_CALI_CH_DATA		cData[MAX_BD_16][MAX_CH_PER_BD_64];
} S_F_CALIBRATION;

typedef struct s_f_cali_point_tag {
	int					set_point_num[MAX_RANGE];
	long				set_point[MAX_RANGE][MAX_F_CALI_POINT];

	int					check_point_num[MAX_RANGE];
	long				check_point[MAX_RANGE][MAX_F_CALI_CHECK_POINT];
} S_F_CALI_POINT;

typedef struct s_cali_set_data_tag {
	S_F_CALI_POINT		main_dav;
	S_F_CALI_POINT		main_dai;

	S_F_CALI_POINT		ch_v;
	S_F_CALI_POINT		ch_i;
} S_CALI_SET_DATA;

typedef struct s_system_data_tag {
	S_MSG				msg[MAX_MSG_RING];
	S_SAVE_MSG			save_msg[MAX_SAVE_MSG_RING];

	S_SAVE_MSG_1		save_msg_1[MAX_SAVE_MSG_RING];
	S_RECORD_MSG_1		record_msg_1[MAX_RECORD_MSG_RING];
	S_PULSE_MSG_1		pulse_msg_1[MAX_PULSE_MSG_RING];

	S_F_SAVE_MSG		f_save_msg[MAX_F_SAVE_MSG_RING]; //debug_size_cob
	S_F_DCIR_SAVE_MSG	save_dcir_msg[MAX_GROUP_2];
	S_F_SAVE_MSG		save_real_data_msg[MAX_GROUP_2];

	S_LOGFILE			log[MAX_LOG];

	S_CALIBRATION		cali[MAX_CH_8];

	//kjg_110712_s
	S_F_CALIBRATION		f_cali; //debug_size_cob
	S_CALI_SET_DATA		cali_set_data;
	S_MSG_CH_DATA_FLAG	cali_ch_flag;
	S_MSG_CH_DATA_FLAG	measure_ch_flag;
	S_CH_TEMP			TempArray1[MAX_CH_256];
	S_CH_TEMP			TempArray2[MAX_CH_256]; //kjg_110712_e

    S_TEST_CONDITION	testCond[MAX_TEST_COND_COUNT];
	S_TEST_CONDITION	testCond_update; //kjg_170810

//	S_TIMESCH_CONTROL	TimeSch; //kjh_160418

	S_CH_NUMBER			CellArray1[MAX_CH_256]; //index : monitor_no
	S_CH_NUMBER			CellArray2[MAX_CH_256]; //index : hw_no
	
	S_APP_CONTROL		AppControl;
//	S_COA_CLIENT		COA_Client[MAX_GROUP_1]; //debug_size_coa
	S_COB_CLIENT		COB_Client[MAX_GROUP_2]; //debug_size_cob
//	S_COC_CLIENT		COC_Client[MAX_GROUP_1]; //debug_size_coc
//	S_COD_CLIENT		COD_Client[MAX_GROUP_1]; //kjg_w

	S_DATA_SAVE			DataSave;
		
    S_MODULE_DATA		mData;
	S_GROUP_DATA		gData[MAX_GROUP_2];
	S_BD_DATA			bData[MAX_BD_16];
	S_CH_DATA			cData[MAX_CH_256];

	unsigned short int	local_codeCount_aux[MAX_CH_8][MAX_AUX_DATA][MAX_AUX_FUNCTION]; //kjg_171223

//	S_FCH_DATA			fch;
//	S_PWM3_DATA			pwm3_ch;

	S_LINE_CHECK		lineCheck[MAX_GROUP_2][MAX_CH_256];

	int					COB_save_step[MAX_GROUP_2][MAX_CH_256];
	S_CH_OP_DATA		COB_opSave[MAX_GROUP_2][MAX_CH_256][MAX_P2_STEP]; //debug_size_cob
	long				dcr_check[MAX_GROUP_2][MAX_CH_256];

	S_ADDR_MAP			addr_map;
	S_DIO				dio;
	S_JIG_DATA			jData[MAX_JIG_8];

	S_CH_ATTRIBUTE		ChAttribute[MAX_CH_256];

	S_SUB_SENS_V		SubSensV;
	S_DAQ_DATA			daq;
	int					auxDataCount[MAX_CH_256][MAX_AUX_TYPE];
	S_AUX_SET_DATA		auxSetData[MAX_AUX_DATA];

	S_CAN				CAN;
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

	S_COM				COM;

	S_MEASURE_CALI		measure_cali[2][MAX_SUB_SENS_V_DATA];
	S_TH_TABLE			th_table[MAX_TH_TABLE];

//kjg_d
//	float				kjg_table_1[300][4];
//	float				kjg_table_2[1240810][4];

	unsigned char		test_val_uc[8][MAX_TEST_VALUE];
	int			 		test_val_i[8][MAX_TEST_VALUE];
	long				test_val_l[MAX_TEST_VALUE];
	long long			test_val_ll[MAX_TEST_VALUE];
	float				test_val_f[MAX_TEST_VALUE];

	int					data_10ms_count[2];
	long				data_10ms[2][4][2000];
	long				data_10ms_tmp[2][4][2000];

	long				delayCount;
	long				delayCount_2;
	long				compV[4];
} S_SYSTEM_DATA;
#endif
