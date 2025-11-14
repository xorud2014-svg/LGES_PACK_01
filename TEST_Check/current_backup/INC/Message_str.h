#ifndef __MESSAGE_STR_H__
#define __MESSAGE_STR_H__

#include "SysDefine.h"
#include "Message_def.h"

typedef struct s_msg_val_tag {
	int					msg;
	int					val[8]; //kjg_180405 4->8
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

typedef struct s_save_msg_ch_data_tag {
	unsigned char		ch;
	unsigned char		state;
	unsigned char		stepType;
	unsigned char		stepMode;

	unsigned char		select;
	unsigned char		code;
	//unsigned char		stepNo;	//jhkw_200410
	short int			stepNo;	//jhkw_200410
	unsigned char		grade;
	unsigned char		reseved1[3];	//jhkw_200410

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

	//long				reserved2[6];
	long				maxAuxT; //kjhw_181223
	long				minAuxT; //kjhw_181223
	long				diffAuxT; //kjhw_181223
	long				avgAuxT; //kjhw_181223

	long				SOC;
	//long				reserved2;	//shhw_230605 //shh_check
	//shhw_230605 
	long 				dIsens;				
	long				dMaxAuxV; //dMaxAuxV[2] -> dMaxAuxV				//shhw_230614s
	long				dMinAuxV; //dMinAuxV[2] -> dMinAuxV	
	short int 		dMaxAuxVChNo; //dMaxAuxVChNo[2] -> dMaxAuxVChNo
	short int		dMinAuxVChNo; //dMinAuxVChNo[2] -> dMinAuxVChNo		//shhw_230614e
	/*
	long				dMaxAuxV[2];
	long				dMinAuxV[2];
	short int 		dMaxAuxVChNo[2];
	short int		dMinAuxVChNo[2];
	*/
	//shhw_230605
} S_SAVE_MSG_CH_DATA;

#ifndef __COA__
typedef struct s_save_msg_ch_data2_tag {
	unsigned char		ch;
	unsigned char		state;
	unsigned char		stepType;
	unsigned char		stepMode;

	unsigned char		select;
	unsigned char		code;
	//unsigned char		stepNo;
	short int			stepNo;	//jhkw_200410
	unsigned char		grade;

	long				Vsens;
	long				Isens;
	long				charge_AmpareHour;
	long				discharge_AmpareHour;
	long				capacitance;
	long				watt;
	long				charge_WattHour;
	long				discharge_WattHour;
	unsigned long		runTime;
	unsigned long		totalRunTime;
	long				z;

	unsigned char		reservedCmd;	//0:normal, 1:stop, 2:pause
	unsigned char		save_flag; //kjg_w_f
	unsigned char		scan_ch; //for formation 400A/420A
	//unsigned char		reserved1;	//jhkw_200410

	short int			auxDataCount;
	short int			canReceiveDataCount;

	unsigned long		totalCycle;
	unsigned long		elementCycle;
	unsigned long		accCycle[MAX_ACC_CYCLE];
	unsigned long		multiCycle[MAX_MULTI_CYCLE];

	long				avgV;
	long				avgI;
	long				resultIndex;

	unsigned long		ccTime;
	unsigned long		cvTime;
	long				realDate;
	long				realClock;
	long				temp;
} S_SAVE_MSG_CH_DATA2;
#endif

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
	unsigned int		total_count;
	unsigned int		count[MAX_SAVE_CH];

	S_SAVE_MSG_VAL		val[MAX_SAVE_MSG];
} S_SAVE_MSG;

//jhkw_190830s
#ifdef __10MS__
typedef struct s_save_msg_10ms_tag {
	int					send_flag;
	int					write_idx;
	int					read_idx;
	int					total_count;
	int					count[MAX_SAVE_CH];
	int					count_100;

	S_SAVE_MSG_VAL		val[MAX_SAVE_MSG];
} S_SAVE_MSG_10MS;

typedef struct s_signal_10ms_tag {
	unsigned char		msg_count;
	unsigned char		send_msg_count;
	unsigned char		response_msg_count;
	unsigned char		save_start_flag;
} S_SIGNAL_10MS;
#endif
//jhkw_190830e

