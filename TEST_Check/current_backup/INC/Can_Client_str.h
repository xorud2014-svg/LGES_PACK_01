#ifndef __CAN_CLIENT_STR_H__
#define __CAN_CLIENT_STR_H__

#include "SysDefine.h"
#include "P1_Client_str.h"
#include "P1_Client_def.h"
#include "Can_Client_def.h"

// HOST <=> CLIENT Command Format define
typedef struct s_can_client_cmd_header_tag {
	long		cmd_id;
	long		ch_flag;
	short int	msg_count;
	short int	can_status;
	long		body_size;
} S_CAN_CLIENT_CMD_HEADER;

typedef struct s_can_client_receive_common_data_tag {
	unsigned char				baudrate;
	unsigned char				extended_id;
	unsigned char				bms_type;
	unsigned char				sjw;
	unsigned char				fd_flag;
	unsigned char				can_datarate;
	unsigned char				terminal_r;
	unsigned char				crc_type;
} S_CAN_CLIENT_RECEIVE_COMMON_DATA;

typedef struct s_can_client_receive_nomal_data_tag {
	long						canID;
} S_CAN_CLIENT_RECEIVE_NORMAL_DATA;

typedef struct s_can_client_send_receive_set_data_tag {
	S_CAN_CLIENT_CMD_HEADER				header;
	S_CAN_CLIENT_RECEIVE_COMMON_DATA	commonData[MAX_CAN_TYPE];
	S_CAN_CLIENT_RECEIVE_NORMAL_DATA	normalData[MAX_RX_LTC_DATA];
} S_CAN_CLIENT_SEND_RECEIVE_SET_DATA;

typedef struct s_can_client_transmit_common_data_tag {
	unsigned char				baudrate;
	unsigned char				extended_id;
	unsigned char				bms_type;
	unsigned char				sjw;
	unsigned char				fd_flag;
	unsigned char				can_datarate;
	unsigned char				terminal_r;
	unsigned char				crc_type;
} S_CAN_CLIENT_TRANSMIT_COMMON_DATA;

typedef struct s_can_client_transmit_nomal_data_tag {
	unsigned char				canType;
	unsigned char				byte_order;
	unsigned char				data_type;
	unsigned char				dlc;

	float						factor;
	short int					startBit;
	short int					bitCount;
	long						canID;
	float						offset;
	float						default_value;
	unsigned long				send_period;
	short int					func_div;
	short int					func_div2;
	short int					func_div3;
	short int					reserved1;
} S_CAN_CLIENT_TRANSMIT_NORMAL_DATA;

typedef struct s_can_client_send_transmit_set_data_tag {
	S_CAN_CLIENT_CMD_HEADER				header;
	S_CAN_CLIENT_TRANSMIT_COMMON_DATA	commonData[MAX_CAN_TYPE];
	S_CAN_CLIENT_TRANSMIT_NORMAL_DATA	normalData[MAX_TX_LTC_DATA];
} S_CAN_CLIENT_SEND_TRANSMIT_SET_DATA;

typedef struct s_can_client_rcv_cmd_heartbeat_req_tag {
	S_CAN_CLIENT_CMD_HEADER		header;
} S_CAN_CLIENT_RCV_CMD_HEARTBEAT_REQ;

typedef struct s_can_client_rcv_cmd_heartbeat_rpy_tag {
	S_CAN_CLIENT_CMD_HEADER		header;
} S_CAN_CLIENT_RCV_CMD_HEARTBEAT_RPY;

typedef struct s_can_client_response_tag {
	unsigned long				cmd_id;
	unsigned char				state;
	unsigned char				reserved[3];
} S_CAN_CLIENT_RESPONSE;

typedef struct s_can_client_rcv_cmd_response_tag {
	S_CAN_CLIENT_CMD_HEADER		header;
	S_CAN_CLIENT_RESPONSE		response;
} S_CAN_CLIENT_RCV_CMD_RESPONSE;

typedef struct s_can_client_send_cmd_response_tag {
	S_CAN_CLIENT_CMD_HEADER		header;
	S_CAN_CLIENT_RESPONSE		response;
} S_CAN_CLIENT_SEND_CMD_RESPONSE;

typedef struct s_can_client_receive_can_data_tag {
	unsigned char				data[8];
	unsigned long				can_id;
} S_CAN_CLIENT_RECEIVE_CAN_DATA;

