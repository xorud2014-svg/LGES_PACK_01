#ifndef __SIL_CLIENT_STR_H__
#define __SIL_CLIENT_STR_H__

#include "SysDefine.h"
#include "P1_Client_str.h"
#include "P1_Client_def.h"
#include "Sil_Client_def.h"

// HOST <=> CLIENT Command Format define
typedef struct s_sil_client_cmd_header_tag {
	short int	module_no[2];
	long		cmd_id;
	long		body_size;
} S_SIL_CLIENT_CMD_HEADER;

typedef struct s_sil_client_cmd_tail_tag {
	short int	reserved;
	short int	etx;
} S_SIL_CLIENT_CMD_TAIL;

typedef struct s_sil_client_send_cmd_module_info_tag {
	S_SIL_CLIENT_CMD_HEADER		header;
	short int					reserved[4];
	S_SIL_CLIENT_CMD_TAIL		tail;
} S_SIL_CLIENT_SEND_CMD_MODULE_INFO;

typedef struct s_sil_client_response_tag {
	int	cmd_id;
	int	code;
} S_SIL_CLIENT_RESPONSE;

typedef struct s_sil_client_rcv_cmd_response_tag {
	S_SIL_CLIENT_CMD_HEADER		header;
	S_SIL_CLIENT_RESPONSE		response;
	S_SIL_CLIENT_CMD_TAIL		tail;
} S_SIL_CLIENT_RCV_CMD_RESPONSE;

typedef struct s_sil_client_send_cmd_response_tag {
	S_SIL_CLIENT_CMD_HEADER		header;
	S_SIL_CLIENT_RESPONSE		response;
	S_SIL_CLIENT_CMD_TAIL		tail;
} S_SIL_CLIENT_SEND_CMD_RESPONSE;

typedef struct s_sil_client_rcv_cmd_heartbeat_req_tag {
	S_SIL_CLIENT_CMD_HEADER		header;
	S_SIL_CLIENT_CMD_TAIL		tail;
} S_SIL_CLIENT_RCV_CMD_HEARTBEAT_REQ;

typedef struct s_sil_client_rcv_cmd_start_req_tag {
	S_SIL_CLIENT_CMD_HEADER		header;
	S_SIL_CLIENT_CMD_TAIL		tail;
} S_SIL_CLIENT_RCV_CMD_START_REQ;

typedef struct s_sil_client_rcv_cmd_stop_req_tag {
	S_SIL_CLIENT_CMD_HEADER		header;
	S_SIL_CLIENT_CMD_TAIL		tail;
} S_SIL_CLIENT_RCV_CMD_STOP_REQ;

typedef struct s_sil_client_temp_data_tag {
	long	daq[MAX_DAQ_CHANNEL];		//shhw_231127s
	long	tc[MAX_TC_CHANNEL];
	//long	temp[MAX_SENSOR_CHANNEL];	//shhw_231127e
} S_SIL_CLIENT_DAQ_DATA;

typedef struct s_sil_client_rcv_cmd_temp_data_rpy_tag {
	S_SIL_CLIENT_CMD_HEADER		header;
	long						loss_check;
	//S_SIL_CLIENT_DAQ_DATA	 	Module[MAX_SENSOR_TYPE];
	S_SIL_CLIENT_DAQ_DATA	 	Module;
	S_SIL_CLIENT_CMD_TAIL		tail;
} S_SIL_CLIENT_RCV_CMD_DAQ_DATA;

typedef struct s_sil_client_misc_tag {
	unsigned long	timer;
	unsigned long	network_timer;
	unsigned long	cmd_serial;
	int				psSignal;
	int				processPointer;
	int				SilNo;
} S_SIL_CLIENT_MISC;

typedef struct s_sil_client_config_tag {
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

	unsigned char	crc_type;
	char			SensorType;
	unsigned short	installedTemp;
} S_SIL_CLIENT_CONFIG;

typedef struct s_silClient_tag {
	S_SIL_CLIENT_MISC		misc;
	S_SIL_CLIENT_CONFIG		config;
	S_P1_RCV_PACKET			rcvPacket;
	S_P1_RCV_COMMAND		rcvCmd;
	unsigned long			pingTimer;
	unsigned long			netTimer;
	unsigned short int		pingCount;
	unsigned short int		reserved;
	unsigned char			signal[MAX_SIGNAL];
} S_SIL_CLIENT; 
#endif
