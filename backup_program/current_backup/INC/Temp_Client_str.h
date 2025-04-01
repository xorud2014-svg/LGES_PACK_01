#ifndef __TEMP_CLIENT_STR_H__
#define __TEMP_CLIENT_STR_H__

#include "SysDefine.h"
#include "P1_Client_str.h"
#include "P1_Client_def.h"
#include "Temp_Client_def.h"

//typedef unsigned char		BYTE;
//typedef unsigned short	WORD;
//typedef unsigned long		DWORD;

// HOST <=> CLIENT Command Format define
typedef struct s_temp_client_cmd_header_tag {
	char		cmd_id[4];
	char		body_size[4];
} S_TEMP_CLIENT_CMD_HEADER;

typedef struct s_temp_client_send_cmd_heartbeat_req_tag {
	S_TEMP_CLIENT_CMD_HEADER		header;
} S_TEMP_CLIENT_SEND_CMD_HEARTBEAT_REQ;

typedef struct s_temp_client_send_cmd_temp_data_req_tag {
	S_TEMP_CLIENT_CMD_HEADER		header;
} S_TEMP_CLIENT_SEND_CMD_TEMP_DATA_REQ;

typedef struct s_temp_client_send_cmd_response_tag {
	S_TEMP_CLIENT_CMD_HEADER		header;
} S_TEMP_CLIENT_SEND_CMD_RESPONSE;

typedef struct s_temp_client_rcv_cmd_heartbeat_rpy_tag {
	S_TEMP_CLIENT_CMD_HEADER		header;
} S_TEMP_CLIENT_RCV_CMD_HEARTBEAT_RPY;

typedef struct s_temp_client_rcv_cmd_response_tag {
	S_TEMP_CLIENT_CMD_HEADER		header;
} S_TEMP_CLIENT_RCV_CMD_RESPONSE;

typedef struct s_temp_client_temp_data_tag {
	char	temp[MAX_TEMP_SENSOR_PER_MODULE][8];
} S_TEMP_CLIENT_TEMP_DATA;

typedef struct s_temp_client_rcv_cmd_temp_data_rpy_tag {
	S_TEMP_CLIENT_CMD_HEADER		header;
	S_TEMP_CLIENT_TEMP_DATA 	Module[MAX_MODULE_PER_TEMP_CONV_BD];
} S_TEMP_CLIENT_RCV_CMD_TEMP_DATA;

typedef struct s_temp_client_misc_tag {
	unsigned long		timer;
	unsigned long		network_timer;
	unsigned long		cmd_serial;
	int					psSignal;
	int					processPointer;
	int					TempNo;
} S_TEMP_CLIENT_MISC;

typedef struct s_temp_client_config_tag {
   	char		ipAddr[16];
	int			sendPort;
	int			receivePort;
	int			networkPort; //common port

   	int    		send_socket;
   	int    		fd_socket;
   	int    		receive_socket;
   	int    		network_socket; //common socket

	unsigned short	retryTimeout;
	unsigned short	retryCount;
	unsigned int	heartbeatTimeout;
	
	unsigned char		CmdSendLog;
	unsigned char		CmdRcvLog;
	unsigned char		CmdSendLog_Hexa;
	unsigned char		CmdRcvLog_Hexa;
	
	unsigned int		protocol_version;

	unsigned char		crc_type;
	char				SensorType;
	unsigned short		installedTemp;
} S_TEMP_CLIENT_CONFIG;

typedef struct s_tempClient_tag {
	S_TEMP_CLIENT_MISC				misc;
	S_TEMP_CLIENT_CONFIG			config;
	S_P1_RCV_PACKET		rcvPacket;
	S_P1_RCV_COMMAND	rcvCmd;
	unsigned long		pingTimer;
	unsigned long		netTimer;
	unsigned short int	pingCount;
	unsigned short int	reserved;
	unsigned char		signal[MAX_SIGNAL];
} S_TEMP_CLIENT; 
#endif