typedef struct s_can_client_rcv_receive_data_tag {
	S_CAN_CLIENT_CMD_HEADER			header;
	S_CAN_CLIENT_RECEIVE_CAN_DATA	value[MAX_RX_LTC_DATA];
} S_CAN_CLIENT_RCV_RECEIVE_DATA;

typedef struct s_can_client_transmit_pack_data_tag {
	long						voltage;
	long						Isens;
	long						tc[MAX_TC_DATA];
	long						ntc[MAX_NTC_DATA];
	long						auxV[MAX_AUX_V_DATA];
	//shhw_231014s				MAX_TX_FUNC_DATA: 10	
	unsigned char				cell_balancing;
	unsigned char				reserved[3];
	
	short int					tx_func_div[MAX_TX_FUNC_DATA];	
	long						tx_func_data[MAX_TX_FUNC_DATA];	
	//shhw_231014e
	
} S_CAN_CLIENT_TRANSMIT_PACK_DATA;

typedef struct s_can_client_send_pack_data_tag {
	S_CAN_CLIENT_CMD_HEADER				header;
	S_CAN_CLIENT_TRANSMIT_PACK_DATA	pack_data[4];
} S_CAN_CLIENT_SEND_PACK_DATA;

typedef struct s_can_client_rcv_cmd_start_req_tag {
	S_CAN_CLIENT_CMD_HEADER		header;
} S_CAN_CLIENT_RCV_CMD_START_REQ;

typedef struct s_can_client_rcv_cmd_receive_set_req_tag {
	S_CAN_CLIENT_CMD_HEADER		header;
} S_CAN_CLIENT_RCV_CMD_RECEIVE_SET_REQ;

typedef struct s_can_client_rcv_cmd_transmit_set_req_tag {
	S_CAN_CLIENT_CMD_HEADER		header;
} S_CAN_CLIENT_RCV_CMD_TRANSMIT_SET_REQ;

typedef struct s_can_client_misc_tag {
	unsigned long	timer;
	unsigned long	network_timer;
	unsigned long	cmd_serial;
	int				psSignal;
	int				processPointer;
	int				CanNo;
	short int		tc_ch[4][MAX_TC_DATA];
	short int		ntc_ch[4][MAX_NTC_DATA];
	short int		auxV_ch[4][MAX_AUX_V_DATA];
	short int		msg_count;
	long			sent_monitor_data_time;
	long			sent_monitor_data_time2;
	//shhw_231014s	MAX_TX_FUNC_DATA: 10
	short int   	tx_func_div[4][MAX_TX_FUNC_DATA];
	float			tx_func_data[4][MAX_TX_FUNC_DATA];
	//shhw_231014e
} S_CAN_CLIENT_MISC;

typedef struct s_can_client_config_tag {
   	char			ipAddr[16];
	int				sendPort;
	int				receivePort;
	int				networkPort; //common port

   	int    			send_socket;
   	int    			fd_socket;
   	int    			receive_socket;
   	int    			network_socket; //common socket

	unsigned short	retryTimeout;
	unsigned short	retryCount;
	unsigned int	heartbeatTimeout;
	
	unsigned char	CmdSendLog;
	unsigned char	CmdRcvLog;
	unsigned char	CmdSendLog_Hexa;
	unsigned char	CmdRcvLog_Hexa;
	
	unsigned int	protocol_version;
} S_CAN_CLIENT_CONFIG;

typedef struct s_can_msg_tag {
	S_CAN_CLIENT_RECEIVE_CAN_DATA	msg[MAX_RX_LTC_DATA];
	unsigned char	data_count;
	unsigned char	reserved[3];
} S_CAN_MSG;

typedef struct s_canClient_tag {
	S_CAN_CLIENT_MISC		misc;
	S_CAN_CLIENT_CONFIG		config;
	S_P1_RCV_PACKET			rcvPacket;
	S_P1_RCV_COMMAND		rcvCmd;
	S_CAN_MSG				can_msg[4];
	unsigned long			pingTimer;
	unsigned long			netTimer;
	unsigned short int		pingCount;
	unsigned short int		reserved;
	unsigned char			signal[MAX_SIGNAL];
} S_CAN_CLIENT; 
#endif
