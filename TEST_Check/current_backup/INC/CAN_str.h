#ifndef __CAN_STR_H__
#define __CAN_STR_H__

#include "SysDefine.h"
#include "CAN_def.h"

/*kjg_180405 typedef union u_can_val_tag {
	unsigned long		ul_val[2];
	long				l_val[2];
	float				f_val[2];
	unsigned char		uc_val[8];
	char				c_val[8];
	double				d_val[1];
} U_CAN_VAL;
*/
#ifdef __CAN_FD__ //jhkw_190714s
typedef union u_can_val_tag { //kjg_180405
	unsigned long		ul_val[16];
	long				l_val[16];
	float				f_val[16];
	unsigned char		uc_val[64];
	char				c_val[64];
	double				d_val[8];
} U_CAN_VAL;
#else //CAN_2P0B
typedef union u_can_val_tag { //kjg_180405
	unsigned long		ul_val[2];
	long				l_val[2];
	float				f_val[2];
	unsigned char		uc_val[8];
	char				c_val[8];
	double				d_val[1];
} U_CAN_VAL;
#endif //jhkw_190714e

typedef struct s_can_config_tag {
	unsigned char		CmdSendLog;
	unsigned char		CmdRcvLog;
	unsigned char		CmdSendLog_Hex;
	unsigned char		CmdRcvLog_Hex;

	unsigned char		CommCheckLog;
	unsigned char		commType;
	unsigned char		rx_file_format;
	unsigned char		tx_file_format;

	unsigned char		reserved1[4];

	long				reserved2[6];
} S_CAN_CONFIG;

typedef struct s_can_receive_common_data_tag {
	unsigned char		can_baudrate; //0:125K, 1:250K, 2:500K(default), 3:1M
	unsigned char		extended_id; //0:unused(default), 1:used
	unsigned char		bms_type;
						//0:bms_none(default), 1:pack_relay, 2:sbl_bms, 3:hmc
	unsigned char		sjw; //0:default

	long				controller_canID;
	long				mask[2];
	long				filter[6];

	//kjg_180405_s long				reserved1[3];
	unsigned char		can_fd_flag; //0:can_2.0B(default), 1:can_fd
	unsigned char		can_datarate;
						//0:500K, 1:833K, 2:1M, 3:1M5, 4:2M(default), 5:3M, 6:4M
	unsigned char		terminal_r; //0:open, 1:120ohm(defualt)
	unsigned char		crc_type; //0:non_iso_crc, 1:iso_crc(default)

	long				reserved2[2]; //kjg_180405_e

	short int			func_div[MAX_CAN_FUNCTION]; //kjg_101220_s
	unsigned char		compare_type[MAX_CAN_FUNCTION];
	unsigned char		data_type[MAX_CAN_FUNCTION];
	float				value[MAX_CAN_FUNCTION]; //kjg_101220_e
} S_CAN_RECEIVE_COMMON_DATA;

#if defined __COA_VER_100B__ || defined __COA_VER_100B2__
typedef struct s_can_receive_normal_data_tag {
	unsigned char		canType;		//0:unused, 1:master, 2:slave
	unsigned char		byte_order;		//0:intel, 1:motolora
	unsigned char		data_type;	//0:unsigned, 1:signed, 2:float, 3:string
	unsigned char		reserved1; //data_attribute; kjg_090202 0x1007

	float				factor;
	short int			startBit;
	short int			bitCount;

	long				canID;			//hex value
	char				name[MAX_CAN_NAME_SIZE];
	float				fault_upper;
	float				fault_lower;
	float				end_upper;
	float				end_lower;

	float				default_value;
	float				offset;
	
	short int			function_div;
	short int			function_div2;
	short int			function_div3;
	short int			reserved2;

	long				sent_period;
	long				reserved3[2];
} S_CAN_RECEIVE_NORMAL_DATA;
#else //COA_VER_100C~
typedef struct s_can_receive_normal_data_tag {
	unsigned char		canType;		//0:unused, 1:master, 2:slave
	unsigned char		byte_order;		//0:intel, 1:motolora
	unsigned char		data_type;	//0:unsigned, 1:signed, 2:float, 3:string
	unsigned char		reserved1; //data_attribute; kjg_090202 0x1007

	float				factor;
	short int			startBit;
	short int			bitCount;

	long				canID;			//hex value
	char				name[MAX_CAN_NAME_SIZE];
	float				fault_upper;
	float				fault_lower;
	float				end_upper;
	float				end_lower;

	float				default_value;
	float				offset;
	
	short int			function_div;
	short int			function_div2;
	short int			function_div3;
	short int			reserved2;

	long				sent_period;
	long				reserved3[2];
	//kjhw_131204s
	short int			startBit2;
	short int			bitCount2;

	unsigned char		byte_order2;	//0:intel, 1:motolora
	unsigned char		data_type2;		//0:unsigned
	unsigned char		reserved4[2];

	float				compare_value;
	//kjhw_131204e
} S_CAN_RECEIVE_NORMAL_DATA;
#endif

