#ifndef __MESSAGE_STR_H__
#define __MESSAGE_STR_H__

#include "SysDefine.h"
#include "Message_def.h"

typedef struct s_msg_val_tag {
	int					msg;
	int					val[13]; //kjg_180405 4->8 //ksh_241112 8->13
} S_MSG_VAL;

typedef struct s_msg_ch_flag_tag {
	unsigned char		flag[8]; //1bytes = 8bits
	unsigned char		value[8];
} S_MSG_CH_FLAG;

typedef struct s_msg_ch_data_flag_tag { //kjg_110712
	unsigned char		flag[MAX_CH_256];
} S_MSG_CH_DATA_FLAG;

typedef struct s_msg_tag {
	int					write_idx;
	int					read_idx;

	S_MSG_VAL			msg_val[MAX_MSG];
	S_MSG_CH_FLAG		msg_ch_flag[MAX_MSG];
} S_MSG;

//jhkw_220103s
typedef union u_save_msg_sub_code_tag {
	unsigned long		ul_val[1];
	long				l_val[1];
	float				f_val[1];
	short int			si_val[2];
	unsigned char		uc_val[4];
	char				c_val[4];
} U_SAVE_MSG_SUB_CODE;
//jhkw_220103e

typedef struct s_save_msg_ch_data_tag {
	unsigned char		ch;
	unsigned char		state;
	unsigned char		stepType;
	unsigned char		stepMode;

	unsigned char		select;
	//unsigned char		code;
	unsigned short int	code;	//ktg_210807
	unsigned char		stepNo;
	unsigned char		grade;
	unsigned char		reserved4[3];	//ktg_210807

	long				Vsens;
	long				Isens;
	long				charge_AmpareHour;
	long				discharge_AmpareHour;
	long				capacitance;
	long				watt;
	long				charge_WattHour;
	long				discharge_WattHour;
	unsigned long		runTime_day;
	unsigned long		runTime;
	unsigned long		totalRunTime_day;
	unsigned long		totalRunTime;
	long				z;

	unsigned char		reservedCmd;	//0:normal, 1:stop, 2:pause
	unsigned char		external_comm_state;
							//0:normal, 1:error
							//0bit:aux_temp, 1bit:aux_v,
							//2bit:can_master, 3bit:can_slave
	unsigned char		ch_output_state; //kjg_101219
							//0:off, 1:on
							//0bit:key_on, 1bit:charge_on, 2bit:pack_relay_on
	unsigned char		ch_input_state; //kjg_101219

	short int			auxDataCount;
	short int			canReceiveDataCount;

	unsigned long		totalCycle;
	unsigned long		elementCycle;
	unsigned long		accCycle[MAX_ACC_CYCLE];
	unsigned long		multiCycle[MAX_MULTI_CYCLE];

	long				avgV;
	long				avgI;
	long				resultIndex;

	unsigned long		cvTime_day;
	unsigned long		cvTime;
//	unsigned long		ccTime_day;
//	unsigned long		ccTime;

	long				realDate;
	long				realClock;

	long				Vinput; //kjg_101102_s
	long				Vpower;
	long				Vbus; //kjg_101102_e

//	long				integral_AmpareHour;
//	long				integral_WattHour;
	unsigned char		chamber_control; //kjg_101220
	unsigned char		record_index;	//RECORD_T1~T5 1base
	//unsigned char		reserved1[2];
	//unsigned char		can_comm_check; //kjhw_151021
	unsigned char		out_mux_use; //kjhw_151021
	unsigned char		out_mux_backup; //kjhw_151021

	unsigned char		cap_bank; //kjh_191014
	unsigned char		freeze_flag;	//ktg_230728
	unsigned char		cv_flag;		//ktg_230728
	unsigned char		reserved2; //kjh_191014
	//unsigned char		reserved2[3]; //kjh_191014

	long				maxCell;	//jhkw_211014s
	long				minCell;
	short int			maxCell_dataNo;
	short int			minCell_dataNo;
	long				diffCell;
	
	long				minAuxV;
	long				maxAuxV;
	long				minAuxTemp;
	long				maxAuxTemp;
	long				minAuxTH;
	long				maxAuxTH;
	short int			minAuxV_auxChNo;
	short int			maxAuxV_auxChNo;
	short int			minAuxTemp_auxChNo;
	short int			maxAuxTemp_auxChNo;
	short int			minAuxTH_auxChNo;
	short int			maxAuxTH_auxChNo;
	short int			maxFreeze_CANChNo;	//ktg_230822
	short int			maxFreeze_CANTime;	//ktg_230822
	//long				reserved3;
	unsigned long		CycleTime_day;	//ktg_231115
	unsigned long		CycleTime;		//ktg_231115

	U_SAVE_MSG_SUB_CODE	sub_code[4];	//jhkw_220103

	//long				reserved2[6];	//jhkw_211014e
} S_SAVE_MSG_CH_DATA;

typedef struct s_save_msg_aux_data_tag {
	short int			auxChNo;		//1base
	short int			auxType;		//0:temperature, 1:v
	long				val;
} S_SAVE_MSG_AUX_DATA;

typedef union u_save_msg_can_val_tag {
	unsigned long		ul_val[2];
	long				l_val[2];
	float				f_val[2];
	unsigned char		uc_val[8];
	char				c_val[8];
	double				d_val[1];
} U_SAVE_MSG_CAN_VAL;

typedef struct s_save_msg_can_data_tag {
	unsigned char		canType;		//0:unused, 1:master, 2:slave
	unsigned char		data_type;	//0:signed, 1:unsigned, 2:float, 3:string
	short int			function_div;

	U_SAVE_MSG_CAN_VAL	val;
} S_SAVE_MSG_CAN_DATA;

typedef struct s_save_msg_val_tag {
	S_SAVE_MSG_CH_DATA	chData;
	S_SAVE_MSG_AUX_DATA	auxData[MAX_AUX_DATA];
	S_SAVE_MSG_CAN_DATA	canData[MAX_CAN_DATA];
} S_SAVE_MSG_VAL;

typedef struct s_save_msg_tag {
	int					send_flag;
	int					write_idx;
	int					read_idx;
	int					total_count;
	int					count[MAX_SAVE_CH];

	S_SAVE_MSG_VAL		val[MAX_SAVE_MSG];
} S_SAVE_MSG;

typedef struct s_pulse_msg_1_val_tag {
	unsigned char		count_flag;
	unsigned char		reserved1[3];

	int					type;
	long				runTime;
	long				Vsens;
	long				Isens;
	long				totalCycle;
	long				stepNo;
	long				capacity;
	long				wattHour;
} S_PULSE_MSG_1_VAL;

typedef struct s_pulse_msg_1_tag {
	int					send_flag;
	int					total_count;
	int					write_idx[MAX_CH_8];
	int					read_idx[MAX_CH_8];
	int					count[MAX_CH_8];

	S_PULSE_MSG_1_VAL	val[MAX_PULSE_MSG][MAX_CH_8];
} S_PULSE_MSG_1;

#endif
