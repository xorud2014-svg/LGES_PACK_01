#ifndef __SUBSENSV_STR_H__
#define __SUBSENSV_STR_H__

#include "SysDefine.h"
#include "SubSensV_def.h"
#include "DAQ_def.h"

typedef union u_data_tag {
	short int			val;
	unsigned char		byte[2];
} U_DATA;

typedef struct s_sub_sens_v_org1_tag {
	unsigned char		zero_val[SUB_SENS_V_ZERO_VAL];
	U_DATA				ref_val[MAX_SUB_SENS_V_REF_INDEX]
						[MAX_SUB_SENS_V_AD_SLOT][MAX_SUB_SENS_V_ADC_INDEX_1];
	U_DATA				sub_val[MAX_SUB_SENS_V_CH_INDEX]
						[MAX_SUB_SENS_V_AD_SLOT][MAX_SUB_SENS_V_ADC_INDEX_1];
	unsigned char		equal_val[2];
} S_SUB_SENS_V_ORG1;

typedef struct s_sub_sens_v_org2_tag {
	unsigned char		zero_val[SUB_SENS_V_ZERO_VAL];
	U_DATA				ref_val[MAX_SUB_SENS_V_REF_INDEX]
						[MAX_SUB_SENS_V_AD_SLOT][MAX_SUB_SENS_V_ADC_INDEX_2];
	U_DATA				sub_val[MAX_SUB_SENS_V_CH_INDEX]
						[MAX_SUB_SENS_V_AD_SLOT][MAX_SUB_SENS_V_ADC_INDEX_2];
	unsigned char		equal_val[2];
} S_SUB_SENS_V_ORG2;

typedef struct s_sub_sens_v_org_tag {
	unsigned char		zero_val[SUB_SENS_V_ZERO_VAL];
	U_DATA				ref_val[MAX_SUB_SENS_V_REF_INDEX]
						[MAX_SUB_SENS_V_AD_SLOT][MAX_SUB_SENS_V_ADC_INDEX];
	U_DATA				sub_val[MAX_SUB_SENS_V_CH_INDEX]
						[MAX_SUB_SENS_V_AD_SLOT][MAX_SUB_SENS_V_ADC_INDEX];
	unsigned char		equal_val[2];
} S_SUB_SENS_V_ORG;

typedef struct s_sub_sens_v_source_tag {
	short int			tmpV[MAX_SUB_SENS_V_REF_INDEX]
							[MAX_SUB_SENS_V_SOURCE_SENS_COUNT];
	short int			tmpV_f1[MAX_SUB_SENS_V_REF_INDEX]
							[MAX_SUB_SENS_V_SOURCE_SENS_COUNT];
	long				sensV[MAX_SUB_SENS_V_REF_INDEX];
	long				calV[MAX_SUB_SENS_V_REF_INDEX];
	double				V_AD_a;
	double				V_AD_b;
	double				V_AD_a_N;
	double				V_AD_b_N;
} S_SUB_SENS_V_SOURCE;

typedef struct s_sub_sens_v_source_div_tag {
	S_SUB_SENS_V_SOURCE	source[MAX_SUB_SENS_V_AD_SLOT]
							[MAX_SUB_SENS_V_ADC_INDEX];
} S_SUB_SENS_V_SOURCE_DIV;

typedef struct s_sub_sens_v_ch_tag {
	long				tmpV[MAX_SUB_SENS_V_FILTER_AD_COUNT];
	long				tmpSensV;
	long				sensV;
	unsigned int		fault_count;	//jhkw_151028
	long				pre_sensV;		//shhw_230605
} S_SUB_SENS_V_CH;

typedef struct s_sub_sens_v_tag {
	unsigned char		phase;
	unsigned char		delay_count;
	unsigned char		check_count;	//kjg_150225
	unsigned char		reserved1;

	unsigned char		signal[MAX_SIGNAL];

	S_SUB_SENS_V_ORG	org_data[MAX_DAQ_BD_NUM];

	S_SUB_SENS_V_SOURCE_DIV	source_div[MAX_DAQ_BD_NUM];

	S_SUB_SENS_V_CH		ch[MAX_SUB_SENS_V_DATA];
} S_SUB_SENS_V;

#endif