typedef struct s_can_receive_set_data_tag {
	S_CAN_RECEIVE_COMMON_DATA	commonData[MAX_CH_8][MAX_CAN_TYPE];
	S_CAN_RECEIVE_NORMAL_DATA	normalData[MAX_CH_8][MAX_CAN_DATA];
} S_CAN_RECEIVE_SET_DATA;

typedef struct s_can_transmit_common_data_tag {
	unsigned char		can_baudrate; //0:125K, 1:250K, 2:500K(default), 3:1M
	unsigned char		extended_id; //0:unused(default), 1:used
	unsigned char		bms_type;
						//0:bms_none(default), 1:pack_relay, 2:sbl_bms, 3:hmc
	unsigned char		sjw; //0:default

	long				controller_canID;

	//kjg_180405_s long				reserved1[2];
	unsigned char		can_fd_flag; //0:can_2.0B(default), 1:can_fd
	unsigned char		can_datarate;
						//0:500K, 1:833K, 2:1M, 3:1M5, 4:2M(default), 5:3M, 6:4M
	unsigned char		terminal_r; //0:open, 1:120ohm(defualt)
	unsigned char		crc_type; //0:non_iso_crc, 1:iso_crc(default)

	long				reserved2; //kjg_180405_e

	short int			func_div[MAX_CAN_FUNCTION]; //kjg_101220
	unsigned char		compare_type[MAX_CAN_FUNCTION];
	unsigned char		data_type[MAX_CAN_FUNCTION];
	float				value[MAX_CAN_FUNCTION];
		//for sbl pack
		//0 : charge_on voltage
		//1 : pack_realy delay_time
		//2 : ignition_min_v
		//3 : ignition_max_v
		//4 : bms restart_time
} S_CAN_TRANSMIT_COMMON_DATA;

typedef struct s_can_transmit_normal_data_tag {
	unsigned char		canType;		//0:unused, 1:master, 2:slave
	unsigned char		byte_order;		//0:intel, 1:motolora
	unsigned char		data_type;	//0:unsigned, 1:signed, 2:float, 3:string
	unsigned char		dlc; //kjg_180405 0, 1~8, 12, 16, 20, 24, 32, 48, 64, default(8)

	float				factor;
	float				default_value;
	short int			startBit;
	short int			bitCount;

	long				canID;			//hex value
	long				send_period;
	char				name[MAX_CAN_NAME_SIZE];

	long				reserved2;
	float				offset;
	
	short int			function_div;
	short int			function_div2;
	short int			function_div3;
	short int			reserved3;

	long				reserved4[2];
} S_CAN_TRANSMIT_NORMAL_DATA;

typedef struct s_can_transmit_set_data_tag {
	S_CAN_TRANSMIT_COMMON_DATA	commonData[MAX_CH_8][MAX_CAN_TYPE];
	S_CAN_TRANSMIT_NORMAL_DATA	normalData[MAX_CH_8][MAX_CAN_DATA];
} S_CAN_TRANSMIT_SET_DATA;

typedef struct s_can_transmit_change_data_tag {
	unsigned char		canType;		//0:unused, 1:master, 2:slave
	unsigned char		reserved1[3];

	float				default_value;

	long				canID;			//hex value

	short int			startBit;
	short int			function_div;
	short int			function_div2;
	short int			function_div3;

	long				reserved2[2];
} S_CAN_TRANSMIT_CHANGE_DATA;

