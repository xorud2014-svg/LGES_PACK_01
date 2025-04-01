#ifndef __FCH_CONTROL_STR_H__
#define __FCH_CONTROL_STR_H__

#include "SysDefine.h"
#include "FCH_Control_def.h"

typedef struct s_fch_val_tag {
	char				firmware_version[12];
	char				firmware_model[12];

	unsigned char		o_val[2];
	unsigned char		i_val[2];

	long				da_val[4];
	long				ad_val[10];
	long				ah;
	long				wh;
	long				avg_v;
	long				avg_i;
	long				max_v;
	long				min_v;
	long				max_i;
	long				min_i;
	long				v_power_bus;
	long				v_output_bus;

	unsigned char		cali_type;
	unsigned char		cali_range;
	unsigned char		cali_mode;
	unsigned char		cali_index;

	long				cali_data;
} S_FCH_VAL;

typedef struct s_fch_data_tag {
	unsigned char		signal[MAX_SIGNAL];

	S_FCH_VAL			ch[MAX_CH_PER_MODULE];
} S_FCH_DATA;

#endif
