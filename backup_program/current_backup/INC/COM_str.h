#ifndef __COM_STR_H__
#define __COM_STR_H__

#include "SysDefine.h"
#include "COM_def.h"

typedef struct s_com_config_tag {
	unsigned char		functionType[MAX_COM_PORT];
	unsigned char		functionModel[MAX_COM_PORT];

	unsigned char		comPortId[MAX_COM_PORT];
	int					comBps[MAX_COM_PORT];
	unsigned char		commType[MAX_COM_PORT];
	unsigned char		externPort[MAX_COM_PORT];

	unsigned char		CmdSendLog[MAX_COM_PORT];
	unsigned char		CmdRcvLog[MAX_COM_PORT];
	unsigned char		CmdSendLog_Hex[MAX_COM_PORT];
	unsigned char		CmdRcvLog_Hex[MAX_COM_PORT];
	unsigned char		CommCheckLog[MAX_COM_PORT];

	unsigned char		autoStart[MAX_COM_PORT];
	unsigned char		countMeter[MAX_COM_PORT];
	unsigned char		readType[MAX_COM_PORT];
	unsigned char		measureI[MAX_COM_PORT];

	long				reserved1[MAX_COM_PORT];
	long				reserved2[MAX_COM_PORT];
} S_COM_CONFIG;

typedef struct s_com_rcv_packet_tag {
	int					usedBufSize;

	int					rcvCount;
	int					rcvStartPoint[MAX_COM_PACKET_COUNT];
	int					rcvSize[MAX_COM_PACKET_COUNT];
	char				rcvPacketBuf[MAX_COM_PACKET_LENGTH];

	int					parseCount;
	int					parseStartPoint[MAX_COM_PACKET_COUNT];
} S_COM_RCV_PACKET;

typedef struct s_com_rcv_command_tag {
	int					cmdBufSize;
	char				cmd[MAX_COM_PACKET_LENGTH];
	char				cmdBuf[MAX_COM_PACKET_LENGTH];
	char				tmpBuf[MAX_COM_PACKET_LENGTH];
	int					cmdFail;
	int					cmdSize;

	int					rcvCmdIndex;
	char				rcvCmd[MAX_COM_PACKET_LENGTH];
	unsigned char		rcvCmdCompleteFlag;
	unsigned char		rcvCmdRestFlag;
	unsigned char		reserved1[2];
} S_COM_RCV_COMMAND;

typedef struct s_com_cali_meter1_cmd_header_tag {
	char				sign1;
	char				digit1;
	char				dot;
	char				digit2[8];
	char				exponent;
	char				sign2;
	char				digit3[2];
	char				cr;
	char				nl;
} S_COM_CALI_METER1_CMD_HEADER;

//jhkw_131209s
typedef struct s_com_cali_meter1_4700_cmd_header_tag {
    char                sign1;
    char                digit1[7];
    char                exponent;
    char                sign2;
	char                digit3[2];
    char                cr;
    char                nl;
} S_COM_CALI_METER1_4700_CMD_HEADER;
//jhkw_131209e

typedef struct s_com_cali_meter1_34461A_cmd_header_tag {
	char				sign1;
	char				digit1;
	char				dot;
	char				digit2[8];
	char				exponent;
	char				sign2;
	char				digit3[2];
	char				nl;
} S_COM_CALI_METER1_34461A_CMD_HEADER;		//csk_170908

typedef struct s_com_analog_meter_cmd_header_tag {
	char				stx;
	char				addr[2];
	char				cmd[3];
} S_COM_ANALOG_METER_CMD_HEADER;

typedef struct s_com_analog_meter_send_cmd_request_tag {
	char				data[8];
} S_COM_ANALOG_METER_SEND_CMD_REQUEST;

typedef struct s_com_cali_switch1_cmd_header_tag {
	char				stx;
	char				addr[2];
	char				cmd[3];
} S_COM_CALI_SWITCH1_CMD_HEADER;

typedef struct s_com_comm_b_cmd_header_tag {
	char				stx;
	char				addr[2];
	char				cmd[3];
} S_COM_COMM_B_CMD_HEADER;

typedef struct s_com_display_cmd_header_tag {
	char				stx;
	char				addr[2];
	char				cmd[2];
} S_COM_DISPLAY_CMD_HEADER;

typedef struct s_com_plc_q03ud_cmd_header_tag {
	char				stx;
	char				addr[2];
	char				plcNo[2];
} S_COM_PLC_Q03UD_CMD_HEADER;

typedef struct s_com_eiom_b_cmd_header_tag {
	char				stx;
	char				addr[2];
	char				cmd[3];
	char				size[2];
} S_COM_EIOM_B_CMD_HEADER;

typedef struct s_com_rcv_cmd_answer_display_tag {
	char				data[12];
} S_COM_RCV_CMD_ANSWER_DISPLAY;

typedef struct s_com_send_cmd_display_value_tag {
	char				data[20];
} S_COM_SEND_CMD_DISPLAY_VALUE;

typedef struct s_com_misc_tag {
	unsigned char		externPort_useFlag;
	unsigned char		retry_count;
	unsigned char		retry_time;
	unsigned char		reserved1;

	unsigned char		bcr_reservation[MAX_GROUP_4];

	char				tmp_tray_id[3][COM_BCR_SIZE];
	char				tray_id[MAX_GROUP_4][COM_BCR_SIZE];

	long				delay_time;
	unsigned long		plc_idle_time;

	int					TEMP_NPV;
	int					TEMP_NSP;
//	int					DAMP_NSP;
//	int					P_TEMP_TSP;
//	int					N_TEMP_TSP;
	int					NOWSTS;
	int					OTHERSTS;
	int					PROC_TIME_H;
	int					PROC_TIME_L;
	int 				DIDATA;			//csk_161028
} S_COM_MISC;

typedef struct s_com_port_tag {
	S_COM_RCV_PACKET	rcvPacket;
	S_COM_RCV_COMMAND	rcvCmd;

	S_COM_MISC			misc;

	unsigned char		signal[MAX_SIGNAL];

	long				value[MAX_AUX_DATA];
	long				temp_value[MAX_AUX_DATA][3];

	unsigned char		temp_cnt;
	unsigned char		temp_cnt_flag;
	unsigned char		reserved[2];
	
	long				test_value[MAX_AUX_DATA]; //kjhw_141027
	unsigned char		fault_count[MAX_AUX_DATA]; //kjhw_141027
} S_COM_PORT;

typedef struct s_com_tag {
	S_COM_CONFIG		config;
	
	S_COM_PORT			com_port[MAX_COM_PORT];
} S_COM;

#endif
