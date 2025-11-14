#ifndef __TEMP_CLIENT_DEF_H__
#define __TEMP_CLIENT_DEF_H__

#define	PROTOCOL_VERSION_1					20190905

#define MAX_MODULE_PER_TEMP_CONV_BD			13
#define MAX_TEMP_SENSOR_PER_MODULE			10

// TempClient Signal
#define TEMP_SIG_NET_CONNECTED				0
#define TEMP_SIG_NET_CONNECT_RETRY			1
#define TEMP_SIG_TEST_STEP_RCV				2
#define TEMP_SIG_CLIENT_CONNECT				3

// General Cmd
// SBC -> Temp Converter
#define	SBC_TO_TEMP_CMD_HEARTBEAT_REQ			101
#define	SBC_TO_TEMP_CMD_HEARTBEAT_RPY			102
#define	SBC_TO_TEMP_CMD_TEMP_DATA_REQ			103

//Temp Converter -> SBC
#define	TEMP_TO_SBC_CMD_HEARTBEAT_REQ		101
#define	TEMP_TO_SBC_CMD_HEARTBEAT_RPY		102
#define	TEMP_TO_SBC_CMD_TEMP_DATA_RPY		103

// ethernet protocol
#define TEMP_CLIENT_STX			0x02
#define TEMP_CLIENT_ETX			0x03

#endif
