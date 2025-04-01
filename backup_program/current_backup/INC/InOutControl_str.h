#ifndef __INOUTCONTROL_STR_H__
#define __INOUTCONTROL_STR_H__

#include "SysDefine.h"
#include "InOutControl_def.h"

typedef struct s_digital_io_config_tag {
	int					sensCount;
	long				dioDelay;
	long				scan_period;

	long				powerSwitchTimeout;
	long				forcePowerSwitchTimeout;
	long				resetSwitchTimeout;
	long				powerFailTimeout1;
	long				powerFailTimeout2;
	long				upsBatteryFailTimeout;

	unsigned char		dio_Control_Flag;
	unsigned char		watchdogType;
	unsigned char		logic_type; //kjg_logic_type_140324
	unsigned char		reserved1;

	unsigned char		upsBatteryFail_TypeFlag;	//kjh_211021
	unsigned char		acPowerFail_TypeFlag;	//kjh_211021
	unsigned char		reserved2[2];	//kjh_211021
} S_DIO_CONFIG;

typedef struct s_digital_io_function_tag {
	unsigned char		use;
	unsigned char		pn;
	unsigned char		group;
	unsigned char		count; //kjg_101101

	short int			function;	
	short int			ch;
} S_DIO_FUNCTION;

typedef struct s_digital_io_function_set_tag {
	int					in_address[MAX_DIO_IN_BYTES];
	S_DIO_FUNCTION		in_set[MAX_DIO_IN_FUNCTION];

	int					out_address[MAX_DIO_OUT_BYTES];
	S_DIO_FUNCTION		out_set[MAX_DIO_OUT_FUNCTION];
} S_DIO_FUNCTION_SET;

typedef struct s_digital_io_misc_tag {
	long				delayTimer;

	long				powerSwitchTimer;
	long				resetSwitchTimer;
	long				powerFailTimer;
	long				upsBatteryFailTimer;

	long				buzzerOnTimer;
	long				buzzerOffTimer;
	long				tmpBuzzerOnTimer;
	long				tmpBuzzerOffTimer;

	unsigned char		buzzerCount;
	unsigned char		tmpBuzzerCount;
	short int			WDT_OutBit;

	long				delayCount;

	long				ac_power_detect1;
	long				ac_power_detect2;
	long				ac_power_detect3;

	unsigned char		in_buf[2][2000];
	short int			in_buf_index1;
	short int			in_buf_index2;

	long				chamber_power_timer;
	long				chamber_emg_timer;
	long				chamber_door_open_timer;
	long				chamber_over_temp_timer;
	long				chamber_air_timer;
	long				chamber_eocr_timer;
	long				chamber_fan_timer;
	long				chamber_fire_timer;
	long				chamber_smoke_timer;
	long				chamber_ref_timer;

	unsigned char		dio_sig_remote_ps; //kjhw_150725 volvo suzhou
	unsigned char		remote_ps_retry_count; //kjhw_150725 volvo suzhou
	unsigned char		reserved1[2]; //kjhw_150725 volvo suzhou //kjg_171219 1->2
} S_DIO_MISC;

typedef struct s_digital_io_input_tag {
	unsigned char		bytes[MAX_DIO_IN_BYTES];
	int					function[MAX_DIO_IN_FUNCTION];
	unsigned char		CountFlag_H[MAX_DIO_IN_FUNCTION];
	unsigned char		CountFlag_L[MAX_DIO_IN_FUNCTION];
	unsigned char		Flag[MAX_DIO_IN_FUNCTION];
} S_DIO_INPUT;

typedef struct s_digital_io_output_tag {
	int					function[MAX_DIO_OUT_FUNCTION];
	unsigned char		bytes[MAX_DIO_OUT_BYTES];
} S_DIO_OUTPUT;

typedef struct s_digital_in_out_tag {
	S_DIO_CONFIG		config;
	S_DIO_FUNCTION_SET	function_set;
	S_DIO_MISC			misc;
	S_DIO_INPUT			in;
	S_DIO_OUTPUT		out;

	//unsigned char		signal[MAX_SIGNAL];
	unsigned char		signal[MAX_DIO_SIGNAL];		//phb_220929
} S_DIO;

#endif
