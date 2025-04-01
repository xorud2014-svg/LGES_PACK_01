#ifndef __IOCOMM_STR_H__
#define __IOCOMM_STR_H__

#include "SysDefine.h"
#include "IOComm_def.h"

typedef struct s_io_comm_misc_tag {
	int					processPointer;
	int					timer;
} S_IO_COMM_MISC;

typedef struct s_io_comm_config_tag {
	int					comPort;
	int					comBps;
	int					ttyS_fd;
	unsigned char		CmdSendLog;
	unsigned char		CmdRcvLog;
	unsigned char		CmdSendLog_Hex;
	unsigned char		CmdRcvLog_Hex;
	
	unsigned char		CommCheckLog;
	unsigned char		countMeter;
	unsigned char		readType;
	unsigned char		ioCommDelay;
	
	unsigned char		autoStart;
	unsigned char		reserved[3];
	long				measure_offset[MAX_METER_COUNT][MAX_METER_CH];
} S_IO_COMM_CONFIG;

typedef struct s_io_comm_map_tag {
	short int			monitor_ch;
	unsigned char		port;
	unsigned char		output;
} S_IO_COMM_MAP;

typedef struct s_io_comm_rcv_packet_tag {
	int					usedBufSize;

	int					rcvCount;
	int					rcvStartPoint[MAX_IO_COMM_PACKET_COUNT];
	int					rcvSize[MAX_IO_COMM_PACKET_COUNT];
	char				rcvPacketBuf[MAX_IO_COMM_PACKET_LENGTH];

	int					parseCount;
	int					parseStartPoint[MAX_IO_COMM_PACKET_COUNT];
} S_IO_COMM_RCV_PACKET;

typedef struct s_io_comm_rcv_command_tag {
	int					cmdBufSize;
	char				cmd[MAX_IO_COMM_PACKET_LENGTH];
	char				cmdBuf[MAX_IO_COMM_PACKET_LENGTH];
	char				tmpBuf[MAX_IO_COMM_PACKET_LENGTH];
	int					cmdFail;
	int					cmdSize;

	int					rcvCmdIndex;
	char				rcvCmd[MAX_IO_COMM_PACKET_LENGTH];
	unsigned char		rcvCmdCompleteFlag;
	unsigned char		rcvCmdRestFlag;
	unsigned char		reserved1;
	unsigned char		reserved2;
} S_IO_COMM_RCV_COMMAND;

//typedef struct s_io_comm_cmd_header_tag {
//	char				stx;
//	char				addr1;
//	char				addr2;
//} S_IO_COMM_CMD_HEADER;

typedef struct s_io_comm_cmd_header_tag {
	char				stx;
	char				addr[2];
	char				cmd[3];
} S_IO_COMM_CMD_HEADER;

typedef struct s_io_comm_rcv_cmd_answer_tag {
	char						data[100];
} S_IO_COMM_RCV_CMD_ANSWER;

typedef struct s_io_comm_send_cmd_data_tag {
	S_IO_COMM_CMD_HEADER header;
	char				  buff[100];
} S_IO_COMM_SEND_CMD_DATA;

typedef struct s_io_comm_send_cmd_initialize_tag {
	char				data[100];
} S_IO_COMM_SEND_CMD;

typedef struct s_io_comm_send_cmd_request_tag {
	char				data[8];
} S_IO_COMM_SEND_CMD_REQUEST;

typedef struct s_io_port_tag {
	unsigned char	Input[8];
	unsigned char	Output[8];
} S_IO_PORT;

typedef struct s_io_comm_tag {
	S_IO_COMM_MISC		misc;
	S_IO_COMM_CONFIG	config;
	S_IO_COMM_MAP		map[MAX_GROUP_4][MAX_CH_256];
	
	S_IO_COMM_RCV_PACKET	rcvPacket;
	S_IO_COMM_RCV_COMMAND	rcvCmd;
	S_IO_PORT				port[MAX_JIG_4];

	unsigned char		signal[MAX_SIGNAL];

	long				temperature[MAX_JIG_4][8];
} S_IO_COMM;

#endif
