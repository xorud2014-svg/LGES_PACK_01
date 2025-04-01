#ifndef __DATASTORE_COMMON_H__
#define __DATASTORE_COMMON_H__

#include "SysDefine.h"
//#include "Message_str.h"
//#include "AppControl_str.h"
//#include "COA_Client_str.h"
//#include "COB_Client_str.h"
//#include "COC_Client_str.h"
//#include "DataSave_str.h"
//#include "ModuleControl_str.h"
//#include "InOutControl_str.h"
//#include "JigControl_str.h"
//#include "SubSensV_str.h"
//#include "CAN_str.h" 
//#include "COM_str.h"
//#include "PLC_def.h"
//#include "FCH_Control_str.h"
//#include "PWM3_Control_str.h"
//#include "TimeSchedule_str.h" //kjh_160418

typedef union u_unsigned_short_tag { //kjg_221106
	unsigned short		val;
	unsigned char		byte[2];
} U_US_DATA;

typedef union u_unsigned_int_tag { //kjg_221106
	unsigned int		val;
	unsigned char		byte[4];
} U_UI_DATA;

typedef struct s_logfile_tag {
	unsigned char		LogFlag;
	unsigned char		reserved1[3];

	char				LogPath[256];
	char				LogFile[256];
	char				OpenLogFile[256];
} S_LOGFILE;

typedef struct s_cali_point_tag {
	unsigned char		setPointNum;
    unsigned char		checkPointNum;
    unsigned char		reserved1[2];
	long				setPoint[MAX_CALI_POINT];
	long				checkPoint[MAX_CALI_POINT];
} S_CALI_POINT;

typedef struct s_cali_tmp_cond_tag {
	int					type;
	int					range;
	int					mode;
	int					count; //kjhw_120506 Vref x 2
	S_CALI_POINT		point;
} S_CALI_TMP_COND;

typedef struct s_cali_tmp_data_tag {
	unsigned char		caliFlag;
	unsigned char		reserved1[3];
	long				set_ad[MAX_CALI_POINT];
	long				set_meter[MAX_CALI_POINT];
	long				check_ad[MAX_CALI_POINT];
	long				check_meter[MAX_CALI_POINT];
	double				DA_A[MAX_CALI_POINT-1];
	double				DA_B[MAX_CALI_POINT-1];
	double				AD_A[MAX_CALI_POINT-1];
	double				AD_B[MAX_CALI_POINT-1];
	double				AD_Ratio[2];
} S_CALI_TMP_DATA;

typedef struct s_cali_data_tag {
	S_CALI_POINT		point;
	long				set_ad[MAX_CALI_POINT];
	long				set_meter[MAX_CALI_POINT];
	long				check_ad[MAX_CALI_POINT];
	long				check_meter[MAX_CALI_POINT];
	double				DA_A[MAX_CALI_POINT-1];
	double				DA_B[MAX_CALI_POINT-1];
	double				AD_A[MAX_CALI_POINT-1];
	double				AD_B[MAX_CALI_POINT-1];
	double				AD_Ratio[2];
} S_CALI_DATA;

typedef struct s_cali_sort_point_tag {	//ktg_230125
	unsigned char		Sort_SetNum[MAX_CALI_POINT];
	unsigned char		Sort_CheckNum[MAX_CALI_POINT];
	unsigned char		Index_SetNum[MAX_CALI_POINT];
} S_CALI_SORT_POINT;

typedef struct s_calibration_tag {
	int					type;
	int					range;
	int					mode;
	int					count; //kjhw_120504 Vref x 2
	double				orgAD[MAX_TYPE];
	S_CALI_TMP_COND		tmpCond[MAX_TYPE][MAX_RANGE];
	S_CALI_TMP_DATA		tmpData[MAX_TYPE][MAX_RANGE];
	S_CALI_DATA			data[MAX_TYPE][MAX_RANGE];
	S_CALI_SORT_POINT	Sort_Point[MAX_TYPE][MAX_RANGE];	//ktg_230125
} S_CALIBRATION;

typedef struct s_ch_number_tag {
	int					number1;
	int					number2;
	int					bd;
	int					ch;
} S_CH_NUMBER;

//jhkw_201117s
typedef struct s_aux_ch_num_tag {
	int                 daq_ch;
	int                 type;
	int                 monitor_ch;
} S_AUX_CH_NUM;
//jhkw_201117e

typedef struct s_address_map_tag {
	short int			BD_BASE_ADDR;
	unsigned char		BD_STEP;
	unsigned char		BD_ADDR_DIV;

	unsigned char		MUX[MAX_CH_8];

	unsigned char		DAV_SYNC[MAX_CH_8];
	unsigned char		DAV_DATA_H[MAX_CH_8];
	unsigned char		DAV_DATA_L[MAX_CH_8];

	unsigned char		DAI_SYNC[MAX_CH_8];
	unsigned char		DAI_DATA_H[MAX_CH_8];
	unsigned char		DAI_DATA_L[MAX_CH_8];

	unsigned char		ADV_SYNC[MAX_CH_8];
	unsigned char		ADV_DATA_H[MAX_CH_8];
	unsigned char		ADV_DATA_L[MAX_CH_8];

	unsigned char		ADI_SYNC[MAX_CH_8];
	unsigned char		ADI_DATA_H[MAX_CH_8];
	unsigned char		ADI_DATA_L[MAX_CH_8];
} S_ADDR_MAP;

typedef struct s_measure_cali_tag {
	float				factor;
	float				offset;
} S_MEASURE_CALI;

typedef struct s_measure_cali_temp_tag {			//khj_210802
	long				temp[5];
	float				factor[4];
	float				offset[4];
} S_MEASURE_TEMP;

typedef struct s_line_check_tag {
	unsigned char		result_code;
	unsigned char		path_val_p;
	unsigned char		path_val_n;
	unsigned char		save_index;

	unsigned char		save_count_end;
	unsigned char		first_compare;
	unsigned char		reserved1[2];

	long				val[5][10];
	long				avg_val[5];
} S_LINE_CHECK;
#endif
