#ifndef __SIL_CLIENT_DEF_H__
#define __SIL_CLIENT_DEF_H__

#define	PROTOCOL_VERSION_1					20190905

#define MAX_SENSOR_TYPE						2
//shhw_231127s
#define MAX_DAQ_CHANNEL						256
#define MAX_TC_CHANNEL						100	
//#define MAX_SENSOR_CHANNEL				256
//shhw_231127e

// SilClient Signal
#define SIL_SIG_NET_CONNECTED				0
#define SIL_SIG_NET_CONNECT_RETRY			1
#define SIL_SIG_TEST_STEP_RCV				2
#define SIL_SIG_CLIENT_CONNECT				3

// General Cmd
// SBC -> SIL
#define SBC_TO_SIL_CMD_MODULE_INFO			0x00000010
#define SBC_TO_SIL_CMD_RESPONSE				0x00000001
#define SBC_TO_SIL_CMD_START				0x00000040
#define SBC_TO_SIL_CMD_STOP					0x00000041

//SIL -> SBC
#define	SIL_TO_SBC_CMD_HEARTBEAT_REQ		0x00000011
#define SIL_TO_SBC_CMD_RESPONSE				0x00000001
#define	SIL_TO_SBC_CMD_DAQ_DATA_RPY			0x00000020

//response code
#define SIL_CLIENT_CD_ACK					0x00000001
#define SIL_CLIENT_CD_NACK					0x00000002

// ethernet protocol
#define SIL_CLIENT_STX			0x0002
#define SIL_CLIENT_ETX			0x0003

#endif