#ifndef __COA__
typedef struct s_save_msg_1_val_tag {
	unsigned char		ch;
	unsigned char		state;
	unsigned char		type;
	unsigned char		mode;

	unsigned char		select;
	unsigned char		code;
	//unsigned char		stepNo;
	short int			stepNo;	//jhkw_200410
	unsigned char		grade;

	long				Vsens;
	long				Isens;
	long				capacity;
	long				watt;
	long				wattHour;
	unsigned long		runTime;
	unsigned long		totalRunTime;
	long				z;
	long				temp[3];

	unsigned char		chamber_control;
	unsigned char		record_index;	//RECORD_T1~T5 1base
	//unsigned char		reserved1[2];
	unsigned char		reserved1;	//jhkw_200410
	long				reserved2;

	unsigned char		reservedCmd;	//0:normal, 1:stop, 2:pause
	unsigned char		virRangeReservedNo;	//0:normal, 1:stop, 2:pause
	unsigned short		gotoCycleCount;

	unsigned long		totalCycle;
	unsigned long		currentCycle;
	long				avgV;
	long				avgI;
	long				resultIndex;

//	long				IntegralAmpareHour;
//	long				IngegralWattHour;
//	long				ChargeAmpareHour;
//	long				ChargeWattHour;
//	long				DischargeAmpareHour;
//	long				DischcargeAmpareHour;
//	long				cvTime;
//	long				reserved1[3];
} S_SAVE_MSG_1_VAL;

typedef struct s_save_msg_1_tag {
	int					send_flag;
	unsigned int		total_count;
	int					write_idx[MAX_CH_PER_MODULE];
	int					read_idx[MAX_CH_PER_MODULE];
	unsigned int		count[MAX_CH_PER_MODULE];

	S_SAVE_MSG_1_VAL	val[MAX_SAVE_MSG][MAX_CH_PER_MODULE];
} S_SAVE_MSG_1;
#endif
#ifndef __COA__
typedef struct s_record_msg_1_val_tag {
	unsigned char		ch;
	unsigned char		state;
	unsigned char		type;
	unsigned char		mode;

	unsigned char		select;
	unsigned char		code;
	//unsigned char		stepNo;
	short int			stepNo;	//jhkw_200410
	unsigned char		grade;

	long				Vsens;
	long				Isens;
	long				capacity;
	long				watt;
	long				wattHour;
	unsigned long		runTime;
	unsigned long		totalRunTime;
	long				z;
	long				temp[3];

	unsigned char		chamber_control;
	unsigned char		record_index;	//RECORD_T1~T5 1base
	//unsigned char		reserved1[2];
	unsigned char		reserved1;	//jhkw_200410
	long				reserved2;

	unsigned char		reservedCmd;	//0:normal, 1:stop, 2:pause
	unsigned char		virRangeReservedNo;	//0:normal, 1:stop, 2:pause
	unsigned short		gotoCycleCount;

	unsigned long		totalCycle;
	unsigned long		currentCycle;
	long				avgV;
	long				avgI;
	long				resultIndex;

//	long				IntegralAmpareHour;
//	long				IngegralWattHour;
//	long				ChargeAmpareHour;
//	long				ChargeWattHour;
//	long				DischargeAmpareHour;
//	long				DischcargeAmpareHour;
//	long				cvTime;
//	long				reserved1[3];
} S_RECORD_MSG_1_VAL;

typedef struct s_record_msg_1_tag {
	int					send_flag;
	unsigned int		total_count;
	int					write_idx[MAX_CH_PER_MODULE];
	int					read_idx[MAX_CH_PER_MODULE];
	unsigned int		count[MAX_CH_PER_MODULE];

	S_RECORD_MSG_1_VAL	val[MAX_RECORD_MSG][MAX_CH_PER_MODULE];
} S_RECORD_MSG_1;
#endif
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
	unsigned int		total_count;
	int					write_idx[MAX_CH_8];
	int					read_idx[MAX_CH_8];
	unsigned int		count[MAX_CH_8];

	S_PULSE_MSG_1_VAL	val[MAX_PULSE_MSG][MAX_CH_8];
} S_PULSE_MSG_1;
#ifndef __COA__
typedef struct s_formation_save_msg_val_tag {
	unsigned char		state;
	unsigned char		type;
	unsigned char		mode;
	unsigned char		attribute;

	S_SAVE_MSG_CH_DATA2	chData[MAX_CH_256];
} S_F_SAVE_MSG_VAL;

typedef struct s_formation_save_msg_tag {
	int					send_flag;
	int					write_idx;
	int					read_idx;
	unsigned int		count;
	S_F_SAVE_MSG_VAL	val[MAX_F_SAVE_MSG];
} S_F_SAVE_MSG;

typedef struct s_formation_dcir_save_data_tag {
	unsigned long		time;
	long				volt;
	long				curr;
} S_F_DCIR_SAVE_DATA;

typedef struct s_formation_dcir_save_msg_val_tag {
	unsigned char		state;
	unsigned char		type;
	unsigned char		mode;
	unsigned char		attribute;

	S_F_DCIR_SAVE_DATA	chData[MAX_CH_PER_MODULE][20];
} S_F_DCIR_SAVE_MSG_VAL;

typedef struct s_formation_dcir_save_msg_tag {
	int					send_flag;
	int					write_idx;
	int					read_idx;
	unsigned int		count;

	S_F_DCIR_SAVE_MSG_VAL	val[MAX_F_DCIR_SAVE_MSG];
} S_F_DCIR_SAVE_MSG;
#endif
#endif
