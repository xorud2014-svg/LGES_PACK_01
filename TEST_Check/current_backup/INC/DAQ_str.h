#ifndef __DAQ_STR_H__
#define __DAQ_STR_H__

#include "SysDefine.h"
#include "DAQ_def.h"

typedef struct s_daq_misc_tag {
	float		DAQ_AD_A;
	float		DAQ_AD_B;
} S_DAQ_MISC;

typedef struct s_daq_config_tag {
	unsigned char		daq_use;
	unsigned char		daq_type;
	unsigned char		installedDAQ;
	unsigned char		Compare_flag;

	char				version[16];

	unsigned short		installedAuxV;
	unsigned short		installedTH;

	unsigned char		Aux_in_DAQ[MAX_DAQ_BD_NUM];
	unsigned char		Aux_type[MAX_DAQ_BD_NUM];
	unsigned char 		useDampingResistor;	//shhw_2306011
	unsigned char		reserved[3];		//shhw_2306011	
} S_DAQ_CONFIG;
/*
typedef struct s_daq_op_data_tag {
	long		ch_vsens[MAX_DAQ_CH];
} S_DAQ_OP_DATA;
*/
typedef struct s_daq_data_tag {
	S_DAQ_MISC		misc;
	S_DAQ_CONFIG	config;
//	S_DAQ_OP_DATA	op;
} S_DAQ_DATA;
#endif