typedef struct s_can_transmit_change_tag {
	S_CAN_TRANSMIT_CHANGE_DATA
		changeData[MAX_CH_8][MAX_CAN_TRANSMIT_CHANGE_DATA];
} S_CAN_TRANSMIT_CHANGE;

#ifdef __CAN_FD__ //jhkw_190714s
typedef struct rt_can_msg_tag {
	unsigned int		id;

	unsigned char		type;
	unsigned char		length;
	unsigned char		can_fd_flag; //0:can_2.0B(default), 1:can_fd
	unsigned char		reserved1;

	unsigned char		data[64]; //kjg_180405 8->64

	long long			time; //kjg_120619
	unsigned int		timeStamp; //kjg_180405
} S_RT_CAN_MSG;
#else //CAN_2P0B
typedef struct rt_can_msg_tag {
	unsigned int		id;
	unsigned char		type;
	unsigned char		length;
	unsigned char		reserved1[2];
	unsigned char		data[8];
	long long			time; //kjg_120619
} S_RT_CAN_MSG;
#endif //jhkw_190714e

typedef struct bms_uds_data_tag {
	unsigned short int	rx_message_count;
	unsigned short int	rx_dtc_count;

	unsigned char		cell_v1[512];
	int					cell_v1_index;
	unsigned char		cell_v2[512];
	int					cell_v2_index;
	unsigned char		busbar_v[512];
	int					busbar_v_index;
	unsigned char		temp[512];
	int					temp_index;
	unsigned char		battery_soc[100]; //kjhw_141110
	int					battery_soc_index; //kjhw_141110
} S_BMS_UDS_DATA;

typedef struct s_can_tag {
	S_CAN_CONFIG		config;
	
	float				percent_input_v[MAX_CAN_PORT]; //kjg_120619_s

	long long			rx_time_old[MAX_CAN_PORT];
	long long			rx_time[MAX_CAN_PORT];
	long long			rx_time_max[MAX_CAN_PORT];
	long long			rx_time_min[MAX_CAN_PORT]; //kjg_120619_e

	unsigned char		abort_transmission[MAX_CAN_PORT];
	unsigned char		signal[MAX_CAN_PORT][MAX_SIGNAL];

	long				receive_time[MAX_CAN_PORT];
	long				transmit_time[MAX_CAN_PORT];

	long				tmp_val; //kjg_w

	unsigned short int	uds_rx_message_count;
	unsigned short int	uds_rx_dtc_count;
	long				reserved1[4];

	unsigned long		can_timer_1sec[MAX_CAN_PORT]; //kjhw_170303
	unsigned long		can_timer_1min[MAX_CAN_PORT]; //kjhw_170303

	unsigned char		can_timer_1sec_count[MAX_CAN_PORT]; //kjhw_170303
	unsigned char		can_timer_1min_count[MAX_CAN_PORT]; //kjhw_170303
	unsigned char		can_timer_1min_count2[MAX_CAN_PORT]; //kjhw_170303
	unsigned char		can_timer_1000ms_count[MAX_CAN_PORT]; //kjhw_170303

	char				can_flash_file_name[128]; //kjg_110906_s
	unsigned char		can_flash_file[MAX_CAN_FLASH_FILE_SIZE];
	char				tmp_can_flash_file_name[128];
	unsigned char		tmp_can_flash_file[MAX_CAN_FLASH_FILE_SIZE];
	int					can_flash_data_total_block;
	int					can_flash_data_block_point[4];
	int					can_flash_data_block_size[4];
	unsigned int		block_count;
	int					block_point;
	unsigned int		uds_ff_count;
	int					uds_tx_size;
	int					uds_tx_block_size; //kjg_110906_e

	U_CAN_VAL			uds_rx[MAX_CAN_PORT]; //kjg_111120
	U_CAN_VAL			uds_tx[MAX_CAN_PORT]; //kjg_111120

	S_BMS_UDS_DATA		bms_uds[MAX_CAN_PORT]; //kjg_120425

	U_CAN_VAL			frame_number_rx[MAX_CAN_PORT]; //kjhw_130220
} S_CAN;

#endif
